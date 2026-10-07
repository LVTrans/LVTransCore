#include <cassert>
#include <cstdlib>
#include <lvtrans/elements/base_pipe.hpp>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

BasePipe::~BasePipe() = default;

BasePipe::BasePipe(PipeParameters params, InitialPipeValue H0,
                   InitialPipeValue Q0)
    : m_params(params),
      m_area(calculate_pipe_area(params.diameter)),
      m_dx(calculate_dx(params.length, params.num_reaches)),
      m_R(calculate_R(params.f, m_dx, params.diameter, m_area)),
      m_B(calculate_B(params.a, m_area)),
      m_num_nodes(params.num_reaches + 1) {
  assert(params.num_reaches >= 1 && "num_reaches must be at least 2");

  if (params.num_reaches % 2 != 0) {
    ++params.num_reaches;
  }

  m_ports[PortType::Left].emplace(*this, PortType::Left);
  m_ports[PortType::Right].emplace(*this, PortType::Right);

  initialize_H_and_Q(H0, Q0);
  m_initial_state = m_state;

  m_Z.resize(m_num_nodes);

  double dZ = params.z1 - params.z0 / static_cast<double>(params.num_reaches);
  for (size_t i{0}; i < m_num_nodes; i++) {
    m_Z[i] = dZ * static_cast<double>(i) + params.z0;
  }
}

void BasePipe::initialize_H_and_Q(const InitialPipeValue& H0,
                                  const InitialPipeValue& Q0) {
  if (auto* val = std::get_if<double>(&H0)) {
    m_state.H.assign(static_cast<size_t>(m_num_nodes), *val);
  } else {
    m_state.H = std::move(std::get<std::vector<double>>(H0));
  }

  if (auto* val = std::get_if<double>(&Q0)) {
    m_state.Q.assign(static_cast<size_t>(m_num_nodes), *val);
  } else {
    m_state.Q = std::move(std::get<std::vector<double>>(Q0));
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
  ScalarValue H = {
      .name = "Head flow",
      .unit = "m",
      .symbol = "H",
      .value = m_state.H.back(),
  };
  ScalarValue Q = {
      .name = "Flow rate",
      .unit = "m³/s",
      .symbol = "Q",
      .value = m_state.Q.back(),
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
