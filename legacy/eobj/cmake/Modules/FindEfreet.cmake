# - Try to find efreet
# Once done this will define
#  EFREET_FOUND - System has efreet
#  EFREET_INCLUDE_DIRS - The efreet include directories
#  EFREET_LIBRARIES - The libraries needed to use efreet
#  EFREET_DEFINITIONS - Compiler switches required for using efreet

find_package(PkgConfig)
pkg_check_modules(PC_LIBEFREET QUIET efreet)
set(EFREET_DEFINITIONS ${PC_LIBEFREET_CFLAGS_OTHER})

find_path(EFREET_INCLUDE_DIR Efreet.h
          HINTS ${PC_LIBEFREET_INCLUDEDIR} ${PC_LIBEFREET_INCLUDE_DIRS}
          PATH_SUFFIXES efreet )

find_library(EFREET_LIBRARY NAMES efreet
             HINTS ${PC_LIBEFREET_LIBDIR} ${PC_LIBEFREET_LIBRARY_DIRS} )

set(EFREET_LIBRARIES ${EFREET_LIBRARY} )
set(EFREET_INCLUDE_DIRS ${EFREET_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EFREET_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(efreet  DEFAULT_MSG
                                  EFREET_LIBRARY EFREET_INCLUDE_DIR)

mark_as_advanced(EFREET_INCLUDE_DIR EFREET_LIBRARY )
