#include "lvtrans/plant.hpp"
#include <iostream>
#include <unordered_set>
#include <utility>
#include "lvtrans/config/plant_config_repository.hpp"
#include "lvtrans/element_types.hpp"
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
  auto& t = m_data.state.current_time;

  t += m_data.config.step_size;
  ++m_data.state.num_iterations;

  const auto& pipes = m_data.element_container.get_pipes();
  const auto& non_pipes = m_data.element_container.get_non_pipes();

  for (auto& pipe : pipes) {
    pipe->iterate(t);
  }

  for (auto& non_pipe : non_pipes) {
    non_pipe->iterate(t);
  }

  for (auto& pipe : pipes) {
    pipe->update_boundaries();
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
  std::unordered_set<int> visited_ids;
  for (auto& e : m_data.element_container.get_elements_sorted()) {
    if (visited_ids.contains(e->get_ID())) {
      continue;
    }

    const auto* pipe = dynamic_cast<const BasePipe*>(e);
    if (pipe && pipe->left_elem()) {
      std::cout << pipe->left_elem()->get_ID() << "<-->";
      visited_ids.insert(pipe->left_elem()->get_ID());
    }
    std::cout << e->get_ID();
    if (pipe && pipe->right_elem()) {
      std::cout << "<-->" << pipe->right_elem()->get_ID();
      visited_ids.insert(pipe->right_elem()->get_ID());
    }
    std::cout << '\n';

    visited_ids.insert(e->get_ID());
  }

  std::cout << "\n";
}

}  // namespace lvtrans
