/*
 *
 * vim:ts=8:sw=3:sts=3:expandtab:cino=>5n-3f0^-2{2(0W1st0
 */

/**
@file Elementary.h.in
@brief Elementary Widget Library
*/

/**
@mainpage Elementary
@image html  elementary.png
@version 0.8.0
@date 2008-2011

@section intro What is Elementary?

This is a VERY SIMPLE toolkit. It is not meant for writing extensive desktop
applications (yet). Small simple ones with simple needs.

It is meant to make the programmers work almost brainless but give them lots
of flexibility.

@li @ref Start - Go here to quickly get started with writing Apps

@section organization Organization

One can divide Elemementary into three main groups:
@li @ref infralist - These are modules that deal with Elementary as a whole.
@li @ref widgetslist - These are the widgets you'll compose your UI out of.
@li @ref containerslist - These are the containers which hold the widgets.

@section license License

LGPL v2 (see COPYING in the base of Elementary's source). This applies to
all files in the source tree.

@section ack Acknowledgements
There is a lot that goes into making a widget set, and they don't happen out of
nothing. It's like trying to make everyone everywhere happy, regardless of age,
gender, race or nationality - and that is really tough. So thanks to people and
organisations behind this, as listed in the @ref authors page.
*/


/**
 * @defgroup Start Getting Started
 *
 * To write an Elementary app, you can get started with the following:
 *
@code
#include <Elementary.h>
EAPI_MAIN int
elm_main(int argc, char **argv)
{
   // create window(s) here and do any application init
   elm_run(); // run main loop
   elm_shutdown(); // after mainloop finishes running, shutdown
   return 0; // exit 0 for exit code
}
ELM_MAIN()
@endcode
 *
 * To use autotools (which helps in many ways in the long run, like being able
 * to immediately create releases of your software directly from your tree
 * and ensure everything needed to build it is there) you will need a
 * configure.ac, Makefile.am and autogen.sh file.
 *
 * configure.ac:
 *
@verbatim
AC_INIT(myapp, 0.0.0, myname@mydomain.com)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)
AM_CONFIG_HEADER(config.h)
AC_PROG_CC
AM_INIT_AUTOMAKE(1.6 dist-bzip2)
PKG_CHECK_MODULES([ELEMENTARY], elementary)
AC_OUTPUT(Makefile)
@endverbatim
 *
 * Makefile.am:
 *
@verbatim
AUTOMAKE_OPTIONS = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.h.in configure depcomp install-sh missing

INCLUDES = -I$(top_srcdir)

bin_PROGRAMS = myapp

myapp_SOURCES = main.c
myapp_LDADD = -pthread -L/usr/local/lib -leina -leet -levas -lecore -lecore_evas -lecore_file -ledje  
myapp_CFLAGS = -I/usr/local/include/eina-1 -I/usr/local/include/eina-1/eina -I/usr/local/include/eet-1 -I/usr/local/include/evas-1 -I/usr/local/include/ecore-1 -I/usr/local/include/edje-1 -I/usr/local/include -I/usr/local/include/embryo-1 -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/include/valgrind -I/usr/include/fribidi -I/usr/include/alsa -I/usr/include/lua5.1  
@endverbatim
 *
 * autogen.sh:
 *
@verbatim
#!/bin/sh
echo "Running aclocal..." ; aclocal $ACLOCAL_FLAGS || exit 1
echo "Running autoheader..." ; autoheader || exit 1
echo "Running autoconf..." ; autoconf || exit 1
echo "Running automake..." ; automake --add-missing --copy --gnu || exit 1
./configure "$@"
@endverbatim
 *
 * To generate all the things needed to bootstrap just run:
 *
@verbatim
./autogen.sh
@endverbatim
 *
 * This will generate Makefile.in's, the confgure script and everything else.
 * After this it works like all normal autotools projects:
@verbatim
./configure
make
sudo make install
@endverbatim
 *
 * Note sudo was assumed to get root permissions, as this would install in
 * /usr/local which is system-owned. Use any way you like to gain root, or
 * specify a different prefix with configure:
 *
@verbatim
./confiugre --prefix=$HOME/mysoftware
@endverbatim
 *
 * Also remember that autotools buys you some useful commands like:
@verbatim
make uninstall
@endverbatim
 *
 * This uninstalls the software after it was installed with "make install".
 * It is very useful to clear up what you built if you wish to clean the
 * system.
 *
@verbatim
make distcheck
@endverbatim
 *
 * This firstly checks if your build tree is "clean" and ready for
 * distribution. It also builds a tarball (myapp-0.0.0.tar.gz) that is
 * ready to upload and distribute to the world, that contains the generated
 * Makefile.in's and configure script. The users do not need to run
 * autogen.sh - just configure and on. They don't need autotools installed.
 * This tarball also builds cleanly, has all the sources it needs to build
 * included (that is sources for your application, not libraries it depends
 * on like Elementary). It builds cleanly in a buildroot and does not
 * contain any files that are temporarily generated like binaries and other
 * build-generated files, so the tarball is clean, and no need to worry
 * about cleaning up your tree before packaging.
 *
@verbatim
make clean
@endverbatim
 *
 * This cleans up all build files (binaries, objects etc.) from the tree.
 *
@verbatim
make distclean
@endverbatim
 *
 * This cleans out all files from the build and from configure's output too.
 *
@verbatim
make maintainer-clean
@endverbatim
 *
 * This deletes all the files autogen.sh will produce so the tree is clean
 * to be put into a revision-control system (like CVS, SVN or GIT for example).
 *
 * There is a more advanced way of making use of the quicklaunch infrastructure
 * in Elementary (which will not be covered here due to its more advanced
 * nature).
 *
 * Now let's actually create an interactive "Hello World" gui that you can
 * click the ok button to exit. It's more code because this now does something
 * much more significant, but it's still very simple:
 *
@code
#include <Elementary.h>

static void
on_done(void *data, Evas_Object *obj, void *event_info)
{
   // quit the mainloop (elm_run function will return)
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *box, *lab, *btn;

   // new window - do the usual and give it a name (hello) and title (Hello)
   win = elm_win_util_standard_add("hello", "Hello");
   // when the user clicks "close" on a window there is a request to delete
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   // add a box object - default is vertical. a box holds children in a row,
   // either horizontally or vertically. nothing more.
   box = elm_box_add(win);
   // make the box hotizontal
   elm_box_horizontal_set(box, EINA_TRUE);
   // add object as a resize object for the window (controls window minimum
   // size as well as gets resized if window is resized)
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   // add a label widget, set the text and put it in the pad frame
   lab = elm_label_add(win);
   // set default text of the label
   elm_object_text_set(lab, "Hello out there world!");
   // pack the label at the end of the box
   elm_box_pack_end(box, lab);
   evas_object_show(lab);

   // add an ok button
   btn = elm_button_add(win);
   // set default text of button to "OK"
   elm_object_text_set(btn, "OK");
   // pack the button at the end of the box
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   // call on_done when button is clicked
   evas_object_smart_callback_add(btn, "clicked", on_done, NULL);

   // now we are done, show the window
   evas_object_show(win);

   // run the mainloop and process events and callbacks
   elm_run();
   elm_shutdown();
   return 0;
}
ELM_MAIN()
@endcode
   *
   */

