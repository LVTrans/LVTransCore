#pragma once

#include <format>
#include <string>
#include <variant>
#include <vector>

namespace lvtrans {
enum class PipeDimension { Circular, CrossSectional };
using InitialPipeState = std::variant<double, std::vector<double>>;

struct PipeState {
  std::vector<double> H{};
  std::vector<double> Q{};
};

struct SimpleConnectionState {
  double Q{};
};

struct ValveInternalServoState {
  double tau{};       // Actual opening; also the servo/limiter feedback value.
  double setpoint{};  // Requested opening.
  bool rate_limited{};
  bool external_active{};  // True if the valve is being controlled by an
                           // external controller.
  double Q{};
  double H1{};
  double H2{};
};

struct ReservoirState {
  double Q{};
};

struct LossCoefficients {
  double cvp{1e5};  //< Loss coefficient with positive flow relative to element
  double cvm{1e5};  //< Loss coefficient with negative flow relative to element

  std::string to_str() const {
    return std::format("LossCoefficients: cvp={}, cvm={}", cvp, cvm);
  }
};

struct SurgeShaftState {
  double H1{};
  double Q1{};
  double H2{};
  double Q2{};
  double H3{};
  double Q3{};

  double L{};
  double Hb{};
  double H_liquid{};
  double H_liquid_0{};
};

struct PIDValveState {
  bool active{true};
  bool pu_SP{false};
  bool automatic{true};
  bool antiwindup{true};
  bool reset{false};
  double PV_Q{};
  double SP_q{};  // [pu]
  double PV_q{};  // [
  double kappa_man{};
  double Tt{10.0};
  double Tf{-1};
  double e_t_h{};
  double integral_initial_value{1.0};
  double integral_min{-1e8};
  double integral_max{1e8};

  // read-only
  double kappa_servo{};  // [pu]
  double PV_Q_out{};     // [m³/s]
  double PV_q_out{};     // [pu]
};

using ElementID = int;
using ElementState = std::variant<PipeState, ReservoirState, SurgeShaftState,
                                  ValveInternalServoState>;
}  // namespace lvtrans
