#pragma once
namespace lvtrans {

class Controller;
class Controllable;

class ControllerConnections {
 public:
  static bool connect(Controller& controller, Controllable& controllable);
  static void disconnect(Controller& controller, Controllable& controllable);
  static void disconnect_all(Controller& controller);
  static void disconnect_all(Controllable& controllable);
};
}  // namespace lvtrans
