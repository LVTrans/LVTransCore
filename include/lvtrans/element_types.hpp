#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace lvtrans {

enum class ElementType : std::uint8_t { Pipe, Reservoir, Valve };

struct PipeParameters {
  double length{};
  double diameter{};
  double f{};
  double a{};
  double z0{};
  double z1{};
  double lambda{};
  double f_max{};
  std::size_t num_reaches{};
  bool use_diameter{};
  bool use_full_moody{};
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
  double cvp{};  // Loss coefficient with positive flow relative to element
  double cvm{};  // Loss coefficient with negative flow relative to element
};

struct ValveState {
  double tau{};
};

struct ReservoirParameters {
  double H0{};   //< Nominal geodesic level of the reservoir from datum.
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
  std::vector<DataPoint> values{};

  void print() const {
    std::cout << "VectorValue: name=" << name << " symbol=" << symbol
              << " x_unit=" << x_unit << " y_unit=" << y_unit
              << " size=" << values.size() << std::endl;

    for (size_t i = 0; i < values.size(); ++i) {
      std::cout << "  x[" << i << "] = " << values[i].x << " y[" << i
                << "] = " << values[i].y << std::endl;
    }
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << "VectorValue: name=" << name << " symbol=" << symbol
       << " x_unit=" << x_unit << " y_unit=" << y_unit
       << " size=" << values.size() << std::endl;

    for (size_t i = 0; i < values.size(); ++i) {
      ss << "  x[" << i << "] = " << values[i].x << " y[" << i
         << "] = " << values[i].y << std::endl;
    }
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
