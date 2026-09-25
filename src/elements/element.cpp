#include "lvtrans/elements/element.hpp"
#include <cassert>

namespace lvtrans {

void Element::connect_to(Element* other, PortType from, PortType to) const {
  assert(other && "other must not be null");
  assert(from >= 0 && from < m_ports.size() &&
         "from must be a valid port index");
  assert(to >= 0 && to < other->m_ports.size() &&
         "to must be a valid port index");
  assert(m_ports[from] && other->m_ports[to] && "ports must exist ");

  m_ports[from]->connect(other->m_ports[to].get());
  other->m_ports[to]->connect(m_ports[from].get());
}

Port* Element::get_first_available_port() const {
  for (auto& port : m_ports) {
    if (port && !port->m_connected_to) {
      return port.get();
    }
  }
  return nullptr;
}

void Element::reset_ports() const {
  for (auto& port : m_ports) {
    if (!port) {
      continue;
    }
    if (auto* peer = port->m_connected_to;
        peer && peer->m_connected_to == port.get()) {
      peer->reset();
    }
    port->reset();
  }
}

}  // namespace lvtrans
