#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
my_entry_bt_1(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_entry_set(en, "");
}

static void
my_entry_bt_2(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   const char *s = elm_entry_entry_get(en);
   printf("ENTRY:\n");
   if (s) printf("%s\n", s);
   printf("ENTRY PLAIN UTF8:\n");
   if (s)
     {
        s = elm_entry_markup_to_utf8(s);
        if (s)
          {
             printf("%s\n", s);
             free((char *)s);
          }
     }
}

static void
my_entry_bt_3(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   const char *s = elm_entry_selection_get(en);
   printf("SELECTION:\n");
   if (s) printf("%s\n", s);
   printf("SELECTION PLAIN UTF8:\n");
   if (s)
     {
        s = elm_entry_markup_to_utf8(s);
        if (s)
          {
             printf("%s\n", s);
             free((char *)s);
          }
     }
}

static void
my_entry_bt_4(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_entry_insert(en, "Insert some <b>BOLD</> text");
}

static void
my_entry_bt_5(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_scrollable_set(en, !elm_entry_scrollable_get(en));
}

static void
my_entry_bt_6(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   static Elm_Wrap_Type wr = ELM_WRAP_NONE;

   wr++;
   if (wr == ELM_WRAP_LAST) wr = ELM_WRAP_NONE;
   printf("wr: %i\n", wr);
   elm_entry_line_wrap_set(en, wr);
}

void
test_entry(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *en;
   char buf[4096];

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   elm_entry_line_wrap_set(en, ELM_WRAP_NONE);
   snprintf(buf, sizeof(buf),
            "This is an entry widget in this window that<br>"
            "uses markup <b>like this</> for styling and<br>"
            "formatting <em>like this</>, as well as<br>"
            "<a href=X><link>links in the text</></a>, so enter text<br>"
            "in here to edit it. By the way, links are<br>"
            "called <a href=anc-02>Anchors</a> so you will need<br>"
            "to refer to them this way.<br>"
            "<br>"

            "Also you can stick in items with (relsize + ascent): "
            "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
            " (full) "
            "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
            " (to the left)<br>"

            "Also (size + ascent): "
            "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
            " (full) "
            "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
            " (before this)<br>"

            "And as well (absize + ascent): "
            "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
            " (full) "
            "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
            " or even paths to image files on disk too like: "
            "<item absize=96x128 vsize=full href=file://%s/images/sky_01.jpg></item>"
            " ... end."
            , PACKAGE_DATA_DIR
            );
   elm_entry_entry_set(en, buf);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clr");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_1, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Prnt");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_2, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Sel");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_3, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Ins");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_4, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Scrl");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_5, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Wr");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_6, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   elm_object_focus_set(en, EINA_TRUE);
   evas_object_show(win);
}

static void
my_scrolled_entry_bt_1(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_entry_set(en, "");
}

static void
my_scrolled_entry_bt_2(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   const char *s = elm_entry_entry_get(en);
   printf("ENTRY:\n");
   if (s) printf("%s\n", s);
   printf("ENTRY PLAIN UTF8:\n");
   if (s)
     {
        s = elm_entry_markup_to_utf8(s);
        if (s)
          {
             printf("%s\n", s);
             free((char *)s);
          }
     }
}

static void
my_scrolled_entry_bt_3(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   const char *s = elm_entry_selection_get(en);
   printf("SELECTION:\n");
   if (s) printf("%s\n", s);
   printf("SELECTION PLAIN UTF8:\n");
   if (s)
     {
        s = elm_entry_markup_to_utf8(s);
        if (s)
          {
             printf("%s\n", s);
             free((char *)s);
          }
     }
}

static void
my_scrolled_entry_bt_4(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_entry_insert(en, "Insert some <b>BOLD</> text");
}

static void
my_scrolled_entry_bt_5(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   const char *s = elm_entry_entry_get(en);
   printf("PASSWORD: '%s'\n", s ? s : "");
}

static void
scrolled_anchor_test(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_entry_insert(en, "ANCHOR CLICKED");
}

