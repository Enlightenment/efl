# - Try to find evas
# Once done this will define
#  EVAS_FOUND - System has evas
#  EVAS_INCLUDE_DIRS - The evas include directories
#  EVAS_LIBRARIES - The libraries needed to use evas
#  EVAS_DEFINITIONS - Compiler switches required for using evas

set(MY_PKG evas)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEVAS ${_QUIET} ${MY_PKG})

find_library(EVAS_LIBRARY
             NAMES ${PC_LIBEVAS_LIBRARIES}
             HINTS ${PC_LIBEVAS_LIBDIR} ${PC_LIBEVAS_LIBRARY_DIRS} )

set(EVAS_DEFINITIONS ${PC_LIBEVAS_CFLAGS_OTHER})
set(EVAS_LIBRARIES ${EVAS_LIBRARY})
set(EVAS_INCLUDE_DIRS ${PC_LIBEVAS_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EVAS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EVAS_LIBRARIES EVAS_INCLUDE_DIRS)

mark_as_advanced(EVAS_INCLUDE_DIRS EVAS_LIBRARY EVAS_LIBRARIES EVAS_DEFINITIONS)

