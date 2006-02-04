dnl Perform a check for a GStreamer element using gst-inspect
dnl Thomas Vander Stichele <thomas at apestaart dot org>
dnl Last modification: 25/01/2005
dnl
dnl Modified by Vincent Torri <vtorri at univ-evry dot fr>
dnl Last modification: 19/10/2005

dnl AM_GST_ELEMENT_CHECK(ELEMENT-NAME, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)

AC_DEFUN([AM_GST_ELEMENT_CHECK],
[
  if test "x$GST_INSPECT" == "x"; then
    AC_CHECK_PROG(GST_INSPECT, gst-inspect, gst-inspect, [])
  fi

  if test "x$GST_INSPECT" != "x"; then
    AC_MSG_CHECKING(GStreamer element $1 with $GST_INSPECT)
    if [ $GST_INSPECT $1 > /dev/null 2> /dev/null ]; then
      AC_MSG_RESULT(found.)
      $2
    else
      AC_MSG_RESULT(not found.)
      AC_CHECK_PROG(GST_INSPECT_0_8, gst-inspect-0.8, gst-inspect-0.8, [])

      if test "x$GST_INSPECT_0_8" != "x"; then
        AC_MSG_CHECKING(GStreamer element $1 with $GST_INSPECT_0_8)
        if [ $GST_INSPECT_0_8 $1 > /dev/null 2> /dev/null ]; then
          AC_MSG_RESULT(found.)
          $2
        else
          AC_MSG_RESULT(not found.)
          $3
        fi
      fi
    fi
  fi
])
