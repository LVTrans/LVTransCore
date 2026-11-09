#include "lvtrans/elements/t_2in_1out.hpp"
#include "lvtrans/element_states.hpp"
#include "lvtrans/elements/base_pipe.hpp"

namespace lvtrans {

T2In1Out::T2In1Out(const TConnectionParameters& parameters)
    : NonPipe(2, 1), m_params{parameters}, m_state{} {}

std::optional<ElementState> T2In1Out::get_state() const { return m_state; }

double T2In1Out::get_H(CharacteristicsType ct, ElementID id) {
  calculate_q();
  if (ct == CharacteristicsType::Minus) {
    return m_state.H2;
  } else {
    if (id == left(0)->get_ID()) {
      return m_state.H1;
    } else {
      return m_state.H3;
    }
  }
}

double T2In1Out::get_Q(CharacteristicsType ct, ElementID id, IterateInput) {
  if (ct == CharacteristicsType::Minus) {
    return m_state.Q2;
  } else {
    if (id == left(0)->get_ID()) {
      return m_state.Q1;
    } else {
      return m_state.Q3;
    }
  }
}

void T2In1Out::iterate(const double) { calculate_q(); }

void T2In1Out::calculate_q() {
  double tol = 0.00001;
  double F01, F02, F1, F2, F11, F12, F21, F22, Fx, dQ1, dQ3, eps;
  int N = 0;

  auto& p = m_params;
  auto& s = m_state;

  Characteristics cp1{}, cp2{}, cm{};
  if (const auto* pipe = left()) {
    cp1 = m_c_plus.at(pipe->get_ID());
  }

  if (const auto* pipe = left(1)) {
    cp2 = m_c_plus.at(pipe->get_ID());
  }

  if (const auto* pipe = right()) {
    cm = m_c_minus.at(pipe->get_ID());
  }

  do {
    F01 = -cm.c - cm.b * (s.Q1 + s.Q3) -
          0.5 * (s.Q1 + s.Q3) * std::abs(s.Q1 + s.Q3) / p.cv2;
    F02 = -cm.b - std::abs(s.Q1 + s.Q3) / p.cv2;

    F1 = F01 + cp1.c - cp1.b * s.Q1 - 0.5 * s.Q1 * std::abs(s.Q1) / p.cv1;
    F2 = F01 + cp2.c - cp2.b * s.Q3 - 0.5 * s.Q3 * std::abs(s.Q3) / p.cv3;
    F11 = F02 - cp1.b - std::abs(s.Q1) / p.cv1;
    F12 = F02;
    F21 = F02;
    F22 = F02 - cp2.b - std::abs(s.Q3) / p.cv3;

    Fx = 1.0 / (F21 * F12 - F11 * F22);
    dQ1 = Fx * (F1 * F22 - F2 * F12);
    dQ3 = Fx * (F2 * F11 - F1 * F21);

    s.Q1 = s.Q1 + dQ1;
    s.Q3 = s.Q3 + dQ3;
    N++;
    eps = std::abs(dQ1) + std::abs(dQ3);
  } while (eps > tol && N < 100);

  s.Q2 = s.Q1 + s.Q3;
  s.H1 = cp1.c - cp1.b * s.Q1;
  s.H2 = cm.c + cm.b * s.Q2;
  s.H3 = cp2.c - cp2.b * s.Q3;
  s.H4 = s.H2 + s.Q2 * std::abs(s.Q2) / (2.0 * p.cv2);
}

ElementView T2In1Out::read_view() const {
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
