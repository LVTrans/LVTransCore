#include "lvtrans/elements/valve_internal_servo.hpp"
#include <cmath>
#include "lvtrans/controller_connections.hpp"
#include "lvtrans/elements/base_pipe.hpp"

namespace lvtrans {

ValveInternalServo::ValveInternalServo(
    const ValveInternalServoParameters& parameters, double dt)
    : ValveInternalServo(
          parameters, dt,
          {parameters.initial_opening, parameters.initial_opening, false}) {}

ValveInternalServo::ValveInternalServo(
    const ValveInternalServoParameters& parameters, double dt,
    ValveInternalServoState state)
    : NonPipe(1, 1), m_params(parameters), m_dt(dt), m_state(state) {}

ValveInternalServo::~ValveInternalServo() { disconnect_all(); }

void ValveInternalServo::disconnect_all() noexcept {
  NonPipe::disconnect_all();
  ControllerConnections::disconnect_all(*this);
}

ModificationResult ValveInternalServo::modify(const ElementModification& mod) {
  const auto* opening = std::get_if<SetValveOpening>(&mod);
  if (!opening) return ModificationResult::UnsupportedModification;
  if (!std::isfinite(opening->tau) || opening->tau < 0 || opening->tau > 1) {
    return ModificationResult::InvalidValue;
  }
  m_state.setpoint = opening->tau;
  return ModificationResult::Ok;
}

void ValveInternalServo::iterate(double) {
  const double previous = m_state.tau;

  const double closing_time = previous >= m_params.switch_over
                                  ? m_params.closing_time_high
                                  : m_params.closing_time_low;
  const double candidate =
      (m_params.servo_time * previous + m_state.setpoint * m_dt) /
      (m_params.servo_time + m_dt);
  const double rate = (candidate - previous) / m_dt;
  const double max_rate = 1.0 / closing_time;
  m_state.tau = candidate;
  m_state.rate_limited = false;
  if (rate > max_rate) {
    m_state.tau = max_rate * m_dt + previous;
    m_state.rate_limited = true;
  } else if (rate < -max_rate) {
    m_state.tau = -max_rate * m_dt + previous;
    m_state.rate_limited = true;
  }

  const double q = calculate_q();
  if (const auto* pipe = left()) {
    const auto& c = m_c_plus.at(pipe->get_ID());
    m_state.H1 = c.c - c.b * q;
  }
  if (const auto* pipe = right()) {
    const auto& c = m_c_minus.at(pipe->get_ID());
    m_state.H2 = c.c + c.b * q;
  }
}

double ValveInternalServo::get_H(CharacteristicsType type, ElementID id) {
  const double q = calculate_q();
  if (type == CharacteristicsType::Plus) {
    const auto& c = m_c_plus.at(id);
    return c.c - c.b * q;
  } else {
    const auto& c = m_c_minus.at(id);
    return c.c + c.b * q;
  }
}

double ValveInternalServo::get_Q(CharacteristicsType, ElementID, IterateInput) {
  return calculate_q();
}

void ValveInternalServo::reset_state() {
  m_state = {m_params.initial_opening, m_params.initial_opening, false};
}

double ValveInternalServo::calculate_q() {
  Characteristics cp{}, cm{};
  if (const auto* pipe = left()) {
    cp = m_c_plus.at(pipe->get_ID());
  }

  if (const auto* pipe = right()) {
    cm = m_c_minus.at(pipe->get_ID());
  }

  const double delta = cp.c - cm.c;
  const double cv =
      (delta >= 0 ? m_params.cvp : m_params.cvm) * m_state.tau * m_state.tau;
  const double cvb = cv * (cp.b + cm.b);

  double q{};
  if (delta >= 0) {
    q = -cvb + std::sqrt(cvb * cvb + 2.0 * cv * delta);
  } else {
    q = cvb - std::sqrt(cvb * cvb - 2.0 * cv * delta);
  }
  m_state.Q = q;
  return q;
}

ElementView ValveInternalServo::read_view() const {
  return {.element_id = get_ID(),
          .values = {
              ScalarValue{.name = "Opening",
                          .unit = "pu",
                          .symbol = "tau",
                          .value = m_state.tau},
              ScalarValue{.name = "Opening setpoint",
                          .unit = "pu",
                          .value = m_state.setpoint},
              ScalarValue{.name = "Flow", .unit = "m³/s", .value = m_state.Q},
              ScalarValue{.name = "Head", .unit = "m", .value = m_state.H1},
          }};
}

}  // namespace lvtrans
