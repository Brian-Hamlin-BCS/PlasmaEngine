PlasmaCmakeInit()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

PlasmaCreateTarget(LIBRARY ${PROJECT_NAME})

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PlasmaConfig.hpp.in ${CMAKE_CURRENT_LIST_DIR}/PlasmaConfig.hpp)

PlasmaSetNatvisFile(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/${PLASMA_SUBMODULE_PREFIX_PATH}/Utilities/Visual Studio Visualizer/PlasmaEngine.natvis")


target_link_libraries(${PROJECT_NAME}
    PUBLIC
        tracy
)