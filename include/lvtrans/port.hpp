#pragma once

#include <cstdint>
namespace lvtrans {

enum PortType : std::size_t { Left, Right, Up, Down };
class Element;

class Port {
 public:
  Element& m_owner;
  Port* m_connected_to{nullptr};
  PortType m_type;

  Port() = delete;
  ~Port() = default;
  Port(Element& comp, PortType type) : m_owner(comp), m_type(type) {}
  PortType get_port_type() const { return m_type; }
  void connect(Port* other) { m_connected_to = other; }
  void reset() { m_connected_to = nullptr; }
  bool is_connected() const { return m_connected_to != nullptr; }
  bool is_complete() const {
    return m_connected_to != nullptr && m_connected_to->m_connected_to == this;
  }
};

}  // namespace lvtrans
