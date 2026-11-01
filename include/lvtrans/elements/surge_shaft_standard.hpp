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
  void reset_state() override;

  double get_H(CharacteristicsType, ElementID) override;
  double get_Q(CharacteristicsType, ElementID,
               const IterateInput = {}) override;

 private:
  void initialize_from_pipes();
  SurgeShaftStandardParameters m_params;
  SurgeShaftState m_state;
  double m_dt{};  // Time step [s]
  double m_area{};
  double m_diameter{};
  double m_base_elevation{};
  bool m_initialized{false};
};

}  // namespace lvtrans
