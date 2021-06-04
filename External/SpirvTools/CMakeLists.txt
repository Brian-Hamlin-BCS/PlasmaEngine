PlasmaCmakeInit()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

PlasmaCreateTarget(LIBRARY ${PROJECT_NAME})

target_include_directories(SpirvTools
  PUBLIC
    SpirvHeaders
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Build/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/source/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/
)
