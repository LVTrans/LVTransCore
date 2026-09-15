#include "lvtrans/reservoir.hpp"
#include "lvtrans/element.hpp"
#include <memory>

namespace lvtrans {

Reservoir::Reservoir(const double elevation) : m_elevation(elevation) {
  auto port = std::make_shared<Port>(*this);
  m_ports.resize(PortRight + 1);
  m_ports[PortRight] = port;
}
Reservoir::~Reservoir() {}

} // namespace lvtrans
