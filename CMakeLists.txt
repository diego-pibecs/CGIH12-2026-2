cmake_minimum_required(VERSION 3.20)
project(CGIH12 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(APPLE)
  list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
endif()

find_package(OpenGL REQUIRED)

find_package(glfw3 CONFIG QUIET)
if(NOT glfw3_FOUND)
  find_package(glfw3 REQUIRED)
endif()

find_package(GLEW CONFIG QUIET)
if(NOT GLEW_FOUND)
  find_package(GLEW REQUIRED)
endif()

find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
  message(STATUS "glm not found in system packages, fetching with FetchContent")
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
  )
  FetchContent_MakeAvailable(glm)
endif()

if(TARGET glm::glm)
  set(CGHI12_GLM_TARGET glm::glm)
elseif(TARGET glm)
  set(CGHI12_GLM_TARGET glm)
else()
  message(FATAL_ERROR "Unable to configure GLM target")
endif()

set(CGHI12_COMMON_LIBS
  OpenGL::GL
  glfw
  GLEW::GLEW
)

add_executable(main_triangulo
  configCGHI12/Main.cpp
)
target_link_libraries(main_triangulo PRIVATE ${CGHI12_COMMON_LIBS})

add_executable(dibujo2d
  configCGHI12/Main_Dibujo2D.cpp
)
target_compile_definitions(dibujo2d PRIVATE SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/configCGHI12/Shader")
target_link_libraries(dibujo2d PRIVATE ${CGHI12_COMMON_LIBS})

add_executable(practica2
  configCGHI12/Main_P3.cpp
)
target_compile_definitions(practica2 PRIVATE SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/configCGHI12/Shader")
target_link_libraries(practica2 PRIVATE ${CGHI12_COMMON_LIBS} ${CGHI12_GLM_TARGET})
