# - Try to find edje
# Once done this will define
#  EDJE_FOUND - System has edje
#  EDJE_INCLUDE_DIRS - The edje include directories
#  EDJE_LIBRARIES - The libraries needed to use edje
#  EDJE_DEFINITIONS - Compiler switches required for using edje

find_package(PkgConfig)
pkg_check_modules(PC_LIBEDJE QUIET edje)
set(EDJE_DEFINITIONS ${PC_LIBEDJE_CFLAGS_OTHER})

find_path(EDJE_INCLUDE_DIR Edje.h
          HINTS ${PC_LIBEDJE_INCLUDEDIR} ${PC_LIBEDJE_INCLUDE_DIRS}
          PATH_SUFFIXES edje )

find_library(EDJE_LIBRARY NAMES edje
             HINTS ${PC_LIBEDJE_LIBDIR} ${PC_LIBEDJE_LIBRARY_DIRS} )

set(EDJE_LIBRARIES ${EDJE_LIBRARY} )
set(EDJE_INCLUDE_DIRS ${EDJE_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EDJE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(edje  DEFAULT_MSG
                                  EDJE_LIBRARY EDJE_INCLUDE_DIR)

mark_as_advanced(EDJE_INCLUDE_DIR EDJE_LIBRARY )
