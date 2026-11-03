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

struct DataPoint {
  double x{};
  double y{};
};

using ElementID = int;

using ElementState = std::variant<PipeState, ReservoirState, SurgeShaftState,
                                  ValveInternalServoState>;
}  // namespace lvtrans
