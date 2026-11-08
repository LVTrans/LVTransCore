#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <lvtrans/elements/pipe.hpp>
#include <lvtrans/elements/reservoir.hpp>
#include <sstream>
#include <string>
#include <vector>
#include "../test_helpers.hpp"
#include "lvtrans/const.hpp"
#include "lvtrans/controller_connections.hpp"
#include "lvtrans/elements/base_pipe.hpp"
#include "lvtrans/elements/constant_level_left.hpp"
#include "lvtrans/elements/constant_level_right.hpp"
#include "lvtrans/elements/pid_valve.hpp"
#include "lvtrans/elements/simple_connection.hpp"
#include "lvtrans/elements/surge_shaft_standard.hpp"
#include "lvtrans/elements/t_1in_2out.hpp"
#include "lvtrans/elements/valve_internal_servo.hpp"
#include "lvtrans/plant.hpp"

// Example taken from W&S book
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

  ValveInternalServoParameters valve_config{};
  valve_config.initial_opening = tau_i;
  valve_config.cvp = CVP;

  Plant plant(system_dt);

  auto pipe = plant.add_element<Pipe>(pipe_config, system_dt).value();
  auto valve =
      plant.add_element<ValveInternalServo>(valve_config, system_dt).value();
  auto reservoir = plant.add_element<ConstantLevelLeft>(HR).value();

  ASSERT_EQ(HydraulicConnections::connect(*pipe, PipeEnd::Left, *reservoir),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe, PipeEnd::Right, *valve),
            ConnectResult::Ok);

  const int Kmax = static_cast<int>(0.5 * Tmax / dt) + 1;
  for (int k = 1; k < Kmax; ++k) {
    const double t = plant.get_current_time() + system_dt;
    const double opening =
        t < tc ? tau_i - (tau_i - tau_f) * std::pow(t / tc, em) : tau_f;
    ASSERT_EQ(valve->modify(SetValveOpening{opening}), ModificationResult::Ok);
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

// Reservoir 1 <->Pipe 1 <->Simple Connection 1 <-->Pipe 2 <->Simple Connection
// 2 <->Pipe 3 <->ValveInternalServo <->Pipe 4 <->Reservoir 2 (LabVIEW)
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

  ValveInternalServoParameters valve_params{.cvp = 10e3,
                                            .cvm = 10e3,
                                            .closing_time_high = 10,
                                            .closing_time_low = 10,
                                            .switch_over = 500e-3,
                                            .servo_time = 1.0,
                                            .initial_opening = 1.0,
                                            .use_cv_file = false,
                                            .cv_file = "cv-file.txt",
                                            .diameter = 1.0};

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

  ASSERT_EQ(
      HydraulicConnections::connect(*pipe1, PipeEnd::Left, *reservoir_left),
      ConnectResult::Ok);
  ASSERT_EQ(
      HydraulicConnections::connect(*pipe1, PipeEnd::Right, *simple_conn1),
      ConnectResult::Ok);

  ASSERT_EQ(HydraulicConnections::connect(*pipe2, PipeEnd::Left, *simple_conn1),
            ConnectResult::Ok);
  ASSERT_EQ(
      HydraulicConnections::connect(*pipe2, PipeEnd::Right, *simple_conn2),
      ConnectResult::Ok);

  ASSERT_EQ(HydraulicConnections::connect(*pipe3, PipeEnd::Left, *simple_conn2),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe3, PipeEnd::Right, *valve2),
            ConnectResult::Ok);

  ASSERT_EQ(HydraulicConnections::connect(*pipe4, PipeEnd::Left, *valve2),
            ConnectResult::Ok);
  ASSERT_EQ(
      HydraulicConnections::connect(*pipe4, PipeEnd::Right, *reservoir_right),
      ConnectResult::Ok);

  const auto test_file_path =
      get_mock_data_file_path("generated/labview_test_B.csv");
  std::ofstream output_file(test_file_path);

  ASSERT_TRUE(output_file.is_open());

  output_file << "t,P1_H0,P1_HN,P1_Q1,P1_Q2,P2_H0,P2_HN,P2_Q1,P2_Q2,P3_H0,P3_"
                 "HN,P3_Q1,P3_Q2,P4_H0,P4_HN,P4_Q1,P4_Q2,V_H1,V_H2,V_Q1,V_Q2,V_"
                 "tau_SP,V_tau_OP\n";

  std::ifstream reference(
      get_mock_data_file_path("expected/labview_test_B.txt"));
  ASSERT_TRUE(reference.is_open());
  std::string line;
  ASSERT_TRUE(static_cast<bool>(std::getline(reference, line)));  // Header.
  ASSERT_EQ(
      line,
      "Time(s)	P1_H0	P1_HN	P1_Q1	P1_Q2	P2_H0	P2_HN	P2_Q1	"
      "P2_Q2	P3_H0	P3_HN	P3_Q1	P3_Q2	P4_H0	P4_HN	P4_Q1	"
      "P4_Q2	V_H1	V_H2	V_Q1	V_Q2	V_tau_SP	V_tau_OP");
  output_file << std::setprecision(8);

  // TODO: Lower the tolerance for higher accuracy
  constexpr double head_tolerance = 5e-6;
  constexpr double flow_tolerance = 5e-6;
  constexpr double opening_tolerance = 5.1e-7;
  int row = 0;
  while (std::getline(reference, line)) {
    std::replace(line.begin(), line.end(), ',', '.');
    std::istringstream fields(line);
    double reported_time, p1_h0, p1_hn, p1_q0, p1_qn, p2_h0, p2_hn, p2_q0,
        p2_qn, p3_h0, p3_hn, p3_q0, p3_qn, p4_h0, p4_hn, p4_q0, p4_qn;

    double v_h1, v_h2, v_q1, v_q2, v_tau_sp, v_tau_op;

    ASSERT_TRUE(static_cast<bool>(
        fields >> reported_time >> p1_h0 >> p1_hn >> p1_q0 >> p1_qn >> p2_h0 >>
        p2_hn >> p2_q0 >> p2_qn >> p3_h0 >> p3_hn >> p3_q0 >> p3_qn >> p4_h0 >>
        p4_hn >> p4_q0 >> p4_qn >> v_h1 >> v_h2 >> v_q1 >> v_q2 >> v_tau_sp >>
        v_tau_op));
    ASSERT_TRUE(fields.eof());

    const auto sample_step =
        static_cast<std::uint64_t>(std::llround(reported_time / dt));
    ASSERT_GE(sample_step, plant.get_num_terations());
    while (plant.get_num_terations() < sample_step) {
      plant.step();
      const auto valve_state = valve2->get_state_as<ValveInternalServoState>();
      output_file << plant.get_current_time() << ',' << pipe1->get_H().front()
                  << ',' << pipe1->get_H().back() << ','
                  << pipe1->get_Q().front() << ',' << pipe1->get_Q().back()
                  << ',' << pipe2->get_H().front() << ','
                  << pipe2->get_H().back() << ',' << pipe2->get_Q().front()
                  << ',' << pipe2->get_Q().back() << ','
                  << pipe3->get_H().front() << ',' << pipe3->get_H().back()
                  << ',' << pipe3->get_Q().front() << ','
                  << pipe3->get_Q().back() << ',' << pipe4->get_H().front()
                  << ',' << pipe4->get_H().back() << ','
                  << pipe4->get_Q().front() << ',' << pipe4->get_Q().back()
                  << ',' << valve_state.H1 << ',' << valve_state.H2 << ','
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

// LabVIEW: ResA -> Pipe_01 -> C_01 -> Pipe_02 -> C_02 -> Pipe_03
//                  -> PID-controlled Valve -> Pipe_04 -> ResB
// Unlike Complex, this test adds PIDValve feedback from Pipe_03 flow.
// This is an integration/smoke test, not yet a LabVIEW reference comparison.
TEST(MultiElementsTest, ComplexPIDValve) {
  using namespace lvtrans;
  constexpr double dt = 0.1;
  constexpr int num_steps = 200;  // 20 seconds simulated
  Plant plant(dt);

  const ReservoirParameters res_a{.H0 = 100.0, .cvp = 100e3, .cvm = 100e3};
  const ReservoirParameters res_b{.H0 = 0.0, .cvp = 100e3, .cvm = 100e3};
  const LossCoefficients connection_params{.cvp = 100e3, .cvm = 100e3};
  const PipeParameters pipe_params{.H0 = 0.0,
                                   .Q0 = 0.0,
                                   .length = 1000.0,
                                   .diameter = 1.0,
                                   .area = 785.4e-3,
                                   .periphery = 3.1416,
                                   .epsilon = 10e-3,
                                   .ny = 1.1e-6,
                                   .rho = 1e3,
                                   .f = 0.01,
                                   .a = 1200.0,
                                   .z0 = 0.0,
                                   .z1 = 0.0,
                                   .lambda = 500e3,
                                   .f_max = 100.0,
                                   .use_diameter = true,
                                   .use_full_moody = false,
                                   .use_Dh = false};
  const ValveInternalServoParameters valve_params{.cvp = 10e3,
                                                  .cvm = 10e3,
                                                  .closing_time_high = 10.0,
                                                  .closing_time_low = 10.0,
                                                  .switch_over = 0.5,
                                                  .servo_time = 1.0,
                                                  .initial_opening = 1.0,
                                                  .use_cv_file = false,
                                                  .cv_file = "cv-file.txt",
                                                  .diameter = 1.0};
  const PIDValveParameters pid_params{
      .Q_wanted = 5.0, .SP_Q = 4.0, .Kp = 1.0, .Ti = 10.0, .Td = 0.0};

  auto* reservoir_left = plant.add_element<ConstantLevelLeft>(res_a).value();
  auto* pipe1 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto* connection1 =
      plant.add_element<SimpleConnection>(connection_params).value();
  auto* pipe2 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto* connection2 =
      plant.add_element<SimpleConnection>(connection_params).value();
  auto* pipe3 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto* valve = plant.add_element<ValveInternalServo>(valve_params, dt).value();
  auto* pipe4 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto* reservoir_right = plant.add_element<ConstantLevelRight>(res_b).value();
  auto* pid = plant.add_element<PIDValve>(pid_params, dt).value();

  ASSERT_EQ(
      HydraulicConnections::connect(*pipe1, PipeEnd::Left, *reservoir_left),
      ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe1, PipeEnd::Right, *connection1),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe2, PipeEnd::Left, *connection1),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe2, PipeEnd::Right, *connection2),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe3, PipeEnd::Left, *connection2),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe3, PipeEnd::Right, *valve),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe4, PipeEnd::Left, *valve),
            ConnectResult::Ok);
  ASSERT_EQ(
      HydraulicConnections::connect(*pipe4, PipeEnd::Right, *reservoir_right),
      ConnectResult::Ok);

  ASSERT_TRUE(ControllerConnections::connect(*pid, *valve));
  ASSERT_EQ(pid->get_controllable(), valve);
  ASSERT_EQ(valve->get_controller(), pid);

  // Explicitly enable automatic PID control. These mode/filter options
  // are assumptions until the LabVIEW PID front-panel settings are known.
  pid->set_controls(/*active=*/true, /*automatic=*/true,
                    /*antiwindup=*/true, /*manual=*/1.0,
                    /*Tt=*/1.0, /*Tf=*/0.1);
  pid->set_SP_Q(pid_params.SP_Q);

  const auto output_path =
      get_mock_data_file_path("generated/labview_test_pid_valve.csv");
  std::ofstream output(output_path);
  ASSERT_TRUE(output.is_open());
  output << std::setprecision(10);
  output << "t,P1_Q0,P1_QN,P2_Q0,P2_QN,P3_Q0,P3_QN,P4_Q0,P4_QN,"
            "V_Q,V_tau_SP,V_tau_OP,PID_PV_Q,PID_output\n";

  for (int k = 0; k < num_steps; ++k) {
    // 'HandQ get Q' in LabVIEW reads Pipe_03 flow. Supply that measurement
    // to the controller before the next step. Plant::step() runs pipes,
    // then controllers, then non-pipe elements.
    const double measured_q = pipe3->get_Q().back();
    pid->set_PV_Q(measured_q);
    plant.step();

    const auto valve_state = valve->get_state_as<ValveInternalServoState>();
    SCOPED_TRACE("step " + std::to_string(k + 1));
    EXPECT_TRUE(std::isfinite(pid->get_output()));
    EXPECT_TRUE(std::isfinite(pid->get_PV_Q()));
    EXPECT_TRUE(std::isfinite(valve->get_tau()));
    EXPECT_TRUE(std::isfinite(valve->get_setpoint()));
    EXPECT_TRUE(std::isfinite(valve_state.Q));
    for (auto* pipe : {pipe1, pipe2, pipe3, pipe4}) {
      ASSERT_FALSE(pipe->get_H().empty());
      ASSERT_FALSE(pipe->get_Q().empty());
      EXPECT_TRUE(std::isfinite(pipe->get_H().front()));
      EXPECT_TRUE(std::isfinite(pipe->get_H().back()));
      EXPECT_TRUE(std::isfinite(pipe->get_Q().front()));
      EXPECT_TRUE(std::isfinite(pipe->get_Q().back()));
    }

    output << plant.get_current_time() << ',' << pipe1->get_Q().front() << ','
           << pipe1->get_Q().back() << ',' << pipe2->get_Q().front() << ','
           << pipe2->get_Q().back() << ',' << pipe3->get_Q().front() << ','
           << pipe3->get_Q().back() << ',' << pipe4->get_Q().front() << ','
           << pipe4->get_Q().back() << ',' << valve_state.Q << ','
           << valve->get_setpoint() << ',' << valve->get_tau() << ','
           << pid->get_PV_Q() << ',' << pid->get_output() << '\n';
  }
  EXPECT_EQ(plant.get_num_terations(), static_cast<std::uint64_t>(num_steps));
  EXPECT_FALSE(output.bad());
}

