#include "lvtrans/element_container.hpp"
#include "lvtrans/reservoir.hpp"
#include "lvtrans/valve.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
using namespace lvtrans;
using ::testing::UnorderedElementsAre;

const PipeConfig pipe_config{600.0, 0.5, 0.018, 1200.0, 10, 10.0, 15.0};

template <typename T> T &add_to(ElementContainer &container) {
  if constexpr (std::is_base_of_v<Pipe, T>) {
    return container.add_element<T>(pipe_config, 150.0, 0.0);
  } else {
    return container.add_element<T>(150.0);
  }
}

TEST(ElementContainerTest, EmptyContainerAcceptsMissingLookupsAndRemovals) {
  ElementContainer container;
  for (ElementID id : {std::numeric_limits<ElementID>::min(), -1, 0,
                       std::numeric_limits<ElementID>::max()}) {
    EXPECT_EQ(container.get_element_by_id(id), nullptr);
    EXPECT_NO_THROW(container.remove_element(id));
  }
  EXPECT_TRUE(container.get_elements().empty());
  EXPECT_TRUE(container.get_pipes().empty());
  EXPECT_TRUE(container.get_non_pipes().empty());
}

TEST(ElementContainerTest, MixedTypesHaveUniqueIdsAndCorrectTypedViews) {
  ElementContainer container;
  auto &reservoir = add_to<Reservoir>(container);
  auto &pipe = add_to<Pipe>(container);
  auto &valve =
      container.add_element<Valve>(ValveConfig{0.8, 0.0, 4.0, 1.0, 0.5});

  EXPECT_NE(reservoir.get_ID(), pipe.get_ID());
  EXPECT_NE(reservoir.get_ID(), valve.get_ID());
  EXPECT_NE(pipe.get_ID(), valve.get_ID());
  EXPECT_EQ(container.get_element_by_id(reservoir.get_ID()), &reservoir);
  EXPECT_EQ(container.get_element_by_id(pipe.get_ID()), &pipe);
  EXPECT_EQ(container.get_element_by_id(valve.get_ID()), &valve);
  EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.8);
  ASSERT_EQ(container.get_pipes().size(), 1u);
  EXPECT_EQ(container.get_pipes().front().get(), &pipe);
  ASSERT_EQ(container.get_non_pipes().size(), 2u);
  const ElementContainer &view = container;
  EXPECT_THAT(view.get_elements(),
              UnorderedElementsAre(&pipe, &reservoir, &valve));
}

TEST(ElementContainerTest, ContainersKeepIdsAndOwnershipIndependent) {
  ElementContainer first;
  ElementContainer second;
  auto &a = add_to<Reservoir>(first);
  auto &b = add_to<Reservoir>(second);
  const auto id = a.get_ID();
  EXPECT_EQ(id, 0);
  EXPECT_EQ(b.get_ID(), 0);
  EXPECT_NE(&a, &b);

  first.remove_element(id);
  EXPECT_TRUE(first.get_elements().empty());
  EXPECT_EQ(second.get_element_by_id(id), &b);
  EXPECT_EQ(second.get_elements().size(), 1u);
}

template <typename T>
class ElementContainerStorageTest : public ::testing::Test {};
using StorageTypes = ::testing::Types<Pipe, Reservoir>;
TYPED_TEST_SUITE(ElementContainerStorageTest, StorageTypes);

TYPED_TEST(ElementContainerStorageTest,
           RemovesOnlyElementAndDoesNotReuseItsId) {
  ElementContainer container;
  const auto removed_id = add_to<TypeParam>(container).get_ID();
  container.remove_element(removed_id);
  EXPECT_EQ(container.get_element_by_id(removed_id), nullptr);
  EXPECT_TRUE(container.get_elements().empty());
  EXPECT_TRUE(container.get_pipes().empty());
  EXPECT_TRUE(container.get_non_pipes().empty());
  EXPECT_NO_THROW(container.remove_element(removed_id));

  auto &added = add_to<TypeParam>(container);
  EXPECT_GT(added.get_ID(), removed_id);
  EXPECT_EQ(container.get_element_by_id(added.get_ID()), &added);
  EXPECT_EQ(container.get_element_by_id(removed_id), nullptr);
}

TYPED_TEST(ElementContainerStorageTest,
           RemovingFirstMiddleOrLastPreservesSurvivors) {
  for (size_t removed_index : {0u, 2u, 4u}) {
    SCOPED_TRACE(removed_index);
    ElementContainer container;
    std::vector<Element *> elements;
    std::vector<ElementID> ids;
    for (int i = 0; i < 5; ++i) {
      auto &element = add_to<TypeParam>(container);
      elements.push_back(&element);
      ids.push_back(element.get_ID());
    }

    container.remove_element(ids[removed_index]);
    ASSERT_EQ(container.get_elements().size(), 4u);
    for (size_t i = 0; i < ids.size(); ++i) {
      if (i == removed_index) {
        EXPECT_EQ(container.get_element_by_id(ids[i]), nullptr);
      } else {
        EXPECT_EQ(container.get_element_by_id(ids[i]), elements[i]);
      }
    }
    // Remove survivors by their original IDs, including any moved element.
    for (auto id : ids) {
      container.remove_element(id);
      EXPECT_EQ(container.get_element_by_id(id), nullptr);
    }
    EXPECT_TRUE(container.get_elements().empty());
  }
}

