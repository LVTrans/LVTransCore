#include "lvtrans/element.hpp"

namespace lvtrans {

void Element::connect_to(Element* other, PortType from, PortType to) const {
  if (!other || from >= m_ports.size() || to >= other->m_ports.size()) {
    return;
  }
  m_ports[from]->connect(other->m_ports[to].get());
  other->m_ports[to]->connect(m_ports[from].get());
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
