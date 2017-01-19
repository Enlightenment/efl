#ifndef CONFIG_H
#define CONFIG_H

#define _GNU_SOURCE
#cmakedefine HAVE_SYS_MMAN_H 1

#cmakedefine VMAJ @VMAJ@
#cmakedefine VMIN @VMIN@
#cmakedefine VMIC @VMIC@
#cmakedefine VREV @VREV@

#cmakedefine NVALGRIND 1
#cmakedefine HAVE_CLOCK_GETTIME 1

#define PACKAGE_LIB_DIR "bla"
#define MODULE_ARCH "bla"
#define SHARED_LIB_SUFFIX "bla"

#endif
