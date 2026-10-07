#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.22: Pressure release valve controlled by a turbine
// governor. Skeleton: remains abstract until the Element interface and solver
// are implemented.
class PRV : public Element {
 public:
  explicit PRV(const PRVParameters& parameters);
  const PRVParameters& config() const { return m_params; }

 private:
  PRVParameters m_params;
};

}  // namespace lvtrans
