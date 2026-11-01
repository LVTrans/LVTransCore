#include <gtest/gtest.h>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <lvtrans/elements/pipe.hpp>
#include <lvtrans/elements/reservoir.hpp>
#include <lvtrans/elements/valve.hpp>
#include "../test_helpers.hpp"
#include "lvtrans/const.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"
#include "lvtrans/plant.hpp"

TEST(MultiElementsTest, ReservoirPipeValve) {
  double a = 1200.0;   // Wave propagation velocity [m/s]
  double f = 0.018;    // Darcy-Weisbach friction factor
  double tau_i = 1.0;  // Initial valve position
  double tau_f = 0.0;  // Final valve position
  double tc = 2.1;     // Valve operating/closure time [s]
  double em = 0.75;    // Exponent defining valve motion

  using namespace lvtrans;

  double HR = 150.0;    // Reservoir head above datum [m]
  double Tmax = 4.3;    // Duration of transient [s]
  double CdA0 = 0.009;  // Valve coefficient/opening parameter

  PipeParameters pipe_config = {
      .length = 600.0,
      .diameter = 0.5,
      .f = f,
      .a = a,
      .z0 = 10,
      .z1 = 15,
      .lambda = 0,
  };

  const auto test_file_path =
      get_mock_data_file_path("generated/reservoir_pipe_valve.csv");
  std::ofstream output_file(test_file_path);

  ASSERT_TRUE(output_file.is_open());

  const double system_dt = 0.1;
  const auto segments =
      calculate_nodes_temp(pipe_config.lambda, pipe_config.length, system_dt,
                           pipe_config.a, pipe_config.rho);
  const double dx = calculate_dx(pipe_config.length, segments);

  const double dt = dx / a;

  output_file << "t,tau,H_valve,Q_valve\n";

  const auto num_segments =
      static_cast<size_t>(calculate_num_segments(segments));
  std::vector<double> H0_pipe(num_segments + 1, 0.0);
  std::vector<double> Q0_pipe(num_segments + 1, 0.0);

  const double area = calculate_pipe_area(
      pipe_config.area, pipe_config.diameter, pipe_config.dimension);
  const double R = calculate_R(pipe_config.f, dx, pipe_config.diameter, area);

  const double Q0 =
      std::sqrt(2.0 * consts::g * CdA0 * CdA0 * HR /
                (R * segments * 2.0 * consts::g * CdA0 * CdA0 + 1.0));

  const double H0 = HR - R * segments * Q0 * Q0;

  const double Qi = std::sqrt(HR * Q0 * Q0 * tau_i * tau_i /
                              (R * segments * Q0 * Q0 * tau_i * tau_i + H0));

  for (size_t i = 0; i < H0_pipe.size(); ++i) {
    H0_pipe[i] = HR - static_cast<double>(i) * R * Qi * Qi;
    Q0_pipe[i] = Qi;
  }

  pipe_config.H0 = std::move(H0_pipe);
  pipe_config.Q0 = std::move(Q0_pipe);

  const double CVP = 0.5 * Q0 * Q0 / H0;

  ValveParameters valve_config{};
  valve_config.tau_i = tau_i;
  valve_config.tau_f = tau_f;
  valve_config.tc = tc;
  valve_config.em = em;
  valve_config.cvp = CVP;

  Plant plant(system_dt);

  auto pipe = plant.add_element<Pipe>(pipe_config, system_dt).value();
  auto valve = plant.add_element<Valve>(valve_config).value();
  auto reservoir = plant.add_element<ConstantLevelLeft>(HR).value();

  ASSERT_EQ(HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe, PipeEnd::Right, *valve),
            ConnectResult::Ok);

  const int Kmax = static_cast<int>(0.5 * Tmax / dt) + 1;
  for (int k = 1; k < Kmax; ++k) {
    plant.step();

    output_file << plant.get_current_time() << "," << valve->get_tau() << ","
                << valve->get_H(CharacteristicsType::Plus, pipe->get_ID())
                << ","
                << valve->get_Q(CharacteristicsType::Plus, pipe->get_ID(), {})
                << '\n';
  }

  output_file.close();

  const auto expected_file_path =
      get_mock_data_file_path("expected/reservoir_pipe_valve_g982.csv");

  EXPECT_TRUE(compare_csv_files(test_file_path, expected_file_path));
}

