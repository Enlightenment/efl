# - Try to find ecore
# Once done this will define
#
#  ECORE_FOUND - System has ecore
#  ECORE_INCLUDE_DIRS - The ecore include directories
#  ECORE_LIBRARIES - The libraries needed to use ecore
#  ECORE_DEFINITIONS - Compiler switches required for using ecore
#
# When the COMPONENTS keyword was passed to find_package(),
# the following variables are defined for additional Ecore modules
# such as Evas, X, Imf, Imf_Evas.
#
#  ECORE_*_FOUND        - System has ecore *
#  ECORE_*_INCLUDE_DIRS - The ecore include directories
#  ECORE_*_LIBRARIES    - The libraries needed to use ecore
#  ECORE_*_DEFINITIONS  - Compiler switches required for using ecore * module.

set(MY_PKG ecore)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBECORE ${_QUIET} ${MY_PKG})

find_library(ECORE_LIBRARY
             NAMES ${PC_LIBECORE_LIBRARIES}
             HINTS ${PC_LIBECORE_LIBDIR} ${PC_LIBECORE_LIBRARY_DIRS} )

set(ECORE_DEFINITIONS ${PC_LIBECORE_CFLAGS_OTHER})
set(ECORE_LIBRARIES ${ECORE_LIBRARY})
set(ECORE_INCLUDE_DIRS ${PC_LIBECORE_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ECORE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   ECORE_LIBRARIES ECORE_INCLUDE_DIRS)

mark_as_advanced(ECORE_INCLUDE_DIRS ECORE_LIBRARY ECORE_LIBRARIES ECORE_DEFINITIONS)

if (Ecore_FIND_COMPONENTS)
   foreach(_component ${Ecore_FIND_COMPONENTS})
      string(TOUPPER ${_component} _COMPONENT)
      string(TOLOWER ${_component} _component)
      string(REGEX REPLACE "_" "-" _MODULE_PKGNAME "ecore-${_component}")

      pkg_check_modules(PC_LIBECORE_${_COMPONENT} ${_QUIET} ${_MODULE_PKGNAME})

      find_library(ECORE_${_COMPONENT}_LIBRARY
         NAMES ${PC_LIBECORE_${_COMPONENT}_LIBRARIES}
         HINTS ${PC_LIBECORE_${_COMPONENT}_LIBDIR}
               ${PC_LIBECORE_${_COMPONENT}_LIBRARY_DIRS})

      set(ECORE_${_COMPONENT}_DEFINITIONS ${PC_LIBECORE_${_COMPONENT}_CFLAGS_OTHER})
      set(ECORE_${_COMPONENT}_LIBRARIES ${ECORE_${_COMPONENT}_LIBRARY})
      set(ECORE_${_COMPONENT}_INCLUDE_DIRS ${PC_LIBECORE_${_COMPONENT}_INCLUDE_DIRS})

      include(FindPackageHandleStandardArgs)
      # handle the QUIETLY and REQUIRED arguments and set ECORE_*_FOUND to TRUE
      # if all listed variables are TRUE
      find_package_handle_standard_args(ecore_${_component} DEFAULT_MSG
         ECORE_${_COMPONENT}_LIBRARIES ECORE_${_COMPONENT}_INCLUDE_DIRS)
   endforeach(_component)
endif(Ecore_FIND_COMPONENTS)
