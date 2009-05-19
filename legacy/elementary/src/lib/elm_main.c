#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <dlfcn.h>      /* dlopen,dlclose,etc */

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Start Getting Started
 * 
 * To write an Elementary app, you can get started with the following:
 * 
 * @code
 * #include <Elementary.h>
 * #ifndef ELM_LIB_QUICKLAUNCH
 * EAPI int
 * elm_main(int argc, char **argv)
 * {
 *    // create window(s) here and do any application init
 *    elm_run(); // run main loop
 *    elm_shutdown(); // after mainloop finishes running, shutdown
 *    return 0; // exit 0 for exit code
 * }
 * #endif
 * ELM_MAIN()
 * @endcode
 * 
 * To take full advantage of the quicklaunch architecture for launching
 * processes as quickly as possible (saving time at startup time like
 * connecting to X11, loading and linking shared libraries) you may want to
 * use the following configure.in/configure.ac and Makefile.am and autogen.sh
 * script to generate your files. It is assumed your application uses the
 * main.c file for its code.
 * 
 * configure.in/configure.ac:
 * 
@verbatim
AC_INIT(myapp, 0.0.0, myname@mydomain.com)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.in)

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([ELEMENTARY], elementary)

AC_OUTPUT(Makefile)
@endverbatim
 * 
 * Makefile.am:
 * 
@verbatim
AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir) @ELEMENTARY_CFLAGS@

bin_PROGRAMS      = myapp
myapp_LTLIBRARIES = myapp.la

myappdir = $(libdir)

myapp_la_SOURCES = main.c
myapp_la_LIBADD = @ELEMENTARY_LIBS@
myapp_la_CFLAGS =
myapp_la_LDFLAGS = -module -avoid-version -no-undefined

myapp_SOURCES = main.c
myapp_LDADD = @ELEMENTARY_LIBS@
myapp_CFLAGS = -DELM_LIB_QUICKLAUNCH=1
@endverbatim
 * 
 * autogen.sh:
 * 
@verbatim
#!/bin/sh
rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh
rm -rf m4
mkdir m4

touch README
echo "Running aclocal..." ; aclocal $ACLOCAL_FLAGS -I m4 || exit 1
echo "Running autoheader..." ; autoheader || exit 1
echo "Running autoconf..." ; autoconf || exit 1
echo "Running libtoolize..." ; (libtoolize --copy --automake || glibtoolize --automake) || exit 1
echo "Running automake..." ; automake --add-missing --copy --gnu || exit 1

if [ -z "$NOCONFIGURE" ]; then
  ./configure "$@"
fi
@endverbatim
 * 
 * The above will build a library - libmyapp.so and install in the target
 * library directory (default is /usr/local/lib). You will also get a
 * myapp.a and myapp.la - these are useless and can be deleted. Libtool likes
 * to generate these all the time. You will also get a binary in the target
 * binary directory (default is /usr/local/bin). This is a "debug binary".
 * This will run and dlopen() the myapp.so and then jump to it's elm_main
 * function. This allows for easy debugging with GDB and Valgrind. When you
 * are ready to go to production do the following:
 * 
 * 1. delete the myapp binary. i.e. rm /usr/local/bin/myapp
 * 
 * 2. symlink the myapp binary to elementary_run (supplied by elementary).
 * i.e. ln -s elmentary_run /usr/local/bin/myapp
 * 
 * 3. run elementary_quicklaunch as part of your graphical login session and
 * keep it running.
 * 
 * This will man elementary_quicklaunch does pre-initialization before the
 * application needs to be run, saving the effort at the time the application
 * is needed, thus speeding up the time it takes to appear.
 * 
 * If you don't want to use the quicklaunch infrastructure (which is
 * optional), you can execute the old fashioned way by just running the
 * myapp binary loader than will load the myapp.so for you, or you can
 * remove the split-file binary and put it into one binary as things always
 * have been with the following configure.in/configure.ac and Makfile.am
 * files:
 * 
 * configure.in/configure.ac:
 * 
@verbatim
AC_INIT(myapp, 0.0.0, myname@mydomain.com)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.in)

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST

PKG_CHECK_MODULES([ELEMENTARY], elementary)

AC_OUTPUT(Makefile)
@endverbatim
 * 
 * Makefile.am:
 * 
@verbatim
AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir) @ELEMENTARY_CFLAGS@

bin_PROGRAMS      = myapp

