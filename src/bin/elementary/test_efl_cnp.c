#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif

#define EO_BETA_API

#include <Elementary.h>

#define ERR(p) printf("%s %d: " p "\n", __func__, __LINE__)

static void
_selection_get_cb(void *data, Efl_Event const *event)
{
   ERR("in");
   Efl_Cnp_Selection *sd = (Efl_Cnp_Selection *)((Efl_Future_Event_Success *)event->info)->value;
   printf("sel: %s, length: %d\n", sd->data, sd->length);
}

static void
_selection_failure_cb(void *data, Efl_Event const *event)
{
   ERR("in");
}

static void
_selection_progress_cb(void *data, Efl_Event const *event)
{
   ERR("in");
}


//copy from ui.text
EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx2, *bx3, *bt, *en;
   Efl_Canvas_Text_Cursor *main_cur, *cur;

   win = elm_win_util_standard_add("entry", "Entry");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = efl_add(EFL_UI_TEXT_CLASS, win,
         efl_text_multiline_set(efl_added, EINA_TRUE));

   printf("Added Efl.Ui.Text object\n");
   efl_key_data_set(en, "wrap_idx", 0);
   efl_text_set(en, "Hello world! Goodbye world! This is a test text for the"
         " new UI Text widget.\xE2\x80\xA9This is the next paragraph.\nThis"
         " is the next line.\nThis is Yet another line! Line and paragraph"
         " separators are actually different!");
   efl_text_font_set(en, "Sans", 14);
   efl_text_font_weight_set(en, EFL_TEXT_FONT_WEIGHT_BOLD);
   efl_text_font_slant_set(en, EFL_TEXT_FONT_SLANT_ITALIC);
   efl_text_font_width_set(en, EFL_TEXT_FONT_WIDTH_ULTRACONDENSED);
   efl_text_normal_color_set(en, 255, 255, 255, 255);


   //test cnp
   Evas_Object *cnp = efl_add(EFL_CNP_CLASS, en);
   efl_cnp_selection_set(cnp, ELM_CNP_MODE_MARKUP, EFL_CNP_FORMAT_TEXT, "abc", 3);

   Efl_Future *f = efl_cnp_selection_get(cnp, ELM_CNP_MODE_MARKUP, EFL_CNP_FORMAT_TEXT, NULL);
   if (f)
     {
        ERR("Register callbacks");
        efl_future_then(f, _selection_get_cb, _selection_failure_cb, _selection_progress_cb, en);
     }
   else
     {
        ERR("future is NULL");
     }
   //

   efl_ui_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, en);
   evas_object_show(en);
   elm_object_focus_set(en, EINA_TRUE);

   evas_object_resize(win, 480, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
