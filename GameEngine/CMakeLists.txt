cmake_minimum_required(VERSION 3.16)
project(GameEngine LANGUAGES CXX)

# Ensure DLLs go into the build root
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Include SDL as a subdirectory
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Your executable
add_executable(GameEngine main.cpp)

# Link against SDL3
target_link_libraries(GameEngine PRIVATE SDL3::SDL3)