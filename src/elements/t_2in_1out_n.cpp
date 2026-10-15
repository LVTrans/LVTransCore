#include "lvtrans/elements/t_2in_1out_n.hpp"

namespace lvtrans {

T2In1OutN::T2In1OutN(const TConnectionParameters& parameters)
    : NonPipe(2, 1), m_params(parameters) {}

}  // namespace lvtrans
