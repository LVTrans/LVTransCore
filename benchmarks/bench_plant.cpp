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

  pipe->connect_to(reservoir, PortType::Left, PortType::Right);
  pipe->connect_to(valve, PortType::Right, PortType::Left);
  return pipe;
}

static void BM_PlantStep(benchmark::State& state, double dt) {
  lvtrans::Plant plant(dt);
  add_pipe_group(plant, dt);

  for (auto _ : state) {
    plant.step();
  }
}

BENCHMARK_CAPTURE(BM_PlantStep, 0.1, 0.1);
BENCHMARK_CAPTURE(BM_PlantStep, 0.01, 0.01);
BENCHMARK_CAPTURE(BM_PlantStep, 0.001, 0.001);
BENCHMARK_CAPTURE(BM_PlantStep, 0.0001, 0.0001);
BENCHMARK_CAPTURE(BM_PlantStep, 0.00001, 0.00001);

static void BM_PlantStepManyPipes(benchmark::State& state) {
  lvtrans::Plant plant(0.01);
  for (int64_t i = 0; i < state.range(0); ++i) {
    add_pipe_group(plant, 100);
  }

  for (auto _ : state) {
    plant.step();
    benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_PlantStepManyPipes)->Arg(1)->Arg(10)->Arg(100);

static void BM_ReadState(benchmark::State& state) {
  lvtrans::Plant plant(0.01);
  const auto pipe_id = add_pipe_group(plant, 100)->get_ID();

  for (auto _ : state) {
    auto view = plant.read_state(pipe_id);
    benchmark::DoNotOptimize(view);
  }
}

BENCHMARK(BM_ReadState);
