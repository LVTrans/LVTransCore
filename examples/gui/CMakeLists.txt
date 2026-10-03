find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

add_library(lvtrans_gui_dependencies STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    implot.cpp
    implot_items.cpp
)
target_include_directories(lvtrans_gui_dependencies PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lvtrans_gui_dependencies PUBLIC glfw OpenGL::GL ${CMAKE_DL_LIBS})
target_compile_features(lvtrans_gui_dependencies PUBLIC cxx_std_20)

add_executable(gui gui.cpp)
target_link_libraries(gui PRIVATE lvtrans_gui_dependencies lvtrans_core)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(gui PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()
