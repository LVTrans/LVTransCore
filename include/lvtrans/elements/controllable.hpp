#pragma once

// #include "lvtrans/elements/controller.hpp"
#include "lvtrans/elements/element.hpp"
namespace lvtrans {

class Controller;
class Controllable {
 public:
  virtual Controller* get_controller() const = 0;
  virtual void set_controller(Controller*) = 0;
  virtual ~Controllable() = default;
};
}  // namespace lvtrans
