#include <cassert>
#include <cstdlib>
#include <format>
#include <lvtrans/elements/base_pipe.hpp>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/non-pipe.hpp"
#include "lvtrans/hydraulic_connections.hpp"

namespace lvtrans {

static PipeState initialize_H_and_Q(const InitialPipeState& H0,
                                    const InitialPipeState& Q0,
                                    const size_t num_nodes);

BasePipe::~BasePipe() { disconnect_all(); }

BasePipe::BasePipe(PipeParameters p, double system_dt)
    : m_params{p},
      m_derived_data{calculate_pipe_data(p, system_dt)},
      m_state{initialize_H_and_Q(p.H0, p.Q0, m_derived_data.grid.num_nodes)},
      m_initial_state{m_state} {
  const auto& grid = m_derived_data.grid;
  m_state.Z.resize(grid.num_nodes);
  double dZ = p.z1 - p.z0 / static_cast<double>(grid.num_segments);
  for (size_t i{0}; i < grid.num_nodes; i++) {
    m_state.Z[i] = dZ * static_cast<double>(i) + p.z0;
  }
}

void BasePipe::update_boundaries() {
  const auto& grid = m_derived_data.grid;
  constexpr size_t L0 = 0;
  const auto L1 = grid.num_segments;

  if (auto* left = left_elem()) {
    m_state.H[L0] = left->get_H(CharacteristicsType::CMinus, get_ID());
    m_state.Q[L0] = left->get_Q(CharacteristicsType::CMinus, get_ID());
  }

  if (auto* right = right_elem()) {
    m_state.H[L1] = right->get_H(CharacteristicsType::CPlus, get_ID());
    m_state.Q[L1] = right->get_Q(CharacteristicsType::CPlus, get_ID());
  }
}

ConnectResult BasePipe::connect_left(NonPipe& element) {
  return HydraulicConnections::connect(*this, PipeEnd::Left, element);
}

ConnectResult BasePipe::connect_right(NonPipe& element) {
  return HydraulicConnections::connect(*this, PipeEnd::Right, element);
}

void BasePipe::disconnect_all() noexcept {
  HydraulicConnections::disconnect_all(*this);
}

void BasePipe::disconnect_left() noexcept {
  HydraulicConnections::disconnect(*this, PipeEnd::Left);
}

void BasePipe::disconnect_right() noexcept {
  HydraulicConnections::disconnect(*this, PipeEnd::Right);
}

ElementView BasePipe::read_view() const {
  VectorValue H = {
      .name = "Head flow",
      .y_unit = "m",
      .values = m_state.H,
  };
  VectorValue Q = {
      .name = "Flow rate",
      .y_unit = "m³/s",
      .values = m_state.Q,
  };
  return {
      .element_id = m_ID,
      .values =
          {
              H,
              Q,
          },
  };
}

PipeState initialize_H_and_Q(const InitialPipeState& H0,
                             const InitialPipeState& Q0,
                             const size_t num_nodes) {
  PipeState state{};

  if (auto* val = std::get_if<double>(&H0)) {
    state.H.assign(num_nodes, *val);
  } else {
    state.H = std::move(std::get<std::vector<double>>(H0));
  }

  if (auto* val = std::get_if<double>(&Q0)) {
    state.Q.assign(num_nodes, *val);
  } else {
    state.Q = std::move(std::get<std::vector<double>>(Q0));
  }

  return state;
}

PipeDerivedData calculate_pipe_data(const PipeParameters& p, double system_dt) {
  const double nodes_temp =
      calculate_nodes_temp(p.lambda, p.length, system_dt, p.a, p.rho);
  PipeDerivedData data{};
  auto& grid = data.grid;
  grid.dt = pipe_dt(system_dt);
  grid.dx = calculate_dx(p.length, nodes_temp);
  grid.wave_speed =
      calculate_wave_speed(p.length, nodes_temp, system_dt, p.lambda, p.rho);
  grid.num_segments =
      static_cast<std::size_t>(calculate_num_segments(nodes_temp));
  grid.num_nodes = grid.num_segments + 1;
  grid.staggered_nodes = (grid.num_nodes + 1) / 2;

  auto& coefficients = data.coefficients;
  coefficients.hydraulic_diameter =
      calculate_pipe_diameter(p.area, p.periphery, p.diameter, p.dimension);
  coefficients.area = calculate_pipe_area(p.area, p.diameter, p.dimension);
  coefficients.R = calculate_R(p.f, grid.dx, coefficients.hydraulic_diameter,
                               coefficients.area);
  coefficients.B = calculate_B(p.a, coefficients.area);
  coefficients.lambda =
      calculate_lambda(p.lambda, grid.dx, p.rho, coefficients.area);
  return data;
}
}  // namespace lvtrans
