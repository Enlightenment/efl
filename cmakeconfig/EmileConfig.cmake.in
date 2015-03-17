# - Try to find emile
# Once done this will define
#  EMILE_FOUND - System has emile
#  EMILE_INCLUDE_DIRS - The emile include directories
#  EMILE_LIBRARIES - The libraries needed to use emile
#  EMILE_DEFINITIONS - Compiler switches required for using emile

set(MY_PKG emile)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEMILE ${_QUIET} ${MY_PKG})

find_library(EMILE_LIBRARY
             NAMES ${PC_LIBEMILE_LIBRARIES}
             HINTS ${PC_LIBEMILE_LIBDIR} ${PC_LIBEMILE_LIBRARY_DIRS} )

set(EMILE_DEFINITIONS ${PC_LIBEMILE_CFLAGS_OTHER})
set(EMILE_LIBRARIES ${EMILE_LIBRARY})
set(EMILE_INCLUDE_DIRS ${PC_LIBEMILE_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EMILE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EMILE_LIBRARIES EMILE_INCLUDE_DIRS)

mark_as_advanced(EMILE_INCLUDE_DIRS EMILE_LIBRARY EMILE_LIBRARIES EMILE_DEFINITIONS)

