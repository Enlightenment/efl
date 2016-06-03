#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

/* NOTE: Respecting header order is important for portability.
 * Always put system first, then EFL, then your public header,
 * and finally your private one. */

#if ENABLE_NLS
# include <libintl.h>
#endif

#include <Ecore_Getopt.h>
#include <Elementary.h>

#include "elm_code_test_private.h"

#define COPYRIGHT "Copyright © 2014 andy <andy@andywilliams.me> and various contributors (see AUTHORS)."

static void
_elm_code_test_win_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_exit();
}

static void _append_line(Elm_Code_File *file, const char *line)
{
   int length;

   length = strlen(line);
   elm_code_file_line_append(file, line, length, NULL);
}

static Eina_Bool
_elm_code_test_line_clicked_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Elm_Code_Line *line;

   line = (Elm_Code_Line *)event->info;

   printf("CLICKED line %d\n", line->number);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_elm_code_test_line_done_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Elm_Code_Line *line;

   line = (Elm_Code_Line *)event->info;

   if (line->number == 1)
     elm_code_line_token_add(line, 17, 24, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
   else if (line->number == 4)
     line->status = ELM_CODE_STATUS_TYPE_ERROR;

   return EO_CALLBACK_STOP;
}

static Evas_Object *
_elm_code_test_welcome_setup(Evas_Object *parent)
{
   Elm_Code *code;
   Elm_Code_Widget *widget;

   code = elm_code_create();
   widget = eo_add(ELM_CODE_WIDGET_CLASS, parent, elm_obj_code_widget_code_set(eo_self, code));
   elm_obj_code_widget_font_set(widget, NULL, 12);
   eo_event_callback_add(widget, &ELM_CODE_EVENT_LINE_LOAD_DONE, _elm_code_test_line_done_cb, NULL);
   eo_event_callback_add(widget, ELM_OBJ_CODE_WIDGET_EVENT_LINE_CLICKED, _elm_code_test_line_clicked_cb, code);

   _append_line(code->file, "❤ Hello World, Elm Code! ❤");
   _append_line(code->file, "");
   _append_line(code->file, "This is a demo of elm_code's capabilities.");
   _append_line(code->file, "⚑ *** Currently experimental ***");

   evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget);

   elm_code_widget_selection_start(widget, 1, 3);
   elm_code_widget_selection_end(widget, 1, 13);

   return widget;
}

static Evas_Object *
_elm_code_test_editor_setup(Evas_Object *parent, Eina_Bool log)
{
   Elm_Code *code;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;

   code = elm_code_create();
   widget = eo_add(ELM_CODE_WIDGET_CLASS, parent, elm_obj_code_widget_code_set(eo_self, code));
   elm_obj_code_widget_font_set(widget, NULL, 14);
   elm_obj_code_widget_editable_set(widget, EINA_TRUE);
   elm_obj_code_widget_show_whitespace_set(widget, EINA_TRUE);
   elm_obj_code_widget_line_numbers_set(widget, EINA_TRUE);

   if (!log)
     {
        _append_line(code->file, "Edit me :)");
        _append_line(code->file, "");
        _append_line(code->file, "");
        _append_line(code->file, "...Please?");

        line = elm_code_file_line_get(code->file, 1);
        elm_code_line_token_add(line, 5, 6, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
        elm_code_callback_fire(code, &ELM_CODE_EVENT_LINE_LOAD_DONE, line);
     }

   evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget);

   return widget;
}

static Evas_Object *
_elm_code_test_mirror_setup(Elm_Code *code, char *font_name, Evas_Object *parent)
{
   Elm_Code_Widget *widget;

   widget = eo_add(ELM_CODE_WIDGET_CLASS, parent, elm_obj_code_widget_code_set(eo_self, code));
   elm_obj_code_widget_font_set(widget, font_name, 11);
   elm_obj_code_widget_line_numbers_set(widget, EINA_TRUE);

   evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget);

   return widget;
}

static Evas_Object *
_elm_code_test_diff_inline_setup(Evas_Object *parent)
{
   Evas_Object *diff;
   Elm_Code *code;

   code = elm_code_create();
   diff = eo_add(ELM_CODE_WIDGET_CLASS, parent, elm_obj_code_widget_code_set(eo_self, code));

   evas_object_size_hint_weight_set(diff, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(diff, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(diff);

   elm_code_parser_standard_add(code, ELM_CODE_PARSER_STANDARD_DIFF);
   elm_code_file_open(code, PACKAGE_DATA_DIR "testdiff.diff");

   return diff;
}

static Evas_Object *
_elm_code_test_diff_setup(Evas_Object *parent)
{
   Evas_Object *diff;
   Elm_Code *code;

   code = elm_code_create();
   elm_code_file_open(code, PACKAGE_DATA_DIR "testdiff.diff");

   diff = elm_code_diff_widget_add(parent, code);
   return diff;
}

static Eina_Bool
_elm_code_test_log_timer(void *data)
{
   Elm_Code *code = data;
   static int line = 0;
   char buf[250];

   sprintf(buf, "line %d", ++line);
   _append_line(code->file, buf);

   return ECORE_CALLBACK_RENEW;
}

static void
_elm_code_test_log_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   static Ecore_Timer *t = NULL;

   if (t)
     {
        elm_object_text_set(obj, "Start");
        ecore_timer_del(t);
        t = NULL;
        return;
     }

   t = ecore_timer_add(0.05, _elm_code_test_log_timer, data);
   elm_object_text_set(obj, "Stop");
}

static void
_elm_code_test_welcome_editor_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *naviframe, *screen;

   naviframe = (Evas_Object *)data;
   screen = elm_box_add(naviframe);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(screen, _elm_code_test_editor_setup(screen, EINA_FALSE));
   evas_object_show(screen);

   elm_naviframe_item_push(naviframe, "Editor",
                           NULL, NULL, screen, NULL);
}

