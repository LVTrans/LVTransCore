#include "lvtrans/elements/algebraic_pipe.hpp"
#include <cassert>
#include <utility>
namespace lvtrans {
AlgebraicPipe::AlgebraicPipe(PipeParameters params, const double dt)
    : BasePipe(std::move(params), dt) {}

void AlgebraicPipe::iterate(const double) {}

}  // namespace lvtrans
