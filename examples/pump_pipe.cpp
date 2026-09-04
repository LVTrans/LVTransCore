#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

int main() {
  // ============================================================
  // Example 3-3: Centrifugal pump -> pipeline -> valve
  // ============================================================

  constexpr double pi = 3.14159265358979323846;

  // ------------------------------------------------------------
  // Pipe parameters
  // ------------------------------------------------------------

  const double a = 1200.0; // Wave speed [m/s]
  const double L = 400.0;  // Pipe length [m]
  const double D = 0.25;   // Inside diameter [m]
  const double f = 0.018;  // Darcy-Weisbach friction factor
  const double g = 9.806;  // Gravitational acceleration [m/s^2]

  // ------------------------------------------------------------
  // Initial operating point from Example 3-3
  // ------------------------------------------------------------

  const double Q0 = 0.1;       // Initial flow [m^3/s]
  const double H_pump0 = 50.0; // Initial pump head [m]
  const double Hs = 70.0;      // Pump shutoff head [m]

  // Pump characteristic:
  //
  // H = Hs + a2 * Q^2
  //
  // Using the known operating point H=50 m, Q=0.1 m^3/s:
  const double a2 = (H_pump0 - Hs) / (Q0 * Q0); // = -2000

  // ------------------------------------------------------------
  // Valve event
  // ------------------------------------------------------------

  const double tc = 2.1; // Time at which valve position changes [s]

  const double tau_before = 1.0;
  const double tau_after = 0.5;

  // Total simulated time.
  // This is not specified by the boundary equations themselves.
  const double Tmax = 10.3;

  // ------------------------------------------------------------
  // MOC grid
  // ------------------------------------------------------------

  int N = 100; // Number of pipe reaches; must be even

  if (N % 2 != 0) {
    ++N;
  }

  const int nodes = N + 1;

  const double area = pi * D * D / 4.0;
  const double dx = L / static_cast<double>(N);

  // MOC friction coefficient
  //
  // R = f*dx / (2*g*D*A^2)
  const double R = f * dx / (2.0 * g * D * area * area);

  // Characteristic impedance
  //
  // B = a / (g*A)
  const double B = a / (g * area);

  // Time required for a characteristic to travel one spatial reach:
  //
  // dx = a*dt
  const double dt = dx / a;
  // complete outer iteration.
  const double system_dt = 2.0 * dt;

  std::cout << "------------------------------\n";
  std::cout << "Grid / MOC parameters\n";
  std::cout << "------------------------------\n";
  std::cout << "Area      = " << area << " m^2\n";
  std::cout << "dx        = " << dx << " m\n";
  std::cout << "dt MOC    = " << dt << " s\n";
  std::cout << "system dt = " << system_dt << " s\n";
  std::cout << "B         = " << B << '\n';
  std::cout << "R         = " << R << '\n';
  std::cout << "a2        = " << a2 << '\n';
  std::cout << "DT: " << dt << '\n';

  // ------------------------------------------------------------
  // Initial steady-state head at downstream valve
  // ------------------------------------------------------------

  // Darcy-Weisbach:
  //
  // h_f = f * L/D * V^2/(2g)
  //
  // with V = Q/A:
  //
  // h_f = f * L/D * Q^2/(2*g*A^2)
  //
  // The pump initially supplies 50 m of head, so the valve head is
  // 50 m minus the steady-state pipe friction loss.

  const double friction_loss =
      (f * L / D) * (Q0 * Q0) / (2.0 * g * area * area);

  const double H0 = H_pump0 - friction_loss;

  std::cout << "\n------------------------------\n";
  std::cout << "Initial operating point\n";
  std::cout << "------------------------------\n";
  std::cout << "Q0             = " << Q0 << " m^3/s\n";
  std::cout << "Pump head      = " << H_pump0 << " m\n";
  std::cout << "Friction loss  = " << friction_loss << " m\n";
  std::cout << "Valve head H0  = " << H0 << " m\n";

  // ------------------------------------------------------------
  // State arrays
  // ------------------------------------------------------------

  std::vector<double> H(nodes, 0.0);
  std::vector<double> Q(nodes, 0.0);

  // Staggered-grid initialization.
  //
  // At t=0, the known stationary nodes are:
  //
  // 0, 2, 4, ..., N
  //
  // Flow is constant throughout the pipe:
  //
  // Q(x,0) = Q0
  //
  // while head falls linearly according to steady friction:
  //
  // H_i = H_pump0 - i*R*Q0^2

  for (int i = 0; i <= N; i += 2) {
    H[i] = H_pump0 - static_cast<double>(i) * R * Q0 * Q0;

    Q[i] = Q0;
  }

  // Sanity check:
  //
  // H[N] should equal H0.
  std::cout << "Grid valve H   = " << H[N] << " m\n";

  // ------------------------------------------------------------
  // Output file
  // ------------------------------------------------------------

  std::ofstream output_file("output.csv");

  if (!output_file) {
    std::cerr << "Error opening output.csv\n";
    return 1;
  }

  output_file << "t,tau,H_pump,Q_pump,H_valve,Q_valve\n";

  // Initial state at t = 0
  output_file << 0.0 << "," << tau_before << "," << H[0] << "," << Q[0] << ","
              << H[N] << "," << Q[N] << '\n';

  // ------------------------------------------------------------
  // Number of complete staggered time steps
  // ------------------------------------------------------------

  const int Kmax = static_cast<int>(Tmax / system_dt) + 1;

  double tau = tau_before;

  // ============================================================
  // Transient calculation
  // ============================================================

  for (int k = 1; k < Kmax; ++k) {

    const double t = system_dt * static_cast<double>(k);

    // ==========================================================
    // Phase 1
    //
    // Calculate odd nodes:
    //
    // 1, 3, 5, ..., N-1
    //
    // from the known neighboring even nodes.
    // ==========================================================

    for (int i = 1; i < N; i += 2) {

      // Positive characteristic arriving from the left
      const double Cp = H[i - 1] + B * Q[i - 1];

      // Negative characteristic arriving from the right
      const double Cm = H[i + 1] - B * Q[i + 1];

      // Friction-adjusted characteristic coefficients
      const double Bp = B + R * std::abs(Q[i - 1]);

      const double Bm = B + R * std::abs(Q[i + 1]);

      // Solve the two characteristic equations:
      //
      // H + Bp*Q = Cp
      // H - Bm*Q = Cm

      H[i] = (Cp * Bm + Cm * Bp) / (Bp + Bm);

      Q[i] = (Cp - Cm) / (Bp + Bm);
    }

    // ==========================================================
    // Phase 2
    //
    // Calculate interior even nodes:
    //
    // 2, 4, 6, ..., N-2
    //
    // using the newly calculated odd nodes.
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
    // UPSTREAM BOUNDARY: CENTRIFUGAL PUMP
    // ==========================================================
    //
    // Only C- arrives at the upstream boundary from inside
    // the pipe.
    //
    // From the first internal node:
    //
    // Cm = H_2 - B*Q_2
    //
    // Bm = B + R|Q_2|
    //
    // Therefore:
    //
    // H_1 = Cm + Bm*Q_1
    //
    // Pump characteristic:
    //
    // H_1 = Hs + a2*Q_1^2
    //
    // Equating these gives a quadratic in Q_1.

    {
      const double Cm = H[1] - B * Q[1];

      const double Bm = B + R * std::abs(Q[1]);

      const double discriminant = Bm * Bm + 4.0 * a2 * (Cm - Hs);

      if (discriminant < 0.0) {
        std::cerr << "Pump boundary produced a negative "
                     "discriminant at t = "
                  << t << " s\n";
        return 1;
      }

      Q[0] = (Bm - std::sqrt(discriminant)) / (2.0 * a2);

      H[0] = Hs + a2 * Q[0] * Q[0];
    }

    // ==========================================================
    // DOWNSTREAM BOUNDARY: VALVE
    // ==========================================================

    if (t <= tc) {
      // Before the sudden valve movement, the downstream
      // boundary remains at the initial steady operating point.

      tau = tau_before;

      Q[N] = Q0;
      H[N] = H0;

    } else {
      // Valve setting suddenly changes from tau = 1.0
      // to tau = 0.5.

      tau = tau_after;

      // Valve equation:
      //
      // Q^2 = 2*Cv*H
      //
      // Initial valve operating point gives
      //
      // Cv = (tau*Q0)^2 / (2*H0)

      const double Cv = (tau * Q0) * (tau * Q0) / (2.0 * H0);

      // C+ characteristic arriving at the valve
      const double Cp = H[N - 1] + B * Q[N - 1];

      const double Bp = B + R * std::abs(Q[N - 1]);

      // At downstream boundary:
      //
      // H = Cp - Bp*Q
      //
      // and
      //
      // Q^2 = 2*Cv*H
      //
      // Combining them gives:
      //
      // Q^2 + 2*Cv*Bp*Q - 2*Cv*Cp = 0

      const double discriminant = Cv * Cv * Bp * Bp + 2.0 * Cv * Cp;

      if (discriminant < 0.0) {
        std::cerr << "Valve boundary produced a negative "
                     "discriminant at t = "
                  << t << " s\n";
        return 1;
      }

      Q[N] = -Cv * Bp + std::sqrt(discriminant);

      H[N] = Cp - Bp * Q[N];
    }

    // ----------------------------------------------------------
    // Save results
    // ----------------------------------------------------------

    output_file << t << "," << tau << "," << H[0] << "," << Q[0] << "," << H[N]
                << "," << Q[N] << '\n';
  }

  output_file.close();

  std::cout << "\nSimulation completed.\n";
  std::cout << "Results written to output.csv\n";

  return 0;
}
