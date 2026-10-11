#include <cassert>
#include <cstdlib>
#include <lvtrans/elements/base_pipe.hpp>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

static PipeState initialize_H_and_Q(const InitialPipeState& H0,
                                    const InitialPipeState& Q0,
                                    const size_t num_nodes);

BasePipe::~BasePipe() = default;

BasePipe::BasePipe(PipeParameters p, double system_dt)
    : m_params{p},
      m_derived_data{calculate_pipe_data(p, system_dt)},
      m_state{initialize_H_and_Q(p.H0, p.Q0, m_derived_data.grid.num_nodes)},
      m_initial_state{m_state} {
  m_ports[PortType::Left].emplace(*this, PortType::Left);
  m_ports[PortType::Right].emplace(*this, PortType::Right);

  const auto& grid = m_derived_data.grid;
  m_state.Z.resize(grid.num_nodes);
  double dZ = p.z1 - p.z0 / static_cast<double>(grid.num_segments);
  for (size_t i{0}; i < grid.num_nodes; i++) {
    m_state.Z[i] = dZ * static_cast<double>(i) + p.z0;
  }
}

void BasePipe::remove_left() {
  if (m_left_elem) {
    const auto peer_port_type =
        m_ports[PortType::Left]->get_connected_to()->get_type();
    m_left_elem->set_port(peer_port_type, nullptr);
  }
  m_ports[PortType::Left]->reset();
  m_left_elem = nullptr;
}

void BasePipe::remove_right() {
  if (m_right_elem) {
    const auto peer_port_type =
        m_ports[PortType::Right]->get_connected_to()->get_type();
    m_right_elem->set_port(peer_port_type, nullptr);
  }
  m_ports[PortType::Right]->reset();
  m_right_elem = nullptr;
}

void BasePipe::on_connection_changed(PortType port, Element* peer) {
  auto* neighbor = dynamic_cast<NonPipe*>(peer);
  if (port == PortType::Left) {
    m_left_elem = neighbor;
  } else if (port == PortType::Right) {
    m_right_elem = neighbor;
  }
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
