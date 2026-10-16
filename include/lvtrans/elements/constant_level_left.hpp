#pragma once

#include <cmath>
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"
#include "lvtrans/elements/reservoir.hpp"
namespace lvtrans {
class ConstantLevelLeft : public Reservoir {
 public:
  ConstantLevelLeft(const double elevation) : Reservoir(elevation, 0, 1) {}
  ConstantLevelLeft(const ReservoirParameters& parameters)
      : Reservoir(parameters, 0, 1) {}

  ElementType get_type() const override {
    return ElementType::ConstantLevelLeft;
  }

  double get_H(CharacteristicsType, ElementID id) const override {
    return m_c_minus.at(id).c + m_c_minus.at(id).b * get_Qp(id);
  }
  double get_Q(CharacteristicsType, ElementID id,
               const IterateInput = {}) const override {
    return get_Qp(id);
  }

  double get_Qp(ElementID) const {
    double CvB{0.0};
    double Qp{0.0};
    double Cm{0.0};
    double Bm{0.0};

    if (const auto* pipe = right_links()[0].pipe) {
      const auto& characteristic = m_c_minus.at(pipe->get_ID());
      Cm = characteristic.c;
      Bm = characteristic.b;
    }

    auto H0 = m_params.H0;
    auto cvp = m_params.cvp;
    auto cvm = m_params.cvm;

    if ((H0 - Cm) >= 0.0) {
      CvB = cvp * Bm;
      Qp = -CvB + std::sqrt(CvB * CvB + 2.0 * cvp * (H0 - Cm));
    } else {
      CvB = cvm * Bm;
      Qp = CvB - std::sqrt(CvB * CvB - 2.0 * cvm * (H0 - Cm));
    }

    return Qp;
  }
};
}  // namespace lvtrans
