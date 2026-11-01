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

  void iterate(double) override {
    if (const auto* pipe = right()) calculate_q(pipe->get_ID());
  }

  double get_H(CharacteristicsType, ElementID id) override {
    const auto& c = m_c_minus.at(id);
    const auto q = calculate_q(id);

    return c.c + c.b * q;
  }
  double get_Q(CharacteristicsType, ElementID, const IterateInput) override {
    return m_state.Q;
  }

  double calculate_q(ElementID id) {
    double CvB{0.0};
    double Qp{0.0};

    const auto& H0 = m_params.H0;
    const auto& cvp = m_params.cvp;
    const auto& cvm = m_params.cvm;

    const auto& characteristic = m_c_minus.at(id);
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
};
}  // namespace lvtrans
