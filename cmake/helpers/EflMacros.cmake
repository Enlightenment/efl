# NOTE:
#
# - Use function() whenever possible, macro() doesn't start a new
#   variable scope and needs careful handling to avoid left overs.
#
# - To 'export' values or lists, use the macros SET_GLOBAL() and
#   LIST_APPEND_GLOBAL().
#

# LIST_APPEND_GLOBAL(Var Element)
#
# Append to a list in the global variable scope (cache internal)
function(LIST_APPEND_GLOBAL _var _element)
  set(_local ${${_var}})
  list(APPEND _local ${_element})
  set(${_var} ${_local} CACHE INTERNAL "")
endfunction()

# SET_GLOBAL(Var Value [Help])
#
# Set a variable in the global variable scope (cache internal)
function(SET_GLOBAL _var _value)
  set(${_var} "${_value}" CACHE INTERNAL "${ARGN}")
endfunction()

# EFL_OPTION(Name Help Default)
#
# Declare an option() that will be automatically printed by
# EFL_OPTIONS_SUMMARY()
#
# To extend the EFL_OPTIONS_SUMMARY() message, use
# EFL_OPTION_SET_MESSAGE(Name Message)
function(EFL_OPTION _name _help _defval)
  set(_type)
  set(_vartype)
  set(_choices)
  list(LENGTH ARGN _argc)
  if(_argc LESS 1)
    set(_type BOOL)
    set(_vartype BOOL)
  else()
    list(GET ARGN 0 _type)
    set(_vartype ${_type})
    list(REMOVE_AT ARGN 0)
  endif()
  if(${_vartype} STREQUAL "CHOICE")
    set(_type STRING)
    SET_GLOBAL(EFL_OPTION_CHOICES_${_name} "${ARGN}" "Possible values for ${_name}")
    set(_choices " (Choices: ${ARGN})")
  endif()

  LIST_APPEND_GLOBAL(EFL_ALL_OPTIONS ${_name})

  SET_GLOBAL(EFL_OPTION_DEFAULT_${_name} "${_defval}" "Default value for ${_name}")
  SET_GLOBAL(EFL_OPTION_TYPE_${_name} "${_vartype}" "Type of ${_name}")
  set(${_name} ${_defval} CACHE ${_type} "${_help}${_choices}")
  option(${_name} "${_help}${_choices}" ${_defval})

  if(_choices)
    list(FIND ARGN "${${_name}}" _ret)
    if(${_ret} EQUAL -1)
      message(FATAL_ERROR "Invalid choice ${_name}=${${_name}}${_choices}")
    endif()
  endif()
endfunction()

# EFL_OPTION_SET_MESSAGE(Name Message)
#
# Extends the summary line output by EFL_OPTIONS_SUMMARY()
# with more details.
function(EFL_OPTION_SET_MESSAGE _name _message)
  SET_GLOBAL(EFL_OPTION_MESSAGE_${_name} "${_message}")
endfunction()

