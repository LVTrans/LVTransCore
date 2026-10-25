#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

struct SurgeShaftState {
  double Q{0.0};  // Flow [m^3/s]
  double H{0.0};  // Head [m from datum]
  double L{0.0};  // Shaft length [m]
};

class SurgeShaftStandard : public NonPipe {
 public:
  SurgeShaftStandard(const SurgeShaftStandardParameters& parameters,
                     double dt = 0.1);
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

 private:
  SurgeShaftStandardParameters m_params;
  SurgeShaftState m_state;
  double m_dt{};  // Time step [s]
};

}  // namespace lvtrans
