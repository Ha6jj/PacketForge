# benchmarks/flatbuffers

find_package(FlatBuffers QUIET)
find_package(flatbuffers QUIET)

if(TARGET flatbuffers::flatbuffers)
    set(FB_TARGET flatbuffers::flatbuffers)
elseif(TARGET FlatBuffers::FlatBuffers)
    set(FB_TARGET FlatBuffers::FlatBuffers)
elseif(TARGET flatbuffers)
    set(FB_TARGET flatbuffers)
else()
    message(STATUS "Compatible FlatBuffers target not found. Fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        flatbuffers
        GIT_REPOSITORY https://github.com/google/flatbuffers.git
        GIT_TAG        v24.3.25
    )
    set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(FLATBUFFERS_INSTALL OFF CACHE BOOL "" FORCE)
    set(FLATBUFFERS_BUILD_FLATC ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(flatbuffers)
    set(FB_TARGET flatbuffers::flatbuffers)
endif()

set(FBS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/benchmark.fbs")
set(FB_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/fb_generated")
file(MAKE_DIRECTORY ${FB_GEN_DIR})

if(TARGET flatc)
    set(FLATC $<TARGET_FILE:flatc>)
else()
    find_program(FLATC flatc REQUIRED)
endif()

add_custom_command(
    OUTPUT "${FB_GEN_DIR}/benchmark_fb_generated.h"
    COMMAND ${FLATC} --cpp -o ${FB_GEN_DIR} ${FBS_FILE}
    DEPENDS ${FBS_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating FlatBuffers C++ code..."
    VERBATIM
)

add_custom_target(generate_fb_code DEPENDS "${FB_GEN_DIR}/benchmark_fb_generated.h")

if(TARGET flatc)
    add_dependencies(generate_fb_code flatc)
endif()

add_executable(fb_benchmark main.cpp)
add_dependencies(fb_benchmark generate_fb_code)

target_include_directories(fb_benchmark PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FB_GEN_DIR}
)

target_link_libraries(fb_benchmark PRIVATE
    BenchmarkUtils
    benchmark::benchmark
    flatbuffers
)

target_compile_options(fb_benchmark PRIVATE -O3 -march=native)

set_target_properties(fb_benchmark PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION TRUE
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)