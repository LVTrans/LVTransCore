#pragma once
#include "lvtrans/port.hpp"
#include <memory>
#include <vector>

namespace lvtrans {

using Ports = std::vector<std::unique_ptr<Port>>;
using ElementID = int;

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

class Element {

public:
  virtual void iterate(const IterateInput = {}, IterateOutput = {}) = 0;
  virtual ~Element() = default;
  ElementID get_ID() const { return m_ID; }
  void set_ID(ElementID id) { m_ID = id; }

protected:
  ElementID m_ID{};
  Ports m_ports;
};
} // namespace lvtrans
