# Configure paths for Evas
# Conrad Parker 2001-01-23
# blatantly ripped from gtk.m4, Owen Taylor     97-11-3

dnl AM_PATH_EVAS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for Evas, and define EVAS_CFLAGS and EVAS_LIBS
dnl
AC_DEFUN(AM_PATH_EVAS,
[dnl 
dnl Get the cflags and libraries from the evas-config script
dnl
AC_ARG_WITH(evas-prefix,[  --with-evas-prefix=PFX   Prefix where Evas is installed (optional)],
            evas_config_prefix="$withval", evas_config_prefix="")
AC_ARG_WITH(evas-exec-prefix,[  --with-evas-exec-prefix=PFX Exec prefix where Evas is installed (optional)],
            evas_config_exec_prefix="$withval", evas_config_exec_prefix="")
AC_ARG_ENABLE(evastest, [  --disable-evastest       Do not try to compile and run a test Evas program],
		    , enable_evastest=yes)

  if test x$evas_config_exec_prefix != x ; then
     evas_config_args="$evas_config_args --exec-prefix=$evas_config_exec_prefix"
     if test x${EVAS_CONFIG+set} != xset ; then
        EVAS_CONFIG=$evas_config_exec_prefix/bin/evas-config
     fi
  fi
  if test x$evas_config_prefix != x ; then
     evas_config_args="$evas_config_args --prefix=$evas_config_prefix"
     if test x${EVAS_CONFIG+set} != xset ; then
        EVAS_CONFIG=$evas_config_prefix/bin/evas-config
     fi
  fi

  AC_PATH_PROG(EVAS_CONFIG, evas-config, no)
  min_evas_version=ifelse([$1], ,0.0.3,$1)
  AC_MSG_CHECKING(for Evas - version >= $min_evas_version)
  no_evas=""
  if test "$EVAS_CONFIG" = "no" ; then
    no_evas=yes
  else
    EVAS_CFLAGS=`$EVAS_CONFIG $evas_config_args --cflags`
    EVAS_LIBS=`$EVAS_CONFIG $evas_config_args --libs`
    evas_config_major_version=`$EVAS_CONFIG $evas_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    evas_config_minor_version=`$EVAS_CONFIG $evas_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    evas_config_micro_version=`$EVAS_CONFIG $evas_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

dnl **********************************************************************
dnl Evas cannot currently report its own version ; version info is not
dnl given in Evas.h
dnl Thus, the "enable_evastest" stuff is currently disabled, enable once
dnl you can determine the currently installed version by querying Evas[.h]
dnl
dnl K. 2001-01-23
dnl **********************************************************************

dnl    if test "x$enable_evastest" = "xyes" ; then
dnl      ac_save_CFLAGS="$CFLAGS"
dnl      ac_save_LIBS="$LIBS"
dnl      CFLAGS="$CFLAGS $EVAS_CFLAGS"
dnl      LIBS="$EVAS_LIBS $LIBS"
dnl dnl
dnl dnl Now check if the installed Evas is sufficiently new. (Also sanity
dnl dnl checks the results of evas-config to some extent
dnl dnl
dnl       rm -f conf.evastest
dnl       AC_TRY_RUN([
dnl #include <Evas.h>
dnl #include <stdio.h>
dnl #include <stdlib.h>
dnl 
dnl int 
dnl main ()
dnl {
dnl   int major, minor, micro;
dnl   char *tmp_version;
dnl 
dnl   system ("touch conf.evastest");
dnl 
dnl   /* HP/UX 9 (%@#!) writes to sscanf strings */
dnl   tmp_version = g_strdup("$min_evas_version");
dnl   if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
dnl      printf("%s, bad version string\n", "$min_evas_version");
dnl      exit(1);
dnl    }
dnl 
dnl   if ((evas_major_version != $evas_config_major_version) ||
dnl       (evas_minor_version != $evas_config_minor_version) ||
dnl       (evas_micro_version != $evas_config_micro_version))
dnl     {
dnl       printf("\n*** 'evas-config --version' returned %d.%d.%d, but Evas (%d.%d.%d)\n", 
dnl              $evas_config_major_version, $evas_config_minor_version, $evas_config_micro_version,
dnl              evas_major_version, evas_minor_version, evas_micro_version);
dnl       printf ("*** was found! If evas-config was correct, then it is best\n");
dnl       printf ("*** to remove the old version of Evas. You may also be able to fix the error\n");
dnl       printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
dnl       printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
dnl       printf("*** required on your system.\n");
dnl       printf("*** If evas-config was wrong, set the environment variable EVAS_CONFIG\n");
dnl       printf("*** to point to the correct copy of evas-config, and remove the file config.cache\n");
dnl       printf("*** before re-running configure\n");
dnl     } 
dnl #if defined (EVAS_MAJOR_VERSION) && defined (EVAS_MINOR_VERSION) && defined (EVAS_MICRO_VERSION)
dnl   else if ((evas_major_version != EVAS_MAJOR_VERSION) ||
dnl 	   (evas_minor_version != EVAS_MINOR_VERSION) ||
dnl            (evas_micro_version != EVAS_MICRO_VERSION))
dnl     {
dnl       printf("*** Evas header files (version %d.%d.%d) do not match\n",
dnl 	     EVAS_MAJOR_VERSION, EVAS_MINOR_VERSION, EVAS_MICRO_VERSION);
dnl       printf("*** library (version %d.%d.%d)\n",
dnl 	     evas_major_version, evas_minor_version, evas_micro_version);
dnl     }
dnl #endif /* defined (EVAS_MAJOR_VERSION) ... */
dnl   else
dnl     {
dnl       if ((evas_major_version > major) ||
dnl         ((evas_major_version == major) && (evas_minor_version > minor)) ||
dnl         ((evas_major_version == major) && (evas_minor_version == minor) && (evas_micro_version >= micro)))
dnl       {
dnl         return 0;
dnl        }
dnl      else
dnl       {
dnl         printf("\n*** An old version of Evas (%d.%d.%d) was found.\n",
dnl                evas_major_version, evas_minor_version, evas_micro_version);
dnl         printf("*** You need a version of Evas newer than %d.%d.%d. The latest version of\n",
dnl 	       major, minor, micro);
dnl         printf("*** Evas is always available from ftp://ftp.enlightenment.org.\n");
dnl         printf("***\n");
dnl         printf("*** If you have already installed a sufficiently new version, this error\n");
dnl         printf("*** probably means that the wrong copy of the evas-config shell script is\n");
dnl         printf("*** being found. The easiest way to fix this is to remove the old version\n");
dnl         printf("*** of Evas, but you can also set the EVAS_CONFIG environment to point to the\n");
dnl         printf("*** correct copy of evas-config. (In this case, you will have to\n");
dnl         printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
dnl         printf("*** so that the correct libraries are found at run-time))\n");
dnl       }
dnl     }
dnl   return 1;
dnl }
dnl ],, no_evas=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
dnl        CFLAGS="$ac_save_CFLAGS"
dnl        LIBS="$ac_save_LIBS"
dnl      fi
dnl **********************************************************************

  fi
  if test "x$no_evas" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$EVAS_CONFIG" = "no" ; then
       echo "*** The evas-config script installed by Evas could not be found"
       echo "*** If Evas was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the EVAS_CONFIG environment variable to the"
       echo "*** full path to evas-config."
     else
       if test -f conf.evastest ; then
        :
       else
          echo "*** Could not run Evas test program, checking why..."
          CFLAGS="$CFLAGS $EVAS_CFLAGS"
          LIBS="$LIBS $EVAS_LIBS"
          AC_TRY_LINK([
#include <Evas.h>
#include <stdio.h>
],      [ return ((evas_major_version) || (evas_minor_version) || (evas_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding Evas or finding the wrong"
          echo "*** version of Evas. If it is not finding Evas, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the Evas package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps evas evas-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means Evas was incorrectly installed"
          echo "*** or that you have moved Evas since it was installed. In the latter case, you"
          echo "*** may want to edit the evas-config script: $EVAS_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     EVAS_CFLAGS=""
     EVAS_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(EVAS_CFLAGS)
  AC_SUBST(EVAS_LIBS)
  rm -f conf.evastest
])
