#include <gtest/gtest.h>
#include <array>
#include <limits>
#include <stdexcept>
#include "../test_helpers.hpp"
#include "lvtrans/config/element_parsers.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"
#include "lvtrans/plant.hpp"

namespace {
using namespace lvtrans;

ValveInternalServoParameters parameters(double initial = 1.0) {
  return {.cvp = 2.0,
          .cvm = 8.0,
          .closing_time_high = 4.0,
          .closing_time_low = 2.0,
          .switch_over = 0.5,
          .servo_time = 1.0,
          .initial_opening = initial,
          .diameter = 1.0};
}

TEST(ValveInternalServoTest, FirstOrderResponseUsesPreviousOutputAndStepSize) {
  auto config = parameters();
  config.closing_time_high = config.closing_time_low = 0.01;
  ValveInternalServo valve(config, 1.0);
  EXPECT_DOUBLE_EQ(valve.get_tau(), 1.0);
  EXPECT_DOUBLE_EQ(valve.get_setpoint(), 1.0);
  ASSERT_EQ(valve.modify(SetValveOpening{0.0}), ModificationResult::Ok);
  EXPECT_DOUBLE_EQ(valve.get_tau(), 1.0);  // Setpoint is not actual opening.
  valve.iterate(100.0);  // Absolute plant time does not replace dt.
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.5);
  valve.iterate(101.0);
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.25);
  EXPECT_FALSE(valve.is_rate_limited());
}

TEST(ValveInternalServoTest,
     ClosingCrossesSwitchUsingOldOpeningAndHighAtEquality) {
  auto config = parameters(0.5);
  config.servo_time = 0.0;
  ValveInternalServo valve(config, 0.5);
  ASSERT_EQ(valve.modify(SetValveOpening{0.0}), ModificationResult::Ok);
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.375);  // High: 0.5 / 4.
  EXPECT_TRUE(valve.is_rate_limited());
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.125);  // Low: 0.5 / 2.
  EXPECT_TRUE(valve.is_rate_limited());
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.0);
  EXPECT_FALSE(valve.is_rate_limited());
}

TEST(ValveInternalServoTest, OpeningUsesSameSelectedTimingAndStrictRateLimit) {
  auto config = parameters(0.25);
  config.servo_time = 0.0;
  ValveInternalServo valve(config, 0.5);
  ASSERT_EQ(valve.modify(SetValveOpening{1.0}), ModificationResult::Ok);
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.5);  // Low timing for entire step.
  EXPECT_TRUE(valve.is_rate_limited());
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.625);  // Switch to high timing.
  EXPECT_TRUE(valve.is_rate_limited());
  ASSERT_EQ(valve.modify(SetValveOpening{0.75}), ModificationResult::Ok);
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.75);
  EXPECT_FALSE(valve.is_rate_limited());  // Exactly max_rate is not limited.
}

TEST(ValveInternalServoTest, LimitedOutputFeedsNextFirstOrderStepAndReset) {
  auto config = parameters(0.0);
  config.closing_time_low = config.closing_time_high = 4.0;
  ValveInternalServo valve(config, 0.5);
  ASSERT_EQ(valve.modify(SetValveOpening{1.0}), ModificationResult::Ok);
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.125);
  ASSERT_EQ(valve.modify(SetValveOpening{0.25}), ModificationResult::Ok);
  valve.iterate();
  EXPECT_NEAR(valve.get_tau(), 1.0 / 6.0, 1e-15);
  EXPECT_FALSE(valve.is_rate_limited());
  valve.reset_state();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.0);
  EXPECT_DOUBLE_EQ(valve.get_setpoint(), 0.0);
  EXPECT_FALSE(valve.is_rate_limited());
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.0);
  ASSERT_EQ(valve.modify(SetValveOpening{1.0}), ModificationResult::Ok);
  valve.iterate();
  EXPECT_DOUBLE_EQ(valve.get_tau(), 0.125);
}

