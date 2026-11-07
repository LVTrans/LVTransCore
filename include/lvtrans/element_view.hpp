#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include "element_types.hpp"
#include "lvtrans/element_states.hpp"

namespace lvtrans {
struct ScalarValue {
  std::string name{};
  std::string unit{};
  std::string symbol{};
  double value{};

  void display(std::ostream& os) const {
    os << "ScalarValue: name=" << name << " unit=" << unit
       << " symbol=" << symbol << " value=" << value << '\n';
  }

  void print() const { display(std::cout); }

  std::string to_string() const {
    std::stringstream ss;
    display(ss);
    return ss.str();
  }
};

struct VectorValue {
  std::string name{};
  std::string symbol{};
  std::string x_unit{};
  std::string y_unit{};
  std::vector<double> values{};
  std::vector<double> x_values{};

  void display(std::ostream& os) const {
    os << "VectorValue: name=" << name << " symbol=" << symbol
       << " x_unit=" << x_unit << " y_unit=" << y_unit << " back"
       << values.back() << " size=" << values.size() << '\n';
  }
  void print() const { display(std::cout); }

  std::string to_string() const {
    std::stringstream ss;
    display(ss);
    return ss.str();
  }
};

using DisplayValue = std::variant<ScalarValue, VectorValue>;

struct ElementView {
  ElementID element_id{};
  std::vector<DisplayValue> values{};
};
}  // namespace lvtrans
