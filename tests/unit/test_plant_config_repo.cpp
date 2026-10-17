#include <gtest/gtest.h>
#include <array>
#include <fstream>
#include <utility>
#include <vector>
#include "../test_helpers.hpp"
#include "lvtrans/config/plant_config_repository.hpp"
#include "lvtrans/config/plant_configuration.hpp"
#include "lvtrans/element_parameters.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/pid_turbine.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/surge_shaft_standard.hpp"
#include "lvtrans/elements/t_1in_2out_n.hpp"
#include "lvtrans/elements/valve.hpp"
#include "lvtrans/plant.hpp"
#include "nlohmann/json.hpp"

TEST(ElementConfigTest, DeserializesVariantsAndClearsAbsentState) {
  using namespace lvtrans;
  const std::vector<ElementConfig> configs{
      {1, "pipe", ElementType::Pipe, PipeParameters{.length = 600.0},
       PipeState{{10.0, 20.0}, {1.0, 2.0}}},
      {2, "algebraic", ElementType::AlgebraicPipe,
       PipeParameters{.length = 300.0}, PipeState{{30.0}, {3.0}}},
      {3, "valve", ElementType::Valve, ValveParameters{.tau_i = 0.5},
       ValveState{0.75}},
      {4, "reservoir", ElementType::Reservoir, ReservoirParameters{.H0 = 150.0},
       std::nullopt},
      {5, "left", ElementType::ConstantLevelLeft,
       ReservoirParameters{.H0 = 160.0}, std::nullopt},
      {6, "right", ElementType::ConstantLevelRight,
       ReservoirParameters{.H0 = 170.0}, std::nullopt},
  };
  ElementConfig loaded{};
  for (const auto& config : configs) {
    SCOPED_TRACE(config.name);
    const nlohmann::json encoded = config;
    encoded.get_to(loaded);
    EXPECT_EQ(loaded.type, config.type);
    EXPECT_EQ(loaded.parameters.index(), config.parameters.index());
    ASSERT_EQ(loaded.state.has_value(), config.state.has_value());
    if (config.state) {
      EXPECT_EQ(loaded.state->index(), config.state->index());
    }
    EXPECT_EQ(nlohmann::json(loaded), encoded);

    auto without_state = encoded;
    without_state.erase("state");
    without_state.get_to(loaded);
    EXPECT_FALSE(loaded.state.has_value());

    encoded.get_to(loaded);
    without_state["state"] = nullptr;
    without_state.get_to(loaded);
    EXPECT_FALSE(loaded.state.has_value());
  }
}

