# - Try to find eo
# Once done this will define
#  EO_CXX_FOUND - System has eo
#  EO_CXX_INCLUDE_DIRS - The eo include directories
#  EO_CXX_LIBRARIES - The libraries needed to use eo
#  EO_CXX_DEFINITIONS - Compiler switches required for using eo

set(MY_PKG eo_cxx)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_EO_CXX ${_QUIET} ${MY_PKG})
find_library(EO_CXX_LIBRARY
             NAMES ${PC_EO_CXX_LIBRARIES}
             HINTS ${PC_EO_CXX_LIBDIR} ${PC_EO_CXX_LIBRARY_DIRS} )

set(EO_CXX_DEFINITIONS ${PC_EO_CXX_CFLAGS_OTHER})
set(EO_CXX_LIBRARIES ${EO_CXX_LIBRARY})
set(EO_CXX_INCLUDE_DIRS ${PC_EO_CXX_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EO_CXX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EO_CXX_LIBRARIES EO_CXX_INCLUDE_DIRS)

mark_as_advanced(EO_CXX_INCLUDE_DIRS EO_CXX_LIBRARY EO_CXX_LIBRARIES EO_CXX_DEFINITIONS)
