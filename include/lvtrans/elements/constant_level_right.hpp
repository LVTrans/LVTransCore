
#pragma once

#include <cmath>
#include "lvtrans/elements/reservoir.hpp"
namespace lvtrans {
class ConstantLevelRight : public Reservoir {
 public:
  ConstantLevelRight(const double elevation) : Reservoir(elevation, 1, 0) {}
  ConstantLevelRight(const ReservoirParameters& parameters)
      : Reservoir(parameters, 1, 0) {}
  ElementType get_type() const override {
    return ElementType::ConstantLevelRight;
  }

  double get_H(CharacteristicsType, ElementID id) const override {
    return m_c_plus.at(id).c - m_c_plus.at(id).b * get_Qp(id);
  }
  double get_Q(CharacteristicsType, ElementID id,
               const IterateInput = {}) const override {
    return get_Qp(id);
  }

  double get_Qp(ElementID id) const {
    double CvB{0.0};
    double Qp{0.0};

    auto Cp1 = m_c_plus.at(id).c;
    auto Bp1 = m_c_plus.at(id).b;
    auto H0 = m_params.H0;
    auto cvp = m_params.cvp;
    auto cvm = m_params.cvm;

    if ((Cp1 - H0) >= 0.0) {
      CvB = cvp * Bp1;
      Qp = -CvB + std::sqrt(CvB * CvB + 2.0 * cvp * (Cp1 - H0));
    } else {
      CvB = cvm * Bp1;
      Qp = CvB - std::sqrt(CvB * CvB - 2.0 * cvm * (Cp1 - H0));
    }

    return Qp;
  }
};
}  // namespace lvtrans
