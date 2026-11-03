#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/controllable.hpp"
#include "lvtrans/elements/non-pipe.hpp"

namespace lvtrans {

class ValveInternalServo : public Controllable, public NonPipe {
 public:
  ValveInternalServo(const ValveInternalServoParameters& parameters, double dt);
  ValveInternalServo(const ValveInternalServoParameters& parameters, double dt,
                     ValveInternalServoState state);
  ~ValveInternalServo() override;
  void disconnect_all() noexcept override;
  ElementType get_type() const override {
    return ElementType::ValveInternalServo;
  }
  ElementParameters get_parameters() const override { return m_params; }
  std::optional<ElementState> get_state() const override { return m_state; }
  ElementView read_view() const override;
  ModificationResult modify(const ElementModification& mod) override;
  void iterate(double t = 0) override;
  void reset_state() override;
  double get_H(CharacteristicsType type, ElementID id) override;
  double get_Q(CharacteristicsType, ElementID, IterateInput = {}) override;
  double get_tau() const { return m_state.tau; }
  double get_setpoint() const { return m_state.setpoint; }
  bool is_rate_limited() const { return m_state.rate_limited; }
  const ValveInternalServoParameters& config() const { return m_params; }

 private:
  double calculate_q();
  double m_dt;
  ValveInternalServoParameters m_params;
  ValveInternalServoState m_state;
};

}  // namespace lvtrans
