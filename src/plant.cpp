#include "lvtrans/plant.hpp"
#include <iostream>
#include <utility>
#include "lvtrans/config/plant_config_repository.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

Plant::Plant(double step_size) : m_data{} {
  m_data.config.step_size = step_size;
}

Plant::Plant(PlantData& data) : m_data(std::move(data)) {}

Plant::Plant(const std::string& file_path) : m_data{} {
  auto path = std::filesystem::path(file_path);
  auto res = PlantConfigRepository::load(path, m_data);

  if (res != PlantRepositoryResult::Ok) {
    std::cerr << "Failed to load plant config\n";
    return;
  }
}

void Plant::step() {
  m_data.state.current_time += m_data.config.step_size;

  for (auto& pipe : m_data.element_container.get_pipes()) {
    pipe->iterate();
  }

  for (auto& non_pipe : m_data.element_container.get_non_pipes()) {
    IterateInput input{};
    input.t = m_data.state.current_time;
    non_pipe->iterate(input);
  }
  m_data.state.num_iterations++;
}

void Plant::run_steps(size_t num_steps) {
  for (size_t i = 0; i < num_steps; ++i) {
    step();
  }
}

void Plant::display() const {
  for (auto& pipe : m_data.element_container.get_pipes()) {
    auto* left_elem = pipe->left_elem();
    if (left_elem) {
      std::cout << "R(" << left_elem->get_ID() << ")<---->";
    }
    std::cout << "P(" << pipe->get_ID() << ")";
    auto* right_elem = pipe->right_elem();
    if (right_elem) {
      std::cout << "<---->V(" << right_elem->get_ID() << ")\n";
    }
  }
}

}  // namespace lvtrans
