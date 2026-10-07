#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.18: Pelton turbine used with a turbine governor and
// optional sump. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class Pelton : public Element {
 public:
  explicit Pelton(const PeltonParameters& parameters);
  const PeltonParameters& config() const { return m_params; }

 private:
  PeltonParameters m_params;
};

}  // namespace lvtrans
