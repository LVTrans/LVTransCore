#pragma once
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"

// Simple Connection is used to:
// • Connect two pipes with different specification
// • Define singular losses
// • Divide the pipe/tunnel better to fit the terrain for instance
namespace lvtrans {
class SimpleConnection : public Element {
 public:
  SimpleConnection(const LossCoefficients& loss_coeffs)
      : loss_coeffs_(loss_coeffs) {}

 private:
  LossCoefficients loss_coeffs_;
};
}  // namespace lvtrans
