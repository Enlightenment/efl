# - Try to find ethumb_client
# Once done this will define
#  ETHUMB_CLIENT_FOUND - System has ethumb_client
#  ETHUMB_CLIENT_INCLUDE_DIRS - The ethumb_client include directories
#  ETHUMB_CLIENT_LIBRARIES - The libraries needed to use ethumb_client
#  ETHUMB_CLIENT_DEFINITIONS - Compiler switches required for using ethumb_client

set(MY_PKG ethumb_client)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBETHUMB_CLIENT ${_QUIET} ${MY_PKG})

find_library(ETHUMB_CLIENT_LIBRARY
             NAMES ${PC_LIBETHUMB_CLIENT_LIBRARIES}
             HINTS ${PC_LIBETHUMB_CLIENT_LIBDIR} ${PC_LIBETHUMB_CLIENT_LIBRARY_DIRS} )

set(ETHUMB_CLIENT_DEFINITIONS ${PC_LIBETHUMB_CLIENT_CFLAGS_OTHER})
set(ETHUMB_CLIENT_LIBRARIES ${ETHUMB_CLIENT_LIBRARY})
set(ETHUMB_CLIENT_INCLUDE_DIRS ${PC_LIBETHUMB_CLIENT_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ETHUMB_CLIENT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   ETHUMB_CLIENT_LIBRARIES ETHUMB_CLIENT_INCLUDE_DIRS)

mark_as_advanced(ETHUMB_CLIENT_INCLUDE_DIRS ETHUMB_CLIENT_LIBRARY ETHUMB_CLIENT_LIBRARIES ETHUMB_CLIENT_DEFINITIONS)

