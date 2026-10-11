#pragma once
#include <cassert>
#include <cmath>
#include <numbers>
#include <variant>
#include <vector>
#include "lvtrans/const.hpp"
#include "lvtrans/elements/non-pipe.hpp"
namespace lvtrans {
inline constexpr double pipe_dt(const double system_dt) {
  return 0.5 * system_dt;
}

inline constexpr long calculate_num_segments(const double nodes) {
  const long n_temp = static_cast<long>(nodes);

  if ((n_temp % 2) != 0) {
    return n_temp + 1;
  }

  return n_temp;
}

inline constexpr double calculate_nodes_temp(const double lambda,
                                             const double length,
                                             const double system_dt,
                                             const double a, const double rho) {
  const double dt = pipe_dt(system_dt);
  double temp{};

  if (lambda == 0.0) {
    temp = std::max(2.0, std::floor(length / (dt * a) + 0.5));
  } else {
    double a_eff = std::sqrt(a * a + lambda / (rho * dt));
    temp = std::max(2.0, std::floor(length / (dt * a_eff)));
  }

  return static_cast<double>(calculate_num_segments(temp));
}

inline constexpr double calculate_R(double f, double dx, double diameter,
                                    double area) {
  return f * dx / (2.0 * consts::g * diameter * area * area);
}

inline constexpr double calculate_wave_speed(const double length,
                                             const double nodes,
                                             const double system_dt,
                                             const double lambda,
                                             const double rho) {
  const double dt = pipe_dt(system_dt);
  if (lambda == 0.0) {
    return length / (dt * nodes);
  } else {
    return std::sqrt(std::pow(length, 2) /
                         (std::pow(nodes, 2) * std::pow(dt, 2)) +
                     lambda / (rho * dt));
  }
}

inline constexpr double calculate_dx(double length, double nodes) {
  return length / nodes;
}

inline constexpr double calculate_pipe_area(double areal_raw,
                                            double diameter_raw,
                                            PipeDimension dimension) {
  if (dimension == PipeDimension::CrossSectional) {
    return areal_raw;
  } else {
    return std::numbers::pi * std::pow(diameter_raw, 2) / 4.0;
  }
}

inline constexpr double calculate_pipe_diameter(double areal_raw,
                                                double perimeter,
                                                double diameter_raw,
                                                PipeDimension dimension) {
  if (dimension == PipeDimension::CrossSectional) {
    return 4.0 * areal_raw / perimeter;
  } else {
    return diameter_raw;
  }
}

inline constexpr double calculate_B(double a, double area) {
  return a / (consts::g * area);
}

inline constexpr double calculate_lambda(double lambda_in, double dx,
                                         double rho_in, double area) {
  return lambda_in / (dx * rho_in * consts::g * area * 2.0);
}

using InitialPipeValue = std::variant<double, std::vector<double>>;
class Element;

class BasePipe : public Element {
 public:
  BasePipe(PipeParameters params, InitialPipeValue H0, InitialPipeValue Q0,
           const double dt);
  virtual ~BasePipe() = 0;
  ElementType get_type() const override { return ElementType::Pipe; }
  ElementParameters get_parameters() const override { return m_params; }
  ElementView read_view() const override;
  virtual void iterate(const double t = 0) override = 0;
  std::optional<ElementState> get_state() const override {
    return std::make_optional(m_state);
  }
  void reset_state() override { m_state = m_initial_state; }

  const PipeParameters& config() const { return m_params; }
  double get_R() const { return m_R; }
  double get_B() const { return m_B; }
  const std::vector<double>& get_H() const { return m_state.H; }
  const std::vector<double>& get_Q() const { return m_state.Q; }
  double get_latest_H() const { return m_state.H.back(); }
  double get_latest_Q() const { return m_state.Q.back(); }

  void remove_left();
  void remove_right();
  NonPipe* left_elem() const noexcept { return m_left_elem; }
  NonPipe* right_elem() const noexcept { return m_right_elem; }
  void on_connection_changed(PortType port, Element* peer) override;

 protected:
  PipeParameters m_params{};
  NonPipe* m_left_elem{nullptr};
  NonPipe* m_right_elem{nullptr};

  const double m_dt{};
  const double m_nodes_temp{};
  const long m_num_segments{};  ///< Number of segments
  const double m_a{};           ///< Calculated wave speed
  const double m_dx{};
  const long m_num_nodes{};
  const long m_stag_nodes{};
  const long m_L0{};       ///< Indices for boundary conditions
  const long m_L1{};       /// Indices for boundary conditions
  double m_D_Dh{};         ///< Hydraulic diameter
  const double m_areal{};  ///< Cross-sectional area

  const double m_R{};
  const double m_B{};
  const double m_lambda{};

  PipeState m_state{};
  PipeState m_initial_state{};
  std::vector<double> m_Z{};
};
}  // namespace lvtrans
