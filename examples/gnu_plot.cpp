#include <cstdio>
#include <iostream>
#include <vector>

struct Point {
  double x;
  double y;
};

void plot(FILE* gp, const std::vector<Point>& data) {
  fprintf(gp, "plot '-' with linespoints lw 2 pt 7 ps 1.5\n");
  for (const auto& point : data) {
    fprintf(gp, "%f %f\n", point.x, point.y);
  }
  fprintf(gp, "e\n");
}

int main() {
  std::vector<Point> data = {{.x=1.0, .y=2.3}, {.x=2.0, .y=3.8}, {.x=3.0, .y=5.1},
                             {.x=4.0, .y=4.5}, {.x=5.0, .y=6.2}, {.x=6.0, .y=7.8}};

  FILE* gp = popen("gnuplot -persist", "w");
  if (!gp) {
    std::cerr << "Error: Could not open pipe to Gnuplot. Is it installed?"
              << '\n';
    return 1;
  }

  fprintf(gp,
          "set terminal dumb\n");  // Prints ASCII chart in terminal
  fprintf(gp, "set title 'C++ Data Plot Example'\n");
  fprintf(gp, "set xlabel 'X Axis'\n");
  fprintf(gp, "set ylabel 'Y Axis'\n");
  fprintf(gp, "set grid\n");

  plot(gp, data);
  // plot(gp, data);

  fflush(gp);
  pclose(gp);

  return 0;
}
