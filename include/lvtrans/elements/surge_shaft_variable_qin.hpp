#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.11: Variable-area surge shaft with a weir and
// external inflow. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class SurgeShaftVariableQin : public Element {
 public:
  explicit SurgeShaftVariableQin(
      const SurgeShaftVariableQinParameters& parameters);
  const SurgeShaftVariableQinParameters& config() const { return m_params; }

 private:
  SurgeShaftVariableQinParameters m_params;
};

}  // namespace lvtrans
