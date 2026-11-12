#include "lvtrans/elements/pipe.hpp"
#include <cassert>
#include <cstdlib>
#include "lvtrans/element_types.hpp"
namespace lvtrans {

Pipe::Pipe(const double dt) : BasePipe(PipeParameters{}, dt) {}

Pipe::Pipe(const PipeParameters& params, const double dt) : BasePipe(params, dt) {}

void Pipe::iterate(const double) {
  const auto& grid = m_derived.grid;
  const auto& coefficients = m_derived.coefficients;
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
  update_characteristics();
}

}  // namespace lvtrans
