# Only for cmake --version >= 3.5.1
cmake_minimum_required(VERSION 3.5.1)

# Project name
project(invaders)

# Creates the variable EXEC and sets it to chip8
set(EXEC invaders)

# set the C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Puts all .cpp files inside src
file(GLOB SOURCES_INVADERS src/*.cpp)

set(SOURCES
  ${SOURCES_INVADERS}
)

# Compiles the files defined by SOURCES to generante the executable defined by EXEC
add_executable(${EXEC} ${SOURCES})
