#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.20: Centrifugal pump described by Suter curves,
// used with PID Pump. Skeleton: remains abstract until the Element interface
// and solver are implemented.
class PumpCentrifugal : public Element {
 public:
  explicit PumpCentrifugal(const PumpCentrifugalParameters& parameters);
  const PumpCentrifugalParameters& config() const { return m_params; }

 private:
  PumpCentrifugalParameters m_params;
};

}  // namespace lvtrans
