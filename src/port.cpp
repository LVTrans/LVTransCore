#include "lvtrans/port.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

void Port::connect(Port* other) {
  m_connected_to = other;
  m_owner.on_connection_changed(m_type, other ? &other->get_owner() : nullptr);
}

}  // namespace lvtrans
