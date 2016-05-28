dnl This code is public domain and can be freely used or copied.
dnl File to auto-detect libunwind

dnl Macro that checks for libunwind, first by using
dnl pkg-config, then by trying to compile and link a simple
dnl program, to see if libunwind is distributed on the system
dnl but has no pkg-config support
dnl
dnl The biggest usecase is on Mac OS X, where there are no
dnl pkg-config files, and the libunwind headers are lost
dnl in an obscure place on the system (but whom the compilers
dnl distributed by Apple are aware of).
dnl
dnl Usage: EFL_CHECK_LIBUNWIND
dnl  will inconditionaly set UNWIND_CFLAGS and UNWIND_LIBS
dnl  to follow pkg-config fashion.
dnl
AC_DEFUN([EFL_CHECK_LIBUNWIND],
[dnl
   dnl First, check with pkg-config
   PKG_CHECK_MODULES([UNWIND], [libunwind libunwind-generic],
                     [have_unwind=yes], [have_unwind=no])

   dnl No pkg-config file... maybe system built-in?
   if test "x${have_unwind}" = "xno"; then
      AC_LANG_PUSH([C])
      AC_LINK_IFELSE(
	 [AC_LANG_PROGRAM(
	    [[
#include <libunwind.h>
	    ]],
	    [[
	       unw_context_t ctx;
	       unw_getcontext(&ctx);
	    ]]
	 )],
	 [
	    have_unwind="yes"
	 ],
	 [
	    have_unwind="no"
	 ]
      )
      AC_MSG_CHECKING([for native libunwind])
      AC_MSG_RESULT([${have_unwind}])
      AC_LANG_POP([C])

      dnl Provide dummy variables to automake.
      dnl In case pkg-config succeeded, these will be set and
      dnl used in other automake files. To avoid, problems,
      dnl we define empty variables.
      UNWIND_CFLAGS=""
      UNWIND_LIBS=""
      AC_SUBST([UNWIND_CFLAGS])
      AC_SUBST([UNWIND_LIBS])
   fi

   AS_IF([test "x$have_unwind" = "xyes"],
	 [AC_DEFINE([HAVE_UNWIND], [1], [Have libunwind])])
   AM_CONDITIONAL(HAVE_UNWIND, test "x$have_unwind" = "xyes")
])