myapp_SOURCES = main.c
myapp_LDADD = @ELEMENTARY_LIBS@
myapp_CFLAGS = 
@endverbatim
 * 
 * Notice that they ae the same as before, just with libtool and library
 * building sections removed. Both ways work for building elementary
 * applications. It is up to you to decide what is best for you. If you just
 * follow the template above, you can do it both ways and can decide at build
 * time. The more advanced of you may suggest making it a configure option.
 * That is perfectly valid, bu has been left out here for simplicity, as our\
 * aim to have an Elementary (and EFL) tutorial, not an autoconf & automake
 * document.
 * 
 */

static int _elm_signal_exit(void *data, int ev_type, void *ev);
#ifdef HAVE_ELEMENTARY_X
static int _elm_window_property_change(void *data, int ev_type, void *ev);
#endif
static void _elm_rescale(void);

char *_elm_appname = NULL;
Elm_Config *_elm_config = NULL;
const char *_elm_data_dir = NULL;

static Ecore_Event_Handler *_elm_exit_handler = NULL;
static Ecore_Event_Handler *_elm_event_property_change = NULL;
#ifdef HAVE_ELEMENTARY_X
static Ecore_X_Atom _elm_atom_enlightenment_scale = 0;
#endif

static int
_elm_signal_exit(void *data, int ev_type, void *ev)
{
   elm_exit();
   return 1;
}

#ifdef HAVE_ELEMENTARY_X
static int
_elm_window_property_change(void *data, int ev_type, void *ev)
{
   Ecore_X_Event_Window_Property *event = ev;
   
   if (event->win == ecore_x_window_root_first_get())
     {
        if (event->atom == _elm_atom_enlightenment_scale)
          {
             int val = 1000;
             
             if (ecore_x_window_prop_card32_get(event->win,
                                                event->atom,
                                                &val, 1) > 0)
               {
                  double pscale;
                  
                  pscale = _elm_config->scale;
                  if (val > 0) _elm_config->scale = (double)val / 1000.0;
                  if (pscale != _elm_config->scale) _elm_rescale();
               }
          }
     }
   return 1;
}
#endif

static void
_elm_rescale(void)
{
   edje_scale_set(_elm_config->scale);
   _elm_win_rescale();
}

/**
 * @defgroup General General
 */

/**
 * Inititalise Elementary
 * 
 * This call is exported only for use by the ELM_MAIN() macro. There is no 
 * need to use this if you use this macro (which is highly advisable).
 * @ingroup General
 */
EAPI void
elm_init(int argc, char **argv)
{
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
}

/**
 * Shut down Elementary
 * 
 * This should be called at the end of your application just before it ceases
 * to do any more processing. This will clean up any permanent resources your
 * application may have allocated via Elementary that would otherwise persist
 * on an exit without this call.
 * @ingroup General
 */
EAPI void
elm_shutdown(void)
{
   elm_quicklaunch_sub_shutdown();
   elm_quicklaunch_shutdown();
}

static const char *elm_engine, *elm_scale, *elm_theme, *elm_prefix, *elm_data_dir;
static const char *elm_font_hinting, *elm_font_path, *elm_image_cache;
static const char *elm_font_cache, *elm_finger_size, *elm_fps;
static const char *elm_thumbscroll_enabled, *elm_thumbscroll_threshhold;
static const char *elm_thumbscroll_momentum_threshhold, *elm_thumbscroll_friction;

