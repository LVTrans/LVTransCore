#include "lvtrans/elements/simple_connection.hpp"
#include <cmath>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {
SimpleConnection::SimpleConnection() : NonPipe(1, 1), m_params{} {}

SimpleConnection::SimpleConnection(const LossCoefficients& loss_coeffs)
    : NonPipe(1, 1), m_params(loss_coeffs) {}

double SimpleConnection::get_H(CharacteristicsType ct, ElementID id) {
  const auto Qp = calculate_Qp();

  if (ct == CharacteristicsType::Plus) {
    const auto& characteristic = m_c_plus.at(id);
    auto cp = characteristic.c;
    auto bp = characteristic.b;
    return cp - bp * Qp;
  } else {
    const auto& characteristic = m_c_minus.at(id);
    auto cm = characteristic.c;
    auto bm = characteristic.b;
    return cm + bm * Qp;
  }
}

double SimpleConnection::get_Q(CharacteristicsType, ElementID, IterateInput) {
  return calculate_Qp();
}

double SimpleConnection::calculate_Qp() const {
  const auto cvm = m_params.cvm;
  const auto cvp = m_params.cvp;
  auto cp = 0.0;
  auto bp = 0.0;
  auto cm = 0.0;
  auto bm = 0.0;

  if (const auto* pipe = left()) {
    const auto& characteristic = m_c_plus.at(pipe->get_ID());
    cp = characteristic.c;
    bp = characteristic.b;
  }

  if (const auto* pipe = right()) {
    const auto& characteristic = m_c_minus.at(pipe->get_ID());
    cm = characteristic.c;
    bm = characteristic.b;
  }

  if ((cp - cm) >= 0.0) {
    const auto CvB = cvp * (bp + bm);
    return -CvB + std::sqrt(CvB * CvB + 2.0 * cvp * (cp - cm));
  } else {
    const auto CvB = cvm * (bp + bm);
    return CvB - std::sqrt(CvB * CvB - 2.0 * cvm * (cp - cm));
  }
}

ElementView SimpleConnection::read_view() const { return {}; }

}  // namespace lvtrans
