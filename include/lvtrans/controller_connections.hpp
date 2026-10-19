#pragma once
// #include "elements/controllable.hpp"
// #include "elements/controller.hpp"
//
namespace lvtrans {

class Controller;
class Controllable;

class ControllerConnections {
 public:
  static bool connect(Controller& controller, Controllable& controllable);
  static void disconnect(Controller& controller, Controllable& controllable);
  static void disconnect_all(Controller& pipe);
  static void disconnect_all(Controllable& element);
};
}  // namespace lvtrans
