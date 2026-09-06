#pragma once
#include "lvtrans/const.hpp"
#include "lvtrans/element.hpp"
#include "lvtrans/non-pipe.hpp"
#include <variant>
#include <vector>
namespace lvtrans {

inline constexpr double calculate_R(double f, double dx, double diameter,
                                    double area) {
  return f * dx / (2.0 * consts::g * diameter * area * area);
}
inline constexpr double calculate_dx(double pipe_length, int num_reaches) {
  return pipe_length / num_reaches;
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
  int num_reaches{};
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

  void connect_left(std::shared_ptr<NonPipe> elem) {
    m_left_elem = elem;
    elem->set_right(std::make_shared<Pipe>(*this));
  }
  void connect_right(std::shared_ptr<NonPipe> elem) {
    m_right_elem = elem;
    elem->set_left(std::make_shared<Pipe>(*this));
  }

private:
  void initialize_h_q(InitialValues H0, InitialValues Q0);
  PipeConfig m_config{};
  const double m_dx{};
  const double m_area{};
  const double m_R{};
  const double m_B{};
  const int m_num_nodes{};

  bool m_first_iter_run{false};

  std::shared_ptr<NonPipe> m_left_elem;
  std::shared_ptr<NonPipe> m_right_elem;
  std::vector<double> m_H{};
  std::vector<double> m_Q{};
  std::vector<double> m_Z{};
};
} // namespace lvtrans
