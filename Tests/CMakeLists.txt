cmake_minimum_required(VERSION 3.14)
project(test_AECS)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
)

FetchContent_MakeAvailable(googletest benchmark)

enable_testing()

add_executable(tests
        test_Chunk.cpp
        test_ChunkFactory.cpp
        test_ArchetypeFactory.cpp
        test_Archetype.cpp
        test_ArchetypeStore.cpp
)

target_link_libraries(tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)

add_executable(benchmarks
        benchmark_ECS.cpp
)

target_link_libraries(benchmarks PRIVATE benchmark::benchmark benchmark::benchmark_main)