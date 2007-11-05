dnl use: EVAS_CONVERT_COLOR(bpp, colorspace, components[, default-enabled])
AC_DEFUN([EVAS_CONVERT_COLOR],
[
pushdef([UP], translit([$1_$2_$3], [a-z], [A-Z]))dnl
pushdef([DOWN_D], translit([$1-$2-$3], [A-Z], [a-z]))dnl
pushdef([DOWN_U], translit([$1_$2_$3], [A-Z], [a-z]))dnl

conv_[]DOWN_U="no"

ifelse("x$4", "xno",
[
  AC_ARG_ENABLE(convert-[]DOWN_D,
    AC_HELP_STRING(
      [--enable-convert-[]DOWN_D],
      [enable the $1bpp $2 $3 converter code]
    ),
    [ conv_[]DOWN_U=$enableval ],
    [ conv_[]DOWN_U=no ]
  )
], [
  AC_ARG_ENABLE(convert-[]DOWN_D,
    AC_HELP_STRING(
      [--disable-convert-[]DOWN_D],
      [disable the $1bpp $2 $3 converter code]
    ),
    [ conv_[]DOWN_U=$enableval ],
    [ conv_[]DOWN_U=yes ]
  )
])

AC_MSG_CHECKING(whether to build $1bpp $2 $3 converter code)
AC_MSG_RESULT($conv_[]DOWN_U)

if test "x$conv_[]DOWN_U" = "xyes" ; then
  AC_DEFINE(BUILD_CONVERT_[]UP, 1, [$1bpp $2 $3 Converter Support])
fi

popdef([UP])
popdef([DOWN_D])
popdef([DOWN_U])
])

dnl use: EVAS_CONVERT_ROT(bpp, colorspace, rot[, default-enabled])
AC_DEFUN([EVAS_CONVERT_ROT],
[
pushdef([UP], translit([$1_$2_ROT$3], [a-z], [A-Z]))dnl
pushdef([DOWN_D], translit([$1-$2-rot-$3], [A-Z], [a-z]))dnl
pushdef([DOWN_U], translit([$1_$2_rot_$3], [A-Z], [a-z]))dnl

conv_[]DOWN_U="no"

ifelse("x$4", "xno",
[
  AC_ARG_ENABLE(convert-[]DOWN_D,
    AC_HELP_STRING(
      [--enable-convert-[]DOWN_D],
      [enable the $1bpp $2 rotation $3 converter code]
    ),
    [ conv_[]DOWN_U=$enableval ],
    [ conv_[]DOWN_U=no ]
  )
], [
  AC_ARG_ENABLE(convert-[]DOWN_D,
    AC_HELP_STRING(
      [--disable-convert-[]DOWN_D],
      [disable the $1bpp $2 rotation $3 converter code]
    ),
    [ conv_[]DOWN_U=$enableval ],
    [ conv_[]DOWN_U=yes ]
  )
])

AC_MSG_CHECKING(whether to build $1bpp $2 rotation $3 converter code)
AC_MSG_RESULT($conv_[]DOWN_U)

if test "x$conv_[]DOWN_U" = "xyes" ; then
  AC_DEFINE(BUILD_CONVERT_[]UP, 1, [$1bpp $2 Rotation $3 Converter Support])
fi

popdef([UP])
popdef([DOWN_D])
popdef([DOWN_U])
])
