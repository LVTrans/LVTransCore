#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.26: Free-surface channel with a right reservoir and
// a left pipe. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class OpenChannelLRNormal : public Element {
 public:
  explicit OpenChannelLRNormal(const OpenChannelParameters& parameters);
  const OpenChannelParameters& config() const { return m_params; }

 private:
  OpenChannelParameters m_params;
};

}  // namespace lvtrans