TYPED_TEST(ElementContainerStorageTest,
           MissingAndRepeatedRemovalLeaveOtherElementsIntact) {
  ElementContainer container;
  auto &first = add_to<TypeParam>(container);
  auto &second = add_to<TypeParam>(container);
  const auto first_id = first.get_ID();
  for (ElementID id : {-1, std::numeric_limits<ElementID>::max()}) {
    container.remove_element(id);
    EXPECT_EQ(container.get_element_by_id(id), nullptr);
  }
  ASSERT_EQ(container.get_elements().size(), 2u);
  EXPECT_EQ(container.get_element_by_id(first_id), &first);

  container.remove_element(first_id);
  container.remove_element(first_id);
  ASSERT_EQ(container.get_elements().size(), 1u);
  EXPECT_EQ(container.get_element_by_id(second.get_ID()), &second);
}

TYPED_TEST(ElementContainerStorageTest,
           AddingAfterRemovalKeepsMovedElementAccessible) {
  ElementContainer container;
  const auto first_id = add_to<TypeParam>(container).get_ID();
  auto &survivor = add_to<TypeParam>(container);
  container.remove_element(first_id);
  auto &added = add_to<TypeParam>(container);

  EXPECT_NE(added.get_ID(), first_id);
  EXPECT_NE(added.get_ID(), survivor.get_ID());
  EXPECT_EQ(container.get_element_by_id(first_id), nullptr);
  EXPECT_EQ(container.get_element_by_id(survivor.get_ID()), &survivor);
  EXPECT_EQ(container.get_element_by_id(added.get_ID()), &added);
  EXPECT_THAT(container.get_elements(),
              UnorderedElementsAre(&survivor, &added));
}

TYPED_TEST(ElementContainerStorageTest, GrowthKeepsReturnedAddressesStable) {
  ElementContainer container;
  std::unordered_map<ElementID, Element *> expected;
  for (int i = 0; i < 128; ++i) {
    auto &element = add_to<TypeParam>(container);
    ASSERT_TRUE(expected.emplace(element.get_ID(), &element).second);
  }
  EXPECT_EQ(container.get_elements().size(), expected.size());
  for (const auto &[id, address] : expected) {
    EXPECT_EQ(container.get_element_by_id(id), address);
  }
}

template <typename Base> class TrackedElement : public Base {
public:
  template <typename... Args>
  TrackedElement(int &destroyed, Args &&...args)
      : Base(std::forward<Args>(args)...), destroyed_(destroyed) {}
  ~TrackedElement() override { ++destroyed_; }

private:
  int &destroyed_;
};

TYPED_TEST(ElementContainerStorageTest,
           DestroysRemovedAndRemainingElementsExactlyOnce) {
  int first_destroyed = 0;
  int second_destroyed = 0;
  {
    ElementContainer container;
    auto add_tracked = [&](int &counter) -> Element & {
      if constexpr (std::is_base_of_v<Pipe, TypeParam>) {
        return container.add_element<TrackedElement<TypeParam>>(
            counter, pipe_config, 150.0, 0.0);
      } else {
        return container.add_element<TrackedElement<TypeParam>>(counter, 150.0);
      }
    };
    const auto removed_id = add_tracked(first_destroyed).get_ID();
    add_tracked(second_destroyed);
    EXPECT_EQ(first_destroyed, 0);
    EXPECT_EQ(second_destroyed, 0);
    container.remove_element(removed_id);
    EXPECT_EQ(first_destroyed, 1);
    EXPECT_EQ(second_destroyed, 0);
    container.remove_element(removed_id);
    EXPECT_EQ(first_destroyed, 1);
  }
  EXPECT_EQ(first_destroyed, 1);
  EXPECT_EQ(second_destroyed, 1);
}

class ThrowingReservoir : public Reservoir {
public:
  ThrowingReservoir() : Reservoir(100.0) {
    throw std::runtime_error("Construction failed");
  }
};

TEST(ElementContainerTest, ConstructorFailureLeavesContainerUsable) {
  ElementContainer container;
  auto &pipe = add_to<Pipe>(container);
  const auto pipe_id = pipe.get_ID();
  EXPECT_THROW(container.add_element<ThrowingReservoir>(), std::runtime_error);
  EXPECT_EQ(container.get_element_by_id(pipe_id), &pipe);
  EXPECT_THAT(container.get_elements(), UnorderedElementsAre(&pipe));
  EXPECT_TRUE(container.get_non_pipes().empty());

  auto &reservoir = add_to<Reservoir>(container);
  EXPECT_EQ(reservoir.get_ID(), pipe_id + 1);
  EXPECT_EQ(container.get_element_by_id(reservoir.get_ID()), &reservoir);
}

