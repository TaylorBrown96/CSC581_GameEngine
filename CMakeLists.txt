cmake_minimum_required(VERSION 3.16)
project(GameEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Option to choose between vendored SDL3 and system SDL3
option(USE_VENDORED_SDL3 "Use vendored SDL3 library" ON)

# macOS Homebrew support
if(APPLE)
    # Add Homebrew paths for both Apple Silicon and Intel Macs
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")    # Apple Silicon
    list(APPEND CMAKE_PREFIX_PATH "/usr/local")       # Intel Macs
    
    # Set minimum macOS version for compatibility
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
endif()

# SDL3 Setup
if(USE_VENDORED_SDL3)
    # Check if vendored SDL directory exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl/CMakeLists.txt")
        message(STATUS "Using vendored SDL3 from third_party/sdl")
        add_subdirectory(third_party/sdl EXCLUDE_FROM_ALL)
    else()
        message(WARNING "Vendored SDL3 not found at third_party/sdl, falling back to system SDL3")
        set(USE_VENDORED_SDL3 OFF)
    endif()
endif()

if(NOT USE_VENDORED_SDL3)
    # Use system-installed SDL3 (via Homebrew or other package manager)
    message(STATUS "Looking for system SDL3...")
    find_package(SDL3 REQUIRED CONFIG)
    
    if(SDL3_FOUND)
        message(STATUS "Found system SDL3 version: ${SDL3_VERSION}")
    else()
        message(FATAL_ERROR "SDL3 not found! Please install via Homebrew: brew install sdl3")
    endif()
endif()

# Define required source files

set(REQUIRED_SOURCES
    src/main.cpp
    src/GameEngine.cpp
    src/Input.cpp
    src/Render.cpp
    src/Physics.cpp
    src/Collisions.cpp
    src/vec2.cpp
)

# Start with required sources
set(ENGINE_SOURCES ${REQUIRED_SOURCES})

# Add optional sources if they exist
foreach(SOURCE_FILE ${OPTIONAL_SOURCES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
        list(APPEND ENGINE_SOURCES ${SOURCE_FILE})
        message(STATUS "Added source file: ${SOURCE_FILE}")
    else()
        message(WARNING "Source file not found: ${SOURCE_FILE}")
    endif()
endforeach()

# Define header files (for IDE organization)
set(ENGINE_HEADERS
    src/GameEngine.h
    src/Input.h
    src/Render.h
    src/Physics.h
    src/Collisions.h
    src/Entity.h
)

# Create your game executable target
add_executable(GameEngine ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Set include directories
target_include_directories(GameEngine PRIVATE src)

# Link to the actual SDL3 library
target_link_libraries(GameEngine PRIVATE SDL3::SDL3)

# macOS specific settings
if(APPLE)
    # Enable bundle creation for macOS apps (optional)
    # set_target_properties(GameEngine PROPERTIES MACOSX_BUNDLE TRUE)
    
    # Set proper install name for dynamic libraries on macOS
    set_target_properties(GameEngine PROPERTIES
        INSTALL_RPATH "@executable_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# Copy media folder to build directory (optional - for easier asset loading)
if(EXISTS "${CMAKE_SOURCE_DIR}/media")
    add_custom_command(TARGET GameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/media $<TARGET_FILE_DIR:GameEngine>/media
        COMMENT "Copying media folder to build directory"
    )
else()
    message(STATUS "No media folder found, skipping media copy")
endif()

# Set project properties for better organization in IDEs
set_target_properties(GameEngine PROPERTIES
    FOLDER "GameEngine"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
)

# Print build information for debugging
message(STATUS "=== GameEngine Build Configuration ===")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "USE_VENDORED_SDL3: ${USE_VENDORED_SDL3}")
if(APPLE)
    message(STATUS "CMAKE_OSX_DEPLOYMENT_TARGET: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    message(STATUS "Homebrew paths added to CMAKE_PREFIX_PATH")
endif()
message(STATUS "=======================================")

# Optional: Add compile flags for better debugging and warnings
target_compile_options(GameEngine PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O3>
)
