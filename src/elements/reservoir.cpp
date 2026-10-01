#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

Reservoir::Reservoir(const double elevation) {
  m_params.H0 = elevation;
  m_ports[PortType::Right].emplace(*this, PortType::Right);
}

Reservoir::Reservoir(const ReservoirParameters& params) : Reservoir(params.H0) {
  m_params = params;
}

Reservoir::~Reservoir() {}

}  // namespace lvtrans
