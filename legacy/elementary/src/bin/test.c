#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

/* all tests prototypes */
void test_bg_plain(void *data, Evas_Object *obj, void *event_info);
void test_bg_image(void *data, Evas_Object *obj, void *event_info);
void test_bg_options(void *data, Evas_Object *obj, void *event_info);
void test_icon(void *data, Evas_Object *obj, void *event_info);
void test_box_vert(void *data, Evas_Object *obj, void *event_info);
void test_box_vert2(void *data, Evas_Object *obj, void *event_info);
void test_box_horiz(void *data, Evas_Object *obj, void *event_info);
void test_box_transition(void *data, Evas_Object *obj, void *event_info);
void test_button(void *data, Evas_Object *obj, void *event_info);
void test_transit(void *data, Evas_Object *obj, void *event_info);
void test_transit2(void *data, Evas_Object *obj, void *event_info);
void test_transit3(void *data, Evas_Object *obj, void *event_info);
void test_transit4(void *data, Evas_Object *obj, void *event_info);
void test_transit5(void *data, Evas_Object *obj, void *event_info);
void test_transit6(void *data, Evas_Object *obj, void *event_info);
void test_transit7(void *data, Evas_Object *obj, void *event_info);
void test_transit8(void *data, Evas_Object *obj, void *event_info);
void test_transit9(void *data, Evas_Object *obj, void *event_info);
void test_fileselector_button(void *data, Evas_Object *obj, void *event_info);
void test_fileselector_entry(void *data, Evas_Object *obj, void *event_info);
void test_toggle(void *data, Evas_Object *obj, void *event_info);
void test_clock(void *data, Evas_Object *obj, void *event_info);
void test_clock2(void *data, Evas_Object *obj, void *event_info);
void test_check(void *data, Evas_Object *obj, void *event_info);
void test_radio(void *data, Evas_Object *obj, void *event_info);
void test_layout(void *data, Evas_Object *obj, void *event_info);
void test_hover(void *data, Evas_Object *obj, void *event_info);
void test_hover2(void *data, Evas_Object *obj, void *event_info);
void test_entry(void *data, Evas_Object *obj, void *event_info);
void test_entry_scrolled(void *data, Evas_Object *obj, void *event_info);
void test_entry3(void *data, Evas_Object *obj, void *event_info);
void test_entry4(void *data, Evas_Object *obj, void *event_info);
void test_entry5(void *data, Evas_Object *obj, void *event_info);
void test_entry_notepad(void *data, Evas_Object *obj, void *event_info);
void test_anchorview(void *data, Evas_Object *obj, void *event_info);
void test_anchorblock(void *data, Evas_Object *obj, void *event_info);
void test_toolbar(void *data, Evas_Object *obj, void *event_info);
void test_toolbar2(void *data, Evas_Object *obj, void *event_info);
void test_toolbar3(void *data, Evas_Object *obj, void *event_info);
void test_toolbar4(void *data, Evas_Object *obj, void *event_info);
void test_toolbar5(void *data, Evas_Object *obj, void *event_info);
void test_toolbar6(void *data, Evas_Object *obj, void *event_info);
void test_hoversel(void *data, Evas_Object *obj, void *event_info);
void test_list(void *data, Evas_Object *obj, void *event_info);
void test_list_horizontal(void *data, Evas_Object *obj, void *event_info);
void test_list2(void *data, Evas_Object *obj, void *event_info);
void test_list3(void *data, Evas_Object *obj, void *event_info);
void test_list4(void *data, Evas_Object *obj, void *event_info);
void test_list5(void *data, Evas_Object *obj, void *event_info);
void test_inwin(void *data, Evas_Object *obj, void *event_info);
void test_inwin2(void *data, Evas_Object *obj, void *event_info);
void test_scaling(void *data, Evas_Object *obj, void *event_info);
void test_scaling2(void *data, Evas_Object *obj, void *event_info);
void test_slider(void *data, Evas_Object *obj, void *event_info);
void test_actionslider(void *data, Evas_Object *obj, void *event_info);
void test_genlist(void *data, Evas_Object *obj, void *event_info);
void test_genlist2(void *data, Evas_Object *obj, void *event_info);
void test_genlist3(void *data, Evas_Object *obj, void *event_info);
void test_genlist4(void *data, Evas_Object *obj, void *event_info);
void test_genlist5(void *data, Evas_Object *obj, void *event_info);
void test_genlist6(void *data, Evas_Object *obj, void *event_info);
void test_genlist7(void *data, Evas_Object *obj, void *event_info);
void test_genlist8(void *data, Evas_Object *obj, void *event_info);
void test_genlist9(void *data, Evas_Object *obj, void *event_info);
void test_genlist10(void *data, Evas_Object *obj, void *event_info);
void test_genlist11(void *data, Evas_Object *obj, void *event_info);
void test_genlist12(void *data, Evas_Object *obj, void *event_info);
void test_gesture_layer(void *data, Evas_Object *obj, void *event_info);
void test_gesture_layer2(void *data, Evas_Object *obj, void *event_info);
void test_table(void *data, Evas_Object *obj, void *event_info);
void test_table2(void *data, Evas_Object *obj, void *event_info);
void test_table3(void *data, Evas_Object *obj, void *event_info);
void test_table4(void *data, Evas_Object *obj, void *event_info);
void test_table5(void *data, Evas_Object *obj, void *event_info);
void test_table6(void *data, Evas_Object *obj, void *event_info);
void test_gengrid(void *data, Evas_Object *obj, void *event_info);
void test_gengrid2(void *data, Evas_Object *obj, void *event_info);
void test_gengrid3(void *data, Evas_Object *obj, void *event_info);
void test_pager(void *data, Evas_Object *obj, void *event_info);
void test_pager_slide(void *data, Evas_Object *obj, void *event_info);
void test_win_state(void *data, Evas_Object *obj, void *event_info);
void test_win_state2(void *data, Evas_Object *obj, void *event_info);
void test_progressbar(void *data, Evas_Object *obj, void *event_info);
void test_fileselector(void *data, Evas_Object *obj, void *event_info);
void test_separator(void *data, Evas_Object *obj, void *event_info);
void test_scroller(void *data, Evas_Object *obj, void *event_info);
void test_scroller2(void *data, Evas_Object *obj, void *event_info);
void test_spinner(void *data, Evas_Object *obj, void *event_info);
void test_index(void *data, Evas_Object *obj, void *event_info);
void test_index2(void *data, Evas_Object *obj, void *event_info);
void test_photocam(void *data, Evas_Object *obj, void *event_info);
void test_photo(void *data, Evas_Object *obj, void *event_info);
void test_thumb(void *data, Evas_Object *obj, void *event_info);
void test_icon_desktops(void *data, Evas_Object *obj, void *event_info);
void test_icon_animated(void *data, Evas_Object *obj, void *event_info);
void test_notify(void *data, Evas_Object *obj, void *event_info);
void test_slideshow(void *data, Evas_Object *obj, void *event_info);
void test_menu(void *data, Evas_Object *obj, void *event_info);
void test_panel(void *data, Evas_Object *obj, void *event_info);
void test_panes(void *data, Evas_Object *obj, void *event_info);
void test_calendar(void *data, Evas_Object *obj, void *event_info);
void test_calendar2(void *data, Evas_Object *obj, void *event_info);
void test_map(void *data, Evas_Object *obj, void *event_info);
void test_weather(void *data, Evas_Object *obj, void *event_info);
void test_flip(void *data, Evas_Object *obj, void *event_info);
void test_flip2(void *data, Evas_Object *obj, void *event_info);
void test_flip3(void *data, Evas_Object *obj, void *event_info);
void test_flip4(void *data, Evas_Object *obj, void *event_info);
void test_flip_page(void *data, Evas_Object *obj, void *event_info);
void test_label(void *data, Evas_Object *obj, void *event_info);
void test_conformant(void *data, Evas_Object *obj, void *event_info);
void test_conformant2(void *data, Evas_Object *obj, void *event_info);
void test_multi(void *data, Evas_Object *obj, void *event_info);
void test_floating(void *data, Evas_Object *obj, void *event_info);
void test_launcher(void *data, Evas_Object *obj, void *event_info);
void test_launcher2(void *data, Evas_Object *obj, void *event_info);
void test_launcher3(void *data, Evas_Object *obj, void *event_info);
void test_anim(void *data, Evas_Object *obj, void *event_info);
void test_tooltip(void *data, Evas_Object *obj, void *event_info);
void test_tooltip2(void *data, Evas_Object *obj, void *event_info);
void test_cursor(void *data, Evas_Object *obj, void *event_info);
void test_cursor2(void *data, Evas_Object *obj, void *event_info);
void test_cursor3(void *data, Evas_Object *obj, void *event_info);
void test_cursor4(void *data, Evas_Object *obj, void *event_info);
void test_focus(void *data, Evas_Object *obj, void *event_info);
void test_focus2(void *data, Evas_Object *obj, void *event_info);
void test_focus3(void *data, Evas_Object *obj, void *event_info);
void test_flipselector(void *data, Evas_Object *obj, void *event_info);
void test_diskselector(void *data, Evas_Object *obj, void *event_info);
void test_colorselector(void *data, Evas_Object *obj, void *event_info);
void test_ctxpopup(void *data, Evas_Object *obj, void *event_info);
void test_bubble(void *data, Evas_Object *obj, void *event_info);
void test_segment_control(void *data, Evas_Object *obj, void *event_info);
void test_store(void *data, Evas_Object *obj, void *event_info);
void test_win_inline(void *data, Evas_Object *obj, void *event_info);
void test_grid(void *data, Evas_Object *obj, void *event_info);
void test_glview_simple(void *data, Evas_Object *obj, void *event_info);
void test_glview(void *data, Evas_Object *obj, void *event_info);
void test_3d(void *data, Evas_Object *obj, void *event_info);
void test_naviframe(void *data, Evas_Object *obj, void *event_info);
void test_factory(void *data, Evas_Object *obj, void *event_info);
#ifdef HAVE_EIO
void test_eio(void *data, Evas_Object *obj, void *event_info);
#endif
#ifdef HAVE_ELEMENTARY_WEB
void test_web(void *data, Evas_Object *obj, void *event_info);
#endif

