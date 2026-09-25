#include <gtest/gtest.h>
#include <array>
#include <fstream>
#include <vector>
#include "../test_helpers.hpp"
#include "lvtrans/config/plant_config_repository.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/elements/valve.hpp"
#include "lvtrans/plant.hpp"
#include "nlohmann/json.hpp"

TEST(PlantConfigRepositoryTest, LoadAndSavePlantConfig) {
  using namespace lvtrans;

  const auto file_path =
      get_mock_data_file_path("expected/plant_config_1_expected.json");
  PlantData plant_data;

  ASSERT_EQ(PlantConfigRepository::load(file_path, plant_data),
            PlantRepositoryResult::Ok);

  EXPECT_DOUBLE_EQ(plant_data.config.step_size, 0.5);
  EXPECT_DOUBLE_EQ(plant_data.state.current_time, 10.0);
  EXPECT_EQ(plant_data.state.num_iterations, 2);
  EXPECT_EQ(plant_data.meta.name, "Plant 1");
  EXPECT_EQ(plant_data.format_version, 1);

  auto& elements = plant_data.element_container;
  ASSERT_EQ(elements.get_elements().size(), 3u);
  ASSERT_EQ(elements.get_pipes().size(), 1u);
  ASSERT_EQ(elements.get_non_pipes().size(), 2u);

  const auto* reservoir = elements.get_element_by_id<Reservoir>(1);
  const auto* pipe = elements.get_element_by_id<Pipe>(2);
  auto* valve = elements.get_element_by_id<Valve>(3);
  ASSERT_NE(reservoir, nullptr);
  ASSERT_NE(pipe, nullptr);
  ASSERT_NE(valve, nullptr);
  EXPECT_EQ(reservoir->get_ID(), 1);
  EXPECT_EQ(pipe->get_ID(), 2);
  EXPECT_EQ(valve->get_ID(), 3);

  EXPECT_DOUBLE_EQ(reservoir->get_H(), 100.0);

  const auto& config = pipe->config();
  EXPECT_DOUBLE_EQ(config.length, 100.0);
  EXPECT_DOUBLE_EQ(config.diameter, 1.0);
  EXPECT_DOUBLE_EQ(config.f, 1.0);
  EXPECT_DOUBLE_EQ(config.a, 1.0);
  EXPECT_EQ(config.num_reaches, 10u);
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

  const auto& reservoir_ports = pipe->left_elem()->get_ports();
  ASSERT_GT(reservoir_ports.size(), static_cast<size_t>(PortType::Right));
  const auto* reservoir_right = reservoir_ports[PortType::Right].get();
  ASSERT_NE(reservoir_right, nullptr);
  ASSERT_NE(reservoir_right->m_connected_to, nullptr);
  EXPECT_EQ(&reservoir_right->m_connected_to->m_owner, pipe);
  EXPECT_EQ(reservoir_right->m_connected_to->m_connected_to, reservoir_right);

  const auto& valve_ports = valve->get_ports();
  ASSERT_GT(valve_ports.size(), static_cast<size_t>(PortType::Left));
  const auto* valve_left = valve_ports[PortType::Left].get();
  ASSERT_NE(valve_left, nullptr);
  ASSERT_NE(valve_left->m_connected_to, nullptr);
  EXPECT_EQ(&valve_left->m_connected_to->m_owner, pipe);
  EXPECT_EQ(valve_left->m_connected_to->m_connected_to, valve_left);

  const auto generated_path =
      get_mock_data_file_path("generated/plant_config_1_generated.json");
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
  ASSERT_EQ(PlantConfigRepository::load(
                get_mock_data_file_path("expected/plant_config_complex.json"),
                loaded),
            PlantRepositoryResult::Ok);
  const auto saved_path =
      get_mock_data_file_path("generated/plant_config_complex_generated.json");
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
      const auto* pipe = elements.get_element_by_id<Pipe>(id);
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
  Plant plant(loaded);
  plant.display();
}
