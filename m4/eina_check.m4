dnl use: EINA_CHECK_MODULE(foo-bar, have_dependency, description)
AC_DEFUN([EINA_CHECK_MODULE],
[
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

# AC_ARG_ENABLE([mempool-$1],
#    [AC_HELP_STRING([--enable-mempool-$1], [enable build of $3 @<:@default=$2@:>@])],
#    [
#     if test "x${enableval}" = "xyes" ; then
#        enable_module="yes"
#     else
#        if test "x${enableval}" = "xstatic" ; then
#           enable_module="static"
#        else
#           enable_module="no"
#        fi
#     fi
#    ],
#    [enable_module=$2])
enable_module=$2

have_module="no"
if test "x${enable_module}" = "xyes" || test "x${enable_module}" = "xstatic" ; then
   have_module="yes"
fi

AC_MSG_CHECKING([whether to enable $3 built])
AC_MSG_RESULT([${have_module}])

static_module="no"
if test "x${enable_module}" = "xstatic" ; then
   static_module="yes"
   have_static_module="yes"
   AC_DEFINE(EINA_STATIC_BUILD_[]UP, 1, [Set to 1 if $2 is statically built])
fi

if ! test "x${enable_module}" = "xno" ; then
   AC_DEFINE(EINA_BUILD_[]UP, 1, [Set to 1 if $2 is built])
fi

AM_CONDITIONAL(EINA_BUILD_[]UP, [test "x${have_module}" = "xyes"])
AM_CONDITIONAL(EINA_STATIC_BUILD_[]UP, [test "x${static_module}" = "xyes"])

enable_[]DOWN=${enable_module}

m4_popdef([UP])
m4_popdef([DOWN])
])
