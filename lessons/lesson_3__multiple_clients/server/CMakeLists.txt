add_executable(server__lesson_3
               src/main.cpp
               src/chat_server.cpp
               src/chat_room.cpp
               src/session.cpp)

target_include_directories(server__lesson_3 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(server__lesson_3
    PRIVATE 
        common__lesson_3 
)
