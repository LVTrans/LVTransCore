#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.17: Francis turbine used with a turbine governor.
// Skeleton: remains abstract until the Element interface and solver are
// implemented.
class Francis : public Element {
 public:
  explicit Francis(const FrancisParameters& parameters);
  const FrancisParameters& config() const { return m_params; }

 private:
  FrancisParameters m_params;
};

}  // namespace lvtrans
