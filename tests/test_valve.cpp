#include <gtest/gtest.h>
#include <lvtrans/pipe.hpp>
#include <lvtrans/valve.hpp>
#include <memory>

using namespace lvtrans;

TEST(ValveTest, InitializesPortsAndOpening) {
  auto valve = std::make_shared<Valve>(ValveConfig{0.8, 0.0, 4.0, 1.0, 0.5});
  EXPECT_DOUBLE_EQ(valve->get_tau(), 0.8);
  ASSERT_GT(valve->get_ports().size(), static_cast<size_t>(PortRight));
  for (auto index : {PortLeft, PortRight}) {
    const auto& port = valve->get_ports()[index];
    ASSERT_NE(port, nullptr);
    EXPECT_EQ(&port->owner, valve.get());
    EXPECT_EQ(port->connected_to, nullptr);
  }
  EXPECT_NE(valve->get_ports()[PortLeft], valve->get_ports()[PortRight]);
}

TEST(ValveTest, FollowsClosureCurveAndHoldsFinalOpening) {
  Valve valve(ValveConfig{0.8, 0.2, 4.0, 2.0, 0.5});
  IterateInput input{};
  valve.iterate(input, {});
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.8);

  input.t = 2.0;
  valve.iterate(input, {});
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.65);

  for (double time : {4.0, 8.0}) {
    input.t = time;
    valve.iterate(input, {});
    EXPECT_DOUBLE_EQ(valve.get_tau(), 0.2);
  }
}

TEST(ValveTest, OpenBoundarySatisfiesHeadAndFlowEquations) {
  Valve valve(ValveConfig{1.0, 0.0, 4.0, 1.0, 0.5});
  valve.set_c_characteristics(8.0);
  valve.set_b_characteristics(2.0);

  // H + 2Q = 8 and Q^2 = H give the forward-flow solution Q = 2, H = 4.
  EXPECT_DOUBLE_EQ(valve.get_Q({}), 2.0);
  EXPECT_DOUBLE_EQ(valve.get_H(), 4.0);
}

TEST(ValveTest, FullyClosedBoundaryStopsFlow) {
  Valve valve(ValveConfig{1.0, 0.0, 4.0, 1.0, 0.5});
  valve.set_c_characteristics(8.0);
  valve.set_b_characteristics(2.0);

  IterateInput input{};
  input.t = 4.0;
  valve.iterate(input, {});
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.0);
  EXPECT_DOUBLE_EQ(valve.get_Q({}), 0.0);
  EXPECT_DOUBLE_EQ(valve.get_H(), 8.0);
}
