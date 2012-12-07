dnl use: ECORE_CHECK_X_EXTENSION(Foo, header, lib, func)
AC_DEFUN([ECORE_CHECK_X_EXTENSION],
[
pushdef([UP], translit([$1], [a-z], [A-Z]))dnl

  SAVE_CFLAGS=$CFLAGS
  CFLAGS="$x_cflags $x_includes"
  AC_CHECK_HEADER(X11/extensions/$2,
    [
     AC_CHECK_LIB($3, $4,
       [AC_DEFINE(ECORE_[]UP, 1, [Build support for $1])],
       [AC_MSG_ERROR([Missing support for X extension: $1])],
       [$x_libs])
    ],
    [AC_MSG_ERROR([Missing X11/extensions/$2])],
    [ #include <X11/Xlib.h> ]
  )
  CFLAGS=$SAVE_CFLAGS

ECORE_X_LIBS="${ECORE_X_LIBS} -l$3"

popdef([UP])
])
