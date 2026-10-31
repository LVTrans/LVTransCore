
#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "helpers.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve.hpp"
#include "lvtrans/plant.hpp"

using Clock = std::chrono::steady_clock;
using namespace lvtrans;

// LabVIEW-style indicators: I/s is sampled each second; X realtime is
// simulated seconds divided by active (not paused) wall-clock seconds.

int main() {
  if (!glfwInit()) return 1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  GLFWwindow* window =
      glfwCreateWindow(520, 300, "LVTrans BenchA", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);  // Max Speed must not be capped by VSync.

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  if (!ImGui_ImplGlfw_InitForOpenGL(window, true) ||
      !ImGui_ImplOpenGL3_Init("#version 130")) {
    std::cerr << "ImGui initialization failed\n";
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  // BenchA: reservoir -> pipe -> reservoir. Keep identical parameters in
  // LabVIEW.
  constexpr double dt = 0.1;
  Plant plant(dt);

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


    auto steps = [&]() -> std::uint64_t {
      return static_cast<std::uint64_t>(plant.get_num_terations());
    };
    BenchStats stats;
    stats.reset(steps());
    bool running = false;
    bool max_speed = true;
    bool invalid = false;
    int speed = 1;
    double accumulator = 0.0;
    auto last_frame = Clock::now();

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      const auto now = Clock::now();
      const double wall_dt =
          std::chrono::duration<double>(now - last_frame).count();
      last_frame = now;

      if (running) {
        if (max_speed) {
          // Run without sleeping; return to GUI approximately every 50 ms.
          const auto deadline = Clock::now() + std::chrono::milliseconds(50);
          do {
            for (int i = 0; i < 1024; ++i) plant.step();
          } while (Clock::now() < deadline);
        } else {
          accumulator += std::min(wall_dt, 0.25) * speed;
          while (accumulator >= dt) {
            plant.step();
            accumulator -= dt;
          }
        }
        stats.update(steps(), dt);
        // if (!std::isfinite(pipe->get_latest_H()) ||
        //     !std::isfinite(pipe->get_latest_Q())) {
        //   running = false;
        //   stats.iterations_per_second = 0.0;
        //   invalid = true;
        // }
      }

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
      ImGui::SetNextWindowSize(ImVec2(495, 275), ImGuiCond_FirstUseEver);
      ImGui::Begin("Simulation Controls");

      if (ImGui::Button(running ? "Pause" : "Start")) {
        if (running) {
          stats.update(steps(), dt);
          stats.iterations_per_second = 0.0;
          running = false;
        } else if (!invalid) {
          stats.resume(steps());
          running = true;
        }
        accumulator = 0.0;
        last_frame = Clock::now();
      }
      ImGui::SameLine();
      if (ImGui::Button("Reset")) {
        plant.reset_state();
        stats.reset(steps());
        running = false;
        invalid = false;
        accumulator = 0.0;
        last_frame = Clock::now();
      }

      if (ImGui::Checkbox("Max Speed", &max_speed)) {
        accumulator = 0.0;
        last_frame = Clock::now();
      }
      ImGui::Text("dT: %.3f s", dt);
      ImGui::BeginDisabled(max_speed);
      ImGui::SliderInt("Execution speed", &speed, 1, 100);
      ImGui::EndDisabled();

      const auto count = steps();
      const auto t =
          static_cast<std::uint64_t>(std::max(0.0, plant.get_current_time()));
      ImGui::Separator();
      ImGui::Text("Time: %02llu:%02llu:%02llu",
                  static_cast<unsigned long long>(t / 3600),
                  static_cast<unsigned long long>((t / 60) % 60),
                  static_cast<unsigned long long>(t % 60));
      ImGui::Text("I: %llu", static_cast<unsigned long long>(count));
      ImGui::Text("I/s: %.0f", stats.iterations_per_second);
      ImGui::Text("X Realtime: %.1f", stats.x_realtime);
      if (invalid) ImGui::Text("Simulation stopped: non-finite H/Q");
      ImGui::End();

      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
  }
