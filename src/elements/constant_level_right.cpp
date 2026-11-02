#include "lvtrans/elements/constant_level_right.hpp"
#include <cmath>
#include "lvtrans/elements/base_pipe.hpp"

namespace lvtrans {

ConstantLevelRight::ConstantLevelRight(const double elevation)
    : Reservoir(elevation, 1, 0) {}
ConstantLevelRight::ConstantLevelRight(const ReservoirParameters& parameters)
    : Reservoir(parameters, 1, 0) {}

void ConstantLevelRight::iterate(double) { calculate_q(); }

double ConstantLevelRight::get_H(CharacteristicsType, ElementID id) {
  const auto& c = m_c_plus.at(id);
  const auto q = calculate_q();

  return c.c - c.b * q;
}

double ConstantLevelRight::get_Q(CharacteristicsType, ElementID,
                                 const IterateInput) {
  return m_state.Q;
}

double ConstantLevelRight::calculate_q() {
  double CvB{0.0};
  double Qp{0.0};

  const auto& H0 = m_params.H0;
  const auto& cvp = m_params.cvp;
  const auto& cvm = m_params.cvm;

  const auto& characteristic = m_c_plus.at(left()->get_ID());
  const auto& cp = characteristic.c;
  const auto& bp = characteristic.b;

  if ((cp - H0) >= 0.0) {
    CvB = cvp * bp;
    Qp = -CvB + std::sqrt(CvB * CvB + 2.0 * cvp * (cp - H0));
  } else {
    CvB = cvm * bp;
    Qp = CvB - std::sqrt(CvB * CvB - 2.0 * cvm * (cp - H0));
  }

  m_state.Q = Qp;

  return Qp;
}
}  // namespace lvtrans