// ResA -> Pipe_01 -> T1In2OutN -> Pipe_02 -> ResB
//                              \-> Pipe_03 -> ResC
TEST(MultiElementsTest, T1In2Out) {
  using namespace lvtrans;
  constexpr double dt = 0.1;
  Plant plant(dt);

  const ReservoirParameters res_a{.H0 = 10.0, .cvp = 100e3, .cvm = 100e3};
  const ReservoirParameters res_out{.H0 = 0.0, .cvp = 100e3, .cvm = 100e3};
  const TConnectionParameters junction_params{
      .cv1 = 100.0, .cv2 = 100.0, .cv3 = 100.0};
  const PipeParameters pipe_params{.H0 = 0.0,
                                   .Q0 = 0.0,
                                   .length = 1200.0,
                                   .diameter = 1.0,
                                   .area = 785.4e-3,
                                   .periphery = 3.1416,
                                   .epsilon = 10e-3,
                                   .ny = 1.1e-6,
                                   .rho = 1e3,
                                   .f = 0.01,
                                   .a = 1200.0,
                                   .z0 = 0.0,
                                   .z1 = 0.0,
                                   .lambda = 500e3,
                                   .f_max = 100.0,
                                   .use_diameter = true,
                                   .use_full_moody = false,
                                   .use_Dh = false};

  auto* resA = plant.add_element<ConstantLevelLeft>(res_a).value();
  auto* pipe1 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto* junction = plant.add_element<T1In2OutN>(junction_params).value();
  auto* pipe2 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto* resB = plant.add_element<ConstantLevelRight>(res_out).value();
  auto* pipe3 = plant.add_element<Pipe>(pipe_params, dt).value();
  auto* resC = plant.add_element<ConstantLevelRight>(res_out).value();

  ASSERT_EQ(HydraulicConnections::connect(*pipe1, PipeEnd::Left, *resA),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe1, PipeEnd::Right, *junction),
            ConnectResult::Ok);  // Inlet 1
  ASSERT_EQ(HydraulicConnections::connect(*pipe2, PipeEnd::Left, *junction, 0),
            ConnectResult::Ok);  // Outlet 2 (right)
  ASSERT_EQ(HydraulicConnections::connect(*pipe3, PipeEnd::Left, *junction, 1),
            ConnectResult::Ok);  // Outlet 3 (north)
  ASSERT_EQ(HydraulicConnections::connect(*pipe2, PipeEnd::Right, *resB),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*pipe3, PipeEnd::Right, *resC),
            ConnectResult::Ok);

  const std::string header =
      "Time(s)\tP1_H1_abs\tP1_H2_abs\tP1_Q1\tP1_Q2"
      "\tP2_H1_abs\tP2_H2_abs\tP2_Q1\tP2_Q2"
      "\tP3_H1_abs\tP3_H2_abs\tP3_Q1\tP3_Q2"
      "\tT_H1\tT_H2\tT_H3\tT_Q1\tT_Q2\tT_Q3";

  const auto output_path =
      get_mock_data_file_path("generated/labview_test_T1in2OutN.txt");
  std::ofstream output(output_path);
  ASSERT_TRUE(output.is_open());
  output << header << '\n' << std::setprecision(12);

  std::ifstream reference(
      get_mock_data_file_path("expected/labview_test_T1in2OutN.txt"));
  std::vector<std::array<double, 19>> expected_rows;
  std::vector<std::uint64_t> sample_steps;
  if (reference.is_open()) {
    std::string line;
    ASSERT_TRUE(static_cast<bool>(std::getline(reference, line)));
    ASSERT_EQ(line, header);
    while (std::getline(reference, line)) {
      if (line.empty()) continue;
      std::replace(line.begin(), line.end(), ',', '.');
      std::istringstream fields(line);
      std::array<double, 19> row{};
      for (double& value : row) {
        ASSERT_TRUE(static_cast<bool>(fields >> value)) << "Row: " << line;
      }
      fields >> std::ws;
      ASSERT_TRUE(fields.eof()) << "Extra reference columns";
      const auto step = static_cast<std::uint64_t>(std::llround(row[0] / dt));
      ASSERT_NEAR(static_cast<double>(step) * dt, row[0], 1e-8);
      ASSERT_TRUE(sample_steps.empty() || step > sample_steps.back());
      sample_steps.push_back(step);
      expected_rows.push_back(row);
    }
    ASSERT_FALSE(reference.bad());
    ASSERT_FALSE(expected_rows.empty());
  } else {
    // Sample at t = 0.3, 1.3, 2.3, ... 59.3 s, as in previous LabVIEW logs.
    for (std::uint64_t step = 3; step <= 593; step += 10) {
      sample_steps.push_back(step);
    }
  }

  for (std::size_t row = 0; row < sample_steps.size(); ++row) {
    const auto step = sample_steps[row];
    ASSERT_GE(step, plant.get_num_terations());
    plant.step(step - plant.get_num_terations());

    const auto& h1 = pipe1->get_H();
    const auto& q1 = pipe1->get_Q();
    const auto& h2 = pipe2->get_H();
    const auto& q2 = pipe2->get_Q();
    const auto& h3 = pipe3->get_H();
    const auto& q3 = pipe3->get_Q();
    ASSERT_FALSE(h1.empty());
    ASSERT_FALSE(q1.empty());
    ASSERT_FALSE(h2.empty());
    ASSERT_FALSE(q2.empty());
    ASSERT_FALSE(h3.empty());
    ASSERT_FALSE(q3.empty());

    const std::array<double, 19> actual{plant.get_current_time(),
                                        h1.front(),
                                        h1.back(),
                                        q1.front(),
                                        q1.back(),
                                        h2.front(),
                                        h2.back(),
                                        q2.front(),
                                        q2.back(),
                                        h3.front(),
                                        h3.back(),
                                        q3.front(),
                                        q3.back(),
                                        h1.back(),
                                        h2.front(),
                                        h3.front(),
                                        q1.back(),
                                        q2.front(),
                                        q3.front()};

    for (std::size_t i = 0; i < actual.size(); ++i) {
      if (i != 0) output << '\t';
      output << actual[i];
      EXPECT_TRUE(std::isfinite(actual[i]));
    }
    output << '\n';

    SCOPED_TRACE("T1In2OutN at t=" + std::to_string(actual[0]));
    // Equal downstream branches: flow and head should match.
    EXPECT_NEAR(q2.front(), q3.front(), 1e-8);
    EXPECT_NEAR(q2.back(), q3.back(), 1e-8);
    EXPECT_NEAR(h2.front(), h3.front(), 1e-8);
    EXPECT_NEAR(h2.back(), h3.back(), 1e-8);
    // Junction continuity, all pipe flows positive left-to-right.
    EXPECT_NEAR(q1.back(), q2.front() + q3.front(), 1e-5);

    if (reference.is_open()) {
      EXPECT_NEAR(actual[0], expected_rows[row][0], 1e-8);
      for (std::size_t i = 1; i < actual.size(); ++i) {
        EXPECT_NEAR(actual[i], expected_rows[row][i], 5e-5)
            << "Column " << i << " at t=" << actual[0];
      }
    }
  }
  EXPECT_FALSE(output.bad());
}

