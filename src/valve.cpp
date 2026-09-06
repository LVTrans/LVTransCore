#include "lvtrans/valve.hpp"
#include "lvtrans/reservoir.hpp"
#include <cmath>
namespace lvtrans {

Valve::Valve(const ValveConfig &conf) : m_config(conf), m_tau(conf.tau_i) {}

Valve::~Valve() {}

void Valve::iterate(const IterateInput input, IterateOutput output) {
  if (input.t < m_config.tc) {
    m_tau = m_config.tau_i - (m_config.tau_i - m_config.tau_f) *
                                 std::pow(input.t / m_config.tc, m_config.em);
  } else {
    m_tau = m_config.tau_f;
  }
}

} // namespace lvtrans
