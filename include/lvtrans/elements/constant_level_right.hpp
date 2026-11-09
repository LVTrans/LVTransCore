#pragma once

#include "lvtrans/elements/reservoir.hpp"
namespace lvtrans {
class ConstantLevelRight : public Reservoir {
 public:
  ConstantLevelRight(const double elevation);
  ConstantLevelRight(const ReservoirParameters& parameters);

  ElementType get_type() const override {
    return ElementType::ConstantLevelRight;
  }

  void iterate(double) override;
  double get_H(CharacteristicsType, ElementID id) override;
  double get_Q(CharacteristicsType, ElementID) override;

 private:
  double calculate_q();
};
}  // namespace lvtrans
