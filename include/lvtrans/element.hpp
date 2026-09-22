#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "lvtrans/port.hpp"

namespace lvtrans {

using Ports = std::vector<std::unique_ptr<Port>>;
using ElementID = int;

struct LossCoefficients {
  double cvp{};  // Loss coefficient with positive flow relative to element
  double cvm{};  // Loss coefficient with negative flow relative to element
};

struct IterateOutput {
  double H{};
  double Q{};
};

struct IterateInput {
  double t{};
  double H{};
  double Q{};
  double B{};
  double R{};
};


class Element {
 public:
  virtual ~Element() = default;
  virtual void iterate(const IterateInput = {}, IterateOutput = {}) = 0;
  void connect_to(Element* other, PortType from, PortType to) const;
  ElementID get_ID() const { return m_ID; }
  void set_ID(ElementID id) { m_ID = id; }
  void reset_ports();
  void set_port(PortType index, Port* port) { m_ports[index]->connect(port); }
  Ports& get_ports() { return m_ports; }
  Port* get_available_port() {
    for (auto& port : m_ports) {
      if (!port->connected_to) {
        return port.get();
      }
    }
    return nullptr;
  }

 protected:
  ElementID m_ID{};
  Ports m_ports;
};
}  // namespace lvtrans
