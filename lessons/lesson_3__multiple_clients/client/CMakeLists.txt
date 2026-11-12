add_executable(client__lesson_3
               src/main.cpp
               src/chat_client.cpp)

target_include_directories(client__lesson_3 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(client__lesson_3 
    PRIVATE 
        common__lesson_3
)
