#include "lvtrans/elements/t_1in_2out.hpp"
#include "lvtrans/element_states.hpp"
#include "lvtrans/elements/base_pipe.hpp"

namespace lvtrans {

T1In2Out::T1In2Out(const TConnectionParameters& parameters)
    : NonPipe(1, 2), m_params{parameters}, m_state{} {}

std::optional<ElementState> T1In2Out::get_state() const { return m_state; }

double T1In2Out::get_H(CharacteristicsType ct, ElementID id) {
  calculate_q();
  if (ct == CharacteristicsType::Plus) {
    return m_state.H1;
  } else {
    if (id == right(0)->get_ID()) {
      return m_state.H2;
    } else {
      return m_state.H3;
    }
  }
}

double T1In2Out::get_Q(CharacteristicsType ct, ElementID id, IterateInput) {
  if (ct == CharacteristicsType::Plus) {
    return m_state.Q1;
  } else {
    if (id == right(0)->get_ID()) {
      return m_state.Q2;
    } else {
      return m_state.Q3;
    }
  }
}

void T1In2Out::iterate(const double) { calculate_q(); }

void T1In2Out::calculate_q() {
  double tol = 0.00001;
  double F01, F02, F1, F2, F11, F12, F21, F22;
  double dQ2, dQ3, Fx, eps;
  int N = 0;

  auto& p = m_params;
  auto& s = m_state;

  Characteristics cp{}, cm1{}, cm2{};
  if (const auto* pipe = left()) {
    cp = m_c_plus.at(pipe->get_ID());
  }

  if (const auto* pipe = right()) {
    cm1 = m_c_minus.at(pipe->get_ID());
  }

  if (const auto* pipe = right(1)) {
    cm2 = m_c_minus.at(pipe->get_ID());
  }

  do {
    F01 = cp.c - cp.b * (s.Q2 + s.Q3) -
          0.5 * (s.Q2 + s.Q3) * std::abs(s.Q2 + s.Q3) / p.cv1;
    F02 = -cp.b - std::abs(s.Q2 + s.Q3) / p.cv1;

    F1 = F01 - cm1.c - cm1.b * s.Q2 - 0.5 * s.Q2 * std::abs(s.Q2) / p.cv2;
    F2 = F01 - cm2.c - cm2.b * s.Q3 - 0.5 * s.Q3 * std::abs(s.Q3) / p.cv3;
    F11 = F02 - cm1.b - std::abs(s.Q2) / p.cv2;
    F12 = F02;
    F21 = F02;
    F22 = F02 - cm2.b - std::abs(s.Q3) / p.cv3;

    Fx = 1.0 / (F21 * F12 - F11 * F22);
    dQ2 = Fx * (F1 * F22 - F2 * F12);
    dQ3 = Fx * (F2 * F11 - F1 * F21);

    s.Q2 = s.Q2 + dQ2;
    s.Q3 = s.Q3 + dQ3;
    N++;
    eps = std::abs(dQ2) + std::abs(dQ3);
  } while (eps > tol && N < 100);

  s.Q1 = s.Q2 + s.Q3;
  s.H1 = cp.c - cp.b * s.Q1;
  s.H2 = cm1.c + cm1.b * s.Q2;
  s.H3 = cm2.c + cm2.b * s.Q3;
  s.H4 = s.H1 - s.Q1 * std::abs(s.Q1) / (2.0 * p.cv1);
}

ElementView T1In2Out::read_view() const {
  return ElementView{.element_id = get_ID(),
                     .values = {ScalarValue{
                                    .name = "H1",
                                    .unit = "m",
                                    .value = m_state.H1,
                                },
                                ScalarValue{
                                    .name = "H2",
                                    .unit = "m",
                                    .value = m_state.H2,
                                },
                                ScalarValue{
                                    .name = "H3",
                                    .unit = "m",
                                    .value = m_state.H3,
                                },
                                ScalarValue{
                                    .name = "Q1",
                                    .unit = "m³/s",
                                    .value = m_state.Q1,
                                },
                                ScalarValue{
                                    .name = "Q2",
                                    .unit = "m³/s",
                                    .value = m_state.Q2,
                                },
                                ScalarValue{
                                    .name = "Q3",
                                    .unit = "m³/s",
                                    .value = m_state.Q3,
                                }}};
}
}  // namespace lvtrans
