# - Try to find ecore
# Once done this will define
#  ECORE_FOUND - System has ecore
#  ECORE_INCLUDE_DIRS - The ecore include directories
#  ECORE_LIBRARIES - The libraries needed to use ecore
#  ECORE_DEFINITIONS - Compiler switches required for using ecore

# Use FIND_PACKAGE( Ecore COMPONENTS ... ) to enable modules
IF( Ecore_FIND_COMPONENTS )
  FOREACH( component ${Ecore_FIND_COMPONENTS} )
    STRING( TOUPPER ${component} _COMPONENT )
    SET( ECORE_USE_${_COMPONENT} 1 )
  ENDFOREACH( component )
ENDIF( Ecore_FIND_COMPONENTS )

find_package(PkgConfig)
pkg_check_modules(PC_LIBECORE QUIET ecore)
set(ECORE_DEFINITIONS ${PC_LIBECORE_CFLAGS_OTHER})

find_path(ECORE_INCLUDE_DIR Ecore.h
          HINTS ${PC_LIBECORE_INCLUDEDIR} ${PC_LIBECORE_INCLUDE_DIRS}
          PATH_SUFFIXES ecore )

find_library(ECORE_LIBRARY NAMES ecore
             HINTS ${PC_LIBECORE_LIBDIR} ${PC_LIBECORE_LIBRARY_DIRS} )

set(ECORE_LIBRARIES ${ECORE_LIBRARY} )
set(ECORE_INCLUDE_DIRS ${ECORE_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ECORE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ecore  DEFAULT_MSG
                                  ECORE_LIBRARY ECORE_INCLUDE_DIR)

mark_as_advanced( ECORE_INCLUDE_DIR ECORE_LIBRARY )

if (ECORE_USE_ECORE-X)
   pkg_check_modules(PC_LIBECORE_X QUIET ecore-x)
   set(ECORE_X_DEFINITIONS ${PC_LIBECORE_X_CFLAGS_OTHER})

   find_path(ECORE_X_INCLUDE_DIR Ecore_X.h
             HINTS ${PC_LIBECORE_X_INCLUDEDIR} ${PC_LIBECORE_X_INCLUDE_DIRS}
             PATH_SUFFIXES ecore )

   find_library(ECORE_X_LIBRARY NAMES ecore_x
                HINTS ${PC_LIBECORE_X_LIBDIR} ${PC_LIBECORE_X_LIBRARY_DIRS} )

   set(ECORE_X_LIBRARIES ${ECORE_X_LIBRARY} )
   set(ECORE_X_INCLUDE_DIRS ${ECORE_X_INCLUDE_DIR} )

   include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ECORE_X_FOUND to TRUE
# if all listed variables are TRUE
   find_package_handle_standard_args(ecore_x  DEFAULT_MSG
                                     ECORE_X_LIBRARY ECORE_X_INCLUDE_DIR)

   mark_as_advanced( ECORE_X_INCLUDE_DIR ECORE_X_LIBRARY )
endif (ECORE_USE_ECORE-X)
