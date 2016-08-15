#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_state_changed_cb(void *d EINA_UNUSED, Evas_Object *o, void *ei EINA_UNUSED)
{
   char buf[100];
   int val;

   val = efl_ui_nstate_value_get(o);

   printf("nstate widget state: %d\n", val);
   sprintf(buf, "nstate = %d", val);
   elm_object_text_set(o, buf);
}

void
test_nstate(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *nstate;

   win = elm_win_util_standard_add("nstate", "nstate");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   nstate = efl_add(EFL_UI_NSTATE_CLASS, win);
   efl_ui_nstate_count_set(nstate, 5);
   elm_object_text_set(nstate, "nstate = 0");
   elm_box_pack_end(bx, nstate);
   evas_object_show(nstate);
   evas_object_smart_callback_add(nstate, "state,changed",
                                  _state_changed_cb, NULL);

   evas_object_resize(win, 100, 100);
   evas_object_show(win);
}
