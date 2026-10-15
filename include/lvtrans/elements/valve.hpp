#pragma once
#include <cmath>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/controller.hpp"
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
  ElementType get_type() const override { return ElementType::Valve; }
  ElementView read_view() const override;

  double get_H(CharacteristicsType ct, ElementID id) const override;
  double get_Q(CharacteristicsType, ElementID id,
               const IterateInput) const override;

  void reset_state() override { m_state = ValveState{m_params.tau_i}; }

  double get_tau() const { return m_state.tau; }

 private:
  double calculate_q(ElementID id) const;

  ValveParameters m_params{};
  ValveState m_state{};
  Controller m_controller{};
};

}  // namespace lvtrans
