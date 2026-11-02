#pragma once

#include <GLFW/glfw3.h>
#include <implot.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
#include <span>
#include "implot.h"

struct BenchStats {
  using Clock = std::chrono::steady_clock;
  Clock::time_point last_tick{};
  std::uint64_t first_steps = 0;
  std::uint64_t sample_steps = 0;
  double active_seconds = 0.0;
  double sample_seconds = 0.0;
  double iterations_per_second = 0.0;
  double x_realtime = 0.0;

  void reset(std::uint64_t steps) {
    first_steps = sample_steps = steps;
    active_seconds = sample_seconds = 0.0;
    iterations_per_second = x_realtime = 0.0;
    last_tick = Clock::now();
  }

  void resume(std::uint64_t steps) {
    last_tick = Clock::now();  // Do not count time spent paused.
    sample_steps = steps;
    sample_seconds = active_seconds;
    iterations_per_second = 0.0;
  }

  void update(std::uint64_t steps, double dt) {
    const auto now = Clock::now();
    active_seconds += std::chrono::duration<double>(now - last_tick).count();
    last_tick = now;

    if (active_seconds > 0.0)
      x_realtime = (steps - first_steps) * dt / active_seconds;

    const double interval = active_seconds - sample_seconds;
    if (interval >= 1.0) {
      iterations_per_second = (steps - sample_steps) / interval;
      sample_steps = steps;
      sample_seconds = active_seconds;
    }
  }
};

inline void setup_y_limits(std::span<const double> values) {
  bool has_value = false;
  double minimum = 0.0;
  double maximum = 0.0;

  for (double value : values) {
    if (!std::isfinite(value)) continue;
    if (!has_value) {
      minimum = maximum = value;
      has_value = true;
    } else {
      minimum = std::min(minimum, value);
      maximum = std::max(maximum, value);
    }
  }

  if (!has_value) return;

  const double range = maximum - minimum;
  const double magnitude = std::max(std::abs(minimum), std::abs(maximum));
  // At least 0.1 units of total span keeps the two-decimal labels readable.
  const double padding = std::max({range * 0.05, magnitude * 0.05, 0.05});
  ImPlot::SetupAxisLimits(ImAxis_Y1, minimum - padding, maximum + padding,
                          ImPlotCond_Always);
}

inline int TimeFormatter(double value, char* buff, int size, void* user_data) {
  unsigned long long t = static_cast<unsigned long long>(value);

  std::string time_str =
      std::format("{:02}:{:02}:{:02}", t / 3600, (t / 60) % 60, t % 60);

  return std::snprintf(buff, size, "%s", time_str.c_str());
}
