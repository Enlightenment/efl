# - Try to find eet
# Once done this will define
#  EET_FOUND - System has eet
#  EET_INCLUDE_DIRS - The eet include directories
#  EET_LIBRARIES - The libraries needed to use eet
#  EET_DEFINITIONS - Compiler switches required for using eet

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEET ${_QUIET} eet)
set(EET_DEFINITIONS ${PC_LIBEET_CFLAGS_OTHER})

find_path(EET_INCLUDE_DIR Eet.h
          HINTS ${PC_LIBEET_INCLUDEDIR} ${PC_LIBEET_INCLUDE_DIRS}
          PATH_SUFFIXES eet )

find_library(EET_LIBRARY NAMES eet
             HINTS ${PC_LIBEET_LIBDIR} ${PC_LIBEET_LIBRARY_DIRS} )

set(EET_LIBRARIES ${EET_LIBRARY} )
set(EET_INCLUDE_DIRS ${EET_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EET_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(eet  DEFAULT_MSG
                                  EET_LIBRARY EET_INCLUDE_DIR)

mark_as_advanced( EET_INCLUDE_DIR EET_LIBRARY )
