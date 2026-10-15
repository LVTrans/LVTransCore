#include "lvtrans/elements/simple_connection.hpp"
#include <cmath>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/non-pipe.hpp"

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

double SimpleConnection::get_Q(CharacteristicsType, ElementID,
                               const IterateInput) const {
  Characteristics cp{};
  Characteristics cm{};

  if (const auto* pipe = left_links()[0].pipe) {
    cp = m_c_plus.at(pipe->get_ID());
  }

  if (const auto* pipe = right_links()[0].pipe) {
    cm = m_c_minus.at(pipe->get_ID());
  }

  return (cp.c - cm.c) / (cp.b + cm.b);
}

double SimpleConnection::calculate_Qp(ElementID) const {

  const auto cvm = m_params.cvm;
  const auto cvp = m_params.cvp;
  auto Cp1 = 0.0;
  auto Bp1 = 0.0;
  auto Cm2 = 0.0;
  auto Bm2 = 0.0;

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
    const auto CvB = cvp * (Bp1 + Bm2);
    return -CvB + std::sqrt(CvB * CvB + 2.0 * cvp * (Cp1 - Cm2));
  } else {
    const auto CvB = cvm * (Bp1 + Bm2);
    return CvB - std::sqrt(CvB * CvB - 2.0 * cvm * (Cp1 - Cm2));
  }
}

}  // namespace lvtrans
