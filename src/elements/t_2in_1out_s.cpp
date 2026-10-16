#include "lvtrans/elements/t_2in_1out_s.hpp"

namespace lvtrans {

T2In1OutS::T2In1OutS(const TConnectionParameters& parameters)
    : NonPipe(2, 1), m_params(parameters) {}

}  // namespace lvtrans
