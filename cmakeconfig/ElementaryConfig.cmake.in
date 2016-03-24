# - Try to find elementary
# Once done this will define
#  ELEMENTARY_FOUND - System has elementary
#  ELEMENTARY_INCLUDE_DIRS - The elementary include directories
#  ELEMENTARY_LIBRARIES - The libraries needed to use elementary
#  ELEMENTARY_DEFINITIONS - Compiler switches required for using elementary

set(MY_PKG elementary)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBELEMENTARY ${_QUIET} ${MY_PKG})

find_library(ELEMENTARY_LIBRARY
             NAMES ${PC_LIBELEMENTARY_LIBRARIES}
             HINTS ${PC_LIBELEMENTARY_LIBDIR} ${PC_LIBELEMENTARY_LIBRARY_DIRS} )

set(ELEMENTARY_DEFINITIONS ${PC_LIBELEMENTARY_CFLAGS_OTHER})
set(ELEMENTARY_LIBRARIES ${ELEMENTARY_LIBRARY})
set(ELEMENTARY_INCLUDE_DIRS ${PC_LIBELEMENTARY_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ELEMENTARY_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   ELEMENTARY_LIBRARIES ELEMENTARY_INCLUDE_DIRS)

mark_as_advanced(ELEMENTARY_INCLUDE_DIRS ELEMENTARY_LIBRARY ELEMENTARY_LIBRARIES ELEMENTARY_DEFINITIONS)

