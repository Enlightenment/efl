dnl Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if fnmatch functions are available or not.

dnl Usage: EFL_CHECK_FNMATCH([, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(EFL_FNMATCH_LIBS)

AC_DEFUN([EFL_CHECK_FNMATCH],
[

AC_CHECK_HEADER([fnmatch.h], [_efl_have_fnmatch="yes"], [_efl_have_fnmatch="no"])

if test "x${_efl_have_fnmatch}" = "xyes" ; then
   AC_SEARCH_LIBS([fnmatch],
      [fnmatch evil exotic iberty],
      [_efl_have_fnmatch="yes"],
      [_efl_have_fnmatch="no"])
fi

EFL_FNMATCH_LIBS=""

if (! test "x${ac_cv_search_fnmatch}" = "xnone required") && (! test "x${ac_cv_search_fnmatch}" = "xno") && (! test "x${ac_cv_search_fnmatch}" = "x-levil") ; then
   EFL_FNMATCH_LIBS=${ac_cv_search_fnmatch}
fi

AC_SUBST(EFL_FNMATCH_LIBS)

AS_IF([test "x$_efl_have_fnmatch" = "xyes"], [$1], [$2])

])
