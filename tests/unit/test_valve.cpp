#include <gtest/gtest.h>
#include <lvtrans/pipe.hpp>
#include <lvtrans/valve.hpp>
#include <memory>

using namespace lvtrans;

TEST(ValveTest, InitializesPortsAndOpening) {
  ValveConfig config{};
  config.tau_i = 0.8;
  config.tau_f = 0.0;
  config.tc = 4.0;
  config.em = 1.0;
  config.cvp = 0.5;
  auto valve = std::make_shared<Valve>(config);

  EXPECT_DOUBLE_EQ(valve->get_tau(), 0.8);
  ASSERT_GT(valve->get_ports().size(), static_cast<size_t>(PortType::Right));
  for (auto index : {PortType::Left, PortType::Right}) {
    const auto& port = valve->get_ports()[index];
    ASSERT_NE(port, nullptr);
    EXPECT_EQ(&port->owner, valve.get());
    EXPECT_EQ(port->connected_to, nullptr);
  }
  EXPECT_NE(valve->get_ports()[PortType::Left],
            valve->get_ports()[PortType::Right]);
}

TEST(ValveTest, FollowsClosureCurveAndHoldsFinalOpening) {
  ValveConfig config{};
  config.tau_i = 0.8;
  config.tau_f = 0.2;
  config.tc = 4.0;
  config.em = 2.0;
  config.cvp = 0.5;
  Valve valve(config);
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
  ValveConfig config{};
  config.tau_i = 1.0;
  config.tau_f = 0.0;
  config.tc = 4.0;
  config.em = 1.0;
  config.cvp = 0.5;
  Valve valve(config);
  valve.set_c_characteristics(8.0);
  valve.set_b_characteristics(2.0);

  // H + 2Q = 8 and Q^2 = H give the forward-flow solution Q = 2, H = 4.
  EXPECT_DOUBLE_EQ(valve.get_Q({}), 2.0);
  EXPECT_DOUBLE_EQ(valve.get_H(), 4.0);
}

TEST(ValveTest, FullyClosedBoundaryStopsFlow) {
  ValveConfig config{};
  config.tau_i = 1.0;
  config.tau_f = 0.0;
  config.tc = 4.0;
  config.em = 1.0;
  config.cvp = 0.5;
  Valve valve(config);
  valve.set_c_characteristics(8.0);
  valve.set_b_characteristics(2.0);

  IterateInput input{};
  input.t = 4.0;
  valve.iterate(input, {});
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.0);
  EXPECT_DOUBLE_EQ(valve.get_Q({}), 0.0);
  EXPECT_DOUBLE_EQ(valve.get_H(), 8.0);
}
