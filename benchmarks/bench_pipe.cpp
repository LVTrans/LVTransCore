#include <benchmark/benchmark.h>
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/elements/valve.hpp"

static void BM_PipeIterate(benchmark::State& state, double dt) {
  using namespace lvtrans;

  auto reservoir = std::make_shared<ConstantLevelLeft>(150.0).get();
  auto valve = std::make_shared<Valve>(ValveParameters{}).get();

  Pipe pipe(
      PipeParameters{
          .length = 600.0,
          .diameter = 0.5,
          .f = 0.018,
          .a = 1200.0,
          .z0 = 0.0,
          .z1 = 0.0,
      },
      150.0, 0.0, dt);

  pipe.connect(reservoir);
  pipe.connect(valve);

  for (auto _ : state) {
    pipe.iterate();
  }
}

BENCHMARK_CAPTURE(BM_PipeIterate, 0.1, 0.1);
BENCHMARK_CAPTURE(BM_PipeIterate, 0.01, 0.01);
BENCHMARK_CAPTURE(BM_PipeIterate, 0.001, 0.001);
BENCHMARK_CAPTURE(BM_PipeIterate, 0.0001, 0.0001);
BENCHMARK_CAPTURE(BM_PipeIterate, 0.00001, 0.00001);

BENCHMARK_MAIN();
