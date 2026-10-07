#pragma once
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"

// Screen is used to:
// •
namespace lvtrans {
class Screen : public Element {
 public:
  Screen(const LossCoefficients& loss_coeffs) : loss_coeffs_(loss_coeffs) {}

 private:
  LossCoefficients loss_coeffs_;
};
}  // namespace lvtrans
