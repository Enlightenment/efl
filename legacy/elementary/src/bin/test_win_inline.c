#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
fill(Evas_Object *win, Eina_Bool do_bg)
{
   Evas_Object *bg, *sc, *bx, *ic, *bb, *av, *en;
   char buf[PATH_MAX];

   if (do_bg)
     {
        bg = elm_bg_add(win);
        elm_win_resize_object_add(win, bg);
        evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_show(bg);
     }

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_entry_set(en, "This is a single line");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_entry_set(en, "Entry 2");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 3");
   elm_object_text_part_set(bb, "info", "10:32 4/11/2008");
   elm_bubble_icon_set(bb, ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   av = elm_anchorblock_add(win);
   elm_anchorblock_hover_style_set(av, "popout");
   elm_anchorblock_hover_parent_set(av, win);
   elm_object_text_set(av,
			   "Hi there. This is the most recent message in the "
			   "list of messages. It has one <a href=tel:+614321234>+61 432 1234</a> "
			   "(phone number) to click on.");
   elm_bubble_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 2");
   elm_object_text_part_set(bb, "info", "7:16 27/10/2008");
   elm_bubble_icon_set(bb, ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   av = elm_anchorblock_add(win);
   elm_anchorblock_hover_style_set(av, "popout");
   elm_anchorblock_hover_parent_set(av, win);
   elm_object_text_set(av,
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
   elm_bubble_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 1");
   elm_object_text_part_set(bb, "info", "20:47 18/6/2008");
   elm_bubble_icon_set(bb, ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   av = elm_anchorblock_add(win);
   elm_anchorblock_hover_style_set(av, "popout");
   elm_anchorblock_hover_parent_set(av, win);
   elm_object_text_set(av, "This is a short message. <item relsize=16x16 vsize=full href=emoticon/haha></item>");
   elm_bubble_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   elm_scroller_content_set(sc, bx);
   evas_object_show(bx);

   evas_object_show(sc);
}

static void
cb_mouse_move(void *data, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *orig = data;
   Evas_Coord x, y;
   Evas_Map *p;
   int i, w, h;

   if (!ev->buttons) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(obj,
                    x + (ev->cur.canvas.x - ev->prev.output.x),
                    y + (ev->cur.canvas.y - ev->prev.output.y));
   evas_object_image_size_get(orig, &w, &h);
   p = evas_map_new(4);
   evas_object_map_enable_set(orig, EINA_TRUE);
   evas_object_raise(orig);
   for (i = 0; i < 4; i++)
     {
        Evas_Object *hand;
        char key[32];

        snprintf(key, sizeof(key), "h-%i\n", i);
        hand = evas_object_data_get(orig, key);
        evas_object_raise(hand);
        evas_object_geometry_get(hand, &x, &y, NULL, NULL);
        x += 15;
        y += 15;
        evas_map_point_coord_set(p, i, x, y, 0);
        if (i == 0) evas_map_point_image_uv_set(p, i, 0, 0);
        else if (i == 1) evas_map_point_image_uv_set(p, i, w, 0);
        else if (i == 2) evas_map_point_image_uv_set(p, i, w, h);
        else if (i == 3) evas_map_point_image_uv_set(p, i, 0, h);
     }
   evas_object_map_set(orig, p);
   evas_map_free(p);
}

static void
create_handles(Evas_Object *obj)
{
   int i;
   Evas_Coord x, y, w, h;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   for (i = 0; i < 4; i++)
     {
        Evas_Object *hand;
        char buf[PATH_MAX];
        char key[32];

        hand = evas_object_image_filled_add(evas_object_evas_get(obj));
        evas_object_resize(hand, 31, 31);
        snprintf(buf, sizeof(buf), "%s/images/pt.png", PACKAGE_DATA_DIR);
        evas_object_image_file_set(hand, buf, NULL);
        if (i == 0)      evas_object_move(hand, x     - 15, y     - 15);
        else if (i == 1) evas_object_move(hand, x + w - 15, y     - 15);
        else if (i == 2) evas_object_move(hand, x + w - 15, y + h - 15);
        else if (i == 3) evas_object_move(hand, x     - 15, y + h - 15);
        evas_object_event_callback_add(hand, EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move, obj);
        evas_object_show(hand);
        snprintf(key, sizeof(key), "h-%i\n", i);
        evas_object_data_set(obj, key, hand);
     }
}

void
test_win_inline(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *win2, *win3;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "window-inline", ELM_WIN_BASIC);
   elm_win_title_set(win, "Window Inline");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   win2 = elm_win_add(win, "inlined", ELM_WIN_INLINED_IMAGE);
   fill(win2, EINA_TRUE);

   evas_object_move(win2, 20, 60);
   evas_object_resize(win2, 300, 200);
   // image object for win2 is unlinked to its pos/size - so manual control
   // this allows also for using map and other things with it.
   evas_object_move(elm_win_inlined_image_object_get(win2), 20, 40);
   evas_object_resize(elm_win_inlined_image_object_get(win2), 200, 320);
   evas_object_show(win2);

   win3 = elm_win_add(win, "inlined", ELM_WIN_INLINED_IMAGE);
   elm_win_alpha_set(win3, EINA_TRUE);
   fill(win3, EINA_FALSE);

   evas_object_resize(win3, 300, 200);
   evas_object_move(elm_win_inlined_image_object_get(win3), 80, 180);
   evas_object_resize(elm_win_inlined_image_object_get(win3), 300, 200);
   evas_object_show(win3);

   create_handles(elm_win_inlined_image_object_get(win3));

   evas_object_resize(win, 400, 600);
   evas_object_show(win);
}
#endif
