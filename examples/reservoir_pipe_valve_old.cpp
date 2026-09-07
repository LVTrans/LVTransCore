#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

int main() {
  // ------------------------------------------------------------
  // Input data from SINGLE.DAT
  // ------------------------------------------------------------

  double a = 1200.0;   // Wave propagation velocity [m/s]
  double L = 600.0;    // Pipeline length [m]
  double D = 0.5;      // Inside diameter [m]
  double f = 0.018;    // Darcy-Weisbach friction factor
  double g = 9.806;    // Gravitational acceleration [m/s^2]
  double HR = 150.0;   // Reservoir head above datum [m]
  double Tmax = 4.3;   // Duration of transient [s]
  double CdA0 = 0.009; // Valve coefficient/opening parameter

  double tau_i = 1.0; // Initial valve position
  double tau_f = 0.0; // Final valve position
  double tc = 2.1;    // Valve operating/closure time [s]
  double em = 0.75;   // Exponent defining valve motion

  int N = 10;     // Number of pipe reaches; must be even
  int IPR = 1;    // Output interval
  int IGRAF = 11; // Original FORTRAN graph location

  // ------------------------------------------------------------
  // Grid setup
  // ------------------------------------------------------------

  if (N % 2 != 0)
    ++N;

  const int nodes = N + 1;

  const double area = M_PI * D * D / 4.0;
  const double dx = L / N;

  // Same R as the FORTRAN expression:
  //
  // R = F*XL / (2*G*D**5*.7854**2*N)
  //
  // because area = 0.7854 * D^2
  const double R = f * dx / (2.0 * g * D * area * area);

  // FORTRAN:
  // B = A / (G * .7854 * D * D)
  //
  // Here old FORTRAN A == wave speed a.
  const double B = a / (g * area);

  // Characteristic travel time over one internal grid spacing.
  const double dt = dx / a;

  // One complete staggered update consists of two dt steps.
  const double system_dt = 2.0 * dt;

  std::cout << "dx        = " << dx << " m\n";
  std::cout << "dt MOC    = " << dt << " s\n";
  std::cout << "system dt = " << system_dt << " s\n";
  std::cout << "B         = " << B << '\n';
  std::cout << "R         = " << R << '\n';

  std::ofstream output_file("output_old.csv");

  if (!output_file) {
    std::cerr << "Error opening output file." << std::endl;
    return 1;
  }

  output_file << "t,tau,H_valve,Q_valve\n";
  // ------------------------------------------------------------
  // Find initial steady-state flow
  // ------------------------------------------------------------

  const double Q0 = std::sqrt(2.0 * g * CdA0 * CdA0 * HR /
                              (R * N * 2.0 * g * CdA0 * CdA0 + 1.0));

  const double H0 = HR - R * N * Q0 * Q0;

  const double Qi = std::sqrt(HR * Q0 * Q0 * tau_i * tau_i /
                              (R * N * Q0 * Q0 * tau_i * tau_i + H0));

  // ------------------------------------------------------------
  // State arrays
  // ------------------------------------------------------------

  std::vector<double> H(nodes, 0.0);
  std::vector<double> Q(nodes, 0.0);

  // In the FORTRAN staggered grid, initial values are stored
  // at every second node.
  //
  // FORTRAN: I = 1,3,5,...,NS
  // C++:     i = 0,2,4,...,N
  for (int i = 0; i <= N; i += 2) {
    H[i] = HR - i * R * Qi * Qi;
    Q[i] = Qi;
  }

  // for (int i = 0; i < N; i++) {
  //   std::cout << "H[" << i << "] = " << H[i] << " m\n";
  //   std::cout << "Q[" << i << "] = " << Q[i] << " m^3/s\n";
  // }

  const double CVP = 0.5 * Q0 * Q0 / H0;

  double tau = tau_i;

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

    // ========================================================
    // Phase 1:
    // compute intermediate staggered nodes
    // ========================================================

    for (int i = 1; i < N; i += 2) {

      const double Cp = H[i - 1] + B * Q[i - 1];

      const double Cm = H[i + 1] - B * Q[i + 1];

      const double Bp = B + R * std::abs(Q[i - 1]);

      const double Bm = B + R * std::abs(Q[i + 1]);

      H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);

      Q[i] = (H[i] - Cm) / Bm;
    }

    // ========================================================
    // Phase 2:
    // compute the original/even staggered nodes
    // ========================================================

    for (int i = 2; i < N; i += 2) {

      const double Cp = H[i - 1] + B * Q[i - 1];

      const double Cm = H[i + 1] - B * Q[i + 1];

      const double Bp = B + R * std::abs(Q[i - 1]);

      const double Bm = B + R * std::abs(Q[i + 1]);

      H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);

      Q[i] = (H[i] - Cm) / Bm;
    }

    // ========================================================
    // Upstream boundary:
    // constant-head reservoir
    // ========================================================
    const double Cm = H[1] - B * Q[1];
    const double Bm = B + R * std::abs(Q[1]);

    H[0] = HR;

    Q[0] = (H[0] - Cm) / Bm;

    // ========================================================
    // Downstream boundary:
    // closing valve
    // ========================================================

    const double CV = tau * tau * CVP;

    // C+ characteristic arriving at valve
    const double Cp = H[N - 1] + B * Q[N - 1];

    const double Bp = B + R * std::abs(Q[N - 1]);

    // // Solve characteristic equation +
    // // nonlinear valve equation simultaneously.
    Q[N] = -CV * Bp + std::sqrt(CV * CV * Bp * Bp + 2.0 * CV * Cp);

    H[N] = Cp - Bp * Q[N];

    // --------------------------------------------------------
    // Example output
    // --------------------------------------------------------

    if (t < tc) {
      tau = tau_i - (tau_i - tau_f) * std::pow(t / tc, em);
    } else {
      tau = tau_f;
    }
    std::cout << t << "," << tau << "," << H[N] << "," << Q[N] << '\n';
    output_file << t << "," << tau << "," << H[N] << "," << Q[N] << '\n';
  }

  output_file.close();

  return 0;
}
