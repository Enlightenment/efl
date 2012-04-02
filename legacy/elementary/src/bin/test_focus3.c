/* Test for Focus Chain Linear*/
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static Evas_Object *foc = NULL;

static void
_foc(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   foc = obj;
   printf("foc -> %p\n", foc);
}

static void
_unfoc(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   foc = NULL;
   printf("foc -> %p\n", foc);
}

static void
_add(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bx = data, *en;

   en = elm_entry_add(elm_object_top_widget_get(bx));
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_text_set(en, "An entry");
   evas_object_smart_callback_add(en, "focused", _foc, NULL);
   evas_object_smart_callback_add(en, "unfocused", _unfoc, NULL);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_box_pack_start(bx, en);
   evas_object_show(en);
}

static void
_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (foc) evas_object_del(foc);
}

static void
_hide(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (foc) evas_object_hide(foc);
}

void
test_focus3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *bt, *en;

   win = elm_win_util_standard_add("focus3", "Focus 3");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_resize(win, 320, 480);
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_text_set(en, "An entry");
   evas_object_smart_callback_add(en, "focused", _foc, NULL);
   evas_object_smart_callback_add(en, "unfocused", _unfoc, NULL);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_focus_allow_set(bt, 0);
   elm_object_text_set(bt, "Add");
   evas_object_smart_callback_add(bt, "clicked", _add, bx);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_focus_allow_set(bt, 0);
   elm_object_text_set(bt, "Del");
   evas_object_smart_callback_add(bt, "clicked", _del, NULL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_focus_allow_set(bt, 0);
   elm_object_text_set(bt, "Hide");
   evas_object_smart_callback_add(bt, "clicked", _hide, NULL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}

static void
highlight_enabled_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_config_focus_highlight_enabled_set(elm_check_state_get(obj));
}

static void
highlight_animate_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_config_focus_highlight_animate_set(elm_check_state_get(obj));
}

static void
custom_chain_unset_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
create_win_bt_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
test_focus4(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *gd, *tg, *bt;
   Evas_Object *lb, *bt1, *bt2, *bt3, *bt4, *bx;
   Eina_List *custom_chain = NULL;

   win = elm_win_util_standard_add("focus4", "Focus 4");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_config_focus_highlight_enabled_set(EINA_TRUE);
   elm_config_focus_highlight_animate_set(EINA_TRUE);

   /* First Example - Using Focus Highlight */
   gd = elm_grid_add(win);
   elm_grid_size_set(gd, 100, 100);
   evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gd);
   evas_object_show(gd);

   tg = elm_check_add(win);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(tg, "Focus Highlight Enabled");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", highlight_enabled_cb, NULL);
   elm_grid_pack(gd, tg, 10, 10, 50, 10);
   evas_object_show(tg);

   tg = elm_check_add(win);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(tg, "Focus Highlight Animate");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", highlight_animate_cb, NULL);
   elm_grid_pack(gd, tg, 10, 20, 50, 10);
   evas_object_show(tg);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Create Window");
   elm_grid_pack(gd, bt, 10, 30, 80, 10);
   evas_object_smart_callback_add(bt, "clicked", create_win_bt_clicked, NULL);
   evas_object_show(bt);

   /* Second Example - Using Custom Chain */
   lb = elm_label_add(win);
   elm_object_text_set(lb, "Custom Chain: Please use tab key to check");
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_grid_pack(gd, lb, 10, 45, 80, 5);
   evas_object_show(lb);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt1 = elm_button_add(win);
   elm_object_text_set(bt1, "Button 1");
   evas_object_size_hint_weight_set(bt1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt1, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt1);
   evas_object_show(bt1);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Button 2");
   evas_object_size_hint_weight_set(bt2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt2);
   evas_object_show(bt2);

   bt3 = elm_button_add(win);
   elm_object_text_set(bt3, "Button 3");
   evas_object_size_hint_weight_set(bt3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt3, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt3);
   evas_object_show(bt3);

   bt4 = elm_button_add(win);
   elm_object_text_set(bt4, "Button 4");
   evas_object_size_hint_weight_set(bt4, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt4, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt4);
   evas_object_show(bt4);

   custom_chain = eina_list_append(custom_chain, bt2);
   custom_chain = eina_list_append(custom_chain, bt1);
   custom_chain = eina_list_append(custom_chain, bt4);
   custom_chain = eina_list_append(custom_chain, bt3);
   elm_object_focus_custom_chain_set(bx, custom_chain);

   elm_grid_pack(gd, bx, 10, 50, 80, 30);
   evas_object_show(bx);

   tg = elm_check_add(win);
   elm_object_text_set(tg, "Custom Chain");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", custom_chain_unset_cb, bx);
   elm_grid_pack(gd, tg, 10, 80, 40, 10);
   evas_object_show(tg);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}
#endif
