#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_conformant(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *win, *conform, *btn, *bx, *en;

   win = elm_win_util_standard_add("conformant", "Conformant");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_conformant_set(win, EINA_TRUE);

   conform = elm_conformant_add(win);
   evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, conform);
   evas_object_show(conform);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_scroller_bounce_set(en, EINA_TRUE, EINA_FALSE);
   elm_object_text_set(en, "This is the top entry here");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Test Conformant");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_scroller_bounce_set(en, EINA_TRUE, EINA_FALSE);
   elm_object_text_set(en, "This is the middle entry here");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Test Conformant");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_scroller_bounce_set(en, EINA_FALSE, EINA_TRUE);
   elm_object_text_set(en,
                       "This is a multi-line entry at the bottom<br/>"
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

   elm_object_content_set(conform, bx);
   evas_object_show(bx);

   evas_object_resize(win, 240, 240);
   evas_object_show(win);
}

static void
popobj(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   elm_naviframe_item_pop(data);
}

void
test_conformant2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *win, *conform, *btn, *bx, *en, *pg;

   win = elm_win_util_standard_add("conformant2", "Conformant 2");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_conformant_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_scroller_bounce_set(en, EINA_TRUE, EINA_FALSE);
   elm_object_text_set(en, "This is the top entry here");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   btn = elm_button_add(win);
   elm_object_focus_allow_set(btn, EINA_FALSE);
   elm_object_text_set(btn, "Delete Below");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   pg = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(pg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, pg);
   evas_object_show(pg);

   evas_object_smart_callback_add(btn, "clicked", popobj, pg);

   conform = elm_conformant_add(win);
   evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(conform, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_naviframe_item_simple_push(pg, conform);
   evas_object_show(conform);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_scroller_bounce_set(en, EINA_FALSE, EINA_TRUE);
   elm_object_text_set(en, "This entry and button below get deleted.");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   btn = elm_button_add(win);
   elm_object_focus_allow_set(btn, EINA_FALSE);
   elm_object_text_set(btn, "Delete this bottom bit 1");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   evas_object_smart_callback_add(btn, "clicked", popobj, pg);

   elm_object_content_set(conform, bx);
   evas_object_show(bx);

   conform = elm_conformant_add(win);
   evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(conform, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_naviframe_item_simple_push(pg, conform);
   evas_object_show(conform);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_scroller_bounce_set(en, EINA_FALSE, EINA_TRUE);
   elm_object_text_set(en, "This entry and button below get deleted.");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   btn = elm_button_add(win);
   elm_object_focus_allow_set(btn, EINA_FALSE);
   elm_object_text_set(btn, "Delete this bottom bit 2");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   evas_object_smart_callback_add(btn, "clicked", popobj, pg);

   elm_object_content_set(conform, bx);
   evas_object_show(bx);

   evas_object_resize(win, 240, 480);
   evas_object_show(win);
}
