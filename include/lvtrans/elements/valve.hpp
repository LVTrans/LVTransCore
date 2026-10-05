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
  ~Valve() = default;
  void iterate(const double t = 0) override;
  ModificationResult modify(const ElementModification& mod) override;
  ElementParameters get_parameters() const override { return m_params; }
  std::optional<ElementState> get_state() const override { return m_state; }

  double get_H() const override {
    return m_c_characteristics - m_b_characteristics * calculate_q();
  }
  double get_Q(const IterateInput) const override { return calculate_q(); }

  double get_tau() const { return m_state.tau; }
  ElementType get_type() override { return ElementType::Valve; }
  ElementView read_view() const override;

 private:
  double calculate_q() const {
    const double CV = m_state.tau * m_state.tau * m_params.cvp;
    return -CV * m_b_characteristics +
           std::sqrt(CV * CV * m_b_characteristics * m_b_characteristics +
                     2.0 * CV * m_c_characteristics);
  }

  ValveParameters m_params{};
  ValveState m_state{};
};

}  // namespace lvtrans
