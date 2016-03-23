//Compile with:
//gcc -g naviframe_example.c -o naviframe_example `pkg-config --cflags --libs elementary`

#include <Elementary.h>

void
_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *nf = data, *btn;
   if (!nf) return;

   btn = elm_button_add(nf);
   elm_object_text_set(btn, "Go to Second Page.");
   evas_object_smart_callback_add(btn, "clicked", _btn_cb, nf);

   elm_naviframe_item_push(nf, "Next Page", NULL, NULL, btn, NULL);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *nf, *btn;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("naviframe", "Naviframe");
   elm_win_autodel_set(win, EINA_TRUE);

   nf = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, nf);
   evas_object_show(nf);

   btn = elm_button_add(nf);
   elm_object_text_set(btn, "Go to Second Page.");
   evas_object_smart_callback_add(btn, "clicked", _btn_cb, nf);

   elm_naviframe_item_push(nf, "First Page", NULL, NULL, btn, NULL);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
