#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace lvtrans {

enum class ElementType : std::uint8_t {
  Pipe,
  AlgebraicPipe,
  Reservoir,
  Valve,
  ValveInternalServo,
  SurgeShaftStandard,
  PID,
  PIDTurbine,
  ConstantLevelLeft,
  ConstantLevelRight,
  SimpleConnection,
  T1In2OutN,
  T1In2OutS,
  T2In1OutN,
  T2In1OutS,
};

constexpr std::string_view element_type_to_str(ElementType type) {
  switch (type) {
    case ElementType::Pipe:
      return "Pipe";
    case ElementType::AlgebraicPipe:
      return "AlgebraicPipe";
    case ElementType::Reservoir:
      return "Reservoir";
    case ElementType::Valve:
      return "Valve";
    case ElementType::ValveInternalServo:
      return "ValveInternalServo";
    case ElementType::SurgeShaftStandard:
      return "SurgeShaftStandard";
    case ElementType::PID:
      return "PID";
    case ElementType::PIDTurbine:
      return "PIDTurbine";
    case ElementType::ConstantLevelLeft:
      return "ConstantLevelLeft";
    case ElementType::ConstantLevelRight:
      return "ConstantLevelRight";
    case ElementType::SimpleConnection:
      return "SimpleConnection";
    case ElementType::T1In2OutN:
      return "T1In2OutN";
    case ElementType::T1In2OutS:
      return "T1In2OutS";
    case ElementType::T2In1OutN:
      return "T2In1OutN";
    case ElementType::T2In1OutS:
      return "T2In1OutS";
    default:
      return "UnknownElementType";
  }
}

enum class PipeDimension { Circular, CrossSectional };
using InitialPipeState = std::variant<double, std::vector<double>>;

namespace detail {

template <typename Range, typename Formatter>
std::string parameter_sequence_to_str(const Range& values, Formatter format) {
  std::string result{"["};
  bool first = true;
  for (const auto& value : values) {
    if (!first) {
      result += ", ";
    }
    result += format(value);
    first = false;
  }
  result += "]";
  return result;
}

inline std::string initial_pipe_state_to_str(const InitialPipeState& state) {
  return std::visit(
      [](const auto& value) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double>) {
          return std::format("{}", value);
        } else {
          return parameter_sequence_to_str(
              value, [](double sample) { return std::format("{}", sample); });
        }
      },
      state);
}

}  // namespace detail

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
                      //
                      //
  std::string to_str() const {
    return std::format(
        "PipeParameters: H0={}, Q0={}, length={}, diameter={}, area={}, "
        "periphery={}, epsilon={}, ny={}, rho={}, f={}, a={}, z0={}, z1={}, "
        "lambda={}, f_max={}, dimension={}, use_diameter={}, "
        "use_full_moody={}, "
        "use_Dh={}",
        detail::initial_pipe_state_to_str(H0),
        detail::initial_pipe_state_to_str(Q0), length, diameter, area,
        periphery, epsilon, ny, rho, f, a, z0, z1, lambda, f_max,
        static_cast<int>(dimension), use_diameter, use_full_moody, use_Dh);
  }
};

struct PipeState {
  std::vector<double> H{};
  std::vector<double> Q{};
  std::vector<double> Z{};
};

struct ValveParameters {
  double tau_i{};
  double tau_f{};
  double tc{};
  double em{};
  double cda0{};
  double cvp{1e5};  // Loss coefficient with positive flow relative to element
  double cvm{1e5};  // Loss coefficient with negative flow relative to element
  double closing_time_h{10};
  double closing_time_l{10};
  double switch_over{0.5};
  double tks{1.0};
  double tau_0{1.0};
  double diameter{1.0};  ///< internal diameter
  std::string to_str() const {
    return std::format(
        "ValveParameters: tau_i={}, tau_f={}, tc={}, em={}, cda0={}, cvp={}, "
        "cvm={}, closing_time_h={}, closing_time_l={}, switch_over={}, tks={}, "
        "tau_0={}, diameter={}",
        tau_i, tau_f, tc, em, cda0, cvp, cvm, closing_time_h, closing_time_l,
        switch_over, tks, tau_0, diameter);
  }
};

struct ValveState {
  double tau{};
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

struct DataPoint {
  double x{};
  double y{};
};

using ElementID = int;

using ElementState = std::variant<ValveState, PipeState, ReservoirState>;

struct ScalarValue {
  std::string name{};
  std::string unit{};
  std::string symbol{};
  double value{};

  void print() const {
    std::cout << "ScalarValue: name=" << name << " unit=" << unit
              << " symbol=" << symbol << " value=" << value << std::endl;
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << "ScalarValue: name=" << name << " unit=" << unit
       << " symbol=" << symbol << " value=" << value;
    return ss.str();
  }
};

struct VectorValue {
  std::string name{};
  std::string symbol{};
  std::string x_unit{};
  std::string y_unit{};
  std::vector<double> values{};

  void print() const {
    std::cout << "VectorValue: name=" << name << " symbol=" << symbol
              << " x_unit=" << x_unit << " y_unit=" << y_unit << " back"
              << values.back() << " size=" << values.size() << std::endl;
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << "VectorValue: name=" << name << " symbol=" << symbol
       << " x_unit=" << x_unit << " y_unit=" << y_unit << " back"
       << values.back() << " size=" << values.size() << std::endl;

    return ss.str();
  }
};

using DisplayValue = std::variant<ScalarValue, VectorValue>;

struct ElementView {
  ElementID element_id{};
  std::vector<DisplayValue> values{};

  void print() const {
    std::cout << "ElementView: element_id=" << element_id
              << " values.size=" << values.size() << std::endl;

    std::cout << "  values: " << std::endl;
    for (size_t i = 0; i < values.size(); ++i) {
      std::cout << "  values[" << i << "] = ";
      std::visit([](const auto& value) { value.print(); }, values[i]);
    }
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << "ElementView: element_id=" << element_id
       << " values.size=" << values.size() << std::endl;
    for (size_t i = 0; i < values.size(); ++i) {
      ss << "  values[" << i << "] = ";
      std::visit([&ss](const auto& value) { ss << value.to_string(); },
                 values[i]);
    }
    return ss.str();
  }
};

}  // namespace lvtrans
