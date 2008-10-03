dnl Copyright (C) 2008 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if several ASM instruction sets are available or not.

dnl Usage: EFL_CHECK_CPU_MMX([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Add -mmmx to EFL_SIMD_FLAGS if the compiler supports it and call AC_SUBST(EFL_SIMD_FLAGS)
dnl Define EFL_HAVE_MMX

AC_DEFUN([EFL_CHECK_CPU_MMX],
[

dnl configure option

AC_ARG_ENABLE([cpu-mmx],
   [AC_HELP_STRING([--enable-cpu-mmx], [enable mmx code @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_cpu_mmx="yes"
    else
       _efl_enable_cpu_mmx="no"
    fi
   ],
   [_efl_enable_cpu_mmx="yes"]
)
AC_MSG_CHECKING([whether to build mmx code])
AC_MSG_RESULT([${_efl_enable_cpu_mmx}])

dnl check if the CPU is supporting MMX instruction sets

_efl_build_cpu_mmx="no"
if test "x${_efl_enable_cpu_mmx}" = "xyes" ; then
case $host_cpu in
  i*86)
    _efl_build_cpu_mmx="yes"
    ;;
  x86_64)
    _efl_build_cpu_mmx="yes"
    ;;
esac
fi

dnl check if the compiler supports -mmmx

SAVE_CFLAGS=${CFLAGS}
CFLAGS="-mmmx"
AC_LANG_PUSH([C])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([[]],
                    [[]])
   ],
   [EFL_SIMD_FLAGS="${EFL_SIMD_FLAGS} -mmmx"]
)

AC_LANG_POP([C])
CFLAGS=${SAVE_CFLAGS}

AC_SUBST(EFL_SIMD_FLAGS)

if test "x${_efl_build_cpu_mmx}" = "xyes" ; then
   AC_DEFINE(EFL_HAVE_MMX, 1, [Define to mention that MMX is supported])
fi

if test "x${_efl_build_cpu_mmx}" = "xyes" ; then
   ifelse([$1], , :, [$1])
else
   ifelse([$2], , :, [$2])
fi
])

dnl Usage: EFL_CHECK_CPU_SSE([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Add -msse to EFL_SIMD_FLAGS if the compiler supports it and call AC_SUBST(EFL_SIMD_FLAGS)
dnl Define EFL_HAVE_SSE

AC_DEFUN([EFL_CHECK_CPU_SSE],
[

dnl configure option

AC_ARG_ENABLE([cpu-sse],
   [AC_HELP_STRING([--enable-cpu-sse], [enable sse code @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_cpu_sse="yes"
    else
       _efl_enable_cpu_sse="no"
    fi
   ],
   [_efl_enable_cpu_sse="yes"]
)
AC_MSG_CHECKING([whether to build sse code])
AC_MSG_RESULT([${_efl_enable_cpu_sse}])

dnl check if the CPU is supporting SSE instruction sets

_efl_build_cpu_sse="no"
if test "x${_efl_enable_cpu_sse}" = "xyes" ; then
case $host_cpu in
  i*86)
    _efl_build_cpu_sse="yes"
    ;;
  x86_64)
    _efl_build_cpu_sse="yes"
    ;;
esac
fi

dnl check if the compiler supports -msse

SAVE_CFLAGS=${CFLAGS}
CFLAGS="-msse"
AC_LANG_PUSH([C])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([[]],
                    [[]])
   ],
   [EFL_SIMD_FLAGS="${EFL_SIMD_FLAGS} -msse"]
)

AC_LANG_POP([C])
CFLAGS=${SAVE_CFLAGS}

AC_SUBST(EFL_SIMD_FLAGS)

if test "x${_efl_build_cpu_sse}" = "xyes" ; then
   AC_DEFINE(EFL_HAVE_SSE, 1, [Define to mention that SSE is supported])
fi

if test "x${_efl_build_cpu_sse}" = "xyes" ; then
   ifelse([$1], , :, [$1])
else
   ifelse([$2], , :, [$2])
fi
])