TEST(PlantConfigRepositoryTest, LoadAndSavePlantConfig) {
  using namespace lvtrans;

  const auto file_path =
      get_mock_data_file_path("expected/plant_config_1.json");
  PlantData plant_data;

  ASSERT_EQ(PlantConfigRepository::load(file_path, plant_data),
            PlantRepositoryResult::Ok);

  EXPECT_DOUBLE_EQ(plant_data.config.step_size, 0.5);
  EXPECT_DOUBLE_EQ(plant_data.state.current_time, 10.0);
  EXPECT_EQ(plant_data.state.num_iterations, 2);
  EXPECT_EQ(plant_data.meta.name, "Plant 1");

  auto& elements = plant_data.element_container;
  ASSERT_EQ(elements.get_elements().size(), 3u);
  ASSERT_EQ(elements.get_pipes().size(), 1u);
  ASSERT_EQ(elements.get_non_pipes().size(), 2u);

  auto* reservoir = elements.get_element_by_id<Reservoir>(1);
  auto* pipe = elements.get_element_by_id<Pipe>(2);
  auto* valve = elements.get_element_by_id<Valve>(3);
  ASSERT_NE(reservoir, nullptr);
  ASSERT_NE(pipe, nullptr);
  ASSERT_NE(valve, nullptr);
  EXPECT_EQ(reservoir->get_ID(), 1);
  EXPECT_EQ(pipe->get_ID(), 2);
  EXPECT_EQ(valve->get_ID(), 3);

  // EXPECT_DOUBLE_EQ(reservoir->get_H(), 100.0);

  const auto& config = pipe->config();
  EXPECT_DOUBLE_EQ(config.length, 100.0);
  EXPECT_DOUBLE_EQ(config.diameter, 1.0);
  EXPECT_DOUBLE_EQ(config.f, 1.0);
  EXPECT_DOUBLE_EQ(config.a, 1.0);
  EXPECT_DOUBLE_EQ(config.z0, 0.0);
  EXPECT_DOUBLE_EQ(config.z1, 100.0);
  EXPECT_DOUBLE_EQ(config.lambda, 0.0);
  EXPECT_DOUBLE_EQ(config.f_max, 0.0);
  EXPECT_FALSE(config.use_diameter);
  EXPECT_FALSE(config.use_full_moody);

  const std::vector<double> expected_heads{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  const std::vector<double> expected_flows{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  EXPECT_EQ(pipe->get_H(), expected_heads);
  EXPECT_EQ(pipe->get_Q(), expected_flows);
  EXPECT_DOUBLE_EQ(valve->get_tau(), 1.0);

  ASSERT_EQ(pipe->left_elem(), reservoir);
  ASSERT_EQ(pipe->right_elem(), valve);

  ASSERT_EQ(reservoir->right_links().size(), 1u);
  EXPECT_EQ(reservoir->right_links()[0].pipe, pipe);
  ASSERT_EQ(valve->left_links().size(), 1u);
  EXPECT_EQ(valve->left_links()[0].pipe, pipe);

  const auto generated_path =
      get_mock_data_file_path("generated/plant_config_1.json");
  ASSERT_EQ(PlantConfigRepository::save(generated_path, plant_data),
            PlantRepositoryResult::Ok);

  std::ifstream expected_file(file_path);
  std::ifstream generated_file(generated_path);
  ASSERT_TRUE(expected_file.is_open());
  ASSERT_TRUE(generated_file.is_open());
  const auto expected = nlohmann::json::parse(expected_file);
  const auto generated = nlohmann::json::parse(generated_file);
  EXPECT_EQ(generated, expected);
}

// Layout:
// Main supply:
//  R101 — P410 — V205 — P720 — V315 — P160 — V605
//
// Auxiliary supply:
//  R900 — P230 — V840 — P550 — V120
//
// Circulation loop:
//  P680 — V350 — P470 — V790 — P260 — V630 — back to P680
TEST(PlantConfigRepositoryTest, ComplexLayoutSurvivesRoundTrip) {
  using namespace lvtrans;
  PlantData loaded, reloaded;
  ASSERT_EQ(
      PlantConfigRepository::load(
          get_mock_data_file_path("expected/plant_config_2.json"), loaded),
      PlantRepositoryResult::Ok);
  const auto saved_path =
      get_mock_data_file_path("generated/plant_config_2.json");
  ASSERT_EQ(PlantConfigRepository::save(saved_path, loaded),
            PlantRepositoryResult::Ok);
  ASSERT_EQ(PlantConfigRepository::load(saved_path, reloaded),
            PlantRepositoryResult::Ok);

  // {pipe ID, left neighbor ID, right neighbor ID}:
  const std::array<std::array<ElementID, 3>, 8> neighbors{{
      {410, 101, 205},
      {720, 205, 315},
      {160, 315, 605},
      {230, 900, 840},
      {550, 840, 120},
      {680, 630, 350},
      {470, 350, 790},
      {260, 790, 630},
  }};
  for (auto* plant_data : {&loaded, &reloaded}) {
    auto& elements = plant_data->element_container;
    ASSERT_EQ(elements.get_elements().size(), 18u);
    ASSERT_EQ(elements.get_pipes().size(), 8u);
    ASSERT_EQ(elements.get_non_pipes().size(), 10u);

    for (const auto& [id, left, right] : neighbors) {
      auto* pipe = elements.get_element_by_id<Pipe>(id);
      ASSERT_NE(pipe, nullptr);
      ASSERT_NE(pipe->left_elem(), nullptr);
      ASSERT_NE(pipe->right_elem(), nullptr);
      EXPECT_EQ(pipe->left_elem()->get_ID(), left);
      EXPECT_EQ(pipe->right_elem()->get_ID(), right);
    }
  }
  std::ifstream saved_file(saved_path);
  ASSERT_TRUE(saved_file.is_open());
  EXPECT_EQ(nlohmann::json::parse(saved_file).at("connections").size(), 16u);
  Plant plant(std::move(loaded));
}

TEST(ConnectionConfigTest, PreservesIndicesAndRejectsNonHydraulicSides) {
  using namespace lvtrans;
  Connection connection{ConnectionPort::Right, 42, 3};
  const nlohmann::json encoded = connection;
  EXPECT_EQ(encoded.at("index"), 3);
  const auto decoded = encoded.get<Connection>();
  EXPECT_EQ(decoded.port_type, ConnectionPort::Right);
  EXPECT_EQ(decoded.element_id, 42);
  EXPECT_EQ(decoded.index, 3u);
}

TEST(PlantConfigRepositoryTest, PreservesDownstreamReservoirConnections) {
  using namespace lvtrans;
  PlantData data;
  data.config.step_size = 0.1;
  auto* pipe =
      data.element_container.add_element<Pipe>(PipeParameters{}, 0.1).value();
  auto* reservoir =
      data.element_container.add_element<ConstantLevelRight>(100.0).value();
  ASSERT_EQ(HydraulicConnections::connect(*pipe, PipeEnd::Right, *reservoir),
            ConnectResult::Ok);
  const auto path = get_mock_data_file_path("generated/right_reservoir.json");
  ASSERT_EQ(PlantConfigRepository::save(path, data), PlantRepositoryResult::Ok);
  PlantData loaded;
  ASSERT_EQ(PlantConfigRepository::load(path, loaded),
            PlantRepositoryResult::Ok);
  auto* loaded_pipe =
      loaded.element_container.get_element_by_id<Pipe>(pipe->get_ID());
  auto* loaded_reservoir =
      loaded.element_container.get_element_by_id<ConstantLevelRight>(
          reservoir->get_ID());
  ASSERT_NE(loaded_pipe, nullptr);
  ASSERT_NE(loaded_reservoir, nullptr);
  EXPECT_EQ(loaded_pipe->right_elem(), loaded_reservoir);
  EXPECT_EQ(loaded_reservoir->left_links()[0].pipe, loaded_pipe);
  EXPECT_TRUE(loaded_reservoir->right_links().empty());
}

TEST(PlantConfigRepositoryTest, ComplexMultiElementTopology) {
  using namespace lvtrans;
  PlantData loaded, reloaded;
  const auto path = get_mock_data_file_path("expected/plant_config_3.json");
  const auto saved_path =
      get_mock_data_file_path("generated/plant_config_3.json");
  // R2-P1-SC4-P7-V3-P8-T5-P10-R12; P9-S6-P11.
  ASSERT_EQ(PlantConfigRepository::load(path, loaded),
            PlantRepositoryResult::Ok);
  ASSERT_EQ(PlantConfigRepository::save(saved_path, loaded),
            PlantRepositoryResult::Ok);
  ASSERT_EQ(PlantConfigRepository::load(saved_path, reloaded),
            PlantRepositoryResult::Ok);

  // pipe elements neighbors. -1 means no connection.
  const std::array<std::array<ElementID, 3>, 6> neighbors{{
      {1, 2, 4},
      {7, 4, 3},
      {8, 3, 5},
      {9, -1, 6},
      {10, 5, 12},
      {11, 6, -1},
  }};
  for (auto* data : {&loaded, &reloaded}) {
    SCOPED_TRACE(data == &loaded ? "initial load" : "saved and reloaded");
    auto& elements = data->element_container;
    ASSERT_EQ(elements.get_elements().size(), 13u);
    ASSERT_NE(elements.get_element_by_id<SimpleConnection>(4), nullptr);
    ASSERT_NE(elements.get_element_by_id<T1In2OutN>(5), nullptr);
    ASSERT_NE(elements.get_element_by_id<SurgeShaftStandard>(6), nullptr);
    for (const auto& [id, left, right] : neighbors) {
      const auto* pipe = elements.get_element_by_id<Pipe>(id);
      ASSERT_NE(pipe, nullptr);

      if (left < 0) {
        EXPECT_EQ(pipe->left_elem(), nullptr);
      } else {
        ASSERT_NE(pipe->left_elem(), nullptr);
        EXPECT_EQ(pipe->left_elem()->get_ID(), left);
      }

      if (right < 0) {
        EXPECT_EQ(pipe->right_elem(), nullptr);
      } else {
        ASSERT_NE(pipe->right_elem(), nullptr);
        EXPECT_EQ(pipe->right_elem()->get_ID(), right);
      }
    }
    // test T junction
    const auto* junction = elements.get_element_by_id<T1In2OutN>(5);
    ASSERT_EQ(junction->left_links().size(), 1u);
    ASSERT_EQ(junction->right_links().size(), 2u);
    EXPECT_EQ(junction->right_links()[0].pipe,
              elements.get_element_by_id<Pipe>(10));
    EXPECT_EQ(junction->right_links()[1].pipe, nullptr);

    // Test Valve's PID
    auto* pid_turbine = elements.get_element_by_id<PIDTurbine>(13);
    auto* valve = elements.get_element_by_id<Valve>(3);
    EXPECT_EQ(valve->get_controller(), pid_turbine);
    EXPECT_EQ(pid_turbine->get_controllable(), valve);
  }
}
