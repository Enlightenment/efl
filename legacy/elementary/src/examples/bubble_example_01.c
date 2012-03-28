//Compile with:
//gcc -o bubble_example_01 bubble_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

void
_bla(void *data, Evas_Object *obj, void *event_info)
{
   static unsigned char corner = 0;
   ++corner;
   if (corner > 3)
     elm_bubble_pos_set(obj, ELM_BUBBLE_POS_TOP_LEFT);
   else
     elm_bubble_pos_set(obj, corner);

}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bubble, *label, *icon;

   win = elm_win_add(NULL, "bubble", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bubble");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   evas_object_resize(win, 300, 200);
   evas_object_show(win);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   label = elm_label_add(win);
   elm_object_text_set(label, "This is the CONTENT of our bubble");
   evas_object_show(label);

   icon = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(icon, 0, 0, 255, 255);
   evas_object_show(icon);

   bubble = elm_bubble_add(win);
   elm_object_part_content_set(bubble, "icon", icon);
   elm_object_part_text_set(bubble, "info", "INFO");
   elm_object_text_set(bubble, "LABEL");
   elm_object_content_set(bubble, label);
   evas_object_resize(bubble, 300, 100);
   evas_object_show(bubble);

   evas_object_smart_callback_add(bubble, "clicked", _bla, NULL);

   label = elm_label_add(win);
   elm_object_text_set(label, "Bubble with no icon, info or label");
   evas_object_show(label);

   bubble = elm_bubble_add(win);
   elm_object_content_set(bubble, label);
   evas_object_resize(bubble, 200, 50);
   evas_object_move(bubble, 0, 110);
   evas_object_show(bubble);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
