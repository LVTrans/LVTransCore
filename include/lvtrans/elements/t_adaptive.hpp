#pragma once
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.16: Lossless junction with up to six inlet and six
// outlet connections. Skeleton: remains abstract until the Element interface
// and solver are implemented.
class TAdaptive : public Element {
 public:
  TAdaptive();
};

}  // namespace lvtrans
