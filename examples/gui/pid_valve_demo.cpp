#include <cmath>
#include <iostream>
#include <string_view>
#include "gui_app.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/pid_valve.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"

using namespace lvtrans;

int main(int argc, char** argv) {
  constexpr double dt = 0.05;
  Plant plant(dt);
  auto* reservoir = plant.add_element<ConstantLevelLeft>(100.0).value();
  PipeParameters pp{.H0 = 100.0, .Q0 = 1.0, .length = 600.0,
                    .diameter = 1.0, .f = 0.02, .a = 1200.0,
                    .use_diameter = true};
  auto* pipe = plant.add_element<Pipe>(pp, dt).value();
  ValveInternalServoParameters vp{};
  vp.cvp = vp.cvm = 0.02;
  vp.initial_opening = 0.5;
  vp.servo_time = 0.2;
  vp.closing_time_high = vp.closing_time_low = 10.0;
  auto* valve = plant.add_element<ValveInternalServo>(vp, dt).value();
  auto* pid = plant.add_element<PIDValve>(
      PIDValveParameters{.Q_wanted = 2.0, .SP_Q = 1.0,
                         .Kp = 0.5, .Ti = 5.0, .Td = 0.0}, dt).value();
  reservoir->set_name("Upstream reservoir (100 m)");
  pipe->set_name("Pipe (600 m)"); valve->set_name("Controlled valve to atmosphere");
  pid->set_name("Flow PID");
  if (HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir) != ConnectResult::Ok ||
      HydraulicConnections::connect(*pipe, PipeEnd::Right, *valve) != ConnectResult::Ok ||
      !ControllerConnections::connect(*pid, *valve)) return 1;
  pid->set_controls(true, true, true, 0.5, 10.0, -1.0);

  if (argc > 1 && std::string_view(argv[1]) == "--headless") {
    for (int i = 0; i < 6000; ++i) {
      if (i == 2000) pid->set_SP_Q(0.6);
      if (i == 4000) pid->set_SP_Q(1.2);
      plant.step();
      if (!std::isfinite(pid->get_output()) || !std::isfinite(valve->get_state_as<ValveInternalServoState>().Q)) return 2;
    }
    const double q = valve->get_state_as<ValveInternalServoState>().Q;
    std::cout << "Final flow: " << q << " m3/s; setpoint: 1.2 m3/s\n";
    return std::abs(q - 1.2) < 0.05 ? 0 : 3;
  }

  GuiApp app(plant);
  if (app.init() != 0) return 1;
  app.max_speed = false;
  app.graph_interval = 0;
  double sp = 1.0, kp = 0.5, ti = 5.0, td = 0.0, tt = 10.0, tf = -1.0, manual = 0.5;
  bool active = true, automatic = true, aw = true;
  app.add_data_point();
  while (!glfwWindowShouldClose(app.window)) {
    glfwPollEvents(); app.advance_plant(); app.render();
    ImGui::SetNextWindowPos(ImVec2(370, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430, 480), ImGuiCond_FirstUseEver);
    ImGui::Begin("PIDValve test controls");
    ImGui::TextUnformatted("100 m reservoir -> 600 m pipe -> valve -> atmosphere");
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
    const bool valid = std::isfinite(sp) && sp >= 0 && std::isfinite(kp) && kp >= 0 &&
        std::isfinite(ti) && ti >= 0 && std::isfinite(td) && td >= 0 &&
        std::isfinite(tt) && tt > 0 && std::isfinite(tf) &&
        std::isfinite(manual) && manual >= 0 && manual <= 1;
    if (valid) {
      pid->set_SP_Q(sp); pid->set_gains(kp, ti, td);
      pid->set_controls(active, automatic, aw, manual, tt, tf);
    } else ImGui::TextUnformatted("Invalid settings: previous valid values remain active.");
    ImGui::Text("Measured Q: %.4f m3/s | opening: %.4f", pid->get_PV_Q(), valve->get_tau());
    ImGui::Text("PID servo: %.4f | residual: %.4f", pid->get_output(), pid->state().e_t_h);
    ImGui::End(); app.draw_frame();
  }
}
