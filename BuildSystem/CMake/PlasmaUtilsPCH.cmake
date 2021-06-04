
######################################
### PlasmaSetTargetPCH(<target> <pch-name>)
######################################

function(PlasmaSetTargetPCH TARGET_NAME PCH_NAME)

  if (NOT PLASMA_USE_PCH)
    return()
  endif()

  #message(STATUS "Setting PCH for '${TARGET_NAME}': ${PCH_NAME}")
  set_property(TARGET ${TARGET_NAME} PROPERTY "PCH_FILE_NAME" ${PCH_NAME})

endfunction()

######################################
### PlasmaRetrieveTargetPCH(<target> <pch-name>)
######################################

function(PlasmaRetrieveTargetPCH TARGET_NAME PCH_NAME)

  if (NOT PLASMA_USE_PCH)
    return()
  endif()

  get_property(RESULT TARGET ${TARGET_NAME} PROPERTY "PCH_FILE_NAME")

  set (${PCH_NAME} ${RESULT} PARENT_SCOPE)
  #message(STATUS "Retrieved PCH for '${TARGET_NAME}': ${RESULT}")

endfunction()

######################################
### PlasmaPCHUse(<pch-header> <cpp-files>)
######################################

function(PlasmaPCHUse PCH_H TARGET_CPPS)

  if (NOT MSVC)
    return()
  endif()

  if (NOT PLASMA_USE_PCH)
    return()
  endif()  

  # only include .cpp files
  list(FILTER TARGET_CPPS INCLUDE REGEX "\.cpp$")

  # exclude files named 'qrc_*'
  list(FILTER TARGET_CPPS EXCLUDE REGEX ".*/qrc_.*")

  # exclude files named 'PCH.cpp'
  list(FILTER TARGET_CPPS EXCLUDE REGEX ".*PCH.cpp$")

  foreach(CPP_FILE ${TARGET_CPPS})

    set_source_files_properties (${CPP_FILE} PROPERTIES COMPILE_FLAGS "/Yu${PCH_H}")

  endforeach()

endfunction()

######################################
### PlasmaPCHCreate(<pch-header> <cpp-file>)
######################################

function(PlasmaPCHCreate PCH_H TARGET_CPP)

  if (NOT MSVC)
    return()
  endif()

  if (NOT PLASMA_USE_PCH)
    return()
  endif()

  set_source_files_properties(${TARGET_CPP} PROPERTIES COMPILE_FLAGS "/Yc${PCH_H}")

endfunction()

######################################
### PlasmaFindPCHInFileList(<files> <out-pch-name>)
######################################

function(PlasmaFindPCHInFileList FILE_LIST PCH_NAME)

  if (NOT PLASMA_USE_PCH)
    return()
  endif()

  foreach (CUR_FILE ${FILE_LIST})

    get_filename_component(CUR_FILE_NAME ${CUR_FILE} NAME_WE)
    get_filename_component(CUR_FILE_EXT ${CUR_FILE} EXT)

    if ((${CUR_FILE_EXT} STREQUAL ".cpp") AND (${CUR_FILE_NAME} MATCHES "PCH$"))
      set (${PCH_NAME} ${CUR_FILE_NAME} PARENT_SCOPE)
      return()
    endif ()

  endforeach ()

endfunction()

######################################
### PlasmaAutoPCH(<target> <files> [<file-exclude-regex> ... ])
######################################

function(PlasmaAutoPCH TARGET_NAME FILES)

  foreach(EXCLUDE_PATTERN ${ARGN})

    list(FILTER FILES EXCLUDE REGEX ${EXCLUDE_PATTERN})

  endforeach()

  PlasmaFindPCHInFileList("${FILES}" PCH_NAME)

  if (NOT PCH_NAME)
    return()
  endif()

  PlasmaPCHCreate("${PCH_NAME}.hpp" "${PCH_NAME}.cpp")

  PlasmaPCHUse("${PCH_NAME}.hpp" "${FILES}")

  PlasmaSetTargetPCH(${TARGET_NAME} ${PCH_NAME})
  
endfunction()
