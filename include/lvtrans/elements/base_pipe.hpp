#pragma once
#include <cassert>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <vector>
#include "lvtrans/const.hpp"
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/non-pipe.hpp"
#include "lvtrans/hydraulic_connections.hpp"
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

struct PipeDiscretization {
  double dt{};
  double dx{};
  double wave_speed{};
  std::size_t num_segments{};
  std::size_t num_nodes{};
  std::size_t staggered_nodes{};
};

struct PipeCoefficients {
  double hydraulic_diameter{};  ///< Hydraulic diameter of the pipe [m] (D_Dh)
  double area{};  ///< Cross-sectional area of the pipe [m^2] (Areal)
  double R{};
  double B{};
  double lambda{};
};

struct PipeDerivedData {
  PipeDiscretization grid;
  PipeCoefficients coefficients;
};

PipeDerivedData calculate_pipe_data(const PipeParameters& parameters,
                                    double system_dt);

class Element;

class BasePipe : public Element {
 public:
  BasePipe() = default;
  BasePipe(PipeParameters params, const double dt);
  virtual ~BasePipe() = 0;
  ElementType get_type() const override { return ElementType::Pipe; }
  virtual ElementParameters get_parameters() const override { return m_params; }
  ElementView read_view() const override;
  virtual void iterate(const double t = 0) override = 0;
  std::optional<ElementState> get_state() const override {
    return std::make_optional(m_state);
  }
  void reset_state() override { m_state = m_initial_state; }
  ConnectResult connect_left(NonPipe& element);
  ConnectResult connect_right(NonPipe& element);

  const PipeParameters& config() const { return m_params; }
  double get_R() const { return m_derived_data.coefficients.R; }
  double get_B() const { return m_derived_data.coefficients.B; }
  const std::vector<double>& get_H() const { return m_state.H; }
  const std::vector<double>& get_Q() const { return m_state.Q; }
  double get_latest_H() const { return m_state.H.back(); }
  double get_latest_Q() const { return m_state.Q.back(); }
  void update_boundaries();

  const BoundaryLink& left_link() const { return m_left; }
  const BoundaryLink& right_link() const { return m_right; }
  NonPipe* left_elem() const { return m_left.element; }
  NonPipe* right_elem() const { return m_right.element; }
  void disconnect_all() noexcept;
  void disconnect_left() noexcept;
  void disconnect_right() noexcept;

 private:
  BoundaryLink m_left{};
  BoundaryLink m_right{};
  friend class HydraulicConnections;

 protected:
  PipeParameters m_params{};

  const PipeDerivedData m_derived_data;
  PipeState m_state{};
  PipeState m_initial_state{};
};
}  // namespace lvtrans
