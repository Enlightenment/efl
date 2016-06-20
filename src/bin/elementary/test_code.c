#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static Evas_Object *_test_code_win_create(const char *id, const char *name)
{
   Evas_Object *win;

   win = elm_win_add(NULL, id, ELM_WIN_BASIC);
   elm_win_title_set(win, name);
   elm_win_autodel_set(win, EINA_TRUE);

   evas_object_resize(win, 360 * elm_config_scale_get(), 220 * elm_config_scale_get());
   return win;
}

static void _append_line(Elm_Code_File *file, const char *line)
{
   int length;

   length = strlen(line);
   elm_code_file_line_append(file, line, length, NULL);
}

static void
_elm_code_test_line_clicked_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Elm_Code_Line *line;

   line = (Elm_Code_Line *)event->info;

   printf("CLICKED line %d\n", line->number);
}

static void
_elm_code_test_line_done_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Elm_Code_Line *line;

   line = (Elm_Code_Line *)event->info;

   if (line->number == 1)
     elm_code_line_token_add(line, 17, 24, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
   else if (line->number == 4)
     line->status = ELM_CODE_STATUS_TYPE_ERROR;

   eo_event_callback_stop(event->object);
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

void
test_code_editor(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *screen;

   win = _test_code_win_create("code-editor", "Code Editor");
   screen = elm_box_add(win);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(screen, _elm_code_test_editor_setup(screen, EINA_FALSE));
   elm_win_resize_object_add(win, screen);
   evas_object_show(screen);

   evas_object_show(win);
}

void
test_code_log(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *screen, *o, *code;

   win = _test_code_win_create("code-log", "Code Log");
   screen = elm_box_add(win);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   code = _elm_code_test_editor_setup(screen, EINA_TRUE);
   elm_box_pack_end(screen, code);

   o = elm_button_add(screen);
   elm_object_text_set(o, "log");
   evas_object_smart_callback_add(o, "clicked", _elm_code_test_log_clicked, elm_obj_code_widget_code_get(code));
   elm_box_pack_end(screen, o);
   evas_object_show(o);

   elm_win_resize_object_add(win, screen);
   evas_object_show(screen);

   evas_object_show(win);
}

void
test_code_mirror(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Code *code;
   Evas_Object *win, *screen, *widget;

   win = _test_code_win_create("code-mirror", "Code Mirror");
   screen = elm_box_add(win);
   elm_box_homogeneous_set(screen, EINA_TRUE);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   widget = _elm_code_test_editor_setup(screen, EINA_FALSE);
   code = elm_obj_code_widget_code_get(widget);
   elm_box_pack_end(screen, widget);

   elm_box_pack_end(screen, _elm_code_test_mirror_setup(code, "Mono:style=Oblique", screen));
   elm_box_pack_end(screen, _elm_code_test_mirror_setup(code, "Nimbus Mono", screen));

   elm_win_resize_object_add(win, screen);
   evas_object_show(screen);

   evas_object_show(win);
}

void
test_code_diff_inline(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *screen;

   win = _test_code_win_create("code-diff-inline", "Code Diff Inline");
   screen = elm_box_add(win);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(screen, _elm_code_test_diff_inline_setup(screen));
   elm_win_resize_object_add(win, screen);
   evas_object_show(screen);

   evas_object_show(win);
}

void
test_code_diff(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *screen;

   win = _test_code_win_create("code-diff", "Code Diff");
   screen = elm_box_add(win);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(screen, _elm_code_test_diff_setup(screen));
   elm_win_resize_object_add(win, screen);
   evas_object_show(screen);

   evas_object_show(win);
}

void
test_code_welcome(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *screen;

   win = _test_code_win_create("code-welcome", "Code Welcome");
   screen = elm_box_add(win);
   evas_object_size_hint_weight_set(screen, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(screen, _elm_code_test_welcome_setup(screen));
   elm_win_resize_object_add(win, screen);
   evas_object_show(screen);

   evas_object_show(win);
}

