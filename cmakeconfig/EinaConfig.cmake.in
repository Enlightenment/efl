# - Try to find eina
# Once done this will define
#  EINA_FOUND - System has eina
#  EINA_INCLUDE_DIRS - The eina include directories
#  EINA_LIBRARIES - The libraries needed to use eina
#  EINA_DEFINITIONS - Compiler switches required for using eina

set(MY_PKG eina)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEINA ${_QUIET} ${MY_PKG})

find_library(EINA_LIBRARY
             NAMES ${PC_LIBEINA_LIBRARIES}
             HINTS ${PC_LIBEINA_LIBDIR} ${PC_LIBEINA_LIBRARY_DIRS} )

set(EINA_DEFINITIONS ${PC_LIBEINA_CFLAGS_OTHER})
set(EINA_LIBRARIES ${EINA_LIBRARY})
set(EINA_INCLUDE_DIRS ${PC_LIBEINA_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EINA_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EINA_LIBRARIES EINA_INCLUDE_DIRS)

mark_as_advanced(EINA_INCLUDE_DIRS EINA_LIBRARY EINA_LIBRARIES EINA_DEFINITIONS)

