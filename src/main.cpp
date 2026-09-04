#include <algorithm>
#include <cmath>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

constexpr double a = 1200.0;  // Wave propagation velocity [m/s]
constexpr double f = 0.018;   // Darcy-Weisbach friction factor
constexpr double g = 9.806;   // Gravitational acceleration [m/s^2]
constexpr double tau_i = 1.0; // Initial valve position
constexpr double tau_f = 0.0; // Final valve position
constexpr double tc = 4.1;    // Valve operating/closure time [s]
constexpr double em = 0.75;   // Exponent defining valve motion

class Element {
public:
  virtual void iterate() const;
  virtual ~Element() = default;

protected:
  std::shared_ptr<Element> m_left_elem;
  std::shared_ptr<Element> m_right_elem;
};

struct NonPipeInput {
  double H{};
  double Q{};
  double B{};
  double R{};
  double t{};
};

class NonPipe : public Element {
public:
  virtual int get_H() const = 0;
  virtual int get_Q(const NonPipeInput &input) const = 0;
  void set_left(std::shared_ptr<Element> elem) { m_left_elem = elem; }
  void set_right(std::shared_ptr<Element> elem) { m_right_elem = elem; }
};

struct PipeConfig {
  double length{};
  double diameter{};
  double f{};
  int num_reaches{};
  int z0{};
  int z1{};
};

// ---RESERVOIR---
//

struct ReservoirOutput {
  double H{};
  double Q{};
};
class Reservoir : public NonPipe {
public:
  Reservoir(const double elevation) : m_elevation(elevation) {}
  void iterate(const NonPipeInput &input, ReservoirOutput &output) {
    output.H = m_elevation;
    output.Q = (m_elevation - input.H + input.B * input.Q) /
               (input.B + input.R * std::abs(input.Q));
  }
  int get_H() const override { return m_elevation; }
  int get_Q(const NonPipeInput &input) const override {
    return m_elevation - input.H;
  }

private:
  double m_elevation{};
};
struct ValveConfig {
  double tau_i{};
  double tau_f{};
  double tc{};
  double em{};
};

struct ValveOutput {
  double H{};
  double Q{};
};
// ---VALVE---
class Valve : public NonPipe {
public:
  Valve(const ValveConfig &conf) : config(conf), m_tau(conf.tau_i) {}
  void iterate(const NonPipeInput &input, ValveOutput &output) {
    if (input.t < config.tc) {
      m_tau = config.tau_i - (config.tau_i - config.tau_f) *
                                 std::pow(input.t / config.tc, config.em);
    } else {
      m_tau = config.tau_f;
    }

    // const double CVP = 0.5 * Q0 * Q0 / H0;
    m_CV = m_tau * m_tau * CVP;

    // C+ characteristic arriving at valve
    const double Cp = input.H + input.B * input.Q;
    const double Bp = input.B + input.R * std::abs(input.Q);

    // Solve characteristic equation +
    // nonlinear valve equation simultaneously.
    output.Q = -m_CV * Bp + std::sqrt(m_CV * m_CV * Bp * Bp + 2.0 * m_CV * Cp);
    output.H = Cp - Bp * output.Q;
  }
  int get_H() const override { return 0; }
  int get_Q(const NonPipeInput &input) const override { return 0; }

private:
  ValveConfig config{};
  double m_tau{};
  double m_CV{};
};

// ---PIPE---
class Pipe : public Element {
public:
  Pipe(PipeConfig conf, const double H0_in, const double Q0_in)
      : m_config(conf), m_area(M_PI * conf.diameter * conf.diameter / 4.0),
        m_dx(conf.length / conf.num_reaches),
        m_R(conf.f * m_dx / (2.0 * g * conf.diameter * m_area * m_area)),
        m_num_nodes(conf.num_reaches + 1), m_B(a / (g * m_area)) {

    m_H.assign(m_num_nodes, H0_in);
    m_Q.assign(m_num_nodes, Q0_in);

    m_Z.resize(m_num_nodes);

    double dZ = (conf.z1 - conf.z0) / conf.num_reaches;
    for (int i = 0; i < m_num_nodes; i++) {
      m_Z[i] = dZ * i + conf.z0;
    }
  }
  void iterate() {
    NonPipeInput input{.H = m_H[0], .Q = m_Q[0], .B = m_B, .R = m_R, .t = 0};
    if (m_left_elem) {
      m_H[0] = m_left_elem->get_H();
      m_Q[0] = m_left_elem->get_Q(input);
    }

    if (m_right_elem) {
      m_H[m_config.num_reaches] = m_right_elem->get_H();
      m_Q[m_config.num_reaches] = m_right_elem->get_Q(input);
    }

    for (int i = 1; i < m_num_nodes; i += 2) {
      const double Cp = m_H[i - 1] + m_B * m_Q[i - 1];
      const double Cm = m_H[i + 1] - m_B * m_Q[i + 1];
      const double Bp = m_B + m_R * std::abs(m_Q[i - 1]);
      const double Bm = m_B + m_R * std::abs(m_Q[i + 1]);

      m_H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
      m_Q[i] = (m_H[i] - Cm) / Bm;
    }

    for (int i = 2; i < m_num_nodes; i += 2) {
      const double Cp = m_H[i - 1] + m_B * m_Q[i - 1];
      const double Cm = m_H[i + 1] - m_B * m_Q[i + 1];
      const double Bp = m_B + m_R * std::abs(m_Q[i - 1]);
      const double Bm = m_B + m_R * std::abs(m_Q[i + 1]);

      m_H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);
      m_Q[i] = (m_H[i] - Cm) / Bm;
    }
  };

  const PipeConfig &config() const { return m_config; }
  double get_R() const { return m_R; }
  double get_B() const { return m_B; }
  const std::vector<double> &get_H() const { return m_H; }
  const std::vector<double> &get_Q() const { return m_Q; }
  void connect_left(std::shared_ptr<Element> elem) {
    m_left_elem = elem;
    elem->set_right(this);
  }
  void connect_right(std::shared_ptr<Element> elem) {
    m_right_elem = elem;
    elem->set_left(this);
  }

