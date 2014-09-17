
dnl use: ELM_CHECK_OPTION_DEP(option, pkgver)
AC_DEFUN([ELM_CHECK_OPTION_DEP],
[dnl

m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

ELM_[]UP[]_DEF="#undef"
have_elementary_[]DOWN="no"

AC_ARG_ENABLE(DOWN,
   [AS_HELP_STRING([--disable-]DOWN, [disable ]DOWN[ support. @<:@default=detect@:>@])],
   [want_option=$enableval],
   [want_option="auto"])

AC_MSG_CHECKING([whether to enable $1 option])
AC_MSG_RESULT([${want_option}])

if test "x${want_option}" != "xno"; then
   PKG_CHECK_EXISTS(DOWN[ >= $2],
      [
       AC_DEFINE([HAVE_ELEMENTARY_]UP, [1], [$1 support for Elementary])
       have_elementary_[]DOWN="yes"
       ELM_[]UP[]_DEF="#define"
       requirement_elm_pc="[]DOWN >= $2 ${requirement_elm_pc}"
      ],
      [have_elementary_]DOWN[="no"]
   )
fi

AC_MSG_CHECKING([whether to build $1 option])
AC_MSG_RESULT([${have_elementary_[]DOWN}])

if test "x${want_elementary_[]DOWN}" = "xyes" && test "x${have_elementary_[]DOWN}" = "xno"; then
   AC_MSG_ERROR([$1 support requested, but $1 was not found by pkg-config.])
fi

AC_SUBST([ELM_]UP[_DEF])

m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])

dnl use: ELM_CHECK_OPTION_DEP_EWK2()
AC_DEFUN([ELM_CHECK_OPTION_DEP_EWK2],
[dnl

ELM_WEB2_DEF="#undef"
elementary_web="no"

AC_ARG_WITH([elementary-web-backend],
   [AS_HELP_STRING(
       [--with-elementary-web-backend=@<:@ewebkit2/detect/none@:>@],
       [Select the WebKit backend for Elementary to support web @<:@default=detect@:>@])],
   [elementary_web_backend=${withval}],
   [elementary_web_backend="detect"])

case "${elementary_web_backend}" in
    ewebkit2|detect|none)
       ;;
    *)
       AC_MSG_ERROR([Invalid web backend: must be ewebkit2, detect or none])
       ;;
esac

if test "x${elementary_web_backend}" = "xdetect"; then
   want_elementary_web="ewebkit2"
else
   want_elementary_web=${elementary_web_backend}
fi

AC_MSG_CHECKING([which web option to use])
AC_MSG_RESULT([${want_elementary_web}])

if test "x${want_elementary_web}" != "xnone"; then
   PKG_CHECK_EXISTS([$want_elementary_web],
      [
       AC_DEFINE([HAVE_ELEMENTARY_WEB], [1], [Web support for Elementary])
       webkit_datadir=$(${PKG_CONFIG} ${want_elementary_web} --variable=datadir)
       AC_DEFINE_UNQUOTED([WEBKIT_DATADIR],
                          ["${webkit_datadir}"],
                          ["WebKit's data dir"])
       ELM_WEB2_DEF="#define"
       elementary_web=${want_elementary_web}
       requirement_elm_pc="${want_elementary_web} ${requirement_elm_pc}"
      ],
      [elementary_web="no"]
   )
fi

AC_MSG_CHECKING([whether to build web option])
AC_MSG_RESULT([${elementary_web}])

if test "x${elementary_web_backend}" = "xewebkit2" && test "x${elementary_web}" = "xnone"; then
   AC_MSG_ERROR([Web support requested, but no ${elementary_web_backend} found by pkg-config.])
fi

AC_SUBST([ELM_WEB2_DEF])
])


dnl use: ELM_CHECK_OPTION(option, pkgver)
AC_DEFUN([ELM_CHECK_OPTION],
[dnl

m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

if test "x$1" = "xeweb" ; then
   ELM_CHECK_OPTION_DEP_EWK2
else
   ELM_CHECK_OPTION_DEP($1, $2)
fi

m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])
