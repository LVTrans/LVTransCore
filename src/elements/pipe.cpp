#include "lvtrans/elements/pipe.hpp"
#include <cassert>
#include <cstdlib>
#include <utility>
#include "lvtrans/element_types.hpp"
namespace lvtrans {

Pipe::Pipe(PipeParameters params, InitialPipeValue H0, InitialPipeValue Q0,
           const double dt)
    : BasePipe(params, std::move(H0), std::move(Q0), dt) {}

void Pipe::iterate(const double) {
  const auto L0 = static_cast<size_t>(m_L0);
  const auto L1 = static_cast<size_t>(m_L1);

  auto& H = m_state.H;
  auto& Q = m_state.Q;

  for (size_t i{1}; i < L1; i += 2) {
    double lambda_current = m_lambda * (Q[i + 1] - Q[i - 1]);
    const double Cp = H[i - 1] + m_B * Q[i - 1] + lambda_current;
    const double Cm = H[i + 1] - m_B * Q[i + 1] + lambda_current;
    const double Bp = m_B + m_R * std::abs(Q[i - 1]);
    const double Bm = m_B + m_R * std::abs(Q[i + 1]);

    H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
    Q[i] = (H[i] - Cm) / Bm;
  }

  for (size_t i{2}; i < L1; i += 2) {
    double lambda_current = m_lambda * (Q[i + 1] - Q[i - 1]);
    const double Cp = H[i - 1] + m_B * Q[i - 1] + lambda_current;
    const double Cm = H[i + 1] - m_B * Q[i + 1] + lambda_current;
    const double Bp = m_B + m_R * std::abs(Q[i - 1]);
    const double Bm = m_B + m_R * std::abs(Q[i + 1]);

    H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
    Q[i] = (H[i] - Cm) / Bm;
  }

  // C- characteristic
  if (m_left_elem) {
    m_left_elem->set_c_characteristics(H[L0 + 1] - m_B * Q[L0 + 1]);
    m_left_elem->set_b_characteristics(m_R * std::abs(Q[L0 + 1]) + m_B);
    H[L0] = m_left_elem->get_H();
    Q[L0] = m_left_elem->get_Q();
  }

  // C+ characteristic
  if (m_right_elem) {
    m_right_elem->set_c_characteristics(H[L1 - 1] + m_B * Q[L1 - 1]);
    m_right_elem->set_b_characteristics(m_B + m_R * std::abs(Q[L1 - 1]));
    H[L1] = m_right_elem->get_H();
    Q[L1] = m_right_elem->get_Q();
  }
}

}  // namespace lvtrans
