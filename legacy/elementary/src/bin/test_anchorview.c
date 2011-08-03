#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
my_anchorview_bt(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *av = data;
   elm_anchorview_hover_end(av);
}

static void
my_anchorview_anchor(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *av = data;
   Elm_Entry_Anchorview_Info *ei = event_info;
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
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Top 2");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Top 3");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
	elm_hover_content_set(ei->hover, "top", bx);
	evas_object_show(bx);
     }
   if (ei->hover_bottom)
     {
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Bot");
	elm_hover_content_set(ei->hover, "bottom", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
     }
   if (ei->hover_left)
     {
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Left");
	elm_hover_content_set(ei->hover, "left", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
     }
   if (ei->hover_right)
     {
	bt = elm_button_add(obj);
	elm_object_text_set(bt, "Right");
	elm_hover_content_set(ei->hover, "right", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
     }
}

void
test_anchorview(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *av;

   win = elm_win_add(NULL, "anchorview", ELM_WIN_BASIC);
   elm_win_title_set(win, "Anchorview");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   av = elm_anchorview_add(win);
   elm_anchorview_hover_style_set(av, "popout");
   elm_anchorview_hover_parent_set(av, win);
   elm_object_text_set(av,
		       "This is an entry widget in this window that<br>"
		       "uses markup <b>like this</> for styling and<br>"
		       "formatting <em>like this</>, as well as<br>"
		       "<a href=X><link>links in the text</></a>, so enter text<br>"
		       "in here to edit it. By the way, links are<br>"
		       "called <a href=anc-02>Anchors</a> so you will need<br>"
		       "to refer to them this way. <item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>");
   evas_object_size_hint_weight_set(av, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(av, "anchor,clicked", my_anchorview_anchor, av);
   elm_win_resize_object_add(win, av);
   evas_object_show(av);

   evas_object_resize(win, 320, 300);

   elm_object_focus_set(win, EINA_TRUE);
   evas_object_show(win);
}
#endif
