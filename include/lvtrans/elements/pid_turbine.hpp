#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/elements/controllable.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

// LVTrans manual section 12.6: Turbine governor for Francis or Pelton; grid and
// island modes. Skeleton: remains abstract until the Element interface and
// solver are implemented.
class PIDTurbine : public Element {
 public:
  explicit PIDTurbine(const PIDTurbineParameters& parameters);
  PIDTurbine() = default;
  ~PIDTurbine() = default;
  const PIDTurbineParameters& config() const { return m_params; }
  void iterate(const double t = 0) override;
  virtual ElementType get_type() const override;
  virtual ElementParameters get_parameters() const override;
  virtual std::optional<ElementState> get_state() const override;
  virtual ElementView read_view() const override;
  virtual void reset_state() override;

 private:
  PIDTurbineParameters m_params;
  Controllable m_controllable;
};

}  // namespace lvtrans
