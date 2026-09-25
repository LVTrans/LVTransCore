#pragma once

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
  int num_iterations{0};
};

}  // namespace lvtrans
