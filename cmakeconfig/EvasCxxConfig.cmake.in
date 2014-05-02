# - Try to find evas
# Once done this will define
#  EVAS_CXX_FOUND - System has evas
#  EVAS_CXX_INCLUDE_DIRS - The evas include directories
#  EVAS_CXX_LIBRARIES - The libraries needed to use evas
#  EVAS_CXX_DEFINITIONS - Compiler switches required for using evas

set(MY_PKG evas_cxx)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_EVAS_CXX ${_QUIET} ${MY_PKG})
find_library(EVAS_CXX_LIBRARY
             NAMES ${PC_EVAS_CXX_LIBRARIES}
             HINTS ${PC_EVAS_CXX_LIBDIR} ${PC_EVAS_CXX_LIBRARY_DIRS} )

set(EVAS_CXX_DEFINITIONS ${PC_EVAS_CXX_CFLAGS_OTHER})
set(EVAS_CXX_LIBRARIES ${EVAS_CXX_LIBRARY})
set(EVAS_CXX_INCLUDE_DIRS ${PC_EVAS_CXX_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EVAS_CXX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EVAS_CXX_LIBRARIES EVAS_CXX_INCLUDE_DIRS)

mark_as_advanced(EVAS_CXX_INCLUDE_DIRS EVAS_CXX_LIBRARY EVAS_CXX_LIBRARIES EVAS_CXX_DEFINITIONS)
