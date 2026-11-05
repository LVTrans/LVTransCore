#pragma once

#include <GLFW/glfw3.h>
#include <implot.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include "helpers.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "lvtrans/plant.hpp"
#include "plot_data.hpp"
using Clock = std::chrono::steady_clock;
using namespace lvtrans;

struct BenchApp {
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
  bool write_to_file = false;
  int speed = 1;
  double dt_accumulator = 0.0;
  std::ofstream file;
  Clock::time_point last_written_time = Clock::now();
  Clock::time_point last_frame = Clock::now();

  explicit BenchApp(Plant& p)
      : plant(p), dt{plant.get_step_size()}, file("bench_results.csv") {}
  ~BenchApp() { shutdown(); }

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

    file << "RealTime [s],IterationsPerSecond,SimTime [s]\n";
    file.flush();
    return 0;
  }
  auto steps() -> std::uint64_t {
    return static_cast<std::uint64_t>(plant.get_num_terations());
  };

  void spin() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      advance_plant();
      render();
      draw_frame();
    }
    shutdown();
  }

  void write_results_to_file() {
    const auto now = Clock::now();
    const auto elapsed =
        std::chrono::duration<double>(now - last_written_time).count();

    if (elapsed >= 1.0) {
      file << stats.x_realtime << ',' << stats.iterations_per_second << ','
           << plant.get_current_time() << '\n';
      file.flush();
      last_written_time = now;
    }
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
          }
        } while (Clock::now() < deadline);
      } else {
        dt_accumulator += std::min(wall_dt, 0.25) * speed;
        while (dt_accumulator >= dt) {
          plant.step();
          dt_accumulator -= dt;
        }
      }
      stats.update(steps(), dt);
      if (write_to_file) {
        write_results_to_file();
      }
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

  void render() { render_ui(); }

  void render_ui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
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
      dt_accumulator = 0.0;
      last_frame = Clock::now();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      plant.reset_state();
      stats.reset(steps());
      last_written_time = Clock::now();
      running = false;
      invalid = false;
      dt_accumulator = 0.0;
      last_frame = Clock::now();
    }

    if (ImGui::Checkbox("Max Speed", &max_speed)) {
      dt_accumulator = 0.0;
      last_frame = Clock::now();
    }

    ImGui::Checkbox("Write results to file", &write_to_file);

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
  }
};
