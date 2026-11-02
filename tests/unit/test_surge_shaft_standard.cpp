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

void check_solution(SurgeShaftStandard& shaft, double q, double area,
                    double diameter, double initial_base = 330.0) {
  const auto old = shaft.get_state_as<SurgeShaftState>();
  const auto& p = shaft.config();
  constexpr double dt = 0.1;
  const double level = old.H_liquid + dt * (q + old.Q3) / (2.0 * area);
  const double cv = q >= 0.0 ? p.cvp : p.cvm;
  double hb = level + initial_base;
  if (level > 0.01) {
    hb = (level + old.H_liquid) * (q - old.Q3) / (area * consts::g * dt) +
         level + old.H_liquid + 2.0 * initial_base - old.Hb +
         p.f *
             (level * q * std::abs(q) +
              old.H_liquid * old.Q3 * std::abs(old.Q3)) /
             (2.0 * consts::g * diameter * area * area);
  }
  const double ha = hb + q * std::abs(q) / (2.0 * cv);
  constexpr double q_in = 6.0, bp = 2.0, bm = 3.0;
  shaft.set_characteristics({ha + bp * q_in, bp, CharacteristicsType::Plus}, 1);
  shaft.set_characteristics(
      {ha - bm * (q_in - q), bm, CharacteristicsType::Minus}, 2);
  shaft.iterate();
  const auto state = shaft.get_state_as<SurgeShaftState>();
  EXPECT_NEAR(state.H_liquid, level, 1e-8);
  EXPECT_NEAR(state.Hb, hb, 1e-7);
  EXPECT_NEAR(state.H3, ha, 1e-7);
  EXPECT_NEAR(state.Q3, q, 1e-7);
  EXPECT_NEAR(state.Q1, q_in, 1e-7);
  EXPECT_NEAR(state.Q2, q_in - q, 1e-7);
  EXPECT_NEAR(state.Q1 - state.Q2, state.Q3, 1e-7);
  EXPECT_DOUBLE_EQ(state.H_liquid_0, old.H_liquid);
  EXPECT_DOUBLE_EQ(state.Cv, cv);
  EXPECT_DOUBLE_EQ(shaft.get_H(CharacteristicsType::Plus, 1), state.H3);
  EXPECT_DOUBLE_EQ(shaft.get_Q(CharacteristicsType::Plus, 1), state.Q1);
  EXPECT_DOUBLE_EQ(shaft.get_Q(CharacteristicsType::Minus, 2), state.Q2);
}

TEST(SurgeShaftStandardTest,
     MatchesPrescribedLabviewSolutionsAndGeometryModes) {
  for (bool use_area : {true, false}) {
    for (double starting_level : {350.0, 330.005}) {
      constexpr double area = 5.0;
      const double diameter = std::sqrt(4.0 * area / std::numbers::pi);
      SurgeShaftStandard shaft({.use_area = use_area,
                                .area = use_area ? area : 999.0,
                                .diameter = use_area ? 999.0 : diameter,
                                .f = 0.05,
                                .cvp = 2.0,
                                .cvm = 7.0,
                                .H_start = starting_level},
                               0.1);
      PipeParameters config{.H0 = starting_level,
                            .Q0 = 0.0,
                            .length = 100.0,
                            .z0 = 330.0,
                            .z1 = 330.0};
      Pipe left(config, 0.1), right(config, 0.1);
      left.set_ID(1);
      right.set_ID(2);
      ASSERT_EQ(left.connect_right(shaft), ConnectResult::Ok);
      ASSERT_EQ(right.connect_left(shaft), ConnectResult::Ok);
      EXPECT_DOUBLE_EQ(shaft.get_H(CharacteristicsType::Plus, 1),
                       starting_level);
      const bool shallow = starting_level < 331.0;
      check_solution(shaft, shallow ? 0.1 : 2.0, area, diameter);
      check_solution(shaft, shallow ? -0.1 : -1.5, area, diameter);
    }
  }
}

TEST(SurgeShaftStandardTest,
     Plant3StartsWithFiniteBoundariesAndResetRepeatsRun) {
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
