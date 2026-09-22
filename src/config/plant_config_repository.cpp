#include "lvtrans/config/plant_config_repository.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include "lvtrans/config/element_parsers.hpp"
#include "lvtrans/config/plant_configuration.hpp"
#include "lvtrans/plant.hpp"
#include "nlohmann/json.hpp"

namespace lvtrans {

PlantRepositoryResult PlantConfigRepository::load(
    const std::filesystem::path& path, PlantData& plant) {
  using namespace nlohmann;
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cout << "Failed to open file: " << path << '\n';
    return PlantRepositoryResult::Error;
  }
  json json;
  PlantConfiguration config;
  try {
    json = json::parse(file);
    config = json.get<PlantConfiguration>();
  } catch (json::parse_error& ex) {
    std::cerr << "parse error at byte " << ex.byte << '\n';
    return PlantRepositoryResult::Error;
  }

  plant.config = config.simulation;
  plant.state = config.state.value_or(PlantState{});
  auto& element_container = plant.element_container;

  // add elements
  for (const auto& element : config.elements) {
    std::cout << "Adding element: " << element.name << '\n';
    // 1. extract concrete type
    // 2. once you know, then add initialize it with corresponding parameters
    // etc.
    auto parser = ParserFactory::create(element.type);
    parser->parse(element_container, element);
  }
  // connect elements
  for (const auto& connection : config.connections) {
    auto from_elem =
        element_container.get_element_by_id(connection.from.element);
    auto to_elem = element_container.get_element_by_id(connection.to.element);
    if (from_elem && to_elem) {
      from_elem->connect_to(to_elem, connection.from.port, connection.to.port);
    }
  }

  return PlantRepositoryResult::Ok;
}

PlantRepositoryResult PlantConfigRepository::save(
    const std::filesystem::path& path, const PlantData&) {
  std::ofstream file(path);
  if (!file.is_open()) {
    std::cout << "Failed to open file: " << path << '\n';
    return PlantRepositoryResult::Error;
  }
  // nlohmann::json json = plant;
  // file << json;
  return PlantRepositoryResult::Ok;
}

}  // namespace lvtrans
