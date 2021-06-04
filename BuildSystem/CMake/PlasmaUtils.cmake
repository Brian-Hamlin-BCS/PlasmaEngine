include("PlasmaUtilsVars")
include("PlasmaUtilsPCH")
include("PlasmaUtilsUnityFiles")
include("PlasmaUtilsDetect")
include("PlasmaCI")
include("PlasmaUtilsCppFlags")
include("PlasmaUtilsTarget")
include("PlasmaUtilsTargetCS")
include("PlasmaUtilsSubmodule")
include("PlasmaGit")
include("PlasmaVersion")


######################################
### PlasmaSetTargetOutputDirs(<target> <lib-output-dir> <dll-output-dir>)
######################################

function(PlasmaSetTargetOutputDirs TARGET_NAME LIB_OUTPUT_DIR DLL_OUTPUT_DIR)

	PlasmaPullAllVars()

	set(SUB_DIR "")

	if (PLASMA_CMAKE_PLATFORM_WINDOWS_UWP)
		# UWP has deployment problems if all applications output to the same path.
		set(SUB_DIR "/${TARGET_NAME}")
	endif()

	set (OUTPUT_LIB_DEBUG       "${LIB_OUTPUT_DIR}/${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}Debug${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}${SUB_DIR}")
	set (OUTPUT_LIB_RELEASE     "${LIB_OUTPUT_DIR}/${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}Release${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}${SUB_DIR}")
	set (OUTPUT_LIB_MINSIZE     "${LIB_OUTPUT_DIR}/${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}MinSize${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}${SUB_DIR}")
	set (OUTPUT_LIB_RELWITHDEB  "${LIB_OUTPUT_DIR}/${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}RelDeb${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}${SUB_DIR}")

	set (OUTPUT_DLL_DEBUG       "${DLL_OUTPUT_DIR}/${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}Debug${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}${SUB_DIR}")
	set (OUTPUT_DLL_RELEASE     "${DLL_OUTPUT_DIR}/${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}Release${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}${SUB_DIR}")
	set (OUTPUT_DLL_MINSIZE     "${DLL_OUTPUT_DIR}/${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}MinSize${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}${SUB_DIR}")
	set (OUTPUT_DLL_RELWITHDEB  "${DLL_OUTPUT_DIR}/${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}RelDeb${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}${SUB_DIR}")

	# If we can't use generator expressions the non-generator expression version of the
	# output directory should point to the version matching CMAKE_BUILD_TYPE. This is the case for
	# add_custom_command BYPRODUCTS for example needed by Ninja.
	if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set_target_properties(${TARGET_NAME} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DLL_DEBUG}"
			LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LIB_DEBUG}"
			ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LIB_DEBUG}"
		)
	elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
		set_target_properties(${TARGET_NAME} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DLL_RELEASE}"
			LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LIB_RELEASE}"
			ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LIB_RELEASE}"
		)
	elseif (${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
		set_target_properties(${TARGET_NAME} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DLL_MINSIZE}"
			LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LIB_MINSIZE}"
			ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LIB_MINSIZE}"
		)
	elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
		set_target_properties(${TARGET_NAME} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DLL_RELWITHDEB}"
			LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LIB_RELWITHDEB}"
			ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LIB_RELWITHDEB}"
		)
	endif()	

	set_target_properties(${TARGET_NAME} PROPERTIES
	  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DLL_DEBUG}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_LIB_DEBUG}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_LIB_DEBUG}"
  )

	set_target_properties(${TARGET_NAME} PROPERTIES
	  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DLL_RELEASE}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_LIB_RELEASE}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_LIB_RELEASE}"
  )

	set_target_properties(${TARGET_NAME} PROPERTIES
	  RUNTIME_OUTPUT_DIRECTORYMINSIZEREL "${OUTPUT_DLL_MINSIZE}"
    LIBRARY_OUTPUT_DIRECTORYMINSIZEREL "${OUTPUT_LIB_MINSIZE}"
    ARCHIVE_OUTPUT_DIRECTORYMINSIZEREL "${OUTPUT_LIB_MINSIZE}"
	)

	set_target_properties(${TARGET_NAME} PROPERTIES
	  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUTPUT_DLL_RELWITHDEB}"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUTPUT_LIB_RELWITHDEB}"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUTPUT_LIB_RELWITHDEB}"
	)

