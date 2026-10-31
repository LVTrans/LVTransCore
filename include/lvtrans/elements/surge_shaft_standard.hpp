#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

class SurgeShaftStandard : public NonPipe {
 public:
  SurgeShaftStandard(const SurgeShaftStandardParameters& parameters, double dt);
  const SurgeShaftStandardParameters& config() const { return m_params; }
  void iterate(const double t = 0) override;
  ElementType get_type() const override {
    return ElementType::SurgeShaftStandard;
  }
  ElementParameters get_parameters() const override { return m_params; }
  std::optional<ElementState> get_state() const override;
  ElementView read_view() const override;
  ModificationResult modify(const ElementModification&) override {
    return ModificationResult::Ok;
  }
  void reset_state() override { m_state = SurgeShaftState{}; }

  double get_H(CharacteristicsType, ElementID) override;
  double get_Q(CharacteristicsType, ElementID,
               const IterateInput = {}) override;

 private:
  SurgeShaftStandardParameters m_params;
  SurgeShaftState m_state;
  double m_dt{};  // Time step [s]
};

}  // namespace lvtrans
