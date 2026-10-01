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
