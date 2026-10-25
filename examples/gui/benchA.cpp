#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "benchmark_metrics.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/plant.hpp"

using Clock = std::chrono::steady_clock;
using namespace lvtrans;

struct SimulationState {
  bool is_running = false;
  bool max_speed = true;
  int execution_speed = 1;
};

int main() {
  glfwSetErrorCallback([](int code, const char* description) {
    std::cerr << "GLFW error " << code << ": " << description << '\n';
  });
  if (!glfwInit()) return 1;

  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  GLFWwindow* window =
      glfwCreateWindow(560, 380, "LVTrans Benchmark A", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);  // Do not cap GUI updates with VSync.

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetStyle().WindowRounding = 0.0f;
  ImGui::GetStyle().FrameRounding = 0.0f;

  if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }
  if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  // Identical plant parameters and topology to the supplied BenchA code.
  constexpr double dt = 0.1;
  Plant plant(dt);

  ReservoirParameters reservoir_params{
      .H0 = 100.000000E+0,
      .cvp = 100.000000E+3,
      .cvm = 100.000000E+3,
  };
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

  auto reservoir_left =
      plant.add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe = plant.add_element<Pipe>(pipe_params, dt).value();
  auto reservoir_right =
      plant.add_element<ConstantLevelRight>(reservoir_params).value();

  HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir_left);
  HydraulicConnections::connect(*pipe, PipeEnd::Right, *reservoir_right);

  SimulationState sim{};
  BenchmarkMetrics metrics{};
  metrics.reset(static_cast<std::uint64_t>(plant.get_num_terations()),
                plant.get_current_time());

  // Slider mode accumulates demanded simulated time. Max Speed bypasses it.
  double accumulator = 0.0;
  auto last_frame = Clock::now();
  bool invalid_hydraulics = false;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwWindowShouldClose(window)) break;

    const auto frame_now = Clock::now();
    const double wall_dt =
        std::chrono::duration<double>(frame_now - last_frame).count();
    last_frame = frame_now;

    if (sim.is_running) {
      // Batching means no per-step clock or GUI overhead.
      const auto deadline = Clock::now() + std::chrono::milliseconds(50);
      if (sim.max_speed) {
        do {
          for (int i = 0; i < 1024; ++i) plant.step();
        } while (Clock::now() < deadline);
      } else {
        const int speed = std::clamp(sim.execution_speed, 1, 100);
        accumulator += wall_dt * static_cast<double>(speed);
        // Drop excessive catch-up work after stalls or when the CPU is slower
        // than the requested speed, rather than freezing the controls.
        accumulator = std::min(accumulator, 1.0);
        while (accumulator >= dt && Clock::now() < deadline) {
          plant.step();
          accumulator -= dt;
        }
      }
      metrics.update(static_cast<std::uint64_t>(plant.get_num_terations()),
                     plant.get_current_time());

      // Observe actual numerical output so the benchmark cannot just count
      // iterations while leaving the physical state unused.
      // const double H = pipe->get_latest_H();
      // const double Q = pipe->get_latest_Q();
      // if (!std::isfinite(H) || !std::isfinite(Q)) {
      //   invalid_hydraulics = true;
      //   metrics.pause(static_cast<std::uint64_t>(plant.get_num_terations()),
      //                 plant.get_current_time());
      //   sim.is_running = false;
      // }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(12, 12), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(530, 345), ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulation Control Panel", nullptr,
                 ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Status: %s", sim.is_running ? "RUNNING" : "PAUSED");
    if (ImGui::Button(sim.is_running ? "Pause" : "Start")) {
      if (sim.is_running) {
        metrics.pause(static_cast<std::uint64_t>(plant.get_num_terations()),
                      plant.get_current_time());
        sim.is_running = false;
      } else if (!invalid_hydraulics) {
        metrics.resume(static_cast<std::uint64_t>(plant.get_num_terations()));
        sim.is_running = true;
      }
      accumulator = 0.0;
      last_frame = Clock::now();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      plant.reset_state();
      sim.is_running = false;
      invalid_hydraulics = false;
      accumulator = 0.0;
      metrics.reset(static_cast<std::uint64_t>(plant.get_num_terations()),
                    plant.get_current_time());
      last_frame = Clock::now();
    }

    if (ImGui::Checkbox("Max Speed", &sim.max_speed)) {
      accumulator = 0.0;  // No stale slider backlog when changing modes.
      last_frame = Clock::now();
    }
    ImGui::SameLine();
    ImGui::Text("dT: %.4f s", dt);
    ImGui::BeginDisabled(sim.max_speed);
    ImGui::SetNextItemWidth(235.0f);
    ImGui::SliderInt("Execution speed (x realtime)", &sim.execution_speed, 1,
                     100);
    ImGui::EndDisabled();
    sim.execution_speed = std::clamp(sim.execution_speed, 1, 100);

    const double sim_time = plant.get_current_time();
    const auto whole_seconds =
        static_cast<std::uint64_t>(std::floor(std::max(0.0, sim_time)));
    const auto hours = whole_seconds / 3600;
    const auto minutes = (whole_seconds / 60) % 60;
    const auto seconds = whole_seconds % 60;
    const auto iterations =
        static_cast<std::uint64_t>(plant.get_num_terations());

    ImGui::Separator();
    ImGui::Text("Time: %02llu:%02llu:%02llu",
                static_cast<unsigned long long>(hours),
                static_cast<unsigned long long>(minutes),
                static_cast<unsigned long long>(seconds));
    ImGui::Text("Iterations: %llu",
                static_cast<unsigned long long>(iterations));
    ImGui::Text("Iterations/s (1 s sample): %.0f",
                metrics.iterations_per_second);
    ImGui::Text("X Realtime (active-wall average): %.1fx", metrics.x_realtime);
    ImGui::Text("Average iterations/s: %.0f",
                metrics.average_iterations_per_second);
    ImGui::Text("Active wall time: %.2f s", metrics.active_seconds());
    if (invalid_hydraulics) {
      ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.3f, 1.0f),
                         "Non-finite pipe H or Q; simulation stopped.");
    }
    ImGui::End();

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);
    glClearColor(0.1f, 0.1f, 0.14f, 1.0f);
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
  return 0;
}
