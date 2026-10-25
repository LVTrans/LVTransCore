#pragma once

#include <chrono>
#include <cstdint>

// Measures throughput over active (unpaused) wall-clock time.
// The current I/s value is a one-second sample. X Realtime is a cumulative
// average: simulated seconds / actual running seconds.
class BenchmarkMetrics {
 public:
  using Clock = std::chrono::steady_clock;

  double iterations_per_second = 0.0;
  double x_realtime = 0.0;
  double average_iterations_per_second = 0.0;

  void reset(std::uint64_t iterations, double simulation_time) {
    running_ = false;
    baseline_iterations_ = iterations;
    baseline_simulation_time_ = simulation_time;
    last_rate_iterations_ = iterations;
    last_realtime_iterations_ = iterations;
    active_seconds_ = 0.0;
    last_rate_active_seconds_ = 0.0;
    last_wall_tick_ = Clock::now();
    iterations_per_second = 0.0;
    average_iterations_per_second = 0.0;
    x_realtime = 0.0;
  }

  // Starting or resuming never adds idle/paused time to the measurements.
  // The cumulative speed and X Realtime continue from the original reset.
  void resume(std::uint64_t iterations) {
    if (running_) return;
    running_ = true;
    last_wall_tick_ = Clock::now();
    last_rate_active_seconds_ = active_seconds_;
    last_rate_iterations_ = iterations;
    iterations_per_second = 0.0;
  }

  void pause(std::uint64_t iterations, double simulation_time) {
    if (!running_) return;
    update(iterations, simulation_time);
    running_ = false;
    iterations_per_second = 0.0;
  }

  // Call after a batch of Plant::step() operations, not within each step.
  void update(std::uint64_t iterations, double simulation_time) {
    if (!running_) return;

    const auto now = Clock::now();
    const double seconds =
        std::chrono::duration<double>(now - last_wall_tick_).count();
    if (seconds > 0.0) active_seconds_ += seconds;
    last_wall_tick_ = now;

    if (active_seconds_ > 0.0) {
      const auto total_steps = iterations - baseline_iterations_;
      average_iterations_per_second =
          static_cast<double>(total_steps) / active_seconds_;
    }

    // Like the LVTrans I/s indicator, sample approximately once per second.
    const double sample_seconds = active_seconds_ - last_rate_active_seconds_;
    if (sample_seconds >= 1.0) {
      iterations_per_second =
          static_cast<double>(iterations - last_rate_iterations_) /
          sample_seconds;
      last_rate_iterations_ = iterations;
      last_rate_active_seconds_ = active_seconds_;
    }

    // The LabVIEW X Realtime display is sampled around every 100 iterations.
    // This check happens at each GUI batch boundary rather than each step.
    if (iterations - last_realtime_iterations_ >= 100) {
      if (active_seconds_ > 0.0) {
        x_realtime =
            (simulation_time - baseline_simulation_time_) / active_seconds_;
      }
      last_realtime_iterations_ = iterations;
    }
  }

  [[nodiscard]] bool running() const { return running_; }
  [[nodiscard]] double active_seconds() const { return active_seconds_; }

 private:
  bool running_ = false;
  Clock::time_point last_wall_tick_{Clock::now()};
  std::uint64_t baseline_iterations_ = 0;
  std::uint64_t last_rate_iterations_ = 0;
  std::uint64_t last_realtime_iterations_ = 0;
  double baseline_simulation_time_ = 0.0;
  double active_seconds_ = 0.0;
  double last_rate_active_seconds_ = 0.0;
};
