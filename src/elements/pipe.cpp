#include "lvtrans/elements/pipe.hpp"
#include <cassert>
#include <cstdlib>
#include <utility>
#include "lvtrans/element_types.hpp"
namespace lvtrans {

Pipe::Pipe(PipeParameters params, InitialPipeValue H0, InitialPipeValue Q0)
    : BasePipe(params, std::move(H0), std::move(Q0)) {}

void Pipe::iterate(const double) {
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
