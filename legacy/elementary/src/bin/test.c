#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

/* all tests prototypes */
void test_bg_plain(void *data, Evas_Object *obj, void *event_info);
void test_bg_image(void *data, Evas_Object *obj, void *event_info);
void test_icon(void *data, Evas_Object *obj, void *event_info);
void test_box_vert(void *data, Evas_Object *obj, void *event_info);
void test_box_vert2(void *data, Evas_Object *obj, void *event_info);
void test_box_horiz(void *data, Evas_Object *obj, void *event_info);
void test_button(void *data, Evas_Object *obj, void *event_info);
void test_toggle(void *data, Evas_Object *obj, void *event_info);
void test_clock(void *data, Evas_Object *obj, void *event_info);
void test_check(void *data, Evas_Object *obj, void *event_info);
void test_radio(void *data, Evas_Object *obj, void *event_info);
void test_layout(void *data, Evas_Object *obj, void *event_info);
void test_hover(void *data, Evas_Object *obj, void *event_info);
void test_hover2(void *data, Evas_Object *obj, void *event_info);
void test_entry(void *data, Evas_Object *obj, void *event_info);
void test_entry_scrolled(void *data, Evas_Object *obj, void *event_info);
void test_notepad(void *data, Evas_Object *obj, void *event_info);
void test_anchorview(void *data, Evas_Object *obj, void *event_info);
void test_anchorblock(void *data, Evas_Object *obj, void *event_info);
void test_toolbar(void *data, Evas_Object *obj, void *event_info);
void test_hoversel(void *data, Evas_Object *obj, void *event_info);
void test_list(void *data, Evas_Object *obj, void *event_info);
void test_list2(void *data, Evas_Object *obj, void *event_info);
void test_list3(void *data, Evas_Object *obj, void *event_info);
void test_carousel(void *data, Evas_Object *obj, void *event_info);
void test_inwin(void *data, Evas_Object *obj, void *event_info);
void test_inwin2(void *data, Evas_Object *obj, void *event_info);
void test_scaling(void *data, Evas_Object *obj, void *event_info);
void test_scaling2(void *data, Evas_Object *obj, void *event_info);
void test_slider(void *data, Evas_Object *obj, void *event_info);
void test_genlist(void *data, Evas_Object *obj, void *event_info);
void test_genlist2(void *data, Evas_Object *obj, void *event_info);
void test_genlist3(void *data, Evas_Object *obj, void *event_info);
void test_genlist4(void *data, Evas_Object *obj, void *event_info);
void test_genlist5(void *data, Evas_Object *obj, void *event_info);
void test_genlist6(void *data, Evas_Object *obj, void *event_info);
void test_table(void *data, Evas_Object *obj, void *event_info);
void test_pager(void *data, Evas_Object *obj, void *event_info);
void test_win_state(void *data, Evas_Object *obj, void *event_info);
void test_progressbar(void *data, Evas_Object *obj, void *event_info);
void test_fileselector(void *data, Evas_Object *obj, void *event_info);
void test_separator(void *data, Evas_Object *obj, void *event_info);
void test_scroller(void *data, Evas_Object *obj, void *event_info);
void test_spinner(void *data, Evas_Object *obj, void *event_info);
void test_index(void *data, Evas_Object *obj, void *event_info);
void test_photocam(void *data, Evas_Object *obj, void *event_info);
void test_photo(void *data, Evas_Object *obj, void *event_info);
void test_icon_desktops(void *data, Evas_Object *obj, void *event_info);
void test_notify(void *data, Evas_Object *obj, void *event_info);
void test_slideshow(void *data, Evas_Object *obj, void *event_info);
void test_menu(void *data, Evas_Object *obj, void *event_info);
void test_panel(void *data, Evas_Object *obj, void *event_info);
void test_map(void *data, Evas_Object *obj, void *event_info);
void test_weather(void *data, Evas_Object *obj, void *event_info);
void test_flip(void *data, Evas_Object *obj, void *event_info);
void test_label(void *data, Evas_Object *obj, void *event_info);
void test_conformant(void *data, Evas_Object *obj, void *event_info);
void test_multi(void *data, Evas_Object *obj, void *event_info);
void test_floating(void *data, Evas_Object *obj, void *event_info);

