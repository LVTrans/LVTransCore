#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.7: Pump governor with an optional check valve.
// Skeleton: remains abstract until the Element interface and solver are
// implemented.
class PIDPump : public Element {
 public:
  explicit PIDPump(const PIDPumpParameters& parameters);
  const PIDPumpParameters& config() const { return m_params; }

 private:
  PIDPumpParameters m_params;
};

}  // namespace lvtrans
