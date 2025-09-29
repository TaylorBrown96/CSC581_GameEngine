cmake_minimum_required(VERSION 3.16)
project(GameEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

if(APPLE)
    # Add Homebrew paths for both Apple Silicon and Intel Macs
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")    # Apple Silicon
    list(APPEND CMAKE_PREFIX_PATH "/usr/local")       # Intel Macs
    
    # Set minimum macOS version for compatibility
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
endif()

#################
# PACKAGES SETUP
#################

#################
# SDL SETUP
#################
# Option to choose between vendored and system packages
option(USE_VENDORED_SDL3 "Use vendored SDL3 library" ON)
option(USE_VENDORED_ZMQ "Use vendored ZeroMQ library" ON)
option(USE_VENDORED_CPPZMQ "Use vendored cppzmq library" ON)


# SDL3 Setup
if(USE_VENDORED_SDL3)
    # Check if vendored SDL directory exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl/CMakeLists.txt")
        message(STATUS "Using vendored SDL3 from third_party/sdl")
        add_subdirectory(third_party/sdl EXCLUDE_FROM_ALL)
    else()
        message(WARNING "Vendored SDL3 not found at third_party/sdl, falling back to system SDL3")
        message(STATUS "Looking for system SDL3...")
        find_package(SDL3 REQUIRED CONFIG) 
        if(SDL3_FOUND)
            message(STATUS "Found system SDL3 version: ${SDL3_VERSION}")
        else()
            message(FATAL_ERROR "SDL3 not found! Please install via Homebrew: brew install sdl3")
        endif()
    endif()
endif()

#################
# ZMQ SETUP
#################

if(USE_VENDORED_ZMQ)
    # Check if vendored SDL directory exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libzmq/CMakeLists.txt")
        message(STATUS "Using vendored ZeroMQ from third_party/libzmq")
        set(POLLER epoll)
        add_subdirectory(third_party/libzmq EXCLUDE_FROM_ALL)
        
    else()
        message(WARNING "Vendored ZMQ not found at third_party/libzmq, falling back to system ZeroMQ")
        message(STATUS "Looking for system ZeroMQ...")
        find_package(ZeroMQ REQUIRED CONFIG) 
        if(ZeroMQ_FOUND)
            message(STATUS "Found system ZeroMQ version: ${ZeroMQ_VERSION}")
        else()
            message(FATAL_ERROR "ZeroMQ not found! Please install via Homebrew: brew install libzmq")
        endif()
    endif()
endif()


if(USE_VENDORED_CPPZMQ)
    # Check if vendored SDL directory exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cppzmq/CMakeLists.txt")
        message(STATUS "Using vendored cppzmq from third_party/cppzmq")
        add_subdirectory(third_party/cppzmq EXCLUDE_FROM_ALL)
    else()
        message(WARNING "Vendored cppzmq not found at third_party/cppzmq, falling back to system cppzmq")
        message(STATUS "Looking for system cppzmq...")
        find_package(cppzmq REQUIRED CONFIG) 
        if(cppzmq_FOUND)
            message(STATUS "Found system cppzmq version: ${cppzmq_VERSION}")
        else()
            message(FATAL_ERROR "cppzmq not found! Please install via Homebrew: brew install cppzmq")
        endif()
    endif()
endif()

#################
# SOURCE FILES
#################

# ---------- ENGINE SOURCES ----------
set(REQUIRED_SOURCES
    src/EntityManager.cpp
    src/main.cpp
    src/GameEngine.cpp
    src/GameServer.cpp
    src/GameClient.cpp
    src/Input.cpp
    src/Render.cpp
    src/Physics.cpp
    src/Collisions.cpp
    src/vec2.cpp
    src/JobSystem.cpp
)

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

set(ENGINE_HEADERS
    src/GameEngine.h
    src/GameServer.h
    src/GameClient.h
    src/Input.h
    src/Render.h
    src/Physics.h
    src/Collisions.h
    src/Entity.h
)

##############
# TARGETS
##############

set(CORE_SOURCES ${ENGINE_SOURCES})
list(REMOVE_ITEM CORE_SOURCES src/main.cpp)

add_library(EngineCore STATIC
    ${CORE_SOURCES}
    ${ENGINE_HEADERS}
)
target_include_directories(EngineCore PUBLIC src)
target_link_libraries(EngineCore PUBLIC SDL3::SDL3 libzmq cppzmq)

##########################
# APPLICATION TARGETS
##########################


# ---------- GAME ENGINE APP ----------
add_executable(GameEngine src/main.cpp)
target_include_directories(GameEngine PRIVATE src)
target_link_libraries(GameEngine PRIVATE EngineCore)

# ---------- SERVER & CLIENT APPS ----------
add_executable(GameServer src/server_main.cpp)
target_include_directories(GameServer PRIVATE src)
target_link_libraries(GameServer PRIVATE EngineCore)

# GameClient app
add_executable(GameClient src/client_main.cpp)
target_include_directories(GameClient PRIVATE src)
target_link_libraries(GameClient PRIVATE EngineCore)

# ---------- P2P TEMPLATE + DEMO ----------
add_library(P2PTemplate STATIC
    src/p2p/P2PTracker.cpp src/p2p/P2PTracker.h
    src/p2p/P2PNode.cpp    src/p2p/P2PNode.h
)
target_include_directories(P2PTemplate PUBLIC src)
target_link_libraries(P2PTemplate PUBLIC SDL3::SDL3 libzmq cppzmq)

add_executable(P2PTrackerExe
    src/p2p/tracker_main.cpp
)
target_include_directories(P2PTrackerExe PRIVATE src)
target_link_libraries(P2PTrackerExe PRIVATE P2PTemplate)

add_executable(P2PDemoPong
    src/p2pPongDemo/p2p_pong_demo_main.cpp
    src/p2pPongDemo/PongDemo.cpp src/p2pPongDemo/PongDemo.h
)
target_include_directories(P2PDemoPong PRIVATE src)
target_link_libraries(P2PDemoPong PRIVATE EngineCore P2PTemplate)

# --- P2P Template V2 (snapshot-enabled node + tracker) ---
add_library(P2PTemplateV2 STATIC
    src/p2p_v2/P2PTrackerV2.cpp src/p2p_v2/P2PTrackerV2.h
    src/p2p_v2/P2PNodeV2.cpp    src/p2p_v2/P2PNodeV2.h
)
target_include_directories(P2PTemplateV2 PUBLIC src)
target_link_libraries(P2PTemplateV2 PUBLIC SDL3::SDL3 libzmq cppzmq)

# v2 tracker executable
add_executable(P2PTrackerV2Exe
    src/p2p_v2/tracker_v2_main.cpp
)
target_include_directories(P2PTrackerV2Exe PRIVATE src)
target_link_libraries(P2PTrackerV2Exe PRIVATE P2PTemplateV2)

# Decentralized Pong demo (no single authority)
add_executable(P2PDemoPongV2
    src/p2pPongDemoV2/p2p_pong_demo_v2_main.cpp
    src/p2pPongDemoV2/PongDemoV2.cpp src/p2pPongDemoV2/PongDemoV2.h
)
target_include_directories(P2PDemoPongV2 PRIVATE src)
target_link_libraries(P2PDemoPongV2 PRIVATE EngineCore P2PTemplateV2)


# macOS specific settings
if(APPLE)
    # Enable bundle creation for macOS apps (optional)
    # set_target_properties(GameEngine PROPERTIES MACOSX_BUNDLE TRUE)
    
    # Set proper install name for dynamic libraries on macOS
    set_target_properties(GameEngine PROPERTIES
        INSTALL_RPATH "@executable_path;/opt/homebrew/lib;/usr/local/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    # Add rpath for Homebrew libraries
    set_target_properties(GameEngine PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
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
