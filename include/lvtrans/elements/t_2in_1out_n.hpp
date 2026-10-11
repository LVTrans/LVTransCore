#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.15: Two inlets and one outlet; third branch drawn
// north. Skeleton: remains abstract until the Element interface and solver are
// implemented.
class T2In1OutN : public Element {
 public:
  explicit T2In1OutN(const TConnectionParameters& parameters);
  const TConnectionParameters& config() const { return m_params; }

 private:
  TConnectionParameters m_params;
};

}  // namespace lvtrans
