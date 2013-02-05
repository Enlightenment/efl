# - Try to find eo
# Once done this will define
#  EO_FOUND - System has eo
#  EO_INCLUDE_DIRS - The eo include directories
#  EO_LIBRARIES - The libraries needed to use eo
#  EO_DEFINITIONS - Compiler switches required for using eo

set(MY_PKG eo)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEO ${_QUIET} ${MY_PKG})

find_library(EO_LIBRARY
             NAMES ${PC_LIBEO_LIBRARIES}
             HINTS ${PC_LIBEO_LIBDIR} ${PC_LIBEO_LIBRARY_DIRS} )

set(EO_DEFINITIONS ${PC_LIBEO_CFLAGS_OTHER})
set(EO_LIBRARIES ${EO_LIBRARY})
set(EO_INCLUDE_DIRS ${PC_LIBEO_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EO_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EO_LIBRARIES EO_INCLUDE_DIRS)

mark_as_advanced(EO_INCLUDE_DIRS EO_LIBRARY EO_LIBRARIES EO_DEFINITIONS)

