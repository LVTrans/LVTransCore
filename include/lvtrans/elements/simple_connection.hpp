#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {
class SimpleConnection : public NonPipe {
 public:
  SimpleConnection();
  explicit SimpleConnection(const LossCoefficients& loss_coeffs);
  double get_H(CharacteristicsType, ElementID) override;
  double get_Q(CharacteristicsType, ElementID) override;

  ElementType get_type() const override {
    return ElementType::SimpleConnection;
  }
  ElementParameters get_parameters() const override { return m_params; }
  void reset_state() override {}
  ElementView read_view() const override;

 private:
  double calculate_q();
  SimpleConnectionState m_state;
  LossCoefficients m_params;
};
}  // namespace lvtrans
