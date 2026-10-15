#include "lvtrans/config/plant_config_repository.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include "lvtrans/config/element_parsers.hpp"
#include "lvtrans/config/plant_configuration.hpp"
#include "lvtrans/plant.hpp"

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
  } catch (std::exception& ex) {
    std::cout << "Failed to parse file: " << ex.what() << '\n';
    return PlantRepositoryResult::Error;
  }

  plant.config = config.simulation;
  plant.meta = config.meta;
  plant.state = config.state.value_or(PlantState{});
  auto& element_container = plant.element_container;

  // add elements
  for (auto& element : config.elements) {
    auto parser = ParserFactory::create(element.type);
    if (parser->parse(element_container, element, plant.config.step_size) ==
        ElementParseResult::Error) {
      std::cout << "Failed to parse element: " << element.name << '\n';
      return PlantRepositoryResult::Error;
    }
  }

  // Connect only opposite sides of a pipe and a hydraulic non-pipe.
  for (const auto& connection : config.connections) {
    const Connection* pipe_end = &connection.from;
    const Connection* boundary = &connection.to;
    auto* pipe = element_container.get_element_by_id<BasePipe>(pipe_end->element_id);
    if (!pipe) {
      std::swap(pipe_end, boundary);
      pipe = element_container.get_element_by_id<BasePipe>(pipe_end->element_id);
    }
    auto* non_pipe = element_container.get_element_by_id<NonPipe>(boundary->element_id);
    if (!pipe || !non_pipe || pipe_end->index != 0 ||
        pipe_end->port_type == boundary->port_type)
      return PlantRepositoryResult::Error;
    const auto end = pipe_end->port_type == Side::Left ? PipeEnd::Left : PipeEnd::Right;
    if (HydraulicConnections::connect(*pipe, end, *non_pipe, boundary->index) != ConnectResult::Ok)
      return PlantRepositoryResult::Error;
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
    configs.emplace_back(config);
  }

  return configs;
}

std::vector<ConnectionConfig> to_connection_configs(
    const std::vector<Element*>& elements) {
  std::vector<ConnectionConfig> configs;
  // Each hydraulic connection belongs to exactly one pipe endpoint.
  for (const auto* element : elements) {
    const auto* pipe = dynamic_cast<const BasePipe*>(element);
    if (!pipe) continue;
    if (const auto* peer = pipe->left_elem()) {
      configs.push_back({{Side::Left, pipe->get_ID()},
                         {Side::Right, peer->get_ID(), pipe->left_link().terminal.index}});
    }
    if (const auto* peer = pipe->right_elem()) {
      configs.push_back({{Side::Right, pipe->get_ID()},
                         {Side::Left, peer->get_ID(), pipe->right_link().terminal.index}});
    }
  }

  return configs;
}

PlantConfiguration to_plant_configuration(const PlantData& plant) {
  PlantConfiguration config{};
  config.meta = plant.meta;
  config.format_version = 1;
  config.simulation = plant.config;
  config.state = std::make_optional(plant.state);
  config.elements = to_element_configs(plant.element_container.get_elements());
  config.connections =
      to_connection_configs(plant.element_container.get_elements());
  return config;
}

}  // namespace lvtrans