void
test_entry_scrolled(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *en, *en_p, *sp;
   static Elm_Entry_Filter_Accept_Set digits_filter_data, digits_filter_data2;
   static Elm_Entry_Filter_Limit_Size limit_filter_data, limit_filter_data2;

   win = elm_win_add(NULL, "entry-scrolled", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry Scrolled");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   /* disabled entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_entry_set(en, "Disabled entry");
   elm_entry_single_line_set(en, 1);
   elm_object_disabled_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   /* password entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_password_set(en, 1);
   elm_entry_single_line_set(en, 1);
   elm_entry_entry_set(en, "Access denied, give up!");
   elm_object_disabled_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   /* multi-line disable entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
   elm_object_disabled_set(en, 1);
   elm_entry_context_menu_item_add(en, "Hello", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_entry_context_menu_item_add(en, "World", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_entry_entry_set(en,
				"Multi-line disabled entry widget :)<br>"
				"We can use markup <b>like this</> for styling and<br>"
				"formatting <em>like this</>, as well as<br>"
				"<a href=X><link>links in the text</></a>,"
				"but it won't be editable or clickable.");
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   /* Single line selected entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_entry_set(en, "This is a single line");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, 1);
   elm_entry_select_all(en);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   /* Only digits entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_entry_set(en, "01234");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   digits_filter_data.accepted = "0123456789";
   digits_filter_data.rejected = NULL;
   elm_entry_text_filter_append(en, elm_entry_filter_accept_set, &digits_filter_data);

   /* No digits entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_entry_set(en, "No numbers here");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   digits_filter_data2.accepted = NULL;
   digits_filter_data2.rejected = "0123456789";
   elm_entry_text_filter_append(en, elm_entry_filter_accept_set, &digits_filter_data2);

   /* Size limited entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_entry_set(en, "Just 20 chars");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   limit_filter_data.max_char_count = 20;
   limit_filter_data.max_byte_count = 0;
   elm_entry_text_filter_append(en, elm_entry_filter_limit_size, &limit_filter_data);

   /* Byte size limited entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_entry_set(en, "And now only 30 bytes");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   limit_filter_data2.max_char_count = 0;
   limit_filter_data2.max_byte_count = 30;
   elm_entry_text_filter_append(en, elm_entry_filter_limit_size, &limit_filter_data2);

   /* Single line password entry */
   en_p = elm_entry_add(win);
   elm_entry_scrollable_set(en_p, EINA_TRUE);
   evas_object_size_hint_weight_set(en_p, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en_p, EVAS_HINT_FILL, 0.5);
   elm_entry_scrollbar_policy_set(en_p, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_entry_set(en_p, "Password here");
   elm_entry_single_line_set(en_p, 1);
   elm_entry_password_set(en_p, 1);
   evas_object_show(en_p);
   elm_box_pack_end(bx, en_p);

   /* scrolled entry with icon/end widgets*/
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, 1);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   bt = elm_icon_add(win);
   elm_icon_standard_set(bt, "home");
   evas_object_size_hint_min_set(bt, 48, 48);
   evas_object_color_set(bt, 255, 0, 0, 128);
   evas_object_show(bt);
   elm_entry_icon_set(en, bt);
   bt = elm_icon_add(win);
   elm_icon_standard_set(bt, "delete");
   evas_object_color_set(bt, 255, 0, 0, 128);
   evas_object_size_hint_min_set(bt, 48, 48);
   evas_object_show(bt);
   elm_entry_end_set(en, bt);
   elm_entry_entry_set(en, "scrolled entry with icon and end objects");
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   /* markup scrolled entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
   elm_entry_entry_set(en,
				"This is an entry widget in this window that<br>"
				"uses markup <b>like this</> for styling and<br>"
				"formatting <em>like this</>, as well as<br>"
				"<a href=X><link>links in the text</></a>, so enter text<br>"
				"in here to edit it. By them way, links are<br>"
				"called <a href=anc-02>Anchors</a> so you will need<br>"
				"to refer to them this way. At the end here is a really long "
				"line to test line wrapping to see if it works. But just in "
				"case this line is not long enough I will add more here to "
				"really test it out, as Elementary really needs some "
				"good testing to see if entry widgets work as advertised.");
   evas_object_smart_callback_add(en, "anchor,clicked", scrolled_anchor_test, en);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", my_scrolled_entry_bt_1, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Print");
   evas_object_smart_callback_add(bt, "clicked", my_scrolled_entry_bt_2, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Print pwd");
   evas_object_smart_callback_add(bt, "clicked", my_scrolled_entry_bt_5, en_p);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Selection");
   evas_object_smart_callback_add(bt, "clicked", my_scrolled_entry_bt_3, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert");
   evas_object_smart_callback_add(bt, "clicked", my_scrolled_entry_bt_4, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 300);

   elm_object_focus_set(win, EINA_TRUE);
   evas_object_show(win);
}

static void
my_ent_bt_clr(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_entry_set(en, "");
}

static void
my_ent_bt_pri(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   const char *s = elm_entry_entry_get(en);
   printf("ENTRY:\n");
   if (s) printf("%s\n", s);
   printf("ENTRY PLAIN UTF8:\n");
   if (s)
     {
        s = elm_entry_markup_to_utf8(s);
        if (s)
          {
             printf("%s\n", s);
             free((char *)s);
          }
     }
}

static void
my_ent_bt_sel(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   const char *s = elm_entry_selection_get(en);
   printf("SELECTION:\n");
   if (s) printf("%s\n", s);
   printf("SELECTION PLAIN UTF8:\n");
   if (s)
     {
        s = elm_entry_markup_to_utf8(s);
        if (s)
          {
             printf("%s\n", s);
             free((char *)s);
          }
     }
}

static void
my_ent_bt_all(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_select_all(en);
}

static void
my_ent_bt_non(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_select_none(en);
}

static void
my_ent_bt_ins(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_entry_insert(en, "Insert text");
}

static void
my_ent_bt_lef(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_prev(en);
}

static void
my_ent_bt_rig(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_next(en);
}

static void
my_ent_bt_up_(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_up(en);
}

static void
my_ent_bt_dow(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_down(en);
}

static void
my_ent_bt_beg(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_begin_set(en);
}

static void
my_ent_bt_end(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_end_set(en);
}

static void
my_ent_bt_lbe(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_line_begin_set(en);
}

static void
my_ent_bt_len(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_line_end_set(en);
}

static void
my_ent_bt_sbe(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_selection_begin(en);
}

static void
my_ent_bt_sen(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_cursor_selection_end(en);
}

static void
my_ent_bt_fmt(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   printf("IS FORMAT: %i\n",
          (int)elm_entry_cursor_is_format_get(en));
}

static void
my_ent_bt_vfm(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   printf("IS VISIBLE FORMAT %i\n",
          (int)elm_entry_cursor_is_visible_format_get(en));
}

static void
my_ent_bt_chr(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   printf("CHAR '%s'\n", elm_entry_cursor_content_get(en));
}

static void
my_ent_bt_cut(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_selection_cut(en);
}

static void
my_ent_bt_cop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_selection_copy(en);
}

static void
my_ent_bt_pas(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_selection_paste(en);
}

void
test_entry3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *en;

   win = elm_win_add(NULL, "entry3", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry 3");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   /* Single line selected entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_entry_set(en, "This is a single line");
   elm_entry_single_line_set(en, 1);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clr");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_clr, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pri");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_pri, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Sel");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sel, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "All");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_all, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Non");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_non, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Ins");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_ins, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Lef");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_lef, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rig");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_rig, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Up ");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_up_, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Dow");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_dow, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Beg");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_beg, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "End");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_end, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "LBe");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_lbe, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "LEn");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_len, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "SBe");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sbe, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "SEn");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sen, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fmt");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_fmt, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "VFm");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_vfm, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Chr");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_chr, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Cut");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_cut, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Cop");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_cop, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pas");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_pas, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   /* markup scrolled entry */
   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_entry_set(en,
				"This is an entry widget in this window that<br>"
				"uses markup <b>like this</> for styling and<br>"
				"formatting <em>like this</>, as well as<br>"
				"<a href=X><link>links in the text</></a>, so enter text<br>"
				"in here to edit it. By them way, links are<br>"
				"called <a href=anc-02>Anchors</a> so you will need<br>"
				"to refer to them this way. At the end here is a really long "
				"line to test line wrapping to see if it works. But just in "
				"case this line is not long enough I will add more here to "
				"really test it out, as Elementary really needs some "
				"good testing to see if entry widgets work as advertised."
                                );
   evas_object_smart_callback_add(en, "anchor,clicked", scrolled_anchor_test, en);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clr");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_clr, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pri");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_pri, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Sel");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sel, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "All");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_all, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Non");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_non, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Ins");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_ins, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Lef");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_lef, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rig");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_rig, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Up ");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_up_, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Dow");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_dow, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Beg");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_beg, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "End");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_end, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "LBe");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_lbe, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "LEn");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_len, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "SBe");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sbe, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "SEn");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sen, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fmt");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_fmt, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "VFm");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_vfm, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Chr");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_chr, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Cut");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_cut, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Cop");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_cop, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pas");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_pas, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 480);

   elm_object_focus_set(win, EINA_TRUE);
   evas_object_show(win);
}

