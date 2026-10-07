#include "lvtrans/plant.hpp"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "lvtrans/config/plant_config_repository.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/port.hpp"

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
  std::unordered_set<int> visited_ids;
  std::cout << "okj..\n";
  static std::unordered_map<ElementType, std::string> element_type_names = {
      {ElementType::Pipe, "P"},
      {ElementType::Valve, "V"},
      {ElementType::Reservoir, "R"},
  };
  for (auto& e : m_data.element_container.get_elements_sorted()) {
    // std::cout << "E(" << e->get_ID() << ")<-->";
    if (visited_ids.contains(e->get_ID())) {
      continue;
    }

    // check left element
    if (e->get_ports()[PortType::Left]) {
      auto left_peer = e->get_ports()[PortType::Left]->get_peer();
      std::cout << element_type_names[left_peer->get_type()] << "("
                << left_peer->get_ID() << ")<-->";
      visited_ids.insert(left_peer->get_ID());
    }
    std::cout << element_type_names[e->get_type()] << "(" << e->get_ID() << ")";
    // check right element
    if (e->get_ports()[PortType::Right]) {
      auto right_peer = e->get_ports()[PortType::Right]->get_peer();
      std::cout << "<-->" << element_type_names[right_peer->get_type()] << "("
                << right_peer->get_ID() << ")";
      visited_ids.insert(right_peer->get_ID());
    }

    visited_ids.insert(e->get_ID());
  }

  std::cout << "\n";
}

}  // namespace lvtrans
