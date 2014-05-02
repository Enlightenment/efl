# - Try to find eina
# Once done this will define
#  EINA_CXX_FOUND - System has eina
#  EINA_CXX_INCLUDE_DIRS - The eina include directories
#  EINA_CXX_LIBRARIES - The libraries needed to use eina
#  EINA_CXX_DEFINITIONS - Compiler switches required for using eina

set(MY_PKG eina_cxx)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_EINA_CXX ${_QUIET} ${MY_PKG})
find_library(EINA_CXX_LIBRARY
             NAMES ${PC_EINA_CXX_LIBRARIES}
             HINTS ${PC_EINA_CXX_LIBDIR} ${PC_EINA_CXX_LIBRARY_DIRS} )

set(EINA_CXX_DEFINITIONS ${PC_EINA_CXX_CFLAGS_OTHER})
set(EINA_CXX_LIBRARIES ${EINA_CXX_LIBRARY})
set(EINA_CXX_INCLUDE_DIRS ${PC_EINA_CXX_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EINA_CXX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EINA_CXX_LIBRARIES EINA_CXX_INCLUDE_DIRS)

mark_as_advanced(EINA_CXX_INCLUDE_DIRS EINA_CXX_LIBRARY EINA_CXX_LIBRARIES EINA_CXX_DEFINITIONS)
