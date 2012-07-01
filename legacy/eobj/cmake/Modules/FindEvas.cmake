# - Try to find evas
# Once done this will define
#  EVAS_FOUND - System has evas
#  EVAS_INCLUDE_DIRS - The evas include directories
#  EVAS_LIBRARIES - The libraries needed to use evas
#  EVAS_DEFINITIONS - Compiler switches required for using evas

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEVAS ${_QUIET} evas)
set(EVAS_DEFINITIONS ${PC_LIBEVAS_CFLAGS_OTHER})

find_path(EVAS_INCLUDE_DIR Evas.h
          HINTS ${PC_LIBEVAS_INCLUDEDIR} ${PC_LIBEVAS_INCLUDE_DIRS}
          PATH_SUFFIXES evas )

find_library(EVAS_LIBRARY NAMES evas
             HINTS ${PC_LIBEVAS_LIBDIR} ${PC_LIBEVAS_LIBRARY_DIRS} )

set(EVAS_LIBRARIES ${EVAS_LIBRARY} )
set(EVAS_INCLUDE_DIRS ${EVAS_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EVAS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(evas  DEFAULT_MSG
                                  EVAS_LIBRARY EVAS_INCLUDE_DIR)

mark_as_advanced(EVAS_INCLUDE_DIR EVAS_LIBRARY )
