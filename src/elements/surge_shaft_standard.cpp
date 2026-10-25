#include "lvtrans/elements/surge_shaft_standard.hpp"
#include <stdexcept>
#include "lvtrans/const.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

SurgeShaftStandard::SurgeShaftStandard(
    const SurgeShaftStandardParameters& parameters, double dt)
    : NonPipe(1, 1), m_params{parameters}, m_state{}, m_dt{dt} {}

void SurgeShaftStandard::iterate(const double) {  // calulate new flow
  //   double dFa, Fa, dQ, Cv, Q{}, L{}, dL{}, Hb, dHa, dHb;
  //   double g = 9.82;
  //   double eps = 0.000001;
  //   double epsL = 0.01;
  //   int N = 0;
  //   double L0{}, L00{}, Q0{};
  //   auto& p = m_params;
  //
  //   auto Cp1 = 0.0;
  //   auto Bp1 = 0.0;
  //   auto Cm2 = 0.0;
  //   auto Bm2 = 0.0;
  //
  //   if (const auto* pipe = left_links()[0].pipe) {
  //     const auto& characteristic = m_c_plus.at(pipe->get_ID());
  //     Cp1 = characteristic.c;
  //     Bp1 = characteristic.b;
  //   }
  //
  //   if (const auto* pipe = right_links()[0].pipe) {
  //     const auto& characteristic = m_c_minus.at(pipe->get_ID());
  //     Cm2 = characteristic.c;
  //     Bm2 = characteristic.b;
  //   }
  //
  //   double Sc = (Cp1 / Bp1) + (Cm2 / Bm2);
  //   double Sb = (1.0 / Bp1) + (1.0 / Bm2);
  //   double Ca = Sc / Sb;
  //   double Ba = 1.0 / Sb;
  //
  //   L = 2.0 * L0 - L00;
  //   double Ha = Ca - Ba * m_state.Q;
  //
  //   double Z0{};
  //   double Hb0{};
  //
  //   do {
  //     m_state.Q = (2 * p.area * (L - L00)) / m_dt - Q0;
  //
  //     if (L > epsL) {
  //       auto term1 = (L + L0) * (Q - Q0) / (p.area * consts::g * m_dt);
  //       auto term2 = (L * Q * std::abs(Q) + L0 * Q0 * std::abs(Q0));
  //       auto term3 = (2 * consts::g * p.diameter * p.area * p.area);
  //
  //       Hb = term1 + L + L0 + 2.0 * Z0 + p.f * term2 / term3 - Hb0;
  //     } else {
  //       Hb = L + Z0;
  //     }
  //
  //     Ha = Hb + Q * std::abs(Q) * 0.5;  // head loss
  //     L += dL;
  //     N++;
  //   } while (std::fabs(dL) > eps && N < 100);
}

std::optional<ElementState> SurgeShaftStandard::get_state() const {
  return std::nullopt;
}

ElementView SurgeShaftStandard::read_view() const {
  return ElementView{.element_id = get_ID(), .values = {}};
}

}  // namespace lvtrans
