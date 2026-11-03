#include <gtest/gtest.h>
#include "../test_helpers.hpp"
#include "gtest/gtest.h"
#include "lvtrans/element_modifications.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"
#include "lvtrans/plant.hpp"

TEST(PlantTest, AddElements) {
  using namespace lvtrans;
  Plant plant(0);

  ASSERT_TRUE(plant.get_elements().empty());

  PipeParameters pipeConfig{
      .H0 = 150.0,
      .length = 600.0,
      .diameter = 0.5,
      .f = 0.018,
      .a = 1200.0,
      .z0 = 10.0,
      .z1 = 15.0,
  };

  auto pipe = plant.add_element<Pipe>(pipeConfig, 0.1).value();
  ASSERT_EQ(plant.get_elements().size(), 1u);
  EXPECT_EQ(pipe->get_ID(), 1);
}

TEST(PlantTest, LooksUpAndRemovesElementsById) {
  using namespace lvtrans;
  Plant plant(0);
  plant.add_element<ConstantLevelLeft>(100.0);
  plant.add_element<ConstantLevelLeft>(100.0);

  ASSERT_EQ(plant.get_elements().size(), 2u);
  EXPECT_NE(plant.get_element_by_id(1), nullptr);
  EXPECT_NE(plant.get_element_by_id(2), nullptr);

  plant.remove_element_by_id(1);
  EXPECT_EQ(plant.get_elements().size(), 1u);
  EXPECT_EQ(plant.get_element_by_id(1), nullptr);

  EXPECT_NE(plant.get_element_by_id(2), nullptr);
  plant.remove_element_by_id(2);
  EXPECT_EQ(plant.get_elements().size(), 0u);
}

TEST(PlantTest, ModifiesElement) {
  using namespace lvtrans;
  Plant plant(0);
  auto reservoir = plant.add_element<ConstantLevelLeft>(100.0).value();
  ASSERT_NE(reservoir, nullptr);
  EXPECT_EQ(plant.modify_element(1, SetReservoirH0{150.0}),
            ModificationResult::Ok);
  EXPECT_EQ(plant.modify_element(1, SetPeltonExtractor{200.0}),
            ModificationResult::UnsupportedModification);

  EXPECT_EQ(plant.modify_element(2, SetPeltonExtractor{200.0}),
            ModificationResult::ElementNotFound);
}

TEST(PlantTest, ReadElementState) {
  using namespace lvtrans;
  Plant plant(get_mock_data_file_path("expected/plant_config_1.json"));

  auto view = plant.read_state(1);
  ASSERT_NE(view, std::nullopt);
  EXPECT_EQ(view->element_id, 1);
}

TEST(PlantTest, BoundaryCharacteristicsExistBeforeFirstStepAndAfterReset) {
  using namespace lvtrans;
  Plant plant(0.1);
  auto pipe =
      plant
          .add_element<Pipe>(
              PipeParameters{.H0 = 0.0, .Q0 = 0.0, .length = 100.0}, 0.1)
          .value();
  auto left = plant.add_element<ConstantLevelLeft>(100.0).value();
  auto right = plant.add_element<ConstantLevelRight>(0.0).value();
  ASSERT_EQ(pipe->connect_left(*left), ConnectResult::Ok);
  ASSERT_EQ(pipe->connect_right(*right), ConnectResult::Ok);

  ASSERT_NO_THROW(plant.step());
  const auto initial_h = pipe->get_H();
  const auto initial_q = pipe->get_Q();
  const double next_inflow = left->get_state_as<ReservoirState>().Q;
  const double next_outflow = right->get_state_as<ReservoirState>().Q;
  plant.step();
  EXPECT_DOUBLE_EQ(pipe->get_Q().front(), next_inflow);
  EXPECT_DOUBLE_EQ(pipe->get_Q().back(), next_outflow);

  plant.step(10);
  plant.reset_state();
  ASSERT_NO_THROW(plant.step());
  EXPECT_EQ(pipe->get_H(), initial_h);
  EXPECT_EQ(pipe->get_Q(), initial_q);
}
