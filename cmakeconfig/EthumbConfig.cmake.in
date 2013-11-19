# - Try to find ethumb
# Once done this will define
#  ETHUMB_FOUND - System has ethumb
#  ETHUMB_INCLUDE_DIRS - The ethumb include directories
#  ETHUMB_LIBRARIES - The libraries needed to use ethumb
#  ETHUMB_DEFINITIONS - Compiler switches required for using ethumb

set(MY_PKG ethumb)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBETHUMB ${_QUIET} ${MY_PKG})

find_library(ETHUMB_LIBRARY
             NAMES ${PC_LIBETHUMB_LIBRARIES}
             HINTS ${PC_LIBETHUMB_LIBDIR} ${PC_LIBETHUMB_LIBRARY_DIRS} )

set(ETHUMB_DEFINITIONS ${PC_LIBETHUMB_CFLAGS_OTHER})
set(ETHUMB_LIBRARIES ${ETHUMB_LIBRARY})
set(ETHUMB_INCLUDE_DIRS ${PC_LIBETHUMB_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ETHUMB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   ETHUMB_LIBRARIES ETHUMB_INCLUDE_DIRS)

mark_as_advanced(ETHUMB_INCLUDE_DIRS ETHUMB_LIBRARY ETHUMB_LIBRARIES ETHUMB_DEFINITIONS)

