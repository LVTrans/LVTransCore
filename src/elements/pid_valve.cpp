#include "lvtrans/elements/pid_valve.hpp"
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_states.hpp"

namespace lvtrans {

PIDValve::PIDValve(const PIDValveParameters& parameters, const double dt)
    : m_params{parameters},
      m_state{PIDValveState{.SP_q = parameters.SP_Q / parameters.Q_wanted}},
      m_dt{dt} {}

PIDValve::PIDValve() = default;
PIDValve::~PIDValve() = default;
void PIDValve::iterate(const double) {
  pid_iterate();
  servo_1st_order();
}

void PIDValve::pid_iterate() {
  const auto& p = m_params;
  const auto& s = m_state;

  const double pv = s.PV_q;

  double tf = p.Td <= 0.0 ? 0.001 : s.Tf;

  if (tf < 0.0) {
    tf = 0.1 * p.Td;
  }

  // Simpsons
  if (!m_initialized) {
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
  if (s.antiwindup) {
    windup = (s.e_t_h * p.Ti) / (p.Kp * s.Tt);
  }
  windup += error;

  double integrator = 0;
  if (!m_initialized) {
    integrator = s.integral_initial_value;
  }

  if (p.Ti > 0.0) {
    integrator =
        (((m_previous_windup * 4.0 + m_previous_windup_0 + windup) / 6.0) *
         p.Kp * m_dt) /
        p.Ti;
  } else {
    integrator = 0.0;
  }
  integrator += m_previous_integral;

  integrator = std::clamp(integrator, s.integral_min, s.integral_max);

  m_previous_integral = integrator;

  m_previous_windup = windup;
  m_previous_windup_0 = m_previous_windup;

  m_pid_output = proportional + integrator;
  if (!s.automatic) {
    m_pid_output = s.kappa_man;
  }

  if (!m_initialized) {
    m_initialized = true;
  }
}

void PIDValve::servo_1st_order() {
  // constexpr auto opening_time{10.0};  // [s]
  constexpr auto closing_time{10.0};  // [s]
  // constexpr auto K1{1.0};
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
  m_previous_output = output;
}

ElementType PIDValve::get_type() const { return ElementType::PIDValve; }

ElementParameters PIDValve::get_parameters() const { return m_params; }

std::optional<ElementState> PIDValve::get_state() const { return std::nullopt; }

ElementView PIDValve::read_view() const { return ElementView{}; }

void PIDValve::reset_state() {
  m_initialized = false;
  m_previous_time = 0.0;
  m_previous_pv = 0.0;
  m_filtered_pv = 0.0;
}

}  // namespace lvtrans
