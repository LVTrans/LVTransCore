#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <memory>
#include "bench_app.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/pid_valve.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"
#include "lvtrans/plant.hpp"

using Clock = std::chrono::steady_clock;
using namespace lvtrans;

auto plant1(double DT) {
  auto plant = std::make_unique<Plant>(DT);
  plant->set_name("Plant 1");
  ReservoirParameters reservoir_params{.H0 = 10.0, .cvp = 100e3, .cvm = 100e3};
  ReservoirParameters reservoir_params2{.H0 = 0.0, .cvp = 100e3, .cvm = 100e3};
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

  auto left = plant->add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe = plant->add_element<Pipe>(pipe_params, DT).value();
  auto right =
      plant->add_element<ConstantLevelRight>(reservoir_params2).value();
  HydraulicConnections::connect(*pipe, PipeEnd::Left, *left);
  HydraulicConnections::connect(*pipe, PipeEnd::Right, *right);
  return plant;
}

auto plant2(double DT) {
  auto plant = std::make_unique<Plant>(DT);
  plant->set_name("Plant 2");
  ReservoirParameters reservoir_params{
      .H0 = 100.000000E+0, .cvp = 100.000000E+3, .cvm = 100.000000E+3};
  ReservoirParameters reservoir_params2{
      .H0 = 0.0, .cvp = 100.000000E+3, .cvm = 100.000000E+3};

  PipeParameters pipe_params = {
      .H0 = 0.0,
      .Q0 = 0.0,
      .length = 1000,
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

  LossCoefficients simple_connection_params{.cvp = 100e3, .cvm = 100e3};

  ValveInternalServoParameters valve_params{.cvp = 10e3,
                                            .cvm = 10e3,
                                            .closing_time_high = 10,
                                            .closing_time_low = 10,
                                            .switch_over = 500e-3,
                                            .servo_time = 1.0,
                                            .initial_opening = 1.0,
                                            .diameter = 1.0};

  auto reservoir_left =
      plant->add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe1 = plant->add_element<Pipe>(pipe_params, DT).value();
  auto simple_conn1 =
      plant->add_element<SimpleConnection>(simple_connection_params).value();
  auto valve2 =
      plant->add_element<ValveInternalServo>(valve_params, DT).value();
  auto pipe2 = plant->add_element<Pipe>(pipe_params, DT).value();
  auto pipe3 = plant->add_element<Pipe>(pipe_params, DT).value();
  auto pipe4 = plant->add_element<Pipe>(pipe_params, DT).value();
  auto simple_conn2 =
      plant->add_element<SimpleConnection>(simple_connection_params).value();
  auto reservoir_right =
      plant->add_element<ConstantLevelRight>(reservoir_params2).value();

  HydraulicConnections::connect(*pipe1, PipeEnd::Left, *reservoir_left);
  HydraulicConnections::connect(*pipe1, PipeEnd::Right, *simple_conn1);

  HydraulicConnections::connect(*pipe2, PipeEnd::Left, *simple_conn1);
  HydraulicConnections::connect(*pipe2, PipeEnd::Right, *simple_conn2);

  HydraulicConnections::connect(*pipe3, PipeEnd::Left, *simple_conn2);
  HydraulicConnections::connect(*pipe3, PipeEnd::Right, *valve2);

  HydraulicConnections::connect(*pipe4, PipeEnd::Left, *valve2);
  HydraulicConnections::connect(*pipe4, PipeEnd::Right, *reservoir_right);
  return plant;
};

auto plant3() {
  constexpr double dt = 0.1;
  auto plant = std::make_unique<Plant>(dt);

  const ReservoirParameters res_a{.H0 = 100.0, .cvp = 100e3, .cvm = 100e3};
  const ReservoirParameters res_b{.H0 = 0.0, .cvp = 100e3, .cvm = 100e3};
  const LossCoefficients connection_params{.cvp = 100e3, .cvm = 100e3};
  const PipeParameters pipe_params{.H0 = 0.0,
                                   .Q0 = 0.0,
                                   .length = 1000.0,
                                   .diameter = 1.0,
                                   .area = 785.4e-3,
                                   .periphery = 3.1416,
                                   .epsilon = 10e-3,
                                   .ny = 1.1e-6,
                                   .rho = 1e3,
                                   .f = 0.01,
                                   .a = 1200.0,
                                   .z0 = 0.0,
                                   .z1 = 0.0,
                                   .lambda = 500e3,
                                   .f_max = 100.0,
                                   .use_diameter = true,
                                   .use_full_moody = false,
                                   .use_Dh = false};
  const ValveInternalServoParameters valve_params{.cvp = 10e3,
                                                  .cvm = 10e3,
                                                  .closing_time_high = 10.0,
                                                  .closing_time_low = 10.0,
                                                  .switch_over = 0.5,
                                                  .servo_time = 1.0,
                                                  .initial_opening = 1.0,
                                                  .use_cv_file = false,
                                                  .cv_file = "cv-file.txt",
                                                  .diameter = 1.0};
  const PIDValveParameters pid_params{
      .Q_wanted = 6.0, .SP_Q = 6.0, .Kp = 1.0, .Ti = 10.0, .Td = 0.0};

  auto* pid = plant->add_element<PIDValve>(pid_params, dt).value();
  auto* valve =
      plant->add_element<ValveInternalServo>(valve_params, dt).value();
  auto* reservoir_left = plant->add_element<ConstantLevelLeft>(res_a).value();
  auto* pipe1 = plant->add_element<Pipe>(pipe_params, dt).value();
  auto* connection1 =
      plant->add_element<SimpleConnection>(connection_params).value();
  auto* pipe2 = plant->add_element<Pipe>(pipe_params, dt).value();
  auto* connection2 =
      plant->add_element<SimpleConnection>(connection_params).value();
  auto* pipe3 = plant->add_element<Pipe>(pipe_params, dt).value();
  auto* pipe4 = plant->add_element<Pipe>(pipe_params, dt).value();
  auto* reservoir_right = plant->add_element<ConstantLevelRight>(res_b).value();

  HydraulicConnections::connect(*pipe1, PipeEnd::Left, *reservoir_left);
  HydraulicConnections::connect(*pipe1, PipeEnd::Right, *connection1);
  HydraulicConnections::connect(*pipe2, PipeEnd::Left, *connection1);
  HydraulicConnections::connect(*pipe2, PipeEnd::Right, *connection2);
  HydraulicConnections::connect(*pipe3, PipeEnd::Left, *connection2);
  HydraulicConnections::connect(*pipe3, PipeEnd::Right, *valve);
  HydraulicConnections::connect(*pipe4, PipeEnd::Left, *valve);
  HydraulicConnections::connect(*pipe4, PipeEnd::Right, *reservoir_right);

  ControllerConnections::connect(*pid, *valve);

  double sp = 7.0, kp = 0.5, ti = 5.0, td = 0.0, tt = 10.0, tf = -1.0,
         manual = 0.5;
  bool active = true, automatic = true, aw = true;

  pid->set_SP_Q(sp);
  pid->set_gains(kp, ti, td);
  pid->set_controls(active, automatic, aw, manual, tt, tf);

  plant->set_name("Plant 3");

  return plant;
}

int main(int argc, char** argv) {
  int bench_plant = 1;
  double dt = 0.1;
  if (argc > 1) {
    bench_plant = std::stoi(argv[1]);
  }

  if (argc > 2) {
    dt = std::stod(argv[2]);
  }

  std::unique_ptr<Plant> plant = nullptr;

  switch (bench_plant) {
    case 1:
      plant = plant1(dt);
      break;
    case 2:
      plant = plant2(dt);
      break;
    case 3:
      plant = plant3();
      break;
  }

  std::cout << "Plant name: " << plant->get_name() << '\n';
  std::cout << "dt: " << plant->get_step_size() << '\n';
  std::cout << "Number of elements: " << plant->get_elements().size() << '\n';
  BenchApp app(*plant);
  if (app.init() != 0) {
    std::cerr << "Failed to initialize GUI application.\n";
    return 1;
  }

  app.spin();
}
