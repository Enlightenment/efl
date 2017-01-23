#ifndef CONFIG_H
#define CONFIG_H

#define _GNU_SOURCE

#include "config_headers.h"

#define VMAJ @VMAJ@
#define VMIN @VMIN@
#define VMIC @VMIC@
#define VREV @VREV@

#cmakedefine NVALGRIND 1

#cmakedefine EINA_STATIC_BUILD_CHAINED_POOL 1
#cmakedefine EINA_STATIC_BUILD_ONE_BIG 1
#cmakedefine EINA_STATIC_BUILD_PASS_THROUGH 1

#define MODULE_ARCH "@MODULE_ARCH@"
#define SHARED_LIB_SUFFIX "@CMAKE_SHARED_LIBRARY_SUFFIX@"
#define EXEEXT "@CMAKE_EXECUTABLE_SUFFIX@"

#if defined (HAVE_LISTXATTR) && defined (HAVE_SETXATTR) && defined (HAVE_GETXATTR)
#define HAVE_XATTR
#endif

//for now statically define that to one
#define STRERROR_R_CHAR_P 1

#cmakedefine EFL_ALWAYS_INLINE @EFL_ALWAYS_INLINE@
#cmakedefine EFL_HAVE_OSX_SPINLOCK 1
#cmakedefine EFL_HAVE_POSIX_THREADS_SPINLOCK 1
#cmakedefine EFL_HAVE_THREADS 1

#endif
