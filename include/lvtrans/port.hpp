#pragma once

#include <cstdint>
namespace lvtrans {

enum PortType : std::uint8_t { Left, Right, Up, Down };
class Element;

class Port {
 public:
  Port() = delete;
  ~Port() = default;
  Port(Element& comp, PortType type) : m_owner(comp), m_type(type) {}
  PortType get_type() const { return m_type; }

  void connect(Port* other) { m_connected_to = other; }
  void reset() { m_connected_to = nullptr; }
  bool is_connected() const { return m_connected_to != nullptr; }
  bool is_complete() const {
    return m_connected_to && m_connected_to->m_connected_to == this;
  }
  Element& get_owner() const { return m_owner; }
  Port* get_connected_to() const { return m_connected_to; }

 private:
  Element& m_owner;
  Port* m_connected_to{nullptr};
  PortType m_type;
};

}  // namespace lvtrans
