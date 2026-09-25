#include "lvtrans/config/plant_config_repository.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <unordered_set>
#include "lvtrans/config/element_parsers.hpp"
#include "lvtrans/config/plant_configuration.hpp"
#include "lvtrans/plant.hpp"
#include "nlohmann/json.hpp"

namespace lvtrans {

static std::vector<ElementConfig> to_element_configs(
    const std::vector<Element*>& elements);
static std::vector<ConnectionConfig> to_connection_configs(
    const std::vector<Element*>& elements);
static PlantConfiguration to_plant_configuration(const PlantData& plant);

PlantRepositoryResult PlantConfigRepository::load(
    const std::filesystem::path& path, PlantData& plant) {
  using namespace nlohmann;
  json json;
  PlantConfiguration config;
  std::ifstream file(path);

  if (!file.is_open()) {
    std::cout << "Failed to open file: " << path << '\n';
    return PlantRepositoryResult::Error;
  }

  // must use try-catch as per the json library
  try {
    json = json::parse(file);
    config = json.get<PlantConfiguration>();
  } catch (json::parse_error& ex) {
    return PlantRepositoryResult::Error;
  }

  plant.config = config.simulation;
  plant.meta = config.meta;
  plant.format_version = config.format_version;
  plant.state = config.state.value_or(PlantState{});
  auto& element_container = plant.element_container;

  // add elements
  for (const auto& element : config.elements) {
    auto parser = ParserFactory::create(element.type);
    parser->parse(element_container, element);
  }

  // connect elements
  for (const auto& connection : config.connections) {
    auto from_elem =
        element_container.get_element_by_id(connection.from.element_id);
    auto to_elem =
        element_container.get_element_by_id(connection.to.element_id);
    if (from_elem && to_elem) {
      from_elem->connect_to(to_elem, connection.from.port_type,
                            connection.to.port_type);
    }
  }

  return PlantRepositoryResult::Ok;
}

PlantRepositoryResult PlantConfigRepository::save(
    const std::filesystem::path& path, const PlantData& plant) {
  std::ofstream file(path);
  if (!file.is_open()) {
    return PlantRepositoryResult::Error;
  }

  nlohmann::json json = to_plant_configuration(plant);
  file << json.dump(4);
  file.close();
  return file.fail() ? PlantRepositoryResult::Error : PlantRepositoryResult::Ok;
}

std::vector<ElementConfig> to_element_configs(
    const std::vector<Element*>& elements) {
  std::vector<ElementConfig> configs;
  for (const auto& element : elements) {
    ElementConfig config;
    config.id = element->get_ID();
    config.name = element->get_name();
    config.type = element->get_type();
    config.parameters = element->get_parameters();
    config.state = element->get_state();
    configs.push_back(config);
  }

  return configs;
}

std::vector<ConnectionConfig> to_connection_configs(
    const std::vector<Element*>& elements) {
  std::vector<ConnectionConfig> configs;

  std::unordered_map<ElementID, std::unordered_set<PortType>> seen;

  for (const auto& element : elements) {
    for (const auto& port : element->get_ports()) {
      if (!port || !port->is_connected()) {
        continue;
      }
      if (seen[element->get_ID()].contains(port->get_port_type())) {
        continue;
      }

      ConnectionConfig config;

      config.from = {.port_type = port->get_port_type(),
                     .element_id = element->get_ID()};

      config.to = {.port_type = port->m_connected_to->get_port_type(),
                   .element_id = port->m_connected_to->m_owner.get_ID()};

      configs.push_back(config);
      seen[config.to.element_id].insert(config.to.port_type);
    }
  }
  return configs;
}

PlantConfiguration to_plant_configuration(const PlantData& plant) {
  PlantConfiguration config{};
  config.meta = plant.meta;
  config.format_version = plant.format_version;
  config.simulation = plant.config;
  config.state = std::make_optional(plant.state);
  config.elements = to_element_configs(plant.element_container.get_elements());
  config.connections =
      to_connection_configs(plant.element_container.get_elements());
  return config;
}

}  // namespace lvtrans
