
dnl use: ELM_CHECK_BACKEND(engine)
AC_DEFUN([ELM_CHECK_BACKEND],
[dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

have_elementary_[]DOWN="no"
want_backend="$2"

AC_MSG_CHECKING([whether to enable $1 backend])
AC_MSG_RESULT([${want_backend}])

if test "x${want_backend}" != "xno"; then
   AC_DEFINE([HAVE_ELEMENTARY_]UP, [1], [$1 support for Elementary])
   have_elementary_[]DOWN="yes"
   requirement_elm_pc="ecore-[]DOWN >= efl_version ${requirement_elm_pc}"
fi

AC_MSG_CHECKING([whether to build $1 backend])
AC_MSG_RESULT([${have_elementary_[]DOWN}])

EFL_ADD_FEATURE([ELEMENTARY], [$1], [${want_backend}])

m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])
