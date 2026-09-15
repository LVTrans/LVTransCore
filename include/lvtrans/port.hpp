#pragma once

#include <memory>
namespace lvtrans {

class Element;

class Port {
public:
  Element &owner;
  std::shared_ptr<Port> connected_to;

  Port() = delete;
  explicit Port(Element &comp) : owner(comp) {}
  void connect(std::shared_ptr<Port> other) { connected_to = other; }
};

} // namespace lvtrans
