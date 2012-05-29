/**
 * @defgroup Start Getting Started
 * @ingroup Elementary
 *
 * To write an Elementary app, you can get started with the following:
 *
 * @code
 * #include <Elementary.h>
 * EAPI_MAIN int
 * elm_main(int argc, char **argv)
 * {
 *    // create window(s) here and do any application init
 *    elm_run(); // run main loop
 *    elm_shutdown(); // after mainloop finishes running, shutdown
 *    return 0; // exit 0 for exit code
 * }
 * ELM_MAIN()
 * @endcode
 *
 * To use autotools (which helps in many ways in the long run, like being able
 * to immediately create releases of your software directly from your tree
 * and ensure everything needed to build it is there) you will need a
 * configure.ac, Makefile.am and autogen.sh file.
 *
 * configure.ac:
 *
 * @code
 * AC_INIT(myapp, 0.0.0, myname@mydomain.com)
 * AC_PREREQ(2.52)
 * AC_CONFIG_SRCDIR(configure.ac)
 * AM_CONFIG_HEADER(config.h)
 * AC_PROG_CC
 * AM_INIT_AUTOMAKE(1.6 dist-bzip2)
 * PKG_CHECK_MODULES([ELEMENTARY], elementary)
 * AC_OUTPUT(Makefile)
 * @endcode
 *
 * Makefile.am:
 *
 * @code
 * AUTOMAKE_OPTIONS = 1.4 foreign
 * MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.h.in configure depcomp install-sh missing
 *
 * INCLUDES = -I$(top_srcdir)
 *
 * bin_PROGRAMS = myapp
 *
 * myapp_SOURCES = main.c
 * myapp_LDADD = @ELEMENTARY_LIBS@
 * myapp_CFLAGS = @ELEMENTARY_CFLAGS@
 * @endcode
 *
 * autogen.sh:
 *
 * @code
 *#!/bin/sh
 * echo "Running aclocal..." ; aclocal $ACLOCAL_FLAGS || exit 1
 * echo "Running autoheader..." ; autoheader || exit 1
 * echo "Running autoconf..." ; autoconf || exit 1
 * echo "Running automake..." ; automake --add-missing --copy --gnu || exit 1
 * ./configure "$@"
 * @endcode
 *
 * To generate all the things needed to bootstrap just run:
 *
 * @code
 * ./autogen.sh
 * @endcode
 *
 * This will generate Makefile.in's, the configure script and everything else.
 * After this it works like all normal autotools projects:
 * @code
 * ./configure
 * make
 * sudo make install
 * @endcode
 *
 * Note sudo was assumed to get root permissions, as this would install in
 * /usr/local which is system-owned. Use any way you like to gain root, or
 * specify a different prefix with configure:
 *
 * @code
 * ./configure --prefix=$HOME/mysoftware
 * @endcode
 *
 * Also remember that autotools buys you some useful commands like:
 * @code
 * make uninstall
 * @endcode
 *
 * This uninstalls the software after it was installed with "make install".
 * It is very useful to clear up what you built if you wish to clean the
 * system.
 *
 * @code
 * make distcheck
 * @endcode
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
 * @code
 * make clean
 * @endcode
 *
 * This cleans up all build files (binaries, objects etc.) from the tree.
 *
 * @code
 * make distclean
 * @endcode
 *
 * This cleans out all files from the build and from configure's output too.
 *
 * @code
 * make maintainer-clean
 * @endcode
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
 * @code
 * #include <Elementary.h>
 *
 * static void
 * on_done(void *data, Evas_Object *obj, void *event_info)
 * {
 *    // quit the mainloop (elm_run function will return)
 *    elm_exit();
 * }
 *
 * EAPI_MAIN int
 * elm_main(int argc, char **argv)
 * {
 *    Evas_Object *win, *box, *lab, *btn;
 *
 *    // new window - do the usual and give it a name (hello) and title (Hello)
 *    win = elm_win_util_standard_add("hello", "Hello");
 *    // when the user clicks "close" on a window there is a request to delete
 *    evas_object_smart_callback_add(win, "delete,request", on_done, NULL);
 *
 *    // add a box object - default is vertical. a box holds children in a row,
 *    // either horizontally or vertically. nothing more.
 *    box = elm_box_add(win);
 *    // make the box horizontal
 *    elm_box_horizontal_set(box, EINA_TRUE);
 *    // add object as a resize object for the window (controls window minimum
 *    // size as well as gets resized if window is resized)
 *    elm_win_resize_object_add(win, box);
 *    evas_object_show(box);
 *
 *    // add a label widget, set the text and put it in the pad frame
 *    lab = elm_label_add(win);
 *    // set default text of the label
 *    elm_object_text_set(lab, "Hello out there world!");
 *    // pack the label at the end of the box
 *    elm_box_pack_end(box, lab);
 *    evas_object_show(lab);
 *
 *    // add an ok button
 *    btn = elm_button_add(win);
 *    // set default text of button to "OK"
 *    elm_object_text_set(btn, "OK");
 *    // pack the button at the end of the box
 *    elm_box_pack_end(box, btn);
 *    evas_object_show(btn);
 *    // call on_done when button is clicked
 *    evas_object_smart_callback_add(btn, "clicked", on_done, NULL);
 *
 *    // now we are done, show the window
 *    evas_object_show(win);
 *
 *    // run the mainloop and process events and callbacks
 *    elm_run();
 *    elm_shutdown();
 *    return 0;
 * }
 * ELM_MAIN()
 * @endcode
 *
 */