EAPI void
elm_quicklaunch_init(int argc, char **argv)
{
   int i;
   char buf[PATH_MAX];
   
   eet_init();
   ecore_init(); 
   ecore_app_args_set(argc, (const char **)argv);
   ecore_file_init();
   evas_init();
   edje_init();
   ecore_evas_init(); // FIXME: check errors
   
   _elm_exit_handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _elm_signal_exit, NULL);
   
   _elm_appname = strdup(ecore_file_file_get(argv[0]));

   elm_engine = getenv("ELM_ENGINE");
   elm_scale = getenv("ELM_SCALE");
   elm_theme = getenv("ELM_THEME");
   elm_prefix = getenv("ELM_PREFIX");
   elm_data_dir = getenv("ELM_DATA_DIR");
   elm_font_hinting = getenv("ELM_FONT_HINTING");
   elm_font_path = getenv("ELM_FONT_PATH");
   elm_image_cache = getenv("ELM_IMAGE_CACHE");
   elm_font_cache = getenv("ELM_FONT_CACHE");
   elm_finger_size = getenv("ELM_FINGER_SIZE");
   elm_fps = getenv("ELM_FPS");
   elm_thumbscroll_enabled = getenv("ELM_THUMBSCROLL_ENABLE");
   elm_thumbscroll_threshhold = getenv("ELM_THUMBSCROLL_THRESHOLD");
   elm_thumbscroll_momentum_threshhold = getenv("ELM_THUMBSCROLL_MOMENTUM_THRESHOLD");
   elm_thumbscroll_friction = getenv("ELM_THUMBSCROLL_FRICTION");
   
   if (!_elm_data_dir)
     {
        _elm_data_dir = eina_stringshare_add(elm_data_dir);
     }
   if (!_elm_data_dir)
     {  
        if (elm_prefix)
          {
             snprintf(buf, sizeof(buf), "%s/share/elementary", elm_prefix);
             _elm_data_dir = eina_stringshare_add(buf);
          }
     }
#ifdef HAVE_DLADDR
   if (!_elm_data_dir)
     {
        Dl_info elementary_dl;
        // libelementary.so/../../share/elementary/
        if (dladdr(elm_init, &elementary_dl))
          {
             char *dir, *dir2;
             
             dir = ecore_file_dir_get(elementary_dl.dli_fname);
             if (dir)
               {
                  dir2 = ecore_file_dir_get(dir);
                  if (dir2)
                    {
                       snprintf(buf, sizeof(buf), "%s/share/elementary", dir2);
                       if (ecore_file_is_dir(buf))
                         _elm_data_dir = eina_stringshare_add(buf);
                       free(dir2);
                    }
                  free(dir);
               }
          }
     }
