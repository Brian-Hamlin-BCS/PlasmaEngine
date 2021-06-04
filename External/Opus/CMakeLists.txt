PlasmaCmakeInit()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

PlasmaCreateTarget(LIBRARY ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    OPUS_BUILD
    USE_ALLOCA
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/silk/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/silk/float/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/celt/
)