/**
@page authors Authors
@author Carsten Haitzler <raster@@rasterman.com>
@author Gustavo Sverzut Barbieri <barbieri@@profusion.mobi>
@author Cedric Bail <cedric.bail@@free.fr>
@author Vincent Torri <vtorri@@univ-evry.fr>
@author Daniel Kolesa <quaker66@@gmail.com>
@author Jaime Thomas <avi.thomas@@gmail.com>
@author Swisscom - http://www.swisscom.ch/
@author Christopher Michael <devilhorns@@comcast.net>
@author Marco Trevisan (Treviño) <mail@@3v1n0.net>
@author Michael Bouchaud <michael.bouchaud@@gmail.com>
@author Jonathan Atton (Watchwolf) <jonathan.atton@@gmail.com>
@author Brian Wang <brian.wang.0721@@gmail.com>
@author Mike Blumenkrantz (discomfitor/zmike) <michael.blumenkrantz@@gmail.com>
@author Samsung Electronics <tbd>
@author Samsung SAIT <tbd>
@author Brett Nash <nash@@nash.id.au>
@author Bruno Dilly <bdilly@@profusion.mobi>
@author Rafael Fonseca <rfonseca@@profusion.mobi>
@author Chuneon Park <hermet@@hermet.pe.kr>
@author Woohyun Jung <wh0705.jung@@samsung.com>
@author Jaehwan Kim <jae.hwan.kim@@samsung.com>
@author Wonguk Jeong <wonguk.jeong@@samsung.com>
@author Leandro A. F. Pereira <leandro@@profusion.mobi>
@author Helen Fornazier <helen.fornazier@@profusion.mobi>
@author Gustavo Lima Chaves <glima@@profusion.mobi>
@author Fabiano Fidêncio <fidencio@@profusion.mobi>
@author Tiago Falcão <tiago@@profusion.mobi>
@author Otavio Pontes <otavio@@profusion.mobi>
@author Viktor Kojouharov <vkojouharov@@gmail.com>
@author Daniel Juyung Seo (SeoZ) <juyung.seo@@samsung.com> <seojuyung2@@gmail.com>
@author Sangho Park <sangho.g.park@@samsung.com> <gouache95@@gmail.com>
@author Rajeev Ranjan (Rajeev) <rajeev.r@@samsung.com> <rajeev.jnnce@@gmail.com>
@author Seunggyun Kim <sgyun.kim@@samsung.com> <tmdrbs@@gmail.com>
@author Sohyun Kim <anna1014.kim@@samsung.com> <sohyun.anna@@gmail.com>
@author Jihoon Kim <jihoon48.kim@@samsung.com>
@author Jeonghyun Yun (arosis) <jh0506.yun@@samsung.com>
@author Tom Hacohen <tom@@stosb.com>
@author Aharon Hillel <a.hillel@@partner.samsung.com>
@author Jonathan Atton (Watchwolf) <jonathan.atton@@gmail.com>
@author Shinwoo Kim <kimcinoo@@gmail.com>
@author Govindaraju SM <govi.sm@@samsung.com> <govism@@gmail.com>
@author Prince Kumar Dubey <prince.dubey@@samsung.com> <prince.dubey@@gmail.com>
@author Sung W. Park <sungwoo@@gmail.com>
@author Thierry el Borgi <thierry@@substantiel.fr>
@author Shilpa Singh <shilpa.singh@@samsung.com> <shilpasingh.o@@gmail.com>
@author Chanwook Jung <joey.jung@@samsung.com>
@author Hyoyoung Chang <hyoyoung.chang@@samsung.com>
@author Guillaume "Kuri" Friloux <guillaume.friloux@@asp64.com>
@author Kim Yunhan <spbear@@gmail.com>
@author Bluezery <ohpowel@@gmail.com>
@author Nicolas Aguirre <aguirre.nicolas@@gmail.com>
@author Sanjeev BA <iamsanjeev@@gmail.com>

Please contact <enlightenment-devel@lists.sourceforge.net> to get in
contact with the developers and maintainers.
 */

