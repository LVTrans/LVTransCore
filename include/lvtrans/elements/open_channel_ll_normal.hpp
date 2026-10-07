#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.25: Free-surface channel with a left reservoir and
// a right pipe. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class OpenChannelLLNormal : public Element {
 public:
  explicit OpenChannelLLNormal(const OpenChannelParameters& parameters);
  const OpenChannelParameters& config() const { return m_params; }

 private:
  OpenChannelParameters m_params;
};

}  // namespace lvtrans
