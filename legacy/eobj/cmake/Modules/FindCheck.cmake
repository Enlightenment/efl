# - Try to find check
# Once done this will define
#  CHECK_FOUND - System has check
#  CHECK_INCLUDE_DIRS - The check include directories
#  CHECK_LIBRARIES - The libraries needed to use check
#  CHECK_DEFINITIONS - Compiler switches required for using check

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBCHECK ${_QUIET} check)
set(CHECK_DEFINITIONS ${PC_LIBCHECK_CFLAGS_OTHER})

find_path(CHECK_INCLUDE_DIR check.h
          HINTS ${PC_LIBCHECK_INCLUDEDIR} ${PC_LIBCHECK_INCLUDE_DIRS}
          PATH_SUFFIXES check )

find_library(CHECK_LIBRARY NAMES check
             HINTS ${PC_LIBCHECK_LIBDIR} ${PC_LIBCHECK_LIBRARY_DIRS} )

set(CHECK_LIBRARIES ${CHECK_LIBRARY} )
set(CHECK_INCLUDE_DIRS ${CHECK_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set CHECK_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(check  DEFAULT_MSG
                                  CHECK_LIBRARY CHECK_INCLUDE_DIR)

mark_as_advanced(CHECK_INCLUDE_DIR CHECK_LIBRARY )
