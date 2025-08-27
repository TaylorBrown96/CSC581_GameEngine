add_executable(GameRunner
  main.cpp
  ExampleGame.cpp
)

target_include_directories(GameRunner PRIVATE ${CMAKE_SOURCE_DIR}/engine)
target_link_libraries(GameRunner PRIVATE engine)

# Copy SDL runtime next to the exe (Windows)
if(WIN32 AND TARGET SDL3::SDL3)
  add_custom_command(TARGET GameRunner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:SDL3::SDL3>
      $<TARGET_FILE_DIR:GameRunner>
  )
endif()

# Copy an asset for the demo if it exists
set(ASSET_SRC "${CMAKE_SOURCE_DIR}/assets/skelly_idle.bmp")
if(EXISTS "${ASSET_SRC}")
  add_custom_command(TARGET GameRunner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:GameRunner>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ASSET_SRC}
            $<TARGET_FILE_DIR:GameRunner>/assets/skelly_idle.bmp
  )
endif()