#endif
   if (!_elm_data_dir)
     {
        _elm_data_dir = eina_stringshare_add(PACKAGE_DATA_DIR);
     }
   if (!_elm_data_dir)
     {
        _elm_data_dir = eina_stringshare_add("/");
     }
        
   // FIXME: actually load config
   _elm_config = ELM_NEW(Elm_Config);
   _elm_config->engine = ELM_SOFTWARE_X11;
   _elm_config->thumbscroll_enable = 1;
   _elm_config->thumbscroll_threshhold = 24;
   _elm_config->thumbscroll_momentum_threshhold = 100.0;
   _elm_config->thumbscroll_friction = 1.0;
   _elm_config->thumbscroll_bounce_friction = 0.5;
   _elm_config->thumbscroll_bounce_enable = 1;
   _elm_config->scale = 1.0;
   _elm_config->font_hinting = 2;
   _elm_config->font_dirs = NULL;
   _elm_config->image_cache = 4096;
   _elm_config->font_cache = 512;
   _elm_config->finger_size = 40;
   _elm_config->bgpixmap = 0;
   _elm_config->compositing = 1;
   _elm_config->fps = 60.0;
   
   if (elm_engine)
     {
        if ((!strcasecmp(elm_engine, "x11")) ||
            (!strcasecmp(elm_engine, "x")) ||
            (!strcasecmp(elm_engine, "software-x11")) ||
            (!strcasecmp(elm_engine, "software_x11")))
          _elm_config->engine = ELM_SOFTWARE_X11;
        else if ((!strcasecmp(elm_engine, "x11-16")) ||
                 (!strcasecmp(elm_engine, "x16")) ||
                 (!strcasecmp(elm_engine, "software-16-x11")) ||
                 (!strcasecmp(elm_engine, "software_16_x11")))
          _elm_config->engine = ELM_SOFTWARE_16_X11;
        else if ((!strcasecmp(elm_engine, "xrender")) ||
                 (!strcasecmp(elm_engine, "xr")) ||
                 (!strcasecmp(elm_engine, "xrender-x11")) ||
                 (!strcasecmp(elm_engine, "xrender_x11")))
          _elm_config->engine = ELM_XRENDER_X11;
        else if ((!strcasecmp(elm_engine, "fb")) ||
                 (!strcasecmp(elm_engine, "software-fb")) ||
                 (!strcasecmp(elm_engine, "software_fb")))
          _elm_config->engine = ELM_SOFTWARE_FB;
        else if ((!strcasecmp(elm_engine, "opengl")) ||
                 (!strcasecmp(elm_engine, "gl")) ||
                 (!strcasecmp(elm_engine, "opengl-x11")) ||
                 (!strcasecmp(elm_engine, "opengl_x11")))
          _elm_config->engine = ELM_OPENGL_X11;
        else if ((!strcasecmp(elm_engine, "ddraw")) ||
                 (!strcasecmp(elm_engine, "software-ddraw")) ||
                 (!strcasecmp(elm_engine, "software_ddraw")))
          _elm_config->engine = ELM_SOFTWARE_WIN32;
        else if ((!strcasecmp(elm_engine, "wince-gdi")) ||
                 (!strcasecmp(elm_engine, "software-16-wince-gdi")) ||
                 (!strcasecmp(elm_engine, "software_16_wince_gdi")))
          _elm_config->engine = ELM_SOFTWARE_16_WINCE;
     }
   
   if (elm_thumbscroll_enabled)
     _elm_config->thumbscroll_enable = atoi(elm_thumbscroll_enabled);
   if (elm_thumbscroll_threshhold)
     _elm_config->thumbscroll_threshhold = atoi(elm_thumbscroll_threshhold);
   // FIXME: floatformat locale issues here 1.0 vs 1,0 - should just be 1.0
   if (elm_thumbscroll_momentum_threshhold)
     _elm_config->thumbscroll_momentum_threshhold = atof(elm_thumbscroll_momentum_threshhold);
   if (elm_thumbscroll_friction)
     _elm_config->thumbscroll_friction = atof(elm_thumbscroll_friction);
   
   if (elm_theme)
     _elm_theme_parse(elm_theme);
   else
     _elm_theme_parse("default");
   
   _elm_config->font_hinting = 2;
   if (elm_font_hinting)
     {
        if (!strcasecmp(elm_font_hinting, "none"))
          _elm_config->font_hinting = 0;
        else if (!strcasecmp(elm_font_hinting, "auto"))
          _elm_config->font_hinting = 1;
        else if (!strcasecmp(elm_font_hinting, "bytecode"))
          _elm_config->font_hinting = 2;
     }
   
   if (elm_font_path)
     {
        const char *p, *pp, *s;
        char *buf;
        
        buf = alloca(strlen(elm_font_path) + 1);
        p = elm_font_path;
        pp = p;
        for (;;)
          {
             if ((*p == ':') || (*p == 0))
               {
                  int len;
                  
                  len = p - pp;
                  strncpy(buf, pp, len);
                  buf[len] = 0;
                  _elm_config->font_dirs = eina_list_append(_elm_config->font_dirs, eina_stringshare_add(buf));
                  if (*p == 0) break;
                  p++;
                  pp = p;
               }
             else
               {
                  if (*p == 0) break;
                  p++;
               }
          }
     }
   
   if (elm_image_cache)
     _elm_config->image_cache = atoi(elm_image_cache);

   if (elm_font_cache)
     _elm_config->font_cache = atoi(elm_font_cache);

   if (elm_scale)
     _elm_config->scale = atof(elm_scale);
   
   _elm_config->finger_size = 
     (double)_elm_config->finger_size * _elm_config->scale;
   if (elm_finger_size)
     _elm_config->finger_size = atoi(elm_finger_size);
   
   if (elm_fps)
     _elm_config->fps = atof(elm_fps);
   
   if (_elm_config->fps < 1.0)
     _elm_config->fps = 1.0;
   
   ecore_animator_frametime_set(1.0 / _elm_config->fps);
}

EAPI void
elm_quicklaunch_sub_init(int argc, char **argv)
{
   ecore_app_args_set(argc, (const char **)argv);
   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11))
     {
#ifdef HAVE_ELEMENTARY_X
	int val = 1000;
	
	if (!ecore_x_init(NULL))
          {
             EINA_ERROR_PERR("elementary: ERROR. Cannot connect to X11 display. check $DISPLAY variable\n");
             exit(1);
          }
	if (!ecore_x_screen_is_composited(0))
	  _elm_config->compositing = 0;
        _elm_atom_enlightenment_scale = ecore_x_atom_get("ENLIGHTENMENT_SCALE");
        ecore_x_event_mask_set(ecore_x_window_root_first_get(),
                               ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
        _elm_event_property_change = ecore_event_handler_add
          (ECORE_X_EVENT_WINDOW_PROPERTY, _elm_window_property_change, NULL);
        if (!elm_scale)
          {
             if (ecore_x_window_prop_card32_get(ecore_x_window_root_first_get(),
                                                _elm_atom_enlightenment_scale,
                                                &val, 1) > 0)
               {
                  if (val > 0)
                    {
                       _elm_config->scale = (double)val / 1000.0;
                       // FIXME: hack until e export finger size too
                       if (!elm_finger_size)
                         _elm_config->finger_size = 40.0 * _elm_config->scale;
                    }
               }
          }
#endif        
      }

}

