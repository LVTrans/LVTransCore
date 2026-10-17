#pragma once
// #include "lvtrans/elements/controllable.hpp"
#include "lvtrans/elements/element.hpp"
namespace lvtrans {
class Controllable;
class Controller : public Element {
 public:
  virtual void disconnect_all() = 0;
  virtual Controllable* get_controllable() const = 0;
  virtual void set_controllable(Controllable*) = 0;
  virtual ~Controller() = default;
};
}  // namespace lvtrans
