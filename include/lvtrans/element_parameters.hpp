#pragma once

#include <array>
#include <cstddef>
#include <string>
#include "lvtrans/element_types.hpp"

namespace lvtrans {

// Parameters for the element skeletons, from the October 2023 LVTrans manual,
// chapter 12. Zero initialization is not a valid physical configuration.
// These types are not yet part of the serialized ElementParameters variant.

// Section 12.6, Table 12.6. Also used by PID Turbine AFF (section 12.8).
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
};

// Section 12.7, Table 12.7.
struct PIDPumpParameters {
  double rated_speed{};   // Nr [rpm]
  double rated_flow{};    // Qr [m^3/s]
  double rated_head{};    // Hr [m]
  double rated_torque{};  // Tr [Nm]
  double kp{};
  double ti{};         // Integral time [s]
  double td{};         // Derivative time [s]
  bool check_valve{};  // Include a check valve at startup
};

// Section 12.9, Table 12.8. Also used by Pelton Sump (section 12.19).
struct SurgeShaftStandardParameters {
  bool use_area{};            // Otherwise use diameter
  double area{};              // Surface area [m^2]
  double diameter{};          // Equivalent diameter [m]
  double f{};                 // Shaft friction factor
  LossCoefficients losses{};  // Positive: up into shaft; negative: down out
  double initial_level{};     // H start [m from datum]
};

struct ShaftProfilePoint {
  double level{};  // L [m above the tunnel floor]
  double area{};   // A [m^2]
};

// Section 12.10, Table 12.9. All ten profile points must be supplied with
// strictly increasing levels. Interpolate between points; clamp area outside.
struct SurgeShaftVariableParameters {
  LossCoefficients losses{};
  double initial_level{};     // H start [m from datum]
  double weir_width{};        // B [m]
  double weir_coefficient{};  // C
  double weir_height{};       // H weir [m above the tunnel floor]
  std::array<ShaftProfilePoint, 10> profile{};
};

// Section 12.11, Table 12.10. Actual inflow is a future runtime input.
struct SurgeShaftVariableQinParameters {
  SurgeShaftVariableParameters shaft{};
  double inflow_fraction{};  // X in X * global Qin; may be greater than 1
  bool individual_q{};       // Use individual inflow instead of global Qin
};

// Sections 12.12-12.13, Table 12.11. Shared by Normal and Normal Q1.
struct CreekShaftParameters {
  SurgeShaftStandardParameters shaft{};
  double inflow_fraction{};  // X in X * global Qin; may be greater than 1
  bool individual_q{};
};

// Sections 12.14-12.15, Tables 12.12-12.13. Each branch coefficient is the
// same for both flow directions. N/S variants differ only in diagram layout.
struct TConnectionParameters {
  double cv1{};  // Branch 1 to the junction
  double cv2{};  // Junction to branch 2
  double cv3{};  // Branch 3 to/from the junction
};

// Section 12.17, Table 12.15. BEP/rated data must be mutually consistent.
struct FrancisParameters {
  double q_bep{};         // Flow [m^3/s]
  double h_bep{};         // Head [m]
  double h_bep_design{};  // Design head [m]
  double rated_speed{};   // N rated [rpm]
  double t_bep{};         // Mechanical torque [Nm]
  double e_bep{};         // Electrical torque [Nm]
  double a1_bep{};        // Inlet angle [degrees]
  double b1_bep{};        // Outlet angle [degrees]
  double r1{};            // Runner inlet radius [m]
  double r2{};            // Runner outlet radius [m]
  double ta{};            // Rotating-mass time constant [s]
  double twt{};           // Internal water-inertia time constant [s]
  double rq{};            // Design constant
  double rm{};            // Design constant
  double rd{};            // Design constant
  double eta_hydraulic_bep{};
  double eta_total_bep{};
  std::size_t num_turbines{};
  std::size_t pole_pairs{};
  double grid_damping{};  // D grid
  // Manual labels Delta r [rad] but describes electrical degrees.
  // Resolve the unit against the original model before implementing a solver.
  double delta_r{};
  double grid_frequency{};  // F grid [Hz]
  double eve_mod{};         // Runaway-speed model constant
};

// Section 12.18, Table 12.16.
struct PeltonParameters {
  double q_bep{};        // Flow [m^3/s]
  double h_bep{};        // Head [m]
  double rated_speed{};  // N rated [rpm]
  double t_bep{};        // Mechanical torque [Nm]
  double e_bep{};        // Electrical torque [Nm]
  double ta{};           // Rotating-mass time constant [s]
  double rm{};           // Design constant
  double eta_bep{};      // Hydraulic efficiency
  std::size_t num_turbines{};
  std::size_t pole_pairs{};
  double grid_damping{};
  double delta_r{};           // Same unresolved manual unit as Francis
  double grid_frequency{};    // [Hz]
  std::size_t num_needles{};  // Number of injectors
  double eta_3{};             // Design constant
  double q_max{};             // Design constant
};

using PeltonSumpParameters = SurgeShaftStandardParameters;

// Section 12.20, Table 12.17. The Suter file has five tab-separated columns;
// the original model reads columns 1, 4, 5 after the header (X, WH, WB).
struct PumpCentrifugalParameters {
  double rated_flow{};    // [m^3/s]
  double rated_head{};    // [m]
  double rated_speed{};   // [rpm]
  double rated_torque{};  // [Nm]
  double rated_power{};   // [MW]
  double inertia{};       // I0, polar moment of inertia [kg m^2]
  std::size_t num_pumps{};
  std::string suter_file{};
};

// Section 12.21, Table 12.18. Cv files contain opening [pu] and Cv columns.
struct ValveInternalServoParameters {
  LossCoefficients losses{};
  double closing_time_high{};  // Extrapolated closing/opening time [s]
  double closing_time_low{};   // Extrapolated closing/opening time [s]
  double switch_over{};        // Servo changeover opening [pu]
  double servo_time{};         // Tk_s, first-order servo time constant [s]
  double initial_opening{};    // tau_0 [pu]
  bool use_cv_file{};
  std::string cv_file{};
  double diameter{};  // Inner valve diameter [m]
};

// Section 12.22, Table 12.19. PRV = Pressure Release Valve.
struct PRVParameters {
  double cv{};
  double opening_time{};      // Extrapolated opening time [s]
  double closing_time{};      // Extrapolated closing time [s]
  double closing_time_low{};  // Extrapolated closing/opening time [s]
  double servo_time{};        // Tk_s [s]
  double initial_opening{};   // tau_0 [pu]
  double max_y_speed{};       // Expressed as PID extrapolated closing time [s]
  bool use_cv_file{};
  std::string cv_file{};
  double diameter{};  // [m]
};

struct ChannelProfilePoint {
  double length{};     // L [m along channel]; first point must be zero
  double elevation{};  // z [m from datum]
};

// Sections 12.23-12.26, Table 12.20. Shared by NL, LL and LR variants.
struct OpenChannelParameters {
  double mannings{};           // Manning n (SI)
  double width{};              // B [m]
  double c1{};                 // Height of rectangular portion [m]
  double c3{};                 // Depth of numerical fixing portion [m]
  double initial_flow{};       // Q_start [m^3/s]
  double initial_level{};      // H_start [m from datum]
  double e{};                  // Width of numerical fixing portion [m]
  std::size_t num_elements{};  // N per sub-element
  double delta{};              // Numerical constant in [0.5, 1.0]
  std::array<ChannelProfilePoint, 5> profile{};
};

}  // namespace lvtrans