EAPI void
elm_quicklaunch_sub_shutdown(void)
{
   _elm_win_shutdown();
   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_WIN32) ||
       (_elm_config->engine == ELM_SOFTWARE_16_WINCE))
     {
#ifdef HAVE_ELEMENTARY_X
        ecore_event_handler_del(_elm_event_property_change);
        _elm_event_property_change = NULL;
        ecore_x_disconnect();
#endif
        evas_cserve_disconnect();
     }
}

EAPI void
elm_quicklaunch_shutdown(void)
{
   const char *fontdir;
   
   eina_stringshare_del(_elm_data_dir);
   _elm_data_dir = NULL;

   EINA_LIST_FREE(_elm_config->font_dirs, fontdir)
     {
        eina_stringshare_del(fontdir);
     }

   ecore_event_handler_del(_elm_exit_handler);
   _elm_exit_handler = NULL;
   
   free(_elm_config);
   free(_elm_appname);
   ecore_evas_shutdown();
   edje_shutdown();
   evas_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();
   eet_shutdown();
}

EAPI void
elm_quicklaunch_seed(void)
{
   Evas_Object *win, *bg, *bt;
   
   win = elm_win_add(NULL, "seed", ELM_WIN_BASIC);
   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);
   bt = elm_button_add(win);
   elm_button_label_set(bt, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~-_=+\\|]}[{;:'\",<.>/?");
   elm_win_resize_object_add(win, bt);
   ecore_main_loop_iterate();
   evas_object_del(win);
   ecore_main_loop_iterate();
   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11))
     {
#ifdef HAVE_ELEMENTARY_X
        ecore_x_sync();
#endif        
      }
   ecore_main_loop_iterate();
}

static void *qr_handle = NULL;
static int (*qr_main) (int argc, char **argv) = NULL;

EAPI Evas_Bool
elm_quicklaunch_prepare(int argc, char **argv)
{
#ifdef HAVE_FORK
   char *exe = elm_quicklaunch_exe_path_get(argv[0]);
   if (!exe)
     {
        printf("ERROR: %s does not exist\n", argv[0]);
        return 0;
     }
   else
     {
        char *exe2, *p;
        char *exename;
        
        exe2 = malloc(strlen(exe) + 1 + 10);
        strcpy(exe2, exe);
        p = strrchr(exe2, '/');
        if (p) p++;
        else p = exe2;
        exename = alloca(strlen(p) + 1);
        strcpy(exename, p);
        *p = 0;
        strcat(p, "../lib/");
        strcat(p, exename);
        strcat(p, ".so");
        if (access(exe2, R_OK | X_OK) == 0)
          {
             free(exe);
             exe = exe2;
          }
        else
          free(exe2);
     }
   qr_handle = dlopen(exe, RTLD_NOW | RTLD_GLOBAL);
   free(exe);
   if (!qr_handle) return 0;
   qr_main = dlsym(qr_handle, "elm_main");
   if (!qr_main)
     {
        dlclose(qr_handle);
        qr_handle = NULL;
        return 0;
     }
   return 1;
#else
   return 0;
#endif
}

#ifdef HAVE_FORK
static void
save_env(void)
{
   int i, size;
   extern char **environ;
   char **oldenv, **p;
   
   oldenv = environ;
   
   for (i = 0, size = 0; environ[i] != NULL; i++)
     size += strlen(environ[i]) + 1;
   
   p = malloc((i + 1) * sizeof(char *));
   if (!p) return;
   
   environ = p;
      
   for (i = 0; oldenv[i] != NULL; i++)
     environ[i] = strdup(oldenv[i]);
   environ[i] = NULL;
}
#endif

