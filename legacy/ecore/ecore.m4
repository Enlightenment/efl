# Configure paths for Ecore
# Conrad Parker 2001-01-24
# blatantly ripped from gtk.m4, Owen Taylor     97-11-3

dnl AM_PATH_ECORE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for Ecore, and define ECORE_CFLAGS and ECORE_LIBS
dnl
AC_DEFUN(AM_PATH_ECORE,
[dnl 
dnl Get the cflags and libraries from the ecore-config script
dnl
AC_ARG_WITH(ecore-prefix,[  --with-ecore-prefix=PFX   Prefix where Ecore is installed (optional)],
            ecore_config_prefix="$withval", ecore_config_prefix="")
AC_ARG_WITH(ecore-exec-prefix,[  --with-ecore-exec-prefix=PFX Exec prefix where Ecore is installed (optional)],
            ecore_config_exec_prefix="$withval", ecore_config_exec_prefix="")
AC_ARG_ENABLE(ecoretest, [  --disable-ecoretest       Do not try to compile and run a test Ecore program],
		    , enable_ecoretest=yes)

  if test x$ecore_config_exec_prefix != x ; then
     ecore_config_args="$ecore_config_args --exec-prefix=$ecore_config_exec_prefix"
     if test x${ECORE_CONFIG+set} != xset ; then
        ECORE_CONFIG=$ecore_config_exec_prefix/bin/ecore-config
     fi
  fi
  if test x$ecore_config_prefix != x ; then
     ecore_config_args="$ecore_config_args --prefix=$ecore_config_prefix"
     if test x${ECORE_CONFIG+set} != xset ; then
        ECORE_CONFIG=$ecore_config_prefix/bin/ecore-config
     fi
  fi

  AC_PATH_PROG(ECORE_CONFIG, ecore-config, no)
  min_ecore_version=ifelse([$1], ,0.0.0,$1)
  AC_MSG_CHECKING(for Ecore - version >= $min_ecore_version)
  no_ecore=""
  if test "$ECORE_CONFIG" = "no" ; then
    no_ecore=yes
  else
    ECORE_CFLAGS=`$ECORE_CONFIG $ecore_config_args --cflags`
    ECORE_LIBS=`$ECORE_CONFIG $ecore_config_args --libs`
    ecore_config_major_version=`$ECORE_CONFIG $ecore_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    ecore_config_minor_version=`$ECORE_CONFIG $ecore_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    ecore_config_micro_version=`$ECORE_CONFIG $ecore_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

dnl **********************************************************************
dnl Ecore cannot currently report its own version ; version info is not
dnl given in Ecore.h
dnl Thus, the "enable_ecoretest" stuff is currently disabled, enable once
dnl you can determine the currently installed version by querying Ecore[.h]
dnl
dnl K. 2001-01-24
dnl **********************************************************************

dnl    if test "x$enable_ecoretest" = "xyes" ; then
dnl      ac_save_CFLAGS="$CFLAGS"
dnl      ac_save_LIBS="$LIBS"
dnl      CFLAGS="$CFLAGS $ECORE_CFLAGS"
dnl      LIBS="$ECORE_LIBS $LIBS"
dnl dnl
dnl dnl Now check if the installed Ecore is sufficiently new. (Also sanity
dnl dnl checks the results of ecore-config to some extent
dnl dnl
dnl       rm -f conf.ecoretest
dnl       AC_TRY_RUN([
dnl #include <Ecore.h>
dnl #include <stdio.h>
dnl #include <stdlib.h>
dnl 
dnl int 
dnl main ()
dnl {
dnl   int major, minor, micro;
dnl   char *tmp_version;
dnl 
dnl   system ("touch conf.ecoretest");
dnl 
dnl   /* HP/UX 9 (%@#!) writes to sscanf strings */
dnl   tmp_version = g_strdup("$min_ecore_version");
dnl   if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
dnl      printf("%s, bad version string\n", "$min_ecore_version");
dnl      exit(1);
dnl    }
dnl 
dnl   if ((ecore_major_version != $ecore_config_major_version) ||
dnl       (ecore_minor_version != $ecore_config_minor_version) ||
dnl       (ecore_micro_version != $ecore_config_micro_version))
dnl     {
dnl       printf("\n*** 'ecore-config --version' returned %d.%d.%d, but Ecore (%d.%d.%d)\n", 
dnl              $ecore_config_major_version, $ecore_config_minor_version, $ecore_config_micro_version,
dnl              ecore_major_version, ecore_minor_version, ecore_micro_version);
dnl       printf ("*** was found! If ecore-config was correct, then it is best\n");
dnl       printf ("*** to remove the old version of Ecore. You may also be able to fix the error\n");
dnl       printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
dnl       printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
dnl       printf("*** required on your system.\n");
dnl       printf("*** If ecore-config was wrong, set the environment variable ECORE_CONFIG\n");
dnl       printf("*** to point to the correct copy of ecore-config, and remove the file config.cache\n");
dnl       printf("*** before re-running configure\n");
dnl     } 
dnl #if defined (ECORE_MAJOR_VERSION) && defined (ECORE_MINOR_VERSION) && defined (ECORE_MICRO_VERSION)
dnl   else if ((ecore_major_version != ECORE_MAJOR_VERSION) ||
dnl 	   (ecore_minor_version != ECORE_MINOR_VERSION) ||
dnl            (ecore_micro_version != ECORE_MICRO_VERSION))
dnl     {
dnl       printf("*** Ecore header files (version %d.%d.%d) do not match\n",
dnl 	     ECORE_MAJOR_VERSION, ECORE_MINOR_VERSION, ECORE_MICRO_VERSION);
dnl       printf("*** library (version %d.%d.%d)\n",
dnl 	     ecore_major_version, ecore_minor_version, ecore_micro_version);
dnl     }
dnl #endif /* defined (ECORE_MAJOR_VERSION) ... */
dnl   else
dnl     {
dnl       if ((ecore_major_version > major) ||
dnl         ((ecore_major_version == major) && (ecore_minor_version > minor)) ||
dnl         ((ecore_major_version == major) && (ecore_minor_version == minor) && (ecore_micro_version >= micro)))
dnl       {
dnl         return 0;
dnl        }
dnl      else
dnl       {
dnl         printf("\n*** An old version of Ecore (%d.%d.%d) was found.\n",
dnl                ecore_major_version, ecore_minor_version, ecore_micro_version);
dnl         printf("*** You need a version of Ecore newer than %d.%d.%d. The latest version of\n",
dnl 	       major, minor, micro);
dnl         printf("*** Ecore is always available from ftp://ftp.enlightenment.org.\n");
dnl         printf("***\n");
dnl         printf("*** If you have already installed a sufficiently new version, this error\n");
dnl         printf("*** probably means that the wrong copy of the ecore-config shell script is\n");
dnl         printf("*** being found. The easiest way to fix this is to remove the old version\n");
dnl         printf("*** of Ecore, but you can also set the ECORE_CONFIG environment to point to the\n");
dnl         printf("*** correct copy of ecore-config. (In this case, you will have to\n");
dnl         printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
dnl         printf("*** so that the correct libraries are found at run-time))\n");
dnl       }
dnl     }
dnl   return 1;
dnl }
dnl ],, no_ecore=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
dnl        CFLAGS="$ac_save_CFLAGS"
dnl        LIBS="$ac_save_LIBS"
dnl      fi
dnl **********************************************************************

  fi
  if test "x$no_ecore" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$ECORE_CONFIG" = "no" ; then
       echo "*** The ecore-config script installed by Ecore could not be found"
       echo "*** If Ecore was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the ECORE_CONFIG environment variable to the"
       echo "*** full path to ecore-config."
     else
       if test -f conf.ecoretest ; then
        :
       else
          echo "*** Could not run Ecore test program, checking why..."
          CFLAGS="$CFLAGS $ECORE_CFLAGS"
          LIBS="$LIBS $ECORE_LIBS"
          AC_TRY_LINK([
#include <Ecore.h>
#include <stdio.h>
],      [ return ((ecore_major_version) || (ecore_minor_version) || (ecore_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding Ecore or finding the wrong"
          echo "*** version of Ecore. If it is not finding Ecore, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the Ecore package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps ecore ecore-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means Ecore was incorrectly installed"
          echo "*** or that you have moved Ecore since it was installed. In the latter case, you"
          echo "*** may want to edit the ecore-config script: $ECORE_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     ECORE_CFLAGS=""
     ECORE_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(ECORE_CFLAGS)
  AC_SUBST(ECORE_LIBS)
  rm -f conf.ecoretest
])
