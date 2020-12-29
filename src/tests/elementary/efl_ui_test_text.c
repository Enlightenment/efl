#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_LAYOUT_CALC_PROTECTED
#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "Evas_Legacy.h"

static void
increment_int_changed(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   int *value = data;
   (*value)++;
}

EFL_START_TEST(text_cnp)
{
   Eo *txt;
   Eo *win = win_add();

   int i_copy = 0, i_paste = 0, i_cut = 0;

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                efl_event_callback_add(efl_added, EFL_UI_TEXTBOX_EVENT_SELECTION_COPY, increment_int_changed, &i_copy),
                efl_event_callback_add(efl_added, EFL_UI_TEXTBOX_EVENT_SELECTION_PASTE, increment_int_changed, &i_paste),
                efl_event_callback_add(efl_added, EFL_UI_TEXTBOX_EVENT_SELECTION_CUT, increment_int_changed, &i_cut)
                );

   efl_text_set(txt, "Hello");
   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_copy(txt);
   efl_text_interactive_all_unselect(txt);
   efl_text_cursor_object_text_insert(efl_text_interactive_main_cursor_get(txt), "X");
   efl_ui_textbox_selection_paste(txt);
   ecore_main_loop_iterate();
   ck_assert_int_eq(i_copy, 1);
   ck_assert_int_eq(i_paste, 1);
   ck_assert_str_eq(efl_text_get(txt), "HelloXHello");

   i_copy = i_cut = i_paste= 0;
   efl_text_set(txt, "Hello");
   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_copy(txt);
   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_cut(txt);
   efl_ui_textbox_selection_paste(txt);
   efl_ui_textbox_selection_paste(txt);
   ecore_main_loop_iterate();
   ck_assert_int_eq(i_copy, 1);
   ck_assert_int_eq(i_cut, 1);
   ck_assert_int_eq(i_paste, 2);
   //FIXME
   //fail_if(strcmp(efl_text_get(txt),"HelloHello"));
   efl_del(txt);
}
EFL_END_TEST

static void
_stop_event_soon(void *data EINA_UNUSED, const Efl_Event *ev)
{
   efl_event_callback_stop(ev->object);
}