dnl Usage: EFL_CHECK_CPU_SSE2([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Add -msse2 to EFL_SIMD_FLAGS if the compiler supports it and call AC_SUBST(EFL_SIMD_FLAGS)
dnl Define EFL_HAVE_SSE2

AC_DEFUN([EFL_CHECK_CPU_SSE2],
[

dnl configure option

AC_ARG_ENABLE([cpu-sse2],
   [AC_HELP_STRING([--enable-cpu-sse2], [enable sse2 code @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_cpu_sse2="yes"
    else
       _efl_enable_cpu_sse2="no"
    fi
   ],
   [_efl_enable_cpu_sse2="yes"]
)
AC_MSG_CHECKING([whether to build sse2 code])
AC_MSG_RESULT([${_efl_enable_cpu_sse2}])

dnl check if the CPU is supporting SSE2 instruction sets

_efl_build_cpu_sse2="no"
if test "x${_efl_enable_cpu_sse2}" = "xyes" ; then
case $host_cpu in
  i686)
    _efl_build_cpu_sse2="yes"
    ;;
  x86_64)
    _efl_build_cpu_sse2="yes"
    ;;
esac
fi

dnl check if the compiler supports -msse2

SAVE_CFLAGS=${CFLAGS}
CFLAGS="-msse2"
AC_LANG_PUSH([C])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([[]],
                    [[]])
   ],
   [EFL_SIMD_FLAGS="${EFL_SIMD_FLAGS} -msse2"]
)

AC_LANG_POP([C])
CFLAGS=${SAVE_CFLAGS}

AC_SUBST(EFL_SIMD_FLAGS)

if test "x${_efl_build_cpu_sse2}" = "xyes" ; then
   AC_DEFINE(EFL_HAVE_SSE2, 1, [Define to mention that SSE2 is supported])
fi

if test "x${_efl_build_cpu_sse2}" = "xyes" ; then
   ifelse([$1], , :, [$1])
else
   ifelse([$2], , :, [$2])
fi
])

dnl Usage: EFL_CHECK_CPU_ALTIVEC([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Add -faltivec or -maltivec to EFL_SIMD_FLAGS if the compiler supports it and
dnl call AC_SUBST(EFL_SIMD_FLAGS)
dnl Define EFL_HAVE_ALTIVEC

AC_DEFUN([EFL_CHECK_CPU_ALTIVEC],
[

dnl configure option

AC_ARG_ENABLE([cpu-altivec],
   [AC_HELP_STRING([--enable-cpu-altivec], [enable altivec code @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       _efl_enable_cpu_altivec="yes"
    else
       _efl_enable_cpu_altivec="no"
    fi
   ],
   [_efl_enable_cpu_altivec="yes"]
)
AC_MSG_CHECKING([whether to build altivec code])
AC_MSG_RESULT([${_efl_enable_cpu_altivec}])

dnl check if the CPU is supporting ALTIVEC instruction sets

_efl_build_cpu_altivec="no"
if test "x${_efl_enable_cpu_altivec}" = "xyes" ; then
case $host_cpu in
  *power* | *ppc*)
    _efl_build_cpu_altivec="yes"
    ;;
esac
fi

dnl check if the compiler supports -faltivec or -maltivec and
dnl if altivec.h is available.

SAVE_CFLAGS=${CFLAGS}
CFLAGS="-faltivec"
AC_LANG_PUSH([C])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([[
#include <altivec.h>
                    ]],
                    [[]])],
   [_efl_have_faltivec="yes"
    _efl_altivec_flag="-faltivec"],
   [_efl_have_faltivec="no"]
)

if test "x${_efl_have_faltivec}" = "xno" ; then
   CFLAGS="-maltivec"

   AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM([[
#include <altivec.h>
                       ]],
                       [[]])],
      [_efl_have_faltivec="yes"
       _efl_altivec_flag="-maltivec"],
      [_efl_have_faltivec="no"]
   )
fi

AC_MSG_CHECKING([whether altivec code is supported])
AC_MSG_RESULT([${_efl_have_faltivec}])

AC_LANG_POP([C])
CFLAGS=${SAVE_CFLAGS}

EFL_SIMD_FLAGS="${EFL_SIMD_FLAGS} ${_efl_altivec_flag}"
AC_SUBST(EFL_SIMD_FLAGS)

if test "x${_efl_have_faltivec}" = "xyes" ; then
   AC_DEFINE(EFL_HAVE_ALTIVEC, 1, [Define to mention that ALTIVEC is supported])
fi

if test "x${_efl_have_faltivec}" = "xyes" ; then
   ifelse([$1], , :, [$1])
else
   ifelse([$2], , :, [$2])
fi
])
