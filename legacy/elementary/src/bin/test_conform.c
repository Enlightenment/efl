#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

void 
test_conformant(void *data, Evas_Object *obj, void *event) 
{
   Evas_Object *win, *bg, *conform, *btn, *bx, *en;

   win = elm_win_add(NULL, "conformant", ELM_WIN_BASIC);
   elm_win_title_set(win, "Conformant");
   elm_win_autodel_set(win, 1);
   elm_win_conformant_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   conform = elm_conformant_add(win);
   elm_win_resize_object_add(win, conform);
   evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(conform);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   
   en = elm_scrolled_entry_add(win);
   elm_scrolled_entry_single_line_set(en, 1);
   elm_scrolled_entry_bounce_set(en, 1, 0);
   elm_scrolled_entry_entry_set(en, "This is the top entry here");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   evas_object_show(en);
   elm_box_pack_end(bx, en);
  
   btn = elm_button_add(win);
   elm_button_label_set(btn, "Test Conformant");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   en = elm_scrolled_entry_add(win);
   elm_scrolled_entry_single_line_set(en, 1);
   elm_scrolled_entry_bounce_set(en, 1, 0);
   elm_scrolled_entry_entry_set(en, "This is the middle entry here");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   evas_object_show(en);
   elm_box_pack_end(bx, en);
  
   btn = elm_button_add(win);
   elm_button_label_set(btn, "Test Conformant");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   en = elm_scrolled_entry_add(win);
   elm_scrolled_entry_bounce_set(en, 0, 1);
   elm_scrolled_entry_entry_set(en, 
                       "This is a multi-line entry at the bottom<br>"
                       "This can contain more than 1 line of text and be "
                       "scrolled around to allow for entering of lots of "
                       "content. It is also to test to see that autoscroll "
                       "moves to the right part of a larger multi-line "
                       "text entry that is inside of a scroller than can be "
                       "scrolled around, thus changing the expected position "
                       "as well as cursor changes updating auto-scroll when "
                       "it is enabled.");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);
  
   elm_conformant_content_set(conform, bx);
   evas_object_show(bx);
   
   evas_object_resize(win, 240, 240);
   evas_object_show(win);
}

#endif
