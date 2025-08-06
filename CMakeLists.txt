cmake_minimum_required(VERSION 3.22)
project(hparena)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Wextra -Wpedantic)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG af442b4da0cd7a07b56fa709bd16571889dc7fda # v3.0
)
message(STATUS "Downloading and configuring argparse")
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG eddb0241389718a23a42db6af5f0164b6e0139af # v1.9.4
)
message(STATUS "Downloading and configuring Benchmark")
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(benchmark)

add_library(hparena INTERFACE)
install(TARGETS hparena)
target_include_directories(hparena INTERFACE include)

# Empirical test executables.
add_executable(hugepages_exhausted src/empirical/hugepages_exhausted.cpp)
add_executable(mmap_non_multiple src/empirical/mmap_non_multiple.cpp)

# Benchmark executables.
add_executable(circular src/benchmarks/circular.cpp)
target_link_libraries(circular PRIVATE argparse::argparse benchmark::benchmark hparena)