EAPI Evas_Bool
elm_quicklaunch_fork(int argc, char **argv, char *cwd, void (postfork_func) (void *data), void *postfork_data)
{
#ifdef HAVE_FORK
   pid_t child;
   int ret;
   int real_argc;
   char **real_argv;
   
   // FIXME:
   // need to accept current environment from elementary_run
   if (!qr_main)
     {
        int i;
        char **args;
        
        child = fork();
        if (child > 0) return 1;
	else if (child < 0)
	  {
	     perror("could not fork");
	     return 0;
	  }
        setsid();
        if (chdir(cwd) != 0)
	  perror("could not chdir");
        args = alloca((argc + 1) * sizeof(char *));
        for (i = 0; i < argc; i++) args[i] = argv[i];
        args[argc] = NULL;
        printf("WARNING: %s not quicklaunch capable\n", argv[0]);
        exit(execvp(argv[0], args));
     }
   child = fork();
   if (child > 0) return 1;
   else if (child < 0)
     {
	perror("could not fork");
	return 0;
     }
   if (postfork_func) postfork_func(postfork_data);

   setsid();
   if (chdir(cwd) != 0)
     perror("could not chdir");
   // FIXME: this is very linux specific. it changes argv[0] of the process
   // so ps etc. report what you'd expect. for other unixes and os's this
   // may just not work
   save_env();
   if (real_argv)
     {
        char *lastarg, *p;

        ecore_app_args_get(&real_argc, &real_argv);
        lastarg = real_argv[real_argc - 1] + strlen(real_argv[real_argc - 1]);
        for (p = real_argv[0]; p < lastarg; p++) *p = 0;
        strcpy(real_argv[0], argv[0]);
     }
   ecore_app_args_set(argc, (const char **)argv);
   ret = qr_main(argc, argv);
   exit(ret);
   return 1;
#else
   return 0;
#endif   
}

EAPI void
elm_quicklaunch_cleanup(void)
{
#ifdef HAVE_FORK
   if (qr_handle)
     {
        dlclose(qr_handle);
        qr_handle = NULL;
        qr_main = NULL;
     }
#endif
}

EAPI int
elm_quicklaunch_fallback(int argc, char **argv)
{
   int ret;
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
   elm_quicklaunch_prepare(argc, argv);
   ret = qr_main(argc, argv);
   exit(ret);
   return ret;
}

EAPI char *
elm_quicklaunch_exe_path_get(const char *exe)
{
   static char *path = NULL;
   static Eina_List *pathlist = NULL;
   const char *pathitr;
   const Eina_List *l;
   char buf[PATH_MAX];
   if (exe[0] == '/') return strdup(exe);
   if ((exe[0] == '.') && (exe[1] == '/')) return strdup(exe);
   if ((exe[0] == '.') && (exe[1] == '.') && (exe[2] == '/')) return strdup(exe);
   if (!path)
     {
        const char *p, *pp, *s;
        char *buf;
        path = getenv("PATH");
        buf = alloca(strlen(path) + 1);
        p = path;
        pp = p;
        for (;;)
          {
             if ((*p == ':') || (*p == 0))
               {
                  int len;
                  
                  len = p - pp;
                  strncpy(buf, pp, len);
                  buf[len] = 0;
                  pathlist = eina_list_append(pathlist, eina_stringshare_add(buf));
                  if (*p == 0) break;
                  p++;
                  pp = p;
               }
             else
               {
                  if (*p == 0) break;
                  p++;
               }
          }
     }
   EINA_LIST_FOREACH(pathlist, l, pathitr)
     {
        snprintf(buf, sizeof(buf), "%s/%s", pathitr, exe);
        if (access(buf, R_OK | X_OK) == 0) return strdup(buf);
     }
   return NULL;
}

/**
 * Run the main loop
 * 
 * This call should be called just after all initialization is complete. This
 * function will not return until elm_exit() is called. It will keep looping
 * running the main event/processing loop for Elementary.
 * @ingroup General
 */
EAPI void
elm_run(void)
{
   ecore_main_loop_begin();
}

/**
 * Exit the main loop
 * 
 * If this call is called, it will flag the main loop to cease processing and
 * return back to its parent function.
 * @ingroup General
 */
EAPI void
elm_exit(void)
{
   ecore_main_loop_quit();
}

/**
 * @defgroup Scaling Selective Widget Scaling
 * 
 * Different widgets can be scaled independently. These functions allow you to
 * manipulate this scaling on a per-widget basis. The object and all its 
 * children get their scaling factors multiplied by the scale factor set. 
 * This is multiplicative, in that if a child also has a scale size set it is
 * in turn multiplied by its parent's scale size. 1.0 means “don't scale”,
 * 2.0 is double size, 0.5 is half etc.
 */

