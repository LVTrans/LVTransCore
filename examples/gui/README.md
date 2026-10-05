# Simulation dashboard example

From the repository root:

```sh
make gui
```

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
