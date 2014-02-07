dnl Copyright (C) 2012 Vincent Torri <vincent dot torri at gmail dot com>
dnl This code is public domain and can be freely used or copied.

dnl Macros that check functions availability for the EFL:

dnl dirfd
dnl dladdr
dnl dlopen
dnl fcntl
dnl fnmatch
dnl gettimeofday
dnl iconv
dnl setxattr (an al.)
dnl shm_open

dnl EFL_CHECK_LIB_CODE(EFL, LIB, VARIABLE, HEADER, BODY)
dnl wrapper around AC_LINK_IFELSE(AC_LANG_PROGRAM()) to check
dnl if some code would work with the given lib.
dnl If the code work, EFL_ADD_LIBS(EFL, LIB) will be called
dnl At the end VARIABLE will be "yes" or "no"
AC_DEFUN([EFL_CHECK_LIB_CODE],
[
LIBS_save="${LIBS}"
LIBS="${LIBS} $2"
AC_LINK_IFELSE([AC_LANG_PROGRAM([$4], [$5])],
   [EFL_ADD_LIBS([$1], [$2])
    $3="yes"], [$3="no"])
LIBS="${LIBS_save}"
])

dnl EFL_FIND_LIB_FOR_CODE(EFL, LIBS, VARIABLE, HEADER, BODY)
AC_DEFUN([EFL_FIND_LIB_FOR_CODE],
[
dnl first try without lib (libc)
EFL_CHECK_LIB_CODE([$1], [], [$3], [$4], [$5])

if test "${$3}" = "no" && test "x$2" != "x"; then
dnl loop through given libraries
  for trylib in $2; do
     EFL_CHECK_LIB_CODE([$1], [${trylib}], [$3], [$4], [$5])
     if test "${$3}" = "yes"; then
       break
     fi
  done
fi
])

dnl _EFL_CHECK_FUNC_DIRFD is for internal use
dnl _EFL_CHECK_FUNC_DIRFD(EFL, VARIABLE)

AC_DEFUN([_EFL_CHECK_FUNC_DIRFD],
[EFL_CHECK_LIB_CODE([$1], [], [$2], [[
#ifdef HAVE_DIRENT_H
# include <dirent.h>
#endif
]], [[DIR *dirp; return dirfd(dirp);]])
])

dnl _EFL_CHECK_FUNC_DLADDR is for internal use
dnl _EFL_CHECK_FUNC_DLADDR(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_DLADDR],
[
case "$host_os" in
   mingw*)
      $2="yes"
      EFL_ADD_LIBS([$1], [lib/evil/libdl.la])
   ;;
   *)
      EFL_FIND_LIB_FOR_CODE([$1], [-ldl], [$2], [[
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <dlfcn.h>
]], [[int res = dladdr(NULL, NULL);]])
   ;;
esac
])

dnl _EFL_CHECK_FUNC_DLOPEN is for internal use
dnl _EFL_CHECK_FUNC_DLOPEN(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_DLOPEN],
[
case "$host_os" in
   mingw*)
      $2="yes"
      EFL_ADD_LIBS([$1], [lib/evil/libdl.la])
   ;;
   *)
      EFL_FIND_LIB_FOR_CODE([$1], [-ldl], [$2], [[
#include <dlfcn.h>
]], [[void *h = dlopen(0, 0);]])
   ;;
esac
])

dnl _EFL_CHECK_FUNC_DLSYM is for internal use
dnl _EFL_CHECK_FUNC_DLSYM(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_DLSYM],
[
case "$host_os" in
   mingw*)
      $2="yes"
      EFL_ADD_LIBS([$1], [lib/evil/libdl.la])
   ;;
   *)
      EFL_FIND_LIB_FOR_CODE([$1], [-ldl], [$2], [[
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <dlfcn.h>
]], [[void *res = dlsym(NULL, NULL);]])
   ;;
esac
])

dnl _EFL_CHECK_FUNC_FCNTL is for internal use
dnl _EFL_CHECK_FUNC_FCNTL(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_FCNTL],
[
case "$host_os" in
   mingw*)
      $2="yes"
   ;;
   *)
      EFL_FIND_LIB_FOR_CODE([$1], [], [$2], [[
#include <fcntl.h>
]], [[int g = fcntl(0, 0);]])
   ;;
esac
])

dnl _EFL_CHECK_FUNC_FNMATCH is for internal use
dnl _EFL_CHECK_FUNC_FNMATCH(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_FNMATCH],
[
case "$host_os" in
   mingw*)
      $2="yes"
   ;;
   *)
      EFL_FIND_LIB_FOR_CODE([$1], [-lfnmatch -liberty], [$2], [[
#include <stdlib.h>
#include <fnmatch.h>
]], [[int g = fnmatch(NULL, NULL, 0);]])
   ;;
esac
])