/**
 * Set the scaling factor
 * 
 * @param obj The object
 * @param scale Scale factor (from 0.0 up, with 1.0 == no scaling)
 * @ingroup Scaling
 */
EAPI void
elm_object_scale_set(Evas_Object *obj, double scale)
{
   return elm_widget_scale_set(obj, scale);
}

/**
 * Get the scaling factor
 * 
 * @param obj The object
 * @return The scaling factor set by elm_object_scale_set()
 * @ingroup Scaling
 */
EAPI double
elm_object_scale_get(const Evas_Object *obj)
{
   return elm_widget_scale_get(obj);
}

/**
 * @defgroup Styles Styles
 * 
 * Widgets can have different styles of look. These generic API's set
 * styles of widgets, if they support them (and if the theme(s) do).
 */

/**
 * Set the style
 * 
 * This sets the name of the style
 * @param obj The object
 * @param style The style name to use
 * @ingroup Styles
 */
EAPI void
elm_object_style_set(Evas_Object *obj, const char *style)
{
   elm_widget_style_set(obj, style);
}

/**
 * Get the style
 * 
 * This gets the style being used for that widget. Note that the string
 * pointer is only valid as longas the object is valid and the style doesn't 
 * change.
 * 
 * @param obj The object
 * @return The style name
 * @ingroup Styles
 */
EAPI const char *
elm_object_style_get(const Evas_Object *obj)
{
   return elm_widget_style_get(obj);
}

/**
 * Get the global scaling factor
 * 
 * This gets the globally configured scaling factor that is applied to all
 * objects.
 * 
 * @return The scaling factor
 * @ingroup Scaling
 */
EAPI double
elm_scale_get(void)
{
   return _elm_config->scale;
}

/**
 * Set the global scaling factor
 * 
 * This sets the globally configured scaling factor that is applied to all
 * objects.
 * 
 * @param scale The scaling factor to set
 * @ingroup Scaling
 */
EAPI void
elm_scale_set(double scale)
{
   if (_elm_config->scale == scale) return;
   _elm_config->scale = scale;
   _elm_rescale();
}

/**
 * @defgroup Fingers Fingers
 * 
 * Elementary is designed to be finger-friendly for touchscreens, and so in
 * addition to scaling for display resolution, it can also scale based on
 * finger "resoltion" (or size).
 */

/**
 * Get the configured finger size
 * 
 * This gets the globally configured finger size in pixels
 * 
 * @return The finger size
 * @ingroup Fingers
 */
EAPI Evas_Coord
elm_finger_size_get(void)
{
   return _elm_config->finger_size;
}

/**
 * Set the configured finger size
 * 
 * This sets the globally configured finger size in pixels
 * 
 * @paramsize The finger size
 * @ingroup Fingers
 */
EAPI void
elm_finger_size_set(Evas_Coord size)
{
   elm_finger_size = NULL;
   if (_elm_config->finger_size == size) return;
   _elm_config->finger_size = size;
   _elm_rescale();
}

/**
 * Adjust size of an element for finger usage 
 * 
 * This takes width and height sizes (in pixels) as input and a size multiple
 * (which is how many fingers you want to place within the area), and adjusts
 * the size tobe large enough to accomodate finger. On return the w and h
 * sizes poiner do by these parameters will be modified.
 * 
 * @param times_w How many fingers should fit horizontally
 * @param w Pointer to the width size to adjust
 * @param times_h How many fingers should fit vertically
 * @param h Pointer to the height size to adjust
 * @ingroup Fingers
 */
EAPI void
elm_coords_finger_size_adjust(int times_w, Evas_Coord *w, int times_h, Evas_Coord *h)
{
   if ((w) && (*w < (_elm_config->finger_size * times_w)))
     *w = _elm_config->finger_size * times_w;
   if ((h) && (*h < (_elm_config->finger_size * times_h)))
     *h = _elm_config->finger_size * times_h;
}

/**
 * @defgroup Focus Focus
 * 
 * Object have focus. This is what determines where the keyboard input goes to
 * within the application window.
 */

/**
 * Set the focus to the object
 * 
 * This sets the focus target forkeyboard input to be the object indicated.
 * 
 * @param obj The object
 * @ingroup Focus
 */
EAPI void
elm_object_focus(Evas_Object *obj)
{
   elm_widget_focus_set(obj, 1);
}
