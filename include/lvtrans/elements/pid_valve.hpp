#pragma once
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_states.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/controllable.hpp"
#include "lvtrans/elements/controller.hpp"

namespace lvtrans {

class PIDValve : public Controller {
 public:
  explicit PIDValve(const PIDValveParameters& parameters,
                    const double dt = 0.1);
  PIDValve();
  ~PIDValve() override;
  const PIDValveParameters& config() const { return m_params; }
  void iterate(const double t = 0) override;
  ElementType get_type() const override;
  ElementParameters get_parameters() const override;
  std::optional<ElementState> get_state() const override;
  ElementView read_view() const override;
  void reset_state() override;
  void set_SP_Q(double Q) { m_state.SP_q = Q / m_params.Q_wanted; }
  double get_PV_Q() const { return m_state.PV_Q; }
  void set_PV_Q(double Q) {
    m_state.PV_Q = Q;
    m_state.PV_q = Q / m_params.Q_wanted;
  }
  const PIDValveState& state() const { return m_state; }
  void set_controls(bool active, bool automatic, bool antiwindup, double manual,
                    double Tt, double Tf);
  void set_gains(double Kp, double Ti, double Td);
  double get_output() const override { return m_state.kappa_servo; }
  bool is_active() const override { return m_state.active; }

 private:
  void pid_iterate();
  void servo_1st_order();
  void update_measurement();
  PIDValveParameters m_params{};
  PIDValveState m_state{};
  double m_previous_time{};
  double m_previous_output{};
  double m_previous_pv{};
  double m_filtered_pv{};

  double m_previous_integral{};
  double m_previous_windup{};
  double m_previous_windup_0{};
  double m_dt{};
  double m_pid_output{};
  double m_ouput{};

  enum class Mode { Init, Auto, Manual };
  Mode m_mode{Mode::Init};
  bool m_initialized{false};
};

}  // namespace lvtrans
