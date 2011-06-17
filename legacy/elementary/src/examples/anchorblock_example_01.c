/*
 * gcc -o anchorblock_example_01 anchorblock_example_01.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void _anchorblock_clicked_cb(void *data, Evas_Object *obj, void *ev);
static void _anchorview_clicked_cb(void *data, Evas_Object *obj, void *ev);

int
elm_main(int argc __UNUSED__, char *argv[] __UNUSED__)
{
   Evas_Object *win, *box, *o, *frame;
   const char *anchortext =
      "Example of some markup text, a long one at that, using anchors in"
      "different ways, like <a href=random>some random text</a>.<br>"
      "Something more useful is to have actions over urls, either in direct"
      "form: <a href=url:http://www.enlightenment.org>"
      "http://www.enlightenment.org</a> or with "
      "<a href=url:http://www.enlightenment.org>more random text</a>.<br>"
      "In any case, the href part of the anchor is what you will receive on"
      "the callback, so it's a good idea to keep anything needed to identify"
      "whatever is linked at in there.<br>"
      "Playing to be a phone, we can also identify contacts from our address"
      "list, like this dude here <a href=contact:42>Thomas Anderson</a>, or"
      "phone numbers <a href=tel:+61432123>+61 432 1234</a>.";

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "Anchortwins example", ELM_WIN_BASIC);
   elm_win_title_set(win, "Anchortwins example");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 320, 300);
   evas_object_show(win);

   o = elm_bg_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   frame = elm_frame_add(win);
   elm_frame_label_set(frame, "Anchorblock");
   evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(frame);
   elm_box_pack_end(box, frame);

   o = elm_anchorblock_add(win);
   elm_anchorblock_hover_style_set(o, "popout");
   elm_anchorblock_hover_parent_set(o, win);
   elm_anchorblock_text_set(o, anchortext);
   evas_object_smart_callback_add(o, "anchor,clicked", _anchorblock_clicked_cb,
                                  NULL);
   evas_object_show(o);
   elm_frame_content_set(frame, o);

   frame = elm_frame_add(win);
   elm_frame_label_set(frame, "Anchorview");
   evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(frame);
   elm_box_pack_end(box, frame);

   o = elm_anchorview_add(win);
   elm_anchorview_hover_parent_set(o, frame);
   elm_anchorview_bounce_set(o, EINA_FALSE, EINA_TRUE);
   elm_anchorview_text_set(o, anchortext);
   evas_object_smart_callback_add(o, "anchor,clicked", _anchorview_clicked_cb,
                                  NULL);
   evas_object_show(o);
   elm_frame_content_set(frame, o);

   elm_run();

   return 0;
}
ELM_MAIN();

static void _anchor_buttons_create(Evas_Object *ao, Elm_Entry_Anchorblock_Info *info, Evas_Smart_Cb btn_end_cb);

static void
_btn_anchorblock_end_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_anchorblock_hover_end((Evas_Object *)data);
}

static void
_anchorblock_clicked_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Elm_Entry_Anchorblock_Info *info = event_info;
   _anchor_buttons_create(obj, info, _btn_anchorblock_end_cb);
}

static void
_btn_anchorview_end_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_anchorview_hover_end((Evas_Object *)data);
}

static void
_anchorview_clicked_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   /* should be an Elm_Entry_Anchorview_Info, but since both of them are
    * the same, it simplifies code in this example to use one only */
   Elm_Entry_Anchorblock_Info *info = event_info;
   _anchor_buttons_create(obj, info, _btn_anchorview_end_cb);
}

static void
_btn_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   const char *lbl = elm_button_label_get(obj);
   printf("%s: %s\n", lbl, (char *)data);
   eina_stringshare_del(data);
}

