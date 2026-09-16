#include "lvtrans/pipe.hpp"
#include "lvtrans/plant.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

namespace {
class TestElement : public lvtrans::Element {
public:
  // explicit TestElement(lvtrans::ElementID id) { m_ID = id; }
  void iterate(lvtrans::IterateInput, lvtrans::IterateOutput) override {}
};
} // namespace

void print_elements(const lvtrans::Plant &plant) {
  for (const auto &[id, element] : plant.get_elements()) {
    std::cout << "Element " << id << std::endl;
    std::cout << "Type: " << typeid(*element).name() << std::endl;
  }
}

TEST(PlantTest, AddElements) {
  using namespace lvtrans;
  Plant plant{};

  ASSERT_TRUE(plant.get_elements().empty());

  PipeConfig pipeConfig{600.0, 0.5, 0.018, 1200.0, 10, 10.0, 15.0};

  auto pipe = std::make_shared<Pipe>(pipeConfig, 150.0, 0.0);

  plant.add_element(pipe);
  ASSERT_EQ(plant.get_elements().size(), 1u);
  EXPECT_EQ(plant.get_elements().at(pipe->get_ID()), pipe);
  EXPECT_EQ(plant.get_element_by_id(pipe->get_ID()).value().get(), pipe.get());
}

TEST(PlantTest, LooksUpAndRemovesElementsById) {
  lvtrans::Plant plant;
  auto first = std::make_shared<TestElement>();
  auto second = std::make_shared<TestElement>();
  plant.add_element(first);
  plant.add_element(second);

  print_elements(plant);

  ASSERT_EQ(plant.get_elements().size(), 2u);
  EXPECT_EQ(plant.get_element_by_id(0).value().get(), first.get());
  EXPECT_EQ(plant.get_element_by_id(1).value().get(), second.get());

  plant.remove_element(0);
  EXPECT_EQ(plant.get_elements().size(), 1u);
  EXPECT_EQ(plant.get_element_by_id(0), std::nullopt);

  EXPECT_EQ(plant.get_element_by_id(1).value().get(), second.get());
  plant.remove_element(1);
  EXPECT_EQ(plant.get_elements().size(), 0u);
}

TEST(PlantTest, RejectsDuplicateIdsAndNullElements) {
  lvtrans::Plant plant;
  EXPECT_NO_THROW(plant.add_element(nullptr));
  ASSERT_EQ(plant.get_elements().size(), 0u);
}