void
test_entry4(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *ly, *en;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "entry4", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry 4");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_entry_set(en, "This is a single line");
   elm_entry_single_line_set(en, 1);
   elm_layout_content_set(ly, "element1", en);
   evas_object_show(en);

   en = elm_entry_add(win);
   elm_entry_line_wrap_set(en, ELM_WRAP_NONE);
   elm_entry_entry_set(en,
		       "This is an entry widget<br>"
		       "that uses markup<br>"
                       "<b>like this</> and has<br>"
		       "no scroller, so you can<br>"
		       "use it more flexibly.");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(ly, "element2", en);
   evas_object_show(en);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_entry_set(en,
				"This is an entry widget in this window that<br>"
				"uses markup <b>like this</> for styling and<br>"
				"formatting <em>like this</>, as well as<br>"
				"<a href=X><link>links in the text</></a>, so enter text<br>"
				"in here to edit it. By them way, links are<br>"
				"called <a href=anc-02>Anchors</a> so you will need<br>"
				"to refer to them this way. At the end here is a really long "
				"line to test line wrapping to see if it works. But just in "
				"case this line is not long enough I will add more here to "
				"really test it out, as Elementary really needs some "
				"good testing to see if entry widgets work as advertised."
                                );
   evas_object_smart_callback_add(en, "anchor,clicked", scrolled_anchor_test, en);
   elm_layout_content_set(ly, "element3", en);
   evas_object_show(en);

   evas_object_show(win);
}

