#include <gtest/gtest.h>
#include "lvtrans/pipe.hpp"
#include "lvtrans/plant.hpp"
#include "lvtrans/reservoir.hpp"

void print_elements(const lvtrans::Plant& plant) {
  for (const auto* element : plant.get_elements()) {
    std::cout << "Element " << element->get_ID() << std::endl;
    std::cout << "Type: " << typeid(*element).name() << std::endl;
  }
}

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

  auto pipe = plant.add_element<Pipe>(pipeConfig, 150.0, 0.0);
  ASSERT_EQ(plant.get_elements().size(), 1u);
  EXPECT_EQ(pipe->get_ID(), 0);
}

TEST(PlantTest, LooksUpAndRemovesElementsById) {
  using namespace lvtrans;
  Plant plant(0);
  plant.add_element<Reservoir>(100.0);
  plant.add_element<Reservoir>(100.0);

  ASSERT_EQ(plant.get_elements().size(), 2u);
  EXPECT_NE(plant.get_element_by_id(0), nullptr);
  EXPECT_NE(plant.get_element_by_id(1), nullptr);

  plant.remove_element(0);
  EXPECT_EQ(plant.get_elements().size(), 1u);
  EXPECT_EQ(plant.get_element_by_id(0), nullptr);

  EXPECT_NE(plant.get_element_by_id(1), nullptr);
  plant.remove_element(1);
  EXPECT_EQ(plant.get_elements().size(), 0u);
}
