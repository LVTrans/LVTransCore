#include <cmath>
#include <fstream>
#include <iostream>
#include <numbers>
#include <vector>


int main() {

  // ============================================================
  // Pipe data
  // ============================================================

  const double a = 1200.0; // Wave propagation velocity [m/s]
  const double L = 400.0;  // Pipe length [m]
  const double D = 0.25;   // Pipe inside diameter [m]
  const double f = 0.018;  // Darcy-Weisbach friction factor
  const double g = 9.806;  // Gravity [m/s^2]

  // ============================================================
  // Pump / check-valve data from Example 3-4 formulation
  // ============================================================

  // Rated-speed pump characteristic:
  //
  // H = Hs + a1*Q + a2*Q^2
  //
  // During startup:
  //
  // H = alpha^2*Hs + alpha*a1*Q + a2*Q^2

  const double Hs = 70.0;    // Rated-speed shutoff head [m]
  const double a1 = 0.0;     // Linear pump curve coefficient
  const double a2 = -2000.0; // Quadratic pump curve coefficient

  const double Hc = 50.0; // Static head downstream of check valve [m]

  // Pump reaches rated speed after ts seconds.
  const double ts = 2.0;

  // ============================================================
  // Initial pipe state
  // ============================================================

  const double Q0 = 0.0;
  const double H0 = Hc;

  // ============================================================
  // Downstream valve data
  //
  // This is retained from your previous test program.
  // It is NOT specified by Example 3-4 itself.
  // ============================================================

  const double tc = 4.0;
  const double tau_before = 1.0;
  const double tau_after = 0.5;

  const double Tmax = 8.0;

  // ============================================================
  // Grid setup
  // ============================================================

  int N = 10;

  if (N % 2 != 0)
    ++N;

  const int nodes = N + 1;

  const double area = std::numbers::pi * D * D / 4.0;
  const double dx = L / static_cast<double>(N);

  const double R = f * dx / (2.0 * g * D * area * area);

  const double B = a / (g * area);

  // Characteristic travel time over one reach
  const double dt = dx / a;

  // Two staggered characteristic steps per complete iteration
  const double system_dt = 2.0 * dt;

  std::cout << "dx        = " << dx << " m\n";
  std::cout << "dt MOC    = " << dt << " s\n";
  std::cout << "system dt = " << system_dt << " s\n";
  std::cout << "B         = " << B << '\n';
  std::cout << "R         = " << R << '\n';

  // ============================================================
  // State arrays
  // ============================================================

  std::vector<double> H(nodes, 0.0);
  std::vector<double> Q(nodes, 0.0);

  // Pump starts from rest.
  //
  // Initially:
  // Q = 0
  // H = Hc
  //
  // at the stationary/even nodes of the staggered grid.

  for (int i = 0; i <= N; i += 2) {
    H[i] = H0;
    Q[i] = Q0;
  }

  // ============================================================
  // Output
  // ============================================================

  std::ofstream output_file("output.csv");

  if (!output_file) {
    std::cerr << "Error opening output.csv\n";
    return 1;
  }

  output_file << "t,alpha,H_pump,Q_pump,H_valve,Q_valve\n";

  output_file << 0.0 << "," << 0.0 << "," << H[0] << "," << Q[0] << "," << H[N]
              << "," << Q[N] << '\n';

  const int Kmax = static_cast<int>(Tmax / system_dt) + 1;

  // ============================================================
  // Transient loop
  // ============================================================

  for (int k = 1; k < Kmax; ++k) {

    const double t = system_dt * static_cast<double>(k);

    // ==========================================================
    // Phase 1:
    // odd internal nodes
    // ==========================================================

    for (int i = 1; i < N; i += 2) {

      const double Cp = H[i - 1] + B * Q[i - 1];

      const double Cm = H[i + 1] - B * Q[i + 1];

      const double Bp = B + R * std::abs(Q[i - 1]);

      const double Bm = B + R * std::abs(Q[i + 1]);

      H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);

      Q[i] = (Cp - Cm) / (Bp + Bm);
    }

    // ==========================================================
    // Phase 2:
    // even internal nodes
    // ==========================================================

    for (int i = 2; i < N; i += 2) {

      const double Cp = H[i - 1] + B * Q[i - 1];

      const double Cm = H[i + 1] - B * Q[i + 1];

      const double Bp = B + R * std::abs(Q[i - 1]);

      const double Bm = B + R * std::abs(Q[i + 1]);

      H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);

      Q[i] = (Cp - Cm) / (Bp + Bm);
    }

    // ==========================================================
    // UPSTREAM BOUNDARY
    // Pump + check valve
    //
    // This is Example 3-4.
    // ==========================================================

    // Pump speed ratio:
    //
    // alpha = t/ts    if t <= ts
    // alpha = 1       if t > ts

    double alpha;

    if (t <= ts)
      alpha = t / ts;
    else
      alpha = 1.0;

    // Incoming C- characteristic from the pipe:
    //
    // CM = H2 - B*Q2
    // BM = B + R|Q2|

    const double Cm = H[1] - B * Q[1];

    const double Bm = B + R * std::abs(Q[1]);

    // ----------------------------------------------------------
    // Check-valve condition
    //
    // If:
    //
    // alpha^2 * Hs <= Hc
    //
    // pump head is not yet sufficient to open the check valve.
    // ----------------------------------------------------------

    if (alpha * alpha * Hs <= Hc) {

      Q[0] = 0.0;

      // From:
      //
      // H1 = CM + BM*Q1
      //
      // and Q1 = 0:
      H[0] = Cm;

    } else {

      // --------------------------------------------------------
      // Check valve open
      //
      // Pump equation:
      //
      // H1 =
      // alpha^2 Hs
      // + alpha a1 Q1
      // + a2 Q1^2
      //
      // Pipe characteristic:
      //
      // H1 = CM + BM Q1
      //
      // Combine and solve quadratic for Q1.
      // --------------------------------------------------------

      const double numerator = Bm - a1 * alpha;

      const double root_argument =
          1.0 -
          (4.0 * a2 * (alpha * alpha * Hs - Cm)) / (numerator * numerator);

      if (root_argument < 0.0) {
        std::cerr << "Negative root argument at pump boundary "
                  << "at t = " << t << '\n';
        return 1;
      }

      Q[0] = numerator / (2.0 * a2) * (1.0 - std::sqrt(root_argument));

      H[0] = Cm + Bm * Q[0];
    }

    // ==========================================================
    // DOWNSTREAM BOUNDARY
    //
    // Same valve model as your earlier main.cpp.
    // Not part of Example 3-4.
    // ==========================================================

    double tau;

    if (t <= tc) {

      tau = tau_before;

      // Keeping initial downstream condition
      Q[N] = Q0;
      H[N] = H0;

    } else {

      tau = tau_after;

      // This becomes problematic if Q0 = 0,
      // since the earlier valve example derives Cv from
      // the initial nonzero operating point.
      //
      // Therefore this block is only here structurally.
      //
      // For a genuine Example 3-4 simulation, you need the
      // actual downstream boundary specified for the system.

      const double Cp = H[N - 1] + B * Q[N - 1];

      const double Bp = B + R * std::abs(Q[N - 1]);

      // Temporary simple constant-head downstream boundary:
      H[N] = Hc;

      Q[N] = (Cp - H[N]) / Bp;
    }

    // ==========================================================
    // Output
    // ==========================================================

    output_file << t << "," << alpha << "," << H[0] << "," << Q[0] << ","
                << H[N] << "," << Q[N] << '\n';
  }

  output_file.close();

  std::cout << "Simulation completed.\n";

  return 0;
}
