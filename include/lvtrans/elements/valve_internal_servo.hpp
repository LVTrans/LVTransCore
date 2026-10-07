#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.21: Valve with a first-order servo and optional
// opening/Cv curve. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class ValveInternalServo : public Element {
 public:
  explicit ValveInternalServo(const ValveInternalServoParameters& parameters);
  const ValveInternalServoParameters& config() const { return m_params; }

 private:
  ValveInternalServoParameters m_params;
};

}  // namespace lvtrans
