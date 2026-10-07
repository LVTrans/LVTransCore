#include "lvtrans/plant.hpp"
#include <iostream>
#include <utility>
#include "lvtrans/config/plant_config_repository.hpp"
#include "lvtrans/elements/element.hpp"

namespace lvtrans {

Plant::Plant(double step_size) : m_data{} {
  m_data.config.step_size = step_size;
}

Plant::Plant(PlantData&& data) : m_data(std::move(data)) {}

Plant::Plant(const std::string& file_path) : m_data{} {
  auto path = std::filesystem::path(file_path);

  if (PlantConfigRepository::load(path, m_data) != PlantRepositoryResult::Ok) {
    std::cerr << "Failed to load plant config\n";
  }
}

ModificationResult Plant::modify_element(ElementID id,
                                         const ElementModification& mod) const {
  auto* elem = m_data.element_container.get_element_by_id(id);
  if (!elem) {
    return ModificationResult::ElementNotFound;
  }

  return elem->modify(mod);
}

void Plant::step() {
  m_data.state.current_time += m_data.config.step_size;
  ++m_data.state.num_iterations;

  for (auto& pipe : m_data.element_container.get_pipes()) {
    pipe->iterate(m_data.state.current_time);
  }

  for (auto& non_pipe : m_data.element_container.get_non_pipes()) {
    non_pipe->iterate(m_data.state.current_time);
  }
}

void Plant::run_steps(size_t num_steps) {
  for (size_t i = 0; i < num_steps; ++i) {
    step();
  }
}

std::optional<ElementView> Plant::read_state(ElementID id) const {
  auto elem = m_data.element_container.get_element_by_id(id);
  if (!elem) {
    return std::nullopt;
  }

  return elem->read_view();
}

void Plant::reset_state() {
  m_data.state.reset();

  for (const auto& e : m_data.element_container.get_elements()) {
    e->reset_state();
  }
}

void Plant::display() const {
  for (auto& pipe : m_data.element_container.get_pipes()) {
    auto left_elem = pipe->left_elem();
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
