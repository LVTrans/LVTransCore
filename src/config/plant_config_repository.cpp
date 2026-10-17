#include "lvtrans/config/plant_config_repository.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include "lvtrans/config/element_parsers.hpp"
#include "lvtrans/config/plant_configuration.hpp"
#include "lvtrans/elements/controllable.hpp"
#include "lvtrans/elements/controller.hpp"
#include "lvtrans/elements/non-pipe.hpp"
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
  std::cout << "Loading plant configuration from: " << path << '\n';

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
  auto& container = plant.element_container;

  // Add elements
  for (auto& element : config.elements) {
    auto parser = ParserFactory::create(element.type);

    if (!parser || parser->parse(container, element, plant.config.step_size) ==
                       ElementParseResult::Error) {
      return PlantRepositoryResult::Error;
    }
  }

  // Connect elements
  for (const auto& connection : config.connections) {
    const auto* from = &connection.from;
    const auto* to = &connection.to;
    // case 1 pipe to non-pipe connection
    if (from->port_type == ConnectionPort::Left ||
        from->port_type == ConnectionPort::Right) {
      // Pipe are always 'from'
      auto* pipe = container.get_element_by_id<BasePipe>(from->element_id);
      auto* non_pipe = container.get_element_by_id<NonPipe>(to->element_id);

      if (!pipe || !non_pipe) {
        continue;
      }

      auto end = from->port_type == ConnectionPort::Left ? PipeEnd::Left
                                                         : PipeEnd::Right;

      if (HydraulicConnections::connect(*pipe, end, *non_pipe, to->index) !=
          ConnectResult::Ok) {
        return PlantRepositoryResult::Error;
      }
    }

    // case 2 non-pipe to controller connection
    if (from->port_type == ConnectionPort::Controller &&
        to->port_type == ConnectionPort::Controllable) {
      auto* controllable =
          container.get_element_by_id<Controllable>(from->element_id);
      auto* controller =
          container.get_element_by_id<Controller>(to->element_id);

      if (!controller || !controllable) {
        continue;
      }

      controllable->set_controller(controller);
      controller->set_controllable(controllable);
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
    configs.emplace_back(config);
  }

  return configs;
}

std::vector<ConnectionConfig> to_connection_configs(
    const std::vector<Element*>& elements) {
  std::vector<ConnectionConfig> configs;

  for (const auto* element : elements) {
    // pipes
    if (const auto* pipe = dynamic_cast<const BasePipe*>(element)) {
      if (const auto* peer = pipe->left_elem()) {
        configs.push_back({{ConnectionPort::Left, pipe->get_ID()},
                           {ConnectionPort::Right, peer->get_ID(),
                            pipe->left_link().terminal.index}});
      }
      if (const auto* peer = pipe->right_elem()) {
        configs.push_back({{ConnectionPort::Right, pipe->get_ID()},
                           {ConnectionPort::Left, peer->get_ID(),
                            pipe->right_link().terminal.index}});
      }
    }

    // non-pipes
    if (const auto* controllable = dynamic_cast<const Controllable*>(element)) {
      if (const auto* controller = controllable->get_controller()) {
        configs.push_back(
            {{ConnectionPort::Controller, element->get_ID()},
             {ConnectionPort::Controllable, controller->get_ID()}});
      }
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
