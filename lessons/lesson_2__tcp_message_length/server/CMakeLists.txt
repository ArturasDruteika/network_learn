cmake_minimum_required(VERSION 3.24)

project(server__lesson_2)

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
        DESTINATION "lesson_2__tcp_message_length/bin"

    COMPONENT runtime
)
