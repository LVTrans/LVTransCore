#include <benchmark/benchmark.h>
#include <stdexcept>
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve.hpp"
#include "lvtrans/plant.hpp"

static lvtrans::Pipe* add_pipe_group(lvtrans::Plant& plant, double dt) {
  using namespace lvtrans;

  auto* reservoir = plant.add_element<ConstantLevelLeft>(150.0).value();
  auto* pipe = plant
                   .add_element<Pipe>(
                       PipeParameters{
                           .H0 = 150.0,
                           .length = 600.0,
                           .diameter = 0.5,
                           .f = 0.018,
                           .a = 1200.0,
                           .z0 = 0.0,
                           .z1 = 0.0,
                       },
                       dt)
                   .value();

  auto* valve = plant
                    .add_element<Valve>(ValveParameters{
                        .tau_i = 0.0,
                        .tau_f = 0.0,
                        .tc = 0.0,
                        .cvp = 0.0001,
                    })
                    .value();

  HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir);

  HydraulicConnections::connect(*pipe, PipeEnd::Right, *valve);
  return pipe;
}

static void BM_PlantStepManyPipes001Dt(benchmark::State& state) {
  const double dt = 0.01;
  lvtrans::Plant plant(dt);
  for (int64_t i = 0; i < state.range(0); ++i) {
    add_pipe_group(plant, dt);
  }

  for (auto _ : state) {
    plant.step();
  }
}

BENCHMARK(BM_PlantStepManyPipes001Dt)->Arg(1)->Arg(10)->Arg(100);

static void BM_LabVIEWCompare(benchmark::State& state) {
  using namespace lvtrans;
  const double dt = 1.0 / static_cast<double>(state.range(0));

  lvtrans::Plant plant(dt);

  auto reservoir_left =
      plant.add_element<ConstantLevelLeft>(ReservoirParameters{}).value();
  auto pipe1 = plant.add_element<Pipe>(PipeParameters{}, dt).value();
  auto simple_conn1 = plant.add_element<SimpleConnection>().value();
  auto valve = plant.add_element<Valve>(ValveParameters{}).value();
  auto pipe2 = plant.add_element<Pipe>(PipeParameters{}, dt).value();
  auto pipe3 = plant.add_element<Pipe>(PipeParameters{}, dt).value();
  auto pipe4 = plant.add_element<Pipe>(PipeParameters{}, dt).value();
  auto simple_conn2 = plant.add_element<SimpleConnection>().value();
  auto reservoir_right =
      plant.add_element<ConstantLevelRight>(ReservoirParameters{}).value();

  // clang-format off
  // Reservoir 1 <->Pipe 1 <->Simple Connection 1 <-->Pipe 2 <->Simple Connection 2 <->Pipe 3 <->Valve <->Pipe 4 <->Reservoir 2
  //
  // Adjust dt : 0.1->0.01->0.001->0.0001
  HydraulicConnections::connect(*pipe1, PipeEnd::Left, *reservoir_left);
  HydraulicConnections::connect(*pipe1, PipeEnd::Right, *simple_conn1);

  HydraulicConnections::connect(*pipe2, PipeEnd::Left, *simple_conn1);
  HydraulicConnections::connect(*pipe2, PipeEnd::Right, *simple_conn2);

  HydraulicConnections::connect(*pipe3, PipeEnd::Left, *simple_conn2);
  HydraulicConnections::connect(*pipe3, PipeEnd::Right, *valve);

  HydraulicConnections::connect(*pipe4, PipeEnd::Left, *valve);
  HydraulicConnections::connect(*pipe4, PipeEnd::Right, *reservoir_right);

  for (int i = 0; i < 1000; ++i) {
    plant.step();
  }
}

BENCHMARK(BM_LabVIEWCompare)->Arg(1)->Arg(10)->Arg(100);
