add_executable(server__lesson_3
    # Header files
    include/server/tcp_server.hpp
    include/server/client_handler.hpp

    # Source files
    src/main.cpp
    src/tcp_server.cpp
    src/client_handler.cpp
)

target_include_directories(server__lesson_3
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(server__lesson_3
    PRIVATE
        tcp_common__lesson_3
        spdlog::spdlog
)