struct elm_test
{
   const char *icon;
   const char *category;
   const char *name;
   void (*cb)(void *, Evas_Object *, void *);
};

static int
elm_test_sort(const void *pa, const void *pb)
{
   const struct elm_test *a = pa, *b = pb;
   int res = strcasecmp(a->category, b->category);
   if (res) return res;
   return strcasecmp(a->name, b->name);
}

static void
elm_test_add(Eina_List **p_list, const char *icon, const char *category, const char *name, void (*cb)(void *, Evas_Object *, void *))
{
   struct elm_test *t = malloc(sizeof(struct elm_test));
   t->icon = icon;
   t->category = category;
   t->name = name;
   t->cb = cb;
   *p_list = eina_list_sorted_insert(*p_list, elm_test_sort, t);
}

static void
my_win_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   /* called when my_win_main is requested to be deleted */
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

static void
_ui_tg_changed(void *data, Evas_Object *obj, void *event_info)
{
   (void) data; (void) event_info;
   elm_mirrored_set(elm_check_state_get(obj));
}


static void
my_win_main(char *autorun, Eina_Bool test_win_only)
{
   Evas_Object *win = NULL, *bg = NULL, *bx0 = NULL, *lb = NULL;
   Evas_Object *fr = NULL, *tg = NULL, *sc = NULL, *ic = NULL;
   Evas_Object *tbx = NULL, *cfr = NULL, *tbx2 = NULL, *bt = NULL;
   Eina_List *tests, *l;
   struct elm_test *t;

   if (test_win_only) goto add_tests;
   /* Create an elm window - It returns an evas object. This is a little
    * special as the object lives in the canvas that is inside the window
    * so what is returned is really inside the window, but as you manipulate
    * the evas object returned - the window will respond. elm_win makes sure
    * of that so you can blindly treat it like any other evas object
    * pretty much, just as long as you know it has special significance.
    * The first parameter is a "parent" window - eg for a dialog you want to
    * have a main window it is related to, here it is NULL meaning there
    * is no parent. "main" is the name of the window - used by the window
    * manager for identifying the window uniquely amongst all the windows
    * within this application (and all instances of the application). The
    * type is a basic window (the final parameter) */
   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   /* Set the title of the window - This is in the titlebar. */
   elm_win_title_set(win, "Elementary Tests");

   /* Set a callback on the window when "delete,request" is emitted as
    * a callback. When this happens my_win_del() is called and the
    * data pointer (first param) is passed the final param here (in this
    * case it is NULL). This is how you can pass specific things to a
    * callback like objects or data layered on top. */
   evas_object_smart_callback_add(win, "delete,request", my_win_del, NULL);

   /* Add a background to our window. This just uses the standard theme set
    * background. Without a background, you could make a window seem
    * transparent with elm_win_alpha_set(win, 1); For example. if you have
    * a compositor running this will make the window able to be
    * semi-transparent and any space not filled by object/widget pixels will
    * be transparent or translucent based on alpha. If you do not have a
    * compositor running this should fall back to using shaped windows
    * (which have a mask). Both these features will be slow and rely on
    * a lot more resources, so only use it if you need it. */
   bg = elm_bg_add(win);
   /* Set weight to 1.0 x 1.0 == expand in both x and y direction. */
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   /* Tell the window that this object is to be resized along with the window.
    * Also as a result this object will be one of several objects that
    * controls the minimum/maximum size of the window. */
   elm_win_resize_object_add(win, bg);
   /* And show the background. */
   evas_object_show(bg);

   /* Add a box layout widget to the window. */
   bx0 = elm_box_add(win);
   /* Allow base box (bx0) to expand in x and y. */
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   /* Tell the window that the box affects window size and also will be
    * resized when the window is resized. */
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Information");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "Please select a test from the list below<br>"
                       "by clicking the test button to show the<br>"
                       "test window.");
   elm_object_content_set(fr, lb);
   evas_object_show(lb);

   tg = elm_check_add(win);
   elm_object_style_set(tg, "toggle");
   elm_object_text_set(tg, "UI-Mirroring:");
   elm_check_state_set(tg, elm_mirrored_get());
   evas_object_smart_callback_add(tg, "changed", _ui_tg_changed, NULL);
   elm_box_pack_end(bx0, tg);
   evas_object_show(tg);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx0, sc);
   evas_object_show(sc);

   tbx = elm_box_add(win);
   evas_object_size_hint_weight_set(tbx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tbx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(sc, tbx);
   evas_object_show(tbx);
   
add_tests:
   tests = NULL;
#define ADD_TEST(icon_, cat_, name_, cb_) elm_test_add(&tests, icon_, cat_, name_, cb_)
   
   //------------------------------//
   ADD_TEST(NULL, "Window / Background", "Bg Plain", test_bg_plain);
   ADD_TEST(NULL, "Window / Background", "Bg Image", test_bg_image);
   ADD_TEST(NULL, "Window / Background", "Bg Options", test_bg_options);
   ADD_TEST(NULL, "Window / Background", "Window States", test_win_state);
   ADD_TEST(NULL, "Window / Background", "Window States 2", test_win_state2);
   ADD_TEST(NULL, "Window / Background", "Inwin", test_inwin);
   ADD_TEST(NULL, "Window / Background", "Inwin 2", test_inwin2);
   ADD_TEST(NULL, "Window / Background", "Window Inline", test_win_inline);
   
   //------------------------------//
   ADD_TEST(NULL, "Images", "Icon Transparent", test_icon);
   ADD_TEST(NULL, "Images", "Icon Animation", test_icon_animated);
   ADD_TEST(NULL, "Images", "Photocam", test_photocam);
   ADD_TEST(NULL, "Images", "Photo", test_photo);
   ADD_TEST(NULL, "Images", "Thumb", test_thumb);
   ADD_TEST(NULL, "Images", "Slideshow", test_slideshow);
   
   //------------------------------//
   ADD_TEST(NULL, "Containers", "Box Vert", test_box_vert);
   ADD_TEST(NULL, "Containers", "Box Vert 2", test_box_vert2);
   ADD_TEST(NULL, "Containers", "Box Horiz", test_box_horiz);
   ADD_TEST(NULL, "Containers", "Box Transition", test_box_transition);
   ADD_TEST(NULL, "Containers", "Table", test_table);
   ADD_TEST(NULL, "Containers", "Table Homogeneous", test_table2);
   ADD_TEST(NULL, "Containers", "Table 3", test_table3);
   ADD_TEST(NULL, "Containers", "Table 4", test_table4);
   ADD_TEST(NULL, "Containers", "Table 5", test_table5);
   ADD_TEST(NULL, "Containers", "Table 6", test_table6);
   ADD_TEST(NULL, "Containers", "Layout", test_layout);
   ADD_TEST(NULL, "Containers", "Grid", test_grid);
   
   //------------------------------//
   ADD_TEST(NULL, "Entries", "Entry", test_entry);
   ADD_TEST(NULL, "Entries", "Entry Scrolled", test_entry_scrolled);
   ADD_TEST(NULL, "Entries", "Entry 3", test_entry3);
   ADD_TEST(NULL, "Entries", "Entry 4", test_entry4);
   ADD_TEST(NULL, "Entries", "Entry 5", test_entry5);
   ADD_TEST(NULL, "Entries", "Entry Notepad", test_entry_notepad);
   
   //------------------------------//
   ADD_TEST(NULL, "Buttons", "Buttons", test_button);
   
   //------------------------------//
   ADD_TEST(NULL, "Effects", "Transit", test_transit);
   ADD_TEST(NULL, "Effects", "Transit 2", test_transit2);
   ADD_TEST(NULL, "Effects", "Transit 3", test_transit3);
   ADD_TEST(NULL, "Effects", "Transit 4", test_transit4);
   ADD_TEST(NULL, "Effects", "Transit 5", test_transit5);
   ADD_TEST(NULL, "Effects", "Transit 6", test_transit6);
   ADD_TEST(NULL, "Effects", "Transit 7", test_transit7);
   ADD_TEST(NULL, "Effects", "Transit 8", test_transit8);
   ADD_TEST(NULL, "Effects", "Transit 9", test_transit9);
   ADD_TEST(NULL, "Effects", "Flip", test_flip);
   ADD_TEST(NULL, "Effects", "Flip 2", test_flip2);
   ADD_TEST(NULL, "Effects", "Flip 3", test_flip3);
   ADD_TEST(NULL, "Effects", "Flip Interactive", test_flip4);
   ADD_TEST(NULL, "Effects", "Flip Page", test_flip_page);
   ADD_TEST(NULL, "Effects", "Animation", test_anim);
   
   //------------------------------//
   ADD_TEST(NULL, "Toolbars", "Toolbar", test_toolbar);
   ADD_TEST(NULL, "Toolbars", "Toolbar 2", test_toolbar2);
   ADD_TEST(NULL, "Toolbars", "Toolbar 3", test_toolbar3);
   ADD_TEST(NULL, "Toolbars", "Toolbar 4", test_toolbar4);
   ADD_TEST(NULL, "Toolbars", "Toolbar 5", test_toolbar5);
   ADD_TEST(NULL, "Toolbars", "Toolbar 6", test_toolbar6);
   
   //------------------------------//
   ADD_TEST(NULL, "Lists", "List", test_list);
   ADD_TEST(NULL, "Lists", "List - Horizontal", test_list_horizontal);
   ADD_TEST(NULL, "Lists", "List 2", test_list2);
   ADD_TEST(NULL, "Lists", "List 3", test_list3);
   ADD_TEST(NULL, "Lists", "List 4", test_list4);
   ADD_TEST(NULL, "Lists", "List 5", test_list5);
   ADD_TEST(NULL, "Lists", "Genlist", test_genlist);
   ADD_TEST(NULL, "Lists", "Genlist 2", test_genlist2);
   ADD_TEST(NULL, "Lists", "Genlist 3", test_genlist3);
   ADD_TEST(NULL, "Lists", "Genlist 4", test_genlist4);
   ADD_TEST(NULL, "Lists", "Genlist 5", test_genlist5);
   ADD_TEST(NULL, "Lists", "Genlist 7", test_genlist7);
   ADD_TEST(NULL, "Lists", "Genlist Tree", test_genlist6);
   ADD_TEST(NULL, "Lists", "Genlist Group", test_genlist8);
   ADD_TEST(NULL, "Lists", "Genlist Group Tree", test_genlist9);
   ADD_TEST(NULL, "Lists", "Genlist Mode", test_genlist10);
   ADD_TEST(NULL, "Lists", "Genlist Reorder Mode", test_genlist11);
#ifdef HAVE_EIO
   ADD_TEST(NULL, "Lists", "Genlist Eio", test_eio);
#endif
   ADD_TEST(NULL, "Lists", "Genlist Textblock", test_genlist12);
   ADD_TEST(NULL, "Lists", "GenGrid", test_gengrid);
   ADD_TEST(NULL, "Lists", "GenGrid 2", test_gengrid2);
   ADD_TEST(NULL, "Lists", "GenGrid Group", test_gengrid3);
   
   //------------------------------//
   ADD_TEST(NULL, "General", "Scaling", test_scaling);
   ADD_TEST(NULL, "General", "Scaling 2", test_scaling2);
   
   //------------------------------//
   ADD_TEST(NULL, "3D", "GLViewSimple", test_glview_simple);
   ADD_TEST(NULL, "3D", "GLView", test_glview);
   ADD_TEST(NULL, "3D", "Evas Map 3D", test_3d);
   
   //------------------------------//
#ifdef HAVE_ELEMENTARY_WEB
   ADD_TEST(NULL, "Web", "Web", test_web);
#endif
   
   //------------------------------//
   ADD_TEST(NULL, "Input", "Gesture Layer", test_gesture_layer);
   ADD_TEST(NULL, "Input", "Gesture Layer 2", test_gesture_layer2);
   ADD_TEST(NULL, "Input", "Multi Touch", test_multi);

   //------------------------------//
   ADD_TEST(NULL, "Selectors", "Index", test_index);
   ADD_TEST(NULL, "Selectors", "Index 2", test_index2);
   ADD_TEST(NULL, "Selectors", "File Selector", test_fileselector);
   ADD_TEST(NULL, "Selectors", "File Selector Entry", test_fileselector_entry);
   ADD_TEST(NULL, "Selectors", "File Selector Button", test_fileselector_button);
   ADD_TEST(NULL, "Selectors", "Actionslider", test_actionslider);
   ADD_TEST(NULL, "Selectors", "Menu", test_menu);
   ADD_TEST(NULL, "Selectors", "Disk Selector", test_diskselector);
   ADD_TEST(NULL, "Selectors", "Color Selector", test_colorselector);
   ADD_TEST(NULL, "Selectors", "Segment Control", test_segment_control);
   ADD_TEST(NULL, "Selectors", "Hoversel", test_hoversel);
   ADD_TEST(NULL, "Selectors", "Radios", test_radio);
   ADD_TEST(NULL, "Selectors", "Flip Selector", test_flipselector);

   //------------------------------//
   ADD_TEST(NULL, "Cursors", "Cursor", test_cursor);
   ADD_TEST(NULL, "Cursors", "Cursor 2", test_cursor2);
   ADD_TEST(NULL, "Cursors", "Cursor 3", test_cursor3);
   ADD_TEST(NULL, "Cursors", "Cursor Layout", test_cursor4);
   
   //------------------------------//
   ADD_TEST(NULL, "Scroller", "Scroller", test_scroller);
   ADD_TEST(NULL, "Scroller", "Scroller 2", test_scroller2);

   //------------------------------//
   // FIXME: add frame test
   ADD_TEST(NULL, "Boundaries", "Bubble", test_bubble);
   ADD_TEST(NULL, "Boundaries", "Separator", test_separator);

   //------------------------------//
   ADD_TEST(NULL, "Range Values", "Spinner", test_spinner);
   ADD_TEST(NULL, "Range Values", "Slider", test_slider);
   ADD_TEST(NULL, "Range Values", "Progressbar", test_progressbar);

   //------------------------------//
   ADD_TEST(NULL, "Booleans", "Checks", test_check);
   ADD_TEST(NULL, "Booleans", "Toggles", test_toggle);

   //------------------------------//
   ADD_TEST(NULL, "Popups", "Ctxpopup", test_ctxpopup);
   ADD_TEST(NULL, "Popups", "Hover", test_hover);
   ADD_TEST(NULL, "Popups", "Hover 2", test_hover2);
   ADD_TEST(NULL, "Popups", "Notify", test_notify);
   ADD_TEST(NULL, "Popups", "Tooltip", test_tooltip);
   ADD_TEST(NULL, "Popups", "Tooltip2", test_tooltip2);

   //------------------------------//
   ADD_TEST(NULL, "Times & Dates", "Calendar", test_calendar);
   ADD_TEST(NULL, "Times & Dates", "Calendar 2", test_calendar2);
   ADD_TEST(NULL, "Times & Dates", "Clock", test_clock);
   ADD_TEST(NULL, "Times & Dates", "Clock 2", test_clock2);
   
   //------------------------------//
   ADD_TEST(NULL, "Text", "Anchorview", test_anchorview);
   ADD_TEST(NULL, "Text", "Anchorblock", test_anchorblock);
   ADD_TEST(NULL, "Text", "Label", test_label);
   
   //------------------------------//
   ADD_TEST(NULL, "Stored Surface Buffer", "Launcher", test_launcher);
   ADD_TEST(NULL, "Stored Surface Buffer", "Launcher 2", test_launcher2);
   ADD_TEST(NULL, "Stored Surface Buffer", "Launcher 3", test_launcher3);
   
   //------------------------------//
   ADD_TEST(NULL, "Focus", "Focus", test_focus);
   ADD_TEST(NULL, "Focus", "Focus 2", test_focus2);
   ADD_TEST(NULL, "Focus", "Focus 3", test_focus3);

   //------------------------------//
   ADD_TEST(NULL, "Storyboard", "Naviframe", test_naviframe);
   ADD_TEST(NULL, "Storyboard", "Pager", test_pager);
   ADD_TEST(NULL, "Storyboard", "Pager Slide", test_pager_slide);

   //------------------------------//
   ADD_TEST(NULL, "Geographic", "Map", test_map);

   //------------------------------//
   ADD_TEST(NULL, "Dividers", "Panel", test_panel);
   ADD_TEST(NULL, "Dividers", "Panes", test_panes);
   
   //------------------------------//
   ADD_TEST(NULL, "Standardization", "Conformant", test_conformant);
   ADD_TEST(NULL, "Standardization", "Conformant 2", test_conformant2);
   
   //------------------------------//
   ADD_TEST(NULL, "Helpers", "Store", test_store);
   ADD_TEST(NULL, "Helpers", "Factory", test_factory);
   
   //------------------------------//
   ADD_TEST(NULL, "Micellaneous", "Weather", test_weather);
   ADD_TEST(NULL, "Micellaneous", "Icon Desktops", test_icon_desktops);
   ADD_TEST(NULL, "Micellaneous", "Floating Objects", test_floating);

#undef ADD_TEST

   if (autorun)
     {
        EINA_LIST_FOREACH(tests, l, t)
          if ((t->name) && (t->cb) && (!strcmp(t->name, autorun)))
            t->cb(NULL, NULL, NULL);

     }

   if (test_win_only) return;

   if (tests)
     {
        const char *pcat = NULL;
        
        EINA_LIST_FREE(tests, t)
          {
             if ((!pcat) || (strcmp(pcat, t->category)))
               {
                  cfr = elm_frame_add(win);
                  // FIXME: add new style of frame for this
                  elm_object_text_set(cfr, t->category);
                  evas_object_size_hint_weight_set(cfr, EVAS_HINT_EXPAND, 0.0);
                  evas_object_size_hint_fill_set(cfr, EVAS_HINT_FILL, 0.0);
                  elm_box_pack_end(tbx, cfr);
                  evas_object_show(cfr);
                  
                  tbx2 = elm_box_add(win);
                  elm_box_layout_set(tbx2, evas_object_box_layout_flow_horizontal, NULL, NULL);
                  evas_object_size_hint_weight_set(tbx2, EVAS_HINT_EXPAND, 0.0);
                  evas_object_size_hint_align_set(tbx2, EVAS_HINT_FILL, 0.0);
                  elm_box_align_set(tbx2, 0.0, 0.5);
                  elm_object_content_set(cfr, tbx2);
                  evas_object_show(tbx2);
               }
             bt = elm_button_add(win);
             // FIXME: add new style of button for this like efm in e17
             elm_object_text_set(bt, t->name);
             if (t->icon)
               {
                  char buf[PATH_MAX];
                  
                  ic = elm_icon_add(win);
                  snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), t->icon);
                  elm_icon_file_set(ic, buf, NULL);
                  elm_object_content_set(bt, ic);
                  evas_object_show(ic);
               }
             elm_box_pack_end(tbx2, bt);
             evas_object_show(bt);
             evas_object_smart_callback_add(bt, "clicked", t->cb, NULL);
             pcat = t->category;
             free(t);
          }
     }

   /* set an initial window size */
   evas_object_resize(win, 480, 480);
   /* show the window */
   if (!test_win_only)
     evas_object_show(win);
}

