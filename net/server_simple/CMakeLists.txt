cmake_minimum_required(VERSION 3.16)
project(SimpleAuthoritativeServer LANGUAGES CXX)

# Use the same standard as the main project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Server executable
add_executable(simple_server
    src/main.cpp
)

# Link Windows Sockets on Win32
if (WIN32)
  target_link_libraries(simple_server PRIVATE ws2_32)
endif()

# Put the server exe under build/server_simple_build/<config>/simple_server.exe
# (overrides the global RUNTIME_OUTPUT_DIRECTORY set by the root CMake)
set_target_properties(simple_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/server_simple_build/$<CONFIGURATION>"
)
