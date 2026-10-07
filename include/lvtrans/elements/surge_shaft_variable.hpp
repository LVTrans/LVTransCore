#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.10: Variable-area surge shaft with a weir between
// two pipes. Skeleton: remains abstract until the Element interface and solver
// are implemented.
class SurgeShaftVariable : public Element {
 public:
  explicit SurgeShaftVariable(const SurgeShaftVariableParameters& parameters);
  const SurgeShaftVariableParameters& config() const { return m_params; }

 private:
  SurgeShaftVariableParameters m_params;
};

}  // namespace lvtrans
