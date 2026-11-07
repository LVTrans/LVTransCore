#include <gtest/gtest.h>
#include <cmath>
#include "lvtrans/elements/pid_valve.hpp"
using namespace lvtrans;
namespace {
double value(const PIDValve& pid, const char* name) {
  for (const auto& v : pid.read_view().values)
    if (const auto* scalar = std::get_if<ScalarValue>(&v);
        scalar && scalar->name == name)
      return scalar->value;
  ADD_FAILURE() << "Missing diagnostic " << name;
  return 0;
}
TEST(PIDValveTest,
     SimpsonHistoryUsesTwoDistinctPreviousSamplesAndResetRepeats) {
  PIDValve pid({.Q_wanted = 1, .SP_Q = 1, .Kp = 1, .Ti = 1}, 0.1);
  pid.set_controls(true, true, false, 0, 10, -1);
  auto run = [&] {
    pid.set_SP_Q(1);
    pid.iterate();
    EXPECT_DOUBLE_EQ(value(pid, "Integral"), 1.0);
    pid.set_SP_Q(2);
    pid.iterate();
    EXPECT_NEAR(value(pid, "Integral"), 1.1, 1e-12);
    pid.set_SP_Q(3);
    pid.iterate();
    EXPECT_NEAR(value(pid, "Integral"), 1.3, 1e-12);
  };
  run();
  pid.reset_state();
  run();
}
TEST(PIDValveTest, DerivativeFilterUsesMeasurementAndNegativeTfConvention) {
  PIDValve pid({.Q_wanted = 1, .SP_Q = 1, .Kp = 1, .Ti = 0, .Td = 1}, 0.1);
  pid.set_controls(true, true, false, 0, 10, -1);
  pid.set_PV_Q(0);
  pid.iterate();
  pid.set_PV_Q(1);
  pid.iterate();
  EXPECT_NEAR(value(pid, "Filtered flow"), 5.5, 1e-12);
  pid.set_SP_Q(2);
  pid.iterate();
  EXPECT_NEAR(value(pid, "Filtered flow"), 3.25, 1e-12);
}
TEST(PIDValveTest, ZeroGainIsFiniteAndServoOutputStaysBounded) {
  PIDValve pid({.Q_wanted = 1, .SP_Q = 100, .Kp = 0, .Ti = 1}, 0.1);
  pid.set_controls(true, true, true, 0, 10, -1);
  for (int i = 0; i < 200; ++i) {
    pid.iterate();
    EXPECT_TRUE(std::isfinite(pid.get_output()));
    EXPECT_GE(pid.get_output(), 0);
    EXPECT_LE(pid.get_output(), 1);
  }
  pid.set_controls(true, false, true, 0.3, 10, -1);
  pid.iterate();
  EXPECT_DOUBLE_EQ(value(pid, "Raw PID output"), 0.3);
}
TEST(PIDValveTest, DefaultConstructorMatchesExplicitDefaults) {
  PIDValve default_pid;
  PIDValve explicit_pid(PIDValveParameters{}, 0.1);
  for (int i = 0; i < 20; ++i) {
    default_pid.iterate();
    explicit_pid.iterate();
    EXPECT_TRUE(std::isfinite(default_pid.get_output()));
    EXPECT_DOUBLE_EQ(default_pid.get_output(), explicit_pid.get_output());
    EXPECT_DOUBLE_EQ(value(default_pid, "Integral"),
                     value(explicit_pid, "Integral"));
  }
}
}  // namespace
