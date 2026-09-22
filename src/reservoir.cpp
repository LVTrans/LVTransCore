#include "lvtrans/reservoir.hpp"
#include <memory>
#include "lvtrans/element.hpp"

namespace lvtrans {

Reservoir::Reservoir(const double elevation) {
  m_config.H0 = elevation;
  m_ports.resize(PortType::Right + 1);
  m_ports[PortType::Right] = std::make_unique<Port>(*this);
}

Reservoir::Reservoir(const ReservoirConfig& config) : Reservoir(config.H0) {
  m_config = config;
}

Reservoir::~Reservoir() {}

}  // namespace lvtrans
