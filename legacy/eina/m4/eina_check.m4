dnl use: EINA_CHECK_MODULE(foo-bar, have_dependency, description)
AC_DEFUN([EINA_CHECK_MODULE],
[
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_module="$2"

AC_ARG_ENABLE([$1],
   [AC_HELP_STRING([--enable-$1@<:@=yes|static|no@:>@], [enable build of $3 @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_module="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          enable_module="static"
       else
          enable_module="no"
       fi
    fi
   ],
   [enable_module="yes"]
)

have_module="no"
if test "x${enable_module}" = "xyes" -o "x${enable_module}" = "xstatic" ; then
   have_module="yes"
fi

AC_MSG_CHECKING([whether to enable $3 built])
AC_MSG_RESULT([${have_module}])

build_module="no"
if test "x${want_module}" = "xyes" -a "x${have_module}" = "xyes" ; then
   build_module="yes"
fi

AC_MSG_CHECKING([whether to build $3])
AC_MSG_RESULT([${build_module}])

static_module="no"
if test "x${want_module}" = "xyes" -a "x${enable_module}" = "xstatic" ; then
   static_module="yes"
fi

AC_MSG_CHECKING([whether to statically link $3])
AC_MSG_RESULT([${static_module}])

AM_CONDITIONAL(EINA_BUILD_[]UP, [test "x${build_module}" = "xyes"])
AM_CONDITIONAL(EINA_STATIC_BUILD_[]UP, [test "x${static_module}" = "xyes"])

if test "x${static_module}" = "xyes" ; then
   AC_DEFINE(EINA_STATIC_BUILD_[]UP, 1, [Set to 1 if $2 is statically built])
fi

enable_[]DOWN="no"
if test "x${want_module}" = "xyes" ; then
   enable_[]DOWN=${enable_module}
fi

m4_popdef([UP])
m4_popdef([DOWN])
])
