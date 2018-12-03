#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

Evas_Object * _focus_autoscroll_mode_frame_create(Evas_Object *parent);

/**** focus 1 ****/

static Eina_Bool
_event(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, Evas_Object *src EINA_UNUSED, Evas_Callback_Type type, void *event_info)
{
   if (type == EVAS_CALLBACK_KEY_DOWN)
     printf ("Key Down:");
   else if (type == EVAS_CALLBACK_KEY_UP)
     printf ("Key Up:");
   else
     return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   printf("%s\n", ev->key);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_on_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *einfo EINA_UNUSED)
{
   //Evas_Event_Key_Down *event = einfo;
   //printf("%s %p Key %s Parent %p\n", evas_object_type_get(obj),
   //       obj, event->key, evas_object_smart_parent_get(obj));
}

static void
my_disable(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt = data;
   elm_object_disabled_set(bt, EINA_TRUE);
}

static void
my_enable(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt = data;
   elm_object_disabled_set(bt, EINA_FALSE);
}

static inline void
my_show(Evas_Object *obj)
{
   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN,
                                  _on_key_down, NULL);
   evas_object_show(obj);
}

static void
_tb_sel(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   printf("tb sel %p\n", obj);
}

static void
_focus_highlight_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     elm_win_focus_highlight_enabled_set(data, EINA_TRUE);
   else
     elm_win_focus_highlight_enabled_set(data, EINA_FALSE);
}

static void
_focus_anim_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     elm_win_focus_highlight_animate_set(data, EINA_TRUE);
   else
     elm_win_focus_highlight_animate_set(data, EINA_FALSE);
}

static void
_rd_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int value = elm_radio_state_value_get(obj);

   if (value == 0)
     elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_CLICK);
   else if (value == 1)
     elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_IN);
   else
     elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_KEY_ONLY);
}

