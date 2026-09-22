#include <benchmark/benchmark.h>
#include "lvtrans/pipe.hpp"
#include "lvtrans/reservoir.hpp"
#include "lvtrans/valve.hpp"

static void BM_PipeIterate(benchmark::State& state) {
  using namespace lvtrans;

  const size_t reaches = static_cast<int>(state.range(0));
  auto reservoir = std::make_shared<Reservoir>(150.0);
  auto valve = std::make_shared<Valve>(ValveParameters{});

  Pipe pipe(
      PipeParameters{
          .length = 600.0,
          .diameter = 0.5,
          .f = 0.018,
          .a = 1200.0,
          .z0 = 0.0,
          .z1 = 0.0,
          .num_reaches = reaches,
      },
      150.0, 0.0);

  pipe.connect_to(reservoir.get(), PortType::Left, PortType::Right);
  pipe.connect_to(valve.get(), PortType::Right, PortType::Left);

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
