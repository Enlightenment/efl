dnl Copyright (C) 2013 Cedric Bail <cedric dot bail at samsung dot com>
dnl That code is public domain and can be freely used or copied

dnl Macro that check if -pie -rdynamic can be given to ld

dnl Usage: ELM_QUICKLAUNCH
dnl add -pie -rdynamic to LDFLAGS and -fpie to CFLAGS

AC_DEFUN([ELM_QUICKLAUNCH],
[
AC_MSG_CHECKING([If the compiler as what it takes to do quicklaunch (-pie -rdynamic)])
old_LDFLAGS="$LDFLAGS"
old_CFLAGS="$CFLAGS"

LDFLAGS="$LDFLAGS -pie -rdynamic"
CFLAGS="$CFLAGS -fpie"

AC_LINK_IFELSE([AC_LANG_SOURCE([int main(){}])],
        [AC_MSG_RESULT([yes])],
        [LDFLAGS="$old_LDFLAGS"
	CFLAGS="$old_CFLAGS"
        AC_MSG_RESULT([no])
        ])
])
