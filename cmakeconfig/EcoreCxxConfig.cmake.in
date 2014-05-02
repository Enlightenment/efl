# - Try to find ecore_cxx
# Once done this will define
#
#  ECORE_CXX_FOUND - System has ecore_cxx
#  ECORE_CXX_INCLUDE_DIRS - The ecore_cxx include directories
#  ECORE_CXX_LIBRARIES - The libraries needed to use ecore_cxx
#  ECORE_CXX_DEFINITIONS - Compiler switches required for using ecore_cxx
#
# When the COMPONENTS keyword was passed to find_package(),
# the following variables are defined for additional Ecore_Cxx modules
# such as Evas, X, Imf, Imf_Evas.
#
#  ECORE_CXX_*_FOUND        - System has ecore_cxx *
#  ECORE_CXX_*_INCLUDE_DIRS - The ecore_cxx include directories
#  ECORE_CXX_*_LIBRARIES    - The libraries needed to use ecore_cxx
#  ECORE_CXX_*_DEFINITIONS  - Compiler switches required for using ecore_cxx * module.

set(MY_PKG ecore_cxx)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.1")
   # "QUIET" was introduced in 2.8.2
   set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBECORE_CXX ${_QUIET} ${MY_PKG})

find_library(ECORE_CXX_LIBRARY
             NAMES ${PC_LIBECORE_CXX_LIBRARIES}
             HINTS ${PC_LIBECORE_CXX_LIBDIR} ${PC_LIBECORE_CXX_LIBRARY_DIRS} )

set(ECORE_CXX_DEFINITIONS ${PC_LIBECORE_CXX_CFLAGS_OTHER})
set(ECORE_CXX_LIBRARIES ${ECORE_CXX_LIBRARY})
set(ECORE_CXX_INCLUDE_DIRS ${PC_LIBECORE_CXX_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ECORE_CXX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
   ECORE_CXX_LIBRARIES ECORE_CXX_INCLUDE_DIRS)

mark_as_advanced(ECORE_CXX_INCLUDE_DIRS ECORE_CXX_LIBRARY ECORE_CXX_LIBRARIES ECORE_CXX_DEFINITIONS)

if (Ecore_Cxx_FIND_COMPONENTS)
   foreach(_component ${Ecore_Cxx_FIND_COMPONENTS})
      string(TOUPPER ${_component} _COMPONENT)
      string(TOLOWER ${_component} _component)
      string(REGEX REPLACE "_" "-" _MODULE_PKGNAME "ecore_cxx-${_component}")

      pkg_check_modules(PC_LIBECORE_CXX_${_COMPONENT} ${_QUIET} ${_MODULE_PKGNAME})

      find_library(ECORE_CXX_${_COMPONENT}_LIBRARY
         NAMES ${PC_LIBECORE_CXX_${_COMPONENT}_LIBRARIES}
         HINTS ${PC_LIBECORE_CXX_${_COMPONENT}_LIBDIR}
               ${PC_LIBECORE_CXX_${_COMPONENT}_LIBRARY_DIRS})

      set(ECORE_CXX_${_COMPONENT}_DEFINITIONS ${PC_LIBECORE_CXX_${_COMPONENT}_CFLAGS_OTHER})
      set(ECORE_CXX_${_COMPONENT}_LIBRARIES ${ECORE_CXX_${_COMPONENT}_LIBRARY})
      set(ECORE_CXX_${_COMPONENT}_INCLUDE_DIRS ${PC_LIBECORE_CXX_${_COMPONENT}_INCLUDE_DIRS})

      include(FindPackageHandleStandardArgs)
      # handle the QUIETLY and REQUIRED arguments and set ECORE_CXX_*_FOUND to TRUE
      # if all listed variables are TRUE
      find_package_handle_standard_args(ecore_cxx_${_component} DEFAULT_MSG
         ECORE_CXX_${_COMPONENT}_LIBRARIES ECORE_CXX_${_COMPONENT}_INCLUDE_DIRS)
   endforeach(_component)
endif(Ecore_Cxx_FIND_COMPONENTS)
