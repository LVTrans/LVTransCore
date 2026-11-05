find_package(benchmark CONFIG QUIET)

if(NOT benchmark_FOUND)
    include(FetchContent)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Build Google Benchmark tests" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Install Google Benchmark" FORCE)
    FetchContent_Declare(
        google_benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.tar.gz
    )
    FetchContent_MakeAvailable(google_benchmark)
endif()

set(BENCHMARK_SOURCES
    bench_pipe.cpp
    bench_plant.cpp
)


add_executable(benchmarks ${BENCHMARK_SOURCES})
add_executable(bench_plantA bench_plantA.cpp)



target_link_libraries(benchmarks
    PRIVATE
        lvtrans_core
        benchmark::benchmark
)

target_link_libraries(bench_plantA
    PRIVATE
        lvtrans_core
)

target_compile_options(benchmarks PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(benchmarks PRIVATE
    LVTRANS_BENCHMARK_DATA_DIR="${PROJECT_SOURCE_DIR}/tests/test_data"
)
