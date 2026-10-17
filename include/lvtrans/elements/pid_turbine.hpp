#pragma once
#include "lvtrans/element_parameters.hpp"
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
  virtual ElementType get_type() const override;
  virtual ElementParameters get_parameters() const override;
  virtual std::optional<ElementState> get_state() const override;
  virtual ElementView read_view() const override;
  virtual void reset_state() override;
  virtual void disconnect_all() override { m_controllable = nullptr; }

  virtual Controllable* get_controllable() const override {
    return m_controllable;
  }
  virtual void set_controllable(Controllable* controllable) override {
    m_controllable = controllable;
  }

 private:
  PIDTurbineParameters m_params{};
  Controllable* m_controllable{nullptr};
};

}  // namespace lvtrans
