cmake_minimum_required(VERSION 3.15)
project(AECS VERSION 1.0)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -march=native -mtune=native -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(Engine/include)

add_executable(runner
        Runner/main.cpp
        Engine/include/ECS/Archetype/ComponentRegistry.hpp
        Engine/include/ECS/Component/SignatureBitset.hpp
        Engine/include/ECS/Archetype/ArrayPool.hpp
)

set_target_properties(runner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_subdirectory(Tests)