endfunction()

######################################
### PlasmaSetDefaultTargetOutputDirs(<target>)
######################################

function(PlasmaSetDefaultTargetOutputDirs TARGET_NAME)

	set (PLASMA_OUTPUT_DIRECTORY_LIB "${CMAKE_SOURCE_DIR}/Output/Lib" CACHE PATH "Where to store the compiled .lib files.")
	set (PLASMA_OUTPUT_DIRECTORY_DLL "${CMAKE_SOURCE_DIR}/Output/Bin" CACHE PATH "Where to store the compiled .dll files.")

	mark_as_advanced(FORCE PLASMA_OUTPUT_DIRECTORY_LIB)
	mark_as_advanced(FORCE PLASMA_OUTPUT_DIRECTORY_DLL)

	PlasmaSetTargetOutputDirs("${TARGET_NAME}" "${PLASMA_OUTPUT_DIRECTORY_LIB}" "${PLASMA_OUTPUT_DIRECTORY_DLL}")

endfunction()


######################################
### PlasmaWriteConfigurationTxt()
######################################

function(PlasmaWriteConfigurationTxt)

	# Clear Targets.txt and Tests.txt
	file(WRITE ${CMAKE_BINARY_DIR}/Targets.txt "")
	file(WRITE ${CMAKE_BINARY_DIR}/Tests.txt "")

	PlasmaPullAllVars()

	# Write configuration to file, as this is done at configure time we must pin the configuration in place (RelDeb is used because all build machines use this).
	file(WRITE ${CMAKE_BINARY_DIR}/Configuration.txt "")
	set(CONFIGURATION_DESC "${PLASMA_CMAKE_PLATFORM_PREFIX}${PLASMA_CMAKE_GENERATOR_PREFIX}${PLASMA_CMAKE_COMPILER_POSTFIX}RelDeb${PLASMA_CMAKE_ARCHITECTURE_POSTFIX}")
	file(APPEND ${CMAKE_BINARY_DIR}/Configuration.txt ${CONFIGURATION_DESC})

endfunction()


######################################
### PlasmaAddTargetFolderAsIncludeDir(<target> <path-to-target>)
######################################

function(PlasmaAddTargetFolderAsIncludeDir TARGET_NAME TARGET_FOLDER)

	get_filename_component(PARENT_DIR ${TARGET_FOLDER} DIRECTORY)

	target_include_directories(${TARGET_NAME} PRIVATE "${TARGET_FOLDER}")
	target_include_directories(${TARGET_NAME} PUBLIC "${PARENT_DIR}")

endfunction()

######################################
### PlasmaSetCommonTargetDefinitions(<target>)
######################################

function(PlasmaSetCommonTargetDefinitions TARGET_NAME)

	PlasmaPullAllVars()

	# set the BUILDSYSTEM_COMPILE_ENGINE_AS_DLL definition
	if (PLASMA_COMPILE_ENGINE_AS_DLL)
		target_compile_definitions(${TARGET_NAME} PUBLIC BUILDSYSTEM_COMPILE_ENGINE_AS_DLL)
	endif()

	# set the BUILDSYSTEM_CONFIGURATION definition
	target_compile_definitions(${TARGET_NAME} PRIVATE BUILDSYSTEM_CONFIGURATION="${PLASMA_CMAKE_GENERATOR_CONFIGURATION}")

	# set the BUILDSYSTEM_BUILDING_XYZ_LIB definition
	string(TOUPPER ${TARGET_NAME} PROJECT_NAME_UPPER)
	target_compile_definitions(${TARGET_NAME} PRIVATE BUILDSYSTEM_BUILDING_${PROJECT_NAME_UPPER}_LIB)
	
	if (PLASMA_BUILD_EXPERIMENTAL_VULKAN)
		target_compile_definitions(${TARGET_NAME} PRIVATE BUILDSYSTEM_ENABLE_VULKAN_SUPPORT)
	endif()

