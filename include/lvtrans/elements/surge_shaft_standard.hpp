#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

class SurgeShaftStandard : public NonPipe {
 public:
  explicit SurgeShaftStandard(const SurgeShaftStandardParameters& parameters);
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
};

}  // namespace lvtrans