void
test_focus(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tbx, *tbar, *mainbx, *menu, *ttb;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   unsigned int i, j;

   win = elm_win_util_standard_add("focus", "Focus");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_resize(win, 800, 600);
   elm_object_event_callback_add(win, _event, NULL);
   elm_win_autodel_set(win, EINA_TRUE);
   my_show(win);

   tbx = elm_box_add(win);
   evas_object_size_hint_weight_set(tbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tbx);
   evas_object_show(tbx);

   tbar = elm_toolbar_add(win);
   elm_toolbar_shrink_mode_set(tbar, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tbar, 0.0, 0.0);
   evas_object_size_hint_align_set(tbar, EVAS_HINT_FILL, 0.0);
   tb_it = elm_toolbar_item_append(tbar, "document-print", "Hello", _tb_sel, NULL);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, 100);

   tb_it = elm_toolbar_item_append(tbar, "folder-new", "World", _tb_sel, NULL);
   elm_toolbar_item_priority_set(tb_it, -100);

   tb_it = elm_toolbar_item_append(tbar, "object-rotate-right", "H", _tb_sel, NULL);
   elm_toolbar_item_priority_set(tb_it, 150);

   tb_it = elm_toolbar_item_append(tbar, "mail-send", "Comes", _tb_sel, NULL);
   elm_toolbar_item_priority_set(tb_it, 0);

   tb_it = elm_toolbar_item_append(tbar, "clock", "Elementary", _tb_sel, NULL);
   elm_toolbar_item_priority_set(tb_it, -200);

   tb_it = elm_toolbar_item_append(tbar, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -9999);
   elm_toolbar_menu_parent_set(tbar, win);

   menu = elm_toolbar_item_menu_get(tb_it);
   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", _tb_sel, NULL);
   menu_it = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", _tb_sel, NULL);
   elm_menu_item_add(menu, menu_it, "edit-paste", "is set to", _tb_sel, NULL);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", _tb_sel, NULL);

   elm_box_pack_end(tbx, tbar);
   evas_object_show(tbar);

   mainbx = elm_box_add(win);
   elm_box_horizontal_set(mainbx, EINA_TRUE);
   evas_object_size_hint_weight_set(mainbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(tbx, mainbx);
   my_show(mainbx);

     { //First Col
        Evas_Object *bx = elm_box_add(win);
        evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_box_pack_end(mainbx, bx);
        my_show(bx);

          {
             Evas_Object *lb = elm_label_add(win);
             elm_object_text_set(lb, "<b>Use Tab or Shift+Tab<br/>or Arrow keys</b>");
             evas_object_size_hint_weight_set(lb, 0.0, 0.0);
             evas_object_size_hint_align_set(lb, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             elm_box_pack_end(bx, lb);
             my_show(lb);
          }

          {
             Evas_Object *tg = elm_check_add(win);
             elm_object_style_set(tg, "toggle");
             elm_object_part_text_set(tg, "on", "Yes");
             elm_object_part_text_set(tg, "off", "No");
             elm_box_pack_end(bx, tg);
             my_show(tg);
          }

          {
             Evas_Object *en = elm_entry_add(win);
             elm_entry_scrollable_set(en, EINA_TRUE);
             evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
             evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
             elm_object_text_set(en, "This is a single line");
             elm_entry_single_line_set(en, EINA_TRUE);
             elm_box_pack_end(bx, en);
             my_show(en);
          }

          {
             Evas_Object *bx2 = elm_box_add(win);
             elm_box_horizontal_set(bx2, EINA_TRUE);
             evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bx2, 0.0, 0.0);
             elm_box_pack_end(bx, bx2);

             for (i = 2; i; i--)
               {
                  Evas_Object *bt;
                  bt = elm_button_add(win);
                  elm_object_text_set(bt, "Box");
                  evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                  if (i%2)
                    elm_object_disabled_set(bt, EINA_TRUE);
                  elm_box_pack_end(bx2, bt);
                  my_show(bt);
               }

               {
                  Evas_Object *sc = elm_scroller_add(win);
                  evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND,
                                                   EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(sc, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  elm_scroller_bounce_set(sc, EINA_TRUE, EINA_TRUE);
                  elm_scroller_content_min_limit(sc, 1, 1);
                  elm_box_pack_end(bx2, sc);
                  my_show(sc);

                    {
                       Evas_Object *bt;
                       bt = elm_button_add(win);
                       elm_object_text_set(bt, "Scroller");
                       evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                       EVAS_HINT_FILL);
                       evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                       elm_object_event_callback_add(bt, _event, NULL);
                       elm_object_content_set(sc, bt);
                       my_show(bt);
                       elm_object_event_callback_del(bt, _event, NULL);
                    }
               }

             my_show(bx2);
          }

          {
             Evas_Object *bt;
             bt = elm_button_add(win);
             elm_object_text_set(bt, "Box");
             evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bt, 0.0, 0.0);
             elm_box_pack_end(bx, bt);
             my_show(bt);
          }

          {
             Evas_Object *bx2 = elm_box_add(win);
             elm_box_horizontal_set(bx2, EINA_TRUE);
             evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bx2, 0.0, 0.0);
             elm_box_pack_end(bx, bx2);
             my_show(bx2);

             for (i = 2; i; i--)
               {
                  Evas_Object *bx3 = elm_box_add(win);
                  evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bx3, 0.0, 0.0);
                  elm_box_pack_end(bx2, bx3);
                  my_show(bx3);

                  for (j = 3; j; j--)
                    {
                       Evas_Object *bt;
                       bt = elm_button_add(win);
                       elm_object_text_set(bt, "Box");
                       evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                       EVAS_HINT_FILL);
                       evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                       elm_box_pack_end(bx3, bt);
                       my_show(bt);
                    }
               }

               {
                  Evas_Object *sc = elm_scroller_add(win);
                  evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND,
                                                   EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(sc, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
                  elm_scroller_content_min_limit(sc, 1, 0);
                  elm_box_pack_end(bx2, sc);
                  my_show(sc);

                  Evas_Object *bx3 = elm_box_add(win);
                  evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bx3, 0.0, 0.0);
                  elm_object_content_set(sc, bx3);
                  my_show(bx3);

                  for (i = 5; i; i--)
                    {
                       Evas_Object *bt;
                       bt = elm_button_add(win);
                       elm_object_text_set(bt, "BX Scroller");
                       evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                       EVAS_HINT_FILL);
                       evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                       elm_box_pack_end(bx3, bt);
                       my_show(bt);
                    }
               }
          }
     }

     {//Second Col
        char buf[PATH_MAX];
        Evas_Object *ly = elm_layout_add(win);
        snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
        elm_layout_file_set(ly, buf, "twolines");
        evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, 0.0);
        elm_box_pack_end(mainbx, ly);
        my_show(ly);

          {
             Evas_Object *bx2 = elm_box_add(win);
             elm_box_horizontal_set(bx2, EINA_TRUE);
             evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bx2, 0.0, 0.0);
             elm_object_part_content_set(ly, "element1", bx2);
             my_show(bx2);

             for (i = 3; i; i--)
               {
                  Evas_Object *bt;
                  bt = elm_button_add(win);
                  elm_object_text_set(bt, "Layout");
                  evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                  elm_box_pack_end(bx2, bt);
                  my_show(bt);
                  elm_object_focus_custom_chain_prepend(bx2, bt, NULL);
               }
          }

          {
             Evas_Object *bx2 = elm_box_add(win);
             evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(bx2, 0.0, 0.0);
             elm_object_part_content_set(ly, "element2", bx2);
             my_show(bx2);

               {
                  Evas_Object *bt;
                  bt = elm_button_add(win);
                  elm_object_text_set(bt, "Disable");
                  evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt, 0.0, 0.0);
                  elm_box_pack_end(bx2, bt);
                  evas_object_smart_callback_add(bt, "clicked", my_disable, bt);
                  my_show(bt);
                  elm_object_focus_custom_chain_prepend(bx2, bt, NULL);

                  Evas_Object *bt2;
                  bt2 = elm_button_add(win);
                  elm_object_text_set(bt2, "Enable");
                  evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt2, 0.0, 0.0);
                  elm_box_pack_end(bx2, bt2);
                  evas_object_smart_callback_add(bt2, "clicked", my_enable, bt);
                  my_show(bt2);
                  elm_object_focus_custom_chain_append(bx2, bt2, NULL);

                  Evas_Object *bt3;
                  bt3 = elm_button_add(win);
                  elm_object_text_set(bt3, "KeyOnly with Auto");
                  elm_object_focus_move_policy_set(bt3, ELM_FOCUS_MOVE_POLICY_KEY_ONLY);
                  elm_object_focus_move_policy_automatic_set(bt, EINA_TRUE); // EINA_TURE is default
                  evas_object_size_hint_align_set(bt3, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt3, 0.0, 0.0);
                  elm_box_pack_end(bx2, bt3);
                  my_show(bt3);
                  elm_object_focus_custom_chain_append(bx2, bt3, NULL);

                  Evas_Object *bt4;
                  bt4 = elm_button_add(win);
                  elm_object_text_set(bt4, "KeyOnly without Auto");
                  elm_object_focus_move_policy_set(bt4, ELM_FOCUS_MOVE_POLICY_KEY_ONLY);
                  elm_object_focus_move_policy_automatic_set(bt4, EINA_FALSE);
                  evas_object_size_hint_align_set(bt4, EVAS_HINT_FILL,
                                                  EVAS_HINT_FILL);
                  evas_object_size_hint_weight_set(bt4, 0.0, 0.0);
                  elm_box_pack_end(bx2, bt4);
                  my_show(bt4);
                  elm_object_focus_custom_chain_append(bx2, bt4, NULL);
               }

          }
     }

     {//Third Col
        Evas_Object *bx = elm_box_add(win);
        evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_box_pack_end(mainbx, bx);
        my_show(bx);

          {
             Evas_Object *fr = elm_frame_add(win);
             elm_object_text_set(fr, "Frame");
             elm_box_pack_end(bx, fr);
             evas_object_show(fr);

               {
                  Evas_Object *tb = elm_table_add(win);
                  evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  elm_object_content_set(fr, tb);
                  my_show(tb);
                  for (j = 0; j < 1; j++)
                    for (i = 0; i < 2; i++)
                      {
                         Evas_Object *bt;
                         bt = elm_button_add(win);
                         elm_object_text_set(bt, "Table");
                         evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                         EVAS_HINT_FILL);
                         evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                         elm_table_pack(tb, bt, i, j, 1, 1);
                         my_show(bt);
                      }
               }
          }

          {
             Evas_Object *fr = elm_bubble_add(win);
             elm_object_text_set(fr, "Bubble");
             evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(fr, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             elm_box_pack_end(bx, fr);
             evas_object_show(fr);

               {
                  Evas_Object *tb = elm_table_add(win);
                  evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  elm_object_content_set(fr, tb);
                  my_show(tb);
                  for (j = 0; j < 2; j++)
                    for (i = 0; i < 1; i++)
                      {
                         Evas_Object *bt;
                         bt = elm_button_add(win);
                         elm_object_text_set(bt, "Table");
                         evas_object_size_hint_align_set(bt, EVAS_HINT_FILL,
                                                         EVAS_HINT_FILL);
                         evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                         elm_table_pack(tb, bt, i, j, 1, 1);
                         my_show(bt);
                      }
               }
          }
     }

   ttb = elm_table_add(win);
   evas_object_size_hint_weight_set(ttb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(tbx, ttb);
   my_show(ttb);

     {
        Evas_Object *ck;

        ck = elm_check_add(ttb);
        elm_object_text_set(ck, "Focus Highlight Enable");
        elm_check_state_set(ck, elm_win_focus_highlight_enabled_get(win));
        evas_object_size_hint_align_set(ck, 0.0, EVAS_HINT_FILL);
        elm_table_pack(ttb, ck, 0, 0, 1, 1);
        my_show(ck);
        evas_object_smart_callback_add(ck, "changed",
                                       _focus_highlight_changed,
                                       win);

        ck = elm_check_add(ttb);
        elm_object_text_set(ck, "Focus Highlight Animation Enable");
        elm_check_state_set(ck, elm_win_focus_highlight_animate_get(win));
        evas_object_size_hint_align_set(ck, 0.0, EVAS_HINT_FILL);
        elm_table_pack(ttb, ck, 0, 1, 1, 1);
        my_show(ck);
        evas_object_smart_callback_add(ck, "changed",
                                       _focus_anim_changed,
                                       win);
     }

     {
        Evas_Object *rd, *rdg;

        for (i = 0; i < 3; i++)
          {
             rd = elm_radio_add(ttb);
             elm_radio_state_value_set(rd, i);
             evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(rd, 0.0, EVAS_HINT_FILL);
             elm_table_pack(ttb, rd, 1, i, 1, 1);
             evas_object_show(rd);
             evas_object_smart_callback_add(rd, "changed", _rd_changed_cb, NULL);

             if (i == 0)
               {
                  rdg = rd;
                  elm_object_text_set(rd, "Move Policy: Key+Click(Default)");
               }
             else if (i == 1)
               {
                  elm_radio_group_add(rd, rdg);
                  elm_object_text_set(rd, "Move Policy: Key+Click+In");
               }
             else
               {
                  elm_radio_group_add(rd, rdg);
                  elm_object_text_set(rd, "Move Policy: Key Only");
               }
          }
     }
}

