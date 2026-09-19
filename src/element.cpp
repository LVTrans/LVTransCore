#include "lvtrans/element.hpp"

namespace lvtrans {

void Element::reset_ports() {
  for (auto& port : m_ports) {
    if (!port) {
      continue;
    }
    if (auto* peer = port->connected_to;
        peer && peer->connected_to == port.get()) {
      peer->reset();
    }
    port->reset();
  }
}

}  // namespace lvtrans
