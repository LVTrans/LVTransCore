#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <lvtrans/elements/pipe.hpp>
#include <lvtrans/elements/reservoir.hpp>
#include <lvtrans/elements/valve.hpp>
#include <memory>
#include "lvtrans/const.hpp"
#include "lvtrans/elements/element.hpp"
#include "lvtrans/plant.hpp"
#include "lvtrans/port.hpp"

inline double a = 1200.0;   // Wave propagation velocity [m/s]
inline double f = 0.018;    // Darcy-Weisbach friction factor
inline double tau_i = 1.0;  // Initial valve position
inline double tau_f = 0.0;  // Final valve position
inline double tc = 2.1;     // Valve operating/closure time [s]
inline double em = 0.75;    // Exponent defining valve motion

using namespace lvtrans;
using SystemElemnts = std::vector<std::shared_ptr<Element>>;

int main() {
  double HR = 150.0;    // Reservoir head above datum [m]
  double Tmax = 4.3;    // Duration of transient [s]
  double CdA0 = 0.009;  // Valve coefficient/opening parameter

  PipeParameters pipe_config = {
      .length = 600.0,
      .diameter = 0.5,
      .f = f,
      .a = a,
      .z0 = 10,
      .z1 = 15,
      .num_reaches = 10,
  };

  std::ofstream output_file("output.csv");

  if (!output_file) {
    std::cerr << "Error opening output file.\n";
    return 1;
  }

  const double dx = calculate_dx(pipe_config.length, pipe_config.num_reaches);

  const double dt = dx / a;

  const double system_dt = 2.0 * dt;

  Plant plant(system_dt);

  std::vector<double> H0_(pipe_config.num_reaches + 1, 0.0);
  std::vector<double> Q0_(pipe_config.num_reaches + 1, 0.0);

  const double area = calculate_pipe_area(pipe_config.diameter);
  const double R = calculate_R(pipe_config.f, dx, pipe_config.diameter, area);

  const double Q0 =
      std::sqrt(2.0 * consts::g * CdA0 * CdA0 * HR /
                (R * static_cast<double>(pipe_config.num_reaches) * 2.0 *
                     consts::g * CdA0 * CdA0 +
                 1.0));

  const double H0 =
      HR - R * static_cast<double>(pipe_config.num_reaches) * Q0 * Q0;

  auto reservoir = plant.add_element<Reservoir>(HR);

  const double Qi =
      std::sqrt(HR * Q0 * Q0 * tau_i * tau_i /
                (R * static_cast<double>(pipe_config.num_reaches) * Q0 * Q0 *
                     tau_i * tau_i +
                 H0));

  for (size_t i = 0; i <= pipe_config.num_reaches; i += 2) {
    H0_[i] = HR - static_cast<double>(i) * R * Qi * Qi;
    Q0_[i] = Qi;
  }

  auto pipe = plant.add_element<Pipe>(pipe_config, H0_, Q0_);

  const double CVP = 0.5 * Q0 * Q0 / H0;

  ValveParameters valve_config{};
  valve_config.tau_i = tau_i;
  valve_config.tau_f = tau_f;
  valve_config.tc = tc;
  valve_config.em = em;
  valve_config.cvp = CVP;

  auto valve = plant.add_element<Valve>(valve_config);

  pipe->connect_to(reservoir, PortType::Left, PortType::Right);
  pipe->connect_to(valve, PortType::Right, PortType::Left);

  const int Kmax = static_cast<int>(0.5 * Tmax / dt) + 1;

  for (int k = 1; k < Kmax; ++k) {
    plant.step();

    output_file << plant.get_current_time() << "," << valve->get_tau() << ","
                << pipe->get_H()[pipe_config.num_reaches] << ","
                << pipe->get_Q()[pipe_config.num_reaches] << '\n';
  }
  output_file.close();
  plant.display();

  return 0;
}
