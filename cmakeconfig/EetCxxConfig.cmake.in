# - Try to find eet
# Once done this will define
#  EET_CXX_FOUND - System has eet
#  EET_CXX_INCLUDE_DIRS - The eet include directories
#  EET_CXX_LIBRARIES - The libraries needed to use eet
#  EET_CXX_DEFINITIONS - Compiler switches required for using eet

set(MY_PKG eet_cxx)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_EET_CXX ${_QUIET} ${MY_PKG})
find_library(EET_CXX_LIBRARY
             NAMES ${PC_EET_CXX_LIBRARIES}
             HINTS ${PC_EET_CXX_LIBDIR} ${PC_EET_CXX_LIBRARY_DIRS} )

set(EET_CXX_DEFINITIONS ${PC_EET_CXX_CFLAGS_OTHER})
set(EET_CXX_LIBRARIES ${EET_CXX_LIBRARY})
set(EET_CXX_INCLUDE_DIRS ${PC_EET_CXX_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EET_CXX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EET_CXX_LIBRARIES EET_CXX_INCLUDE_DIRS)

mark_as_advanced(EET_CXX_INCLUDE_DIRS EET_CXX_LIBRARY EET_CXX_LIBRARIES EET_CXX_DEFINITIONS)