#ifndef ELEMENTARY_H
#define ELEMENTARY_H

/**
 * @file Elementary.h
 * @brief Elementary's API
 *
 * Elementary API.
 */

#define ELM_UNIX
#undef ELM_WIN32
#undef ELM_WINCE
#define ELM_EDBUS
#define ELM_EFREET
#define ELM_ETHUMB
#undef ELM_WEB
#undef ELM_EMAP
#undef ELM_DEBUG
#define ELM_ALLOCA_H
#define ELM_LIBINTL_H
#define ELM_DIRENT_H

/* Standard headers for standard system calls etc. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <math.h>
#include <fnmatch.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#ifdef ELM_DIRENT_H
# include <dirent.h>
#endif
#include <pwd.h>
#include <errno.h>

#ifdef ELM_UNIX
# include <locale.h>
# ifdef ELM_LIBINTL_H
#  include <libintl.h>
# endif
# include <signal.h>
# include <grp.h>
# include <glob.h>
#endif

#ifdef ELM_ALLOCA_H
# include <alloca.h>
#endif

#if defined (ELM_WIN32) || defined (ELM_WINCE)
# include <malloc.h>
# ifndef alloca
#  define alloca _alloca
# endif
#endif


/* EFL headers */
#include <Eina.h>
#include <Eet.h>
#include <Evas.h>
#include <Evas_GL.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Ecore_IMF.h>
#include <Ecore_Con.h>
#include <Edje.h>

