cmake_minimum_required(VERSION 3.24)

project(network_learn)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Intermediate build output
#
# This is NOT the final installation layout.
#
# Linux example:
#
#   build/linux_clang_release/
#       bin/
#       lib/
#
# The lesson-specific final layout is created by install() rules.
# ---------------------------------------------------------------------------

add_library(asio INTERFACE)

target_include_directories(asio
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/asio/asio/include
)

target_compile_definitions(asio
    INTERFACE
        ASIO_STANDALONE
)

if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")

    set(
        CMAKE_RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/bin"
    )

    set(
        CMAKE_LIBRARY_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/lib"
    )

    set(
        CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/lib"
    )

endif()

add_subdirectory(external)
add_subdirectory(lessons)