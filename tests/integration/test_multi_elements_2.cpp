#include <gtest/gtest.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <lvtrans/elements/pipe.hpp>
#include <lvtrans/elements/reservoir.hpp>
#include <sstream>
#include "../test_helpers.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/plant.hpp"

// R <-> P <-> R (comparison against LabVIEW)
TEST(MultiElementsTest, ReservoirPipeReservoir) {
  using namespace lvtrans;
  constexpr double dt = 0.1;
  Plant plant(dt);
  ReservoirParameters reservoir_params{.H0 = 100.0, .cvp = 100e3, .cvm = 100e3};
  const auto pipe_params = lvtrans::PipeParameters{.H0 = 0.0,
                                                   .Q0 = 0.0,
                                                   .length = 100,
                                                   .diameter = 1,
                                                   .area = 785.400000e-3,
                                                   .periphery = 3.141600,
                                                   .epsilon = 10e-3,
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
                                                   .use_Dh = false};

  auto reservoir_left =
      plant.add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe = plant.add_element<Pipe>(pipe_params, dt).value();
  auto reservoir_right =
      plant.add_element<ConstantLevelRight>(reservoir_params).value();

  ASSERT_EQ(
      HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir_left),
      ConnectResult::Ok);
  ASSERT_EQ(
      HydraulicConnections::connect(*pipe, PipeEnd::Right, *reservoir_right),
      ConnectResult::Ok);

  const auto test_file_path =
      get_mock_data_file_path("generated/multi_element_test_2.csv");
  std::ofstream output_file(test_file_path);

  ASSERT_TRUE(output_file.is_open());

  output_file << "t,H_0,H_N,Q_0,Q_N,LR_Q2,RR_Q1\n";

  std::ifstream reference(
      get_mock_data_file_path("expected/labview_pipe_and_reservoirs.csv"));
  ASSERT_TRUE(reference.is_open());
  std::string line;
  ASSERT_TRUE(static_cast<bool>(std::getline(reference, line)));  // Header.
  ASSERT_EQ(line, "reported_time,H_0,H_N,Q_0,Q_N,LR_Q2,RR_Q1");
  output_file << std::setprecision(7);

  constexpr double export_tolerance = 5.1e-7;  // Six-decimal rounding.
  int row = 0;
  while (std::getline(reference, line)) {
    std::replace(line.begin(), line.end(), ',', ' ');
    std::istringstream fields(line);
    double reported_time, h0, hn, q0, qn, lr_q, rr_q;

    ASSERT_TRUE(static_cast<bool>(fields >> reported_time >> h0 >> hn >> q0 >>
                                  qn >> lr_q >> rr_q));
    lr_q *= -1;
    rr_q *= -1;
    const auto sample_step =
        static_cast<std::uint64_t>(std::llround(reported_time / dt));
    ASSERT_GE(sample_step, plant.get_num_terations());
    plant.run_steps(sample_step - plant.get_num_terations());
    ASSERT_NEAR(plant.get_current_time(), reported_time, 1e-10);

    SCOPED_TRACE("LabVIEW reported t=" + std::to_string(reported_time) +
                 ", solver t=" + std::to_string(plant.get_current_time()) +
                 " (row " + std::to_string(row) + ")");

    EXPECT_NEAR(pipe->get_H().front(), h0, export_tolerance);
    EXPECT_NEAR(pipe->get_H().back(), hn, export_tolerance);
    EXPECT_NEAR(pipe->get_Q().front(), q0, export_tolerance);
    EXPECT_NEAR(pipe->get_Q().back(), qn, export_tolerance);
    // TODO: Clarify with author how reservoir's Q is sampled
    // EXPECT_NEAR(reservoir_left->get_state_as<ReservoirState>().Q, lr_q,
    //             export_tolerance);
    // EXPECT_NEAR(reservoir_right->get_state_as<ReservoirState>().Q, rr_q,
    //             export_tolerance);

    output_file << plant.get_current_time() << ',' << pipe->get_H().front()
                << ',' << pipe->get_H().back() << ',' << pipe->get_Q().front()
                << ',' << pipe->get_Q().back() << ','
                << reservoir_left->get_state_as<ReservoirState>().Q << ','
                << reservoir_right->get_state_as<ReservoirState>().Q << '\n';
    ++row;
  }
  EXPECT_EQ(row, 184);
  EXPECT_FALSE(reference.bad());
}
