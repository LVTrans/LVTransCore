#include "lvtrans/elements/constant_level_left.hpp"

namespace lvtrans {

ConstantLevelLeft::ConstantLevelLeft(const double elevation)
    : Reservoir(elevation, 0, 1) {}
ConstantLevelLeft::ConstantLevelLeft(const ReservoirParameters& parameters)
    : Reservoir(parameters, 0, 1) {}

void ConstantLevelLeft::iterate(double) { calculate_q(); }

double ConstantLevelLeft::get_H(CharacteristicsType, ElementID id) {
  const auto& c = m_c_minus.at(id);
  const auto q = calculate_q();

  return c.c + c.b * q;
}
double ConstantLevelLeft::get_Q(CharacteristicsType, ElementID,
                                const IterateInput) {
  return m_state.Q;
}

double ConstantLevelLeft::calculate_q() {
  double CvB{0.0};
  double Qp{0.0};

  const auto& H0 = m_params.H0;
  const auto& cvp = m_params.cvp;
  const auto& cvm = m_params.cvm;

  const auto& characteristic = m_c_minus.at(right()->get_ID());
  const auto& cm = characteristic.c;
  const auto& bm = characteristic.b;

  if ((H0 - cm) >= 0.0) {
    CvB = cvp * bm;
    Qp = -CvB + std::sqrt(CvB * CvB + 2.0 * cvp * (H0 - cm));
  } else {
    CvB = cvm * bm;
    Qp = CvB - std::sqrt(CvB * CvB - 2.0 * cvm * (H0 - cm));
  }
  m_state.Q = Qp;

  return Qp;
}
}  // namespace lvtrans
