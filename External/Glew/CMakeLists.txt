PlasmaCmakeInit()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

PlasmaCreateTarget(LIBRARY ${PROJECT_NAME} ARG_MANUAL_SOURCE_FILES)

target_sources(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Repo/src/glew.c
)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    GLEW_STATIC
)
target_include_directories(Glew
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/
)