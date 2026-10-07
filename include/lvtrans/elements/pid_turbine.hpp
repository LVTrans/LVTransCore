#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.6: Turbine governor for Francis or Pelton; grid and
// island modes. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class PIDTurbine : public Element {
 public:
  explicit PIDTurbine(const PIDTurbineParameters& parameters);
  const PIDTurbineParameters& config() const { return m_params; }

 private:
  PIDTurbineParameters m_params;
};

}  // namespace lvtrans
