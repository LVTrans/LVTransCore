# Simulation dashboard example

From the repository root:

```sh
make gui
```

Or configure, build, and launch explicitly:

```sh
cmake -S . -B build -DLVTRANS_BUILD_GUI=ON
cmake --build build --target gui
./build/examples/gui/gui
```

Requires GLFW 3.3+ development files, OpenGL development files, and a graphical
desktop session. ImGui and ImPlot sources are bundled here. GUI dependencies
are only required when `LVTRANS_BUILD_GUI` is enabled.

Click **Start** to animate the sine-wave demo, **Pause** to stop advancing it,
and **Reset** to clear the history. This example does not yet run an LVTrans plant.

The missing `imgui_impl_opengl3_loader.h` was obtained unchanged from Dear ImGui
revision `aa0181478b182f7170378a5d8629401b77ee326e`:
https://github.com/ocornut/imgui/blob/aa0181478b182f7170378a5d8629401b77ee326e/backends/imgui_impl_opengl3_loader.h
