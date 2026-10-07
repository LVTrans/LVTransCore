#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.13: Brook intake connected at the left end of one
// pipe. Skeleton: remains abstract until the Element interface and solver are
// implemented.
class CreekShaftNormalQ1 : public Element {
 public:
  explicit CreekShaftNormalQ1(const CreekShaftParameters& parameters);
  const CreekShaftParameters& config() const { return m_params; }

 private:
  CreekShaftParameters m_params;
};

}  // namespace lvtrans
