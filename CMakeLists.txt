cmake_minimum_required(VERSION 3.20)
project(practica0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(practica0
    src/Main.cpp
)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(practica0 PRIVATE glfw GLEW::GLEW)

if(APPLE)
    target_link_libraries(practica0 PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()
