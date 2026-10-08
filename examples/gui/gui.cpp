#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_levle_right.hpp"
#include "lvtrans/elements/valve.hpp"
#include "lvtrans/plant.hpp"

struct PlotContainer {
  std::vector<double> time;
  std::vector<std::vector<double>> plots;
};

int main() {
  // 1. Initialize GLFW & Open a Window
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
  glfwSwapInterval(1);  // Enable vsync

  // 2. Initialize ImGui & ImPlot Contexts
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  // Setup Platform/Renderer backends
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

  // Simulation Data Variables
  float sim_speed = 1.0f;
  bool is_running = false;
  // std::vector<double> time_steps;
  // std::vector<double> object_position;
  using namespace lvtrans;

  double a = 1200.0;   // Wave propagation velocity [m/s]
  double f = 0.018;    // Darcy-Weisbach friction factor
  double tau_i = 1.0;  // Initial valve position
  double tau_f = 0.0;  // Final valve position
  double tc = 2.1;     // Valve operating/closure time [s]
  double em = 0.75;    // Exponent defining valve motion

  double HR = 150.0;    // Reservoir head above datum [m]
  double CdA0 = 0.009;  // Valve coefficient/opening parameter

  Plant plant(0.1);
  PipeParameters pipe_config = {
      .length = 600.0,
      .diameter = 0.5,
      .f = f,
      .a = a,
      .z0 = 10,
      .z1 = 15,
  };

  const double sim_dt = plant.get_step_size();
  const auto segments =
      calculate_nodes_temp(pipe_config.lambda, pipe_config.length, sim_dt,
                           pipe_config.a, pipe_config.rho);
  const auto segments_real = calculate_num_segments(segments);
  const double dx = calculate_dx(pipe_config.length, segments);

  const auto num_segments =
      static_cast<size_t>(calculate_num_segments(segments));
  std::vector<double> H0_(num_segments + 1, 0.0);
  std::vector<double> Q0_(num_segments + 1, 0.0);

  const double area = calculate_pipe_area(
      pipe_config.area, pipe_config.diameter, pipe_config.dimension);
  const double R = calculate_R(pipe_config.f, dx, pipe_config.diameter, area);

  const double Q0 =
      std::sqrt(2.0 * consts::g * CdA0 * CdA0 * HR /
                (R * segments * 2.0 * consts::g * CdA0 * CdA0 + 1.0));

  const double H0 = HR - R * segments * Q0 * Q0;

  const double Qi = std::sqrt(HR * Q0 * Q0 * tau_i * tau_i /
                              (R * segments * Q0 * Q0 * tau_i * tau_i + H0));

  for (size_t i = 0; i < H0_.size(); i += 2) {
    H0_[i] = HR - static_cast<double>(i) * R * Qi * Qi;
    Q0_[i] = Qi;
  }

  const double CVP = 0.5 * Q0 * Q0 / H0;
  auto pipe = plant.add_element<Pipe>(pipe_config, H0_, Q0_, sim_dt).value();
  auto reservoir = plant.add_element<ConstantLevelLeft>(150.0).value();
  auto pipe2 = plant.add_element<Pipe>(pipe_config, H0_, Q0_, sim_dt).value();
  auto reservoir2 = plant.add_element<ConstantLevelRight>(150.0).value();

  ValveParameters valve_config{};
  valve_config.tau_i = tau_i;
  valve_config.tau_f = tau_f;
  valve_config.tc = tc;
  valve_config.em = em;
  valve_config.cvp = CVP;

  auto valve = plant.add_element<Valve>(valve_config).value();

  pipe->connect_to(reservoir, PortType::Left, PortType::Right);
  pipe->connect_to(valve, PortType::Right, PortType::Left);
  pipe2->connect_to(reservoir2, PortType::Right, PortType::Left);
  pipe2->connect_to(valve, PortType::Left, PortType::Right);

  PlotContainer plots{.time = std::vector<double>{},
                      .plots = std::vector<std::vector<double>>{
                          std::vector<double>{},
                          std::vector<double>{},
                          std::vector<double>{},
                      }};
  const double real_dt = ImGui::GetIO().DeltaTime;
  double acc = 0;
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start the ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // --- SIMULATION LOGIC ---
    if (is_running) {
      acc += real_dt * sim_speed;
      while (acc >= sim_dt) {
        plant.step();
        plots.time.push_back(plant.get_current_time());

        plots.plots[0].push_back(pipe->get_latest_H());
        plots.plots[1].push_back(pipe->get_latest_Q());
        plots.plots[2].push_back(valve->get_tau());

        if (plots.time.size() > 200) {
          plots.time.erase(plots.time.begin());
          plots.plots[0].erase(plots.plots[0].begin());
          plots.plots[1].erase(plots.plots[1].begin());
          plots.plots[2].erase(plots.plots[2].begin());
        }

        acc -= sim_dt;
      }
    }

    // --- UI WINDOW 1: SIMULATION CONTROLS ---
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(340, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulation Control Panel");
    ImGui::Text("Status: %s", is_running ? "RUNNING" : "PAUSED");
    ImGui::Text("Time: %.2f s", plant.get_current_time());
    ImGui::Text("Iterations: %d", plant.get_num_terations());

    if (ImGui::Button(is_running ? "Pause" : "Start")) {
      is_running = !is_running;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      plots.time.clear();
      plots.plots[0].clear();
      plots.plots[1].clear();
      plots.plots[2].clear();
      plant.reset_state();
      is_running = false;
    }

    ImGui::SliderFloat("Simulation Speed", &sim_speed, 0.1f, 5.0f);
    ImGui::Spacing();
    ImGui::TextUnformatted("Reservoir head (m)");
    ImGui::SetNextItemWidth(180.0f);
    ImGui::Spacing();
    ImGui::SeparatorText("Element States");
    for (const auto& e : plant.get_elements()) {
      ImGui::PushID(e->get_ID());
      const auto title = "Element " + std::to_string(e->get_ID());
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
        ImGui::PopTextWrapPos();
        ImGui::Unindent();
      }
      ImGui::PopID();
    }
    ImGui::End();

    // --- UI WINDOW 2: REAL-TIME GRAPH (Using ImPlot) ---
    ImGui::SetNextWindowPos(ImVec2(380, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(880, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Data Analytics");
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.0f, 0.1f));
    if (ImPlot::BeginAlignedPlots("SimulationPlots")) {
      if (ImPlot::BeginPlot("Pipe Head at Valve")) {
        ImPlot::SetupAxes("Time (s)", "Head H (m)", ImPlotAxisFlags_AutoFit,
                          ImPlotAxisFlags_AutoFit);
        ImPlot::PlotLine("H at valve", plots.time.data(), plots.plots[0].data(),
                         static_cast<int>(plots.time.size()));
        ImPlot::EndPlot();
      }

      if (ImPlot::BeginPlot("Pipe Flow at Valve")) {
        ImPlot::SetupAxes("Time (s)", "Flow Q (m³/s)", ImPlotAxisFlags_AutoFit,
                          ImPlotAxisFlags_AutoFit);
        ImPlot::PlotLine("Q at valve", plots.time.data(), plots.plots[1].data(),
                         static_cast<int>(plots.time.size()));
        ImPlot::EndPlot();
      }
      if (ImPlot::BeginPlot("Valve Tau")) {
        ImPlot::SetupAxes("Time (s)", "Valve opening (0-1)",
                          ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_None);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -0.05, 1.05, ImPlotCond_Always);
        ImPlot::PlotLine("Tau at valve", plots.time.data(),
                         plots.plots[2].data(),
                         static_cast<int>(plots.time.size()));
        ImPlot::EndPlot();
      }
      ImPlot::EndAlignedPlots();
    }
    ImPlot::PopStyleVar();
    ImGui::End();

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
