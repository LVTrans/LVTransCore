#include "lvtrans/valve.hpp"
#include "lvtrans/reservoir.hpp"
#include <cmath>
namespace lvtrans {

Valve::Valve(const ValveConfig &conf) : m_config(conf), m_tau(conf.tau_i) {
  auto port_left = std::make_shared<Port>(*this);
  auto port_right = std::make_shared<Port>(*this);

  m_ports.resize(2);

  m_ports[PortLeft] = port_left;
  m_ports[PortRight] = port_right;
}

Valve::~Valve() {}

void Valve::iterate(const IterateInput input, IterateOutput) {
  if (input.t < m_config.tc) {
    m_tau = m_config.tau_i - (m_config.tau_i - m_config.tau_f) *
                                 std::pow(input.t / m_config.tc, m_config.em);
  } else {
    m_tau = m_config.tau_f;
  }
}

} // namespace lvtrans
