# Only for cmake --version >= 3.5.1
cmake_minimum_required(VERSION 3.5.1)

# Project name
project(invaders)

# Creates the variable EXEC and sets it to chip8
set(EXEC invaders)

# Set the C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Set the third_party module directory
SET(THIRD_PARTY_DIR third_party)

# GLFW (https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_source)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${THIRD_PARTY_DIR}/glfw)

# -I includes
include_directories(${THIRD_PARTY_DIR}/glfw/include)
include_directories(${THIRD_PARTY_DIR}/imgui)
include_directories(${THIRD_PARTY_DIR}/imgui/backends)
include_directories(${THIRD_PARTY_DIR}/imgui_club)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Puts all .cpp files inside src
file(GLOB_RECURSE SOURCES_INVADERS src/*.cpp)

# Put imgui .cpp files to sources
file(GLOB SOURCES_IMGUI ${THIRD_PARTY_DIR}/imgui/*.cpp)

set(SOURCES
  ${SOURCES_INVADERS}
  ${SOURCES_IMGUI}
  # Add the GLFW and OpenGL backend
  ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
  ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_glfw.cpp
)

# Compiles the files defined by SOURCES to generante the executable defined by EXEC
add_executable(${EXEC} ${SOURCES})

# Link with GLFW and OpenGL
target_link_libraries(${EXEC} glfw)
target_link_libraries(${EXEC} OpenGL::GL)
