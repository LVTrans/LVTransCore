#pragma once
#include <array>
#include <optional>
#include <string>
#include "lvtrans/element_modifications.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/port.hpp"

namespace lvtrans {

using Ports = std::array<std::optional<Port>, 4>;

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
  Element() = default;
  virtual ~Element() = default;
  virtual void iterate(const double t = 0) = 0;
  virtual ElementType get_type() = 0;
  virtual ElementParameters get_parameters() const = 0;
  virtual std::optional<ElementState> get_state() const = 0;
  virtual ElementView read_view() const = 0;
  virtual void reset_state() = 0;
  virtual ModificationResult modify(const ElementModification&) {
    return ModificationResult::Ok;
  }

  void connect_to(Element* other, PortType from, PortType to);
  void reset_ports();
  const Port* get_first_available_port() const;

  void set_name(const std::string& name) { m_name = name; }
  std::string get_name() const { return m_name; }
  ElementID get_ID() const { return m_ID; }
  void set_ID(ElementID id) { m_ID = id; }
  void set_port(PortType index, Port* port) { m_ports[index]->connect(port); }
  const Ports& get_ports() const { return m_ports; }

 protected:
  ElementID m_ID{};
  std::string m_name{};
  Ports m_ports{};
};
}  // namespace lvtrans
