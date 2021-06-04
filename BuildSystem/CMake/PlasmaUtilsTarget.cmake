######################################
### PlasmaCreateTarget(<LIBRARY | APPLICATION> <target-name> [NO_PCH] [NO_UNITY] [NO_QT] [EXCLUDE_FOLDER_FOR_UNITY <relative-folder>...])
######################################

function(PlasmaCreateTarget TYPE TARGET_NAME)

    set(ARG_OPTIONS NO_PCH NO_UNITY NO_QT NO_PLASMA_PREFIX)
    set(ARG_ONEVALUEARGS "")
    set(ARG_MULTIVALUEARGS EXCLUDE_FOLDER_FOR_UNITY EXCLUDE_FROM_PCH_REGEX MANUAL_SOURCE_FILES)
    cmake_parse_arguments(ARG "${ARG_OPTIONS}" "${ARG_ONEVALUEARGS}" "${ARG_MULTIVALUEARGS}" ${ARGN} )

    if (ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "PlasmaCreateTarget: Invalid arguments '${ARG_UNPARSED_ARGUMENTS}'")
    endif()

    PlasmaPullAllVars()

    if(DEFINED ARG_MANUAL_SOURCE_FILES)
		set(ALL_SOURCE_FILES ${ARG_MANUAL_SOURCE_FILES})
	else()
		PlasmaGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} ALL_SOURCE_FILES)
	endif()
	

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

        PlasmaSetLibraryProperties(${TARGET_NAME})
    elseif (${TYPE} STREQUAL "APPLICATION")

        message (STATUS "Application: ${TARGET_NAME}")
		
		# On Android we can't use executables. Instead we have to use shared libraries which are loaded from java code.
		if (PLASMA_CMAKE_PLATFORM_ANDROID)
			# All Plasma applications must include the native app glue implementation
			add_library(${TARGET_NAME} SHARED ${ALL_SOURCE_FILES} "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
			
			# Prevent the linker from stripping away the application entry point of android_native_app_glue: ANativeActivity_onCreate
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

			# The log and android libraries are library dependencies of android_native_app_glue
            target_link_libraries(${TARGET_NAME} PRIVATE log android EGL GLESv1_CM)    
		else()
			add_executable (${TARGET_NAME} ${ALL_SOURCE_FILES})
		endif()
        
        PlasmaSetApplicationProperties(${TARGET_NAME})

    else()

        message(FATAL_ERROR "PlasmaCreateTarget: Missing argument to specify target type. Pass in 'APP' or 'LIB'.")

    endif()

    # sort files into the on-disk folder structure in the IDE
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCE_FILES})

    if (NOT ${ARG_NO_PCH})

        PlasmaAutoPCH(${TARGET_NAME} "${ALL_SOURCE_FILES}" ${ARG_EXCLUDE_FROM_PCH_REGEX})

    endif()
	
	# When using the Open Folder workflow inside visual studio on android, visual studio gets confused due to our custom output directory
	# Do not set the custom output directory in this case
	if((NOT ANDROID) OR (NOT PLASMA_CMAKE_INSIDE_VS))
		PlasmaSetDefaultTargetOutputDirs(${TARGET_NAME})
	endif()
	
    # #We need the target directory to add the apk packaging steps for android. Thus, this step needs to be done here.
	# if (${TYPE} STREQUAL "APPLICATION")
	# 	PlasmaAndroidAddDefaultContent(${TARGET_NAME})
	# endif()
	
    PlasmaAddTargetFolderAsIncludeDir(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

    PlasmaSetCommonTargetDefinitions(${TARGET_NAME})

    PlasmaSetBuildFlags(${TARGET_NAME})

    PlasmaSetProjectIDEFolder(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
	
	# Pass the windows sdk include paths to the resource compiler when not generating a visual studio solution.
	if(PLASMA_CMAKE_PLATFORM_WINDOWS AND NOT PLASMA_CMAKE_GENERATOR_MSVC)
		set(RC_FILES ${ALL_SOURCE_FILES})
		list(FILTER RC_FILES INCLUDE REGEX ".*\\.rc$")
		if(RC_FILES)
			set_source_files_properties(${RC_FILES} 
				PROPERTIES COMPILE_FLAGS "/I\"C:/Program Files (x86)/Windows Kits/10/Include/${PLASMA_CMAKE_WINDOWS_SDK_VERSION}/shared\" /I\"C:/Program Files (x86)/Windows Kits/10/Include/${PLASMA_CMAKE_WINDOWS_SDK_VERSION}/um\""
			)
		endif()
	endif()
	
	if(PLASMA_CMAKE_PLATFORM_ANDROID)
		# Add the location for native_app_glue.h to the include directories.
		target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")
	endif()

    PlasmaCIAddToTargetsList(${TARGET_NAME} C++)

    if (NOT ${ARG_NO_UNITY})

        PlasmaGenerateFolderUnityFilesForTarget(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR} "${ARG_EXCLUDE_FOLDER_FOR_UNITY}")

    endif()
	
	get_property(GATHER_EXTERNAL_PROJECTS GLOBAL PROPERTY "GATHER_EXTERNAL_PROJECTS")
	
	if (GATHER_EXTERNAL_PROJECTS)
		set_property(GLOBAL APPEND PROPERTY "EXTERNAL_PROJECTS" ${TARGET_NAME})
	endif()

endfunction()