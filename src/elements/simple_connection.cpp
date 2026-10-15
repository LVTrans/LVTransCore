#include "lvtrans/elements/simple_connection.hpp"
#include <cmath>
#include "lvtrans/element_types.hpp"

namespace lvtrans {
SimpleConnection::SimpleConnection() : NonPipe(1, 1), m_params{} {}

SimpleConnection::SimpleConnection(const LossCoefficients& loss_coeffs)
    : NonPipe(1, 1), m_params(loss_coeffs) {}

double SimpleConnection::get_H(CharacteristicsType ct, ElementID id) const {
  const auto Qp = calculate_Qp(id);

  if (ct == CharacteristicsType::CPlus) {
    return m_c_plus.at(id).c - m_c_plus.at(id).b * Qp;
  } else {
    return m_c_minus.at(id).c + m_c_minus.at(id).b * Qp;
  }
}

double SimpleConnection::get_Q(CharacteristicsType, ElementID id,
                               const IterateInput) const {
  auto& cp = m_c_plus.at(id);
  auto& cm = m_c_minus.at(id);

  return (cp.c - cm.c) / (cp.b + cm.b);
}

double SimpleConnection::calculate_Qp(ElementID id) const {
  const auto cvm = m_params.cvm;
  const auto cvp = m_params.cvp;

  auto Cp1 = m_c_plus.at(id).c;
  auto Bp1 = m_c_plus.at(id).b;

  auto Cm2 = m_c_minus.at(id).c;
  auto Bm2 = m_c_minus.at(id).b;

  if ((Cp1 - Cm2) >= 0.0) {
    const auto CvB = cvp * (Bp1 + Bm2);
    return -CvB + std::sqrt(CvB * CvB + 2.0 * cvp * (Cp1 - Cm2));
  } else {
    const auto CvB = cvm * (Bp1 + Bm2);
    return CvB - std::sqrt(CvB * CvB - 2.0 * cvm * (Cp1 - Cm2));
  }
}

}  // namespace lvtrans
