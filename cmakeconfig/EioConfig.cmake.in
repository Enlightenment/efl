# trys to find eio
# EIO_INCLUDE_DIRS eio include directories
# EIO_LIBRARIES dependencys of eio
# EIO_DEFINITIONS compiler stuff to use it
set(MY_PKG eio)

find_package(PkgConfig)
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER "2.8.2")
  set(_QUIET QUIET)
endif ()
pkg_check_modules(PC_LIBEIO ${_QUIET} ${MY_PKG})

find_library(EIO_LIBRARY
             NAMES ${PC_LIBEIO_LIBRARIES}
             HINTS ${PC_LIBEIO_LIBDIR} ${PC_LIBEIO_LIBRARY_DIRS} )

set(EIO_DEFINITIONS ${PC_LIBEIO_CLAGS_OTHER})
set(EIO_LIBRARIES ${EIO_LIBRARY})
set(EIO_INCLUDE_DIRS ${PC_LIBEIO_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${MY_PKG} DEFAULT_MSG
  EIO_LIBRARIES EIO_INCLUDE_DIRS)

mark_as_advanced(EIO_INCLUDE_DIRS EIO_LIBRARY EIO_LIBRARIES EIO_DEFINITIONS)
