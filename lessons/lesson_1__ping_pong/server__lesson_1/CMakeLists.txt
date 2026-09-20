cmake_minimum_required(VERSION 3.24)

project(server__lesson_1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------

add_executable(
    ${PROJECT_NAME}
    main.cpp
)

# ---------------------------------------------------------------------------
# Libraries
# ---------------------------------------------------------------------------

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        spdlog::spdlog
)

# ---------------------------------------------------------------------------
# Runtime RPATH
# ---------------------------------------------------------------------------

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------

install(
    TARGETS ${PROJECT_NAME}

    RUNTIME
        DESTINATION "lesson_1__ping_pong/bin"

    COMPONENT runtime
)