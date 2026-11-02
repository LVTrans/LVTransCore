#include <chrono>
#include <cstdint>
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/pipe.hpp"
#include "lvtrans/plant.hpp"
int main() {
  using namespace lvtrans;
  double dt{0.1};

  Plant plant(dt);
  ReservoirParameters reservoir_params{.H0 = 100.0, .cvp = 100e3, .cvm = 100e3};
  ReservoirParameters reservoir_params2{
      .H0 = 100.0, .cvp = 100e3, .cvm = 100e3};
  PipeParameters pipe_params{.H0 = 0.0,
                             .Q0 = 0.0,
                             .length = 1000,
                             .diameter = 1,
                             .area = 785.400000e-3,
                             .periphery = 3.141600,
                             .epsilon = 10e-3,
                             .ny = 1.1e-6,
                             .rho = 1e3,
                             .f = 0.01,
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
  auto right = plant.add_element<ConstantLevelRight>(reservoir_params2).value();
  HydraulicConnections::connect(*pipe, PipeEnd::Left, *left);
  HydraulicConnections::connect(*pipe, PipeEnd::Right, *right);

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
