cmake_minimum_required(VERSION 3.12...3.14)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not supported. Please choose a different binary directory.")
	return()
endif()

set(PLASMA_CMAKE_UTILS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/BuildSystem/CMake" CACHE INTERNAL "The path where Plasma stores CMake utility functions.")
set(PLASMA_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PLASMA_TOOLCHAIN_DIR ${PLASMA_CMAKE_DIR}/Toolchain/)
set(PLASMA_TARGETOS_DIR ${PLASMA_CMAKE_DIR}/TargetOs/)
set(PLASMA_LIBRARIES_DIR ${PLASMA_CORE_DIR}/Source/)
set(PLASMA_PLATFORM_DATA_DIR ${PLASMA_CORE_DIR}/PlatformData/${PLASMA_TOOLCHAIN})

# Required to find cmake modules like 'FindDirectX.cmake'
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PLASMA_CMAKE_UTILS_PATH})

include("PlasmaUtils")



file(RELATIVE_PATH PLASMA_SUBMODULE_PREFIX_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(GLOBAL PROPERTY PLASMA_SUBMODULE_PREFIX_PATH ${PLASMA_SUBMODULE_PREFIX_PATH})
if(PLASMA_SUBMODULE_PREFIX_PATH STREQUAL "")
	set(PLASMA_SUBMODULE_MODE FALSE)
else()
	set(PLASMA_SUBMODULE_MODE TRUE)
endif()
set_property(GLOBAL PROPERTY PLASMA_SUBMODULE_MODE ${PLASMA_SUBMODULE_MODE})

PlasmaBuildFilterInit()

PlasmaDetectProjectName(DETECTED_NAME)

if(NOT PLASMA_SUBMODULE_MODE)
	PlasmaDetectLanguages()

	set (PLASMA_SOLUTION_NAME ${DETECTED_NAME} CACHE STRING "The name of the solution")
	project (${PLASMA_SOLUTION_NAME}
		VERSION 1.0
		DESCRIPTION "An open-source game engine"
		HOMEPAGE_URL "https://plasmagameengine.com/"
		LANGUAGES ${PLASMA_LANGUAGES})
endif()

PlasmaCheckBuildType()
PlasmaWriteConfigurationTxt()
PlasmaInitProjects()

add_definitions(-DPlasmaConfigName="${PLASMA_CONFIG}")
add_definitions(-DPlasmaArchitectureName="x64")

add_subdirectory(Source/Platform/${CMAKE_SYSTEM_NAME})

add_subdirectory(External)
add_subdirectory(Source)

# Enable colored output for Ninja if supported
if (CMAKE_GENERATOR STREQUAL "Ninja")
	include(CheckCXXCompilerFlag)
	
	check_cxx_compiler_flag("-fdiagnostics-color=always" F_DIAGNOSTIC_COLOR_ALWAYS)
	if (F_DIAGNOSTIC_COLOR_ALWAYS)
		add_compile_options("-fdiagnostics-color=always")
	endif()
endif()


message(STATUS "[PLASMA BUILD] GIT Revision ID: ${PLASMA_REVISION_ID}")
message(STATUS "[PLASMA BUILD] GIT Short ChangeSet: ${PLASMA_SHORT_CHANGE_SET}")
message(STATUS "[PLASMA BUILD] GIT ChangeSet: ${PLASMA_CHANGE_SET}")
message(STATUS "[PLASMA BUILD] GIT ChangeSet Date: ${PLASMA_CHANGE_SET_DATE}")
message(STATUS "[PLASMA BUILD] Plasma Engine version ${PLASMA_VERSION}")
message(STATUS "[PLASMA BUILD] Build platform: ${PLASMA_PLATFORM_NAME}")
