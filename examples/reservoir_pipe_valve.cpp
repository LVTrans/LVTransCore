#include "lvtrans/const.hpp"
#include "lvtrans/element.hpp"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <lvtrans/pipe.hpp>
#include <lvtrans/reservoir.hpp>
#include <lvtrans/valve.hpp>
#include <memory>

inline double a = 1200.0;  // Wave propagation velocity [m/s]
inline double f = 0.018;   // Darcy-Weisbach friction factor
inline double tau_i = 1.0; // Initial valve position
inline double tau_f = 0.0; // Final valve position
inline double tc = 2.1;    // Valve operating/closure time [s]
inline double em = 0.75;   // Exponent defining valve motion

using namespace lvtrans;
using SystemElemnts = std::vector<std::shared_ptr<Element>>;

int main() {
  double HR = 150.0;   // Reservoir head above datum [m]
  double Tmax = 4.3;   // Duration of transient [s]
  double CdA0 = 0.009; // Valve coefficient/opening parameter

  PipeConfig pipe_config = {
      .length = 600.0,
      .diameter = 0.5,
      .f = f,
      .a = a,
      .num_reaches = 10,
      .z0 = 10,
      .z1 = 15,
  };

  int IPR = 1; // Output interval
  std::ofstream output_file("output.csv");

  if (!output_file) {
    std::cerr << "Error opening output file." << std::endl;
    return 1;
  }

  const double dx = calculate_dx(pipe_config.length, pipe_config.num_reaches);

  const double dt = dx / a;

  // One complete staggered update consists of two dt steps.
  const double system_dt = 2.0 * dt;

  std::cout << "dx        = " << dx << " m\n";
  std::cout << "dt MOC    = " << dt << " s\n";
  std::cout << "system dt = " << system_dt << " s\n";

  output_file << "t,tau,H_valve,Q_valve\n";

  std::vector<double> H0_(pipe_config.num_reaches + 1, 0.0);
  std::vector<double> Q0_(pipe_config.num_reaches + 1, 0.0);

  const double area = calculate_pipe_area(pipe_config.diameter);
  const double R = calculate_R(pipe_config.f, dx, pipe_config.diameter, area);

  std::cout << "dx        = " << dx << " m\n";
  std::cout << "dt MOC    = " << dt << " s\n";
  std::cout << "system dt = " << system_dt << " s\n";

  const double Q0 = std::sqrt(
      2.0 * consts::g * CdA0 * CdA0 * HR /
      (R * pipe_config.num_reaches * 2.0 * consts::g * CdA0 * CdA0 + 1.0));

  const double H0 = HR - R * pipe_config.num_reaches * Q0 * Q0;

  auto reservoir = std::make_shared<Reservoir>(HR);

  const double Qi =
      std::sqrt(HR * Q0 * Q0 * tau_i * tau_i /
                (R * pipe_config.num_reaches * Q0 * Q0 * tau_i * tau_i + H0));

  for (int i = 0; i <= pipe_config.num_reaches; i += 2) {
    H0_[i] = HR - i * R * Qi * Qi;
    Q0_[i] = Qi;
    std::cout << "H0_[i] = " << H0_[i] << ", Q0_[i] = " << Q0_[i] << '\n';
  }

  auto pipe = std::make_shared<Pipe>(pipe_config, H0_, Q0_);

  const double CVP = 0.5 * Q0 * Q0 / H0;

  ValveConfig valve_config{
      .tau_i = tau_i, .tau_f = tau_f, .tc = tc, .em = em, .cvp = CVP};
  auto valve = std::make_shared<Valve>(valve_config);
  pipe->connect_left(reservoir);
  pipe->connect_right(valve);

  SystemElemnts system_elements{};
  system_elements.push_back(reservoir);
  system_elements.push_back(pipe);
  system_elements.push_back(valve);

  std::vector<std::shared_ptr<Pipe>> pipes = {pipe};
  std::vector<std::shared_ptr<NonPipe>> non_pipes = {reservoir, valve};

  assert(system_elements.size() == 3 &&
         "system_elements must contain 3 elements");

  const int Kmax = static_cast<int>(0.5 * Tmax / dt) + 1;

  for (int k = 1; k < Kmax; ++k) {
    const double t = 2.0 * dt * k;
    // 1. For all Pipes and (PID).
    //    1. calculate the internal variables; H Q and predict
    //    the A in (PID) for T1 based on T0
    //    2. Send C+, C- and A to each connected NP element
    for (const auto &pipe : pipes) {
      pipe->iterate();
    }

    // 2. For all NP elements

    //    1. Calculate all internal variables and the boundary values for T1
    //     based on T0 and values received from Pipe and (PID)
    //    2. Iterate each NP-PID pair once more to correct the predicted A if
    //    needed*.
    //    3. Send H, Q and B to each Pipe and (PID)

    // 3. Update the boundary H and Q for each pipe and B for the (PID)
    // 4. Start at 1 again for the next time step.
    // // Pipe
    IterateInput input{.t = t};
    for (const auto &non_pipe : non_pipes) {
      non_pipe->iterate(input);
    }

    output_file << t << "," << valve->get_tau() << ","
                << pipe->get_H()[pipe_config.num_reaches] << ","
                << pipe->get_Q()[pipe_config.num_reaches] << '\n';
  }
  output_file.close();

  return 0;
}
