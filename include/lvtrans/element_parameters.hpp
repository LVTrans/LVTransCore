#pragma once

#include <string>
#include <variant>
#include "lvtrans/element_states.hpp"

namespace lvtrans {

struct PIDValveParameters {
  double Q_wanted{10};  //< Average flow wanted through the valve [m³/s]
  double SP_Q{10};      //< Setpoint flow [m³/s]
  double Kp{1};         //< Proportional gain
  double Ti{10};        //< Integral time
  double Td{0};         //< Derivative time

  std::string to_str() const {
    return std::format(
        "PIDValveParameters: Q_wanted={}, SP_Q={}, Kp={}, Ti={}, Td={}",
        Q_wanted, SP_Q, Kp, Ti, Td);
  }
};

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

struct PipeParameters {
  InitialPipeState H0{100.0};
  InitialPipeState Q0{100.0};
  double length{100.0};  //< length of the pipe [m]
  double diameter{2.0};  //< diameter of the pipe [m]
  double area{};         //< cross sectional area of the pipe [m^2]
  double periphery{};    //< periphery of the pipe [m]
  double epsilon{0.1};   //< pipe roughness [m]
  double ny{
      1.1e-6};      //< Kinematic viscosity, ν (1e-6 for water a 20 deg C) when
                    // using full_moody
  double rho{1e3};  //< Density, ρ (998.2 kg/m³ for water at 20 deg C) when
                    // using full_moody
  double f{0.01};
  double a{1200.0};
  double z0{0};
  double z1{0};
  double lambda{5e5};
  double f_max{10.0};
  PipeDimension dimension{PipeDimension::Circular};
  bool use_diameter{};  //< TRUE = use pipe diameter instead of cross sectional
                        // area
  bool use_full_moody{};
  bool use_Dh{true};  //< Use hydraulic diameter

  std::string to_str() const {
    return std::format(
        "PipeParameters: length={}, diameter={}, area={}, "
        "periphery={}, epsilon={}, ny={}, rho={}, f={}, a={}, z0={}, z1={}, "
        "lambda={}, f_max={}, dimension={}, use_diameter={}, "
        "use_full_moody={}, "
        "use_Dh={}",
        length, diameter, area, periphery, epsilon, ny, rho, f, a, z0, z1,
        lambda, f_max, static_cast<int>(dimension), use_diameter,
        use_full_moody, use_Dh);
  }
};

struct ReservoirParameters {
  double H0{100.0};  //< Nominal geodesic level of the reservoir from datum.
  double cvp{1e5};   //< Loss coefficient with positive flow relative to element
  double cvm{1e5};  //< Loss coefficient with negative flow relative to elements
  std::string to_str() const {
    return std::format("ReservoirParameters: H0={}, cvp={}, cvm={}", H0, cvp,
                       cvm);
  }
};

using ElementParameters =
    std::variant<PipeParameters, ReservoirParameters, LossCoefficients,
                 TConnectionParameters, SurgeShaftStandardParameters,
                 PIDValveParameters, PIDTurbineParameters,
                 ValveInternalServoParameters>;

}  // namespace lvtrans
