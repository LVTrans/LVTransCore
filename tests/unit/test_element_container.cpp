#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>
#include "lvtrans/element_container.hpp"
#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/elements/valve.hpp"

namespace {
using namespace lvtrans;
using ::testing::UnorderedElementsAre;

const PipeParameters pipe_config{
    .length = 600.0,
    .diameter = 0.5,
    .f = 0.018,
    .a = 1200.0,
    .z0 = 10.0,
    .z1 = 15.0,
    .num_reaches = 10,
};

template <typename T>
T* add_to(ElementContainer& container) {
  if constexpr (std::is_base_of_v<Pipe, T>) {
    return container.add_element<T>(pipe_config, 150.0, 0.0).value();
  } else {
    return container.add_element<T>(150.0).value();
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
  auto reservoir = add_to<Reservoir>(container);
  auto pipe = add_to<Pipe>(container);
  ValveParameters valve_config{};
  valve_config.tau_i = 0.8;
  valve_config.tau_f = 0.0;
  valve_config.tc = 4.0;
  valve_config.em = 1.0;
  valve_config.cvp = 0.5;
  auto valve = container.add_element<Valve>(valve_config).value();

  EXPECT_NE(reservoir->get_ID(), pipe->get_ID());
  EXPECT_NE(reservoir->get_ID(), valve->get_ID());
  EXPECT_NE(pipe->get_ID(), valve->get_ID());
  EXPECT_EQ(container.get_element_by_id(reservoir->get_ID()), reservoir);
  EXPECT_EQ(container.get_element_by_id(pipe->get_ID()), pipe);
  EXPECT_EQ(container.get_element_by_id(valve->get_ID()), valve);
  EXPECT_DOUBLE_EQ(reservoir->get_H(), 150.0);
  EXPECT_DOUBLE_EQ(valve->get_tau(), 0.8);
  ASSERT_EQ(container.get_pipes().size(), 1u);
  EXPECT_EQ(container.get_pipes().front().get(), pipe);
  ASSERT_EQ(container.get_non_pipes().size(), 2u);
  const ElementContainer& view = container;
  EXPECT_THAT(view.get_elements(),
              UnorderedElementsAre(pipe, reservoir, valve));
}

TEST(ElementContainerTest, DuplicateInsertIDShouldFail) {
  ElementContainer container;
  EXPECT_NE(container.add_element_with_id<Reservoir>(0, 1.0), nullptr);
  EXPECT_EQ(container.add_element_with_id<Reservoir>(0, 1.0), nullptr);
}

TEST(ElementContainerTest, ContainersKeepIdsAndOwnershipIndependent) {
  ElementContainer first;
  ElementContainer second;
  auto a = add_to<Reservoir>(first);
  auto b = add_to<Reservoir>(second);
  const auto id = a->get_ID();
  EXPECT_EQ(id, 0);
  EXPECT_EQ(b->get_ID(), 0);
  EXPECT_NE(a, b);

  first.remove_element(id);
  EXPECT_TRUE(first.get_elements().empty());
  EXPECT_EQ(second.get_element_by_id(id), b);
  EXPECT_EQ(second.get_elements().size(), 1u);
}

TEST(ElementContainerTest, RemovingConnectedPipeClearsSurvivingPeerPorts) {
  ElementContainer container;
  auto reservoir = add_to<Reservoir>(container);
  auto valve = container.add_element<Valve>(ValveParameters{}).value();
  auto pipe = add_to<Pipe>(container);
  pipe->connect_to(reservoir, PortType::Left, PortType::Right);
  pipe->connect_to(valve, PortType::Right, PortType::Left);
  ASSERT_NE(reservoir->get_ports()[PortType::Right]->m_connected_to, nullptr);
  ASSERT_NE(valve->get_ports()[PortType::Left]->m_connected_to, nullptr);

  container.remove_element(pipe->get_ID());
  EXPECT_EQ(reservoir->get_ports()[PortType::Right]->m_connected_to, nullptr);
  EXPECT_EQ(valve->get_ports()[PortType::Left]->m_connected_to, nullptr);
  EXPECT_THAT(container.get_elements(), UnorderedElementsAre(reservoir, valve));

  auto replacement = add_to<Pipe>(container);
  replacement->connect_to(reservoir, PortType::Left, PortType::Right);
  replacement->connect_to(valve, PortType::Right, PortType::Left);
  EXPECT_EQ(replacement->left_elem(), reservoir);
  EXPECT_EQ(replacement->right_elem(), valve);
}

TEST(ElementContainerTest,
     ResettingSparsePortsIsRepeatableAndPreservesOtherConnections) {
  ElementContainer container;
  auto reservoir = add_to<Reservoir>(container);
  auto valve = container.add_element<Valve>(ValveParameters{}).value();
  auto pipe = add_to<Pipe>(container);
  pipe->connect_to(reservoir, PortType::Left, PortType::Right);
  pipe->connect_to(valve, PortType::Right, PortType::Left);

  reservoir->reset_ports();
  reservoir->reset_ports();
  EXPECT_EQ(reservoir->get_ports()[PortType::Right]->m_connected_to, nullptr);
  EXPECT_EQ(pipe->left_elem(), nullptr);
  EXPECT_EQ(pipe->right_elem(), valve);

  pipe->reset_ports();
  pipe->reset_ports();
  EXPECT_EQ(pipe->left_elem(), nullptr);
  EXPECT_EQ(pipe->right_elem(), nullptr);
  EXPECT_EQ(valve->get_ports()[PortType::Left]->m_connected_to, nullptr);
}
}  // namespace
