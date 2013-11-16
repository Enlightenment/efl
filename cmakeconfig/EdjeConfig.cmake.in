# - Try to find edje
# Once done this will define
#  EDJE_FOUND - System has edje
#  EDJE_INCLUDE_DIRS - The edje include directories
#  EDJE_LIBRARIES - The libraries needed to use edje
#  EDJE_DEFINITIONS - Compiler switches required for using edje
#  EDJE_CC_EXECUTABLE - full path to the `edje_cc' program.

set(MY_PKG edje)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEDJE ${_QUIET} ${MY_PKG})

find_library(EDJE_LIBRARY
             NAMES ${PC_LIBEDJE_LIBRARIES}
             HINTS ${PC_LIBEDJE_LIBDIR} ${PC_LIBEDJE_LIBRARY_DIRS})

find_program(EDJE_CC_EXECUTABLE
             NAMES edje_cc)

set(EDJE_DEFINITIONS ${PC_LIBEDJE_CFLAGS_OTHER})
set(EDJE_LIBRARIES ${EDJE_LIBRARY})
set(EDJE_INCLUDE_DIRS ${PC_LIBEDJE_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EDJE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EDJE_LIBRARIES EDJE_INCLUDE_DIRS)

mark_as_advanced(EDJE_INCLUDE_DIRS EDJE_LIBRARY EDJE_LIBRARIES EDJE_DEFINITIONS EDJE_CC_EXECUTABLE)

