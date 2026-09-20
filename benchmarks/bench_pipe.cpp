#include <benchmark/benchmark.h>
#include "lvtrans/pipe.hpp"
#include "lvtrans/reservoir.hpp"
#include "lvtrans/valve.hpp"

static void BM_PipeIterate(benchmark::State& state) {
  using namespace lvtrans;

  const size_t reaches = static_cast<int>(state.range(0));
  auto reservoir = std::make_shared<Reservoir>(150.0);
  auto valve = std::make_shared<Valve>(ValveConfig{});

  Pipe pipe(PipeConfig{600.0, 0.5, 0.018, 1200.0, reaches, 10.0, 15.0}, 150.0,
            0.0);

  pipe.connect_left(*reservoir);
  pipe.connect_right(*valve);

  for (auto _ : state) {
    pipe.iterate();
  }
}

BENCHMARK(BM_PipeIterate)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1'000)
    ->Arg(10'000)
    ->Arg(100'000);

BENCHMARK_MAIN();
