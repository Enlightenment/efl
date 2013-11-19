# - Try to find eldbus
# Once done this will define
#  ELDBUS_FOUND - System has eldbus
#  ELDBUS_INCLUDE_DIRS - The eldbus include directories
#  ELDBUS_LIBRARIES - The libraries needed to use eldbus
#  ELDBUS_DEFINITIONS - Compiler switches required for using eldbus

set(MY_PKG eldbus)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBELDBUS ${_QUIET} ${MY_PKG})

find_library(ELDBUS_LIBRARY
             NAMES ${PC_LIBELDBUS_LIBRARIES}
             HINTS ${PC_LIBELDBUS_LIBDIR} ${PC_LIBELDBUS_LIBRARY_DIRS} )

set(ELDBUS_DEFINITIONS ${PC_LIBELDBUS_CFLAGS_OTHER})
set(ELDBUS_LIBRARIES ${ELDBUS_LIBRARY})
set(ELDBUS_INCLUDE_DIRS ${PC_LIBELDBUS_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ELDBUS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   ELDBUS_LIBRARIES ELDBUS_INCLUDE_DIRS)

mark_as_advanced(ELDBUS_INCLUDE_DIRS ELDBUS_LIBRARY ELDBUS_LIBRARIES ELDBUS_DEFINITIONS)