endfunction()

######################################
### PlasmaSetProjectIDEFolder(<target> <path-to-target>)
######################################

function(PlasmaSetProjectIDEFolder TARGET_NAME PROJECT_SOURCE_DIR)

	# globally enable sorting targets into folders in IDEs
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	get_filename_component (PARENT_FOLDER ${PROJECT_SOURCE_DIR} PATH)
	get_filename_component (FOLDER_NAME ${PARENT_FOLDER} NAME)

	set(IDE_FOLDER "${FOLDER_NAME}")

	if (${PROJECT_SOURCE_DIR} MATCHES "/Code/")

		get_filename_component (PARENT_FOLDER ${PARENT_FOLDER} PATH)
		get_filename_component (FOLDER_NAME ${PARENT_FOLDER} NAME)

		while(NOT ${FOLDER_NAME} STREQUAL "Code")

			set(IDE_FOLDER "${FOLDER_NAME}/${IDE_FOLDER}")

			get_filename_component (PARENT_FOLDER ${PARENT_FOLDER} PATH)
			get_filename_component (FOLDER_NAME ${PARENT_FOLDER} NAME)
		
		endwhile()

	endif()

	get_property(PLASMA_SUBMODULE_MODE GLOBAL PROPERTY PLASMA_SUBMODULE_MODE)
	
	if(PLASMA_SUBMODULE_MODE)
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "PlasmaEngine/${IDE_FOLDER}")
	else()
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER ${IDE_FOLDER})
	endif()

endfunction()

######################################
### PlasmaAddOutputPlasmaPrefix(<target>)
######################################

function(PlasmaAddOutputPlasmaPrefix TARGET_NAME)

	set_target_properties(${TARGET_NAME} PROPERTIES IMPORT_PREFIX "Plasma")
	set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "Plasma")

endfunction()

######################################
### PlasmaSetLibraryProperties(<target>)
######################################

function(PlasmaSetLibraryProperties TARGET_NAME)

    PlasmaPullAllVars()

	if (PLASMA_CMAKE_PLATFORM_LINUX)
		# Workaround for: https://bugs.launchpad.net/ubuntu/+source/gcc-5/+bug/1568899
		target_link_libraries (${TARGET_NAME} PRIVATE -lgcc_s -lgcc pthread rt)
	endif ()

	if (PLASMA_CMAKE_PLATFORM_OSX OR PLASMA_CMAKE_PLATFORM_LINUX)
		find_package(X11 REQUIRED)
		find_package(SFML REQUIRED system window)
		target_include_directories (${TARGET_NAME} PRIVATE ${X11_X11_INCLUDE_PATH})
		target_link_libraries (${TARGET_NAME} PRIVATE ${X11_X11_LIB} sfml-window sfml-system)
	endif ()

endfunction()

######################################
### PlasmaSetApplicationProperties(<target>)
######################################

function(PlasmaSetApplicationProperties TARGET_NAME)

    PlasmaPullAllVars()

	if (PLASMA_CMAKE_PLATFORM_OSX OR PLASMA_CMAKE_PLATFORM_LINUX)
		find_package(X11 REQUIRED)
		find_package(SFML REQUIRED system window)
		target_include_directories (${TARGET_NAME} PRIVATE ${X11_X11_INCLUDE_PATH})
		target_link_libraries (${TARGET_NAME} PRIVATE ${X11_X11_LIB} sfml-window sfml-system)
	endif ()

	# We need to link against X11, pthread and rt last or linker errors will occur.
	if (PLASMA_CMAKE_COMPILER_GCC)
		target_link_libraries (${TARGET_NAME} PRIVATE pthread rt)
	endif ()

endfunction()

######################################
### PlasmaSetNatvisFile(<target> <path-to-natvis-file>)
######################################

function(PlasmaSetNatvisFile TARGET_NAME NATVIS_FILE)

	# We need at least visual studio 2015 for this to work
	if ((MSVC_VERSION GREATER 1900) OR (MSVC_VERSION EQUAL 1900))

		target_sources(${TARGET_NAME} PRIVATE ${NATVIS_FILE})

	endif()

