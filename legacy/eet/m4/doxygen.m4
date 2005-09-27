dnl Configure script for doxygen
dnl Vincent Torri 2005-22-09
dnl
dnl AM_CHECK_DOXYGEN([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for the doxygen program, and define BUILD_DOCS and DOXYGEN.
dnl
AC_DEFUN([AM_CHECK_DOXYGEN],
   [
    DOXYGEN="doxygen"
    dnl
    dnl Disable the build of the documentation
    dnl
    AC_ARG_ENABLE(
       [build_docs],
       AC_HELP_STRING(
          [--disable-build-docs],
          [Disable the build of the documentation]),
       [if test "${disable_build_docs}" = "yes" ; then
           enable_build_docs="no"
        else
           enable_build_docs="yes"
        fi],
       [enable_build_docs="yes"])
    dnl
    dnl Get the prefix where doxygen is installed.
    dnl
    AC_ARG_WITH(
       [doxygen],
       AC_HELP_STRING(
          [--with-doxygen=FILE],
          [doxygen program to use (eg /usr/bin/doxygen)]),
       dnl
       dnl Check the given doxygen program.
       dnl
       [DOXYGEN=${withval}
        AC_CHECK_PROG(
           [BUILD_DOCS],
           [${DOXYGEN}],
           [yes],
           [no])
        if test $BUILD_DOCS = no; then
           echo "WARNING:"
           echo "The doxygen program you specified:"
           echo "$DOXYGEN"
           echo "was not found.  Please check the path and make sure "
           echo "the program exists and is executable."
           AC_MSG_WARN(
              [Warning: no doxygen detected. Documentation will not be built])
        fi],
       [AC_CHECK_PROG(
           [BUILD_DOCS],
           [${DOXYGEN}],
           [yes],
           [no])
        if test ${BUILD_DOCS} = no; then
           echo "WARNING:"
           echo "The doxygen program was not found in your execute"
           echo "You may have doxygen installed somewhere not covered by your path."
           echo ""
           echo "If this is the case make sure you have the packages installed, AND"
           echo "that the doxygen program is in your execute path (see your"
           echo "shell's manual page on setting the \$PATH environment variable), OR"
           echo "alternatively, specify the program to use with --with-doxygen."
           AC_MSG_WARN(
              [Warning: no doxygen detected. Documentation will not be built])
        fi])
    dnl
    dnl Substitution
    dnl
    AC_SUBST([DOXYGEN])
    AM_CONDITIONAL(BUILD_DOCS, test "x$BUILD_DOCS" = "xyes")
   ])
dnl End of doxygen.m4
