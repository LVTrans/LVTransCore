#pragma once

#include <cstddef>
#include <cstdint>
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

using ElementState = std::variant<ValveState, PipeState>;
using ElementParameters =
    std::variant<ValveParameters, PipeParameters, ReservoirParameters>;

}  // namespace lvtrans
