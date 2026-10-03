#include <cassert>
#include <cstdlib>
#include <lvtrans/elements/pipe.hpp>
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

Pipe::Pipe(PipeParameters params, InitialPipeValue H0, InitialPipeValue Q0)
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

  initialize_h_q(H0, Q0);

  m_Z.resize(m_num_nodes);

  double dZ = params.z1 - params.z0 / static_cast<double>(params.num_reaches);
  for (size_t i{0}; i < m_num_nodes; i++) {
    m_Z[i] = dZ * static_cast<double>(i) + params.z0;
  }
}

Pipe::~Pipe() {}

void Pipe::initialize_h_q(InitialPipeValue& H0, InitialPipeValue& Q0) {
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
  const size_t L1 = m_params.num_reaches;
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
    H[m_params.num_reaches] = right_elem->get_H();
    Q[m_params.num_reaches] = right_elem->get_Q();
  }
}
}  // namespace lvtrans
