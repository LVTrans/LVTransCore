#include "lvtrans/elements/algebraic_pipe.hpp"
#include <cassert>
namespace lvtrans {
AlgebraicPipe::AlgebraicPipe(PipeParameters params, const double dt)
    : BasePipe(params, dt) {}

void AlgebraicPipe::iterate(const double) {}

}  // namespace lvtrans
