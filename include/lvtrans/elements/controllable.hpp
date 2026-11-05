#pragma once

#include "lvtrans/elements/controller.hpp"
namespace lvtrans {

class Controller;
class Controllable {
 public:
  virtual ~Controllable() = default;
  Controller* get_controller() const { return m_controller; }
  virtual double get_measurement() const = 0;

 protected:
  Controller* m_controller{nullptr};
  friend class ControllerConnections;
};
}  // namespace lvtrans
