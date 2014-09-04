
dnl use: ELM_CHECK_BACKEND(engine)
AC_DEFUN([ELM_CHECK_BACKEND],
[dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

have_elementary_[]DOWN="no"

AC_ARG_ENABLE([ecore-]DOWN,
   [AC_HELP_STRING([--disable-ecore-]DOWN, [disable ecore-]DOWN[ support. @<:@default=detect@:>@])],
   [want_backend=$enableval],
   [want_backend="auto"])

AC_MSG_CHECKING([whether to enable $1 backend])
AC_MSG_RESULT([${want_backend}])

if test "x${want_backend}" != "xno"; then
   PKG_CHECK_EXISTS([ecore-]DOWN[ >= efl_version],
      [
       AC_DEFINE([HAVE_ELEMENTARY_]UP, [1], [$1 support for Elementary])
       have_elementary_[]DOWN="yes"
       requirement_elm_pc="ecore-[]DOWN >= efl_version ${requirement_elm_pc}"
      ],
      [have_elementary_]DOWN[="no"]
   )
fi

AC_MSG_CHECKING([whether to build $1 backend])
AC_MSG_RESULT([${have_elementary_[]DOWN}])

if test "x${want_backend}" = "xyes" && test "x$have_elementary_[]DOWN" = "xno"; then
    AC_MSG_ERROR([ecore-]DOWN[ support requested, but ecore-]DOWN[ was not found by pkg-config.])
fi

m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])