void
test_entry5(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *en;

   win = elm_win_add(NULL, "entry5", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry 5");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_entry_set(en,
				"This is an entry widget in this window that "
				"uses markup like this for styling and "
				"formatting like this, as well as "
				"to refer to them this way. At the end here is a really long "
				"line to test line wrapping to see if it works. But just in "
				"case this line is not long enough I will add more here to "
				"really test it out, as Elementary really needs some "
				"good testing to see if entry widgets work as advertised."
                                );
   evas_object_smart_callback_add(en, "anchor,clicked", scrolled_anchor_test, en);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clr");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_clr, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pri");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_pri, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Sel");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sel, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "All");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_all, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Non");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_non, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Ins");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_ins, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Lef");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_lef, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rig");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_rig, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Up ");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_up_, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Dow");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_dow, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Beg");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_beg, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "End");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_end, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "LBe");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_lbe, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "LEn");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_len, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "SBe");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sbe, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "SEn");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_sen, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fmt");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_fmt, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "VFm");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_vfm, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Chr");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_chr, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Cut");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_cut, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Cop");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_cop, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pas");
   evas_object_smart_callback_add(bt, "clicked", my_ent_bt_pas, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_propagate_events_set(bt, 0);
   elm_object_focus_allow_set(bt, 0);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 480);

   elm_object_focus_set(win, EINA_TRUE);
   evas_object_show(win);
}

static void
_scrolled_entry_clear(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = data;
   elm_entry_entry_set(en, "");
}

void
test_entry_notepad(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *np;

   win = elm_win_add(NULL, "entry-notepad", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry Notepad");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   np = elm_entry_add(win);
   elm_entry_scrollable_set(np, EINA_TRUE);
   elm_entry_file_set(np, "note.txt", ELM_TEXT_FORMAT_PLAIN_UTF8);
   evas_object_size_hint_weight_set(np, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(np, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, np);
   evas_object_show(np);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", _scrolled_entry_clear, np);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 300);

   elm_object_focus_set(win, EINA_TRUE);
   evas_object_show(win);
}
#endif
