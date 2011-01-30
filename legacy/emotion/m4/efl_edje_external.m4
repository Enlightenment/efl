dnl Copyright (C) 2010-2011 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if edje external should be enabled or not

dnl Usage: EFL_EDJE_EXTERNAL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(EDJE_EXTERNAL_CFLAGS)
dnl Call AC_SUBST(EDJE_EXTERNAL_LIBS)
dnl Define conditional ENABLE_EDJE_EXTERNAL

AC_DEFUN([EFL_EDJE_EXTERNAL],
[

AC_ARG_ENABLE([edje-external],
   [AC_HELP_STRING([--disable-edje-external],
      [disable providing Edje EXTERNAL support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_want_edje_external="yes"
    else
       _efl_want_edje_external="no"
    fi
   ],
   [_efl_want_edje_external="auto"])

AC_MSG_CHECKING([if should provide Edje EXTERNAL support...])
AC_MSG_RESULT([${_efl_want_edje_external}])

_efl_enable_edje_external="no"
if ! test "x${_efl_want_edje_external}" = "xno" ; then
   PKG_CHECK_MODULES([EDJE_EXTERNAL],
      [edje >= 1.0.0],
      [_efl_enable_edje_external="yes"],
      [_efl_enable_edje_external="no"])
fi

AC_SUBST(EDJE_EXTERNAL_CFLAGS)
AC_SUBST(EDJE_EXTERNAL_LIBS)

if test "x${_efl_want_edje_external}" = "xyes" && test "x${_efl_enable_edje_external}" = "xno" ; then
   AC_MSG_ERROR([Edje EXTERNAL support requested, but Edje not found by pkg-config.])
fi

if test "x${_efl_enable_edje_external}" = "xyes" ; then
   EDJE_VERSION=`${PKG_CONFIG} edje --modversion | awk -F . '{printf("%s.0.0", $[]1);}'`
   MODULE_EDJE="$host_os-$host_cpu-${EDJE_VERSION}"
   AC_DEFINE_UNQUOTED(MODULE_EDJE, "$MODULE_EDJE", "Edje module architecture")
fi

AC_SUBST(MODULE_EDJE)

AM_CONDITIONAL([ENABLE_EDJE_EXTERNAL], [test "x${_efl_enable_edje_external}" = "xyes"])

AS_IF([test "x${_efl_enable_edje_external}" = "xyes"], [$1], [$2])

])
