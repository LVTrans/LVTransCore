// #include <gtest/gtest.h>
// #include <fstream>
// #include "../test_helpers.hpp"
// #include "lvtrans/config/plant_configuration.hpp"

// namespace {
// using namespace lvtrans;
// using nlohmann::json;

// json read_fixture() {
//   std::ifstream file(get_mock_data_file_path("plant_config_1.json"));
//   return json::parse(file);
// }

// TEST(PlantConfigurationTest, LoadsFixtureAndRoundTripsRepresentedFields) {
//   auto input = read_fixture();
//   const auto config = input.get<PlantConfiguration>();
//   EXPECT_EQ(config.format_version, 1);
//   EXPECT_EQ(config.meta.name, "Plant 1");
//   EXPECT_DOUBLE_EQ(config.simulation.dt, 0.1);
//   ASSERT_TRUE(config.state.has_value());
//   EXPECT_DOUBLE_EQ(config.state->step_size, 0.1);
//   ASSERT_EQ(config.elements.size(), 3u);
//   EXPECT_EQ(config.elements[0].type, ElementType::Reservoir);
//   EXPECT_TRUE(
//       std::holds_alternative<ReservoirConfig>(config.elements[0].parameters));
//   EXPECT_FALSE(config.elements[0].state.has_value());
//   EXPECT_EQ(config.elements[1].type, ElementType::Pipe);
//   EXPECT_TRUE(
//       std::holds_alternative<PipeConfig>(config.elements[1].parameters));
//   ASSERT_TRUE(config.elements[1].state.has_value());
//   EXPECT_TRUE(std::holds_alternative<PipeState>(*config.elements[1].state));
//   EXPECT_EQ(config.elements[2].type, ElementType::Valve);
//   EXPECT_TRUE(
//       std::holds_alternative<ValveConfig>(config.elements[2].parameters));
//   ASSERT_TRUE(config.elements[2].state.has_value());
//   EXPECT_TRUE(std::holds_alternative<ValveState>(*config.elements[2].state));

//   // CdA0 is not represented by ValveConfig. Optional inherited losses
//   default
//   // to zero, but are explicitly written on serialization.
//   input["elements"][2]["parameters"].erase("CdA0");
//   input["elements"][2]["parameters"]["Cvp"] = 0.0;
//   input["elements"][2]["parameters"]["Cvm"] = 0.0;
//   const json output = config;
//   EXPECT_EQ(output, input);
//   EXPECT_EQ(json(output.get<PlantConfiguration>()), output);
// }

// TEST(PlantConfigurationTest, ClearsMissingOrNullOptionalStatesOnReload) {
//   for (bool use_null : {false, true}) {
//     SCOPED_TRACE(use_null);
//     auto input = read_fixture();
//     auto config = input.get<PlantConfiguration>();
//     input.erase("plant_state");
//     if (use_null) input["plant_state"] = nullptr;
//     for (auto& element : input["elements"]) {
//       element.erase("state");
//       if (use_null) element["state"] = nullptr;
//     }
//     input.get_to(config);
//     EXPECT_FALSE(config.state.has_value());
//     for (const auto& element : config.elements) {
//       EXPECT_FALSE(element.state.has_value());
//     }
//     const json output = config;
//     EXPECT_FALSE(output.contains("plant_state"));
//     for (const auto& element : output["elements"]) {
//       EXPECT_FALSE(element.contains("state"));
//     }
//   }
// }

// TEST(PlantConfigurationTest, KeepsValveAndInheritedLossCoefficientsDistinct)
// {
//   auto input = read_fixture();
//   auto& parameters = input["elements"][2]["parameters"];
//   parameters["Cvp"] = 1.5;
//   parameters["Cvm"] = 2.5;
//   parameters["cvp"] = 3.5;
//   const auto config = input.get<PlantConfiguration>();
//   const auto& valve = std::get<ValveConfig>(config.elements[2].parameters);
//   EXPECT_DOUBLE_EQ(valve.LossCoefficients::cvp, 1.5);
//   EXPECT_DOUBLE_EQ(valve.cvm, 2.5);
//   EXPECT_DOUBLE_EQ(valve.cvp, 3.5);
//   const json output = valve;
//   parameters.erase("CdA0");
//   EXPECT_EQ(output, parameters);
// }

// TEST(PlantConfigurationTest, TakesTimeStepFromSimulationSettings) {
//   auto input = read_fixture();
//   input["simulation"]["dt"] = 0.025;
//   const auto config = input.get<PlantConfiguration>();
//   ASSERT_TRUE(config.state.has_value());
//   EXPECT_DOUBLE_EQ(config.state->step_size, 0.025);
//   const json output = config;
//   EXPECT_FALSE(output["plant_state"].contains("step_size"));
// }

// TEST(PlantConfigurationTest, RejectsUnknownTagsAndMissingRequiredFields) {
//   auto input = read_fixture();
//   input["elements"][1]["type"] = "UnknownPipe";
//   EXPECT_THROW(input.get<PlantConfiguration>(), std::invalid_argument);
//   input = read_fixture();
//   input["connections"][0]["from"]["port"] = "unknown";
//   EXPECT_THROW(input.get<PlantConfiguration>(), std::invalid_argument);
//   input = read_fixture();
//   input["elements"][1]["parameters"].erase("length");
//   EXPECT_THROW(input.get<PlantConfiguration>(), json::out_of_range);
// }

// TEST(PlantConfigurationTest, RejectsStateOrParametersOfTheWrongElementType) {
//   auto input = read_fixture();
//   input["elements"][1]["state"] = {{"tau", 0.5}};
//   EXPECT_THROW(input.get<PlantConfiguration>(), json::out_of_range);
//   input = read_fixture();
//   input["elements"][0]["state"] = {{"tau", 0.5}};
//   EXPECT_THROW(input.get<PlantConfiguration>(), std::invalid_argument);

//   auto config = read_fixture().get<PlantConfiguration>();
//   config.elements[1].parameters = ValveConfig{};
//   EXPECT_THROW(static_cast<void>(json(config)), std::bad_variant_access);
//   config = read_fixture().get<PlantConfiguration>();
//   config.elements[1].state = ValveState{};
//   EXPECT_THROW(static_cast<void>(json(config)), std::bad_variant_access);
// }
// }  // namespace
