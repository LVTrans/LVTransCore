#include "lvtrans/elements/surge_shaft_standard.hpp"
#include <cmath>
#include <numbers>
#include "lvtrans/const.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

SurgeShaftStandard::SurgeShaftStandard(const SurgeShaftStandardParameters& p,
                                       double dt)
    : NonPipe(1, 1), m_params{p}, m_state{}, m_dt{dt} {
  std::vector<double> left_z{};
  std::vector<double> right_z{};

  if (p.use_area) {
    m_derived.D_Dh = std::sqrt(4.0 * p.area / std::numbers::pi);
    m_derived.area = p.area;
  } else {
    m_derived.D_Dh = p.diameter;
    m_derived.area = (std::numbers::pi * (p.diameter * p.diameter)) * 0.25;
  }
  reset_state();
}

SurgeShaftStandard::SurgeShaftStandard(
    const SurgeShaftStandardParameters& parameters, double dt,
    const SurgeShaftState& state)
    : SurgeShaftStandard(parameters, dt) {
  m_state = state;
  m_restored_state = true;
}

void SurgeShaftStandard::reset_state() {
  m_state = {};
  m_state.H1 = m_state.H2 = m_state.H3 = m_params.H_start;
  m_state.Hb = m_params.H_start;
  m_initialized = false;
  m_restored_state = false;
}

void SurgeShaftStandard::initialize_from_pipes() {
  if (m_initialized || !left() || !right()) {
    return;
  }
  const auto& p = m_params;
  auto& s = m_state;

  // TODO: verify if Z0 (elevation) is calcualted corectly here
  m_base_elevation = (left()->parameters().z1 + right()->parameters().z0) / 2.0;
  m_initialized = true;
  if (m_restored_state) { return;
}
  s.Hb = p.H_start;

  s.H_liquid = p.H_start - m_base_elevation;
  s.H_liquid_0 = s.H_liquid;
  s.Q3 = 0;
}

double SurgeShaftStandard::get_H(CharacteristicsType ct, ElementID) {
  initialize_from_pipes();
  if (ct == CharacteristicsType::Plus) {
    return m_state.H1;
  } else {
    return m_state.H2;
  }
}
double SurgeShaftStandard::get_Q(CharacteristicsType ct, ElementID) {
  initialize_from_pipes();
  if (ct == CharacteristicsType::Plus) {
    return m_state.Q1;
  } else {
    return m_state.Q2;
  }
}

void SurgeShaftStandard::iterate(const double) {
  initialize_from_pipes();
  calculate_q();
}

void SurgeShaftStandard::calculate_q() {
  double dFa, Fa, dQ{}, Ha{}, Cv, Q{}, dL{}, Hb{}, dHa{}, dHb{};
  const double eps{0.000001};
  const double epsL{0.01};
  int N = 0;
  const double Q0{m_state.Q3};
  const double Hb0{m_state.Hb};
  const double Z0 = m_base_elevation;
  const double L0 = m_state.H_liquid;
  const double L00 = m_state.H_liquid_0;
  const auto diameter = m_derived.D_Dh;
  const auto area = m_derived.area;

  const auto& p = m_params;

  Characteristics cp{}, cm{};
  if (const auto* pipe = left()) {
    cp = m_c_plus.at(pipe->get_ID());
  }

  if (const auto* pipe = right()) {
    cm = m_c_minus.at(pipe->get_ID());
  }

  const double Sc = cp.c / cp.b + cm.c / cm.b;
  const double Sb = 1.0 / cp.b + 1.0 / cm.b;
  const double Ca = Sc / Sb;
  const double Ba = 1.0 / Sb;
  double L = 2.0 * L0 - L00;

  do {
    Q = 2.0 * area * (L - L0) / m_dt - Q0;

    if (Q >= 0.0) {
      Cv = p.cvp;
    } else {
      Cv = p.cvm;
    }

    if (L > epsL) {
      const auto term1 = (L + L0) * (Q - Q0) / (area * consts::g * m_dt);
      const auto term2 = (L * Q * std::abs(Q) + L0 * Q0 * std::abs(Q0)) /
                         (2 * consts::g * diameter * area * area);
      Hb = term1 + L + L0 + 2.0 * Z0 + p.f * term2 - Hb0;
    } else {
      Hb = L + Z0;
    }

    Ha = Hb + Q * std::abs(Q) * 0.5 / Cv;
    Fa = Ha - Ca + Ba * Q;
    dQ = 2.0 * area / m_dt;
    if (L > epsL) {
      const auto term1 = (Q - Q0 + (L + L0) * dQ) / (area * consts::g * m_dt);
      const auto term2 = p.f * (Q * std::abs(Q) + L * 2.0 * std::abs(Q) * dQ) /
                         (2 * consts::g * diameter * area * area);

      dHb = term1 + 1.0 + term2;
    } else {
      dHb = 1.0;
    }

    dHa = dHb + std::abs(Q) * dQ / Cv;
    dFa = dHa + Ba * dQ;

    dL = -Fa / dFa;

    L += dL;
    N++;
  } while (std::abs(dL) > eps && N < 100);

  Q = 2.0 * area * (L - L0) / m_dt - Q0;

  if (Q >= 0.0) {
    Cv = p.cvp;
  } else {
    Cv = p.cvm;
  }

  if (L > epsL) {
    const auto term1 = (L + L0) * (Q - Q0) / (area * consts::g * m_dt);
    const auto term2 = (L * Q * std::abs(Q) + L0 * Q0 * std::abs(Q0)) /
                       (2 * consts::g * diameter * area * area);
    Hb = term1 + L + L0 + 2.0 * Z0 + p.f * term2 - Hb0;
  } else {
    Hb = L + Z0;
  }

  Ha = Hb + Q * std::abs(Q) * 0.5 / Cv;

  m_state.H1 = Ha;
  m_state.H2 = Ha;
  m_state.H3 = Ha;
  m_state.Q1 = (cp.c - Ha) / cp.b;
  m_state.Q2 = (-cm.c + Ha) / cm.b;
  m_state.Q3 = Q;

  m_state.Hb = Hb;
  m_state.H_liquid = L;
  m_state.H_liquid_0 = L0;
}

std::optional<ElementState> SurgeShaftStandard::get_state() const {
  return m_state;
}

ElementView SurgeShaftStandard::read_view() const {
  return ElementView{.element_id = get_ID(),
                     .values = {ScalarValue{
                                    .name = "Head",
                                    .unit = "m",
                                    .value = m_state.H3,
                                },
                                ScalarValue{
                                    .name = "Q_in",
                                    .unit = "m³/s",
                                    .value = m_state.Q1,
                                },
                                ScalarValue{
                                    .name = "Q_out",
                                    .unit = "m³/s",
                                    .value = m_state.Q2,
                                },
                                ScalarValue{
                                    .name = "Q_up",
                                    .unit = "m³/s",
                                    .value = m_state.Q3,
                                }}};
}

}  // namespace lvtrans
