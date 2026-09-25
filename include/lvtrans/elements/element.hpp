#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include "lvtrans/element_types.hpp"
#include "lvtrans/port.hpp"

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

class Element {
 public:
  virtual ~Element() = default;
  virtual void iterate(const IterateInput = {}, IterateOutput = {}) = 0;
  virtual ElementType get_type() = 0;
  void connect_to(Element* other, PortType from, PortType to) const;
  void reset_ports() const;
  Port* get_first_available_port() const;

  void set_name(const std::string& name) { m_name = name; }
  std::string get_name() const { return m_name; }
  ElementID get_ID() const { return m_ID; }
  void set_ID(ElementID id) { m_ID = id; }
  void set_port(PortType index, Port* port) const {
    m_ports[index]->connect(port);
  }
  // FIXME: can return nullptr ports
  Ports& get_ports() { return m_ports; }
  virtual ElementParameters get_parameters() const = 0;
  virtual std::optional<ElementState> get_state() const = 0;

 protected:
  ElementID m_ID{};
  std::string m_name{};
  Ports m_ports;
};
}  // namespace lvtrans
