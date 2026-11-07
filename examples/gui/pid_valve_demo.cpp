#include <cmath>
#include <iostream>
#include <string_view>
#include "gui_app.hpp"
#include "helpers.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/pid_valve.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"

using namespace lvtrans;

int main(int argc, char** argv) {
  print_build_type();
  constexpr double dt = 0.05;
  Plant plant(dt);
  auto* reservoir = plant.add_element<ConstantLevelLeft>(100.0).value();
  PipeParameters pp{.H0 = 100.0,
                    .Q0 = 1.0,
                    .length = 600.0,
                    .diameter = 1.0,
                    .f = 0.02,
                    .a = 1200.0,
                    .use_diameter = true};
  auto* pipe = plant.add_element<Pipe>(pp, dt).value();
  ValveInternalServoParameters vp{};
  vp.cvp = vp.cvm = 0.02;
  vp.initial_opening = 0.5;
  vp.servo_time = 0.2;
  vp.closing_time_high = vp.closing_time_low = 10.0;
  auto* valve = plant.add_element<ValveInternalServo>(vp, dt).value();
  auto* pid = plant
                  .add_element<PIDValve>(PIDValveParameters{.Q_wanted = 2.0,
                                                            .SP_Q = 1.0,
                                                            .Kp = 0.5,
                                                            .Ti = 5.0,
                                                            .Td = 0.0},
                                         dt)
                  .value();
  reservoir->set_name("Upstream reservoir (100 m)");
  pipe->set_name("Pipe (600 m)");
  valve->set_name("Controlled valve to atmosphere");
  pid->set_name("Flow PID");

  if (HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir) !=
          ConnectResult::Ok ||
      HydraulicConnections::connect(*pipe, PipeEnd::Right, *valve) !=
          ConnectResult::Ok ||
      !ControllerConnections::connect(*pid, *valve))
    return 1;

  pid->set_controls(true, true, true, 0.5, 10.0, -1.0);

  GuiApp app(plant);
  if (app.init() != 0) return 1;
  app.max_speed = false;
  app.graph_interval = 0;
  double sp = 1.0, kp = 0.5, ti = 5.0, td = 0.0, tt = 10.0, tf = -1.0,
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
    const double q_min = 0.0, q_max = 2.0;
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
