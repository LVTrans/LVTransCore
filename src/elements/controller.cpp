#include "lvtrans/elements/controller.hpp"
#include "lvtrans/controller_connections.hpp"
namespace lvtrans {

void Controller::disconnect_all() {
  ControllerConnections::disconnect_all(*this);
}
}  // namespace lvtrans
