#pragma once

#include <cstdint>
#include <string>

namespace lvtrans {

struct PlantMetaData {
  std::string name;
};

struct SimulationConfig {
  double step_size{0.1};
};

struct PlantState {
  double current_time{0};
  std::uint64_t num_iterations{0};

  void reset() {
    current_time = 0;
    num_iterations = 0;
  }
};

}  // namespace lvtrans
