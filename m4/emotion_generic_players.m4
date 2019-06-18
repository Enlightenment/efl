dnl EMOTION_GENERIC_PLAYER(NAME, DEFAULT_STATE, [PKG_CONFIG_DEPS])
dnl Does the following:
dnl  * AC_ARG_WITH(NAME)
dnl  * define with_name to yes or no
dnl  * PKG_CHECK_MODULES(NAME, PKG_CONFIG_DEPS)
dnl  * AC_DEFINE([HAVE_NAME])
dnl  * AM_CONDITIONAL([HAVE_NAME])
AC_DEFUN([EMOTION_GENERIC_PLAYER],
[dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl

AC_ARG_WITH([DOWN],
   [AC_HELP_STRING([--with-]m4_defn([DOWN]),
       [build generic player ]m4_defn([UP])[. @<:@default=$2@:>@])],
   [], [with_]m4_defn([DOWN])[="$2"])

if test "${with_[]m4_defn([DOWN])}" = "yes"; then
   m4_ifval([$3], [dnl
      PKG_CHECK_MODULES(m4_defn([UP]), [$3])
   ], [dnl
      m4_defn([UP])_LIBS="${m4_defn([UP])_LIBS}"
      m4_defn([UP])_CFLAGS="${m4_defn([UP])_CFLAGS}"
      AC_SUBST(m4_defn([UP])[_LIBS])
      AC_SUBST(m4_defn([UP])[_CFLAGS])
   ])

   AC_SEARCH_LIBS([shm_open], [rt], [], [AC_MSG_ERROR([unable to find the shm_open() function])])
   if test "${ac_cv_search_shm_open}" != "none required"; then
      m4_defn([UP])_LIBS="${m4_defn([UP])_LIBS} ${ac_cv_search_shm_open}"
   fi
fi
AM_CONDITIONAL([HAVE_]m4_defn([UP]), [test "$with_[]m4_defn([DOWN])" = "yes"])

m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])
