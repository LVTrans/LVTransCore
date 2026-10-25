#include <chrono>
#include <cstdint>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/plant.hpp"
int main() {
  using namespace lvtrans;
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

  double dt{0.1};

  Plant plant(dt);

  auto reservoir_left =
      plant.add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe = plant.add_element<Pipe>(pipe_params, dt).value();
  auto reservoir_right =
      plant.add_element<ConstantLevelRight>(reservoir_params).value();

  HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir_left);
  HydraulicConnections::connect(*pipe, PipeEnd::Right, *reservoir_right);

  uint64_t iterations{};
  auto start = std::chrono::steady_clock::now();

  while (std::chrono::steady_clock::now() < start + std::chrono::seconds(5)) {
    plant.step();
    ++iterations;
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;

  std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
  std::cout << "Iterations: " << iterations << "\n";
  std::cout << "Iterations per second: " << iterations / elapsed_seconds.count()
            << "\n";
  return 0;
}
