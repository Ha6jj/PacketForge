cmake_minimum_required(VERSION 3.10)

find_package(Protobuf REQUIRED)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.proto)
set(GENERATED_SOURCES
    ${GENERATED_DIR}/benchmark.pb.cc
    ${GENERATED_DIR}/benchmark.pb.h
)

add_custom_target(create_protobuf_dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
    COMMENT "Creating protobuf output directory: ${GENERATED_DIR}"
    VERBATIM
)

add_custom_command(
    OUTPUT ${GENERATED_SOURCES}
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        --cpp_out=${GENERATED_DIR}
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILE}
    DEPENDS 
        ${PROTO_FILE}
        create_protobuf_dir
    COMMENT "Generating Protocol Buffers code"
    VERBATIM
)

add_executable(protobuf_benchmark
    main.cpp
    ${GENERATED_SOURCES}
)

target_include_directories(protobuf_benchmark PRIVATE
    ${GENERATED_DIR}
)

target_link_libraries(protobuf_benchmark PRIVATE
    benchmark::benchmark
    ${PROTOBUF_LIBRARIES}
)

target_compile_options(protobuf_benchmark PRIVATE
    -O3
    -march=native
    -flto
)

add_dependencies(protobuf_benchmark create_protobuf_dir)
