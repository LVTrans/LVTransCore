#pragma once

#include "lvtrans/elements/reservoir.hpp"
namespace lvtrans {
class ConstantLevelLeft : public Reservoir {
 public:
  ConstantLevelLeft(const double elevation) : Reservoir(elevation) {
    m_ports[PortType::Right].emplace(*this, PortType::Right);
  }
  ConstantLevelLeft(const ReservoirParameters& parameters) : Reservoir(parameters) {
    m_ports[PortType::Right].emplace(*this, PortType::Right);
  }
};
}  // namespace lvtrans
