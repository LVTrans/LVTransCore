#pragma once
#include <cmath>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

class Valve : public NonPipe {
 public:
  Valve(const ValveParameters& conf);
  Valve(const ValveParameters& conf, ValveState state);
  ~Valve();
  void iterate(const IterateInput input, IterateOutput output) override;
  ElementParameters get_parameters() const override { return m_config; }
  std::optional<ElementState> get_state() const override { return m_state; }
  double get_H() const override {
    return m_c_characteristics - m_b_characteristics * calculate_q();
  }
  double get_Q(const IterateInput) const override { return calculate_q(); }

  double get_tau() const { return m_state.tau; }
  ElementType get_type() override { return ElementType::Valve; }

 private:
  double calculate_q() const {
    const double CV = m_state.tau * m_state.tau * m_config.cvp;
    return -CV * m_b_characteristics +
           std::sqrt(CV * CV * m_b_characteristics * m_b_characteristics +
                     2.0 * CV * m_c_characteristics);
  }

  ValveParameters m_config{};
  ValveState m_state{};
};

}  // namespace lvtrans
