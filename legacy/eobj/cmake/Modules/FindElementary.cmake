# - Try to find elementary
# Once done this will define
#  ELEMENTARY_FOUND - System has elementary
#  ELEMENTARY_INCLUDE_DIRS - The elementary include directories
#  ELEMENTARY_LIBRARIES - The libraries needed to use elementary
#  ELEMENTARY_DEFINITIONS - Compiler switches required for using elementary

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBELEMENTARY ${_QUIET} elementary)
set(ELEMENTARY_DEFINITIONS ${PC_LIBELEMENTARY_CFLAGS_OTHER})

find_path(ELEMENTARY_INCLUDE_DIR Elementary.h
          HINTS ${PC_LIBELEMENTARY_INCLUDEDIR} ${PC_LIBELEMENTARY_INCLUDE_DIRS}
          PATH_SUFFIXES elementary )

find_library(ELEMENTARY_LIBRARY NAMES elementary
             HINTS ${PC_LIBELEMENTARY_LIBDIR} ${PC_LIBELEMENTARY_LIBRARY_DIRS} )

set(ELEMENTARY_LIBRARIES ${ELEMENTARY_LIBRARY} )
set(ELEMENTARY_INCLUDE_DIRS ${PC_LIBELEMENTARY_INCLUDEDIR} ${PC_LIBELEMENTARY_INCLUDE_DIRS} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ELEMENTARY_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(elementary  DEFAULT_MSG
                                  ELEMENTARY_LIBRARY ELEMENTARY_INCLUDE_DIR)

mark_as_advanced(ELEMENTARY_INCLUDE_DIR ELEMENTARY_LIBRARY )