static void
my_win_del(void *data, Evas_Object *obj, void *event_info)
{
   /* called when my_win_main is requested to be deleted */
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

static void
my_win_main(void)
{
   Evas_Object *win, *bg, *bx0, *lb, *li, *fr;

   /* 1 create an elm window - it returns an evas object. this is a little
    * special as the object lives in the canvas that is inside the window
    * so what is returned is really inside the window, but as you manipulate
    * the evas object returned - the window will respond. elm_win makes sure
    * of that so you can blindly treat it like any other evas object
    * pretty much, just as long as you know it has special significance */
   /* the first parameter is a "parent" window - eg for a dialog you want to
    * have a main window it is related to, here it is NULL meaning there
    * is no parent. "main" is the name of the window - used by the window
    * manager for identifying the window uniquely amongst all the windows
    * within this application (and all instances of the application). the
    * type is a basic window (the final parameter) */
   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   /* set the title of the window - this is in the titlebar */
   elm_win_title_set(win, "Elementary Tests");

   /* set a callback on the window when "delete,request" is emitted as
    * a callback. when this happens my_win_del() is called and the
    * data pointer (first param) is passed the final param here (in this
    * case it is NULL). This is how you can pass specific things to a
    * callback like objects or data layered on top */
   evas_object_smart_callback_add(win, "delete,request", my_win_del, NULL);

   /* add a background to our window. this just uses the standard theme set
    * background. without a backgorund, you could make a window seem
    * transparent with elm_win_alpha_set(win, 1); for example. if you have
    * a compositor running this will make the window able to be
    * semi-transparent and any space not filled by object/widget pixels will
    * be transparent or translucent based on alpha. if you do not have a
    * comnpositor running this should fall back to using shaped windows
    * (which have a mask). both these features will be slow and rely on
    * a lot more resources, so only use it if you need it. */
   bg = elm_bg_add(win);
   /* set weight to 1.0 x 1.0 == expand in both x and y direction */
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   /* tell the window that this object is to be resized along with the window.
    * also as a result this object will be one of several objects that
    * controls the minimum/maximum size of the window */
   elm_win_resize_object_add(win, bg);
   /* and show the background */
   evas_object_show(bg);

   /* add a box layout widget to the window */
   bx0 = elm_box_add(win);
   /* allow base box (bx0) to expand in x and y */
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   /* tell the window that the box affects window size and also will be
    * resized when the window is */
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Information");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "Please select a test from the list below<br>"
		       "by clicking the test button to show the<br>"
		       "test window.");
   elm_frame_content_set(fr, lb);
   evas_object_show(lb);

   li = elm_list_add(win);
   elm_list_always_select_mode_set(li, 1);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx0, li);
   evas_object_show(li);

   elm_list_item_append(li, "Bg Plain", NULL, NULL, test_bg_plain, NULL);
   elm_list_item_append(li, "Bg Image", NULL, NULL, test_bg_image, NULL);
   elm_list_item_append(li, "Icon Transparent", NULL, NULL, test_icon, NULL);
   elm_list_item_append(li, "Box Vert", NULL, NULL, test_box_vert, NULL);
   elm_list_item_append(li, "Box Vert 2", NULL, NULL, test_box_vert2, NULL);
   elm_list_item_append(li, "Box Horiz", NULL, NULL, test_box_horiz, NULL);
   elm_list_item_append(li, "Buttons", NULL, NULL, test_button, NULL);
   elm_list_item_append(li, "Toggles", NULL, NULL, test_toggle, NULL);
   elm_list_item_append(li, "Table", NULL, NULL, test_table, NULL);
   elm_list_item_append(li, "Clock", NULL, NULL, test_clock, NULL);
   elm_list_item_append(li, "Layout", NULL, NULL, test_layout, NULL);
   elm_list_item_append(li, "Hover", NULL, NULL, test_hover, NULL);
   elm_list_item_append(li, "Hover 2", NULL, NULL, test_hover2, NULL);
   elm_list_item_append(li, "Entry", NULL, NULL, test_entry, NULL);
   elm_list_item_append(li, "Entry Scrolled", NULL, NULL, test_entry_scrolled, NULL);
   elm_list_item_append(li, "Notepad", NULL, NULL, test_notepad, NULL);
   elm_list_item_append(li, "Anchorview", NULL, NULL, test_anchorview, NULL);
   elm_list_item_append(li, "Anchorblock", NULL, NULL, test_anchorblock, NULL);
   elm_list_item_append(li, "Toolbar", NULL, NULL, test_toolbar, NULL);
   elm_list_item_append(li, "Hoversel", NULL, NULL, test_hoversel, NULL);
   elm_list_item_append(li, "List", NULL, NULL, test_list, NULL);
   elm_list_item_append(li, "List 2", NULL, NULL, test_list2, NULL);
   elm_list_item_append(li, "List 3", NULL, NULL, test_list3, NULL);
   elm_list_item_append(li, "Carousel", NULL, NULL, test_carousel, NULL);
   elm_list_item_append(li, "Inwin", NULL, NULL, test_inwin, NULL);
   elm_list_item_append(li, "Inwin 2", NULL, NULL, test_inwin2, NULL);
   elm_list_item_append(li, "Scaling", NULL, NULL, test_scaling, NULL);
   elm_list_item_append(li, "Scaling 2", NULL, NULL, test_scaling2, NULL);
   elm_list_item_append(li, "Slider", NULL, NULL, test_slider, NULL);
   elm_list_item_append(li, "Genlist", NULL, NULL, test_genlist, NULL);
   elm_list_item_append(li, "Genlist 2", NULL, NULL, test_genlist2, NULL);
   elm_list_item_append(li, "Genlist 3", NULL, NULL, test_genlist3, NULL);
   elm_list_item_append(li, "Genlist 4", NULL, NULL, test_genlist4, NULL);
   elm_list_item_append(li, "Genlist 5", NULL, NULL, test_genlist5, NULL);
   elm_list_item_append(li, "Genlist Tree", NULL, NULL, test_genlist6, NULL);
   elm_list_item_append(li, "Checks", NULL, NULL, test_check, NULL);
   elm_list_item_append(li, "Radios", NULL, NULL, test_radio, NULL);
   elm_list_item_append(li, "Pager", NULL, NULL, test_pager, NULL);
   elm_list_item_append(li, "Window States", NULL, NULL, test_win_state, NULL);
   elm_list_item_append(li, "Progressbar", NULL, NULL, test_progressbar, NULL);
   elm_list_item_append(li, "File Selector", NULL, NULL, test_fileselector, NULL);
   elm_list_item_append(li, "Separator", NULL, NULL, test_separator, NULL);
   elm_list_item_append(li, "Scroller", NULL, NULL, test_scroller, NULL);
   elm_list_item_append(li, "Spinner", NULL, NULL, test_spinner, NULL);
   elm_list_item_append(li, "Index", NULL, NULL, test_index, NULL);
   elm_list_item_append(li, "Photocam", NULL, NULL, test_photocam, NULL);
   elm_list_item_append(li, "Photo", NULL, NULL, test_photo, NULL);
   elm_list_item_append(li, "Icon Desktops", NULL, NULL, test_icon_desktops, NULL);
   elm_list_item_append(li, "Notify", NULL, NULL, test_notify, NULL);
   elm_list_item_append(li, "Slideshow", NULL, NULL, test_slideshow, NULL);
   elm_list_item_append(li, "Menu", NULL, NULL, test_menu, NULL);
   elm_list_item_append(li, "Panel", NULL, NULL, test_panel, NULL);
   elm_list_item_append(li, "Map", NULL, NULL, test_map, NULL);
   elm_list_item_append(li, "Weather", NULL, NULL, test_weather, NULL);
   elm_list_item_append(li, "Flip", NULL, NULL, test_flip, NULL);
   elm_list_item_append(li, "Label", NULL, NULL, test_label, NULL);
   elm_list_item_append(li, "Conformant", NULL, NULL, test_conformant, NULL);
   elm_list_item_append(li, "Multi Touch", NULL, NULL, test_multi, NULL);
   elm_list_item_append(li, "Floating Objects", NULL, NULL, test_floating, NULL);

   elm_list_go(li);

   /* set an initial window size */
   evas_object_resize(win, 240, 480);
   /* show the window */
   evas_object_show(win);
}

/* this is your elementary main function - it MUSt be called IMMEDIATELY
 * after elm_init() and MUSt be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */
EAPI int
elm_main(int argc, char **argv)
{
   /* put ere any init specific to this app like parsing args etc. */
   my_win_main(); /* create main window */
   elm_run(); /* and run the program now  and handle all events etc. */
   /* if the mainloop that elm_run() runs exist - we exit the app */
   elm_shutdown(); /* clean up and shut down */
   /* exit code */
   return 0;
}
#endif
/* all emeentary apps should use this. but it right after elm_main() */
ELM_MAIN()
