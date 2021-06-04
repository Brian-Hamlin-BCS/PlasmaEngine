PlasmaCmakeInit()
# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

PlasmaCreateTarget(LIBRARY ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Common
    Support
)

target_link_libraries(RendererGL
  PUBLIC
    GL
    Glew
    tracy
)

if (NOT WIN32)
  target_link_libraries(RendererGL
    PUBLIC
      SDL
  )
endif()