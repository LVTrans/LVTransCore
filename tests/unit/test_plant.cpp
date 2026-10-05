#include <gtest/gtest.h>
#include "../test_helpers.hpp"
#include "lvtrans/element_modifications.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/plant.hpp"

TEST(PlantTest, AddElements) {
  using namespace lvtrans;
  Plant plant(0);

  ASSERT_TRUE(plant.get_elements().empty());

  PipeParameters pipeConfig{
      .length = 600.0,
      .diameter = 0.5,
      .f = 0.018,
      .a = 1200.0,
      .z0 = 10.0,
      .z1 = 15.0,
      .num_reaches = 10,
  };

  auto pipe = plant.add_element<Pipe>(pipeConfig, 150.0, 0.0).value();
  ASSERT_EQ(plant.get_elements().size(), 1u);
  EXPECT_EQ(pipe->get_ID(), 1);
}

TEST(PlantTest, LooksUpAndRemovesElementsById) {
  using namespace lvtrans;
  Plant plant(0);
  plant.add_element<Reservoir>(100.0);
  plant.add_element<Reservoir>(100.0);

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
  auto reservoir = plant.add_element<Reservoir>(100.0).value();
  ASSERT_NE(reservoir, nullptr);
  EXPECT_DOUBLE_EQ(reservoir->get_H(), 100.0);
  EXPECT_EQ(plant.modify_element(1, SetReservoirH0{150.0}),
            ModificationResult::Ok);
  EXPECT_DOUBLE_EQ(reservoir->get_H(), 150.0);
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