private:
  PipeConfig m_config{};
  const double m_dx{};
  const double m_area{};
  const double m_R{};
  const double m_B{};
  const int m_num_nodes{};

  std::vector<double> m_H{};
  std::vector<double> m_Q{};
  std::vector<double> m_Z{};
};

int main() {
  double HR = 150.0;   // Reservoir head above datum [m]
  double Tmax = 20.3;  // Duration of transient [s]
  double CdA0 = 0.009; // Valve coefficient/opening parameter

  PipeConfig pipe_config = {
      .length = 600.0,
      .diameter = 0.5,
      .f = f,
      .num_reaches = 10,
      .z0 = 10,
      .z1 = 15,
  };

  Pipe pipe(pipe_config, 0, 0);

  int IPR = 1; // Output interval
  std::ofstream output_file("output.csv");

  if (!output_file) {
    std::cerr << "Error opening output file." << std::endl;
    return 1;
  }

  // ------------------------------------------------------------
  // Grid setup
  // ------------------------------------------------------------
  const double dx = pipe_config.length / pipe_config.num_reaches;

  // Characteristic travel time over one internal grid spacing.
  const double dt = dx / a;

  // One complete staggered update consists of two dt steps.
  const double system_dt = 2.0 * dt;

  std::cout << "dx        = " << dx << " m\n";
  std::cout << "dt MOC    = " << dt << " s\n";
  std::cout << "system dt = " << system_dt << " s\n";

  output_file << "t,tau,H_valve,Q_valve\n";

  // ------------------------------------------------------------
  // Find initial steady-state flow
  // ------------------------------------------------------------

  // ------------------------------------------------------------
  // State arrays
  // ------------------------------------------------------------

  const double Q0 = std::sqrt(
      2.0 * g * CdA0 * CdA0 * HR /
      (pipe.get_R() * pipe.config().num_reaches * 2.0 * g * CdA0 * CdA0 + 1.0));

  const double H0 = HR - pipe.get_R() * pipe.config().num_reaches * Q0 * Q0;

  Reservoir reservoir = std::make_shared<Reservoir>(HR);

  // const double Qi = std::sqrt(
  //     HR * Q0 * Q0 * tau_i * tau_i /
  //     (pipe.get_R() * pipe.config().num_reaches * Q0 * Q0 * tau_i * tau_i +
  //      H0));

  const double CVP = 0.5 * Q0 * Q0 / H0;

  ValveConfig valve_config{
      .tau_i = tau_i,
      .tc = tc,
      .em = em,
      .tau_f = tau_f,
  };
  Valve valve = std::make_shared<Valve>(valve_config);
  pipe.connect_left(reservoir);
  pipe.connect_right(valve);
  // ------------------------------------------------------------
  // Transient loop
  // ------------------------------------------------------------

  const int Kmax = static_cast<int>(0.5 * Tmax / dt) + 1;

  for (int k = 1; k < Kmax; ++k) {
    // FORTRAN:
    //
    // T = 2.*DT*K
    //
    // One complete iteration advances two staggered half-steps.
    const double t = 2.0 * dt * k;
    // Pipe
    pipe.iterate();

    // ========================================================
    // Upstream boundary:
    // constant-head reservoir
    // ========================================================
    // reservoir.iterate(ReservoirInput{HR, 0.0});
    // ========================================================
    // Downstream boundary:
    // closing valve
    // ========================================================
    // valve->iterate();

    // output_file << t << "," << tau << "," << H[N] << "," << Q[N] << '\n';
  }
  output_file.close();

  return 0;
}
