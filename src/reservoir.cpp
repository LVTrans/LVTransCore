#include "lvtrans/reservoir.hpp"
#include <memory>
#include "lvtrans/element.hpp"

namespace lvtrans {

Reservoir::Reservoir(const double elevation) : m_elevation(elevation) {
  m_ports.resize(PortRight + 1);
  m_ports[PortRight] = std::make_unique<Port>(*this);
}
Reservoir::~Reservoir() {}

}  // namespace lvtrans
