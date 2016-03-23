#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_print_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("bubble clicked\n");
}

static void
my_entry_anchor_bt(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *av = data;
   elm_entry_anchor_hover_end(av);
}

static void
my_entry_anchor(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *av = data;
   Elm_Entry_Anchor_Hover_Info *ei = event_info;
   Evas_Object *bt, *bx;

   bt = elm_button_add(obj);
   elm_object_text_set(bt, ei->anchor_info->name);
   elm_object_part_content_set(ei->hover, "middle", bt);
   evas_object_show(bt);

   // hints as to where we probably should put hover contents (buttons etc.).
   if (ei->hover_top)
     {
        bx = elm_box_add(obj);
        bt = elm_button_add(obj);
        elm_object_text_set(bt, "Top 1");
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", my_entry_anchor_bt, av);
        evas_object_show(bt);
        bt = elm_button_add(obj);
        elm_object_text_set(bt, "Top 2");
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", my_entry_anchor_bt, av);
        evas_object_show(bt);
        bt = elm_button_add(obj);
        elm_object_text_set(bt, "Top 3");
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", my_entry_anchor_bt, av);
        evas_object_show(bt);
        elm_object_part_content_set(ei->hover, "top", bx);
        evas_object_show(bx);
     }
   if (ei->hover_bottom)
     {
        bt = elm_button_add(obj);
        elm_object_text_set(bt, "Bot");
        elm_object_part_content_set(ei->hover, "bottom", bt);
        evas_object_smart_callback_add(bt, "clicked", my_entry_anchor_bt, av);
        evas_object_show(bt);
     }
   if (ei->hover_left)
     {
        bt = elm_button_add(obj);
        elm_object_text_set(bt, "Left");
        elm_object_part_content_set(ei->hover, "left", bt);
        evas_object_smart_callback_add(bt, "clicked", my_entry_anchor_bt, av);
        evas_object_show(bt);
     }
   if (ei->hover_right)
     {
        bt = elm_button_add(obj);
        elm_object_text_set(bt, "Right");
        elm_object_part_content_set(ei->hover, "right", bt);
        evas_object_smart_callback_add(bt, "clicked", my_entry_anchor_bt, av);
        evas_object_show(bt);
     }
}

static void
my_entry_anchor_edge_left(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("left\n");
}

static void
my_entry_anchor_edge_right(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("right\n");
}

static void
my_entry_anchor_edge_top(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("top\n");
}

static void
my_entry_anchor_edge_bottom(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("bottom\n");
}

static void
my_entry_anchor_scroll(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord x, y, w, h, vw, vh;

   elm_scroller_region_get(obj, &x, &y, &w, &h);
   elm_scroller_child_size_get(obj, &vw, &vh);
   printf("scroll %ix%i +%i+%i in %ix%i\n", w, h, x, y, vw, vh);
}

void
test_entry_anchor(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *en, *sc, *bx, *bb, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("entry_anchor", "Entry Anchor");
   elm_win_autodel_set(win, EINA_TRUE);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   evas_object_smart_callback_add(sc, "edge,left", my_entry_anchor_edge_left, NULL);
   evas_object_smart_callback_add(sc, "edge,right", my_entry_anchor_edge_right, NULL);
   evas_object_smart_callback_add(sc, "edge,top", my_entry_anchor_edge_top, NULL);
   evas_object_smart_callback_add(sc, "edge,bottom", my_entry_anchor_edge_bottom, NULL);
   evas_object_smart_callback_add(sc, "scroll", my_entry_anchor_scroll, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 3");
   elm_object_part_text_set(bb, "info", "10:32 4/11/2008");
   elm_object_part_content_set(bb, "icon", ic);
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   en = elm_entry_add(win);
   elm_entry_anchor_hover_style_set(en, "popout");
   elm_entry_anchor_hover_parent_set(en, win);
   elm_object_text_set(en,
                       "Hi there. This is the most recent message in the "
                       "list of messages. It has one <a href=tel:+614321234>+61 432 1234</a> "
                       "(phone number) to click on.");
   evas_object_smart_callback_add(en, "anchor,hover,opened",
                                  my_entry_anchor, en);
   elm_object_content_set(bb, en);
   evas_object_show(en);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 2");
   elm_object_part_text_set(bb, "info", "7:16 27/10/2008");
   elm_object_part_content_set(bb, "icon", ic);
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   en = elm_entry_add(win);
   elm_entry_anchor_hover_style_set(en, "popout");
   elm_entry_anchor_hover_parent_set(en, win);
   elm_object_text_set(en,
                       "Hey what are you doing? This is the second last message "
                       "Hi there. This is the most recent message in the "
                       "list. It's a longer one so it can wrap more and "
                       "contains a <a href=contact:john>John</a> contact "
                       "link in it to test popups on links. The idea is that "
                       "all SMS's are scanned for things that look like phone "
                       "numbers or names that are in your contacts list, and "
                       "if they are, they become clickable links that pop up "
                       "a menus of obvious actions to perform on this piece "
                       "of information. This of course can be later explicitly "
                       "done by links maybe running local apps or even being "
                       "web URL's too that launch the web browser and point it "
                       "to that URL. <item relsize=16x16 vsize=full href=emoticon/omg></item>");
   evas_object_smart_callback_add(en, "anchor,hover,opened", my_entry_anchor, en);
   elm_object_content_set(bb, en);
   evas_object_show(en);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 1");
   elm_object_part_text_set(bb, "info", "20:47 18/6/2008");
   elm_object_part_content_set(bb, "icon", ic);
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   en = elm_entry_add(win);
   elm_entry_anchor_hover_style_set(en, "popout");
   elm_entry_anchor_hover_parent_set(en, win);
   elm_object_text_set(en, "This is a short message. <item relsize=16x16 vsize=full href=emoticon/haha></item>");
   evas_object_smart_callback_add(en, "anchor,hover,opened",
                                  my_entry_anchor, en);
   elm_object_content_set(bb, en);
   evas_object_show(en);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   elm_object_content_set(sc, bx);
   evas_object_show(bx);

   evas_object_show(sc);

   elm_object_focus_set(win, EINA_TRUE);
   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}
