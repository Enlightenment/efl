#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* NOTE: Respecting header order is important for portability.
 * Always put system first, then EFL, then your public header,
 * and finally your private one. */

#include <Ecore_Getopt.h>
#include <Elementary.h>

#include "gettext.h"

#include <Elm_Code.h>

#include "elm_code_test_private.h"

#define COPYRIGHT "Copyright © 2014 andy <andy@andywilliams.me> and various contributors (see AUTHORS)."

static void
_elm_code_test_win_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_exit();
}

static Evas_Object *
elm_code_test_win_setup(void)
{
   Evas_Object *win;
   Elm_Code *code;
   Elm_Code_Line *line;
   Evas_Object *widget;

   win = elm_win_util_standard_add("main", "Elm_code_test");
   if (!win) return NULL;

   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _elm_code_test_win_del, NULL);

   code = elm_code_create(elm_code_file_new());
   widget = elm_code_widget_add(win, code);
   elm_code_file_line_append(code->file, "Hello World, Elm Code!");
   elm_code_file_line_append(code->file, "");
   elm_code_file_line_append(code->file, "This is a demo of elm_code's capabilities.");

   elm_code_file_line_append(code->file, "*** Currently experimental ***");
   line = elm_code_file_line_get(code->file, 4);
   line->status = ELM_CODE_STATUS_TYPE_ERROR;

   evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget);

   elm_win_resize_object_add(win, widget);

   evas_object_resize(win, 280 * elm_config_scale_get(), 70 * elm_config_scale_get());
   evas_object_show(win);

   return win;
}

static const Ecore_Getopt optdesc = {
  "elm_code_test",
  "%prog [options]",
  PACKAGE_VERSION,
  COPYRIGHT,
  "BSD with advertisement clause",
  "An EFL elm_code_test program",
  0,
  {
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win;
   int args;
   Eina_Bool quit_option = EINA_FALSE;

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };

#if ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   bind_textdomain_codeset(PACKAGE, "UTF-8");
   textdomain(PACKAGE);
#endif

   elm_code_init();

   args = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (args < 0)
     {
	EINA_LOG_CRIT("Could not parse arguments.");
	goto end;
     }
   else if (quit_option)
     {
	goto end;
     }

   elm_app_info_set(elm_main, "elm_code_test", "images/elm_code.png");

   if (!(win = elm_code_test_win_setup()))
     goto end;

   elm_run();

 end:
   elm_code_shutdown();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
