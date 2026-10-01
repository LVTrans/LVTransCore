#include <gtest/gtest.h>
#include <lvtrans/elements/reservoir.hpp>
#include <memory>

using namespace lvtrans;

TEST(ReservoirTest, InitializesRightPort) {
  auto reservoir = std::make_shared<Reservoir>(150.0);
  ASSERT_GT(reservoir->get_ports().size(),
            static_cast<size_t>(PortType::Right));
  const auto& port = reservoir->get_ports()[PortType::Right];
  ASSERT_TRUE(port.has_value());
  EXPECT_FALSE(reservoir->get_ports()[PortType::Left].has_value());
  EXPECT_FALSE(reservoir->get_ports()[PortType::Up].has_value());
  EXPECT_FALSE(reservoir->get_ports()[PortType::Down].has_value());
  EXPECT_EQ(&port->get_owner(), reservoir.get());
  EXPECT_EQ(port->get_connected_to(), nullptr);
}

TEST(ReservoirTest, KeepsHeadConstantAcrossIterations) {
  Reservoir reservoir(150.0);
  reservoir.set_c_characteristics(100.0);
  reservoir.set_b_characteristics(50.0);

  EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
  for (double time : {0.0, 1.0, 10.0}) {
    IterateInput input{};
    input.t = time;
    reservoir.iterate(input);
    EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
  }
}

TEST(ReservoirTest, SupportsOutflowEquilibriumAndReverseFlow) {
  Reservoir reservoir(150.0);
  reservoir.set_b_characteristics(50.0);

  reservoir.set_c_characteristics(100.0);
  EXPECT_DOUBLE_EQ(reservoir.get_Q(), 1.0);

  reservoir.set_c_characteristics(150.0);
  EXPECT_DOUBLE_EQ(reservoir.get_Q(), 0.0);

  reservoir.set_c_characteristics(200.0);
  EXPECT_DOUBLE_EQ(reservoir.get_Q(), -1.0);
  EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
}

TEST(ReservoirTest, ModifiesHead) {
  Reservoir reservoir(150.0);
  EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
  reservoir.modify(SetReservoirH0{200.0});
  EXPECT_DOUBLE_EQ(reservoir.get_H(), 200.0);
}
