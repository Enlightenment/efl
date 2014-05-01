#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
highlight_enabled_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_config_focus_highlight_enabled_set(elm_check_state_get(obj));
}

static void
highlight_animate_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_config_focus_highlight_animate_set(elm_check_state_get(obj));
}

static void
win_highlight_enabled_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_win_focus_highlight_enabled_set((Evas_Object *)data,
                                       elm_check_state_get(obj));
}

static void
win_highlight_animate_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_win_focus_highlight_animate_set((Evas_Object *)data,
                                       elm_check_state_get(obj));
}

static void
custom_chain_unset_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *c, *child = NULL;
   Evas_Object *bx = (Evas_Object *)data;
   Eina_List *l, *children = NULL, *custom_chain = NULL;
   int i = 0;

   if (elm_check_state_get(obj))
     {
        children = elm_box_children_get(bx);
        EINA_LIST_FOREACH(children, l, c)
          {
             if (i == 0)
               {
                  child = c;
                  custom_chain = eina_list_append(custom_chain, c);
                  elm_object_focus_custom_chain_set(bx, custom_chain);
               }
             else if (i == 1) elm_object_focus_custom_chain_prepend(bx, c, child);
             else if (i == 2)
               {
                  elm_object_focus_custom_chain_append(bx, c, child);
                  child = c;
               }
              else if (i == 3) elm_object_focus_custom_chain_prepend(bx, c, child);
              i++;
          }
     }
   else
     {
        if (!elm_object_focus_custom_chain_get(bx)) return;
        elm_object_focus_custom_chain_unset(bx);
     }
}

static void
create_win_bt_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *gd, *bt;

   win = elm_win_util_standard_add("test focus", "Test Focus");
   elm_win_autodel_set(win, EINA_TRUE);

   gd = elm_grid_add(win);
   elm_grid_size_set(gd, 100, 100);
   evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gd);
   evas_object_show(gd);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   elm_grid_pack(gd, bt, 10, 10, 80, 20);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   elm_grid_pack(gd, bt, 10, 30, 80, 20);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   elm_grid_pack(gd, bt, 10, 50, 80, 20);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 4");
   elm_grid_pack(gd, bt, 10, 70, 80, 20);
   evas_object_show(bt);

   evas_object_resize(win, 160, 160);
   evas_object_show(win);
}

void
test_focus_custom_chain(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fr, *bx, *bx2, *tg, *bt, *sp;
   Evas_Object *lb, *bt1, *bt2, *bt3, *bt4;
   Eina_List *custom_chain = NULL;

   elm_config_focus_highlight_enabled_set(EINA_TRUE);
   elm_config_focus_highlight_animate_set(EINA_TRUE);

   win = elm_win_util_standard_add("focus-custom-chain", "Focus Custom Chain");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, fr);
   elm_object_style_set(fr, "pad_large");
   evas_object_show(fr);

   /* First Example - Using Focus Highlight */
   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   tg = elm_check_add(bx);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(tg, "Focus Highlight Enabled (Config)");
   elm_check_state_set(tg, elm_config_focus_highlight_enabled_get());
   evas_object_smart_callback_add(tg, "changed", highlight_enabled_cb, NULL);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   tg = elm_check_add(bx);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(tg, "Focus Highlight Animate (Config)");
   elm_check_state_set(tg, elm_config_focus_highlight_animate_get());
   evas_object_smart_callback_add(tg, "changed", highlight_animate_cb, NULL);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   tg = elm_check_add(bx);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(tg, "Focus Highlight Enabled (Win)");
   elm_check_state_set(tg, elm_win_focus_highlight_enabled_get(win));
   evas_object_smart_callback_add(tg, "changed", win_highlight_enabled_cb, win);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   tg = elm_check_add(bx);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(tg, "Focus Highlight Animate (Win)");
   elm_check_state_set(tg, elm_win_focus_highlight_enabled_get(win));
   evas_object_smart_callback_add(tg, "changed", win_highlight_animate_cb, win);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Create Window");
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "clicked", create_win_bt_clicked, NULL);
   evas_object_show(bt);

   sp = elm_separator_add(bx);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   /* Second Example - Using Custom Chain */
   lb = elm_label_add(bx);
   elm_object_text_set(lb, "Custom Chain: Please use tab key to check");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bx2 = elm_box_add(bx);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bt1 = elm_button_add(bx2);
   elm_object_text_set(bt1, "Button 1");
   evas_object_size_hint_weight_set(bt1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt1, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt1);
   evas_object_show(bt1);

   bt2 = elm_button_add(bx2);
   elm_object_text_set(bt2, "Button 2");
   evas_object_size_hint_weight_set(bt2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt2);
   evas_object_show(bt2);

   bt3 = elm_button_add(bx2);
   elm_object_text_set(bt3, "Button 3");
   evas_object_size_hint_weight_set(bt3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt3, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt3);
   evas_object_show(bt3);

   bt4 = elm_button_add(bx2);
   elm_object_text_set(bt4, "Button 4");
   evas_object_size_hint_weight_set(bt4, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt4, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt4);
   evas_object_show(bt4);

   custom_chain = eina_list_append(custom_chain, bt2);
   custom_chain = eina_list_append(custom_chain, bt1);
   custom_chain = eina_list_append(custom_chain, bt4);
   custom_chain = eina_list_append(custom_chain, bt3);
   elm_object_focus_custom_chain_set(bx2, custom_chain);

   tg = elm_check_add(bx);
   elm_object_text_set(tg, "Custom Chain");
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", custom_chain_unset_cb, bx2);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}
