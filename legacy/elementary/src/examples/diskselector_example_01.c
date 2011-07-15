/**
 * Simple Elementary's <b>diskselector widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` diskselector_example_01.c -o diskselector_example_01
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
_on_done(void *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void *event_info __UNUSED__)
{
   elm_exit();
}

EAPI int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ds;
   unsigned int i;
   static const char *lbl[] =
   {
      "Sunday",
      "Monday",
      "Tuesday",
      "Wednesday",
      "Thursday",
      "Friday",
      "Saturday"
   };

   win = elm_win_add(NULL, "diskselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "Diskselector Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   /* default */
   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_diskselector_item_append(ds, lbl[i], NULL, NULL, NULL);
   evas_object_show(ds);

   /* set round mode and define side label length */
   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_diskselector_item_append(ds, lbl[i], NULL, NULL, NULL);
   elm_diskselector_round_set(ds, EINA_TRUE);
   elm_diskselector_side_label_length_set(ds, 2);
   evas_object_show(ds);

   /* display more than 3 items */
   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_diskselector_item_append(ds, lbl[i], NULL, NULL, NULL);
   elm_diskselector_display_item_num_set(ds, 5);
   evas_object_show(ds);

   /* set bounce and scroller policy */
   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_diskselector_item_append(ds, lbl[i], NULL, NULL, NULL);
   elm_diskselector_bounce_set(ds, EINA_TRUE, EINA_TRUE);
   elm_diskselector_scroller_policy_set(ds, ELM_SCROLLER_POLICY_AUTO,
                                        ELM_SCROLLER_POLICY_ON);
   evas_object_show(ds);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