static void
_elm_code_test_welcome_log_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *naviframe, *screen, *o, *code;

   naviframe = (Evas_Object *)data;
   screen = elm_box_add(naviframe);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   code = _elm_code_test_editor_setup(screen, EINA_TRUE);
   elm_box_pack_end(screen, code);

   o = elm_button_add(screen);
   elm_object_text_set(o, "log");
   evas_object_smart_callback_add(o, "clicked", _elm_code_test_log_clicked, elm_obj_code_widget_code_get(code));
   elm_box_pack_end(screen, o);
   evas_object_show(o);

   evas_object_show(screen);

   elm_naviframe_item_push(naviframe, "Editor",
                           NULL, NULL, screen, NULL);
}

static void
_elm_code_test_welcome_mirror_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Code *code;
   Evas_Object *naviframe, *screen, *widget;

   naviframe = (Evas_Object *)data;
   screen = elm_box_add(naviframe);
   elm_box_homogeneous_set(screen, EINA_TRUE);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   widget = _elm_code_test_editor_setup(screen, EINA_FALSE);
   code = elm_obj_code_widget_code_get(widget);
   elm_box_pack_end(screen, widget);

   elm_box_pack_end(screen, _elm_code_test_mirror_setup(code, "Mono:style=Oblique", screen));
   elm_box_pack_end(screen, _elm_code_test_mirror_setup(code, "Nimbus Mono", screen));

   evas_object_show(screen);
   elm_naviframe_item_push(naviframe, "Mirrored editor",
                           NULL, NULL, screen, NULL);
}

static void
_elm_code_test_welcome_diff_inline_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *naviframe, *screen;

   naviframe = (Evas_Object *)data;
   screen = elm_box_add(naviframe);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(screen, _elm_code_test_diff_inline_setup(screen));
   evas_object_show(screen);

   elm_naviframe_item_push(naviframe, "Diff widget (inline)",
                           NULL, NULL, screen, NULL);
}

static void
_elm_code_test_welcome_diff_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *naviframe, *screen;

   naviframe = (Evas_Object *)data;
   screen = elm_box_add(naviframe);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(screen, _elm_code_test_diff_setup(screen));
   evas_object_show(screen);

   elm_naviframe_item_push(naviframe, "Diff widget (comparison)",
                           NULL, NULL, screen, NULL);
}

static Evas_Object *
elm_code_test_win_setup(void)
{
   Evas_Object *win, *vbox, *text, *button, *naviframe;
   Elm_Object_Item *item;

   win = elm_win_util_standard_add("main", "Elm_Code Demo");
   if (!win) return NULL;

   naviframe = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, naviframe);
   evas_object_show(naviframe);

   vbox = elm_box_add(win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(vbox);

   text = _elm_code_test_welcome_setup(vbox);
   evas_object_size_hint_weight_set(text, EVAS_HINT_EXPAND, 0.5);
   elm_box_pack_end(vbox, text);

   button = elm_button_add(vbox);
   elm_object_text_set(button, "Editor");
   evas_object_size_hint_weight_set(button, 0.5, 0.25);
   evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 1.0);
   evas_object_smart_callback_add(button, "clicked",
                                       _elm_code_test_welcome_editor_cb, naviframe);
   elm_box_pack_end(vbox, button);
   evas_object_show(button);

   button = elm_button_add(vbox);
   elm_object_text_set(button, "Log viewer");
   evas_object_size_hint_weight_set(button, 0.5, 0.0);
   evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 1.0);
   evas_object_smart_callback_add(button, "clicked",
                                  _elm_code_test_welcome_log_cb, naviframe);
   elm_box_pack_end(vbox, button);
   evas_object_show(button);

   button = elm_button_add(vbox);
   elm_object_text_set(button, "Mirrored editor");
   evas_object_size_hint_weight_set(button, 0.5, 0.0);
   evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0.5);
   evas_object_smart_callback_add(button, "clicked",
                                       _elm_code_test_welcome_mirror_cb, naviframe);
   elm_box_pack_end(vbox, button);
   evas_object_show(button);

   button = elm_button_add(vbox);
   elm_object_text_set(button, "Diff (inline)");
   evas_object_size_hint_weight_set(button, 0.5, 0.0);
   evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0.5);
   evas_object_smart_callback_add(button, "clicked",
                                       _elm_code_test_welcome_diff_inline_cb, naviframe);
   elm_box_pack_end(vbox, button);
   evas_object_show(button);

   button = elm_button_add(vbox);
   elm_object_text_set(button, "Diff (comparison)");
   evas_object_size_hint_weight_set(button, 0.5, 0.25);
   evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0.0);
   evas_object_smart_callback_add(button, "clicked",
                                       _elm_code_test_welcome_diff_cb, naviframe);
   elm_box_pack_end(vbox, button);
   evas_object_show(button);

   item = elm_naviframe_item_push(naviframe, "Choose Demo",
                                  NULL, NULL,vbox, NULL);
   elm_naviframe_item_title_enabled_set(item, EINA_FALSE, EINA_FALSE);
   elm_win_resize_object_add(win, naviframe);

   evas_object_smart_callback_add(win, "delete,request", _elm_code_test_win_del, NULL);
   evas_object_resize(win, 400 * elm_config_scale_get(), 320 * elm_config_scale_get());
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
   bindtextdomain(PACKAGE, LOCALE_DIR);
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

   /* tell elm about our app so it can figure out where to get files */
   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
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
