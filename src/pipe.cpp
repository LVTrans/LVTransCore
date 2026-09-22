#include <cassert>
#include <cstdlib>
#include <iostream>
#include <lvtrans/pipe.hpp>
#include <memory>
#include "lvtrans/element.hpp"

namespace lvtrans {

Pipe::Pipe(PipeParameters conf, InitialValues H0, InitialValues Q0)
    : m_config(conf),
      m_area(calculate_pipe_area(conf.diameter)),
      m_dx(calculate_dx(conf.length, conf.num_reaches)),
      m_R(calculate_R(conf.f, m_dx, conf.diameter, m_area)),
      m_B(calculate_B(conf.a, m_area)),
      m_num_nodes(conf.num_reaches + 1) {
  assert(conf.num_reaches >= 1 && "num_reaches must be at least 2");

  if (conf.num_reaches % 2 != 0) {
    ++conf.num_reaches;
  }

  m_ports.resize(2);

  m_ports[PortType::Left] = std::make_unique<Port>(*this, PortType::Left);
  m_ports[PortType::Right] = std::make_unique<Port>(*this, PortType::Right);

  initialize_h_q(H0, Q0);

  m_Z.resize(m_num_nodes);

  double dZ = conf.z1 - conf.z0 / static_cast<double>(conf.num_reaches);
  for (size_t i{0}; i < m_num_nodes; i++) {
    m_Z[i] = dZ * static_cast<double>(i) + conf.z0;
  }
}

Pipe::~Pipe() {}

void Pipe::initialize_h_q(InitialValues& H0, InitialValues& Q0) {
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

void Pipe::iterate(const IterateInput, IterateOutput) {
  const size_t L0 = 0;
  const size_t L1 = m_config.num_reaches;
  auto& H = m_state.H;
  auto& Q = m_state.Q;

  for (size_t i{1}; i < L1; i += 2) {
    const double Cp = H[i - 1] + m_B * Q[i - 1];
    const double Cm = H[i + 1] - m_B * Q[i + 1];
    const double Bp = m_B + m_R * std::abs(Q[i - 1]);
    const double Bm = m_B + m_R * std::abs(Q[i + 1]);

    H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
    Q[i] = (H[i] - Cm) / Bm;
  }

  for (size_t i{2}; i < L1; i += 2) {
    const double Cp = H[i - 1] + m_B * Q[i - 1];
    const double Cm = H[i + 1] - m_B * Q[i + 1];
    const double Bp = m_B + m_R * std::abs(Q[i - 1]);
    const double Bm = m_B + m_R * std::abs(Q[i + 1]);

    H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
    Q[i] = (H[i] - Cm) / Bm;
  }

  // C- characteristic
  if (auto* left_elem = this->left_elem()) {
    left_elem->set_c_characteristics(H[L0 + 1] - m_B * Q[L0 + 1]);
    left_elem->set_b_characteristics(m_R * std::abs(Q[L0 + 1]) + m_B);
    H[0] = left_elem->get_H();
    Q[0] = left_elem->get_Q();
  }

  // C+ characteristic
  if (auto* right_elem = this->right_elem()) {
    right_elem->set_c_characteristics(H[L1 - 1] + m_B * Q[L1 - 1]);
    right_elem->set_b_characteristics(m_B + m_R * std::abs(Q[L1 - 1]));
    H[m_config.num_reaches] = right_elem->get_H();
    Q[m_config.num_reaches] = right_elem->get_Q();
  }
}
}  // namespace lvtrans
