#include <Elementary.h>

static void
my_entry_bt_1(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   elm_entry_entry_set(en, "");
}

static void
my_entry_bt_2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   const char *s = elm_entry_entry_get(en);
   printf("ENTRY:\n");
   if (s) printf("%s\n", s);
}

static void
my_entry_bt_3(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   const char *s = elm_entry_selection_get(en);
   printf("SELECTION:\n");
   if (s) printf("%s\n", s);
}

static void
my_entry_bt_4(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   elm_entry_entry_insert(en, "Insert some <b>BOLD</> text");
}

static void
anchor_test(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   elm_entry_entry_insert(en, "ANCHOR CLICKED");
}

void
test_entry(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *en;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   elm_entry_line_wrap_set(en, 0);
   elm_entry_entry_set(en,
		       "This is an entry widget in this window that<br>"
		       "uses markup <b>like this</> for styling and<br>"
		       "formatting <em>like this</>, as well as<br>"
		       "<a href=X><link>links in the text</></a>, so enter text<br>"
		       "in here to edit it. By the way, links are<br>"
		       "called <a href=anc-02>Anchors</a> so you will need<br>"
		       "to refer to them this way.");
   evas_object_size_hint_weight_set(en, 1.0, 1.0);
   evas_object_size_hint_align_set(en, -1.0, -1.0);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_1, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Print");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_2, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Selection");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_3, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Insert");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_4, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   elm_object_focus(en);
   evas_object_show(win);
}

void
test_entry_scrolled(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *en, *sc, *sp;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "entry-scrolled", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry Scrolled");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   sc = elm_scroller_add(win);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_single_line_set(en, 1);
   elm_entry_entry_set(en, "Disabled entry");
   evas_object_size_hint_weight_set(en, 1.0, 0.0);
   evas_object_size_hint_align_set(en, -1.0, 0.0);
   elm_object_disabled_set(en, 1);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   sc = elm_scroller_add(win);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_password_set(en, 1);
   elm_entry_entry_set(en, "Access denied, give up!");
   evas_object_size_hint_weight_set(en, 1.0, 0.0);
   evas_object_size_hint_align_set(en, -1.0, 0.0);
   elm_object_disabled_set(en, 1);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_context_menu_item_add(en, "Hello", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_entry_context_menu_item_add(en, "World", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_entry_entry_set(en,
		       "Multi-line disabled entry widget :)<br>"
		       "We can use markup <b>like this</> for styling and<br>"
		       "formatting <em>like this</>, as well as<br>"
		       "<a href=X><link>links in the text</></a>, but it won't be editable or clickable.");
   evas_object_size_hint_weight_set(en, 1.0, 1.0);
   evas_object_size_hint_align_set(en, -1.0, -1.0);
   elm_object_disabled_set(en, 1);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   sc = elm_scroller_add(win);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_single_line_set(en, 1);
   elm_entry_entry_set(en, "This is a single line");
   evas_object_size_hint_weight_set(en, 1.0, 0.0);
   evas_object_size_hint_align_set(en, -1.0, 0.0);
   elm_entry_select_all(en);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   sc = elm_scroller_add(win);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_password_set(en, 1);
   elm_entry_entry_set(en, "Password here");
   evas_object_size_hint_weight_set(en, 1.0, 0.0);
   evas_object_size_hint_align_set(en, -1.0, 0.0);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_entry_set(en,
		       "This is an entry widget in this window that<br>"
		       "uses markup <b>like this</> for styling and<br>"
		       "formatting <em>like this</>, as well as<br>"
		       "<a href=X><link>links in the text</></a>, so enter text<br>"
		       "in here to edit it. By the way, links are<br>"
		       "called <a href=anc-02>Anchors</a> so you will need<br>"
		       "to refer to them this way. At the end here is a really long line to test line wrapping to see if it works. But just in case this line is not long enough I will add more here to really test it out, as Elementary really needs some good testing to see if entry widgets work as advertised.");
   evas_object_smart_callback_add(en, "anchor,clicked", anchor_test, en);
   evas_object_size_hint_weight_set(en, 1.0, 1.0);
   evas_object_size_hint_align_set(en, -1.0, -1.0);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_1, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Print");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_2, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Selection");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_3, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Insert");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_4, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 300);

   elm_object_focus(win);
   evas_object_show(win);
}
