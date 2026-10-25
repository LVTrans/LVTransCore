#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {
class SimpleConnection : public NonPipe {
 public:
  SimpleConnection();
  explicit SimpleConnection(const LossCoefficients& loss_coeffs);
  double get_H(CharacteristicsType, ElementID) override;
  double get_Q(CharacteristicsType, ElementID,
               const IterateInput = {}) override;

  virtual ElementType get_type() const override {
    return ElementType::SimpleConnection;
  }
  virtual ElementParameters get_parameters() const override { return m_params; }

 private:
  double calculate_Qp(ElementID id) const;
  LossCoefficients m_params;
};
}  // namespace lvtrans
