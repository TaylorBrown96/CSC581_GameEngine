cmake_minimum_required(VERSION 3.21)
project(CSC581_GameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Place binaries in build/bin for both single & multi-config generators
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

option(USE_VENDORED_SDL "Build using third_party/SDL" ON)

set(SDL3_ADDED OFF)
if(USE_VENDORED_SDL AND EXISTS "${CMAKE_SOURCE_DIR}/third_party/SDL/CMakeLists.txt")
  message(STATUS "Using vendored SDL at third_party/SDL")
  add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)
  set(SDL3_ADDED ON)
endif()

if(NOT SDL3_ADDED)
  find_package(SDL3 REQUIRED CONFIG)
endif()

add_subdirectory(engine)
