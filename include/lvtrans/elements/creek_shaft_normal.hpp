#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.12: Brook intake between two pipes, with global or
// individual inflow. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class CreekShaftNormal : public Element {
 public:
  explicit CreekShaftNormal(const CreekShaftParameters& parameters);
  const CreekShaftParameters& config() const { return m_params; }

 private:
  CreekShaftParameters m_params;
};

}  // namespace lvtrans
