#pragma once

#include <cmath>
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"
#include "lvtrans/elements/reservoir.hpp"
namespace lvtrans {
class ConstantLevelLeft : public Reservoir {
 public:
  ConstantLevelLeft(const double elevation);
  ConstantLevelLeft(const ReservoirParameters& parameters);
  ElementType get_type() const override {
    return ElementType::ConstantLevelLeft;
  }

<<<<<<< Updated upstream
  double get_H(CharacteristicsType, ElementID id) override {
    return m_c_minus.at(id).c + m_c_minus.at(id).b * get_Qp(id);
  }
  double get_Q(CharacteristicsType, ElementID id,
               const IterateInput = {}) override {
    return get_Qp(id);
  }

  double get_Qp(ElementID id) {
    double CvB{0.0};
    double Qp{0.0};

    const auto& characteristic = m_c_minus.at(id);
    const auto& Cm = characteristic.c;
    const auto& Bm = characteristic.b;
    const auto& H0 = m_params.H0;
    const auto& cvp = m_params.cvp;
    const auto& cvm = m_params.cvm;

    if ((H0 - Cm) >= 0.0) {
      CvB = cvp * Bm;
      Qp = -CvB + std::sqrt(CvB * CvB + 2.0 * cvp * (H0 - Cm));
    } else {
      CvB = cvm * Bm;
      Qp = CvB - std::sqrt(CvB * CvB - 2.0 * cvm * (H0 - Cm));
    }
    m_state.Q = Qp;

    return Qp;
  }
=======
  void iterate(double) override;

  double get_H(CharacteristicsType, ElementID id) override;
  double get_Q(CharacteristicsType, ElementID, const IterateInput) override;

 private:
  double calculate_q();
>>>>>>> Stashed changes
};
}  // namespace lvtrans
