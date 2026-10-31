#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include "lvtrans/element_types.hpp"

struct PlotContainer {
  static constexpr int capacity = 200;
  using Key =
      std::tuple<lvtrans::ElementID, std::string, std::string, std::string>;

  struct History {
    std::array<double, capacity> time{};
    std::array<double, capacity> values{};
    int count = 0;
    int offset = 0;

    void append(double t, double value) {
      const int index = (offset + count) % capacity;
      time[index] = t;
      values[index] = value;
      if (count < capacity) {
        ++count;
      } else {
        offset = (offset + 1) % capacity;
      }
    }
  };

  std::map<Key, History> histories;
  std::vector<lvtrans::ElementView> views;

  static Key key(lvtrans::ElementID id, const lvtrans::ScalarValue& value) {
    return {id, value.name, value.symbol, value.unit};
  }

  bool sample_due(std::uint64_t step) const {
    return !last_sample_step || (step >= *last_sample_step &&
                                 step - *last_sample_step >= sampling_steps);
  }

  void set_sampling_steps(std::uint64_t steps) {
    sampling_steps = steps > 0 ? steps : 1;
    reset();
  }

  void sample(std::uint64_t step, double time,
              std::vector<lvtrans::ElementView> current_views) {
    if (!sample_due(step)) return;
    last_sample_step = step;
    views = std::move(current_views);
    for (const auto& view : views) {
      for (const auto& value : view.values) {
        if (const auto* scalar = std::get_if<lvtrans::ScalarValue>(&value)) {
          histories[key(view.element_id, *scalar)].append(time, scalar->value);
        }
      }
    }
  }

  void reset() {
    histories.clear();
    views.clear();
    last_sample_step.reset();
  }

 private:
  std::uint64_t sampling_steps = 10;  // 1 simulated second with dt = 0.1 s.
  std::optional<std::uint64_t> last_sample_step;
};
