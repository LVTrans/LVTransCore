#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include "lvtrans/element.hpp"
#include "lvtrans/pipe.hpp"
#include "lvtrans/plant_types.hpp"
#include "lvtrans/reservoir.hpp"
#include "lvtrans/valve.hpp"
#include "nlohmann/json.hpp"
namespace lvtrans {

enum class ElementType : std::uint8_t { Pipe, Reservoir, Valve };
using ParameterValue = std::variant<double, std::string>;
struct PlantMetaData {
  std::string name;
};

using ElementState = std::variant<ValveState, PipeState>;
using ElementParameters =
    std::variant<ValveParameters, PipeParameters, ReservoirParameters>;

struct ElementConfig {
  int id;
  std::string name;
  ElementType type;
  ElementParameters parameters;
  std::optional<ElementState> state;
};

struct ConnectionConfig {
  struct Connection {
    PortType port;
    ElementID element;
  };

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
  int format_version;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantMetaData, name)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimulationConfig, step_size)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PipeParameters, length, diameter, f, a, z0,
                                   z1, lambda, f_max, num_reaches, use_diameter,
                                   use_full_moody)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReservoirParameters, H0)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PipeState, H, Q)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ValveState, tau)
// The time step is stored once, in simulation.step_size.
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantState, current_time, num_iterations)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LossCoefficients, cvp, cvm)

NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(ValveParameters, LossCoefficients,
                                           tau_i, tau_f, tc, em)

NLOHMANN_JSON_SERIALIZE_ENUM(ElementType,
                             {{ElementType::Pipe, "Pipe"},
                              {ElementType::Pipe, "NormalPipe"},
                              {ElementType::Reservoir, "Reservoir"},
                              {ElementType::Valve, "Valve"}})

NLOHMANN_JSON_SERIALIZE_ENUM(PortType, {{PortType::Left, "left"},
                                        {PortType::Right, "right"},
                                        {PortType::Up, "up"},
                                        {PortType::Down, "down"}})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConnectionConfig::Connection, port, element)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConnectionConfig, from, to)

inline void to_json(nlohmann::json& j, const ElementConfig& value) {
  j = {{"id", value.id}, {"name", value.name}, {"type", value.type}};
  switch (value.type) {
    case ElementType::Pipe:
      j["parameters"] = std::get<PipeParameters>(value.parameters);
      if (value.state) j["state"] = std::get<PipeState>(*value.state);
      break;
    case ElementType::Valve:
      j["parameters"] = std::get<ValveParameters>(value.parameters);
      if (value.state) j["state"] = std::get<ValveState>(*value.state);
      break;
    case ElementType::Reservoir:
      j["parameters"] = std::get<ReservoirParameters>(value.parameters);
      break;
  }
}

inline void from_json(const nlohmann::json& j, ElementConfig& value) {
  j.at("id").get_to(value.id);
  j.at("name").get_to(value.name);
  j.at("type").get_to(value.type);
  value.state.reset();
  const bool has_state = j.contains("state") && !j.at("state").is_null();
  switch (value.type) {
    case ElementType::Pipe:
      value.parameters = j.at("parameters").get<PipeParameters>();
      if (has_state) value.state = j.at("state").get<PipeState>();
      break;
    case ElementType::Valve:
      value.parameters = j.at("parameters").get<ValveParameters>();
      if (has_state) value.state = j.at("state").get<ValveState>();
      break;
    case ElementType::Reservoir:
      value.parameters = j.at("parameters").get<ReservoirParameters>();
      break;
  }
}

inline void to_json(nlohmann::json& j, const PlantConfiguration& value) {
  j = {{"plant_meta", value.meta},
       {"simulation", value.simulation},
       {"elements", value.elements},
       {"connections", value.connections},
       {"format_version", value.format_version}};
  if (value.state) j["plant_state"] = *value.state;
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
