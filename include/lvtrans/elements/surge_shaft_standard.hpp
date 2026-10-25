#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

struct SurgeShaftState {
  double Q{};  // Flow [m^3/s]
  double H{};  // Head [m from datum]
  double L{};  // Shaft length [m]
  double Hb{};
  double Cv{};  //
  double H_liquid{};
  double H_liquid_0{};
};

class SurgeShaftStandard : public NonPipe {
 public:
  SurgeShaftStandard(const SurgeShaftStandardParameters& parameters, double dt);
  const SurgeShaftStandardParameters& config() const { return m_params; }
  virtual void iterate(const double t = 0) override;
  virtual ElementType get_type() const override {
    return ElementType::SurgeShaftStandard;
  }
  virtual ElementParameters get_parameters() const override { return m_params; }
  virtual std::optional<ElementState> get_state() const override;
  virtual ElementView read_view() const override;
  virtual ModificationResult modify(const ElementModification&) override {
    return ModificationResult::Ok;
  }

  double get_H(CharacteristicsType, ElementID) override;
  double get_Q(CharacteristicsType, ElementID,
               const IterateInput = {}) override;

 private:
  SurgeShaftStandardParameters m_params;
  SurgeShaftState m_state;
  double m_dt{};  // Time step [s]
};

}  // namespace lvtrans
