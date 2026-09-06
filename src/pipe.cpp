#include "lvtrans/const.hpp"
#include "lvtrans/element.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <lvtrans/pipe.hpp>
#include <typeinfo>

namespace lvtrans {

Pipe::Pipe(PipeConfig conf, InitialValues H0, InitialValues Q0)
    : m_config(conf), m_area(calculate_pipe_area(conf.diameter)),
      m_dx(calculate_dx(conf.length, conf.num_reaches)),
      m_R(calculate_R(conf.f, m_dx, conf.diameter, m_area)),
      m_num_nodes(conf.num_reaches + 1), m_B(calculate_B(conf.a, m_area)) {

  assert(conf.num_reaches >= 1 && "num_reaches must be at least 2");

  if (conf.num_reaches % 2 != 0) {
    ++conf.num_reaches;
  }

  initialize_h_q(H0, Q0);

  std::cout << "R        = " << m_R << " m\n";
  std::cout << "B        = " << m_B << " s/m\n";
  std::cout << "N       = " << conf.num_reaches << "\n";

  for (int i = 0; i < conf.num_reaches; i++) {
    std::cout << "H[" << i << "] = " << m_H[i] << " m\n";
    std::cout << "Q[" << i << "] = " << m_Q[i] << " m^3/s\n";
  }

  m_Z.resize(m_num_nodes);

  double dZ = static_cast<double>(conf.z1 - conf.z0) / conf.num_reaches;
  for (int i = 0; i < m_num_nodes; i++) {
    m_Z[i] = dZ * i + conf.z0;
  }
}

Pipe::~Pipe() {}

void Pipe::initialize_h_q(InitialValues H0, InitialValues Q0) {
  if (auto *val = std::get_if<double>(&H0)) {
    m_H.assign(m_num_nodes, *val);
  } else {
    m_H = std::move(std::get<std::vector<double>>(H0));
  }

  if (auto *val = std::get_if<double>(&Q0)) {
    m_Q.assign(m_num_nodes, *val);
  } else {
    m_Q = std::move(std::get<std::vector<double>>(Q0));
  }
}

void Pipe::iterate(const IterateInput input_, IterateOutput output) {
  int L0 = 0;
  int L1 = m_config.num_reaches;
  IterateInput input{.H = m_H[0], .Q = m_Q[0], .B = m_B, .R = m_R, .t = 0};
  if (m_first_iter_run) {
    if (m_left_elem) {
        m_H[0] = m_left_elem->get_H();
        m_Q[0] = m_left_elem->get_Q();
    }

    if (m_right_elem) {
        m_H[m_config.num_reaches] = m_right_elem->get_H();
        m_Q[m_config.num_reaches] = m_right_elem->get_Q();
    }
  }

  for (int i = 1; i < L1; i += 2) {
    const double Cp = m_H[i - 1] + m_B * m_Q[i - 1];
    const double Cm = m_H[i + 1] - m_B * m_Q[i + 1];
    const double Bp = m_B + m_R * std::abs(m_Q[i - 1]);
    const double Bm = m_B + m_R * std::abs(m_Q[i + 1]);

    m_H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
    m_Q[i] = (m_H[i] - Cm) / Bm;
  }

  for (int i = 2; i < L1; i += 2) {
    const double Cp = m_H[i - 1] + m_B * m_Q[i - 1];
    const double Cm = m_H[i + 1] - m_B * m_Q[i + 1];
    const double Bp = m_B + m_R * std::abs(m_Q[i - 1]);
    const double Bm = m_B + m_R * std::abs(m_Q[i + 1]);

    m_H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
    m_Q[i] = (m_H[i] - Cm) / Bm;
  }

  // C- characteristic
  if (m_left_elem) {
    m_left_elem->set_c_characteristics(m_H[L0 + 1] - m_B * m_Q[L0 + 1]);
    m_left_elem->set_b_characteristics(m_R * std::abs(m_Q[L0 + 1]) + m_B);
  }

  // C+ characteristic
  if (m_right_elem) {
    m_right_elem->set_c_characteristics(m_H[L1 - 1] + m_B * m_Q[L1 - 1]);
    m_right_elem->set_b_characteristics(m_B + m_R * std::abs(m_Q[L1 - 1]));
  }

  if (!m_first_iter_run) {
    m_first_iter_run = true;
  }
}
} // namespace lvtrans
