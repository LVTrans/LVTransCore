# Simulation dashboard example

From the repository root:

```sh
make gui
```

Plots read data through `Element::read_view()`. Scalars retain the latest 200
time samples. Vectors show the latest profile using `x_values` when provided,
or node indices otherwise. Nonempty `x_values` must match `values` in length;
the GUI reports mismatched profiles instead of plotting them.

The graph interval defaults to **1 simulated second**, while the solver still
steps every **0.1 s**. Use **Graph interval** to select 0.1, 0.5, 1, 2, 5, or
10 seconds. Select **0.1 s (every step)** to inspect all solver values. Sampling
selects individual states; it does not average or smooth the simulation.
Fast execution and normal execution use the same completed-step schedule.

Initial and reset states are sampled immediately. Changing the graph interval
clears the plots, samples the current state, and starts the new interval from
that step. Both scalar histories and vector profiles update at the selected
interval; the element state panel continues to show the current state.

Pipe head and flow profiles export the even-indexed solver nodes (`0, 2, 4,
... N`), matching the original LVTrans read H/Q interface. Odd-indexed nodes
are intermediate half-step values and are excluded from the displayed profile.
Coordinates are `0, 2*dx, 4*dx, ... L`, with the horizontal axis labeled
`Distance (m)`. A three-node solver grid therefore produces two plotted points,
one at each end. Raw `get_H()`/`get_Q()` and `get_state()` retain the full arrays.
Flow is volumetric Q in m³/s; LabVIEW's optional velocity display instead uses
Q divided by the pipe area, in m/s.

Vertical limits follow the finite plotted minimum and maximum with a fixed
10-unit margin on each side. This keeps tiny near-equilibrium variations from
filling the plot. The margin uses each plot's units (metres for head, m³/s for
flow); horizontal axes still fit automatically. Vertical limits are set on
every frame, overriding manual vertical zoom.

CMake downloads pinned ImGui and ImPlot sources using FetchContent when
`LVTRANS_BUILD_GUI=ON`. The first configuration needs internet access; sources
are cached under the build directory's `_deps` folder. GLFW 3.3+ and OpenGL
development packages must be installed separately.

The GUI dependencies are only linked to the GUI example, not `lvtrans_core`.
Builds with `LVTRANS_BUILD_GUI=OFF` do not fetch or build ImGui or ImPlot.

The revisions and archive SHA-256 hashes are recorded in `CMakeLists.txt`.
To use local checkouts without downloading, set
`FETCHCONTENT_SOURCE_DIR_IMGUI` and `FETCHCONTENT_SOURCE_DIR_IMPLOT` to the
corresponding upstream repository directories.
