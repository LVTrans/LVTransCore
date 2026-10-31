#include "lvtrans/elements/surge_shaft_standard.hpp"
#include "lvtrans/const.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

SurgeShaftStandard::SurgeShaftStandard(
    const SurgeShaftStandardParameters& parameters, double dt)
    : NonPipe(1, 1), m_params{parameters}, m_state{}, m_dt{dt} {}

double SurgeShaftStandard::get_H(CharacteristicsType, ElementID) {
  return m_state.Hb + m_state.Q * std::abs(m_state.Q) * 0.5 / m_state.Cv;
}
double SurgeShaftStandard::get_Q(CharacteristicsType ct, ElementID id,
                                 const IterateInput) {
  const auto Ha = get_H(ct, id);

  if (ct == CharacteristicsType::CPlus) {
    const auto Cp1 = m_c_plus.at(id).c;
    const auto Bp1 = m_c_plus.at(id).b;
    return (Cp1 - Ha) / Bp1;  // plus
  } else {
    const auto Cm2 = m_c_minus.at(id).c;
    const auto Bm2 = m_c_minus.at(id).b;
    return (-Cm2 + Ha) / Bm2;  // minus
  }
}

void SurgeShaftStandard::iterate(const double) {
  double dFa, Fa, dQ{}, Cv, Q{}, L{}, dL{}, Hb{}, dHa{}, dHb;
  double eps = 0.000001;
  double epsL = 0.01;
  int N = 0;
  double Q0{};
  double Z0 = m_params.initial_level;
  double L0 = m_state.H_liquid;
  double L00 = m_state.H_liquid_0;

  auto& p = m_params;

  auto Cp1 = 0.0;
  auto Bp1 = 0.0;
  auto Cm2 = 0.0;
  auto Bm2 = 0.0;

  if (const auto* pipe = left()) {
    const auto& characteristic = m_c_plus.at(pipe->get_ID());
    Cp1 = characteristic.c;
    Bp1 = characteristic.b;
  }

  if (const auto* pipe = right()) {
    const auto& characteristic = m_c_minus.at(pipe->get_ID());
    Cm2 = characteristic.c;
    Bm2 = characteristic.b;
  }

  double Sc = (Cp1 / Bp1) + (Cm2 / Bm2);
  double Sb = (1.0 / Bp1) + (1.0 / Bm2);
  double Ca = Sc / Sb;
  double Ba = 1.0 / Sb;

  L = 2.0 * L0 - L00;
  double Ha = Ca - Ba * m_state.Q;

  double Hb0{};

  do {
    m_state.Q = (2 * p.area * (L - L00)) / m_dt - Q0;

    if (Q >= 0.0) {
      Cv = p.cvp;
    } else {
      Cv = p.cvm;
    }

    if (L > epsL) {
      auto term1 = (L + L0) * (Q - Q0) / (p.area * consts::g * m_dt);
      auto term2 = (L * Q * std::abs(Q) + L0 * Q0 * std::abs(Q0));
      auto term3 = (2 * consts::g * p.diameter * p.area * p.area);

      Hb = term1 + L + L0 + 2.0 * Z0 + p.f * term2 / term3 - Hb0;
    } else {
      Hb = L + Z0;
    }

    Ha = Hb + Q * std::abs(Q) * 0.5;  // head loss
    Fa = Ha - Ca + Ba * Q;

    if (L > epsL)
      dHb = (Q - Q0 + (L + L0) * dQ) / (p.area * consts::g * m_dt) + 1.0 +
            p.f * (Q * std::abs(Q) + L * 2.0 * std::abs(Q) * dQ) /
                (2 * consts::g * p.diameter * p.area);
    else {
      dHb = 1.0;
    }

    dHa = dHb + std::abs(Q) * dQ / Cv;
    dFa = dHa + Ba * dQ;

    dL = -Fa / dFa;

    L += dL;
    N++;
  } while (std::fabs(dL) > eps && N < 100);

  // HLiqKote = L + Z0;
  // H_Liquid = L;

  Q = 2.0 * p.area * (L - L0) / m_dt - Q0;

  if (Q >= 0.0) {
    Cv = p.cvp;
  } else {
    Cv = p.cvm;
  }

  if (L > epsL) {
    Hb = (L + L0) * (Q - Q0) / (p.area * consts::g * m_dt) + L + L0 + 2.0 * Z0 +
         p.f * (L * Q * std::abs(Q) + L0 * Q0 * std::abs(Q0)) /
             (2 * consts::g * p.diameter * p.area) -
         Hb0;
  } else {
    Hb = L + Z0;
  }

  m_state.H = Ha;
  m_state.Q = Q;

  m_state.Hb = Hb;
  m_state.H_liquid = L;
  m_state.H_liquid_0 = L0;
}

std::optional<ElementState> SurgeShaftStandard::get_state() const {
  return m_state;
}

ElementView SurgeShaftStandard::read_view() const {
  return ElementView{.element_id = get_ID(),
                     .values = {
                         ScalarValue{.name = "H", .value = m_state.H},
                     }};
}

}  // namespace lvtrans
