#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/controllable.hpp"
#include "lvtrans/elements/controller.hpp"

namespace lvtrans {
struct PIDValveDerievedData {
  double Tt{0.0};    // Anti-windup
  double SP_q{0.0};  // Setpoint for flow rate [pu]
};

class PIDValve : public Controller {
 public:
  explicit PIDValve(const PIDValveParameters& parameters);
  PIDValve();
  ~PIDValve();
  const PIDValveParameters& config() const { return m_params; }
  void iterate(const double t = 0) override;
  ElementType get_type() const override;
  ElementParameters get_parameters() const override;
  std::optional<ElementState> get_state() const override;
  ElementView read_view() const override;
  void reset_state() override;
  void set_Q_input(double Q) { m_derived.SP_q = Q / m_params.Q_wanted; }

 private:
  PIDValveParameters m_params{};
  PIDValveDerievedData m_derived{};
};

}  // namespace lvtrans
