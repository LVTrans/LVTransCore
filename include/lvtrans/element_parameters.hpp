#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <variant>
#include "lvtrans/element_types.hpp"

namespace lvtrans {

struct PIDTurbineParameters {
  double rated_power{};        // Pr [MW]
  double rated_speed{};        // Nr [rpm]
  double power_setpoint{};     // SP [MW], initial value
  double kp_grid{};            // Grid proportional gain
  double ti_grid{};            // Grid integral time [s]
  double td_grid{};            // Grid derivative time [s]
  double kp_island{};          // Island proportional gain
  double ti_island{};          // Island integral time [s]
  double td_island{};          // Island derivative time [s]
  double tip{};                // Special power-feedback integral time [s]
  double ramp_time{};          // Power-setpoint ramp time [s]
  double droop{};              // Rp [pu]
  double closing_time_high{};  // Extrapolated closing time above changeover [s]
  double closing_time_low{};   // Extrapolated closing time below changeover [s]
  double opening_time_high{};  // Extrapolated opening time above changeover [s]
  double opening_time_low{};   // Extrapolated opening time below changeover [s]
  double kappa_change{};       // Servo changeover opening [pu]
  double a{};                  // Y/P polynomial coefficient
  double b{1.0};               // Y/P polynomial coefficient; manual default = 1
  double c{};                  // Y/P polynomial coefficient
  double servo_max{};          // Maximum servo opening [pu]
  double manual_default{};     // Initial manual opening [pu]
  bool power_droop{};  // Power-related rather than opening-related droop
                       //
  std::string to_str() const {
    return std::format(
        "PIDTurbineParameters: rated_power={}, rated_speed={}, "
        "power_setpoint={}, "
        "kp_grid={}, ti_grid={}, td_grid={}, kp_island={}, ti_island={}, "
        "td_island={}, tip={}, ramp_time={}, droop={}, closing_time_high={}, "
        "closing_time_low={}, opening_time_high={}, opening_time_low={}, "
        "kappa_change={}, a={}, b={}, c={}, servo_max={}, manual_default={}, "
        "power_droop={}",
        rated_power, rated_speed, power_setpoint, kp_grid, ti_grid, td_grid,
        kp_island, ti_island, td_island, tip, ramp_time, droop,
        closing_time_high, closing_time_low, opening_time_high,
        opening_time_low, kappa_change, a, b, c, servo_max, manual_default,
        power_droop);
  }
};

struct SurgeShaftStandardParameters {
  bool use_area{true};   // Otherwise use diameter
  double area{10.0};     // Surface area [m^2]
  double diameter{5.0};  // Equivalent diameter [m]
  double f{50.0e-3};     // Shaft friction factor
  double cvp{1e5};  //< Loss coefficient with positive flow relative to element
  double cvm{1e5};  //< Loss coefficient with negative flow relative to element
  double H_start{100.0};  // H start [m from datum]
  std::string to_str() const {
    return std::format(
        "SurgeShaftStandardParameters: use_area={}, area={}, diameter={}, "
        "f={}, cvp={}, cvm={}, initial_level={}",
        use_area, area, diameter, f, cvp, cvm, H_start);
  }
};

struct TConnectionParameters {
  double cv1{};  // Branch 1 to the junction
  double cv2{};  // Junction to branch 2
  double cv3{};  // Branch 3 to/from the junction
  std::string to_str() const {
    return std::format("TConnectionParameters: cv1={}, cv2={}, cv3={}", cv1,
                       cv2, cv3);
  }
};

struct ValveInternalServoParameters {
  double cvp{1e5};  //< Loss coefficient with positive flow relative to element
  double cvm{1e5};  //< Loss coefficient with negative flow relative to element
  double closing_time_high{};  // Extrapolated closing/opening time [s]
  double closing_time_low{};   // Extrapolated closing/opening time [s]
  double switch_over{};        // Servo changeover opening [pu]
  double servo_time{};         // Tk_s, first-order servo time constant [s]
  double initial_opening{};    // tau_0 [pu]
  bool use_cv_file{};
  std::string cv_file{};
  double diameter{};  // Inner valve diameter [m]
                      //
  std::string to_str() const {
    return std::format(
        "ValveInternalServoParameters: cvp={}, cvm={}, closing_time_high={}, "
        "closing_time_low={}, switch_over={}, servo_time={}, "
        "initial_opening={}, use_cv_file={}, cv_file={}, diameter={}",
        cvp, cvm, closing_time_high, closing_time_low, switch_over, servo_time,
        initial_opening, use_cv_file, cv_file, diameter);
  }
};

using ElementParameters =
    std::variant<ValveParameters, PipeParameters, ReservoirParameters,
                 LossCoefficients, TConnectionParameters,
                 SurgeShaftStandardParameters, PIDTurbineParameters,
                 ValveInternalServoParameters>;

}  // namespace lvtrans
