#include <benchmark/benchmark.h>
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/reservoir.hpp"
#include "lvtrans/elements/valve.hpp"
#include "lvtrans/plant.hpp"

static lvtrans::Pipe* add_pipe_group(lvtrans::Plant& plant, float dt) {
  using namespace lvtrans;

  auto* reservoir = plant.add_element<ConstantLevelLeft>(150.0).value();
  auto* pipe = plant
                   .add_element<Pipe>(
                       PipeParameters{
                           .length = 600.0,
                           .diameter = 0.5,
                           .f = 0.018,
                           .a = 1200.0,
                           .z0 = 0.0,
                           .z1 = 0.0,
                       },
                       150.0, 0.0, dt)
                   .value();

  auto* valve = plant
                    .add_element<Valve>(ValveParameters{
                        .tau_i = 0.0,
                        .tau_f = 0.0,
                        .tc = 0.0,
                        .cvp = 0.0001,
                    })
                    .value();

  pipe->connect(reservoir);
  pipe->connect(valve);
  return pipe;
}

static void BM_PlantStepManyPipes001Dt(benchmark::State& state) {
  const auto dt = 0.01;
  lvtrans::Plant plant(dt);
  for (int64_t i = 0; i < state.range(0); ++i) {
    add_pipe_group(plant, dt);
  }

  for (auto _ : state) {
    plant.step();
  }
}

BENCHMARK(BM_PlantStepManyPipes001Dt)->Arg(1)->Arg(10)->Arg(100);

static void BM_ReadState(benchmark::State& state) {
  lvtrans::Plant plant(0.01);
  const auto pipe_id = add_pipe_group(plant, 100)->get_ID();

  for (auto _ : state) {
    auto view = plant.read_state(pipe_id);
    benchmark::DoNotOptimize(view);
  }
}

BENCHMARK(BM_ReadState);
