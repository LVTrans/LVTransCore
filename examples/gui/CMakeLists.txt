find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

include(FetchContent)

FetchContent_Declare(imgui
    URL https://codeload.github.com/ocornut/imgui/tar.gz/aa0181478b182f7170378a5d8629401b77ee326e
    URL_HASH SHA256=acab4d8e75a7aa9d54dc58502932532ee85d71550bd29c996f8e1a1b0b600219
)
FetchContent_Declare(implot
    URL https://codeload.github.com/epezent/implot/tar.gz/09e2ba71766e25d88053a2173936c9d1043bae42
    URL_HASH SHA256=17dd3b860237cb95c0d1c0c6accaf92ad4ea4be61add84eca85e3dc077963e12
)
FetchContent_MakeAvailable(imgui implot)

add_library(lvtrans_gui_dependencies STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)
target_include_directories(lvtrans_gui_dependencies SYSTEM PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${implot_SOURCE_DIR}
)
target_link_libraries(lvtrans_gui_dependencies PUBLIC glfw OpenGL::GL ${CMAKE_DL_LIBS})
target_compile_features(lvtrans_gui_dependencies PUBLIC cxx_std_20)

add_executable(gui gui.cpp)
target_link_libraries(gui PRIVATE lvtrans_gui_dependencies lvtrans_core)
target_compile_options(gui PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
