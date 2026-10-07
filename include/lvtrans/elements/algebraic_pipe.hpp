#pragma once

#include "lvtrans/elements/base_pipe.hpp"
namespace lvtrans {

class AlgebraicPipe : public BasePipe {
 public:
  AlgebraicPipe(PipeParameters params, InitialPipeValue H0,
                InitialPipeValue Q0);
  ElementType get_type() override { return ElementType::Pipe; }
  void iterate(const double t) override;
};

}  // namespace lvtrans
