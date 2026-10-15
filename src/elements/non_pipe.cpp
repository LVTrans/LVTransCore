#include "lvtrans/elements/non-pipe.hpp"
namespace lvtrans {
NonPipe::~NonPipe() { disconnect_all(); }

void NonPipe::disconnect_all() noexcept {
  HydraulicConnections::disconnect_all(*this);
}
}  // namespace lvtrans
