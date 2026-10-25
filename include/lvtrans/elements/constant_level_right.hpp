
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

  double get_H(CharacteristicsType, ElementID id) override {
    return m_c_plus.at(id).c - m_c_plus.at(id).b * get_Qp(id);
  }
  double get_Q(CharacteristicsType, ElementID id,
               const IterateInput = {}) override {
    return get_Qp(id);
  }

  double get_Qp(ElementID id) {
    double CvB{0.0};
    double Qp{0.0};

    const auto& characteristic = m_c_plus.at(id);
    const auto& Cp1 = characteristic.c;
    const auto& Bp1 = characteristic.b;
    const auto& H0 = m_params.H0;
    const auto& cvp = m_params.cvp;
    const auto& cvm = m_params.cvm;

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
