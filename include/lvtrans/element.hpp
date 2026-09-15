#pragma once
#include "lvtrans/port.hpp"
#include <memory>
#include <vector>

namespace lvtrans {

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

enum PortIndex { PortLeft, PortRight, PortUp, PortDown };
using Ports = std::vector<std::shared_ptr<Port>>;

class Element {
  using ElementID = int;

public:
  virtual void iterate(const IterateInput = {}, IterateOutput = {}) = 0;
  virtual ~Element() = default;
  ElementID get_ID() const { return m_ID; }

protected:
  ElementID m_ID;
  Ports m_ports;
};
} // namespace lvtrans
