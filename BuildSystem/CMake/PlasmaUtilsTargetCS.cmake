######################################
### PlasmaCreateTargetCS(<LIBRARY | APPLICATION> <target-name>)
######################################

function(PlasmaCreateTargetCS TYPE TARGET_NAME)

    set(ARG_OPTIONS NO_PLASMA_PREFIX)
    set(ARG_ONEVALUEARGS "")
    set(ARG_MULTIVALUEARGS "")
    cmake_parse_arguments(ARG "${ARG_OPTIONS}" "${ARG_ONEVALUEARGS}" "${ARG_MULTIVALUEARGS}" ${ARGN} )

    if (ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "PlasmaCreateTargetCS: Invalid arguments '${ARG_UNPARSED_ARGUMENTS}'")
    endif()

    PlasmaPullAllVars()

    PlasmaGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} ALL_SOURCE_FILES)

    if ((${TYPE} STREQUAL "LIBRARY") OR (${TYPE} STREQUAL "STATIC_LIBRARY"))

        if ((${PLASMA_COMPILE_ENGINE_AS_DLL}) AND (${TYPE} STREQUAL "LIBRARY"))

            message (STATUS "Shared Library: ${TARGET_NAME}")
            add_library (${TARGET_NAME} SHARED ${ALL_SOURCE_FILES})

        else ()

            message (STATUS "Static Library: ${TARGET_NAME}")
            add_library (${TARGET_NAME} ${ALL_SOURCE_FILES})

        endif ()

        if (NOT ARG_NO_PLASMA_PREFIX)
            PlasmaAddOutputPlasmaPrefix(${TARGET_NAME})
        endif()

    elseif (${TYPE} STREQUAL "APPLICATION")

        message (STATUS "Application: ${TARGET_NAME}")

        add_executable (${TARGET_NAME} ${ALL_SOURCE_FILES})

    else()

        message(FATAL_ERROR "PlasmaCreateTargetCS: Missing argument to specify target type. Pass in 'APP' or 'LIB'.")

    endif()

    # sort files into the on-disk folder structure in the IDE
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCE_FILES})

    # default c# settings
    target_compile_options(${TARGET_NAME} PRIVATE "/langversion:6")
    set_property(TARGET ${TARGET_NAME} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
    set_property(TARGET ${TARGET_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
        
    PlasmaSetDefaultTargetOutputDirs(${TARGET_NAME})
    PlasmaSetProjectIDEFolder(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

endfunction()
