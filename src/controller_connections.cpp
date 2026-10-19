#include "lvtrans/controller_connections.hpp"
#include "lvtrans/elements/controllable.hpp"
#include "lvtrans/elements/controller.hpp"
namespace lvtrans {

bool ControllerConnections::connect(Controller& controller,
                                    Controllable& controllable) {
  if (controller.m_controllable || controllable.m_controller) {
    return false;
  }

  controller.m_controllable = &controllable;
  controllable.m_controller = &controller;

  return true;
}
void ControllerConnections::disconnect(Controller& controller,
                                       Controllable& controllable) {
  controller.m_controllable = nullptr;
  controllable.m_controller = nullptr;
}

void ControllerConnections::disconnect_all(Controller& controller) {
  if (controller.m_controllable) {
    controller.m_controllable->m_controller = nullptr;
    controller.m_controllable = nullptr;
  }
}

void ControllerConnections::disconnect_all(Controllable& controllable) {
  if (controllable.m_controller) {
    controllable.m_controller->m_controllable = nullptr;
    controllable.m_controller = nullptr;
  }
}
}  // namespace lvtrans
