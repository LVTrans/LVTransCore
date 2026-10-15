#include "lvtrans/elements/valve.hpp"
#include <cmath>
#include "lvtrans/element_modifications.hpp"
namespace lvtrans {

Valve::Valve(const ValveParameters& params)
    : Valve(params, ValveState{params.tau_i}) {}

Valve::Valve(const ValveParameters& conf, ValveState state)
    : NonPipe(1, 1), m_params{conf}, m_state{state} {}

ModificationResult Valve::modify(const ElementModification& mod) {
  if (std::get_if<SetValveOpening>(&mod)) {
    // m_config.tau_i = std::get<SetValveTauI>(mod).tau_i;
  }
  return ModificationResult::Ok;
}

void Valve::iterate(const double t) {
  if (t < m_params.tc) {
    m_state.tau = m_params.tau_i - (m_params.tau_i - m_params.tau_f) *
                                       std::pow(t / m_params.tc, m_params.em);
  } else {
    m_state.tau = m_params.tau_f;
  }
}

double Valve::get_H(CharacteristicsType ct, ElementID id) const {
  const auto Qp = calculate_q(id);
  if (ct == CharacteristicsType::CPlus) {
    return m_c_plus.at(id).c - m_c_plus.at(id).b * Qp;
  } else {
    return m_c_minus.at(id).c + m_c_minus.at(id).b * Qp;
  }
}

double Valve::get_Q(CharacteristicsType, ElementID id,
                    const IterateInput) const {
  return calculate_q(id);
}

double Valve::calculate_q(ElementID id) const {
  double Cv{0.0};
  double CvB{0.0};
  double Qp{0.0};
  auto Cp1 = 0.0;
  auto Bp1 = 0.0;
  auto Cm2 = 0.0;
  auto Bm2 = 0.0;

  // We can't guarantee that both endpoints are connected to pipes
  if (m_c_plus.size() > 0) {
    Cp1 = m_c_plus.at(id).c;
    Bp1 = m_c_plus.at(id).b;
  }

  if (m_c_minus.size() > 0) {
    Cm2 = m_c_minus.at(id).c;
    Bm2 = m_c_minus.at(id).b;
  }

  if ((Cp1 - Cm2) >= 0.0) {
    Cv = m_params.cvp * m_state.tau * m_state.tau;
    CvB = Cv * (Bp1 + Bm2);
    Qp = -CvB + std::sqrt(CvB * CvB + 2.0 * Cv * (Cp1 - Cm2));
  } else {
    Cv = m_params.cvm * m_state.tau * m_state.tau;
    CvB = Cv * (Bp1 + Bm2);
    Qp = CvB - std::sqrt(CvB * CvB - 2.0 * Cv * (Cp1 - Cm2));
  }

  return Qp;
}

ElementView Valve::read_view() const {
  return {
      .element_id = m_ID,
      .values =
          {
              ScalarValue{.name = "Tau",
                          .unit = "s",
                          .symbol = "tau",
                          .value = m_state.tau},
          },
  };
}

}  // namespace lvtrans