/**** focus 2 ****/

static void
_focus_in(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info)
{
   const char *type = evas_object_type_get(event_info);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(event_info);
   printf("Evas_Object focus in: %p %s\n", event_info, type);
}

static void
_focus_out(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info)
{
   const char *type = evas_object_type_get(event_info);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(event_info);
   printf("Evas_Object focus out: %p %s\n", event_info, type);
}

static void
_focus_obj(void *data, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *newfocus = data;
   const char *type = evas_object_type_get(newfocus);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(newfocus);
   printf("elm_object_focus_set(%p, EINA_TRUE) %s\n", newfocus, type);
   elm_object_focus_set(newfocus, EINA_TRUE);
}

static void
_focus_layout_part(void *data, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *ed = elm_layout_edje_get(data);

   edje_object_freeze(ed);
   Evas_Object *newfocus = (Evas_Object *)edje_object_part_object_get(ed, "sky");
   edje_object_thaw(ed);
   const char *type = evas_object_type_get(newfocus);
   printf("evas_object_focus_set(%p, EINA_TRUE) %s\n", newfocus, type);
   evas_object_focus_set(newfocus, EINA_TRUE);;
}

void
test_focus2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *ly, *bt, *en, *en1, *bt1, *bt2;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("focus2", "Focus 2");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   evas_event_callback_add
     (evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
      _focus_in, NULL);
   evas_event_callback_add
     (evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
      _focus_out, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

#define PARENT bx /* this is broken, but should work */
//#define PARENT win

   en = elm_entry_add(PARENT);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_object_text_set(en, "Entry that should get focus");
   elm_entry_single_line_set(en, EINA_TRUE);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to entry");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, en);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   ly = elm_layout_add(PARENT);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   bt1 = bt = elm_button_add(ly);
   elm_object_text_set(bt, "Button 1");
   elm_object_part_content_set(ly, "element1", bt);

   en1 = elm_entry_add(ly);
   elm_entry_scrollable_set(en1, EINA_TRUE);
   evas_object_size_hint_weight_set(en1, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en1, EVAS_HINT_FILL, 0.5);
   elm_scroller_policy_set(en1, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_object_text_set(en1, "Scrolled Entry that should get focus");
   elm_entry_single_line_set(en1, EINA_TRUE);
   elm_object_part_content_set(ly, "element2", en1);

   bt = elm_button_add(ly);
   elm_object_text_set(bt, "Button 2");
   elm_object_part_content_set(ly, "element3", bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, ly);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout part");
   evas_object_smart_callback_add(bt, "clicked", _focus_layout_part, ly);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout 'Button 1'");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, bt1);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt2 = elm_button_add(PARENT);
   elm_object_text_set(bt2, "Give focus to layout 'Entry'");
   evas_object_smart_callback_add(bt2, "clicked", _focus_obj, en1);
   evas_object_size_hint_weight_set(bt2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt2);
   evas_object_show(bt2);

   elm_object_focus_next_object_set(bt2, en, ELM_FOCUS_DOWN);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

/**** focus hide/del ****/

static Evas_Object *foc = NULL;

static void
_foc(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   foc = obj;
   printf("foc -> %p\n", foc);
}

static void
_unfoc(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   foc = NULL;
   printf("foc -> %p\n", foc);
}

static void
_add(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (foc) evas_object_del(foc);
}

static void
_hide(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (foc) evas_object_hide(foc);
}

void
test_focus_hide_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *en;

   win = elm_win_util_standard_add("focus-hide-del", "Focus Hide/Del");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
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
   elm_object_focus_allow_set(bt, EINA_FALSE);
   elm_object_text_set(bt, "Add");
   evas_object_smart_callback_add(bt, "clicked", _add, bx);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   elm_object_text_set(bt, "Del");
   evas_object_smart_callback_add(bt, "clicked", _del, NULL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   elm_object_text_set(bt, "Hide");
   evas_object_smart_callback_add(bt, "clicked", _hide, NULL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

/**** focus 3 ****/

static Evas_Object *
create_button(Evas_Object *parent, const char *text, Eina_Bool expand)
{
   Evas_Object *btn = elm_button_add(parent);
   elm_object_text_set(btn, text);
   if (expand)
     {
        evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
     }

   evas_object_show(btn);

   return btn;
}

static void
_focus_highlight_clip_disable_changed_cb(void *data EINA_UNUSED,
                                         Evas_Object *obj,
                                         void *event_info EINA_UNUSED)
{
   Eina_Bool disable = elm_check_state_get(obj);
   elm_config_focus_highlight_clip_disabled_set(disable);
}

static void
_horizontal_btn(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *box_btn = data;
   elm_box_horizontal_set(box_btn, elm_check_state_get(obj));
}

void
test_focus3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *sc, *btn_top, *btn_down, *btn[20], *box_btn, *lb, *fr, *ck;

   char win_focus_theme[PATH_MAX] = { 0 };
   char item_name[PATH_MAX];
   int  i;

   snprintf(win_focus_theme, sizeof(win_focus_theme), "%s/objects/test_focus_custom.edj", elm_app_data_dir_get());

   elm_theme_extension_add(NULL, win_focus_theme);

   win = elm_win_util_standard_add("focus3", "Focus 3");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);
   elm_win_focus_highlight_style_set(win, "glow");

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   fr = elm_frame_add(box);
   elm_object_text_set(fr, "Focus Check Points");
   elm_box_pack_end(box, fr);
   evas_object_show(fr);

   lb = elm_label_add(fr);
   elm_object_text_set(lb, "<align=left>This test cases list down the"
                           " focus related check points for regression test.<br/>"
                           " 1. Focus cut for the first/last item by scroller<br/>"
                           " 2. Focus animation on the last item<br/>"
                           " 3. Focus goes out of view port while scrolling<br/>"
                           " 4. Sometimes focus moves but the area of focus does not change<br/>"
                           "    e.g if focus is on 'top' and with down key it comes on Item1 <br/>"
                           "    but the area is same as of top <br/>"
                           " </align>");
   elm_object_content_set(fr, lb);
   evas_object_show(lb);

   btn_top = create_button(box, "top", EINA_FALSE);
   elm_box_pack_end(box, btn_top);

   sc = elm_scroller_add(box);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, sc);

   box_btn = elm_box_add(sc);
   evas_object_size_hint_weight_set(box_btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(sc, box_btn);
   evas_object_show(box_btn);
   evas_object_show(sc);

   for (i = 0; i < 20; i++)
     {
        sprintf(item_name, "Item %d", i);
        btn[i] = create_button(box_btn, item_name, EINA_TRUE);
        elm_box_pack_end(box_btn, btn[i]);
     }

   btn_down = create_button(box, "down", EINA_FALSE);
   evas_object_show(btn_down);
   elm_box_pack_end(box, btn_down);

   ck = elm_check_add(box);
   elm_object_text_set(ck, "Focus Highlight Clip Disable");
   elm_check_state_set(ck, elm_config_focus_highlight_clip_disabled_get());
   elm_box_pack_end(box, ck);
   evas_object_show(ck);
   evas_object_smart_callback_add(ck, "changed",
                                  _focus_highlight_clip_disable_changed_cb,
                                  NULL);

   // Focus Autoscroll Mode
   fr = _focus_autoscroll_mode_frame_create(box);
   elm_box_pack_end(box, fr);

   ck = elm_check_add(box);
   elm_object_text_set(ck, "Horizontal Mode");
   evas_object_smart_callback_add(ck, "changed", _horizontal_btn, box_btn);
   elm_box_pack_end(box, ck);
   evas_object_show(ck);

   evas_object_resize(win, 320, 500);
   evas_object_show(win);
}

/**** focus 4 ****/

static void
btn_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt = data;
   elm_object_focus_set(bt, EINA_FALSE);
   elm_object_focus_set(bt, EINA_TRUE);
}

static Eina_Bool toggle = EINA_FALSE;
static void
btn_clicked2(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt = data;
   toggle = !toggle;
   if (toggle)
     evas_object_size_hint_min_set(bt, 500, 500);
   else
     evas_object_size_hint_min_set(bt, 100, 100);
}

void
test_focus4(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *sc, *btn, *ly, *btn2;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("focus4", "Focus 4");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   sc = elm_scroller_add(box);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, 0.9);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end (box, sc);
   evas_object_show(sc);

   btn = elm_button_add(box);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, 0.1);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, btn);
   elm_object_text_set(btn, "Focus to Button");
   evas_object_show(btn);

   ly = elm_layout_add(sc);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "layout3");
   elm_object_content_set(sc, ly);
   evas_object_show(ly);

   btn2 = elm_button_add(ly);
   elm_object_text_set(btn2, "Button Resize");
   elm_object_part_content_set(ly, "swallow", btn2);
   evas_object_show(btn2);

   evas_object_smart_callback_add(btn, "clicked", btn_clicked, btn2);
   evas_object_smart_callback_add(btn2, "clicked", btn_clicked2, btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

/**** focus 5 ****/

struct _focus5_obj {
   const char *name;
   Evas_Coord x, y, w, h;
};

struct _focus5_obj _focus5_layout_data1[] = {
   {"top",    40,  0, 20, 10},
   {"bottom", 40, 90, 20, 10},
   {"left",    0, 45, 20, 10},
   {"right",  80, 45, 20, 10},
   {NULL, 0, 0, 0, 0} /* sentinel */
};

struct _focus5_obj _focus5_layout_data2[] = {
   {"top L",   0,  0, 20, 10},
   {"top R",  80,  0, 20, 10},
   {"bot L",   0, 90, 20, 10},
   {"bot R",  80, 90, 20, 10},
   {"center", 40, 40, 20, 10},
   {NULL, 0, 0, 0, 0} /* sentinel */
};

struct _focus5_obj _focus5_layout_data3[] = {
   {"top",    40,  0, 20, 10},
   {"bottom", 40, 90, 20, 10},
   {"left",    0, 45, 20, 10},
   {"right",  80, 45, 20, 10},
   {"top L",   0,  0, 20, 10},
   {"top R",  80,  0, 20, 10},
   {"bot L",   0, 90, 20, 10},
   {"bot R",  80, 90, 20, 10},
   {"center", 40, 45, 20, 10},
   {NULL, 0, 0, 0, 0} /* sentinel */
};

void
_focus5_layout(Evas_Object *grid, struct _focus5_obj *layout)
{
   Evas_Object *obj;

   evas_object_data_set(grid, "layout", layout);
   elm_grid_clear(grid, EINA_TRUE);

   while(layout->name) {
      printf("button: %s\n", layout->name);
      obj = elm_button_add(grid);
      efl_name_set(obj, layout->name);
      elm_object_text_set(obj, layout->name);
      elm_grid_pack(grid, obj, layout->x, layout->y, layout->w, layout->h);
      evas_object_show(obj);
      layout++;
   }
}

static void
_focus5_btn_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = data;
   struct _focus5_obj *layout = evas_object_data_get(grid, "layout");

   // brrr...a really naive looping
   if (layout == _focus5_layout_data1)
      _focus5_layout(grid, _focus5_layout_data2);
   else if (layout == _focus5_layout_data2)
      _focus5_layout(grid, _focus5_layout_data3);
   // else if (layout == _focus5_layout_data3)
   //    _focus5_layout(grid, _focus5_layout_data4);
   else
      _focus5_layout(grid, _focus5_layout_data1);

}

