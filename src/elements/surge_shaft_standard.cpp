#include "lvtrans/elements/surge_shaft_standard.hpp"
#include <cmath>
#include <numbers>
#include <stdexcept>
#include "lvtrans/const.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

SurgeShaftStandard::SurgeShaftStandard(
    const SurgeShaftStandardParameters& parameters, double dt)
    : NonPipe(1, 1), m_params{parameters}, m_state{}, m_dt{dt} {
  std::vector<double> left_z{};
  std::vector<double> right_z{};

  // TODO: verify if Z0 is calcualted corectly here
  // if (const auto* pipe = left()) {
  //   left_z = pipe->get_derived_data().Z;
  // }
  // if (const auto* pipe = right()) {
  //   right_z = pipe->get_derived_data().Z;
  // }
  //
  // const auto min = std::min(left_z.size(), right_z.size());
  // for (size_t i = 0; i < min; ++i) {
  //   m_derived.Z0[i] = (left_z[i] + right_z[i]) / 2.0;
  // }
  //
  if (parameters.use_area) {
    m_derived.D_Dh = std::sqrt(4.0 * parameters.area / std::numbers::pi);
    m_derived.area = parameters.area;
  } else {
    m_derived.D_Dh = parameters.diameter;
    m_derived.area =
        std::numbers::pi * parameters.diameter * parameters.diameter / 4.0;
  }
  reset_state();
}

void SurgeShaftStandard::reset_state() {
  m_state = {};
  m_state.H1 = m_state.H2 = m_state.H3 = m_params.H_start;
  m_state.Hb = m_params.H_start;
  m_state.Cv = m_params.cvp;
  // Defer until both pipes exist and have completed their own reset.
  m_initialized = false;
}

void SurgeShaftStandard::initialize_from_pipes() {
  if (m_initialized || !left() || !right()) return;

  // Startup convention: use the mean connected tunnel-floor elevation.
  // Confirm this convention against the LabVIEW core initialization diagram.
  m_base_elevation = (left()->config().z1 + right()->config().z0) / 2.0;
  m_state.H_liquid = m_params.H_start - m_base_elevation;
  m_state.H_liquid_0 = m_state.H_liquid;
  m_state.Q1 = left()->get_Q().back();
  m_state.Q2 = right()->get_Q().front();
  m_state.Q3 = m_state.Q1 - m_state.Q2;
  m_state.Cv = m_state.Q3 >= 0.0 ? m_params.cvp : m_params.cvm;
  m_state.Hb = m_params.H_start;
  m_state.H1 = m_state.H2 = m_state.H3 =
      m_state.Hb + m_state.Q3 * std::abs(m_state.Q3) * 0.5 / m_state.Cv;
  m_initialized = true;
}

double SurgeShaftStandard::get_H(CharacteristicsType, ElementID) {
  initialize_from_pipes();
  return m_state.H3;
}
double SurgeShaftStandard::get_Q(CharacteristicsType ct, ElementID,
                                 const IterateInput) {
  initialize_from_pipes();
  if (ct == CharacteristicsType::Plus) {
    return m_state.Q1;
  } else {
    return m_state.Q2;
  }
}

void SurgeShaftStandard::iterate(const double) {
  initialize_from_pipes();

  double dFa, Fa, dQ{}, Ha{}, Cv, Q{}, L{}, dL{}, Hb{}, dHa{}, dHb{};
  double eps = 0.000001;
  double epsL = 0.01;
  int N = 0;
  double Q0{m_state.Q3};
  double Hb0{m_state.Hb};
  double Z0 = m_base_elevation;
  double L0 = m_state.H_liquid;
  double L00 = m_state.H_liquid_0;
  const auto diameter = m_derived.D_Dh;
  const auto area = m_derived.area;

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

  do {
    Q = (2 * area * (L - L0)) / m_dt - Q0;

    if (Q >= 0.0) {
      Cv = p.cvp;
    } else {
      Cv = p.cvm;
    }

    if (L > epsL) {
      auto term1 = (L + L0) * (Q - Q0) / (area * consts::g * m_dt);
      auto term2 = (L * Q * std::abs(Q) + L0 * Q0 * std::abs(Q0));
      auto term3 = (2 * consts::g * diameter * area * area);

      Hb = term1 + L + L0 + 2.0 * Z0 + p.f * term2 / term3 - Hb0;
    } else {
      Hb = L + Z0;
    }

    Ha = Hb + Q * std::abs(Q) * 0.5 / Cv;
    Fa = Ha - Ca + Ba * Q;
    dQ = 2.0 * area / m_dt;
    if (L > epsL)
      dHb = (Q - Q0 + (L + L0) * dQ) / (area * consts::g * m_dt) + 1.0 +
            p.f * (Q * std::abs(Q) + L * 2.0 * std::abs(Q) * dQ) /
                (2 * consts::g * diameter * area * area);
    else {
      dHb = 1.0;
    }

    dHa = dHb + std::abs(Q) * dQ / Cv;
    dFa = dHa + Ba * dQ;

    dL = -Fa / dFa;

    L += dL;
    N++;
  } while (std::fabs(dL) > eps && N < 100);

  Q = 2.0 * area * (L - L0) / m_dt - Q0;

  if (Q >= 0.0) {
    Cv = p.cvp;
  } else {
    Cv = p.cvm;
  }

  if (L > epsL) {
    Hb = (L + L0) * (Q - Q0) / (area * consts::g * m_dt) + L + L0 + 2.0 * Z0 +
         p.f * (L * Q * std::abs(Q) + L0 * Q0 * std::abs(Q0)) /
             (2 * consts::g * diameter * area * area) -
         Hb0;
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
  m_state.Cv = Cv;

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
                         ScalarValue{.name = "Head", .value = m_state.H3},
                         ScalarValue{.name = "Flow", .value = m_state.Q3},
                     }};
}

}  // namespace lvtrans
