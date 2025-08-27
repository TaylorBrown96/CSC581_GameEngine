# Build the engine library
add_library(engine
  core/Application.cpp
  core/Window.cpp
  render/Renderer.cpp
  render/Texture.cpp
  render/SpriteSheet.cpp
  render/SpriteAnimator.cpp
  utils/AssetManager.cpp
)

# Expose "engine/" as an include root so we can do #include "render/Renderer.hpp"
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link SDL (vendored or system)
if(TARGET SDL3::SDL3)
  target_link_libraries(engine PUBLIC SDL3::SDL3)
else()
  message(FATAL_ERROR "SDL3::SDL3 not found.")
endif()

# Demo executable using the engine
add_executable(GameEngine
  main.cpp
)
target_link_libraries(GameEngine PRIVATE engine)

# Copy SDL runtime next to the exe on Windows
if(WIN32 AND TARGET SDL3::SDL3)
  add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:SDL3::SDL3>
      $<TARGET_FILE_DIR:GameEngine>
  )
endif()

# Copy demo asset into the exe dir (optional)
set(ASSET_SRC "${CMAKE_SOURCE_DIR}/assets/skelly_idle.bmp")
if(EXISTS "${ASSET_SRC}")
  add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:GameEngine>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ASSET_SRC}
            $<TARGET_FILE_DIR:GameEngine>/assets/skelly_idle.bmp
  )
endif()
