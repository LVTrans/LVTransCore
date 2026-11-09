#pragma once

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

  void iterate(double) override;

  double get_H(CharacteristicsType, ElementID id) override;
  double get_Q(CharacteristicsType, ElementID) override;

 private:
  double calculate_q();
};
}  // namespace lvtrans