endfunction()


######################################
### PlasmaMakeWinmainExecutable(<target>)
######################################

function(PlasmaMakeWinmainExecutable TARGET_NAME)

	set_property(TARGET ${TARGET_NAME} PROPERTY WIN32_EXECUTABLE ON)

endfunction()


######################################
### PlasmaGatherSubfolders(<abs-path-to-folder> <out-sub-folders>)
######################################

function(PlasmaGatherSubfolders START_FOLDER RESULT_FOLDERS)

	set(ALL_FILES "")
	set(ALL_DIRS "")

	file(GLOB_RECURSE ALL_FILES RELATIVE "${START_FOLDER}" "${START_FOLDER}/*")

	foreach(FILE ${ALL_FILES})

		get_filename_component(FILE_PATH ${FILE} DIRECTORY)

		list(APPEND ALL_DIRS ${FILE_PATH})

	endforeach()

	list(REMOVE_DUPLICATES ALL_DIRS)

	set(${RESULT_FOLDERS} ${ALL_DIRS} PARENT_SCOPE)

endfunction()


######################################
### PlasmaGlobSourceFiles(<path-to-folder> <out-files>)
######################################

function(PlasmaGlobSourceFiles ROOT_DIR RESULT_ALL_SOURCES)

  file(GLOB_RECURSE CPP_FILES "${ROOT_DIR}/*.cpp" "${ROOT_DIR}/*.cc")
  file(GLOB_RECURSE H_FILES "${ROOT_DIR}/*.h" "${ROOT_DIR}/*.hpp" "${ROOT_DIR}/*.inl")
  file(GLOB_RECURSE C_FILES "${ROOT_DIR}/*.c")
  file(GLOB_RECURSE CS_FILES "${ROOT_DIR}/*.cs")

  file(GLOB_RECURSE UI_FILES "${ROOT_DIR}/*.ui")
  file(GLOB_RECURSE QRC_FILES "${ROOT_DIR}/*.qrc")
  file(GLOB_RECURSE RES_FILES "${ROOT_DIR}/*.ico" "${ROOT_DIR}/*.rc")

  set(${RESULT_ALL_SOURCES} ${CPP_FILES} ${H_FILES} ${C_FILES} ${CS_FILES} ${UI_FILES} ${QRC_FILES} ${RES_FILES} PARENT_SCOPE)

endfunction()

######################################
### PlasmaAddAllSubdirs()
######################################