#ifdef ELM_EDBUS
# include <E_DBus.h>
#endif

#ifdef ELM_EFREET
# include <Efreet.h>
# include <Efreet_Mime.h>
# include <Efreet_Trash.h>
#endif

#ifdef ELM_ETHUMB
# include <Ethumb_Client.h>
#endif

#ifdef ELM_EMAP
# include <EMap.h>
#endif

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef ELEMENTARY_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef _WIN32
# define EAPI_MAIN
#else
# define EAPI_MAIN EAPI
#endif

/* allow usage from c++ */
#ifdef __cplusplus
extern "C" {
#endif

#define ELM_VERSION_MAJOR 0
#define ELM_VERSION_MINOR 8

   typedef struct _Elm_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Elm_Version;

   EAPI extern Elm_Version *elm_version;

/* include these first for general used definitions */
#include <elm_general.h>
#include <elm_object_item.h>
#include <elm_tooltip.h>
   
/* special widgets - types used elsewhere */   
#include <elm_icon.h>
#include <elm_scroller.h>
#include <elm_entry.h>
#include <elm_list.h>
   
/* other includes */
#include <elc_anchorblock.h>
#include <elc_anchorview.h>
#include <elc_ctxpopup.h>
#include <elc_fileselector_button.h>
#include <elc_fileselector_entry.h>
#include <elc_fileselector.h>
#include <elc_hoversel.h>
#include <elc_multibuttonentry.h>
#include <elc_naviframe.h>
#include <elm_actionslider.h>
#include <elm_app.h>
#include <elm_bg.h>
#include <elm_box.h>
#include <elm_bubble.h>
#include <elm_button.h>
#include <elm_cache.h>
#include <elm_calendar.h>
#include <elm_check.h>
#include <elm_clock.h>
#include <elm_cnp.h>
#include <elm_colorselector.h>
#include <elm_config.h>
#include <elm_conform.h>
#include <elm_cursor.h>
#include <elm_debug.h>
#include <elm_deprecated.h>
#include <elm_diskselector.h>
#include <elm_engine.h>
#include <elm_factory.h>
#include <elm_finger.h>
#include <elm_flip.h>
#include <elm_flipselector.h>
#include <elm_focus.h>
#include <elm_fonts.h>
#include <elm_frame.h>
#include <elm_gengrid.h>
#include <elm_genlist.h>
#include <elm_gesture_layer.h>
#include <elm_glview.h>
#include <elm_grid.h>
#include <elm_hover.h>
#include <elm_image.h>
#include <elm_index.h>
#include <elm_label.h>
#include <elm_layout.h>
#include <elm_macros.h>
#include <elm_mapbuf.h>
#include <elm_map.h>
#include <elm_menu.h>
#include <elm_mirroring.h>
#include <elm_need.h>
#include <elm_notify.h>
#include <elm_object.h>
#include <elm_pager.h>
#include <elm_panel.h>
#include <elm_panes.h>
#include <elm_password.h>
#include <elm_photocam.h>
#include <elm_photo.h>
#include <elm_progressbar.h>
#include <elm_radio.h>
#include <elm_route.h>
#include <elm_scale.h>
#include <elm_scroll.h>
#include <elm_segment_control.h>
#include <elm_separator.h>
#include <elm_slider.h>
#include <elm_slideshow.h>
#include <elm_spinner.h>
#include <elm_store.h>
#include <elm_table.h>
#include <elm_theme.h>
#include <elm_thumb.h>
#include <elm_toolbar.h>
#include <elm_transit.h>
#include <elm_video.h>
#include <elm_web.h>
#include <elm_win.h>
   
#ifdef __cplusplus
}
#endif

#endif
