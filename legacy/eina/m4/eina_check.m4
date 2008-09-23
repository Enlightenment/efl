dnl use: EINA_CHECK_STATIC(foo-bar, description)
AC_DEFUN([EINA_CHECK_STATIC],
[
pushdef([UP], translit([$1], [-a-z], [_A-Z]))dnl
pushdef([DOWN], translit([$1], [-A-Z], [_a-z]))dnl

AC_ARG_ENABLE([static-$1],
   [AC_HELP_STRING([--enable-static-$1], [enable static build of $2 @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_static_[]DOWN="yes"
    else
       enable_static_[]DOWN="no"
    fi
   ],
   [enable_static_[]DOWN="no"]
)
AC_MSG_CHECKING([whether $2 is statically build])
AC_MSG_RESULT([${enable_static_[]DOWN}])

AM_CONDITIONAL(EINA_STATIC_BUILD_[]UP, test "x${enable_static_[]DOWN}" = "xyes")

if test "x${enable_static_[]DOWN}" = "xyes" ; then
   AC_DEFINE(EINA_STATIC_BUILD_[]UP, 1, [Set to 1 if $2 is statically built])
fi

popdef([UP])
popdef([DOWN])
])