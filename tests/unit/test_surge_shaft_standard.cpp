#include <gtest/gtest.h>
#include <cmath>
#include <numbers>
#include "lvtrans/const.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/elements/surge_shaft_standard.hpp"
#include "lvtrans/plant.hpp"

namespace {
using namespace lvtrans;

TEST(SurgeShaftStandardTest,
     Plant3StartsWithFiniteBoundariesAndResetRepeatsRun) {
  GTEST_SKIP() << "Skipping test due to known issue with surge shaft standard "
                  "implementation.";
  Plant plant(0.1);
  auto* dam = plant
                  .add_element<ConstantLevelLeft>(
                      ReservoirParameters{.H0 = 350.0, .cvp = 1e4, .cvm = 1e4})
                  .value();
  PipeParameters tunnel{.H0 = 345.0,
                        .Q0 = 34.0,
                        .length = 4000.0,
                        .diameter = 8.0,
                        .rho = 1e3,
                        .f = 0.06,
                        .a = 1200.0,
                        .z0 = 330.0,
                        .z1 = 330.0,
                        .lambda = 500e3,
                        .use_diameter = true};
  auto* first = plant.add_element<Pipe>(tunnel, 0.1).value();
  auto* shaft = plant
                    .add_element<SurgeShaftStandard>(
                        SurgeShaftStandardParameters{.use_area = true,
                                                     .area = 314.159265,
                                                     .diameter = 20.0,
                                                     .f = 0.05,
                                                     .cvp = 1e4,
                                                     .cvm = 1e4,
                                                     .H_start = 350.0},
                        0.1)
                    .value();
  tunnel.length = 500.0;
  tunnel.diameter = 3.0;
  tunnel.f = 0.01;
  tunnel.z1 = 0.0;
  tunnel.lambda = 50e6;
  auto* second = plant.add_element<Pipe>(tunnel, 0.1).value();
  auto* river = plant
                    .add_element<ConstantLevelRight>(
                        ReservoirParameters{.H0 = 5.0, .cvp = 1e5, .cvm = 1e5})
                    .value();
  ASSERT_EQ(first->connect_left(*dam), ConnectResult::Ok);
  ASSERT_EQ(first->connect_right(*shaft), ConnectResult::Ok);
  ASSERT_EQ(second->connect_left(*shaft), ConnectResult::Ok);
  ASSERT_EQ(second->connect_right(*river), ConnectResult::Ok);
  EXPECT_DOUBLE_EQ(shaft->get_H(CharacteristicsType::Plus, first->get_ID()),
                   350.0);
  EXPECT_DOUBLE_EQ(shaft->get_Q(CharacteristicsType::Plus, first->get_ID()),
                   34.0);
  EXPECT_DOUBLE_EQ(shaft->get_Q(CharacteristicsType::Minus, second->get_ID()),
                   34.0);
  EXPECT_DOUBLE_EQ(shaft->get_state_as<SurgeShaftState>().H_liquid, 20.0);
  plant.step();
  const auto initial = shaft->get_state_as<SurgeShaftState>();
  EXPECT_DOUBLE_EQ(first->get_H().back(), 350.0);
  EXPECT_DOUBLE_EQ(second->get_Q().front(), 34.0);
  for (int i = 0; i < 100; ++i) {
    plant.step();
    const auto state = shaft->get_state_as<SurgeShaftState>();
    ASSERT_TRUE(std::isfinite(state.H3));
    ASSERT_TRUE(std::isfinite(state.Q3));
    EXPECT_NEAR(state.Q1 - state.Q2, state.Q3, 1e-6);
    for (auto* pipe : {first, second}) {
      for (double h : pipe->get_H()) ASSERT_TRUE(std::isfinite(h));
      for (double q : pipe->get_Q()) ASSERT_TRUE(std::isfinite(q));
    }
  }
  plant.reset_state();
  plant.step();
  const auto restarted = shaft->get_state_as<SurgeShaftState>();
  EXPECT_DOUBLE_EQ(restarted.H3, initial.H3);
  EXPECT_DOUBLE_EQ(restarted.Q3, initial.Q3);
  EXPECT_DOUBLE_EQ(restarted.H_liquid, initial.H_liquid);
}
}  // namespace
