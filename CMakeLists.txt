include(FetchContent)
cmake_minimum_required(VERSION 3.16)
project(
    LVTransCore
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)


option(LVTRANS_BUILD_TESTS "Build LVTrans tests" ON)
option(LVTRANS_BUILD_EXAMPLES "Build LVTrans examples" OFF)
option(LVTRANS_BUILD_BENCHMARKS "Build Benchmarks" ON)

add_library(lvtrans_core)

target_sources(
    lvtrans_core
    PRIVATE
        src/pipe.cpp
        src/valve.cpp
        src/reservoir.cpp
        src/element.cpp
        src/plant.cpp
        src/element_container.cpp
        src/config/plant_config_repository.cpp
        src/config/element_parsers.cpp
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
    cxx_std_20
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    MESSAGE("Using GNU/Clang compiler")
    target_compile_options(lvtrans_core PRIVATE
        -Wall -Werror -Wextra -Wpedantic -Wconversion -Wcast-align
        -Wunused
        -Wshadow
        -Wold-style-cast
        -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Wno-missing-braces
    )
elseif(MSVC)
    MESSAGE("Using MSVC compiler")
    target_compile_options(lvtrans_core PRIVATE
        /W4 /permissive- /GS
    )
endif()

target_link_libraries(lvtrans_core PUBLIC nlohmann_json::nlohmann_json)

# if(LVTRANS_ENABLE_OPENMP)
#     find_package(OpenMP)
#
#     if(OpenMP_CXX_FOUND)
#         target_link_libraries(
#             lvtrans_core
#             PUBLIC
#                 OpenMP::OpenMP_CXX
#         )
#     endif()
# endif()

if(LVTRANS_BUILD_TESTS)
    MESSAGE("Building tests...")
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

if(LVTRANS_BUILD_EXAMPLES)
    MESSAGE("Building examples...")
    add_subdirectory(examples)
endif()

if(LVTRANS_BUILD_BENCHMARKS AND CMAKE_BUILD_TYPE STREQUAL "Release")
    MESSAGE("Building benchmarks...")
    add_subdirectory(benchmarks)
endif()
