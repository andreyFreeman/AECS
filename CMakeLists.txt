cmake_minimum_required(VERSION 3.28)
project(AECS VERSION 1.0 LANGUAGES CXX)

set(ECS_MAX_COMPONENTS 128 CACHE STRING "Max components")
set(ECS_CHUNK_SIZE "(128 * 1024)" CACHE STRING "Chunk size in bytes")
set(ECS_MAX_CHUNK_COUNT 4096 CACHE STRING "Max chunks")
set(ECS_MAX_ENTITIES 10000000 CACHE STRING "Max entities")

add_library(AECS INTERFACE)

set_target_properties(AECS PROPERTIES INTERFACE_CXX_EXTENSIONS OFF)
target_compile_features(AECS INTERFACE cxx_std_23)
target_compile_options(AECS INTERFACE
        $<$<CONFIG:Release>:-O3 -march=native -mtune=native>
        $<$<CONFIG:Debug>:-g -O0>
)

target_include_directories(AECS INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(AECS INTERFACE
        MAX_COMPONENTS=${ECS_MAX_COMPONENTS}
        CHUNK_SIZE=${ECS_CHUNK_SIZE}
        MAX_CHUNK_COUNT=${ECS_MAX_CHUNK_COUNT}
        MAX_ENTITIES=${ECS_MAX_ENTITIES}
)

add_library(Kladov::AECS ALIAS AECS)

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_executable(runner
            Runner/main.cpp
    )
    target_compile_options(runner PRIVATE
            $<$<CONFIG:Release>:-O3 -march=native -mtune=native -DNDEBUG>
            $<$<CONFIG:Debug>:-g -O0>
    )
    set_target_properties(runner PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    target_link_libraries(runner PRIVATE AECS)
    add_subdirectory(Tests)
endif()