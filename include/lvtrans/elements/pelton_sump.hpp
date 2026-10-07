#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.19: Tailrace sump collecting flow from one to six
// Pelton turbines. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class PeltonSump : public Element {
 public:
  explicit PeltonSump(const PeltonSumpParameters& parameters);
  const PeltonSumpParameters& config() const { return m_params; }

 private:
  PeltonSumpParameters m_params;
};

}  // namespace lvtrans
