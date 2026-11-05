#include "lvtrans/elements/pid_valve.hpp"
#include <algorithm>
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_states.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"

namespace lvtrans {

PIDValve::PIDValve(const PIDValveParameters& parameters, const double dt)
    : m_params{parameters},
      m_state{PIDValveState{.SP_q = parameters.SP_Q / parameters.Q_wanted}},
      m_dt{dt} {}

PIDValve::PIDValve() : PIDValve(PIDValveParameters{}, 0.1) {}
PIDValve::~PIDValve() { disconnect_all(); }
void PIDValve::iterate(const double) {
  if (const auto* valve =
          dynamic_cast<const ValveInternalServo*>(m_controllable)) {
    set_PV_Q(valve->get_state_as<ValveInternalServoState>().Q);
  }
  pid_iterate();
  servo_1st_order();
  m_state.e_t_h = m_state.kappa_servo - m_pid_output;
  m_state.PV_Q_out = m_state.PV_Q;
  m_state.PV_q_out = m_state.PV_q;
}

void PIDValve::pid_iterate() {
  const auto& p = m_params;
  const auto& s = m_state;

  const bool initialize = !m_initialized || m_mode == Mode::Init || s.reset;
  const double pv = s.PV_q;

  double tf = p.Td <= 0.0 ? 0.001 : s.Tf;

  if (tf < 0.0) {
    tf = 0.1 * p.Td;
  }

  if (initialize) {
    m_previous_pv = pv;
  }

  m_filtered_pv =
      (tf * m_filtered_pv + (m_dt + p.Td) * pv - p.Td * m_previous_pv) /
      (m_dt + tf);
  m_previous_pv = pv;

  const double error = s.SP_q - m_filtered_pv;
  const double proportional = p.Kp * error;

  // Anti-windup
  double windup{0.0};
  if (s.antiwindup && p.Kp != 0.0 && s.Tt > 0.0) {
    windup = (s.e_t_h * p.Ti) / (p.Kp * s.Tt);
  }
  windup += error;

  // integrator using Simpson's rule
  double integrator = s.integral_initial_value;
  if (!initialize) {
    integrator = p.Ti > 0.0
                     ? m_previous_integral + (windup + 4.0 * m_previous_windup +
                                              m_previous_windup_0) *
                                                 p.Kp * m_dt / (6.0 * p.Ti)
                     : 0.0;
  }

  integrator = std::clamp(integrator, s.integral_min, s.integral_max);

  m_previous_integral = integrator;

  m_previous_windup_0 = m_previous_windup;
  m_previous_windup = windup;

  m_pid_output = proportional + integrator;
  if (!s.automatic) {
    m_pid_output = s.kappa_man;
  }

  if (initialize) {
    m_mode = s.automatic ? Mode::Auto : Mode::Manual;
  } else if ((m_mode == Mode::Auto && !s.automatic) ||
             (m_mode == Mode::Manual && s.automatic)) {
    m_mode = Mode::Init;
  }
  m_initialized = true;
}

void PIDValve::servo_1st_order() {
  constexpr auto closing_time{10.0};  // [s]
  constexpr auto T1{0.001};
  const auto input{m_pid_output};
  const double previous{m_previous_output};

  const double candidate = (T1 * previous + input * m_dt) / (T1 + m_dt);
  const double rate = (candidate - previous) / m_dt;
  const double max_rate = 1.0 / closing_time;
  double output{0.0};
  m_ouput = candidate;
  if (rate > max_rate) {
    m_ouput = max_rate * m_dt + previous;
  } else if (rate < -max_rate) {
    m_ouput = -max_rate * m_dt + previous;
  }
  output = std::clamp(m_ouput, 0.0, 1.0);
  m_state.kappa_servo = output;
  m_previous_output = m_ouput;
}

ElementType PIDValve::get_type() const { return ElementType::PIDValve; }

ElementParameters PIDValve::get_parameters() const { return m_params; }

std::optional<ElementState> PIDValve::get_state() const { return std::nullopt; }

ElementView PIDValve::read_view() const {
  return {
      .element_id = get_ID(),
      .values = {
          ScalarValue{.name = "Flow setpoint",
                      .unit = "m³/s",
                      .value = m_state.SP_q * m_params.Q_wanted},
          ScalarValue{
              .name = "Measured flow", .unit = "m³/s", .value = m_state.PV_Q},
          ScalarValue{
              .name = "Filtered flow", .unit = "pu", .value = m_filtered_pv},
          ScalarValue{
              .name = "Integral", .unit = "pu", .value = m_previous_integral},
          ScalarValue{
              .name = "Raw PID output", .unit = "pu", .value = m_pid_output},
          ScalarValue{.name = "Servo output",
                      .unit = "pu",
                      .value = m_state.kappa_servo},
          ScalarValue{.name = "Anti-windup residual",
                      .unit = "pu",
                      .value = m_state.e_t_h}}};
}

void PIDValve::reset_state() {
  m_mode = Mode::Init;
  m_initialized = false;
  m_previous_time = 0.0;
  m_previous_pv = 0.0;
  m_filtered_pv = 0.0;
  m_previous_output = 0.0;
  m_previous_integral = 0.0;
  m_previous_windup = 0.0;
  m_previous_windup_0 = 0.0;
  m_pid_output = 0.0;
  m_ouput = 0.0;
  m_state.e_t_h = 0.0;
  m_state.kappa_servo = 0.0;
}

void PIDValve::set_controls(bool active, bool automatic, bool antiwindup,
                            double manual, double Tt, double Tf) {
  m_state.active = active;
  m_state.automatic = automatic;
  m_state.antiwindup = antiwindup;
  m_state.kappa_man = manual;
  m_state.Tt = Tt;
  m_state.Tf = Tf;
}

void PIDValve::set_gains(double Kp, double Ti, double Td) {
  m_params.Kp = Kp;
  m_params.Ti = Ti;
  m_params.Td = Td;
}

}  // namespace lvtrans