TEST(MultiElementsTest, SurgeShaft) {
  GTEST_SKIP()
      << "Skipping SurgeShaft test due to bugs related Surge Shaft's states";
  using namespace lvtrans;
  constexpr double dt = 0.1;
  Plant plant(dt);

  const ReservoirParameters dammen_params{
      .H0 = 350.0, .cvp = 10e3, .cvm = 10e3};
  const ReservoirParameters elva_params{.H0 = 5.0, .cvp = 100e3, .cvm = 100e3};
  const SurgeShaftStandardParameters shaft_params{.use_area = true,
                                                  .area = 314.159265,
                                                  .diameter = 20.0,
                                                  .f = 50e-3,
                                                  .cvp = 10e3,
                                                  .cvm = 10e3,
                                                  .H_start = 350.0};

  const PipeParameters tun1_params{.H0 = 345.0,
                                   .Q0 = 34.0,
                                   .length = 4000.0,
                                   .diameter = 8.0,
                                   .area = 0.0,
                                   .periphery = 0.0,
                                   .epsilon = 10e-3,
                                   .ny = 1.1e-6,
                                   .rho = 1e3,
                                   .f = 60e-3,
                                   .a = 1.2e3,
                                   .z0 = 330.0,
                                   .z1 = 330.0,
                                   .lambda = 500e3,
                                   .f_max = 100.0,
                                   .use_diameter = true,
                                   .use_full_moody = false,
                                   .use_Dh = false};

  auto tun2_params = tun1_params;
  tun2_params.length = 500.0;
  tun2_params.diameter = 3.0;
  tun2_params.f = 10e-3;
  tun2_params.z1 = 0.0;
  tun2_params.lambda = 500e3;

  auto dammen = plant.add_element<ConstantLevelLeft>(dammen_params).value();
  auto tun1 = plant.add_element<Pipe>(tun1_params, dt).value();
  auto shaft = plant.add_element<SurgeShaftStandard>(shaft_params, dt).value();
  auto tun2 = plant.add_element<Pipe>(tun2_params, dt).value();
  auto elva = plant.add_element<ConstantLevelRight>(elva_params).value();

  // dammen -> tun1 -> svingesjakt -> tun2 -> elva
  ASSERT_EQ(HydraulicConnections::connect(*tun1, PipeEnd::Left, *dammen),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*tun1, PipeEnd::Right, *shaft),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*tun2, PipeEnd::Left, *shaft),
            ConnectResult::Ok);
  ASSERT_EQ(HydraulicConnections::connect(*tun2, PipeEnd::Right, *elva),
            ConnectResult::Ok);

  const auto test_file_path =
      get_mock_data_file_path("generated/labview_test_D.csv");
  std::ofstream output_file(test_file_path);

  ASSERT_TRUE(output_file.is_open());

  output_file << "t,s_H1,s_H2,s_H3,s_Q1,s_Q2,s_Q3\n";

  std::ifstream reference(
      get_mock_data_file_path("expected/labview_test_D.txt"));
  ASSERT_TRUE(reference.is_open());
  std::string line;
  ASSERT_TRUE(static_cast<bool>(std::getline(reference, line)));  // Header.
  output_file << std::setprecision(8);

  constexpr double head_tolerance = 5e-1;
  constexpr double flow_tolerance = 5e-1;
  int row = 0;
  while (std::getline(reference, line)) {
    std::replace(line.begin(), line.end(), ',', '.');
    std::istringstream fields(line);
    double reported_time, s_h1, s_h2, s_h3, s_q1, s_q2, s_q3;
    double h_floor, h_datum;

    ASSERT_TRUE(static_cast<bool>(fields >> reported_time >> s_h1 >> s_h2 >>
                                  s_h3 >> s_q1 >> s_q2 >> s_q3 >> h_floor >>
                                  h_datum));
    fields >> std::ws;
    ASSERT_TRUE(fields.eof()) << "Unexpected extra reference columns";

    const auto sample_step =
        static_cast<std::uint64_t>(std::llround(reported_time / dt));
    ASSERT_GE(sample_step, plant.get_num_terations());
    ASSERT_NEAR(static_cast<double>(sample_step) * dt, reported_time, 1e-10);
    while (plant.get_num_terations() < sample_step) {
      plant.step();
      auto shaft_state = shaft->get_state_as<SurgeShaftState>();
      output_file << plant.get_current_time() << ',' << shaft_state.H1 << ','
                  << shaft_state.H2 << ',' << shaft_state.H3 << ','
                  << shaft_state.Q1 << ',' << shaft_state.Q2 << ','
                  << shaft_state.Q3 << '\n';
    }

    SCOPED_TRACE("LabVIEW reported t=" + std::to_string(reported_time) +
                 ", solver t=" + std::to_string(plant.get_current_time()) +
                 " (row " + std::to_string(row) + ")");
    ASSERT_EQ(plant.get_num_terations(), sample_step);

    auto shaft_state = shaft->get_state_as<SurgeShaftState>();

    EXPECT_NEAR(shaft_state.H1, s_h1, head_tolerance);
    EXPECT_NEAR(shaft_state.H2, s_h2, head_tolerance);
    EXPECT_NEAR(shaft_state.H3, s_h3, head_tolerance);
    EXPECT_NEAR(shaft_state.Q1, s_q1, flow_tolerance);
    EXPECT_NEAR(shaft_state.Q2, s_q2, flow_tolerance);
    EXPECT_NEAR(shaft_state.Q3, s_q3, flow_tolerance);

    ++row;
  }
  EXPECT_GT(row, 0);
  EXPECT_FALSE(reference.bad());
}
