# - Try to find elua
# Once done this will define
#  ELUA_FOUND - System has elua
#  ELUA_INCLUDE_DIRS - The elua include directories
#  ELUA_LIBRARIES - The libraries needed to use elua
#  ELUA_DEFINITIONS - Compiler switches required for using elua
#  ELUA_EXECUTABLE - full path to the `elua' program.

set(MY_PKG elua)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBELUA ${_QUIET} ${MY_PKG})

find_library(ELUA_LIBRARY
             NAMES ${PC_LIBELUA_LIBRARIES}
             HINTS ${PC_LIBELUA_LIBDIR} ${PC_LIBELUA_LIBRARY_DIRS})

find_program(ELUA_EXECUTABLE
             NAMES elua)

set(ELUA_DEFINITIONS ${PC_LIBELUA_CFLAGS_OTHER})
set(ELUA_LIBRARIES ${ELUA_LIBRARY})
set(ELUA_INCLUDE_DIRS ${PC_LIBELUA_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ELUA_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   ELUA_LIBRARIES ELUA_INCLUDE_DIRS)

mark_as_advanced(ELUA_INCLUDE_DIRS ELUA_LIBRARY ELUA_LIBRARIES ELUA_DEFINITIONS ELUA_EXECUTABLE)

