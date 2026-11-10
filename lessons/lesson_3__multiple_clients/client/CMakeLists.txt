add_executable(client__lesson_3
    # Header files
    include/client/tcp_client.hpp

    # Source files
    src/main.cpp
    src/tcp_client.cpp
)

target_include_directories(client__lesson_3
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(client__lesson_3
    PRIVATE
        tcp_common__lesson_3
        spdlog::spdlog
)