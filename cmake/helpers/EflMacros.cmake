# EFL_OPTION(Name Help Default)
#
# Declare an option() that will be automatically printed by
# EFL_OPTIONS_SUMMARY()
#
# To extend the EFL_OPTIONS_SUMMARY() message, use
# EFL_OPTION_SET_MESSAGE(Name Message)
macro(EFL_OPTION _name _help _defval)
  set(_type ${ARGN})
  if(NOT _type)
    set(_type BOOL)
  endif()
  list(APPEND EFL_ALL_OPTIONS ${_name})
  set(EFL_OPTION_DEFAULT_${_name} "${_defval}")
  set(EFL_OPTION_TYPE_${_name} "${_type}")
  set(${_name} ${_defval} CACHE ${_type} "${_help}")
  option(${_name} "${_help}" ${_defval})
  unset(_type)
endmacro()

# EFL_OPTION_SET_MESSAGE(Name Message)
#
# Extends the summary line output by EFL_OPTIONS_SUMMARY()
# with more details.
macro(EFL_OPTION_SET_MESSAGE _name _message)
  set(EFL_OPTION_MESSAGE_${_name} "${_message}")
endmacro()

# EFL_OPTIONS_SUMMARY()
# Shows the summary of options, their values and related messages.
function(EFL_OPTIONS_SUMMARY)
  message(STATUS "EFL Options Summary:")
  message(STATUS "  CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
  message(STATUS "  CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
  message(STATUS "  BUILD_PROFILE=${BUILD_PROFILE}")
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
    endif()
    message(STATUS "  ${_o}=${_v} (${_i})${_m}")
  endforeach()
endfunction()

set(EFL_ALL_LIBS)
set(EFL_ALL_TESTS)

macro(EFL_FINALIZE)
  add_custom_target(all-libs DEPENDS ${EFL_ALL_LIBS})
  add_custom_target(all-tests DEPENDS ${EFL_ALL_TESTS})
endmacro()

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
macro(EFL_LIB _target)
  string(TOLOWER ${_target} _target_lc)
  set(EFL_LIB_CURRENT ${_target})
  set(EFL_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/${_target_lc})
  set(EFL_LIB_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/lib/${_target_lc})
  set(EFL_BIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/bin/${_target_lc})
  set(EFL_BIN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/bin/${_target_lc})
  set(EFL_MODULES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/${_target_lc})
  set(EFL_MODULES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/modules/${_target_lc})
  set(EFL_TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/${_target_lc})
  set(EFL_TESTS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/tests/${_target_lc})

  set(${_target}_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
  set(${_target}_SOVERSION ${PROJECT_VERSION_MAJOR})
  set(${_target}_LIBRARY_TYPE SHARED)

  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/${_target_lc}.cmake OPTIONAL)
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

  foreach(t ${${_target}_TESTS})
    EFL_TEST(${t})
  endforeach()
  add_custom_target(${_target}-tests DEPENDS ${${_target}_TESTS})
  add_custom_target(${_target}-modules DEPENDS ${${_target}_MODULES})
  list(APPEND EFL_ALL_LIBS ${_target})
  list(APPEND EFL_ALL_TESTS ${_target}-tests)

  unset(_sources)
  unset(_headers)
  unset(_deps)
  unset(EFL_LIB_CURRENT)
  unset(EFL_LIB_SOURCE_DIR)
  unset(EFL_LIB_BINARY_DIR)
  unset(EFL_BIN_SOURCE_DIR)
  unset(EFL_BIN_BINARY_DIR)
  unset(EFL_MODULES_SOURCE_DIR)
  unset(EFL_MODULES_BINARY_DIR)
  unset(EFL_TESTS_SOURCE_DIR)
  unset(EFL_TESTS_BINARY_DIR)
endmacro()


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
  # TODO: depend on modules!
  set_target_properties(${_testname} PROPERTIES OBJECT_DEPENDS "${_deps}")
  message(STATUS "${_testname} ${_deps}")

  target_include_directories(${_testname} SYSTEM PUBLIC
    ${EFL_TESTS_SOURCE_DIR}
    ${EFL_TESTS_BINARY_DIR}
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
