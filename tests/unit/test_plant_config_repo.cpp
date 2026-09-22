#include <gtest/gtest.h>
#include <vector>
#include "../test_helpers.hpp"
#include "lvtrans/config/plant_config_repository.hpp"
#include "lvtrans/plant.hpp"
#include "lvtrans/reservoir.hpp"
#include "lvtrans/valve.hpp"

TEST(PlantConfigRepositoryTest, LoadPlantConfig) {
  using namespace lvtrans;
  const auto file_path = get_mock_data_file_path("plant_config_1.json");
  PlantConfigRepository repo;
  PlantData plant_data;

  ASSERT_EQ(repo.load(file_path, plant_data), PlantRepositoryResult::Ok);

  EXPECT_DOUBLE_EQ(plant_data.config.step_size, 0.5);
  EXPECT_DOUBLE_EQ(plant_data.state.current_time, 10.0);
  EXPECT_EQ(plant_data.state.num_iterations, 2);

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
  // Saved state takes precedence over the initial opening (tau_i = 0).
  EXPECT_DOUBLE_EQ(valve->get_tau(), 1.0);

  ASSERT_EQ(pipe->left_elem(), reservoir);
  ASSERT_EQ(pipe->right_elem(), valve);

  // Check both directions of each connection, including the non-pipe port.
  const auto& reservoir_ports = pipe->left_elem()->get_ports();
  ASSERT_GT(reservoir_ports.size(), static_cast<size_t>(PortType::Right));
  const auto* reservoir_right = reservoir_ports[PortType::Right].get();
  ASSERT_NE(reservoir_right, nullptr);
  ASSERT_NE(reservoir_right->connected_to, nullptr);
  EXPECT_EQ(&reservoir_right->connected_to->owner, pipe);
  EXPECT_EQ(reservoir_right->connected_to->connected_to, reservoir_right);

  const auto& valve_ports = valve->get_ports();
  ASSERT_GT(valve_ports.size(), static_cast<size_t>(PortType::Left));
  const auto* valve_left = valve_ports[PortType::Left].get();
  ASSERT_NE(valve_left, nullptr);
  ASSERT_NE(valve_left->connected_to, nullptr);
  EXPECT_EQ(&valve_left->connected_to->owner, pipe);
  EXPECT_EQ(valve_left->connected_to->connected_to, valve_left);

  Plant plant(plant_data);
  EXPECT_DOUBLE_EQ(plant.get_current_time(), 10.0);
  EXPECT_EQ(plant.get_element_by_id<Reservoir>(1), reservoir);
  EXPECT_EQ(plant.get_element_by_id<Pipe>(2), pipe);
  EXPECT_EQ(plant.get_element_by_id<Valve>(3), valve);
}
