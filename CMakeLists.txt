# Only for cmake --version >= 3.5.1
cmake_minimum_required(VERSION 3.5.1)

# Project name
project(invaders)

# Creates the variable EXEC and sets it to chip8
set(EXEC invaders)

# Set the C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Set vcpkg environment
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CRT_LINKAGE static)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find all vcpkg packages
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Puts all .cpp files inside src
file(GLOB SOURCES_INVADERS src/*.cpp)

set(SOURCES
  ${SOURCES_INVADERS}
)

# Compiles the files defined by SOURCES to generante the executable defined by EXEC
add_executable(${EXEC} ${SOURCES})

# Link with libs
target_link_libraries(${EXEC}
  PRIVATE
  SDL2::SDL2main
  SDL2::SDL2
  SDL2::SDL2-static
)
target_link_libraries(${EXEC}
  PRIVATE
  imgui::imgui
)
target_link_libraries(${EXEC} PRIVATE OpenGL::GL)
