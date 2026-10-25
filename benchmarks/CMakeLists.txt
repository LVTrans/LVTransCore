find_package(benchmark REQUIRED)

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
