# - Try to find efl
# Once done this will define
#  EFL_FOUND - System has efl
#  EFL_INCLUDE_DIRS - The efl include directories
#  EFL_LIBRARIES - The libraries needed to use efl
#  EFL_DEFINITIONS - Compiler switches required for using efl

set(MY_PKG efl)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEFL ${_QUIET} ${MY_PKG})

find_library(EFL_LIBRARY
             NAMES ${PC_LIBEFL_LIBRARIES}
             HINTS ${PC_LIBEFL_LIBDIR} ${PC_LIBEFL_LIBRARY_DIRS} )

set(EFL_DEFINITIONS ${PC_LIBEFL_CFLAGS_OTHER})
set(EFL_LIBRARIES ${EFL_LIBRARY})
set(EFL_INCLUDE_DIRS ${PC_LIBEFL_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EFL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EFL_LIBRARIES EFL_INCLUDE_DIRS)

mark_as_advanced(EFL_INCLUDE_DIRS EFL_LIBRARY EFL_LIBRARIES EFL_DEFINITIONS)

