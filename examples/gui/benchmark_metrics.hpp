#include <algorithm>
#include <chrono>
#include <cmath>
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
