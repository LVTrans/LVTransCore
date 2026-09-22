#pragma once

#include <cstdint>
namespace lvtrans {

enum PortType : std::uint8_t { Left, Right, Up, Down };
class Element;

class Port {
 public:
  Element& owner;
  Port* connected_to{nullptr};
  PortType type;

  Port() = delete;
  ~Port() = default;
  Port(Element& comp, PortType type) : owner(comp), type(type) {}
  PortType get_port_type() const { return type; }
  void connect(Port* other) { connected_to = other; }
  void reset() { connected_to = nullptr; }
};

}  // namespace lvtrans
