#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.24: Free-surface channel between two pipes (NL: no
// reservoir level). Skeleton: remains abstract until the Element interface and
// solver are implemented.
class OpenChannelNLNormal : public Element {
 public:
  explicit OpenChannelNLNormal(const OpenChannelParameters& parameters);
  const OpenChannelParameters& config() const { return m_params; }

 private:
  OpenChannelParameters m_params;
};

}  // namespace lvtrans
