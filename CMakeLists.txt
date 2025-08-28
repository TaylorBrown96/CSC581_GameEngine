cmake_minimum_required(VERSION 3.16)
project(GameEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Define source files for the game engine
set(ENGINE_SOURCES
    src/main.cpp
    src/GameEngine.cpp
)

# Define header files (for IDE organization)
set(ENGINE_HEADERS
    src/GameEngine.h
)

# Create your game executable target
add_executable(GameEngine ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Set include directories
target_include_directories(GameEngine PRIVATE src)

# Link to the actual SDL3 library
target_link_libraries(GameEngine PRIVATE SDL3::SDL3)

# Copy media folder to build directory (optional - for easier asset loading)
add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/media $<TARGET_FILE_DIR:GameEngine>/media
    COMMENT "Copying media folder to build directory"
)

# Set project properties for better organization in IDEs
set_target_properties(GameEngine PROPERTIES
    FOLDER "GameEngine"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
)