# EFL_OPTIONS_SUMMARY()
# Shows the summary of options, their values and related messages.
function(EFL_OPTIONS_SUMMARY)
  message(STATUS "EFL Options Summary:")
  message(STATUS "  CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
  message(STATUS "  CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
  foreach(_o ${EFL_ALL_OPTIONS})
    set(_v ${${_o}})
    set(_d ${EFL_OPTION_DEFAULT_${_o}})
    if("${_v}" STREQUAL "${_d}")
      set(_i "default")
    else()
      set(_i "default: ${_d}")
    endif()
    if(EFL_OPTION_MESSAGE_${_o})
      set(_m " [${EFL_OPTION_MESSAGE_${_o}}]")
    else()
      set(_m)
    endif()
    message(STATUS "  ${_o}=${_v} (${_i})${_m}")
  endforeach()
  message(STATUS "EFL Libraries:")
  foreach(_o ${EFL_ALL_LIBS})
    message(STATUS "  ${_o}${_mods}")
    foreach(_m ${${_o}_MODULES})
      string(REGEX REPLACE "^${_o}-module-" "" _m ${_m})
      message(STATUS "    dynamic: ${_m}")
    endforeach()
    foreach(_m ${${_o}_STATIC_MODULES})
      string(REGEX REPLACE "^${_o}-module-" "" _m ${_m})
      message(STATUS "    static.: ${_m}")
    endforeach()
    unset(_m)
  endforeach()
endfunction()

set(EFL_ALL_LIBS)
set(EFL_ALL_TESTS)

# EFL_FINALIZE()
#
# Finalize EFL processing, adding extra targets.
function(EFL_FINALIZE)
  add_custom_target(all-libs DEPENDS ${EFL_ALL_LIBS})
  add_custom_target(all-tests DEPENDS ${EFL_ALL_TESTS})
endfunction()

# EFL_LIB(Name)
#
# adds a library ${Name} automatically setting:
#  - target_include_directories to ${Name}_INCLUDE_DIRECTORIES
#  - target_include_directories(SYSTEM) to ${Name}_SYSTEM_INCLUDE_DIRECTORIES
#  - OUTPUT_NAME to ${Name}_OUTPUT_NAME
#  - SOURCES to ${Name}_SOURCES
#  - HEADER to ${Name}_HEADERS (to be installed)
#  - VERSION to ${Name}_VERSION (defaults to project version)
#  - SOVERSION to ${Name}_SOURCES (defaults to project major version)
#  - OBJECT_DEPENDS to ${Name}_DEPENDENCIES
#  - target_link_libraries() to ${Name}_LIBRARIES
#  - target_compile_definitions() to ${Name}_DEFINITIONS
#  - compile tests in ${Name}_TESTS using EFL_TEST()
#
function(EFL_LIB _target)
  set(EFL_LIB_CURRENT ${_target})
  set(EFL_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/${_target})
  set(EFL_LIB_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/lib/${_target})
  set(EFL_BIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/bin/${_target})
  set(EFL_BIN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/bin/${_target})
  set(EFL_MODULES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/${_target})
  set(EFL_MODULES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/modules/${_target})
  set(EFL_TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/${_target})
  set(EFL_TESTS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/tests/${_target})

  set(${_target}_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
  set(${_target}_SOVERSION ${PROJECT_VERSION_MAJOR})
  set(${_target}_LIBRARY_TYPE SHARED)

  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/${_target}.cmake OPTIONAL)
  include(${EFL_LIB_SOURCE_DIR}/CMakeLists.txt OPTIONAL)
  include(${EFL_BIN_SOURCE_DIR}/CMakeLists.txt OPTIONAL)
  include(${EFL_MODULES_SOURCE_DIR}/CMakeLists.txt OPTIONAL)
  include(${EFL_TESTS_SOURCE_DIR}/CMakeLists.txt OPTIONAL)

  set(_headers "")
  foreach(f ${${_target}_HEADERS})
    if(EXISTS "${EFL_LIB_SOURCE_DIR}/${f}")
      list(APPEND _headers "${EFL_LIB_SOURCE_DIR}/${f}")
    else()
      list(APPEND _headers "${EFL_LIB_BINARY_DIR}/${f}")
    endif()
  endforeach()

  set(_sources "")
  foreach(f ${${_target}_SOURCES})
    if(EXISTS "${EFL_LIB_SOURCE_DIR}/${f}")
      list(APPEND _sources "${EFL_LIB_SOURCE_DIR}/${f}")
    else()
      list(APPEND _sources "${EFL_LIB_BINARY_DIR}/${f}")
    endif()
  endforeach()

  set(_deps "")
  foreach(f ${${_target}_DEPENDENCIES})
    if(EXISTS "${EFL_LIB_SOURCE_DIR}/${f}")
      list(APPEND _deps "${EFL_LIB_SOURCE_DIR}/${f}")
    else()
      list(APPEND _deps "${EFL_LIB_BINARY_DIR}/${f}")
    endif()
  endforeach()

  add_library(${_target} ${${_target}_LIBRARY_TYPE} ${_sources} ${_headers})
  set_target_properties(${_target} PROPERTIES
    FRAMEWORK TRUE
    PUBLIC_HEADER "${_headers}"
    OBJECT_DEPENDS "${_deps}")

  if(${_target}_LIBRARIES)
    target_link_libraries(${_target} ${${_target}_LIBRARIES})
  endif()

  target_include_directories(${_target} PUBLIC
    ${${_target}_INCLUDE_DIRECTORIES}
    ${EFL_LIB_SOURCE_DIR}
    )
  if(${_target}_SYSTEM_INCLUDE_DIRECTORIES)
    target_include_directories(${_target} SYSTEM PUBLIC ${${_target}_SYSTEM_INCLUDE_DIRECTORIES})
  endif()

  if(${_target}_DEFINITIONS)
    target_compile_definitions(${_target} PRIVATE ${${_target}_DEFINITIONS})
  endif()

  if(${_target}_OUTPUT_NAME)
    set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${${_target}_OUTPUT_NAME})
  endif()

  if(${_target}_VERSION AND ${_target}_SOVERSION)
    set_target_properties(${_target} PROPERTIES
      VERSION ${${_target}_VERSION}
      SOVERSION ${${_target}_SOVERSION})
  endif()

  install(TARGETS ${_target}
    PUBLIC_HEADER DESTINATION include/${_target}-${PROJECT_VERSION_MAJOR}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

  file(GLOB modules RELATIVE ${EFL_MODULES_SOURCE_DIR} ${EFL_MODULES_SOURCE_DIR}/*)
  foreach(module ${modules})
    if(IS_DIRECTORY ${EFL_MODULES_SOURCE_DIR}/${module})
      set(EFL_MODULE_SCOPE ${module})

      include(${EFL_MODULES_SOURCE_DIR}/${module}/CMakeLists.txt OPTIONAL)

      file(GLOB submodules RELATIVE ${EFL_MODULES_SOURCE_DIR}/${EFL_MODULE_SCOPE} ${EFL_MODULES_SOURCE_DIR}/${EFL_MODULE_SCOPE}/*)
      foreach(submodule ${submodules})
        if(IS_DIRECTORY ${EFL_MODULES_SOURCE_DIR}/${EFL_MODULE_SCOPE}/${submodule})
          EFL_MODULE(${submodule})
        endif()
        unset(submodule)
        unset(submodules)
      endforeach()
    else()
      set(EFL_MODULE_SCOPE)
      EFL_MODULE(${module})
    endif()
    unset(EFL_MODULE_SCOPE)
  endforeach()
  unset(module)
  unset(modules)

  foreach(t ${${_target}_TESTS})
    EFL_TEST(${t})
  endforeach()
  unset(t)
  add_custom_target(${_target}-tests DEPENDS ${${_target}_TESTS})
  add_custom_target(${_target}-modules DEPENDS ${${_target}_MODULES})
  LIST_APPEND_GLOBAL(EFL_ALL_LIBS ${_target})
  LIST_APPEND_GLOBAL(EFL_ALL_TESTS ${_target}-tests)
endfunction()


# EFL_TEST(Name)
#
# Adds a test for ${EFL_LIB_CURRENT} using
# ${EFL_TESTS_SOURCE_DIR} and ${EFL_TESTS_BINARY_DIR}
#
# Settings:
#  - include_directories to ${Name}_INCLUDE_DIRECTORIES
#  - include_directories(SYSTEM) to ${Name}_SYSTEM_INCLUDE_DIRECTORIES
#  - OUTPUT_NAME to ${Name}_OUTPUT_NAME
#  - SOURCES to ${Name}_SOURCES
#  - target_link_libraries() to ${Name}_LIBRARIES (${EFL_LIB_CURRENT}
#    is automatic)
#  - target_compile_definitions() to ${Name}_DEFINITIONS
#
# NOTE: it's meant to be called by files included by EFL_LIB() or similar,
# otherwise you need to prepare the environment yourself.
function(EFL_TEST _testname)
  if(NOT CHECK_FOUND)
    message(STATUS "${EFL_LIB_CURRENT} test ${_testname} ignored since no 'check' library was found.")
    return()
  endif()
  set(_sources "")
  foreach(f ${${_testname}_SOURCES})
    if(EXISTS "${EFL_TESTS_SOURCE_DIR}/${f}")
      list(APPEND _sources "${EFL_TESTS_SOURCE_DIR}/${f}")
    else()
      list(APPEND _sources "${EFL_TESTS_BINARY_DIR}/${f}")
    endif()
  endforeach()
  add_executable(${_testname} EXCLUDE_FROM_ALL ${_sources})

  set(_deps "")
  foreach(f ${${_testname}_DEPENDENCIES})
    if(EXISTS "${EFL_TESTS_SOURCE_DIR}/${f}")
      list(APPEND _deps "${EFL_TESTS_SOURCE_DIR}/${f}")
    else()
      list(APPEND _deps "${EFL_TESTS_BINARY_DIR}/${f}")
    endif()
  endforeach()
  add_dependencies(${_testname} ${EFL_LIB_CURRENT}-modules)
  set_target_properties(${_testname} PROPERTIES OBJECT_DEPENDS "${_deps}")

  target_include_directories(${_testname} PRIVATE
    ${EFL_TESTS_SOURCE_DIR}
    ${EFL_TESTS_BINARY_DIR}
    ${${_testname}_INCLUDE_DIRECTORIES})
  target_include_directories(${_testname} SYSTEM PRIVATE
    ${${_testname}_SYSTEM_INCLUDE_DIRECTORIES}
    ${CHECK_INCLUDE_DIRS})
  target_link_libraries(${_testname}
    ${EFL_LIB_CURRENT}
    ${${_testname}_LIBRARIES}
    ${CHECK_LIBRARIES})

  target_compile_definitions(${_testname} PRIVATE
    "-DTESTS_SRC_DIR=\"${EFL_TESTS_SOURCE_DIR}\""
    "-DTESTS_BUILD_DIR=\"${EFL_TESTS_BINARY_DIR}\""
    "-DTESTS_WD=\"${PROJECT_BINARY_DIR}\""
    "-DPACKAGE_BUILD_DIR=\"1\""
    ${${_testname}_DEFINITIONS}
    )

  if(${_testname}_OUTPUT_NAME)
    set_target_properties(${_testname} PROPERTIES OUTPUT_NAME ${${_testname}_OUTPUT_NAME})
  endif()

  set_target_properties(${_testname} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${EFL_TESTS_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${EFL_TESTS_BINARY_DIR}")

  add_test(NAME ${_testname} COMMAND ${_testname})
endfunction()

# EFL_MODULE(Name)
#
# Adds a module for ${EFL_LIB_CURRENT} using
# ${EFL_MODULES_SOURCE_DIR} and ${EFL_MODULES_BINARY_DIR}
# as well as ${EFL_MODULE_SCOPE} if it's contained into
# a subdir, such as eina's "mp" or evas "engines".
#
# To keep it simple to use, user is only expected to define variables:
#  - SOURCES
#  - DEPENDENCIES
#  - LIBRARIES
#  - INCLUDE_DIRECTORIES
#  - SYSTEM_INCLUDE_DIRECTORIES
#  - DEFINITIONS
#
# NOTE: since the file will be included it shouldn't mess with global variables!
function(EFL_MODULE _modname)
  if(EFL_MODULE_SCOPE)
    set(_modsrcdir ${EFL_MODULES_SOURCE_DIR}/${EFL_MODULE_SCOPE}/${_modname})
  else()
    set(_modsrcdir ${EFL_MODULES_SOURCE_DIR}/${_modname})
  endif()

  set(SOURCES)
  set(DEPENDENCIES)
  set(LIBRARIES)
  set(INCLUDE_DIRECTORIES)
  set(SYSTEM_INCLUDE_DIRECTORIES)
  set(DEFINITIONS)
  set(MODULE_TYPE "ON")

  include(${_modsrcdir}/CMakeLists.txt)

  if(NOT SOURCES)
    message(WARNING "${_modsrcdir}/CMakeLists.txt defines no SOURCES")
    return()
  endif()

  if(EFL_MODULE_SCOPE)
    set(_modbindir ${EFL_MODULES_BINARY_DIR}/${EFL_MODULE_SCOPE}/${_modname})
    set(_modtarget ${EFL_LIB_CURRENT}-module-${EFL_MODULE_SCOPE}-${_modname})
    set(_modoutdir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${EFL_LIB_CURRENT}/modules/${EFL_MODULE_SCOPE}/${_modname}/v-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
  else()
    set(_modbindir ${EFL_MODULES_BINARY_DIR}/${_modname})
    set(_modtarget ${EFL_LIB_CURRENT}-module-${_modname})
    set(_modoutdir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${EFL_LIB_CURRENT}/modules/${_modname}/v-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
  endif()

  if("${MODULE_TYPE}" STREQUAL "OFF")
    message(WARNING "${_modsrcdir} disabled")
    return()
  elseif("${MODULE_TYPE}" STREQUAL "STATIC")
    set(_modtype STATIC)
  else()
    set(_modtype MODULE)
  endif()

  set(_sources "")
  foreach(f ${SOURCES})
    if(EXISTS "${_modsrcdir}/${f}")
      list(APPEND _sources "${_modsrcdir}/${f}")
    else()
      list(APPEND _sources "${_modbindir}/${f}")
    endif()
  endforeach()

  set(_deps "")
  foreach(f ${DEPENDENCIES})
    if(EXISTS "${_modsrcdir}/${f}")
      list(APPEND _deps "${_modsrcdir}/${f}")
    else()
      list(APPEND _deps "${_modbindir}/${f}")
    endif()
  endforeach()

  add_library(${_modtarget} ${_modtype} ${_sources})
  set_target_properties(${_modtarget} PROPERTIES
    OBJECT_DEPENDS "${_deps}"
    PREFIX ""
    OUTPUT_NAME "module")

  target_include_directories(${_modtarget} PRIVATE
    ${_modsrcdir}
    ${_modbindir}
    ${INCLUDE_DIRECTORIES})
  target_include_directories(${_modtarget} SYSTEM PUBLIC
    ${SYSTEM_INCLUDE_DIRECTORIES})
  target_link_libraries(${_modtarget} ${LIBRARIES})

  target_compile_definitions(${_modtarget} PRIVATE ${DEFINITIONS})

  set_target_properties(${_modtarget} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${_modoutdir}"
    ARCHIVE_OUTPUT_DIRECTORY "${_modoutdir}"
    RUNTIME_OUTPUT_DIRECTORY "${_modoutdir}")

  if("${MODULE_TYPE}" STREQUAL "STATIC")
    target_link_libraries(${EFL_LIB_CURRENT} ${_modtarget})
    target_include_directories(${_modtarget} PRIVATE
      ${EFL_LIB_SOURCE_DIR}
      ${EFL_LIB_BINARY_DIR})
    set_target_properties(${_modtarget} PROPERTIES
      POSITION_INDEPENDENT_CODE TRUE)

    LIST_APPEND_GLOBAL(${EFL_LIB_CURRENT}_STATIC_MODULES ${_modtarget})
  else()
    target_link_libraries(${_modtarget} ${EFL_LIB_CURRENT})
    LIST_APPEND_GLOBAL(${EFL_LIB_CURRENT}_MODULES ${_modtarget})
  endif()
endfunction()
