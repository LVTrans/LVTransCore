#include <GLFW/glfw3.h>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <format>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve.hpp"
#include "lvtrans/plant.hpp"

using Clock = std::chrono::steady_clock;

using namespace lvtrans;
struct PlotContainer {
  std::vector<double> time;
  std::vector<std::vector<double>> plots;
};

struct SimulationState {
  Clock::time_point timer_iterations{Clock::now()};
  double x_realtime{};
  bool is_running{};
  bool max_speed{};
  int execution_speed{1};
  std::uint64_t iterations_p_s{1};
  std::uint64_t last_iteartions{};
  int seconds{};
  int minutes{};
  int hours{};

  void reset_time() {
    seconds = 0;
    minutes = 0;
    hours = 0;
  }
};

int main() {
  glfwSetErrorCallback([](int code, const char* description) {
    std::cerr << "GLFW error " << code << ": " << description << '\n';
  });
  if (!glfwInit()) {
    return -1;
  }
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  GLFWwindow* window =
      glfwCreateWindow(1280, 720, "Simulation Dashboard", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);  // No VSync

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
    std::cerr << "Failed to initialize the ImGui GLFW backend\n";
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }
  if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
    std::cerr << "Failed to initialize the ImGui OpenGL backend\n";
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 0.0f;       // Sharp window corners
  style.FrameRounding = 0.0f;        // Sharp button/input corners
  style.ItemSpacing = ImVec2(8, 8);  // Tight horizontal and vertical spacing

  SimulationState sim_state{};

  double dt = 0.1;
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

  PlotContainer plots{.time = std::vector<double>{},
                      .plots = std::vector<std::vector<double>>{
                          std::vector<double>{},
                          std::vector<double>{},
                          std::vector<double>{},
                      }};
  plant.display();
  const double real_dt = ImGui::GetIO().DeltaTime;
  auto stats_last = Clock::now();

  double acc = 0;
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start the ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
    auto now = Clock::now();
    if (std::chrono::duration<double>(now - sim_state.timer_iterations)
            .count() >= 1.0) {
      // sim_state.iterations_p_s =
      // plant.get_num_terations() - sim_state.last_iteartions;
      // sim_state.last_iteartions = plant.get_num_terations();
      sim_state.seconds += sim_state.execution_speed;
      sim_state.timer_iterations = now;

      if (sim_state.seconds >= 60) {
        sim_state.seconds -= 60;
        sim_state.minutes++;
        while (sim_state.seconds >= 60) {
          sim_state.seconds -= 60;
          sim_state.minutes++;
        }
        if (sim_state.minutes >= 60) {
          sim_state.minutes = 0;
          sim_state.hours++;
          if (sim_state.hours >= 24) {
            sim_state.hours = 0;
          }
        }
      }
    }

    // --- SIMULATION LOGIC ---
    if (sim_state.is_running) {
      if (sim_state.max_speed) {
        auto deadline = Clock::now() + std::chrono::milliseconds(10);

        do {
          for (int i = 0; i < 1024; ++i) {
            plant.step();
          }

        } while (Clock::now() < deadline);

      } else {
        acc += real_dt * sim_state.execution_speed;
        while (acc >= dt) {
          plant.step();
          acc -= dt;
        }
      }

      auto stats_now = Clock::now();
      auto elapsed =
          std::chrono::duration<double>(stats_now - stats_last).count();
      if (elapsed > 1.0f) {
        const auto iterations = plant.get_num_terations();
        sim_state.iterations_p_s =
            (iterations - sim_state.last_iteartions) / elapsed;

        sim_state.x_realtime = sim_state.iterations_p_s * dt;

        sim_state.last_iteartions = iterations;
        stats_last = stats_now;
      }
    }

    // --- UI WINDOW 1: SIMULATION CONTROLS ---
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(340, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulation Control Panel");
    ImGui::Text("Status: %s", sim_state.is_running ? "RUNNING" : "PAUSED");

    if (ImGui::Button(sim_state.is_running ? "Pause" : "Start")) {
      sim_state.is_running = !sim_state.is_running;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      plots.time.clear();
      plots.plots[0].clear();
      plots.plots[1].clear();
      plots.plots[2].clear();
      plant.reset_state();
      sim_state.is_running = false;
      sim_state.reset_time();
    }
    ImGui::Separator();
    ImGui::Checkbox("Max Speed", &sim_state.max_speed);

    ImGui::BeginGroup();
    ImGui::Text("dT");
    ImGui::SetNextItemWidth(110.0f);
    ImGui::BeginDisabled(true);
    ImGui::InputDouble("##dT", &dt, 1, 10);
    ImGui::EndDisabled();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    if (sim_state.max_speed) {
      ImGui::BeginDisabled(true);
    }
    ImGui::Text("Execution speed (X realtime)");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100.0f);
    ImGui::SliderInt("##slider_int", &sim_state.execution_speed, 1, 100);

    if (sim_state.max_speed) {
      ImGui::EndDisabled();
    }
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Text(" ");

    ImGui::SetNextItemWidth(100.0f);

    if (sim_state.max_speed) {
      ImGui::BeginDisabled(true);
    }
    ImGui::InputInt("##speed_int", &sim_state.execution_speed, 1, 10);

    if (sim_state.max_speed) {
      ImGui::EndDisabled();
    }
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text("Time");
    ImGui::SetNextItemWidth(90.0f);
    ImGui::BeginDisabled(true);
    ImGui::Text("%02d:%02d:%02d", sim_state.hours, sim_state.minutes,
                sim_state.seconds);
    ImGui::EndDisabled();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Text("x Realtime");

    ImGui::SetNextItemWidth(90.0f);
    ImGui::BeginDisabled(true);
    ImGui::InputDouble("##x_realtime", &sim_state.x_realtime, 0.0f, 0.0f,
                       "%.2f");
    ImGui::EndDisabled();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Text("Iterations");

    ImGui::SetNextItemWidth(90.0f);
    ImGui::BeginDisabled(true);
    ImGui::Text("%li", plant.get_num_terations());
    ImGui::EndDisabled();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Text("Iterations/s");

    ImGui::SetNextItemWidth(90.0f);
    ImGui::BeginDisabled(true);
    ImGui::InputScalar("##iterations_p_s", ImGuiDataType_U64,
                       &sim_state.iterations_p_s, 0);
    ImGui::EndDisabled();
    ImGui::EndGroup();

    ImGui::SeparatorText("Element States");
    for (const auto& e : plant.get_elements()) {
      ImGui::PushID(e->get_ID());
      const auto title = std::format("Element {} ({})", e->get_ID(),
                                     element_type_to_str(e->get_type()));

      if (ImGui::CollapsingHeader(title.c_str(),
                                  ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto view = e->read_view();
        ImGui::Indent();
        ImGui::PushTextWrapPos(0.0f);
        if (view.values.empty()) {
          ImGui::TextDisabled("No state values available.");
        }
        for (const auto& value : view.values) {
          std::visit(
              [](const auto& state) {
                const auto text = state.to_string();
                ImGui::TextUnformatted(text.c_str());
              },
              value);
          ImGui::Spacing();
        }

        auto params = e->get_parameters();
        ImGui::Spacing();
        ImGui::SeparatorText("Element Params");
        ImGui::PushTextWrapPos(0.0f);
        std::visit(
            [](const auto& state) {
              const auto text = state.to_str();
              ImGui::TextUnformatted(text.c_str());
            },
            params);
        ImGui::PopTextWrapPos();
        ImGui::Unindent();
      }
      ImGui::PopID();
    }
    ImGui::End();

    // --- UI WINDOW 2: REAL-TIME GRAPH (Using ImPlot) ---
    // ImGui::SetNextWindowPos(ImVec2(380, 20), ImGuiCond_FirstUseEver);
    // ImGui::SetNextWindowSize(ImVec2(880, 680), ImGuiCond_FirstUseEver);
    // ImGui::Begin("Data Analytics");
    // ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.0f, 0.1f));
    // if (ImPlot::BeginAlignedPlots("SimulationPlots")) {
    //   if (ImPlot::BeginPlot("Pipe Head at Valve")) {
    //     ImPlot::SetupAxes("Time (s)", "Head H (m)", ImPlotAxisFlags_AutoFit,
    //                       ImPlotAxisFlags_AutoFit);
    //     ImPlot::PlotLine("H at valve", plots.time.data(),
    //     plots.plots[0].data(),
    //                      static_cast<int>(plots.time.size()));
    //     ImPlot::EndPlot();
    //   }
    //
    //   if (ImPlot::BeginPlot("Pipe Flow at Valve")) {
    //     ImPlot::SetupAxes("Time (s)", "Flow Q (m³/s)",
    //     ImPlotAxisFlags_AutoFit,
    //                       ImPlotAxisFlags_AutoFit);
    //     ImPlot::PlotLine("Q at valve", plots.time.data(),
    //     plots.plots[1].data(),
    //                      static_cast<int>(plots.time.size()));
    //     ImPlot::EndPlot();
    //   }
    //   if (ImPlot::BeginPlot("Reservoir 1 Q ")) {
    //     ImPlot::SetupAxes("Time (s)", "Q [m³/s]", ImPlotAxisFlags_AutoFit,
    //                       ImPlotAxisFlags_None);
    //     ImPlot::SetupAxisLimits(ImAxis_Y1, -0.05, 1.05, ImPlotCond_Always);
    //     ImPlot::PlotLine("Tau at valve", plots.time.data(),
    //                      plots.plots[2].data(),
    //                      static_cast<int>(plots.time.size()));
    //     ImPlot::EndPlot();
    //   }
    //   ImPlot::EndAlignedPlots();
    // }
    // ImPlot::PopStyleVar();
    // ImGui::End();

    // --- RENDER THE FRAME ---
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
