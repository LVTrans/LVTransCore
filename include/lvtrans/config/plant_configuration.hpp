#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include "lvtrans/element_types.hpp"
#include "lvtrans/plant_types.hpp"
#include "lvtrans/port.hpp"
#include "nlohmann/detail/macro_scope.hpp"
#include "nlohmann/json.hpp"
namespace lvtrans {

using ParameterValue = std::variant<double, std::string>;
struct ElementConfig {
  ElementID id;
  std::string name;
  ElementType type;
  ElementParameters parameters;
  std::optional<ElementState> state;
};

struct Connection {
  PortType port_type;
  ElementID element_id;
};

struct ConnectionConfig {
  Connection from;
  Connection to;
};

enum class PlantConfigFormatVersion : std::uint8_t { V1 };

struct PlantConfiguration {
  PlantMetaData meta;
  SimulationConfig simulation;
  std::vector<ElementConfig> elements;
  std::vector<ConnectionConfig> connections;
  std::optional<PlantState> state;
  int format_version{1};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantMetaData, name)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimulationConfig, step_size)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    PipeParameters, length, diameter, area, periphery, epsilon, ny, rho, f, a,
    z0, z1, lambda, f_max, dimension, use_diameter, use_full_moody, use_Dh)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PipeState, H, Q)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ValveState, tau)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantState, current_time, num_iterations)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ReservoirParameters, H0, cvp,
                                                cvm)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ValveParameters, tau_i, tau_f,
                                                tc, em, cda0, cvp, cvm)
NLOHMANN_JSON_SERIALIZE_ENUM(PipeDimension,
                             {{PipeDimension::Circular, "Circular"},
                              {PipeDimension::CrossSectional,
                               "CrossSectional"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
    ElementType, {{ElementType::Pipe, "NormalPipe"},
                  {ElementType::Pipe, "Pipe"},
                  {ElementType::Reservoir, "Reservoir"},
                  {ElementType::Valve, "Valve"},
                  {ElementType::ConstantLevelLeft, "ConstantLevelLeft"},
                  {ElementType::ConstantLevelRight, "ConstantLevelRight"},
                  {ElementType::AlgebraicPipe, "AlgebraicPipe"}})

NLOHMANN_JSON_SERIALIZE_ENUM(PortType, {{PortType::Left, "left"},
                                        {PortType::Right, "right"},
                                        {PortType::Up, "up"},
                                        {PortType::Down, "down"}})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Connection, port_type, element_id)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConnectionConfig, from, to)

inline void to_json(nlohmann::json& j, const ElementConfig& value) {
  j = {{"id", value.id}, {"name", value.name}, {"type", value.type}};
  std::visit([&](const auto& parameters) { j["parameters"] = parameters; },
             value.parameters);
  if (value.state) {
    std::visit([&](const auto& state) { j["state"] = state; }, *value.state);
  }
}

inline void from_json(const nlohmann::json& j, ElementConfig& value) {
  j.at("id").get_to(value.id);
  j.at("name").get_to(value.name);
  j.at("type").get_to(value.type);
  using ElementDefaults =
      std::pair<ElementParameters, std::optional<ElementState>>;
  static const std::unordered_map<ElementType, ElementDefaults> defaults{
      {ElementType::Pipe, {PipeParameters{}, PipeState{}}},
      {ElementType::AlgebraicPipe, {PipeParameters{}, PipeState{}}},
      {ElementType::Valve, {ValveParameters{}, ValveState{}}},
      {ElementType::Reservoir, {ReservoirParameters{}, std::nullopt}},
      {ElementType::ConstantLevelLeft, {ReservoirParameters{}, std::nullopt}},
      {ElementType::ConstantLevelRight, {ReservoirParameters{}, std::nullopt}},
  };

  const auto& [parameters, state] = defaults.at(value.type);
  value.parameters = parameters;
  value.state.reset();
  std::visit(
      [&](auto& parameter_values) {
        j.at("parameters").get_to(parameter_values);
      },
      value.parameters);
  if (state && j.contains("state") && !j.at("state").is_null()) {
    value.state = state;
    std::visit([&](auto& state_values) { j.at("state").get_to(state_values); },
               *value.state);
  }
}

inline void to_json(nlohmann::json& j, const PlantConfiguration& value) {
  j = {{"plant_meta", value.meta},
       {"simulation", value.simulation},
       {"elements", value.elements},
       {"connections", value.connections},
       {"format_version", value.format_version}};
  if (value.state) {
    j["plant_state"] = *value.state;
  }
}

inline void from_json(const nlohmann::json& j, PlantConfiguration& value) {
  j.at("plant_meta").get_to(value.meta);
  j.at("simulation").get_to(value.simulation);
  j.at("elements").get_to(value.elements);
  j.at("connections").get_to(value.connections);
  j.at("format_version").get_to(value.format_version);
  value.state.reset();
  if (j.contains("plant_state") && !j.at("plant_state").is_null()) {
    value.state = j.at("plant_state").get<PlantState>();
  }
}
}  // namespace lvtrans
