add_library(common__lesson_3 
    STATIC 
        src/protocol.cpp
)

target_include_directories(common__lesson_3 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
