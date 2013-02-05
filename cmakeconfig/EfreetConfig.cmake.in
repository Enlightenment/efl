# - Try to find efreet
# Once done this will define
#  EFREET_FOUND - System has efreet
#  EFREET_INCLUDE_DIRS - The efreet include directories
#  EFREET_LIBRARIES - The libraries needed to use efreet
#  EFREET_DEFINITIONS - Compiler switches required for using efreet

set(MY_PKG efreet)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEFREET ${_QUIET} ${MY_PKG})

find_library(EFREET_LIBRARY
             NAMES ${PC_LIBEFREET_LIBRARIES}
             HINTS ${PC_LIBEFREET_LIBDIR} ${PC_LIBEFREET_LIBRARY_DIRS} )

set(EFREET_DEFINITIONS ${PC_LIBEFREET_CFLAGS_OTHER})
set(EFREET_LIBRARIES ${EFREET_LIBRARY})
set(EFREET_INCLUDE_DIRS ${PC_LIBEFREET_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EFREET_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EFREET_LIBRARIES EFREET_INCLUDE_DIRS)

mark_as_advanced(EFREET_INCLUDE_DIRS EFREET_LIBRARY EFREET_LIBRARIES EFREET_DEFINITIONS)

