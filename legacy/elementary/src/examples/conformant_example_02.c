/**
 * Simple Elementary's <b>conformant widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` conformant_example_02.c -o conformant_example_02
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

EAPI int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *conform, *btn, *bx, *en;

   win = elm_win_add(NULL, "conformant", ELM_WIN_BASIC);
   elm_win_title_set(win, "Conformant Example");
   elm_win_autodel_set(win, EINA_TRUE);

   elm_win_conformant_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   conform = elm_conformant_add(win);
   elm_win_resize_object_add(win, conform);
   evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(conform);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Test Conformant");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_entry_set(en,
                       "This is a multi-line entry at the bottom<br>"
                       "This can contain more than 1 line of text and be "
                       "scrolled around to allow for entering of lots of "
                       "content. It is also to test to see that autoscroll "
                       "moves to the right part of a larger multi-line "
                       "text entry that is inside of a scroller than can be "
                       "scrolled around, thus changing the expected position "
                       "as well as cursor changes updating auto-scroll when "
                       "it is enabled.");

   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Test Conformant");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   elm_conformant_content_set(conform, bx);
   evas_object_show(bx);

   evas_object_resize(win, 240, 480);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
