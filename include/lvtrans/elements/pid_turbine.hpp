#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/controllable.hpp"
#include "lvtrans/elements/controller.hpp"

namespace lvtrans {

class PIDTurbine : public Controller {
 public:
  explicit PIDTurbine(const PIDTurbineParameters& parameters);
  PIDTurbine();
  ~PIDTurbine();
  const PIDTurbineParameters& config() const { return m_params; }
  void iterate(const double t = 0) override;
  ElementType get_type() const override;
  ElementParameters get_parameters() const override;
  std::optional<ElementState> get_state() const override;
  ElementView read_view() const override;
  void reset_state() override;
  double get_output() const override { return 0; }
  bool is_active() const override { return true; }

 private:
  PIDTurbineParameters m_params{};
};

}  // namespace lvtrans
