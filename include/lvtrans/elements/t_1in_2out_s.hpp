#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.14: One inlet and two outlets; third branch drawn
// south. Skeleton: remains abstract until the Element interface and solver are
// implemented.
class T1In2OutS : public Element {
 public:
  explicit T1In2OutS(const TConnectionParameters& parameters);
  const TConnectionParameters& config() const { return m_params; }

 private:
  TConnectionParameters m_params;
};

}  // namespace lvtrans
