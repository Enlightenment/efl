dnl Copyright (C) 2008 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if mpatrol is wanted and if yes, if
dnl it is available.

dnl Usage: EFL_CHECK_MPATROL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Defines EFL_MPATROL_CPPFLAGS and EFL_MPATROL_LIBS variables

AC_DEFUN([EFL_CHECK_MPATROL],
[

dnl configure options

AC_ARG_ENABLE([mpatrol],
   [AC_HELP_STRING([--enable-mpatrol], [enable mpatrol @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_mpatrol="yes"
    else
       _efl_enable_mpatrol="no"
    fi
   ],
   [_efl_enable_mpatrol="no"]
)
AC_MSG_CHECKING([whether mpatrol is used])
AC_MSG_RESULT([${_efl_enable_mpatrol}])

AC_ARG_WITH([mpatrol-dir],
   [AC_HELP_STRING([--enable-mpatrol-dir], [specify the directory of mpatrol @<:@default=/usr/local@:>@])],
   [_efl_with_mpatrol_dir=${withval}],
   [_efl_with_mpatrol_dir="/usr/local"]
)

AC_ARG_WITH([mpatrol-bindir],
   [AC_HELP_STRING([--enable-mpatrol-bindir], [specify the binary directory of mpatrol])],
   [_efl_with_mpatrol_bindir=${withval}]
)

AC_ARG_WITH([mpatrol-includedir],
   [AC_HELP_STRING([--enable-mpatrol-includedir], [specify the include directory of mpatrol])],
   [_efl_with_mpatrol_includedir=${withval}]
)

AC_ARG_WITH([mpatrol-libdir],
   [AC_HELP_STRING([--enable-mpatrol-libdir], [specify the library directory of mpatrol])],
   [_efl_with_mpatrol_libdir=${withval}]
)

if test "x${_efl_enable_mpatrol}" = "xyes" ; then

dnl values of the different paths

   if test ! "x${_efl_with_mpatrol_bindir}" = "x" ; then
      _efl_mpatrol_bindir=${_efl_with_mpatrol_bindir}
   else
      _efl_mpatrol_bindir="${_efl_with_mpatrol_dir}/bin"
   fi

   if test ! "x${_efl_with_mpatrol_includedir}" = "x" ; then
      _efl_mpatrol_includedir=${_efl_with_mpatrol_includedir}
   else
      _efl_mpatrol_includedir="${_efl_with_mpatrol_dir}/include"
   fi

   if test ! "x${_efl_with_mpatrol_libdir}" = "x" ; then
      _efl_mpatrol_libdir=${_efl_with_mpatrol_libdir}
   else
      _efl_mpatrol_libdir="${_efl_with_mpatrol_dir}/lib"
   fi

dnl check of mpatrol program

   AC_CHECK_PROG([_efl_have_mpatrol],
      [mpatrol],
      ["yes"],
      ["no"],
      [$PATH$PATH_SEPARATOR${_efl_mpatrol_bindir}]
   )

   SAVE_CPPFLAGS=${CPPFLAGS}
   SAVE_LIBS=${LDFLAGS}
   CPPFLAGS="-I${_efl_mpatrol_includedir}"
   LIBS="-L${_efl_mpatrol_libdir} -lmpatrol -lbfd -liberty -limagehlp"

dnl check of mpatrol.h header file

   AC_CHECK_HEADER([mpatrol.h],
      [],
      [_efl_have_mpatrol="no"]
   )

dnl check of mpatrol library and its needed dependencies

   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[
#include <mpatrol.h>
                       ]],
                       [[
__mp_clearleaktable();
                       ]])],
      [],
      [_efl_have_mpatrol="no"]
   )

   AC_MSG_CHECKING([whether libmpatrol is usuable])
   AC_MSG_RESULT([${_efl_have_mpatrol}])

   CPPFLAGS=${SAVE_CPPFLAGS}
   LIBS=${SAVE_LIBS}

fi

EFL_MPATROL_CPPFLAGS="-include ${_efl_mpatrol_includedir}/mpatrol.h"
EFL_MPATROL_LIBS="-L${_efl_mpatrol_libdir} -lmpatrol -lbfd -liberty -limagehlp"

AC_SUBST(EFL_MPATROL_CPPFLAGS)
AC_SUBST(EFL_MPATROL_LIBS)

AM_CONDITIONAL(EFL_HAVE_MPATROL, test "x${_efl_have_mpatrol}" = "xyes")

if test "x${_efl_have_mpatrol}" = "xyes" ; then
   ifelse([$1], , :, [$1])
else
   ifelse([$2], , :, [$2])
fi

])