EFL_START_TEST(text_all_select_all_unselect)
{
   Eo *txt, *txt2;
   Eo *win = win_add();

   int i_have_selection = 0, i_selection = 0;

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                efl_event_callback_add(efl_added, EFL_TEXT_INTERACTIVE_EVENT_HAVE_SELECTION_CHANGED,
                            increment_int_changed, &i_have_selection),
                efl_event_callback_add(efl_added, EFL_TEXT_INTERACTIVE_EVENT_SELECTION_CHANGED,
                            increment_int_changed, &i_selection)
                );
   efl_event_callback_priority_add(txt, EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _stop_event_soon, NULL);
   efl_text_set(txt, "Hello");
   efl_text_interactive_all_select(txt);
   Efl_Text_Cursor_Object *c1=NULL, *c2 =NULL;
   efl_text_interactive_selection_cursors_get(txt, &c1, &c2);
   ck_assert_ptr_ne(c1, NULL);
   ck_assert_ptr_ne(c2, NULL);
   fail_if(!efl_text_interactive_have_selection_get(txt));
   ck_assert_int_eq(efl_text_cursor_object_position_get(c1), 0);
   ck_assert_int_eq(efl_text_cursor_object_position_get(c2), 5);
   efl_text_interactive_all_unselect(txt);
   fail_if(efl_text_interactive_have_selection_get(txt));
   ecore_main_loop_iterate();
   ck_assert_int_eq(i_have_selection, 2);
   ck_assert_int_eq(i_selection, 1);

   /*Partial select, the select all*/
   Eo *sel1, *sel2;
   i_selection = 0;
   efl_text_interactive_selection_cursors_get(txt, &sel1, &sel2);
   efl_text_cursor_object_position_set(sel1, 1);
   efl_text_cursor_object_position_set(sel2, 2);
   ck_assert_int_eq(i_selection, 2);
   efl_text_interactive_all_select(txt);
   ck_assert_int_eq(i_selection, 3);
   ck_assert_int_eq(efl_text_cursor_object_position_get(sel1), 0);
   ck_assert_int_eq(efl_text_cursor_object_position_get(sel2), 5);

   Eo *cur1 = efl_ui_textbox_cursor_create(txt);
   Eo *cur2 = efl_ui_textbox_cursor_create(txt);
   efl_text_cursor_object_position_set(cur1, 1);
   efl_text_cursor_object_position_set(cur2, 2);
   efl_text_interactive_selection_cursors_set(txt, cur1, cur2);
   ck_assert_int_eq(i_selection, 4);
   efl_text_interactive_selection_cursors_get(txt, &sel1, &sel2);
   ck_assert_int_eq(efl_text_cursor_object_position_get(sel1),1);
   ck_assert_int_eq(efl_text_cursor_object_position_get(sel2),2);


   /*Select part then select all*/
   efl_text_interactive_all_unselect(txt);
   i_have_selection = 0, i_selection = 0;
   efl_text_cursor_object_position_set(cur1, 1);
   efl_text_cursor_object_position_set(cur2, 2);
   efl_text_interactive_selection_cursors_set(txt, cur1, cur2);
   ck_assert_int_eq(i_selection, 1);
   ck_assert_int_eq(i_have_selection, 1);
   efl_text_interactive_all_select(txt);
   ck_assert_int_eq(i_selection, 2);
   ck_assert_int_eq(i_have_selection, 1);
   efl_text_interactive_all_unselect(txt);
   ck_assert_int_eq(i_have_selection, 2);

   //cursor selection change on efl_markup_set
   txt2 = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_text_markup_set(txt2, "<ps>");

   efl_del(txt);
   efl_del(txt2);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_selection)
{
   Eo *txt;
   Eo *win = win_add();

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_event_callback_priority_add(txt, EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _stop_event_soon, NULL);
   efl_gfx_entity_size_set(txt, EINA_SIZE2D(300, 300));
   efl_text_set(txt, "Hello");
   get_me_to_those_events(txt);
   drag_object(txt, 2, 2, 295, 295, EINA_TRUE);
   ecore_main_loop_iterate();
   Efl_Text_Cursor_Object *c1=NULL, *c2 =NULL;
   efl_text_interactive_selection_cursors_get(txt, &c1, &c2);
   ck_assert_ptr_ne(c1, NULL);
   ck_assert_ptr_ne(c2, NULL);
   fail_if(!efl_text_interactive_have_selection_get(txt));
   ck_assert_int_eq(efl_text_cursor_object_position_get(c1), 0);
   ck_assert_int_eq(efl_text_cursor_object_position_get(c2), 5);
   efl_text_interactive_all_unselect(txt);
   fail_if(efl_text_interactive_have_selection_get(txt));
   efl_text_interactive_selection_allowed_set(txt, EINA_FALSE);
   drag_object(txt, 2, 2, 295, 295, EINA_TRUE);
   ecore_main_loop_iterate();
   fail_if(efl_text_interactive_have_selection_get(txt));
   efl_del(txt);
}
EFL_END_TEST

static void
user_changed(void *data, const Efl_Event *ev)
{
   Efl_Text_Change_Info *info = (Efl_Text_Change_Info *)ev->info;
   Efl_Text_Change_Info *stored = (Efl_Text_Change_Info *)data;
   *stored = *info;
}

