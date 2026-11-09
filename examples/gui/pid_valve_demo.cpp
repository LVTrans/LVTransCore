#include <cmath>
#include <iostream>
#include <memory>
#include <string_view>
#include "gui_app.hpp"
#include "helpers.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/pid_valve.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"

using namespace lvtrans;

auto plant1() {
  constexpr double dt = 0.05;
  auto plant = std::make_unique<Plant>(dt);

  auto* pid = plant
                  ->add_element<PIDValve>(PIDValveParameters{.Q_wanted = 2.0,
                                                             .SP_Q = 1.0,
                                                             .Kp = 0.5,
                                                             .Ti = 5.0,
                                                             .Td = 0.0},
                                          dt)
                  .value();

  ValveInternalServoParameters vp{};
  vp.cvp = vp.cvm = 0.02;
  vp.initial_opening = 0.5;
  vp.servo_time = 0.2;
  vp.closing_time_high = vp.closing_time_low = 10.0;
  auto* valve = plant->add_element<ValveInternalServo>(vp, dt).value();

  auto* reservoir = plant->add_element<ConstantLevelLeft>(100.0).value();
  PipeParameters pp{.H0 = 100.0,
                    .Q0 = 1.0,
                    .length = 600.0,
                    .diameter = 1.0,
                    .f = 0.02,
                    .a = 1200.0,
                    .use_diameter = true};
  auto* pipe = plant->add_element<Pipe>(pp, dt).value();
  reservoir->set_name("Upstream reservoir (100 m)");
  pipe->set_name("Pipe (600 m)");
  valve->set_name("Controlled valve to atmosphere");
  pid->set_name("Flow PID");

  HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir);
  HydraulicConnections::connect(*pipe, PipeEnd::Right, *valve);
  ControllerConnections::connect(*pid, *valve);

  pid->set_controls(true, true, true, 0.5, 10.0, -1.0);

  return plant;
}

auto plant2() {
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

  return plant;
}

int main(int argc, char** argv) {
  print_build_type();
  auto plant = plant2();
  auto pid = plant->get_element_by_id<PIDValve>(1).value();
  auto valve = plant->get_element_by_id<ValveInternalServo>(2).value();

  GuiApp app(*plant);
  if (app.init() != 0) {
    return 1;
  }
  app.max_speed = false;
  app.graph_interval = 0;
  double sp = 7.0, kp = 0.5, ti = 5.0, td = 0.0, tt = 3.0, tf = -1.0,
         manual = 0.5;
  bool active = true, automatic = true, aw = true;
  app.add_data_point();
  while (!glfwWindowShouldClose(app.window)) {
    glfwPollEvents();
    app.advance_plant();
    app.render();
    ImGui::SetNextWindowPos(ImVec2(370, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430, 480), ImGuiCond_FirstUseEver);
    ImGui::Begin("PIDValve test controls");
    ImGui::TextUnformatted(
        "100 m reservoir -> 600 m pipe -> valve -> atmosphere");
    ImGui::TextUnformatted("Use Start/Pause/Reset in Simulation Controls.");
    const double q_min = 0.0, q_max = 20.0;
    ImGui::SliderScalar("Flow setpoint [m3/s]", ImGuiDataType_Double, &sp,
                        &q_min, &q_max);
    ImGui::Checkbox("Controller active", &active);
    ImGui::Checkbox("Automatic", &automatic);
    ImGui::Checkbox("Anti-windup", &aw);
    ImGui::InputDouble("Manual opening [0..1]", &manual, 0.01);
    ImGui::InputDouble("Kp", &kp, 0.1);
    ImGui::InputDouble("Ti [s] (0 disables integral)", &ti, 0.1);
    ImGui::InputDouble("Td [s]", &td, 0.01);
    ImGui::InputDouble("Tt [s]", &tt, 0.1);
    ImGui::InputDouble("Tf [s] (-1: 0.1 * Td)", &tf, 0.01);

    pid->set_SP_Q(sp);
    pid->set_gains(kp, ti, td);
    pid->set_controls(active, automatic, aw, manual, tt, tf);

    ImGui::Text("Measured Q: %.4f m3/s | opening: %.4f", pid->get_PV_Q(),
                valve->get_tau());
    ImGui::Text("PID servo: %.4f | residual: %.4f", pid->get_output(),
                pid->state().e_t_h);
    ImGui::End();
    app.draw_frame();
  }
}
