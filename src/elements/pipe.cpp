#include "lvtrans/elements/pipe.hpp"
#include <cassert>
#include <cstdlib>
#include "lvtrans/element_types.hpp"
namespace lvtrans {

Pipe::Pipe(PipeParameters params, const double dt) : BasePipe(params, dt) {}

void Pipe::iterate(const double) {
  const auto& grid = m_derived_data.grid;
  const auto& coefficients = m_derived_data.coefficients;
  constexpr size_t L0 = 0;
  const auto L1 = grid.num_segments;

  auto& H = m_state.H;
  auto& Q = m_state.Q;

  for (size_t i{1}; i < L1; i += 2) {
    double lambda_current = coefficients.lambda * (Q[i + 1] - Q[i - 1]);
    const double Cp = H[i - 1] + coefficients.B * Q[i - 1] + lambda_current;
    const double Cm = H[i + 1] - coefficients.B * Q[i + 1] + lambda_current;
    const double Bp = coefficients.B + coefficients.R * std::abs(Q[i - 1]);
    const double Bm = coefficients.B + coefficients.R * std::abs(Q[i + 1]);

    H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
    Q[i] = (H[i] - Cm) / Bm;
  }

  for (size_t i{2}; i < L1; i += 2) {
    double lambda_current = coefficients.lambda * (Q[i + 1] - Q[i - 1]);
    const double Cp = H[i - 1] + coefficients.B * Q[i - 1] + lambda_current;
    const double Cm = H[i + 1] - coefficients.B * Q[i + 1] + lambda_current;
    const double Bp = coefficients.B + coefficients.R * std::abs(Q[i - 1]);
    const double Bm = coefficients.B + coefficients.R * std::abs(Q[i + 1]);

    H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
    Q[i] = (H[i] - Cm) / Bm;
  }

  // C- characteristic
  if (m_left_elem) {
    m_left_elem->set_c_characteristics(H[L0 + 1] - coefficients.B * Q[L0 + 1]);
    m_left_elem->set_b_characteristics(coefficients.R * std::abs(Q[L0 + 1]) +
                                       coefficients.B);
    H[L0] = m_left_elem->get_H();
    Q[L0] = m_left_elem->get_Q();
  }

  // C+ characteristic
  if (m_right_elem) {
    m_right_elem->set_c_characteristics(H[L1 - 1] + coefficients.B * Q[L1 - 1]);
    m_right_elem->set_b_characteristics(coefficients.B +
                                        coefficients.R * std::abs(Q[L1 - 1]));
    H[L1] = m_right_elem->get_H();
    Q[L1] = m_right_elem->get_Q();
  }
}

}  // namespace lvtrans
