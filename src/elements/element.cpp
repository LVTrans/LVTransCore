#include "lvtrans/elements/element.hpp"
#include <cassert>

namespace lvtrans {

void Element::connect_to(Element* other, PortType from, PortType to) {
  assert(other && "other must not be null");
  assert(from < m_ports.size() && "from must be a valid port index");
  assert(to < other->m_ports.size() && "to must be a valid port index");
  assert(m_ports[from] && other->m_ports[to] && "ports must exist ");

  m_ports[from]->connect(&*other->m_ports[to]);
  other->set_port(to, &*m_ports[from]);
  std::cout << "connected " << get_ID() << " to " << other->get_ID()
            << std::endl;
  // print ports
  std::cout << "from: " << static_cast<int>(from)
            << " to: " << static_cast<int>(to) << std::endl;
}

void Element::connect(Element* other) {
  for (size_t from{0}; from < m_ports.size(); ++from) {
    for (size_t to{0}; to < other->m_ports.size(); ++to) {
      if (m_ports[from] && other->m_ports[to] && m_ports[from]->is_free() &&
          other->m_ports[to]->is_free()) {
        connect_to(other, static_cast<PortType>(from),
                   static_cast<PortType>(to));
      }
    }
  }
}

const Port* Element::get_first_available_port() const {
  for (auto& port : m_ports) {
    if (port && !port->is_connected()) {
      return &*port;
    }
  }
  return nullptr;
}

void Element::reset_ports() {
  for (auto& port : m_ports) {
    if (!port) {
      continue;
    }
    if (auto* peer = port->get_connected_to();
        peer && peer->get_connected_to() == &*port) {
      peer->reset();
    }
    port->reset();
  }
}

}  // namespace lvtrans
