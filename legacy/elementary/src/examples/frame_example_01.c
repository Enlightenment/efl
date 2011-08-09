//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` frame_example_01.c -o frame_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg;
   Evas_Object *f1, *r1, *f2, *r2, *f3, *r3, *f4, *r4;
   char buf[256];

   win = elm_win_add(NULL, "frame", ELM_WIN_BASIC);
   elm_win_title_set(win, "Frame");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   r1 = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(r1, 255, 0, 0, 255);
   evas_object_show(r1);

   f1= elm_frame_add(win);
   elm_frame_content_set(f1, r1);
   elm_object_text_set(f1, "Default frame");
   evas_object_resize(f1, 100, 100);
   evas_object_move(f1, 25, 25);
   evas_object_show(f1);

   r2 = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(r2, 0, 255, 0, 255);
   evas_object_show(r2);

   f2 = elm_frame_add(win);
   elm_frame_content_set(f2, r2);
   elm_object_text_set(f2, "Padding frame");
   evas_object_resize(f2, 100, 100);
   evas_object_move(f2, 150, 25);
   elm_object_style_set(f2, "pad_small");
   evas_object_show(f2);

   r3 = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(r3, 0, 0, 255, 255);
   evas_object_show(r3);

   f3 = elm_frame_add(win);
   elm_frame_content_set(f3, r3);
   elm_object_text_set(f3, "Top outdent frame");
   evas_object_resize(f3, 100, 100);
   evas_object_move(f3, 25, 150);
   elm_object_style_set(f3, "outdent_top");
   evas_object_show(f3);

   r4 = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(r4, 0, 0, 0, 255);
   evas_object_show(r4);

   f4 = elm_frame_add(win);
   elm_frame_content_set(f4, r4);
   elm_object_text_set(f4, "Bottom outdent frame");
   evas_object_resize(f4, 100, 100);
   evas_object_move(f4, 150, 150);
   elm_object_style_set(f4, "outdent_bottom");
   evas_object_show(f4);

   evas_object_resize(win, 275, 275);
   evas_object_show(win);

   elm_run();

   evas_object_del(r1);
   evas_object_del(r2);
   evas_object_del(r3);
   evas_object_del(r4);

   return 0;
}
ELM_MAIN()
