#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace lvtrans {

enum class ElementType : std::uint8_t { Pipe, Reservoir, Valve };
enum class PipeDimension { Circular, CrossSectional };

// #### PIPE ####
struct PipeParameters {
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
};

struct PipeState {
  std::vector<double> H{};
  std::vector<double> Q{};
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
  double diameter{1};  ///< internal diameter
};

struct ValveState {
  double tau{};
};

struct ReservoirParameters {
  double H0{};      //< Nominal geodesic level of the reservoir from datum.
  double cvp{1e5};  //< Loss coefficient with positive flow relative to element
  double cvm{1e5};  //< Loss coefficient with negative flow relative to element
};

struct LossCoefficients {
  double cvp{};  //< Loss coefficient with positive flow relative to element
  double cvm{};  //< Loss coefficient with negative flow relative to element
};

struct DataPoint {
  double x{};
  double y{};
};

using ElementID = int;

using ElementState = std::variant<ValveState, PipeState>;
using ElementParameters =
    std::variant<ValveParameters, PipeParameters, ReservoirParameters>;

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
              << " x_unit=" << x_unit << " y_unit=" << y_unit
              << " size=" << values.size() << std::endl;

    // for (size_t i = 0; i < values.size(); ++i) {
    //   std::cout << "  x[" << i << "] = " << values[i].x << " y[" << i
    //             << "] = " << values[i].y << std::endl;
    // }
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << "VectorValue: name=" << name << " symbol=" << symbol
       << " x_unit=" << x_unit << " y_unit=" << y_unit
       << " size=" << values.size() << std::endl;

    // for (size_t i = 0; i < values.size(); ++i) {
    //   ss << "  x[" << i << "] = " << values[i].x << " y[" << i
    //      << "] = " << values[i].y << std::endl;
    // }
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
