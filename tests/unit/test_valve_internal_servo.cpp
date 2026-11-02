#include <gtest/gtest.h>
#include "../test_helpers.hpp"
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
