#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <span>
#include <type_traits>
#include <vector>
#include "gui_app.hpp"
#include "helpers.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"
#include "lvtrans/hydraulic_connections.hpp"
#include "lvtrans/plant.hpp"
#include "plot_data.hpp"

using namespace lvtrans;
auto plant1() {
  constexpr double dt = 0.1;
  auto plant = std::make_unique<Plant>(dt);
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
  auto pipe = plant->add_element<Pipe>(pipe_params, dt).value();
  auto right =
      plant->add_element<ConstantLevelRight>(reservoir_params2).value();
  HydraulicConnections::connect(*pipe, PipeEnd::Left, *left);
  HydraulicConnections::connect(*pipe, PipeEnd::Right, *right);
  return plant;
}

auto plant2() {
  constexpr double dt = 0.1;
  auto plant = std::make_unique<Plant>(dt);
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
                                            .use_cv_file = false,
                                            .cv_file = "cv-file.txt",
                                            .diameter = 1.0};

  auto reservoir_left =
      plant->add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe1 = plant->add_element<Pipe>(pipe_params, dt).value();
  auto simple_conn1 =
      plant->add_element<SimpleConnection>(simple_connection_params).value();
  auto valve = plant->add_element<ValveInternalServo>(valve_params, dt).value();
  auto pipe2 = plant->add_element<Pipe>(pipe_params, dt).value();
  auto pipe3 = plant->add_element<Pipe>(pipe_params, dt).value();
  auto pipe4 = plant->add_element<Pipe>(pipe_params, dt).value();
  auto simple_conn2 =
      plant->add_element<SimpleConnection>(simple_connection_params).value();
  auto reservoir_right =
      plant->add_element<ConstantLevelRight>(reservoir_params2).value();

  reservoir_left->set_name("Reservoir Left");
  pipe1->set_name("Pipe 1");
  pipe2->set_name("Pipe 2");
  pipe3->set_name("Pipe 3");
  pipe4->set_name("Pipe 4");
  simple_conn1->set_name("Simple Connection 1");
  simple_conn2->set_name("Simple Connection 2");
  valve->set_name("Valve");

  HydraulicConnections::connect(*pipe1, PipeEnd::Left, *reservoir_left);
  HydraulicConnections::connect(*pipe1, PipeEnd::Right, *simple_conn1);

  HydraulicConnections::connect(*pipe2, PipeEnd::Left, *simple_conn1);
  HydraulicConnections::connect(*pipe2, PipeEnd::Right, *simple_conn2);

  HydraulicConnections::connect(*pipe3, PipeEnd::Left, *simple_conn2);
  HydraulicConnections::connect(*pipe3, PipeEnd::Right, *valve);

  HydraulicConnections::connect(*pipe4, PipeEnd::Left, *valve);
  HydraulicConnections::connect(*pipe4, PipeEnd::Right, *reservoir_right);
  return plant;
};

int main() {
  auto plant = plant2();
  GuiApp app(*plant);
  if (app.init() != 0) {
    std::cerr << "Failed to initialize GUI application.\n";
    return 1;
  }

  app.spin();
}
