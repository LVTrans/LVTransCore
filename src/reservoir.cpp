#include "lvtrans/reservoir.hpp"
#include <memory>
#include "lvtrans/element.hpp"

namespace lvtrans {

Reservoir::Reservoir(const double elevation) {
  m_config.H0 = elevation;
  m_ports.resize(PortRight + 1);
  m_ports[PortRight] = std::make_unique<Port>(*this);
}

Reservoir::Reservoir(const ReservoirConfig& config) : Reservoir(config.H0) {
  m_config = config;
}

Reservoir::~Reservoir() {}

}  // namespace lvtrans
