# - Try to find eeze
# Once done this will define
#  EEZE_FOUND - System has eeze
#  EEZE_INCLUDE_DIRS - The eeze include directories
#  EEZE_LIBRARIES - The libraries needed to use eeze
#  EEZE_DEFINITIONS - Compiler switches required for using eeze

set(MY_PKG eeze)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEEZE ${_QUIET} ${MY_PKG})

find_library(EEZE_LIBRARY
             NAMES ${PC_LIBEEZE_LIBRARIES}
             HINTS ${PC_LIBEEZE_LIBDIR} ${PC_LIBEEZE_LIBRARY_DIRS} )

set(EEZE_DEFINITIONS ${PC_LIBEEZE_CFLAGS_OTHER})
set(EEZE_LIBRARIES ${EEZE_LIBRARY})
set(EEZE_INCLUDE_DIRS ${PC_LIBEEZE_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EEZE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EEZE_LIBRARIES EEZE_INCLUDE_DIRS)

mark_as_advanced(EEZE_INCLUDE_DIRS EEZE_LIBRARY EEZE_LIBRARIES EEZE_DEFINITIONS)

