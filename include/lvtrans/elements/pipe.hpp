#pragma once

#include "lvtrans/elements/base_pipe.hpp"
namespace lvtrans {
class Pipe : public BasePipe {
 public:
  Pipe(PipeParameters params, InitialPipeValue H0, InitialPipeValue Q0);
  ElementType get_type() const override { return ElementType::Pipe; }
  void iterate(const double t = 0) override;
};
}  // namespace lvtrans
