#pragma once
// #include "lvtrans/elements/controllable.hpp"
#include "lvtrans/controller_connections.hpp"
#include "lvtrans/elements/element.hpp"
namespace lvtrans {
class Controllable;
class Controller : public Element {
 public:
  virtual ~Controller() = default;
  Controllable* get_controllable() const { return m_controllable; }
  void disconnect_all();

 protected:
  Controllable* m_controllable{nullptr};
  friend class ControllerConnections;
};
}  // namespace lvtrans
