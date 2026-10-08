#include "lvtrans/elements/algebraic_pipe.hpp"
#include <cassert>
namespace lvtrans {
AlgebraicPipe::AlgebraicPipe(PipeParameters params, InitialPipeValue H0,
                             InitialPipeValue Q0, const double dt)
    : BasePipe(params, H0, Q0, dt) {}

void AlgebraicPipe::iterate(const double) {}

}  // namespace lvtrans
