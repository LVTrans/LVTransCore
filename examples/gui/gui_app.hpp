#pragma once

#include <GLFW/glfw3.h>
#include <implot.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <format>
#include <iostream>
#include <span>
#include <type_traits>
#include <vector>
#include "helpers.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "lvtrans/element_states.hpp"
#include "lvtrans/element_types.hpp"
#include "lvtrans/element_view.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/plant.hpp"
#include "plot_data.hpp"
using Clock = std::chrono::steady_clock;
using namespace lvtrans;

struct GuiApp {
  Plant& plant;
  PlotContainer plots;
  GLFWwindow* window{nullptr};
  bool initialized{false};
  double dt = 0.1;
  const std::uint64_t graph_sampling_steps[6] = {1, 5, 10, 20, 50, 100};
  const char* graph_intervals[6]{
      "0.1 s (every step)", "0.5 s", "1.0 s", "2.0 s", "5.0 s", "10.0 s"};
  int graph_interval = 2;
  BenchStats stats{};
  bool running = false;
  bool max_speed = true;
  bool invalid = false;
  int speed = 1;
  double dt_accumulator = 0.0;
  char file_path[256] = "plant_state.json";
  Clock::time_point last_frame = Clock::now();

  explicit GuiApp(Plant& p) : plant(p), dt{plant.get_step_size()} {}
  ~GuiApp() { shutdown(); }

