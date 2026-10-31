#include <gtest/gtest.h>
#include "../../examples/gui/plot_data.hpp"

namespace {
std::vector<lvtrans::ElementView> sample_views(double value) {
  return {{.element_id = 1,
           .values = {lvtrans::ScalarValue{
                          .name = "Q", .unit = "m3/s", .value = value},
                      lvtrans::VectorValue{.name = "Profile",
                                           .values = {value, -value}}}}};
}

TEST(GuiPlotSampling, SamplesEveryTenStepsDespiteTimeRounding) {
  PlotContainer plots;
  double time = 0.0;
  for (std::uint64_t step = 0; step <= 29; ++step) {
    plots.sample(step, time, sample_views(static_cast<double>(step)));
    time += 0.1;
  }
  const auto& history = plots.histories.at({1, "Q", "", "m3/s"});
  ASSERT_EQ(history.count, 3);
  EXPECT_DOUBLE_EQ(history.values[0], 0.0);
  EXPECT_DOUBLE_EQ(history.values[1], 10.0);
  EXPECT_DOUBLE_EQ(history.values[2], 20.0);
  EXPECT_NEAR(history.time[1], 1.0, 1e-14);
  EXPECT_NEAR(history.time[2], 2.0, 1e-14);
  EXPECT_FALSE(plots.sample_due(29));
  EXPECT_TRUE(plots.sample_due(30));
  EXPECT_DOUBLE_EQ(
      std::get<lvtrans::VectorValue>(plots.views[0].values[1]).values[0], 20.0);
}

TEST(GuiPlotSampling, EveryStepModePreservesAlternatingValuesAcrossWraparound) {
  PlotContainer plots;
  plots.set_sampling_steps(1);
  for (std::uint64_t step = 0; step <= 250; ++step) {
    plots.sample(step, static_cast<double>(step) * 0.1,
                 sample_views(step % 2 == 0 ? -0.5 : 0.5));
  }
  const auto& history = plots.histories.at({1, "Q", "", "m3/s"});
  ASSERT_EQ(history.count, PlotContainer::capacity);
  for (int i = 0; i < history.count; ++i) {
    const int index = (history.offset + i) % PlotContainer::capacity;
    const int step = 51 + i;
    EXPECT_DOUBLE_EQ(history.time[index], step * 0.1);
    EXPECT_DOUBLE_EQ(history.values[index], step % 2 == 0 ? -0.5 : 0.5);
  }
}

TEST(GuiPlotSampling, PauseDoesNotDuplicateSamplesAndResetRetainsInterval) {
  PlotContainer plots;
  plots.set_sampling_steps(5);
  plots.sample(100, 10.0, sample_views(1.0));
  plots.sample(100, 10.0, sample_views(2.0));
  const auto& history = plots.histories.at({1, "Q", "", "m3/s"});
  EXPECT_EQ(history.count, 1);
  EXPECT_DOUBLE_EQ(history.values[0], 1.0);
  plots.reset();
  EXPECT_TRUE(plots.views.empty());
  EXPECT_TRUE(plots.histories.empty());
  EXPECT_TRUE(plots.sample_due(0));
  plots.sample(0, 0.0, sample_views(0.0));
  EXPECT_FALSE(plots.sample_due(4));
  EXPECT_TRUE(plots.sample_due(5));
}

TEST(GuiPlotSampling, IntervalChangeStartsNewHistoryAtCurrentStep) {
  PlotContainer plots;
  plots.sample(0, 0.0, sample_views(0.0));
  plots.set_sampling_steps(20);
  EXPECT_TRUE(plots.histories.empty());
  plots.sample(13, 1.3, sample_views(13.0));
  EXPECT_FALSE(plots.sample_due(32));
  EXPECT_TRUE(plots.sample_due(33));
}
}  // namespace