void
test_focus5(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *sep, *btn, *grid;

   win = elm_win_util_standard_add("focus5", "Focus 5");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   grid = elm_grid_add(box);
   elm_grid_size_set(grid, 100, 100);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, grid);
   evas_object_show(grid);
   _focus5_layout(grid, _focus5_layout_data1);

   sep = elm_separator_add(box);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(box, sep);
   evas_object_show(sep);

   btn = elm_button_add(box);
   elm_object_focus_allow_set(btn, EINA_FALSE);
   elm_box_pack_end(box, btn);
   elm_object_text_set(btn, "Show next layout  (this btn is NOT focusable)");
   evas_object_smart_callback_add(btn, "clicked", _focus5_btn_clicked, grid);
   evas_object_show(btn);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

/**** focus 6 ****/
static char *
_focus6_gl_text_get(void *data, Evas_Object *obj EINA_UNUSED,
                    const char *part EINA_UNUSED)
{
   char buf[32];
   snprintf(buf, sizeof(buf), "Focus item %d", (int)(uintptr_t)data);
   return strdup(buf);
}

static void
_focus6_btn_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ly = data;
   Elm_Focus_Direction dir = (uintptr_t)evas_object_data_get(obj, "direction");

   elm_object_focus_next(ly, dir);
}

void
test_focus6(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *box2, *sep, *ly, *lb, *btn, *gl;
   Elm_Genlist_Item_Class *itc;
   Elm_Object_Item *it;
   char buf[PATH_MAX];
   int i;

   win = elm_win_util_standard_add("focus6", "Focus 6");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   // main vertical box
   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   // main layout
   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "focus_test_6");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ly);
   evas_object_show(ly);

   lb = elm_label_add(ly);
   elm_object_text_set(lb, "The game is to reach the buttons and the list items"
                           " using only the keyboard");
   elm_layout_content_set(ly, "label_swallow", lb);

   // genlist in a swallow
   gl = elm_genlist_add(ly);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_genlist_select_mode_set(gl, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_layout_content_set(ly, "list_swallow", gl);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = _focus6_gl_text_get;
   for (i = 0; i < 3; i++)
     {
        it = elm_genlist_item_append(gl, itc, (void*)(uintptr_t)i, NULL,
                                     ELM_GENLIST_ITEM_NONE, NULL, NULL);
        
        if (i == 1)
          {
             elm_genlist_item_selected_set(it, EINA_TRUE);

             /* focus should start from second item */
             elm_object_item_focus_set(it, EINA_TRUE);
          }
     }
   elm_genlist_item_class_free(itc);

   // 3 buttons in an edje box
   for (i = 0; i < 3; i++)
     {
        btn = elm_button_add(ly);
        elm_object_text_set(btn, "btn");
        elm_layout_box_append(ly, "box", btn);
        evas_object_show(btn);
        /* focus should start from second button */
        // if (i == 1)
          // elm_object_focus_set(btn, EINA_TRUE);
     }

   // 4 buttons (not focusable) to test focus move by API
   sep = elm_separator_add(win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(box, sep);
   evas_object_show(sep);

   box2 = elm_box_add(win);
   elm_box_horizontal_set(box2, EINA_TRUE);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   lb = elm_label_add(ly);
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   elm_object_text_set(lb, "Move the focus using elm_object_focus_next()");
   elm_box_pack_end(box2, lb);
   evas_object_show(lb);
   
   btn = elm_button_add(win);
   elm_object_text_set(btn, "LEFT");
   elm_object_focus_allow_set(btn, EINA_FALSE);
   evas_object_data_set(btn, "direction", (void *)(uintptr_t)ELM_FOCUS_LEFT);
   evas_object_smart_callback_add(btn, "clicked", _focus6_btn_clicked, ly);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "UP");
   elm_object_focus_allow_set(btn, EINA_FALSE);
   evas_object_data_set(btn, "direction", (void *)(uintptr_t)ELM_FOCUS_UP);
   evas_object_smart_callback_add(btn, "clicked", _focus6_btn_clicked, ly);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "DOWN");
   elm_object_focus_allow_set(btn, EINA_FALSE);
   evas_object_data_set(btn, "direction", (void *)(uintptr_t)ELM_FOCUS_DOWN);
   evas_object_smart_callback_add(btn, "clicked", _focus6_btn_clicked, ly);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "RIGHT");
   elm_object_focus_allow_set(btn, EINA_FALSE);
   evas_object_data_set(btn, "direction", (void *)(uintptr_t)ELM_FOCUS_RIGHT);
   evas_object_smart_callback_add(btn, "clicked", _focus6_btn_clicked, ly);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   // size and show the window
   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

void
test_focus7(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *btn, *gl;
   Elm_Genlist_Item_Class *itc;
   Elm_Object_Item *it;
   int i;

   win = elm_win_util_standard_add("focus7", "Focus 7");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   // main vertical box
   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   // genlist in a swallow
   gl = elm_genlist_add(box);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_genlist_select_mode_set(gl, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_box_pack_end(box, gl);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = _focus6_gl_text_get;
   for (i = 0; i < 3; i++)
     {
        it = elm_genlist_item_append(gl, itc, (void*)(uintptr_t)i, NULL,
                                     ELM_GENLIST_ITEM_NONE, NULL, NULL);

        if (i == 1)
          {
             elm_genlist_item_selected_set(it, EINA_TRUE);

             /* focus should start from second item */
             elm_object_item_focus_set(it, EINA_TRUE);
          }
     }
   elm_genlist_item_class_free(itc);
   evas_object_show(gl);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "To the right will result in genlist focus");
   elm_object_focus_next_item_set(btn, it, ELM_FOCUS_RIGHT);
   elm_box_pack_end(box, btn);
   evas_object_show(btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "UP");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);

   // size and show the window
   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