/* this is your elementary main function - it MUST be called IMMEDIATELY
 * after elm_init() and MUST be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */
EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Eina_Bool test_win_only = EINA_FALSE;
   char *autorun = NULL;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   /* tell elm about our app so it can figure out where to get files */
   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
   elm_app_info_set(elm_main, "elementary", "images/logo.png");

   /* if called with a single argument try to autorun a test with
    * the same name as the given param
    * ex:  elementary_test "Box Vert 2" */
   if (argc == 2)
     autorun = argv[1];
   else if (argc == 3)
     {
        /* Just a workaround to make the shot module more
         * useful with elementary test. */
        if ((!strcmp(argv[1], "--test-win-only")) ||
            (!strcmp(argv[1], "-to")))
          {
             test_win_only = EINA_TRUE;
             autorun = argv[2];
          }
     }

   /* put here any init specific to this app like parsing args etc. */
   my_win_main(autorun, test_win_only); /* create main window */
   elm_run(); /* and run the program now  and handle all events etc. */
   /* if the mainloop that elm_run() runs exist - we exit the app */
   elm_shutdown(); /* clean up and shut down */
   /* exit code */
   return 0;
}
#endif
/* all elementary apps should use this. but it should be placed right after
 * elm_main() */
ELM_MAIN()
