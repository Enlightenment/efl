# - Try to find emotion
# Once done this will define
#  EMOTION_FOUND - System has emotion
#  EMOTION_INCLUDE_DIRS - The emotion include directories
#  EMOTION_LIBRARIES - The libraries needed to use emotion
#  EMOTION_DEFINITIONS - Compiler switches required for using emotion

set(MY_PKG emotion)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEMOTION ${_QUIET} ${MY_PKG})

find_library(EMOTION_LIBRARY
             NAMES ${PC_LIBEMOTION_LIBRARIES}
             HINTS ${PC_LIBEMOTION_LIBDIR} ${PC_LIBEMOTION_LIBRARY_DIRS} )

set(EMOTION_DEFINITIONS ${PC_LIBEMOTION_CFLAGS_OTHER})
set(EMOTION_LIBRARIES ${EMOTION_LIBRARY})
set(EMOTION_INCLUDE_DIRS ${PC_LIBEMOTION_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EMOTION_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   EMOTION_LIBRARIES EMOTION_INCLUDE_DIRS)

mark_as_advanced(EMOTION_INCLUDE_DIRS EMOTION_LIBRARY EMOTION_LIBRARIES EMOTION_DEFINITIONS)

