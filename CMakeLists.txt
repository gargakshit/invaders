# Only for cmake --version >= 3.5.1
cmake_minimum_required(VERSION 3.5.1)

# Project name
project(invaders)

# Creates the variable EXEC and sets it to chip8
set(EXEC invaders)

# Set the C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Set vcpkg environment
# set(VCPKG_LIBRARY_LINKAGE static)
# set(VCPKG_CRT_LINKAGE static)

# Find all vcpkg packages
if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tmp/space-invaders.rom")
    message(
      FATAL_ERROR
      "tmp/space-invaders.rom not found. Please read the README on how to build with emscripten"
    )
  endif()

  message("-- Copying tmp/space-invaders.rom to ${CMAKE_CURRENT_BINARY_DIR}/")
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/tmp/space-invaders.rom"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

  set(USE_FLAGS "-s USE_SDL=2 -s USE_WEBGL2=1")

  if (NOT CMAKE_BUILD_TYPE MATCHES Release)
    set(USE_FLAGS "${USE_FLAGS} -g")
    message("-- Debug with Emscripten")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS} --shell-file ../emscripten_shell.html --embed-file space-invaders.rom"
  )
  set(CMAKE_EXECUTABLE_SUFFIX .html) 

  include_directories(${SDL2_INCLUDE_DIRS})
else()
  # Find OpenGL
  find_package(OpenGL REQUIRED)
  find_package(SDL2 CONFIG REQUIRED)
endif()
find_package(imgui CONFIG REQUIRED)

# Puts all .cpp files inside src
file(GLOB_RECURSE SOURCES_INVADERS src/*.cpp)

set(SOURCES
  ${SOURCES_INVADERS}
)

# Compiles the files defined by SOURCES to generante the executable defined by EXEC
add_executable(${EXEC} ${SOURCES})

# Link with libs
if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  message("-- Emscripten")
else()
  target_link_libraries(${EXEC}
    PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    SDL2::SDL2-static
  )
  target_link_libraries(${EXEC} PRIVATE OpenGL::GL)
endif()
target_link_libraries(${EXEC}
  PRIVATE
  imgui::imgui
)
