cmake_minimum_required(VERSION 3.7)
project(efl VERSION 1.19.99.1)
if (NOT EFL_BUILD_ID)
  set(EFL_BUILD_ID 120)
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Use: mkdir -p build && cmake -H. -Bbuild")
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "Path to store built libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Path to store built executables")

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Absolute path to library installation directory")
set(EXEC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Absolute path to executable installation directory")
set(HEADER_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Absolute path to install headers by the library")

add_definitions(
  "-DHAVE_CONFIG_H"
  "-DLIB_INSTALL_DIR=\"${LIB_INSTALL_DIR}\""
  "-DEXEC_INSTALL_DIR=\"${EXEC_INSTALL_DIR}\""
  )
include_directories(
  ${PROJECT_SOURCE_DIR}/src/lib/efl
  ${PROJECT_BINARY_DIR}/src/lib/efl
  )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/helpers")
include(EflMacros)

include(${CMAKE_SOURCE_DIR}/cmake/config/common.cmake)

EFL_LIB(eina)

EFL_OPTIONS_SUMMARY()

configure_file(
  ${PROJECT_SOURCE_DIR}/src/lib/efl/config.h.cmake
  ${PROJECT_BINARY_DIR}/src/lib/efl/config.h)
configure_file(
  ${PROJECT_SOURCE_DIR}/src/lib/efl/Efl_Config.h.cmake
  ${PROJECT_BINARY_DIR}/src/lib/efl/Efl_Config.h)

EFL_FINALIZE()