EFL_START_TEST(text_user_change)
{
   Eo *txt;
   Eo *win = win_add();
   Efl_Text_Change_Info info = {0};
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                efl_event_callback_add(efl_added, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, user_changed, &info)
                );
   efl_event_callback_priority_add(txt, EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _stop_event_soon, NULL);
   efl_text_set(txt, "Hello");
   efl_gfx_entity_size_set(txt, EINA_SIZE2D(300, 300));
   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_cut(txt);
   ck_assert_int_eq(info.position, 0);
   ck_assert_int_eq(info.length, 5);
   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_scroll_mode)
{
   Eo *txt, *win, *cur;
   win = win_add();
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_event_callback_priority_add(txt, EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _stop_event_soon, NULL);
   cur = efl_text_interactive_main_cursor_get(txt);
   efl_text_set(txt, "Hello");
   /*scroll mode is false by default*/
   fail_if(efl_ui_textbox_scrollable_get(txt));
   efl_ui_textbox_scrollable_set(txt, !efl_ui_textbox_scrollable_get(txt));
   efl_text_cursor_object_text_insert(cur, "World");
   fail_if(!efl_ui_textbox_scrollable_get(txt));
   efl_ui_textbox_scrollable_set(txt, !efl_ui_textbox_scrollable_get(txt));
   efl_text_cursor_object_text_insert(cur, "!!!");

   ck_assert_str_eq(efl_text_get(txt),"HelloWorld!!!");
   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_change_event)
{
   Eo *txt;
   Eo *win = win_add();

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_event_callback_priority_add(txt, EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _stop_event_soon, NULL);
   efl_gfx_entity_size_set(txt, EINA_SIZE2D(300, 300));
   efl_text_set(txt, "Hello");
   int i_changed = 0;
   efl_event_callback_add(txt, EFL_UI_TEXTBOX_EVENT_CHANGED, increment_int_changed, &i_changed);
   efl_gfx_entity_visible_set(txt, EINA_TRUE);
   Evas *e = evas_object_evas_get(txt);
   efl_ui_focus_util_focus(txt);
   evas_event_feed_key_down(e, "s", "s", "s", "s", time(NULL), NULL);
   ecore_main_loop_iterate();
   ck_assert_str_eq(efl_text_get(txt),"Hellos");
   ck_assert_int_eq(i_changed,1);
   ecore_main_loop_iterate();

   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_keys_handler)
{
   Eo *txt;
   Eo *win = win_add();

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_event_callback_priority_add(txt, EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _stop_event_soon, NULL);
   efl_gfx_entity_size_set(txt, EINA_SIZE2D(300, 300));
   efl_text_set(txt, "Hello");

   efl_gfx_entity_visible_set(txt, EINA_TRUE);
   Evas *e = evas_object_evas_get(txt);
   efl_ui_focus_util_focus(txt);

   efl_text_cursor_object_move(efl_text_interactive_main_cursor_get(txt), EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
   evas_key_modifier_on(e, "Control");
   evas_event_feed_key_down(e, "BackSpace", "BackSpace", "\b", "\b", time(NULL), NULL);
   ecore_main_loop_iterate();
   ck_assert_str_eq(efl_text_get(txt),"");
   ck_assert_int_eq(efl_text_cursor_object_position_get(efl_text_interactive_main_cursor_get(txt)), 0);
   evas_event_feed_key_up(e, "BackSpace", "BackSpace", "\b", "\b", time(NULL), NULL);
   ecore_main_loop_iterate();

   efl_text_set(txt, "Hello");
   efl_text_cursor_object_position_set(efl_text_interactive_main_cursor_get(txt), 0);
   evas_key_modifier_on(e, "Control");
   evas_event_feed_key_down(e, "Delete", "Delete", "\177", "\177", time(NULL), NULL);
   ecore_main_loop_iterate();
   ck_assert_str_eq(efl_text_get(txt),"");
   ck_assert_int_eq(efl_text_cursor_object_position_get(efl_text_interactive_main_cursor_get(txt)), 0);
   evas_event_feed_key_up(e, "Delete", "Delete", "\177", "\177", time(NULL), NULL);
   ecore_main_loop_iterate();

   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_editable)
{
   Eo *txt, *win;
   win = win_add();
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_text_font_size_set(txt, 100);
   efl_text_font_family_set(txt, "Arial");
   efl_text_interactive_editable_set(txt, !efl_text_interactive_editable_get(txt));
   ck_assert_int_eq(efl_text_font_size_get(txt), 100);
   ck_assert_str_eq(efl_text_font_family_get(txt), "Arial");

   efl_ui_widget_disabled_set(txt, EINA_TRUE);
   ck_assert_int_eq(efl_text_font_size_get(txt), 100);
   ck_assert_str_eq(efl_text_font_family_get(txt), "Arial");

   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_on_startup)
{
   Eo *txt, *win;
   win = win_add();
   unsigned char r,g,b,a;
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
               efl_text_color_set(efl_added, 0, 255, 0, 255),
               efl_text_font_size_set(efl_added, 50),
               efl_text_font_family_set(efl_added, "Arial"));

   ck_assert_int_eq(efl_text_font_size_get(txt), 50);
   ck_assert_str_eq(efl_text_font_family_get(txt), "Arial");
   efl_text_color_get(txt, &r, &g, &b, &a);
   ck_assert_int_eq(r, 0);
   ck_assert_int_eq(g, 255);
   ck_assert_int_eq(b, 0);
   ck_assert_int_eq(a, 255);

   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_multiline_selection)
{
   Eo *txt, *win;
   Eo *cursor1, *cursor2;
   Eina_Rect rc1, rc2;
   win = win_add();
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_text_markup_set(txt, "p1<ps/>p2<ps/>p3");
   efl_text_multiline_set(txt, EINA_FALSE);
   ecore_main_loop_iterate();
   efl_text_interactive_all_select(txt);
   efl_text_interactive_selection_cursors_get(txt, &cursor1, &cursor2);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor1, EFL_TEXT_CURSOR_TYPE_BEFORE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor2, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_ne(rc1.x, rc2.x);

   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_singleline_cursor_movement)
{
   Eo *txt, *win;
   Eo *cursor;
   Eina_Rect rc1, rc2;
   win = win_add();
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_text_markup_set(txt, "p1<ps>p<b>2</b>2<ps>p3");
   efl_text_multiline_set(txt, EINA_FALSE);
   ecore_main_loop_iterate();

   cursor = efl_text_interactive_main_cursor_get(txt);
   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cursor), 0);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);

   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cursor), 9);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_ne(rc1.x, rc2.x);

   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cursor), 0);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);

   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cursor), 9);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_ne(rc1.x, rc2.x);

   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_NEXT);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cursor), 9); //do not move
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);

   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_PREVIOUS);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cursor), 9); //do not move
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_eq(rc1.x, rc2.x);

   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_START);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cursor), 0);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);

   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_END);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cursor), 9);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_ne(rc1.x, rc2.x);

   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_multiline_singleline_cursor_pos)
{
   Eo *txt, *win;
   Eo *cursor, *cursor1, *cursor2;
   Eina_Rect rc1, rc2;
   win = win_add();
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_text_markup_set(txt, "p1<ps>p<b>2</b>2<ps>p3<ps>");
   cursor = efl_text_interactive_main_cursor_get(txt);
   cursor1 = efl_ui_textbox_cursor_create(txt);
   efl_text_cursor_object_position_set(cursor1, 4);
   cursor2 = efl_ui_textbox_cursor_create(txt);
   efl_text_cursor_object_position_set(cursor2, 8);

   efl_text_multiline_set(txt, EINA_FALSE);
   ck_assert_uint_eq(efl_text_cursor_object_content_get(cursor1), '2');
   ck_assert_uint_eq(efl_text_cursor_object_content_get(cursor2), '3');

   efl_text_cursor_object_position_set(cursor, 0);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   efl_text_multiline_set(txt, EINA_TRUE);
   ck_assert_uint_eq(efl_text_cursor_object_content_get(cursor1), '2');
   ck_assert_uint_eq(efl_text_cursor_object_content_get(cursor2), '3');
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_eq(rc1.x, rc2.x);

   efl_text_multiline_set(txt, EINA_FALSE);
   efl_text_cursor_object_position_set(cursor, 2);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   efl_text_multiline_set(txt, EINA_TRUE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_eq(rc1.x, rc2.x);
   efl_text_multiline_set(txt, EINA_FALSE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_eq(rc1.x, rc2.x);

   efl_text_multiline_set(txt, EINA_FALSE);
   efl_text_cursor_object_position_set(cursor, 3);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   efl_text_multiline_set(txt, EINA_TRUE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_ne(rc1.y, rc2.y);
   ck_assert_int_ne(rc1.x, rc2.x);
   efl_text_multiline_set(txt, EINA_FALSE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_eq(rc1.x, rc2.x);

   efl_text_multiline_set(txt, EINA_FALSE);
   efl_text_cursor_object_position_set(cursor, 4);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   efl_text_multiline_set(txt, EINA_TRUE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_ne(rc1.y, rc2.y);
   ck_assert_int_ne(rc1.x, rc2.x);
   efl_text_multiline_set(txt, EINA_FALSE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_eq(rc1.x, rc2.x);

   efl_text_multiline_set(txt, EINA_FALSE);
   efl_text_cursor_object_position_set(cursor, 10);
   rc1 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   efl_text_multiline_set(txt, EINA_TRUE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_ne(rc1.y, rc2.y);
   ck_assert_int_ne(rc1.x, rc2.x);
   efl_text_multiline_set(txt, EINA_FALSE);
   rc2 = efl_text_cursor_object_cursor_geometry_get(cursor, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rc1.y, rc2.y);
   ck_assert_int_eq(rc1.x, rc2.x);


   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

/*
EFL_START_TEST(text_keyboard_mouse_cluster_cursor_movement)
{
   Eo *txt;
   Eo *cursor;
   Eo *win = win_add();
   Evas *e;
   Eina_Rect rc, rc2;

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 300));
   efl_gfx_entity_size_set(txt, EINA_SIZE2D(300, 300));
   efl_ui_textbox_scrollable_set(txt, EINA_TRUE);
   get_me_to_those_events(txt);

   efl_text_markup_set(txt, "A\u1100\u1161\u11AA");
   cursor = efl_text_interactive_main_cursor_get(txt);
   efl_text_cursor_object_position_set(cursor, 1);

   e = evas_object_evas_get(txt);
   efl_ui_focus_util_focus(txt);
   evas_event_feed_key_down(e, "Right", "Right", "Right", "Right", time(NULL), NULL);

   ck_assert_int_eq(4, efl_text_cursor_object_position_get(cursor));

   efl_text_cursor_object_position_set(cursor, 1);
   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT);
   efl_text_cursor_object_move(cursor, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT);
   rc = efl_text_cursor_object_content_geometry_get(cursor);
   rc2 = efl_ui_scrollable_viewport_geometry_get(txt);

   efl_text_cursor_object_position_set(cursor, 0);
   click_object_at(win, rc2.x + rc.x + (rc.w/2), rc2.y + rc.y + (rc.h/2));

   ck_assert_int_eq(4, efl_text_cursor_object_position_get(cursor));

   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST
*/

void efl_ui_test_text(TCase *tc)
{
   tcase_add_test(tc, text_cnp);
   tcase_add_test(tc, text_all_select_all_unselect);
   tcase_add_test(tc, text_selection);
   tcase_add_test(tc, text_user_change);
   tcase_add_test(tc, text_scroll_mode);
   tcase_add_test(tc, text_change_event);
   tcase_add_test(tc, text_keys_handler);
   tcase_add_test(tc, text_editable);
   tcase_add_test(tc, text_multiline_selection);
   tcase_add_test(tc, text_singleline_cursor_movement);
   tcase_add_test(tc, text_multiline_singleline_cursor_pos);
   tcase_add_test(tc, text_on_startup);
// works on some systemd and not others - is suepct font or harfbuzz system
// dependency issue... for now - disable.
//   tcase_add_test(tc, text_keyboard_mouse_cluster_cursor_movement);
}