static void
_anchor_buttons_create(Evas_Object *ao, Elm_Entry_Anchorblock_Info *info, Evas_Smart_Cb btn_end_cb)
{
   Evas_Object *btn, *secondary = NULL, *box = NULL;
   char *p;
   const char *str;

   btn = elm_button_add(ao);
   evas_object_show(btn);

   p = strchr(info->name, ':');
   if (!p)
     {
        elm_button_label_set(btn, "Nothing to see here");
        evas_object_smart_callback_add(btn, "clicked", btn_end_cb, ao);
        elm_hover_content_set(info->hover, "middle", btn);
        return;
     }

   str = eina_stringshare_add(p + 1);
   if (!strncmp(info->name, "tel:", 4))
     {
        Evas_Object *o;
        elm_button_label_set(btn, "Call");

        secondary = elm_button_add(ao);
        elm_button_label_set(secondary, "Send SMS");
        evas_object_show(secondary);
        evas_object_smart_callback_add(secondary, "clicked", btn_end_cb, ao);
        evas_object_smart_callback_add(secondary, "clicked", _btn_clicked_cb,
                                       str);

        box = elm_box_add(ao);
        evas_object_show(box);

        o = elm_button_add(ao);
        elm_button_label_set(o, "Add to contacts");
        elm_box_pack_end(box, o);
        evas_object_show(o);
        evas_object_smart_callback_add(o, "clicked", btn_end_cb, ao);
        evas_object_smart_callback_add(o, "clicked", _btn_clicked_cb, str);

        o = elm_button_add(ao);
        elm_button_label_set(o, "Send MMS");
        elm_box_pack_end(box, o);
        evas_object_show(o);
        evas_object_smart_callback_add(o, "clicked", btn_end_cb, ao);
        evas_object_smart_callback_add(o, "clicked", _btn_clicked_cb, str);
     }
   else if (!strncmp(info->name, "contact:", 8))
     {
        Evas_Object *o;
        elm_button_label_set(btn, "Call");

        secondary = elm_button_add(ao);
        elm_button_label_set(secondary, "Send SMS");
        evas_object_show(secondary);
        evas_object_smart_callback_add(secondary, "clicked", btn_end_cb, ao);
        evas_object_smart_callback_add(secondary, "clicked", _btn_clicked_cb,
                                       str);

        box = elm_box_add(ao);
        evas_object_show(box);

        o = elm_button_add(ao);
        elm_button_label_set(o, "Send MMS");
        elm_box_pack_end(box, o);
        evas_object_show(o);
        evas_object_smart_callback_add(o, "clicked", btn_end_cb, ao);
        evas_object_smart_callback_add(o, "clicked", _btn_clicked_cb, str);
     }
   else if (!strncmp(info->name, "mailto:", 7))
     {
        elm_button_label_set(btn, "Send E-Mail");

        secondary = elm_button_add(ao);
        elm_button_label_set(secondary, "Add to contacts");
        evas_object_show(secondary);
        evas_object_smart_callback_add(secondary, "clicked", btn_end_cb, ao);
        evas_object_smart_callback_add(secondary, "clicked", _btn_clicked_cb,
                                       str);
     }
   else if (!strncmp(info->name, "url:", 4))
     {
        Evas_Object *o;
        elm_button_label_set(btn, "Launch in browser");

        box = elm_box_add(ao);
        evas_object_show(box);

        o = elm_button_add(ao);
        elm_button_label_set(o, "Send as mail to...");
        elm_box_pack_end(box, o);
        evas_object_show(o);
        evas_object_smart_callback_add(o, "clicked", btn_end_cb, ao);
        evas_object_smart_callback_add(o, "clicked", _btn_clicked_cb, str);

        o = elm_button_add(ao);
        elm_button_label_set(o, "Send as SMS to...");
        elm_box_pack_end(box, o);
        evas_object_show(o);
        evas_object_smart_callback_add(o, "clicked", btn_end_cb, ao);
        evas_object_smart_callback_add(o, "clicked", _btn_clicked_cb, str);
     }

   evas_object_smart_callback_add(btn, "clicked", btn_end_cb, ao);
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked_cb, str);
   elm_hover_content_set(info->hover, "middle", btn);

   if (secondary)
     {
        if (info->hover_right)
          elm_hover_content_set(info->hover, "right", secondary);
        else if (info->hover_left)
          elm_hover_content_set(info->hover, "left", secondary);
        else
          evas_object_del(secondary);
     }

   if (box)
     {
        if (info->hover_bottom)
          elm_hover_content_set(info->hover, "bottom", box);
        else if (info->hover_top)
          elm_hover_content_set(info->hover, "top", box);
        else
          evas_object_del(box);
     }
}