  [[nodiscard]] int init() {
    glfwSetErrorCallback([](int code, const char* description) {
      std::cerr << "GLFW error " << code << ": " << description << '\n';
    });
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window =
        glfwCreateWindow(1280, 800, "LVTrans Simulation", nullptr, nullptr);
    if (!window) {
      glfwTerminate();
      return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    const bool glfw_ready = ImGui_ImplGlfw_InitForOpenGL(window, true);
    const bool opengl_ready =
        glfw_ready && ImGui_ImplOpenGL3_Init("#version 130");
    if (!opengl_ready) {
      std::cerr << "ImGui initialization failed\n";
      if (glfw_ready) ImGui_ImplGlfw_Shutdown();
      ImPlot::DestroyContext();
      ImGui::DestroyContext();
      glfwDestroyWindow(window);
      glfwTerminate();
      return 1;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.ItemSpacing = ImVec2(8, 8);
    initialized = true;
    return 0;
  }
  auto steps() -> std::uint64_t {
    return static_cast<std::uint64_t>(plant.get_num_terations());
  };

  auto add_data_point() {
    const auto step = steps();
    if (!plots.sample_due(step)) return;
    std::vector<ElementView> views;
    for (const auto* element : plant.get_elements()) {
      auto view = element->read_view();
      view.element_id = element->get_ID();
      views.push_back(std::move(view));
    }
    plots.sample(step, plant.get_current_time(), std::move(views));
  };

  bool load_plant(const std::string& path) {
    if (plant.load(path) != PlantRepositoryResult::Ok) {
      std::cerr << "Failed to load plant configuration from: " << path << '\n';
      return false;
    }

    running = false;
    invalid = false;
    dt = plant.get_step_size();
    dt_accumulator = 0.0;
    stats.reset(steps());
    // Cached views and histories belong to the plant that was replaced.
    plots.reset();
    add_data_point();
    last_frame = Clock::now();
    std::cerr << "Loaded plant configuration from: " << path << '\n';
    return true;
  }

  void spin() {
    add_data_point();

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      advance_plant();
      render();
      draw_frame();
    }
    shutdown();
  }

  void draw_frame() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  void advance_plant() {
    const auto t =
        static_cast<std::uint64_t>(std::max(0.0, plant.get_current_time()));

    std::string time_str =
        std::format("{:02}:{:02}:{:02}", t / 3600, (t / 60) % 60, t % 60);

    const auto now = Clock::now();
    const double wall_dt =
        std::chrono::duration<double>(now - last_frame).count();
    last_frame = now;

    if (running) {
      if (max_speed) {
        const auto deadline = Clock::now() + std::chrono::milliseconds(50);
        do {
          for (int i = 0; i < 1024; ++i) {
            plant.step();
            add_data_point();
          }
        } while (Clock::now() < deadline);
      } else {
        dt_accumulator += std::min(wall_dt, 0.25) * speed;
        while (dt_accumulator >= dt) {
          plant.step();
          add_data_point();
          dt_accumulator -= dt;
        }
      }
      stats.update(steps(), dt);
    }
  }

  void shutdown() {
    if (!initialized) return;
    initialized = false;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void render() {
    render_ui();
    render_element_states();
    plot_element_data();
  }

  void render_ui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 760), ImGuiCond_FirstUseEver);
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
      dt_accumulator = 0.0;
      last_frame = Clock::now();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      plant.reset_state();
      stats.reset(steps());
      plots.reset();
      add_data_point();
      running = false;
      invalid = false;
      dt_accumulator = 0.0;
      last_frame = Clock::now();
    }

    if (ImGui::Button("Save")) {
      plant.save("plant_state.json");
    }
    ImGui::InputText("File path", file_path, IM_ARRAYSIZE(file_path));

    if (ImGui::Button("Load")) {
      load_plant(file_path);
    }

    if (ImGui::Checkbox("Max Speed", &max_speed)) {
      dt_accumulator = 0.0;
      last_frame = Clock::now();
    }
    ImGui::Text("dT: %.3f s", dt);
    ImGui::BeginDisabled(max_speed);
    ImGui::SliderInt("Execution speed", &speed, 1, 100);
    ImGui::EndDisabled();

    if (ImGui::Combo("Graph interval", &graph_interval, graph_intervals,
                     IM_ARRAYSIZE(graph_intervals))) {
      plots.set_sampling_steps(graph_sampling_steps[graph_interval]);
      add_data_point();
    }
    ImGui::TextDisabled(
        "Simulation seconds; changing interval restarts plots.");

    const auto count = steps();
    ImGui::Separator();

    const auto t =
        static_cast<std::uint64_t>(std::max(0.0, plant.get_current_time()));

    std::string time_str =
        std::format("{:02}:{:02}:{:02}", t / 3600, (t / 60) % 60, t % 60);
    ImGui::Text("Time: %s", time_str.c_str());
    ImGui::Text("I: %llu", static_cast<unsigned long long>(count));
    ImGui::Text("I/s: %.0f", stats.iterations_per_second);
    ImGui::Text("X Realtime: %.1f", stats.x_realtime);
  }
  void render_element_states() {
    ImGui::SeparatorText("Element States");
    for (const auto& e : plant.get_elements()) {
      ImGui::PushID(e->get_ID());
      const auto title = std::format("Element {} ({})", e->get_ID(),
                                     element_type_str(e->get_type()));

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
                if constexpr (std::is_same_v<std::decay_t<decltype(state)>,
                                             VectorValue>) {
                  if (state.values.empty()) {
                    ImGui::TextDisabled("%s: no values", state.name.c_str());
                    return;
                  }
                }
                const auto text = state.to_string();
                ImGui::TextUnformatted(text.c_str());
              },
              value);
          ImGui::Spacing();
        }

        ImGui::PopTextWrapPos();

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
  }

  void plot_element_data() {
    ImGui::SetNextWindowPos(ImVec2(380, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(880, 760), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Data Analytics")) {
      ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.0f, 0.1f));
      if (ImPlot::BeginAlignedPlots("SimulationPlots")) {
        for (const auto& view : plots.views) {
          const auto found = plant.get_element_by_id(view.element_id);
          if (!found || !*found) continue;
          const auto* element = *found;
          ImGui::PushID(view.element_id);
          const auto group_title = std::format(
              "{} — {} (ID {})###element_plots", element->get_name(),
              element_type_str(element->get_type()), view.element_id);
          if (!ImGui::CollapsingHeader(group_title.c_str(),
                                       ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PopID();
            continue;
          }
          ImGui::Indent();
          ImGui::BeginGroup();
          if (view.values.empty()) {
            ImGui::TextDisabled("No plot data available for this element.");
          }
          for (std::size_t i = 0; i < view.values.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            std::visit(
                [&](const auto& value) {
                  using Value = std::decay_t<decltype(value)>;
                  const auto title = std::format(
                      "{} ({}): {} ", element->get_name(),
                      element_type_str(element->get_type()), value.name);
                  if (ImPlot::BeginPlot(title.c_str(), ImVec2(-1, 240))) {
                    ImPlot::SetupAxisFormat(ImAxis_Y1, "%.2f");
                    if constexpr (std::is_same_v<Value, ScalarValue>) {
                      const auto label =
                          value.unit.empty()
                              ? value.name
                              : std::format("{} ({})", value.name, value.unit);
                      ImPlot::SetupAxes("Time (s)", label.c_str(),
                                        ImPlotAxisFlags_AutoFit);
                      ImPlot::SetupAxisFormat(ImAxis_X1, TimeFormatter,
                                              nullptr);
                      const auto& history = plots.histories.at(
                          PlotContainer::key(view.element_id, value));
                      setup_y_limits(std::span<const double>(
                          history.values.data(),
                          static_cast<std::size_t>(history.count)));
                      ImPlotSpec spec;
                      spec.Offset = history.offset;
                      const auto legend = std::format(
                          "{}: {:.2f}{}{}###series", value.name, value.value,
                          value.unit.empty() ? "" : " ", value.unit);
                      ImPlot::PlotLine(legend.c_str(), history.time.data(),
                                       history.values.data(), history.count,
                                       spec);
                    } else {
                      const auto label =
                          value.y_unit.empty()
                              ? value.name
                              : std::format("{} ({})", value.name,
                                            value.y_unit);
                      const auto x_label =
                          value.x_values.empty()
                              ? std::string("Node index")
                              : (value.x_unit.empty()
                                     ? std::string("Distance")
                                     : std::format("Distance ({})",
                                                   value.x_unit));
                      ImPlot::SetupAxes(x_label.c_str(), label.c_str(),
                                        ImPlotAxisFlags_AutoFit);
                      setup_y_limits(value.values);
                      if (!value.values.empty()) {
                        const auto legend = std::format(
                            "{}: start {:.2f}, end {:.2f}{}{}###series",
                            value.name, value.values.front(),
                            value.values.back(),
                            value.y_unit.empty() ? "" : " ", value.y_unit);
                        if (value.x_values.empty()) {
                          ImPlot::PlotLine(
                              legend.c_str(), value.values.data(),
                              static_cast<int>(value.values.size()));
                        } else {
                          ImPlot::PlotLine(
                              legend.c_str(), value.x_values.data(),
                              value.values.data(),
                              static_cast<int>(value.values.size()));
                        }
                      }
                    }
                    ImPlot::EndPlot();
                  }
                },
                view.values[i]);
            ImGui::PopID();
          }
          ImGui::EndGroup();
          ImGui::Unindent();
          ImGui::Spacing();
          ImGui::PopID();
        }
        ImPlot::EndAlignedPlots();
      }
      ImPlot::PopStyleVar();
    }
    ImGui::End();
  }
};
