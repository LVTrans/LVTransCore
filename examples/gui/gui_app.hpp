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
#include <unordered_map>
#include <vector>
#include "helpers.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "lvtrans/element_types.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/plant.hpp"
#include "plot_data.hpp"
using Clock = std::chrono::steady_clock;
using namespace lvtrans;

struct GuiApp {
  Plant& plant;
  PlotContainer plots;
  GLFWwindow* window{nullptr};
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
  Clock::time_point last_frame = Clock::now();

  explicit GuiApp(Plant& p) : plant(p), dt{plant.get_step_size()} {}
  ~GuiApp() {}

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

    // Return the Y-unit of either ScalarValue or VectorValue.
    const auto unit_of = [](const DisplayValue& data) -> const std::string& {
      return std::visit(
          [](const auto& value) -> const std::string& {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, ScalarValue>)
              return value.unit;
            else
              return value.y_unit;
          },
          data);
    };

    if (ImGui::Begin("Data Analytics")) {
      ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.0f, 0.1f));

      if (ImPlot::BeginAlignedPlots("SimulationPlots")) {
        constexpr std::array<ImAxis, 3> y_axes = {ImAxis_Y1, ImAxis_Y2,
                                                  ImAxis_Y3};

        for (const auto& [e_id, groups] : plots.element_plots) {
          const auto element = plant.get_element_by_id(e_id);
          if (!element) continue;

          ImGui::PushID(e_id);

          const auto title =
              std::format("{} ({}):", (*element)->get_name(),
                          element_type_to_str((*element)->get_type()));

          for (const auto& [g_id, view] : groups) {
            if (view.values.empty()) continue;

            ImGui::PushID(g_id);

            // Collect unique units in this group.
            std::vector<std::string> units;

            for (const auto& data : view.values) {
              const auto& unit = unit_of(data);

              if (std::find(units.begin(), units.end(), unit) == units.end()) {
                units.push_back(unit);
              }
            }

            // Determine shared X-axis label.
            // Values in a group should share X semantics.
            const auto x_label = std::visit(
                [](const auto& value) -> std::string {
                  using T = std::decay_t<decltype(value)>;

                  if constexpr (std::is_same_v<T, ScalarValue>) {
                    return "Time (s)";
                  } else {
                    if (value.x_values.empty()) return "Node index";

                    return value.x_unit.empty()
                               ? "Distance"
                               : std::format("Distance ({})", value.x_unit);
                  }
                },
                view.values.front());

            // ImPlot supports at most 3 Y-axes per plot.
            // Additional units get another plot.
            for (std::size_t start = 0; start < units.size();
                 start += y_axes.size()) {
              const auto end = std::min(start + y_axes.size(), units.size());

              const auto plot_title =
                  std::format("{}##unit_group_{}", title, start);

              if (ImPlot::BeginPlot(plot_title.c_str(), ImVec2(-1, 240))) {
                ImPlot::SetupAxis(ImAxis_X1, x_label.c_str(),
                                  ImPlotAxisFlags_AutoFit);

                if (std::holds_alternative<ScalarValue>(view.values.front())) {
                  ImPlot::SetupAxisFormat(ImAxis_X1, TimeFormatter, nullptr);
                }

                // One independently fitted Y-axis per unit.
                for (std::size_t j = start; j < end; ++j) {
                  const auto axis = y_axes[j - start];

                  const auto label = units[j].empty()
                                         ? std::string("Value")
                                         : std::format("Value ({})", units[j]);

                  const ImPlotAxisFlags flags =
                      j == start
                          ? ImPlotAxisFlags_AutoFit
                          : ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_Opposite;

                  ImPlot::SetupAxis(axis, label.c_str(), flags);

                  ImPlot::SetupAxisFormat(axis, "%.2f");
                }

                // Finish all setup before plotting.
                ImPlot::SetupFinish();

                for (std::size_t i = 0; i < view.values.size(); ++i) {
                  const auto& data = view.values[i];

                  const auto first = units.begin() + start;
                  const auto last = units.begin() + end;

                  const auto it = std::find(first, last, unit_of(data));

                  if (it == last) continue;

                  const auto axis = y_axes[std::distance(first, it)];

                  ImPlot::SetAxis(axis);

                  std::visit(
                      [&](const auto& value) {
                        using T = std::decay_t<decltype(value)>;

                        if constexpr (std::is_same_v<T, ScalarValue>) {
                          const auto& history = plots.histories.at(
                              PlotContainer::key(view.element_id, value));

                          if (history.count == 0) return;

                          ImPlotSpec spec;
                          spec.Offset = history.offset;

                          const auto legend = std::format(
                              "{}: {:.2f}{}{}###series_{}", value.name,
                              value.value, value.unit.empty() ? "" : " ",
                              value.unit, i);

                          ImPlot::PlotLine(legend.c_str(), history.time.data(),
                                           history.values.data(), history.count,
                                           spec);

                        } else {
                          if (value.values.empty()) return;

                          const auto legend = std::format(
                              "{}: start {:.2f}, end {:.2f}"
                              "{}{}###series_{}",
                              value.name, value.values.front(),
                              value.values.back(),
                              value.y_unit.empty() ? "" : " ", value.y_unit, i);

                          if (value.x_values.empty()) {
                            ImPlot::PlotLine(
                                legend.c_str(), value.values.data(),
                                static_cast<int>(value.values.size()));
                          } else {
                            const auto count = std::min(value.x_values.size(),
                                                        value.values.size());

                            ImPlot::PlotLine(
                                legend.c_str(), value.x_values.data(),
                                value.values.data(), static_cast<int>(count));
                          }
                        }
                      },
                      data);
                }

                ImPlot::EndPlot();
              }
            }

            ImGui::PopID();
          }

          ImGui::PopID();
        }

        ImPlot::EndAlignedPlots();
      }

      ImPlot::PopStyleVar();
    }

    ImGui::End();
  }
};
