#include "lvtrans/elements/valve.hpp"
#include <cmath>
#include "lvtrans/element_modifications.hpp"
namespace lvtrans {

Valve::Valve(const ValveParameters& conf)
    : Valve(conf, ValveState{conf.tau_i}) {}

Valve::Valve(const ValveParameters& conf, ValveState state)
    : m_params{conf}, m_state{state} {
  m_ports[PortType::Left].emplace(*this, PortType::Left);
  m_ports[PortType::Right].emplace(*this, PortType::Right);
}

ModificationResult Valve::modify(const ElementModification& mod) {
  if (std::get_if<SetValveOpening>(&mod)) {
    // m_config.tau_i = std::get<SetValveTauI>(mod).tau_i;
  }
  return ModificationResult::Ok;
}

void Valve::iterate(const IterateInput input, IterateOutput) {
  if (input.t < m_params.tc) {
    m_state.tau =
        m_params.tau_i - (m_params.tau_i - m_params.tau_f) *
                             std::pow(input.t / m_params.tc, m_params.em);
  } else {
    m_state.tau = m_params.tau_f;
  }
}

}  // namespace lvtrans
