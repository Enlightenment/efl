# - Try to find eolian
# Once done this will define
#  EOLIAN_CXX_FOUND - System has eolian
#  EOLIAN_CXX_INCLUDE_DIRS - The eolian include directories
#  EOLIAN_CXX_LIBRARIES - The libraries needed to use eolian
#  EOLIAN_CXX_DEFINITIONS - Compiler switches required for using eolian

set(MY_PKG eolian_cxx)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_EOLIAN_CXX ${_QUIET} ${MY_PKG})
find_library(EOLIAN_CXX_LIBRARY
             NAMES ${PC_EOLIAN_CXX_LIBRARIES}
             HINTS ${PC_EOLIAN_CXX_LIBDIR} ${PC_EOLIAN_CXX_LIBRARY_DIRS} )

set(EOLIAN_CXX_DEFINITIONS ${PC_EOLIAN_CXX_CFLAGS_OTHER})
set(EOLIAN_CXX_LIBRARIES ${EOLIAN_CXX_LIBRARY})
set(EOLIAN_CXX_INCLUDE_DIRS ${PC_EOLIAN_CXX_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EOLIAN_CXX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EOLIAN_CXX_LIBRARIES EOLIAN_CXX_INCLUDE_DIRS)

mark_as_advanced(EOLIAN_CXX_INCLUDE_DIRS EOLIAN_CXX_LIBRARY EOLIAN_CXX_LIBRARIES EOLIAN_CXX_DEFINITIONS)