function(PlasmaAddAllSubdirs)

	# find all cmake files below this directory
	file (GLOB SUB_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/*/CMakeLists.txt")

	foreach (VAR ${SUB_DIRS})

		get_filename_component (RES ${VAR} DIRECTORY)

		add_subdirectory (${RES})

	endforeach ()

endfunction()

######################################
### PlasmaCmakeInit()
######################################

macro(PlasmaCmakeInit)

    PlasmaPullAllVars()

endmacro()

######################################
### PlasmaRequires(<variable>)
######################################

macro(PlasmaRequires)

  if(${ARGC} EQUAL 0)
    return()
  endif()

  set(ALL_ARGS "${ARGN}")

  foreach(arg IN LISTS ALL_ARGS)

    if (NOT ${arg})
      return()
    endif()

  endforeach()

endmacro()

######################################
### PlasmaRequiresWindows()
######################################

macro(PlasmaRequiresWindows)

    PlasmaRequires(PLASMA_CMAKE_PLATFORM_WINDOWS)

endmacro()

######################################
### PlasmaRequiresWindowsDesktop()
######################################

macro(PlasmaRequiresWindowsDesktop)

    PlasmaRequires(PLASMA_CMAKE_PLATFORM_WINDOWS_DESKTOP)

endmacro()

######################################
### PlasmaAddExternalFolder(<project-number>)
######################################

function(PlasmaAddExternalFolder PROJECT_NUMBER)

	set(CACHE_VAR_NAME "PLASMA_EXTERNAL_PROJECT${PROJECT_NUMBER}")

	set (${CACHE_VAR_NAME} "" CACHE PATH "A folder outside the plasma repository that should be parsed for CMakeLists.txt files to include projects into the plasma solution.")

	set(CACHE_VAR_VALUE ${${CACHE_VAR_NAME}})

	if (NOT CACHE_VAR_VALUE)
		return()
	endif()

	set_property(GLOBAL PROPERTY "GATHER_EXTERNAL_PROJECTS" TRUE)
	add_subdirectory(${CACHE_VAR_VALUE} "${CMAKE_BINARY_DIR}/ExternalProject${PROJECT_NUMBER}")
	set_property(GLOBAL PROPERTY "GATHER_EXTERNAL_PROJECTS" FALSE)

endfunction()

######################################
### PlasmaInitProjects()
######################################

function(PlasmaInitProjects)

	# find all init.cmake files below this directory
	file (GLOB_RECURSE INIT_FILES "init.cmake")

	foreach (INIT_FILE ${INIT_FILES})

		message(STATUS "Including '${INIT_FILE}'")
		include("${INIT_FILE}")

	endforeach ()
	
endfunction()

######################################
### PlasmaFinalizeProjects()
######################################

function(PlasmaFinalizeProjects)

	# find all init.cmake files below this directory
	file (GLOB_RECURSE INIT_FILES "finalize.cmake")

	foreach (INIT_FILE ${INIT_FILES})

		message(STATUS "Including '${INIT_FILE}'")
		include("${INIT_FILE}")

	endforeach ()
	
endfunction()

######################################
### PlasmaBuildFilterInit()
######################################

# The build filter is intended to only build a subset of PlasmaEngine. 
# This is modeled as a enum so it is extensible
# use the PlasmaBuildFilterXXX macros to filter out libraries.
# Currently the only two values are
# Foundation - 0: only build foundation and related tests
# Everything - 1: build everything
function(PlasmaBuildFilterInit)

	set(PLASMA_BUILD_FILTER "Everything" CACHE STRING "Whether tool projects should be added to the solution")
	set(PLASMA_BUILD_FILTER_VALUES FoundationOnly Runtime Renderer Everything)
	set_property(CACHE PLASMA_BUILD_FILTER PROPERTY STRINGS ${PLASMA_BUILD_FILTER_VALUES})
	list(FIND PLASMA_BUILD_FILTER_VALUES ${PLASMA_BUILD_FILTER} index)
	set_property(GLOBAL PROPERTY PLASMA_BUILD_FILTER_INDEX ${index})
	
endfunction()

set(BUILD_FILTER_INDEX_FOUNDATIONONLY 0)
set(BUILD_FILTER_INDEX_RUNTIME 1)
set(BUILD_FILTER_INDEX_RENDERER 2)
set(BUILD_FILTER_INDEX_EVERYTHING 3)

######################################
### PlasmaBuildFilterGetIndex(<variable>)
######################################

function(PlasmaBuildFilterGetIndex OUT_NAME)

	get_property(INDEX GLOBAL PROPERTY PLASMA_BUILD_FILTER_INDEX)

	set(${OUT_NAME} "${INDEX}" PARENT_SCOPE)

endfunction()

######################################
### PlasmaBuildFilterRuntime()
######################################

# Project will only be included if build filter is set to runtime or higher
macro(PlasmaBuildFilterRuntime)

    PlasmaBuildFilterGetIndex(filterIndex)

	if(${filterIndex} LESS ${BUILD_FILTER_INDEX_RUNTIME})
		return()
	endif()
endmacro()

######################################
### PlasmaBuildFilterRenderer()
######################################

# Project will only be included if build filter is set to renderer or higher
macro(PlasmaBuildFilterRenderer)

    PlasmaBuildFilterGetIndex(filterIndex)

	if(${filterIndex} LESS ${BUILD_FILTER_INDEX_RENDERER})
		return()
	endif()
endmacro()

######################################
### PlasmaBuildFilterEerything()
######################################

# Project will only be included if build filter is set to everything
macro(PlasmaBuildFilterEerything)

    PlasmaBuildFilterGetIndex(filterIndex)

	if(${filterIndex} LESS ${BUILD_FILTER_INDEX_EVERYTHING})
		return()
	endif()
endmacro()