dnl _EFL_CHECK_FUNC_GETTIMEOFDAY is for internal use
dnl _EFL_CHECK_FUNC_GETTIMEOFDAY(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_GETTIMEOFDAY],
[
case "$host_os" in
   mingw*)
      $2="yes"
   ;;
   *)
      EFL_CHECK_LIB_CODE([$1], [], [$2], [[
#include <stdlib.h>
#include <sys/time.h>
]], [[int res = gettimeofday(NULL, NULL);]])

      if test "${$2}" = "no" && test "x${enable_exotic}" = "xyes"; then
         SAVE_CFLAGS="${CFLAGS}"
         CFLAGS="${CFLAGS} ${EXOTIC_CFLAGS}"
         EFL_CHECK_LIB_CODE([$1], [${EXOTIC_LIBS}], [$2], [[
#include <Exotic.h>
]], [[int res = gettimeofday(NULL, NULL);]])
         CFLAGS="${SAVE_CFLAGS}"
      fi
   ;;
esac
])

dnl _EFL_CHECK_FUNC_ICONV is for internal use
dnl _EFL_CHECK_FUNC_ICONV(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_ICONV],
[dnl
AC_ARG_WITH([iconv-link],
   AC_HELP_STRING([--with-iconv-link=ICONV_LINK], [explicitly specify an iconv link option]),
   [
    $2="yes"
    iconv_libs=${withval}
   ],
   [$2="no"])

if test "x${iconv_libs}" = "x" ; then
   EFL_FIND_LIB_FOR_CODE([$1], [-liconv -liconv_plug], [$2], [[
#include <stdlib.h>
#include <iconv.h>
]], [[iconv_t ic; size_t count = iconv(ic, NULL, NULL, NULL, NULL);]])
fi
])

dnl _EFL_CHECK_FUNC_SETXATTR is for internal use
dnl _EFL_CHECK_FUNC_SETXATTR(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_SETXATTR],
[EFL_CHECK_LIB_CODE([$1], [], [$2], [[
#include <stdlib.h>
#include <sys/types.h>
#include <sys/xattr.h>
]], [[
size_t tmp = listxattr("/", NULL, 0);
tmp = getxattr("/", "user.ethumb.md5", NULL, 0);
setxattr("/", "user.ethumb.md5", NULL, 0, 0);
]])

if test "${$2}" = "yes"; then
  AC_DEFINE([HAVE_XATTR], [1], [Define to 1 if you have the `listxattr', `setxattr' and `getxattr' functions.])
fi
])

dnl _EFL_CHECK_FUNC_SHM_OPEN is for internal use
dnl _EFL_CHECK_FUNC_SHM_OPEN(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_SHM_OPEN],
[EFL_FIND_LIB_FOR_CODE([$1], [-lrt], [$2], [[
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
]], [[
int fd = shm_open("/dev/null", O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
]])
])

dnl _EFL_CHECK_FUNC_SPLICE is for internal use
dnl _EFL_CHECK_FUNC_SPLICE(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_SPLICE],
[EFL_FIND_LIB_FOR_CODE([$1], [], [$2], [[
#include <unistd.h>
#include <fcntl.h>
]], [[
long ret = splice(0, 0, 1, 0, 400, 0);
]])
])

dnl _EFL_CHECK_FUNC_GETPAGESIZE is for internal use
dnl _EFL_CHECK_FUNC_GETPAGESIZE(EFL, VARIABLE)
AC_DEFUN([_EFL_CHECK_FUNC_GETPAGESIZE],
[EFL_FIND_LIB_FOR_CODE([$1], [], [$2], [[
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
]],
[[
long sz;
sz = getpagesize();
]])
])

dnl Macro that checks function availability
dnl
dnl EFL_CHECK_FUNC(EFL, FUNCTION)
dnl AC_SUBST : EFL_CFLAGS and EFL_LIBS (EFL being replaced by its value)
dnl AC_DEFINE : HAVE_FUNCTION (FUNCTION being replaced by its value)
dnl result in efl_func_function (function being replaced by its value)

AC_DEFUN([EFL_CHECK_FUNC],
[dnl
m4_pushdef([UP], m4_translit([$2], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$2], [-A-Z], [_a-z]))dnl

m4_default([_EFL_CHECK_FUNC_]m4_defn([UP]))($1, [have_fct])
AC_MSG_CHECKING([for $2])
AC_MSG_RESULT([${have_fct}])

if test "x${have_fct}" = "xyes" ; then
  AC_DEFINE([HAVE_]m4_defn([UP]), [1], [Define to 1 if you have the `]m4_defn([DOWN])[' function.])
fi

efl_func_[]m4_defn([DOWN])="${have_fct}"
m4_popdef([DOWN])dnl
m4_popdef([UP])dnl
])

dnl Macro that iterates over a sequence of space separated functions
dnl and that calls EFL_CHECK_FUNC() for each of these functions
dnl
dnl EFL_CHECK_FUNCS(EFL, FUNCTIONS)

AC_DEFUN([EFL_CHECK_FUNCS],
[m4_foreach_w([fct], [$2], [EFL_CHECK_FUNC($1, m4_defn([fct]))])])
