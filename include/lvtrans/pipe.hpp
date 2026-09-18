#pragma once
#include "lvtrans/const.hpp"
#include "lvtrans/non-pipe.hpp"
#include <variant>
#include <vector>
namespace lvtrans {

inline constexpr double calculate_R(double f, double dx, double diameter,
                                    double area) {
  return f * dx / (2.0 * consts::g * diameter * area * area);
}
inline constexpr double calculate_dx(double pipe_length, size_t num_reaches) {
  return pipe_length / static_cast<double>(num_reaches);
}

inline constexpr double calculate_pipe_area(double diameter) {
  return consts::pi * diameter * diameter / 4.0;
}

inline constexpr double calculate_B(double a, double area) {
  return a / (consts::g * area);
}

struct PipeConfig {
  double length{};
  double diameter{};
  double f{};
  double a{};
  size_t num_reaches{};
  double z0{};
  double z1{};
};

class Pipe : public Element {
  using InitialValues = std::variant<double, std::vector<double>>;

public:
  Pipe(PipeConfig conf, InitialValues H0, InitialValues Q0);
  ~Pipe();
  void iterate(const IterateInput input = {},
               IterateOutput output = {}) override;

  const PipeConfig &config() const { return m_config; }
  double get_R() const { return m_R; }
  double get_B() const { return m_B; }
  const std::vector<double> &get_H() const { return m_H; }
  const std::vector<double> &get_Q() const { return m_Q; }

  void connect_left(NonPipe &elem) {
    m_ports[PortLeft]->connect(
        elem.get_ports()[PortRight].get());            // connect ours
    elem.set_port(PortRight, m_ports[PortLeft].get()); // connect theirs
  }
  void connect_right(NonPipe &elem) {
    m_ports[PortRight]->connect(elem.get_ports()[PortLeft].get());
    elem.set_port(PortLeft, m_ports[PortRight].get());
  }

  void remove_left() {
    if (auto *elem = left_elem()) {
      elem->set_port(PortRight, nullptr); // reset theirs
    }
    m_ports[PortLeft]->connected_to = nullptr; // reset ours
  }
  void remove_right() {
    if (auto *elem = right_elem()) {
      elem->set_port(PortLeft, nullptr);
    }
    m_ports[PortRight]->connected_to = nullptr;
  }

  NonPipe *left_elem() {
    if (m_ports[PortLeft]->connected_to) {
      return dynamic_cast<NonPipe *>(&m_ports[PortLeft]->connected_to->owner);
    }
    return nullptr;
  }

  NonPipe *right_elem() {
    if (m_ports[PortRight]->connected_to) {
      return dynamic_cast<NonPipe *>(&m_ports[PortRight]->connected_to->owner);
    }
    return nullptr;
  }

private:
  void initialize_h_q(InitialValues H0, InitialValues Q0);

  PipeConfig m_config{};
  const double m_area{};
  const double m_dx{};
  const double m_R{};
  const double m_B{};
  const size_t m_num_nodes{};

  std::vector<double> m_H{};
  std::vector<double> m_Q{};
  std::vector<double> m_Z{};
};
} // namespace lvtrans
