#include "lvtrans/valve.hpp"
#include <cmath>
namespace lvtrans {

Valve::Valve(const ValveParameters& conf) : Valve(conf, ValveState{conf.tau_i}) {}

Valve::Valve(const ValveParameters& conf, ValveState state)
    : m_config{conf}, m_state{state} {
  m_ports.resize(2);

  m_ports[PortType::Left] = std::make_unique<Port>(*this, PortType::Left);
  m_ports[PortType::Right] = std::make_unique<Port>(*this, PortType::Right);
}

Valve::~Valve() {}

void Valve::iterate(const IterateInput input, IterateOutput) {
  if (input.t < m_config.tc) {
    m_state.tau =
        m_config.tau_i - (m_config.tau_i - m_config.tau_f) *
                             std::pow(input.t / m_config.tc, m_config.em);
  } else {
    m_state.tau = m_config.tau_f;
  }
}

}  // namespace lvtrans
