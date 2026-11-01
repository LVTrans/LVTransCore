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

  if (ct == CharacteristicsType::Plus) {
    const auto& characteristics = m_c_plus.at(id);
    return (characteristics.c - Ha) / characteristics.b;
  } else {
    const auto& characteristics = m_c_minus.at(id);
    return (-characteristics.c + Ha) / characteristics.b;
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

  Characteristics cp{}, cm{};
  if (const auto* pipe = left()) {
    cp = m_c_plus.at(pipe->get_ID());
  }

  if (const auto* pipe = right()) {
    cm = m_c_minus.at(pipe->get_ID());
  }

  double Sc = (cp.c / cp.b) + (cm.c / cm.b);
  double Sb = (1.0 / cp.b) + (1.0 / cm.b);
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
