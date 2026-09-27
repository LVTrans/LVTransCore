#pragma once

#include <cstdint>
#include <variant>
struct SetValveOpening {
  double tau;
};
struct SetPeltonInjector {
  double flow_rate;
};
struct SetPeltonExtractor {
  double flow_rate;
};
struct SetReservoirH0 {
  double h0;
};

using ElementModification = std::variant<SetValveOpening, SetPeltonInjector,
                                         SetPeltonExtractor, SetReservoirH0>;

enum class ModificationResult : std::uint8_t {
  Ok,
  ElementNotFound,
  UnsupportedModification,
  InvalidValue
};
