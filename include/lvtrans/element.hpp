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

enum PortType : std::uint8_t { Left, Right, Up, Down };

class Element {
 public:
  virtual void iterate(const IterateInput = {}, IterateOutput = {}) = 0;
  virtual ~Element() = default;
  ElementID get_ID() const { return m_ID; }
  void set_ID(ElementID id) { m_ID = id; }
  void reset_ports();

 protected:
  ElementID m_ID{};
  Ports m_ports;
};
}  // namespace lvtrans
