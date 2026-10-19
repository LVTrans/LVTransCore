#pragma once

// #include "lvtrans/elements/controller.hpp"
#include "lvtrans/elements/element.hpp"
namespace lvtrans {

class Controller;
class Controllable {
 public:
  virtual ~Controllable() = default;
  Controller* get_controller() const { return m_controller; }

 protected:
  Controller* m_controller{nullptr};
  friend class ControllerConnections;
};
}  // namespace lvtrans
