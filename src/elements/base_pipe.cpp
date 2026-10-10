#include <cassert>
#include <cstdlib>
#include <lvtrans/elements/base_pipe.hpp>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

static PipeState initialize_H_and_Q(const InitialPipeValue& H0,
                                    const InitialPipeValue& Q0,
                                    const size_t num_nodes);
BasePipe::~BasePipe() = default;

BasePipe::BasePipe(PipeParameters p, InitialPipeValue H0, InitialPipeValue Q0,
                   double system_dt)
    : m_params{p},
      m_dt{pipe_dt(system_dt)},
      m_nodes_temp{
          calculate_nodes_temp(p.lambda, p.length, system_dt, p.a, p.rho)},
      m_num_segments{calculate_num_segments(m_nodes_temp)},
      m_a{calculate_wave_speed(p.length, m_nodes_temp, system_dt, p.lambda,
                               p.rho)},
      m_dx{calculate_dx(p.length, m_nodes_temp)},
      m_num_nodes{m_num_segments + 1},
      m_stag_nodes{(m_num_nodes + 1) / 2},
      m_L0{0},
      m_L1{m_num_segments},
      m_D_Dh{calculate_pipe_diameter(p.area, p.periphery, p.diameter,
                                     p.dimension)},
      m_areal{calculate_pipe_area(p.area, p.diameter, p.dimension)},
      m_R{calculate_R(p.f, m_dx, m_D_Dh, m_areal)},
      m_B{calculate_B(p.a, m_areal)},
      m_lambda{calculate_lambda(p.lambda, m_dx, p.rho, m_areal)},
      m_state{initialize_H_and_Q(H0, Q0, static_cast<size_t>(m_num_nodes))},
      m_initial_state{m_state} {
  m_ports[PortType::Left].emplace(*this, PortType::Left);
  m_ports[PortType::Right].emplace(*this, PortType::Right);
  std::cout << "BasePipe: m_lambda: " << m_lambda << std::endl;
  std::cout << "lambda in: " << p.lambda << std::endl;

  m_Z.resize(static_cast<size_t>(m_num_nodes));
  double dZ = p.z1 - p.z0 / m_nodes_temp;
  for (size_t i{0}; i < static_cast<size_t>(m_num_nodes); i++) {
    m_Z[i] = dZ * static_cast<double>(i) + p.z0;
  }
}

PipeState initialize_H_and_Q(const InitialPipeValue& H0,
                             const InitialPipeValue& Q0,
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
}  // namespace lvtrans
