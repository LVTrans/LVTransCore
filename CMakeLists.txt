cmake_minimum_required(VERSION 3.16)
project(
    LVTransCore
    VERSION 0.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(LVTRANS_BUILD_TESTS "Build LVTrans tests" ON)
option(LVTRANS_BUILD_EXAMPLES "Build LVTrans examples" ON)
option(LVTRANS_ENABLE_OPENMP "Enable OpenMP" ON)

add_library(lvtrans_core)

target_sources(
    lvtrans_core
    PRIVATE
        src/pipe.cpp
        src/valve.cpp
        src/reservoir.cpp
)


target_include_directories(
    lvtrans_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)


target_compile_features(
    lvtrans_core
    PUBLIC
        cxx_std_17
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(lvtrans_core PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Debug>:-O0 -g3>
    )
elseif(MSVC)
    target_compile_options(lvtrans_core PRIVATE
        /W4 /permissive- /GS
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    )
endif()

if(LVTRANS_ENABLE_OPENMP)
  find_package(OpenMP)

  if(OpenMP_CXX_FOUND)
    target_link_libraries(
            lvtrans_core
            PUBLIC
                OpenMP::OpenMP_CXX
        )
  endif()
endif()

if(LVTRANS_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

if(LVTRANS_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
