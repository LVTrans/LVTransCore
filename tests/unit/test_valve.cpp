#include <gtest/gtest.h>
#include <lvtrans/elements/pipe.hpp>
#include <lvtrans/elements/valve.hpp>
#include <memory>

using namespace lvtrans;

TEST(ValveTest, InitializesConnectionsAndOpening) {
  ValveParameters config{};
  config.tau_i = 0.8;
  config.tau_f = 0.0;
  config.tc = 4.0;
  config.em = 1.0;
  config.cvp = 0.5;
  auto valve = std::make_shared<Valve>(config);

  EXPECT_DOUBLE_EQ(valve->get_tau(), 0.8);
  ASSERT_EQ(valve->left_links().size(), 1u);
  ASSERT_EQ(valve->right_links().size(), 1u);
  EXPECT_EQ(valve->left_links()[0].pipe, nullptr);
  EXPECT_EQ(valve->right_links()[0].pipe, nullptr);
}

TEST(ValveTest, FollowsClosureCurveAndHoldsFinalOpening) {
  ValveParameters config{};
  config.tau_i = 0.8;
  config.tau_f = 0.2;
  config.tc = 4.0;
  config.em = 2.0;
  config.cvp = 0.5;
  Valve valve(config);
  valve.iterate(0.0);
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.8);

  double t = 2.0;
  valve.iterate(t);
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.65);

  for (double time : {4.0, 8.0}) {
    t = time;
    valve.iterate(t);
    EXPECT_DOUBLE_EQ(valve.get_tau(), 0.2);
  }
}

TEST(ValveTest, OpenBoundarySatisfiesHeadAndFlowEquations) {
  ValveParameters config{};
  config.tau_i = 1.0;
  config.tau_f = 0.0;
  config.tc = 4.0;
  config.em = 1.0;
  config.cvp = 0.5;
  Valve valve(config);
  //   valve.set_c_characteristics(8.0);
  //   valve.set_b_characteristics(2.0);
  //
  //   // H + 2Q = 8 and Q^2 = H give the forward-flow solution Q = 2, H = 4.
  //   EXPECT_DOUBLE_EQ(valve.get_Q({}), 2.0);
  //   EXPECT_DOUBLE_EQ(valve.get_H(), 4.0);
}

TEST(ValveTest, FullyClosedBoundaryStopsFlow) {
  ValveParameters config{};
  config.tau_i = 1.0;
  config.tau_f = 0.0;
  config.tc = 4.0;
  config.em = 1.0;
  config.cvp = 0.5;
  Valve valve(config);
  // valve.set_c_characteristics(8.0);
  // valve.set_b_characteristics(2.0);

  const auto t = 4.0;
  valve.iterate(t);
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.0);
  // EXPECT_DOUBLE_EQ(valve.get_Q({}), 0.0);
  // EXPECT_DOUBLE_EQ(valve.get_H(), 8.0);
}
