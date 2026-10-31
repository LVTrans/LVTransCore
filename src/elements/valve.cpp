#include "lvtrans/elements/valve.hpp"
#include <cmath>
#include "lvtrans/element_modifications.hpp"
#include "lvtrans/elements/base_pipe.hpp"
namespace lvtrans {

Valve::Valve(const ValveParameters& params)
    : Valve(params, ValveState{params.tau_i}) {}

Valve::Valve(const ValveParameters& conf, ValveState state)
    : NonPipe(1, 1), m_params{conf}, m_state{state} {}

void Valve::disconnect_all() noexcept {
  NonPipe::disconnect_all();
  ControllerConnections::disconnect_all(*this);
}

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

double Valve::get_H(CharacteristicsType ct, ElementID id) {
  const auto Qp = calculate_q();
  if (ct == CharacteristicsType::Plus) {
    return m_c_plus.at(id).c - m_c_plus.at(id).b * Qp;
  } else {
    return m_c_minus.at(id).c + m_c_minus.at(id).b * Qp;
  }
}

double Valve::get_Q(CharacteristicsType, ElementID, const IterateInput) {
  return calculate_q();
}

double Valve::calculate_q() const {
  double Cv{0.0};
  double CvB{0.0};
  double Qp{0.0};
  auto Cp1 = 0.0;
  auto Bp1 = 0.0;
  auto Cm2 = 0.0;
  auto Bm2 = 0.0;

  // We can't guarantee that both endpoints are connected to pipes
  if (const auto* pipe = left_links()[0].pipe) {
    const auto& characteristic = m_c_plus.at(pipe->get_ID());
    Cp1 = characteristic.c;
    Bp1 = characteristic.b;
  }

  if (const auto* pipe = right_links()[0].pipe) {
    const auto& characteristic = m_c_minus.at(pipe->get_ID());
    Cm2 = characteristic.c;
    Bm2 = characteristic.b;
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
