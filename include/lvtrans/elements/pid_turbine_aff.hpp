#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.8: Turbine governor variant for frequency-response
// (AFF) analysis. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class PIDTurbineAFF : public Element {
 public:
  explicit PIDTurbineAFF(const PIDTurbineParameters& parameters);
  const PIDTurbineParameters& config() const { return m_params; }

 private:
  PIDTurbineParameters m_params;
};

}  // namespace lvtrans