TEST(ValveInternalServoTest, MatchesLabviewHydraulicsInBothDirections) {
  auto config = parameters(0.5);
  config.servo_time = 0.0;
  config.closing_time_high = config.closing_time_low = 0.01;
  ValveInternalServo valve(config, 0.1);
  PipeParameters pipe_config{.H0 = 0.0,
                             .Q0 = 0.0,
                             .length = 1000,
                             .diameter = 1.0,
                             .use_diameter = true};
  Pipe left(pipe_config, 0.1), right(pipe_config, 0.1);
  left.set_ID(1);
  right.set_ID(2);
  ASSERT_EQ(HydraulicConnections::connect(left, PipeEnd::Right, valve),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(right, PipeEnd::Left, valve),
            ConnectResult::Ok);
  // Independently chosen solutions to deltaC = (Bp+Bm)*Q + Q*abs(Q)/(2*Cv).
  for (const auto& row : std::array<std::array<double, 5>, 3>{
           {{10, 2, 2, 8, 4}, {2, 7, -2, 4, 5}, {5, 5, 0, 5, 5}}}) {
    valve.set_characteristics({row[0], 1.0, CharacteristicsType::Plus}, 1);
    valve.set_characteristics({row[1], 1.0, CharacteristicsType::Minus}, 2);
    EXPECT_NEAR(valve.get_Q(CharacteristicsType::Plus, 1), row[2], 1e-14);
    EXPECT_NEAR(valve.get_Q(CharacteristicsType::Minus, 2), row[2], 1e-14);
    EXPECT_NEAR(valve.get_H(CharacteristicsType::Plus, 1), row[3], 1e-14);
    EXPECT_NEAR(valve.get_H(CharacteristicsType::Minus, 2), row[4], 1e-14);
  }
  ASSERT_EQ(valve.modify(SetValveOpening{0.0}), ModificationResult::Ok);
  valve.iterate();
  valve.set_characteristics({10.0, 1.0, CharacteristicsType::Plus}, 1);
  valve.set_characteristics({2.0, 1.0, CharacteristicsType::Minus}, 2);
  EXPECT_DOUBLE_EQ(valve.get_Q(CharacteristicsType::Plus, 1), 0.0);
  EXPECT_DOUBLE_EQ(valve.get_H(CharacteristicsType::Plus, 1), 10.0);
  EXPECT_DOUBLE_EQ(valve.get_H(CharacteristicsType::Minus, 2), 2.0);
}

TEST(ValveInternalServoTest, PlantSaveReloadResumesOpeningAndSetpoint) {
  Plant plant(0.1);
  auto* valve =
      plant.add_element<ValveInternalServo>(parameters(), 0.1).value();
  const ElementID id = valve->get_ID();
  auto* pipe = plant
                   .add_element<Pipe>(PipeParameters{.H0 = 0.0,
                                                     .Q0 = 0.0,
                                                     .length = 1000,
                                                     .diameter = 1.0,
                                                     .use_diameter = true},
                                      0.1)
                   .value();
  const ElementID pipe_id = pipe->get_ID();
  ASSERT_EQ(HydraulicConnections::connect(*pipe, PipeEnd::Right, *valve),
            ConnectResult::Ok);
  ASSERT_EQ(plant.modify_element(id, SetValveOpening{0.0}),
            ModificationResult::Ok);
  plant.step(3);
  ASSERT_TRUE(valve->is_rate_limited());
  const auto path = get_mock_data_file_path("generated/servo_resume.json");
  ASSERT_EQ(plant.save(path), PlantRepositoryResult::Ok);
  PlantData data;
  ASSERT_EQ(PlantConfigRepository::load(path, data), PlantRepositoryResult::Ok);
  Plant resumed(std::move(data));
  auto* restored = resumed.get_element_by_id<ValveInternalServo>(id).value();
  ASSERT_NE(restored, nullptr);
  EXPECT_EQ(restored->get_type(), ElementType::ValveInternalServo);
  EXPECT_DOUBLE_EQ(restored->get_tau(), valve->get_tau());
  EXPECT_DOUBLE_EQ(restored->get_setpoint(), 0.0);
  EXPECT_TRUE(restored->is_rate_limited());
  EXPECT_EQ(restored->left()->get_ID(), pipe_id);
  for (int i = 0; i < 20; ++i) {
    plant.step();
    resumed.step();
    EXPECT_DOUBLE_EQ(restored->get_tau(), valve->get_tau());
  }
  resumed.reset_state();
  EXPECT_DOUBLE_EQ(restored->get_tau(), 1.0);
  EXPECT_DOUBLE_EQ(restored->get_setpoint(), 1.0);
  EXPECT_FALSE(restored->is_rate_limited());
}
}  // namespace
