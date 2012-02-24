dnl Copyright (C) 2011 Cedric Bail <cedric.bail@free.fr>
dnl This code is public domain and can be freely used or copied.

dnl Macro that check for gettimeofday definition

dnl Usage: EFL_CHECK_GETTIMEOFDAY(ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
dnl Define EFL_HAVE_GETTIMEOFDAY

AC_DEFUN([EFL_CHECK_GETTIMEOFDAY],
[

_efl_have_gettimeofday="no"

AC_LINK_IFELSE(
   [AC_LANG_PROGRAM([[
#include <stdlib.h>
#include <sys/time.h>
		    ]],
		    [[
int res;
res = gettimeofday(NULL, NULL);
      	            ]])],
   [_efl_have_gettimeofday="yes"],
   [_efl_have_gettimeofday="no"])

if test "x${_efl_have_gettimeofday}" = "xno" -a "x${enable_exotic}" = "xyes"; then
   SAVE_LIBS="${LIBS}"
   SAVE_CFLAGS="${CFLAGS}"
   LIBS="${LIBS} ${EXOTIC_LIBS}"
   CFLAGS="${CFLAGS} ${EXOTIC_CFLAGS}"
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[
#include <Exotic.h>
		       ]],
		       [[
int res;
res = gettimeofday(NULL, NULL);
      		       ]])],
      [_efl_have_gettimeofday="yes"],
      [_efl_have_gettimeofday="no"])
fi

if test "x${_efl_have_gettimeofday}" = "xyes"; then
   AC_DEFINE([EFL_HAVE_GETTIMEOFDAY], [1], [Defined if gettimeofday is available.])
fi

AS_IF([test "x${_efl_have_gettimeofday}" = "xyes"], [$1], [$2])
])
