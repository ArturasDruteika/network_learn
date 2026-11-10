add_library(tcp_common__lesson_3 STATIC
    # Header files
    include/common/socket.hpp
    include/common/message_protocol.hpp
    include/common/tcp_connection.hpp

    # Source files
    src/socket.cpp
    src/message_protocol.cpp
    src/tcp_connection.cpp
)

target_include_directories(tcp_common__lesson_3
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)