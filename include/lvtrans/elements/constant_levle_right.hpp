
#pragma once

#include "lvtrans/elements/reservoir.hpp"
namespace lvtrans {
class ConstantLevelRight : public Reservoir {
 public:
  ConstantLevelRight(const double elevation) : Reservoir(elevation) {
    m_ports[PortType::Left].emplace(*this, PortType::Left);
  }
  ConstantLevelRight(const ReservoirParameters& parameters)
      : Reservoir(parameters) {
    m_ports[PortType::Left].emplace(*this, PortType::Left);
  }
};
}  // namespace lvtrans
