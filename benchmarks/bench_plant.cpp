#include <benchmark/benchmark.h>
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve.hpp"
#include "lvtrans/plant.hpp"

static void BM_LabVIEWCompareA(benchmark::State& state) {
  using namespace lvtrans;
  const double dt = 1.0 / static_cast<double>(state.range(0));

  lvtrans::Plant plant(dt);

  ReservoirParameters reservoir_params{.H0 = 100.0, .cvp = 100e3, .cvm = 100e3};
  PipeParameters pipe_params{.H0 = 0.0,
                             .Q0 = 0.0,
                             .length = 100,
                             .diameter = 1,
                             .area = 785.400000E-3,
                             .periphery = 3.141600,
                             .epsilon = 10e-3,
                             .ny = 1.1e-6,
                             .rho = 1e3,
                             .f = 10e-3,
                             .a = 1.2e3,
                             .z0 = 0,
                             .z1 = 0,
                             .lambda = 500e3,
                             .f_max = 100.0,
                             .use_diameter = true,
                             .use_full_moody = false,
                             .use_Dh = false};

  auto left = plant.add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe = plant.add_element<Pipe>(pipe_params, dt).value();
  auto right = plant.add_element<ConstantLevelRight>(reservoir_params).value();
  HydraulicConnections::connect(*pipe, PipeEnd::Left, *left);
  HydraulicConnections::connect(*pipe, PipeEnd::Right, *right);

  for (auto _ : state) {
    plant.step();
  }
}

BENCHMARK(BM_LabVIEWCompareA)->Arg(10)->Arg(100)->Arg(1000);

static void BM_LabVIEWCompareB(benchmark::State& state) {
  using namespace lvtrans;
  const double dt = 1.0 / static_cast<double>(state.range(0));

  lvtrans::Plant plant(dt);

  ReservoirParameters reservoir_params{
      .H0 = 100.000000E+0, .cvp = 100.000000E+3, .cvm = 100.000000E+3};

  PipeParameters pipe_params = {
      .H0 = 0.0,
      .Q0 = 0.0,
      .length = 100,
      .diameter = 1,
      .area = 785.400000E-3,
      .periphery = 3.141600E+0,
      .epsilon = 10.000000E-3,
      .ny = 1.1e-6,
      .rho = 1e3,
      .f = 10e-3,
      .a = 1.2e3,
      .z0 = 0,
      .z1 = 0,
      .lambda = 500e3,
      .f_max = 100.0,
      .use_diameter = true,
      .use_full_moody = false,
      .use_Dh = false,
  };
  LossCoefficients simple_connection_params{.cvp = 1e5, .cvm = 1e5};

  ValveParameters valve_params{.tau_i = 1.0,
                               .tau_f = 0.0,
                               .tc = 2.1,
                               .em = 0.75,
                               .cda0 = 0.009,
                               .cvp = 1e5,
                               .cvm = 1e5,
                               .closing_time_h = 10,
                               .closing_time_l = 10,
                               .switch_over = 0.5,
                               .tks = 1.0,
                               .tau_0 = 1.0,
                               .diameter = 1.0};

  auto reservoir_left =
      plant.add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe1 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto simple_conn1 =
      plant.add_element<SimpleConnection>(simple_connection_params).value();
  auto valve2 = plant.add_element<Valve>(valve_params).value();
  auto pipe22 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto pipe3 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto pipe4 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto simple_conn2 =
      plant.add_element<SimpleConnection>(simple_connection_params).value();
  auto reservoir_right =
      plant.add_element<ConstantLevelRight>(reservoir_params).value();

  // clang-format off
    // Reservoir 1 <->Pipe 1 <->Simple Connection 1 <-->Pipe 2 <->Simple Connection 2 <->Pipe 3 <->Valve <->Pipe 4 <->Reservoir 2
    HydraulicConnections::connect(*pipe1, PipeEnd::Left, *reservoir_left);
    HydraulicConnections::connect(*pipe1, PipeEnd::Right, *simple_conn1);

    HydraulicConnections::connect(*pipe22, PipeEnd::Left, *simple_conn1);
    HydraulicConnections::connect(*pipe22, PipeEnd::Right, *simple_conn2);

    HydraulicConnections::connect(*pipe3, PipeEnd::Left, *simple_conn2);
    HydraulicConnections::connect(*pipe3, PipeEnd::Right, *valve2);

    HydraulicConnections::connect(*pipe4, PipeEnd::Left, *valve2);
    HydraulicConnections::connect(*pipe4, PipeEnd::Right, *reservoir_right);

  for (auto _ : state) {
    plant.step();
  }
}


BENCHMARK(BM_LabVIEWCompareB)->Arg(10)->Arg(100)->Arg(1000);
