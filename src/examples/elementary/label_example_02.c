//Compile with:
//gcc -g label_example_02.c -o label_example_02 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void
_slide_end_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *label2 = data;

   printf("\"slide,end\" signal is emitted!\n");

   elm_label_slide_mode_set(obj, ELM_LABEL_SLIDE_MODE_NONE);
   elm_label_slide_mode_set(label2, ELM_LABEL_SLIDE_MODE_ALWAYS);
   elm_label_slide_go(obj);
   elm_label_slide_go(label2);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char** argv EINA_UNUSED)
{
   Evas_Object *win, *box, *label1, *label2, *label3;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   // Win
   win = elm_win_util_standard_add("label", "Label Slide");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 200, 100);
   evas_object_show(win);

   // Box
   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(box);
   elm_win_resize_object_add(win, box);

   // Label1
   label1 = elm_label_add(box);
   elm_object_style_set(label1, "slide_long");
   elm_object_text_set(label1, "This first label starts sliding automatically, and stops after one sliding.");
   elm_label_wrap_width_set(label1, 200);
   elm_label_slide_duration_set(label1, 4.0);
   evas_object_show(label1);
   elm_box_pack_end(box, label1);

   // Label2
   label2 = elm_label_add(box);
   elm_object_style_set(label2, "slide_short");
   elm_object_text_set(label2, "The second label starts sliding after the first label stops its sliding.");
   elm_label_wrap_width_set(label2, 200);
   elm_label_slide_duration_set(label2, 3.0);
   evas_object_show(label2);
   elm_box_pack_end(box, label2);

    // Label3
   label3 = elm_label_add(box);
   elm_object_style_set(label3, "slide_bounce");
   elm_object_text_set(label3, "The third label starts sliding after the second label stops its sliding.");
   elm_label_wrap_width_set(label3, 200);
   elm_label_slide_duration_set(label3, 2.0);
   evas_object_show(label3);
   elm_box_pack_end(box, label3);

   evas_object_smart_callback_add(label1, "slide,end", _slide_end_cb, label2);
   evas_object_smart_callback_add(label2, "slide,end", _slide_end_cb, label3);
   evas_object_smart_callback_add(label3, "slide,end", _slide_end_cb, label1);

   elm_label_slide_mode_set(label1, ELM_LABEL_SLIDE_MODE_ALWAYS);
   elm_label_slide_go(label1);

   elm_run();

   return 0;
}
ELM_MAIN()