class OwningReservoir : public Reservoir {
public:
  explicit OwningReservoir(std::unique_ptr<int> value)
      : Reservoir(*value), value_(std::move(value)) {}
  const int *value() const { return value_.get(); }

private:
  std::unique_ptr<int> value_;
};

TEST(ElementContainerTest, ForwardsMoveOnlyConstructorArguments) {
  ElementContainer container;
  auto value = std::make_unique<int>(123);
  const auto *address = value.get();
  auto &element = container.add_element<OwningReservoir>(std::move(value));
  EXPECT_EQ(value, nullptr);
  EXPECT_EQ(element.value(), address);
  EXPECT_DOUBLE_EQ(element.get_H(), 123.0);
  EXPECT_EQ(container.get_element_by_id(element.get_ID()), &element);
}

TEST(ElementContainerTest, MixedInsertionsAndRemovalsMatchLiveElements) {
  ElementContainer container;
  std::unordered_map<ElementID, Element *> expected;
  std::vector<ElementID> ids;
  std::mt19937 random(42);
  for (int step = 0; step < 200; ++step) {
    SCOPED_TRACE(step);
    if (ids.empty() || random() % 3 != 0) {
      Element &element =
          random() % 2 == 0
              ? static_cast<Element &>(add_to<Pipe>(container))
              : static_cast<Element &>(add_to<Reservoir>(container));
      ASSERT_TRUE(expected.emplace(element.get_ID(), &element).second);
      ids.push_back(element.get_ID());
    } else {
      const auto id = ids[random() % ids.size()];
      container.remove_element(id);
      expected.erase(id);
    }

    const auto actual = container.get_elements();
    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_EQ(
        std::unordered_set<Element *>(actual.begin(), actual.end()).size(),
        actual.size());
    EXPECT_EQ(container.get_pipes().size() + container.get_non_pipes().size(),
              expected.size());
    for (auto id : ids) {
      const auto it = expected.find(id);
      EXPECT_EQ(container.get_element_by_id(id),
                it == expected.end() ? nullptr : it->second);
    }
    for (auto *element : actual) {
      ASSERT_NE(element, nullptr);
      ASSERT_NE(expected.find(element->get_ID()), expected.end());
      EXPECT_EQ(expected.at(element->get_ID()), element);
    }
  }
}
TEST(ElementContainerTest, RemovingConnectedPipeClearsSurvivingPeerPorts) {
  ElementContainer container;
  auto &reservoir = add_to<Reservoir>(container);
  auto &valve = container.add_element<Valve>(ValveConfig{});
  auto &pipe = add_to<Pipe>(container);
  pipe.connect_left(reservoir);
  pipe.connect_right(valve);
  ASSERT_NE(reservoir.get_ports()[PortRight]->connected_to, nullptr);
  ASSERT_NE(valve.get_ports()[PortLeft]->connected_to, nullptr);

  container.remove_element(pipe.get_ID());
  EXPECT_EQ(reservoir.get_ports()[PortRight]->connected_to, nullptr);
  EXPECT_EQ(valve.get_ports()[PortLeft]->connected_to, nullptr);
  EXPECT_THAT(container.get_elements(),
              UnorderedElementsAre(&reservoir, &valve));

  auto &replacement = add_to<Pipe>(container);
  replacement.connect_left(reservoir);
  replacement.connect_right(valve);
  EXPECT_EQ(replacement.left_elem(), &reservoir);
  EXPECT_EQ(replacement.right_elem(), &valve);
}

TEST(ElementContainerTest,
     ResettingSparsePortsIsRepeatableAndPreservesOtherConnections) {
  ElementContainer container;
  auto &reservoir = add_to<Reservoir>(container);
  auto &valve = container.add_element<Valve>(ValveConfig{});
  auto &pipe = add_to<Pipe>(container);
  pipe.connect_left(reservoir);
  pipe.connect_right(valve);

  reservoir.reset_ports();
  reservoir.reset_ports();
  EXPECT_EQ(reservoir.get_ports()[PortRight]->connected_to, nullptr);
  EXPECT_EQ(pipe.left_elem(), nullptr);
  EXPECT_EQ(pipe.right_elem(), &valve);

  pipe.reset_ports();
  pipe.reset_ports();
  EXPECT_EQ(pipe.left_elem(), nullptr);
  EXPECT_EQ(pipe.right_elem(), nullptr);
  EXPECT_EQ(valve.get_ports()[PortLeft]->connected_to, nullptr);
}
} // namespace