// R <-> P <-> R (comparison against LabVIEW)
TEST(MultiElementsTest, ReservoirPipeReservoir) {
  using namespace lvtrans;
  constexpr double dt = 0.1;
  Plant plant(dt);
  ReservoirParameters reservoir_params{.H0 = 10.0, .cvp = 100e3, .cvm = 100e3};
  ReservoirParameters reservoir_params2{.H0 = 0.0, .cvp = 100e3, .cvm = 100e3};
  PipeParameters pipe_params{.H0 = 0.0,
                             .Q0 = 0.0,
                             .length = 1000,
                             .diameter = 1,
                             .area = 785.400000e-3,
                             .periphery = 3.141600,
                             .epsilon = 10e-3,
                             .ny = 1.1e-6,
                             .rho = 1e3,
                             .f = 0.01,
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
      plant.add_element<ConstantLevelRight>(reservoir_params2).value();
  ASSERT_EQ(
      HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir_left),
      ConnectResult::Ok);
  ASSERT_EQ(
      HydraulicConnections::connect(*pipe, PipeEnd::Right, *reservoir_right),
      ConnectResult::Ok);

  const auto test_file_path =
      get_mock_data_file_path("generated/labview_test_A.csv");
  std::ofstream output_file(test_file_path);

  ASSERT_TRUE(output_file.is_open());

  output_file << "t,H_0,H_N,Q_0,Q_N,LR_Q,RR_Q\n";

  std::ifstream reference(
      get_mock_data_file_path("expected/labview_test_A.txt"));
  ASSERT_TRUE(reference.is_open());
  std::string line;
  ASSERT_TRUE(static_cast<bool>(std::getline(reference, line)));
  output_file << std::setprecision(7);

  // TODO: Lower the tolerance for higher accuracy
  constexpr double export_tolerance = 5.1e-6;
  int row = 0;
  while (std::getline(reference, line)) {
    std::replace(line.begin(), line.end(), ',', '.');
    SCOPED_TRACE("Parsing line " + std::to_string(row) + ": " + line);
    std::istringstream fields(line);
    double reported_time, h0, hn, q0, qn, lr_q, rr_q;

    ASSERT_TRUE(static_cast<bool>(fields >> reported_time >> h0 >> hn >> q0 >>
                                  qn >> lr_q >> rr_q));
    const auto sample_step =
        static_cast<std::uint64_t>(std::llround(reported_time / dt));
    plant.step(sample_step - plant.get_num_terations());

    SCOPED_TRACE("LabVIEW reported t=" + std::to_string(reported_time) +
                 ", solver t=" + std::to_string(plant.get_current_time()) +
                 " (row " + std::to_string(row) + ")");

    EXPECT_NEAR(pipe->get_H().front(), h0, export_tolerance);
    EXPECT_NEAR(pipe->get_H().back(), hn, export_tolerance);
    EXPECT_NEAR(pipe->get_Q().front(), q0, export_tolerance);
    EXPECT_NEAR(pipe->get_Q().back(), qn, export_tolerance);
    EXPECT_NEAR(reservoir_left->get_state_as<ReservoirState>().Q, lr_q,
                export_tolerance);
    EXPECT_NEAR(reservoir_right->get_state_as<ReservoirState>().Q, rr_q,
                export_tolerance);

    output_file << plant.get_current_time() << ',' << pipe->get_H().front()
                << ',' << pipe->get_H().back() << ',' << pipe->get_Q().front()
                << ',' << pipe->get_Q().back() << ','
                << reservoir_left->get_state_as<ReservoirState>().Q << ","
                << reservoir_right->get_state_as<ReservoirState>().Q << '\n';
    ++row;
  }
  EXPECT_FALSE(reference.bad());
}

// clang-format off
    // Reservoir 1 <->Pipe 1 <->Simple Connection 1 <-->Pipe 2 <->Simple Connection 2 <->Pipe 3 <->Valve <->Pipe 4 <->Reservoir 2
