cmake_minimum_required(VERSION 3.24)

project(lessons)

# ---------------------------------------------------------------------------
# Lessons
# ---------------------------------------------------------------------------

set(
    LESSONS
    lesson_1__ping_pong
    lesson_2__tcp_message_length
)

# ---------------------------------------------------------------------------
# Configure lessons
# ---------------------------------------------------------------------------

foreach(LESSON IN LISTS LESSONS)

    set(
        CMAKE_RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/bin/${LESSON_OUTPUT_NAME}/${LESSON}"
    )

    add_subdirectory("${LESSON}")

endforeach()