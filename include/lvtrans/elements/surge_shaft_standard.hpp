#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.9: Constant-area surge shaft between two pipes.
// Skeleton: remains abstract until the Element interface and solver are
// implemented.
class SurgeShaftStandard : public Element {
 public:
  explicit SurgeShaftStandard(const SurgeShaftStandardParameters& parameters);
  const SurgeShaftStandardParameters& config() const { return m_params; }

 private:
  SurgeShaftStandardParameters m_params;
};

}  // namespace lvtrans
