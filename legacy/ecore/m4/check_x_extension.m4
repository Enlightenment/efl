dnl use: ECORE_CHECK_X_EXTENSION(Foo, header, lib, func)
AC_DEFUN([ECORE_CHECK_X_EXTENSION],
[
pushdef([UP], translit([$1], [a-z], [A-Z]))dnl
pushdef([DOWN], translit([$1], [A-Z], [a-z]))dnl

UP[]_CFLAGS=""
UP[]_LIBS=""
use_[]DOWN="no"

SAVE_CFLAGS=$CFLAGS
CFLAGS="$x_cflags $x_includes"
AC_CHECK_HEADER(X11/extensions/$2,
  [
   AC_CHECK_LIB($3, $4,
     [
      AC_DEFINE(ECORE_[]UP, 1, [Build support for $1])
      UP[]_LIBS="-l$3"
      use_[]DOWN="yes"
     ],
     [ use_[]DOWN="no" ],
     [ $x_libs ]
   )
  ],
  [ use_[]DOWN="no" ],
  [ #include <X11/Xlib.h> ]
)
CFLAGS=$SAVE_CFLAGS

AC_SUBST(UP[]_CFLAGS)
AC_SUBST(UP[]_LIBS)

popdef([UP])
popdef([DOWN])
])
