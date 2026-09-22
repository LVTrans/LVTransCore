#pragma once
#include <cmath>
#include "lvtrans/element.hpp"
#include "lvtrans/non-pipe.hpp"

namespace lvtrans {

struct ValveConfig : public LossCoefficients {
  double tau_i{};
  double tau_f{};
  double tc{};
  double em{};
};

struct ValveState {
  double tau{};
};

class Valve : public NonPipe {
 public:
  Valve(const ValveConfig& conf);
  Valve(const ValveConfig& conf, ValveState state);
  ~Valve();
  void iterate(const IterateInput input, IterateOutput output) override;
  double get_H() const override {
    return m_c_characteristics - m_b_characteristics * calculate_q();
  }
  double get_Q(const IterateInput) const override { return calculate_q(); }

  double get_tau() const { return m_state.tau; }

 private:
  double calculate_q() const {
    const double CV = m_state.tau * m_state.tau * m_config.cvp;
    return -CV * m_b_characteristics +
           std::sqrt(CV * CV * m_b_characteristics * m_b_characteristics +
                     2.0 * CV * m_c_characteristics);
  }

  ValveConfig m_config{};
  ValveState m_state{};
};

}  // namespace lvtrans
