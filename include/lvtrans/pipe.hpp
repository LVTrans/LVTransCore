#pragma once
#include <iostream>
#include <variant>
#include <vector>
#include "lvtrans/const.hpp"
#include "lvtrans/non-pipe.hpp"
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

struct PipeParameters {
  double length{};
  double diameter{};
  double f{};
  double a{};
  double z0{};
  double z1{};
  double lambda{};
  double f_max{};
  size_t num_reaches{};
  bool use_diameter{};
  bool use_full_moody{};
};

struct PipeState {
  std::vector<double> H{};
  std::vector<double> Q{};
};

class Pipe : public Element {
  using InitialValues = std::variant<double, std::vector<double>>;

 public:
  Pipe(PipeParameters conf, InitialValues H0, InitialValues Q0);
  ~Pipe();
  void iterate(const IterateInput input = {},
               IterateOutput output = {}) override;

  const PipeParameters& config() const { return m_config; }
  double get_R() const { return m_R; }
  double get_B() const { return m_B; }
  const std::vector<double>& get_H() const { return m_state.H; }
  const std::vector<double>& get_Q() const { return m_state.Q; }

  void remove_left() const {
    if (auto* elem = left_elem()) {
      auto their_port_type =
          m_ports[PortType::Left]->m_connected_to->get_port_type();
      elem->set_port(their_port_type, nullptr);
    }
    m_ports[PortType::Left]->reset();
  }

  void remove_right() const {
    if (auto* elem = right_elem()) {
      auto their_port_type =
          m_ports[PortType::Right]->m_connected_to->get_port_type();
      elem->set_port(their_port_type, nullptr);
    }
    m_ports[PortType::Right]->reset();
  }

  NonPipe* left_elem() const {
    if (m_ports[PortType::Left]->m_connected_to) {
      return dynamic_cast<NonPipe*>(
          &m_ports[PortType::Left]->m_connected_to->m_owner);
    }
    return nullptr;
  }

  NonPipe* right_elem() const {
    if (m_ports[PortType::Right]->m_connected_to) {
      return dynamic_cast<NonPipe*>(
          &m_ports[PortType::Right]->m_connected_to->m_owner);
    }
    return nullptr;
  }

 private:
  void initialize_h_q(InitialValues& H0, InitialValues& Q0);

  PipeParameters m_config{};

  const double m_area{};
  const double m_dx{};
  const double m_R{};
  const double m_B{};
  const size_t m_num_nodes{};

  std::vector<double> m_Z{};
  PipeState m_state{};
};
}  // namespace lvtrans
