#pragma once
#include "lvtrans/non-pipe.hpp"
#include <cmath>

namespace lvtrans {

struct ValveConfig {
  double tau_i{};
  double tau_f{};
  double tc{};
  double em{};
  double cvp{};
};

class Valve : public NonPipe {
public:
  Valve(const ValveConfig &conf);
  ~Valve();
  void iterate(const IterateInput input, IterateOutput output) override;
  double get_H() const override {
    return m_c_characteristics - m_b_characteristics * calculate_q();
  }
  double get_Q(const IterateInput) const override { return calculate_q(); }

  double get_tau() const { return m_tau; }

private:
  double calculate_q() const {
    const double CV = m_tau * m_tau * m_config.cvp;
    return -CV * m_b_characteristics +
           std::sqrt(CV * CV * m_b_characteristics * m_b_characteristics +
                     2.0 * CV * m_c_characteristics);
  }

  ValveConfig m_config{};
  double m_tau{};
  double m_CV{};
};

} // namespace lvtrans