TEST(MultiElementsTest, Complex) {
  using namespace lvtrans;
  constexpr double dt = 0.1;
  Plant plant(dt);

  ReservoirParameters reservoir_params{
      .H0 = 100.000000E+0, .cvp = 100.000000E+3, .cvm = 100.000000E+3};
  ReservoirParameters reservoir_params2{
      .H0 = 0.0, .cvp = 100.000000E+3, .cvm = 100.000000E+3};

  PipeParameters pipe_params = {
      .H0 = 0.0,
      .Q0 = 0.0,
      .length = 1000,
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

  LossCoefficients simple_connection_params{.cvp = 100e3, .cvm = 100e3};

  ValveInternalServoParameters valve_params{
      .cvp = 10e3, 
      .cvm = 10e3,
      .closing_time_high = 10,
      .closing_time_low = 10,
      .switch_over = 500e-3,
      .servo_time = 1.0,
      .initial_opening = 1.0,
      .use_cv_file = false,
      .cv_file = "cv-file.txt",
      .diameter = 1.0
    };

  auto reservoir_left =
      plant.add_element<ConstantLevelLeft>(reservoir_params).value();
  auto pipe1 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto simple_conn1 =
      plant.add_element<SimpleConnection>(simple_connection_params).value();
  auto valve2 = plant.add_element<ValveInternalServo>(valve_params, dt).value();
  auto pipe2 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto pipe3 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto pipe4 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto simple_conn2 =
      plant.add_element<SimpleConnection>(simple_connection_params).value();
  auto reservoir_right =
      plant.add_element<ConstantLevelRight>(reservoir_params2).value();

    ASSERT_EQ(HydraulicConnections::connect(*pipe1, PipeEnd::Left, *reservoir_left), ConnectResult::Ok);
    ASSERT_EQ(HydraulicConnections::connect(*pipe1, PipeEnd::Right, *simple_conn1), ConnectResult::Ok);

    ASSERT_EQ(HydraulicConnections::connect(*pipe2, PipeEnd::Left, *simple_conn1), ConnectResult::Ok);
    ASSERT_EQ(HydraulicConnections::connect(*pipe2, PipeEnd::Right, *simple_conn2), ConnectResult::Ok);

    ASSERT_EQ(HydraulicConnections::connect(*pipe3, PipeEnd::Left, *simple_conn2), ConnectResult::Ok);
    ASSERT_EQ(HydraulicConnections::connect(*pipe3, PipeEnd::Right, *valve2), ConnectResult::Ok);

    ASSERT_EQ(HydraulicConnections::connect(*pipe4, PipeEnd::Left, *valve2), ConnectResult::Ok);
    ASSERT_EQ(HydraulicConnections::connect(*pipe4, PipeEnd::Right, *reservoir_right), ConnectResult::Ok);


  const auto test_file_path =
      get_mock_data_file_path("generated/labview_test_B.csv");
  std::ofstream output_file(test_file_path);

  ASSERT_TRUE(output_file.is_open());

  output_file << "t,P1_H0,P1_HN,P1_Q1,P1_Q2,P2_H0,P2_HN,P2_Q1,P2_Q2,P3_H0,P3_HN,P3_Q1,P3_Q2,P4_H0,P4_HN,P4_Q1,P4_Q2,V_H1,V_H2,V_Q1,V_Q2,V_tau_SP,V_tau_OP\n";

  std::ifstream reference(
      get_mock_data_file_path("expected/labview_test_B.txt"));
  ASSERT_TRUE(reference.is_open());
  std::string line;
  ASSERT_TRUE(static_cast<bool>(std::getline(reference, line)));  // Header.
  ASSERT_EQ(line, "Time(s)	P1_H0	P1_HN	P1_Q1	P1_Q2	P2_H0	P2_HN	P2_Q1	P2_Q2	P3_H0	P3_HN	P3_Q1	P3_Q2	P4_H0	P4_HN	P4_Q1	P4_Q2	V_H1	V_H2	V_Q1	V_Q2	V_tau_SP	V_tau_OP");
  output_file << std::setprecision(8);

    
  // TODO: Lower the tolerance for higher accuracy
  constexpr double head_tolerance = 5e-6;  
  constexpr double flow_tolerance = 1e-6; 
  constexpr double opening_tolerance = 5.1e-7;  
  int row = 0;
  while (std::getline(reference, line)) {
    std::replace(line.begin(), line.end(), ',', '.');
    std::istringstream fields(line);
    double reported_time, p1_h0, p1_hn, p1_q0, p1_qn, p2_h0, p2_hn, p2_q0,
        p2_qn, p3_h0, p3_hn, p3_q0, p3_qn, p4_h0, p4_hn, p4_q0, p4_qn;

    double v_h1, v_h2, v_q1, v_q2, v_tau_sp, v_tau_op;

    ASSERT_TRUE(
        static_cast<bool>(fields >> reported_time >> p1_h0 >> p1_hn >> p1_q0 >> p1_qn >> p2_h0 >>
                          p2_hn >> p2_q0 >> p2_qn >> p3_h0 >> p3_hn >>
                          p3_q0 >> p3_qn >> p4_h0 >> p4_hn >> p4_q0 >>
                          p4_qn >> v_h1 >> v_h2 >> v_q1 >> v_q2 >>
                          v_tau_sp >> v_tau_op));
    ASSERT_TRUE(fields.eof());

    const auto sample_step =
        static_cast<std::uint64_t>(std::llround(reported_time / dt));
    ASSERT_GE(sample_step, plant.get_num_terations());
    while (plant.get_num_terations() < sample_step) {
        plant.step();
        const auto valve_state = valve2->get_state_as<ValveInternalServoState>();
        output_file << plant.get_current_time() << ',' << pipe1->get_H().front()
                << ',' << pipe1->get_H().back() << ',' << pipe1->get_Q().front()
                << ',' << pipe1->get_Q().back() << ','
                << pipe2->get_H().front() << ',' << pipe2->get_H().back() << ','
                << pipe2->get_Q().front() << ',' << pipe2->get_Q().back() << ','
                << pipe3->get_H().front() << ',' << pipe3->get_H().back() << ','
                << pipe3->get_Q().front() << ',' << pipe3->get_Q().back() << ','
                << pipe4->get_H().front() << ',' << pipe4->get_H().back() << ','
                << pipe4->get_Q().front() << ',' << pipe4->get_Q().back() << ','
                << valve_state.H1 << ',' << valve_state.H2 << ','
                << valve_state.Q << ',' << valve_state.Q << ','
            << valve2->get_setpoint() << ',' << valve2->get_tau() << '\n';
    }

    SCOPED_TRACE("LabVIEW reported t=" + std::to_string(reported_time) +
                 ", solver t=" + std::to_string(plant.get_current_time()) +
                 " (row " + std::to_string(row) + ")");
    ASSERT_NEAR(plant.get_current_time(), reported_time, 1e-10);

    const auto valve_state = valve2->get_state_as<ValveInternalServoState>();

    EXPECT_NEAR(pipe1->get_H().front(), p1_h0, head_tolerance);
    EXPECT_NEAR(pipe1->get_H().back(), p1_hn, head_tolerance);
    EXPECT_NEAR(pipe1->get_Q().front(), p1_q0, flow_tolerance);
    EXPECT_NEAR(pipe1->get_Q().back(), p1_qn, flow_tolerance);

    EXPECT_NEAR(pipe2->get_H().front(), p2_h0, head_tolerance);
    EXPECT_NEAR(pipe2->get_H().back(), p2_hn, head_tolerance);
    EXPECT_NEAR(pipe2->get_Q().front(), p2_q0, flow_tolerance);
    EXPECT_NEAR(pipe2->get_Q().back(), p2_qn, flow_tolerance);

    EXPECT_NEAR(pipe3->get_H().front(), p3_h0, head_tolerance);
    EXPECT_NEAR(pipe3->get_H().back(), p3_hn, head_tolerance);
    EXPECT_NEAR(pipe3->get_Q().front(), p3_q0, flow_tolerance);
    EXPECT_NEAR(pipe3->get_Q().back(), p3_qn, flow_tolerance);

    EXPECT_NEAR(pipe4->get_H().front(), p4_h0, head_tolerance);
    EXPECT_NEAR(pipe4->get_H().back(), p4_hn, head_tolerance);
    EXPECT_NEAR(pipe4->get_Q().front(), p4_q0, flow_tolerance);
    EXPECT_NEAR(pipe4->get_Q().back(), p4_qn, flow_tolerance);

    EXPECT_NEAR(valve_state.H1, v_h1, head_tolerance);
    EXPECT_NEAR(valve_state.H2, v_h2, head_tolerance);
    EXPECT_NEAR(valve_state.Q, v_q1, flow_tolerance);
    EXPECT_NEAR(valve_state.Q, v_q2, flow_tolerance);
    EXPECT_NEAR(valve2->get_setpoint(), v_tau_sp, opening_tolerance);
    EXPECT_NEAR(valve2->get_tau(), v_tau_op, opening_tolerance);

    ++row;
  }
}
