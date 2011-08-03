#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
_print_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("bubble clicked\n");
}

static void
my_anchorblock_bt(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *av = data;
   elm_anchorblock_hover_end(av);
}

static void
my_anchorblock_anchor(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *av = data;
   Elm_Entry_Anchorblock_Info *ei = event_info;
   Evas_Object *bt, *bx;

   bt = elm_button_add(obj);
   elm_object_text_set(bt, ei->name);
   elm_hover_content_set(ei->hover, "middle", bt);
   evas_object_show(bt);

   // hints as to where we probably should put hover contents (buttons etc.).
   if (ei->hover_top)
     {
	bx = elm_box_add(obj);
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Top 1");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Top 2");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Top 3");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
	elm_hover_content_set(ei->hover, "top", bx);
	evas_object_show(bx);
     }
   if (ei->hover_bottom)
     {
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Bot");
	elm_hover_content_set(ei->hover, "bottom", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
     }
   if (ei->hover_left)
     {
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Left");
	elm_hover_content_set(ei->hover, "left", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
     }
   if (ei->hover_right)
     {
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Right");
	elm_hover_content_set(ei->hover, "right", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
     }
}

static void
my_anchorblock_edge_left(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("left\n");
}

static void
my_anchorblock_edge_right(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("right\n");
}

static void
my_anchorblock_edge_top(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("top\n");
}

static void
my_anchorblock_edge_bottom(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("bottom\n");
}

static void
my_anchorblock_scroll(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord x, y, w, h, vw, vh;

   elm_scroller_region_get(obj, &x, &y, &w, &h);
   elm_scroller_child_size_get(obj, &vw, &vh);
   printf("scroll %ix%i +%i+%i in %ix%i\n", w, h, x, y, vw, vh);
}

void
test_anchorblock(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *av, *sc, *bx, *bb, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "anchorblock", ELM_WIN_BASIC);
   elm_win_title_set(win, "Anchorblock");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   evas_object_smart_callback_add(sc, "edge_left", my_anchorblock_edge_left, NULL);
   evas_object_smart_callback_add(sc, "edge_right", my_anchorblock_edge_right, NULL);
   evas_object_smart_callback_add(sc, "edge_top", my_anchorblock_edge_top, NULL);
   evas_object_smart_callback_add(sc, "edge_bottom", my_anchorblock_edge_bottom, NULL);
   evas_object_smart_callback_add(sc, "scroll", my_anchorblock_scroll, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 3");
   elm_object_text_part_set(bb, "info", "10:32 4/11/2008");
   elm_bubble_icon_set(bb, ic);
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
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
   evas_object_smart_callback_add(av, "anchor,clicked",
                                  my_anchorblock_anchor, av);
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
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
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
   evas_object_smart_callback_add(av, "anchor,clicked", my_anchorblock_anchor, av);
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
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   av = elm_anchorblock_add(win);
   elm_anchorblock_hover_style_set(av, "popout");
   elm_anchorblock_hover_parent_set(av, win);
   elm_object_text_set(av, "This is a short message. <item relsize=16x16 vsize=full href=emoticon/haha></item>");
   evas_object_smart_callback_add(av, "anchor,clicked",
                                  my_anchorblock_anchor, av);
   elm_bubble_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   elm_scroller_content_set(sc, bx);
   evas_object_show(bx);

   evas_object_show(sc);

   evas_object_resize(win, 320, 300);

   elm_object_focus_set(win, EINA_TRUE);
   evas_object_show(win);
}
#endif
