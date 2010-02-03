dnl Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if fnmatch functions are available or not.

dnl Usage: EFL_CHECK_FNMATCH([, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EFL_CHECK_FNMATCH],
[

AC_CHECK_HEADER([fnmatch.h], [_efl_have_fnmatch="yes"], [_efl_have_fnmatch="no"])

if test "x${_efl_have_fnmatch}" = "xyes" ; then
   AC_SEARCH_LIBS([fnmatch],
      [fnmatch evil iberty],
      [_efl_have_fnmatch="yes"],
      [_efl_have_fnmatch="no"])
fi

AS_IF([test "x$_efl_have_fnmatch" = "xyes"], [$1], [$2])

])
