cmake_minimum_required(VERSION 3.4)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/helpers")

include(GNUInstallDirs)
include(EflMacros)

EFL_PROJECT(1.18.99)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Use: mkdir -p build && cmake -H. -Bbuild")
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}" CACHE PATH "Path to store built libraries")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}" CACHE PATH "Path to store built static libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Path to store built executables")

add_definitions(
  "-DHAVE_CONFIG_H"
  "-DLIB_INSTALL_DIR=\"${LIB_INSTALL_DIR}\""
  "-DEXEC_INSTALL_DIR=\"${EXEC_INSTALL_DIR}\""
  "-DPACKAGE_BIN_DIR=\"${CMAKE_INSTALL_FULL_BINDIR}\""
  "-DPACKAGE_LIB_DIR=\"${CMAKE_INSTALL_FULL_LIBDIR}\""
  "-DPACKAGE_BUILD_DIR=\"${CMAKE_SOURCE_DIR}\""
  "-DPACKAGE_VERSION=\"${PROJECT_VERSION}\""
  )

include_directories(
  ${PROJECT_SOURCE_DIR}/src/lib/efl
  ${PROJECT_BINARY_DIR}/src/lib/efl
  )

include(${CMAKE_SOURCE_DIR}/cmake/config/common.cmake)

EFL_SUPPORT_LIB(lz4)
EFL_SUPPORT_LIB(draw)
EFL_SUPPORT_LIB(freetype)
EFL_SUPPORT_LIB(rg_etc)
EFL_SUPPORT_LIB(triangulator)
EFL_SUPPORT_LIB(scenegraph)

EFL_LIB(eina)
EFL_LIB(eolian)
EFL_LIB(eo)
EFL_LIB(efl)
EFL_LIB(emile)
EFL_LIB(eet)
EFL_LIB(ecore)
EFL_LIB(ecore_con)
EFL_LIB(ecore_ipc)
EFL_LIB(ecore_file)
EFL_LIB(eldbus)
EFL_LIB(embryo)
EFL_LIB(ector)
EFL_LIB(eeze)
EFL_LIB(ecore_input)
EFL_LIB(ecore_x)

EFL_OPTIONS_SUMMARY()

# TODO: when autotools is gone, rename this file in repository
# and remove this copy (using generate to skip @-subst)
file(GENERATE
  OUTPUT ${PROJECT_BINARY_DIR}/src/lib/efl/config.h
  INPUT ${PROJECT_SOURCE_DIR}/src/lib/efl/config.h.cmake)
EFL_HEADER_CHECKS_FINALIZE(${PROJECT_BINARY_DIR}/src/lib/efl/config_gen.h)

configure_file(
  ${PROJECT_SOURCE_DIR}/src/lib/efl/Efl_Config.h.cmake
  ${PROJECT_BINARY_DIR}/src/lib/efl/Efl_Config.h)
EFL_FINALIZE()
