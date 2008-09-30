#include <Elementary.h>

/* This is a test app (that is also functional) to test the api and serve
 * as an example
 */

static void
on_win_resize(void *data, Elm_Win *win, Elm_Cb_Type type, void *info)
{
   /* window is resized */
   printf("resize to: %ix%i\n", win->w, win->h);
}

static void
on_win_del_req(void *data, Elm_Win *win, Elm_Cb_Type type, void *info)
{
   /* because autodel is on - after this callback the window will be deleted */
   /* but to be explicit - exit the app when window is closed */
   elm_exit();
}

static void
win_bg_simple(void)
{
   Elm_Win *win;
   Elm_Bg *bg;

   win = elm_win_new(); /* create a window */
   win->name_set(win, "win_simple"); /* set the window name - used by window 
				      * manager. make it uniqie for windows
				      * with in this application */
   win->title_set(win, "Simple Window with default Bg"); /* set the title */
   win->autodel = 0; /* dont auto delete the window if someone closes it.
		      * this means the del+req handler has to delete it. by
		      * default it is on */
   /* add a callback that is called when the user tries to close the window */
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   /* add a callback that gets called when the window is resized */
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   /* our window needs a baground, so ask for one - it will be set with a
    * default bg */
   bg = elm_bg_new(win);
   bg->show(bg); /* show the bg */
   
   win->size_req(win, NULL, 240, 320); /* request that the window is 240x240 
					* no min/max size enforced */
   win->show(win); /* show the window */
}

static void
win_bg_image(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   char buf[PATH_MAX];

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with and image Bg");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   /* this is a test - shows how to have your own custom wallpaper in
    * your app - don't use this unless you have a very good reason! there
    * is a default and all apps look nicer sharing the default, but if
    * you insist... */
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   bg->file_set(bg, buf, NULL); /* set the bg - the NULL is for special
				 * files that contain multiple images
				 * inside 1 file. not normally used but
				 * might be if you have archive files with
				 * multiple images in them */
   bg->show(bg);

   win->size_req(win, NULL, 240, 240);
   win->show(win);
}

static void
win_scrollable_label(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Scroller *scroller;
   Elm_Label *label;

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with scroller and label inside");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   bg->show(bg);
   
   scroller = elm_scroller_new(win);
   
   label = elm_label_new(win);
   label->text_set(label, 
		   "Hello world<br>"
		   "<br>"
		   "I am a label. I come here to temonstrate how to put<br>"
		   "text into a label, with newlines, even markup to test<br>"
		   "things like <b>bold text</b> where markup can be custom<br>"
		   "and extensible, defined by the theme's textbloxk style<br>"
		   "for the label.<br>"
		   "<br>"
		   "Note that the markup is html-like and things like newline<br>"
		   "chars and tab chars like stdout text are not valid text<br>"
		   "markup mechanisms. Use markup tags instead.<br>"
		   "<br>"
		   "Now we shall borrow some text from another test program<br>"
		   "(Expedite) to put some more tests in here<br>"
		   "<br>"
		   "<style=outline color=#fff outline_color=#000>Styled</> "
		   "<style=shadow shadow_color=#fff8>text</> "
		   "<style=soft_shadow shadow_color=#0002>should</> "
		   "<style=glow color=#fff glow2_color=#fe87 glow_color=#f214 >go here</> "
		   "<style=far_shadow shadow_color=#0005>as it is</> "
		   "<style=outline_shadow color=#fff outline_color=#8228 shadow_color=#005>within</> "
		   "<style=outline_soft_shadow color=#fff outline_color=#8228 shadow_color=#0002>right tags</> "
		   "<style=far_soft_shadow color=#fff shadow_color=#0002>to make it align to the</> "
		   "<underline=on underline_color=#00f>right hand</> "
		   "<backing=on backing_color=#fff8>side </><backing_color=#ff08>of</><backing_color=#0f08> </>"
		   "<strikethrough=on strikethrough_color=#f0f8>the textblock</>."
		   "<br>"
		   "<underline=double underline_color=#f00 underline2_color=#00f>now we need</> "
		   "to test some <color=#f00 font_size=8>C</><color=#0f0 font_size=10>O</>"
		   "<color=#00f font_size=12>L</><color=#fff font_size=14>O</>"
		   "<color=#ff0 font_size=16>R</><color=#0ff font_size=18> Bla Rai</>"
		   "<color=#f0f font_size=20> Stuff</>."
		   "<br>"
		   "<style=outline color=#fff outline_color=#000>Round about the cauldron go;</> "
		   "In the poison'd entrails throw.<br>"
		   "<style=shadow shadow_color=#fff8>Toad, that under cold stone</> "
		   "Days and nights has thirty-one<br>"
		   "<style=soft_shadow shadow_color=#0002>Swelter'd venom sleeping got,</> "
		   "<style=glow color=#fff glow2_color=#fe87 glow_color=#f214 >Boil thou first i' the charmed pot.</><br>"
		   "Double, double toil and trouble; "
		   "Fire burn, and cauldron bubble.<br>"
		   "<style=far_shadow shadow_color=#0005>Fillet of a fenny snake,</> "
		   "In the cauldron boil and bake;<br>"
		   "<style=outline_shadow color=#fff outline_color=#8228 shadow_color=#005>Eye of newt and toe of frog,</> "
		   "<underline=on underline_color=#00f>Wool of bat and tongue of dog,</><br>"
		   "<backing=on backing_color=#fff8>Adder's fork and blind-worm's sting,</> "
		   "<underline=double underline_color=#f00 underline2_color=#00f>Lizard's leg and owlet's wing,</><br>"
		   "<color=#808 font_size=20>For a charm of powerful trouble, "
		   "Like a hell-broth boil and bubble.<br>"
		   "Double, double toil and trouble;</> "
		   "Fire burn and cauldron bubble.<br>"
		   "Scale of dragon, tooth of wolf, "
		   "Witches' mummy, maw and gulf<br>"
		   "Of the ravin'd salt-sea shark, "
		   "Root of hemlock digg'd i' the dark,<br>"
		   "Liver of blaspheming Jew, "
		   "Gall of goat, and slips of yew<br>"
		   "Silver'd in the moon's eclipse, "
		   "Nose of Turk and Tartar's lips,<br>"
		   "Finger of birth-strangled babe "
		   "Ditch-deliver'd by a drab,<br>"
		   "Make the gruel thick and slab: "
		   "Add thereto a tiger's chaudron,<br>"
		   "For the ingredients of our cauldron. "
		   "Double, double toil and trouble;<br>"
		   "Fire burn and cauldron bubble. "
		   "Cool it with a baboon's blood,<br>"
		   "Then the charm is firm and good.<br>"
		   "<br>"
		   "Heizölrückstoßabdämpfung fløde pingüino kilómetros cœur déçu l'âme<br>"
		   "plutôt naïve Louÿs rêva crapaüter Íosa Úrmhac Óighe pór Éava Ádhaim<br>"
		   );
   scroller->child_add(scroller, label);
   label->show(label);
   scroller->show(scroller);

   win->size_req(win, NULL, 240, 480);
   win->show(win);
}

static void
win_label_determines_min_size(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Label *label;

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with label setting minimum size");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   bg->expand_x = 0; /* allows the window to grow in the y axis because */
   bg->expand_y = 1; /* its only child can expand in y */
   bg->show(bg);
   
   label = elm_label_new(win);
   label->text_set(label, 
		   "Hello world<br>"
		   "<br>"
		   "I am a label. I come here to temonstrate how to put<br>"
		   "text into a label, with newlines, even markup to test<br>"
		   "things like <b>bold text</b> where markup can be custom<br>"
		   "and extensible, defined by the theme's textbloxk style<br>"
		   "for the label.<br>"
		   "<br>"
		   "Note that the markup is html-like and things like newline<br>"
		   "chars and tab chars like stdout text are not valid text<br>"
		   "markup mechanisms. Use markup tags instead.<br>"
		   );
   label->show(label);
   label->expand_x = 0; /* allows the window to grow in the y axis because */
   label->expand_y = 1; /* its only child can expand in y */
   /* why do i change expand on both bg and label? both are children of the
    * window widget and thus both affect the window sizing. if any expands
    * in an axis then window expanding is allowed always */
   elm_widget_sizing_update(label); /* make sure that the lable knows about its
				     * sizing changes like expand above */
   win->show(win);
}

static void
win_box_vert_of_labels(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Box *box;
   Elm_Label *label;

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with box + labels setting minimum size");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   bg->expand_x = 1;
   bg->expand_y = 1;
   bg->show(bg);
   
   box = elm_box_new(win);
   box->expand_x = 1;
   box->expand_y = 1;
   
   label = elm_label_new(win);
   label->text_set(label, "Expand X/Y 0/0, Fill X/Y 0/0, Align: 0.5 0.5");
   box->pack_end(box, label);
   label->show(label);
   label->align_x = 0.5;
   label->align_y = 0.5;
   label->expand_x = 0;
   label->expand_y = 0;
   label->fill_x = 0;
   label->fill_y = 0;
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Expand X/Y 1/1, Fill X/Y 0/0, Align: 0.5 0.5");
   box->pack_end(box, label);
   label->show(label);
   label->align_x = 0.5;
   label->align_y = 0.5;
   label->expand_x = 1;
   label->expand_y = 1;
   label->fill_x = 0;
   label->fill_y = 0;
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Expand X/Y 1/1, Fill X/Y 1/1, Align: 0.5 0.5");
   box->pack_end(box, label);
   label->show(label);
   label->align_x = 0.5;
   label->align_y = 0.5;
   label->expand_x = 1;
   label->expand_y = 1;
   label->fill_x = 1;
   label->fill_y = 1;
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Expand X/Y 0/0, Fill X/Y 1/1, Align: 0.5 0.5");
   box->pack_end(box, label);
   label->show(label);
   label->align_x = 0.5;
   label->align_y = 0.5;
   label->expand_x = 0;
   label->expand_y = 0;
   label->fill_x = 1;
   label->fill_y = 1;
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Expand X/Y 0/0, Fill X/Y 1/1, Align: 0.0 0.5");
   box->pack_end(box, label);
   label->show(label);
   label->align_x = 0.0;
   label->align_y = 0.5;
   label->expand_x = 0;
   label->expand_y = 0;
   label->fill_x = 1;
   label->fill_y = 1;
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Expand X/Y 0/0, Fill X/Y 1/1, Align: 1.0 0.5");
   box->pack_end(box, label);
   label->show(label);
   label->align_x = 1.0;
   label->align_y = 0.5;
   label->expand_x = 0;
   label->expand_y = 0;
   label->fill_x = 1;
   label->fill_y = 1;
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Expand X/Y 0/0, Fill X/Y 1/1, Align: 0.5 0.0");
   box->pack_end(box, label);
   label->show(label);
   label->align_x = 0.5;
   label->align_y = 0.0;
   label->expand_x = 0;
   label->expand_y = 0;
   label->fill_x = 1;
   label->fill_y = 1;
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Expand X/Y 0/0, Fill X/Y 1/1, Align: 0.5 1.0");
   box->pack_end(box, label);
   label->show(label);
   label->align_x = 0.5;
   label->align_y = 1.0;
   label->expand_x = 0;
   label->expand_y = 0;
   label->fill_x = 1;
   label->fill_y = 1;
   elm_widget_sizing_update(label);

   elm_widget_sizing_update(box);
   box->show(box);
   
   win->show(win);
}

static void
win_scrollable_box_vert_of_labels(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Scroller *scroller;
   Elm_Box *box;
   Elm_Label *label;
   int i;

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with scroller and box + labels inside");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   bg->expand_x = 1;
   bg->expand_y = 1;
   bg->show(bg);
   
   scroller = elm_scroller_new(win);

   box = elm_box_new(win);
   box->expand_x = 1;
   box->expand_y = 1;

   for (i = 0; i < 40; i++)
     {
	char buf[200];
	
	snprintf(buf, sizeof(buf), "This is a Label in a box, #%i", i);
	label = elm_label_new(win);
	label->text_set(label, buf);
	box->pack_end(box, label);
	label->show(label);
	label->expand_x = 0;
	label->expand_y = 0;
	elm_widget_sizing_update(label);
	label->show(label);
     }

   scroller->child_add(scroller, box);
   
   elm_widget_sizing_update(box);
   box->show(box);
   
   scroller->show(scroller);

   win->size_req(win, NULL, 240, 240);
   win->show(win);
}

static void
win_table_of_labels(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Table *table;
   Elm_Label *label;

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with table + labels setting minimum size");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   bg->expand_x = 1;
   bg->expand_y = 1;
   bg->show(bg);
   
   table = elm_table_new(win);
   table->expand_x = 1;
   table->expand_y = 1;
   
   label = elm_label_new(win);
   label->text_set(label, "X");
   table->pack(table, label, 0, 0, 1, 1);
   label->fill_x = 0;
   label->fill_y = 0;
   label->show(label);
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Y");
   table->pack(table, label, 1, 0, 1, 1);
   label->fill_x = 0;
   label->fill_y = 0;
   label->show(label);
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "Z");
   table->pack(table, label, 2, 0, 1, 1);
   label->fill_x = 0;
   label->fill_y = 0;
   label->show(label);
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "A");
   table->pack(table, label, 0, 1, 1, 1);
   label->fill_x = 0;
   label->fill_y = 0;
   label->show(label);
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "BB");
   table->pack(table, label, 1, 1, 2, 1);
   label->fill_x = 0;
   label->fill_y = 0;
   label->show(label);
   elm_widget_sizing_update(label);

   label = elm_label_new(win);
   label->text_set(label, "CCC");
   table->pack(table, label, 0, 2, 3, 1);
   label->fill_x = 0;
   label->fill_y = 0;
   label->show(label);
   elm_widget_sizing_update(label);

   elm_widget_sizing_update(table);
   table->show(table);
   
   win->show(win);
}

static void
on_button_activate(void *data, Elm_Button *bt, Elm_Cb_Type type, void *info)
{
   printf("Button %p activate\n", bt);
}

static void
win_table_of_buttons(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Icon *icon;
   Elm_Table *table;
   Elm_Button *button;
   char buf[PATH_MAX];

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with table + buttons setting minimum size");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   bg->expand_x = 1;
   bg->expand_y = 1;
   bg->show(bg);
   
   table = elm_table_new(win);
   table->expand_x = 1;
   table->expand_y = 1;

   button = elm_button_new(win);
   button->text_set(button, "Button 1");
   table->pack(table, button, 0, 0, 1, 1);
   button->fill_x = 0;
   button->fill_y = 0;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);

   button = elm_button_new(win);
   button->text_set(button, "Button 2");
   table->pack(table, button, 1, 0, 1, 1);
   button->fill_x = 0;
   button->fill_y = 0;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);

   button = elm_button_new(win);
   button->text_set(button, "Button 3");
   table->pack(table, button, 2, 0, 1, 1);
   button->fill_x = 0;
   button->fill_y = 0;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);

   button = elm_button_new(win);
   button->text_set(button, "Button 4");
   table->pack(table, button, 0, 1, 1, 1);
   button->fill_x = 1;
   button->fill_y = 0;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);

   button = elm_button_new(win);
   icon = elm_icon_new(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   icon->file_set(icon, buf, NULL);
   icon->scale_up = 0;
   icon->layout_update(icon);
   button->child_add(button, icon);
   table->pack(table, button, 1, 1, 2, 1);
   button->fill_x = 1;
   button->fill_y = 1;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);

   button = elm_button_new(win);
   button->text_set(button, "Button 6");
   icon = elm_icon_new(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
   icon->file_set(icon, buf, NULL);
   button->child_add(button, icon);
   table->pack(table, button, 0, 2, 3, 1);
   button->fill_x = 1;
   button->fill_y = 1;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);

   elm_widget_sizing_update(table);
   table->show(table);
   
   win->show(win);
}

static void
on_toggle_changed(void *data, Elm_Toggle *tg, Elm_Cb_Type type, void *info)
{
   printf("toggle: %i\n", tg->state);
}

static void
win_box_vert_of_toggles(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Box *box;
   Elm_Toggle *toggle;
   Elm_Icon *icon;
   char buf[PATH_MAX];

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with box + toggles setting min size");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   bg->expand_x = 0;
   bg->expand_y = 0;
   bg->show(bg);
   
   box = elm_box_new(win);
   box->expand_x = 0;
   box->expand_y = 0;

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Label ON/OFF");
   toggle->state = 1;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Icon + Label ON/OFF");
   icon = elm_icon_new(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
   icon->file_set(icon, buf, NULL);
   toggle->child_add(toggle, icon);
   toggle->state = 1;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, NULL);
   icon = elm_icon_new(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
   icon->file_set(icon, buf, NULL);
   toggle->child_add(toggle, icon);
   toggle->state = 1;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Label Yes/No");
   toggle->states_text_set(toggle, "Yes", "No");
   toggle->state = 1;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   elm_widget_sizing_update(box);
   box->show(box);
   
   win->show(win);
}

static void
win_scrollable_box_vert_of_toggles_and_buttons(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Box *box;
   Elm_Scroller *scroller;
   Elm_Toggle *toggle;
   Elm_Icon *icon;
   Elm_Button *button;
   Elm_Label *label;
   char buf[PATH_MAX];

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with scroller + box + toggles + buttons");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   bg->expand_x = 1;
   bg->expand_y = 1;
   bg->show(bg);
   
   scroller = elm_scroller_new(win);

   box = elm_box_new(win);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Label ON/OFF");
   toggle->state = 1;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Icon + Label ON/OFF");
   icon = elm_icon_new(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
   icon->file_set(icon, buf, NULL);
   toggle->child_add(toggle, icon);
   toggle->state = 1;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, NULL);
   icon = elm_icon_new(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
   icon->file_set(icon, buf, NULL);
   toggle->child_add(toggle, icon);
   toggle->state = 1;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Label Yes/No");
   toggle->states_text_set(toggle, "Yes", "No");
   toggle->state = 1;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Toggle ");
   toggle->states_text_set(toggle, "Up", "Down");
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Toggle ");
   toggle->states_text_set(toggle, "In", "Out");
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Toggle ");
   toggle->states_text_set(toggle, "Up", "Down");
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   label = elm_label_new(win);
   label->text_set(label, 
		   "Hello world<br>"
		   "<br>"
		   "I am a label. I come here to temonstrate how to put<br>"
		   "text into a label, with newlines, even markup to test<br>"
		   "things like <b>bold text</b> where markup can be custom<br>"
		   "and extensible, defined by the theme's textbloxk style<br>"
		   "for the label.<br>"
		   "<br>"
		   "Note that the markup is html-like and things like newline<br>"
		   "chars and tab chars like stdout text are not valid text<br>"
		   "markup mechanisms. Use markup tags instead.<br>"
		   );
   box->pack_end(box, label);
   label->expand_y = 0;
   label->fill_y = 0;
   elm_widget_sizing_update(label);
   label->show(label);
   
   button = elm_button_new(win);
   button->text_set(button, "Button 1");
   box->pack_end(box, button);
   button->expand_y = 0;
   button->fill_y = 0;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);
   
   button = elm_button_new(win);
   button->text_set(button, "Button 2");
   box->pack_end(box, button);
   button->expand_y = 0;
   button->fill_y = 0;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);
   
   button = elm_button_new(win);
   button->text_set(button, "Button 3");
   box->pack_end(box, button);
   button->expand_y = 0;
   button->fill_y = 0;
   button->show(button);
   elm_widget_sizing_update(button);
   button->cb_add(button, ELM_CB_ACTIVATED, on_button_activate, NULL);
   
   scroller->child_add(scroller, box);
   
   elm_widget_sizing_update(box);
   box->show(box);
   
   scroller->show(scroller);

   win->size_req(win, NULL, 400, 320);
   
   win->show(win);
}

static void
on_clock_changed(void *data, Elm_Clock *cloc, Elm_Cb_Type type, void *info)
{
   printf("clock: %i:%i:%i\n", cloc->hrs, cloc->min, cloc->sec);
}

static void
win_box_vert_of_clock_and_toggles(void)
{
   Elm_Win *win;
   Elm_Bg *bg;
   Elm_Box *box;
   Elm_Toggle *toggle;
   Elm_Clock *cloc;
   Elm_Pad *pad;
   Elm_Frame *frame;
   char buf[PATH_MAX];

   win = elm_win_new();
   win->name_set(win, "win_bg");
   win->title_set(win, "Simple Window with box + toggles setting min size");
   win->autodel = 0;
   win->cb_add(win, ELM_CB_DEL_REQ, on_win_del_req, NULL);
   win->cb_add(win, ELM_CB_RESIZE, on_win_resize, NULL);

   bg = elm_bg_new(win);
   snprintf(buf, sizeof(buf), "%s/images/sky_04.jpg", PACKAGE_DATA_DIR);
   bg->file_set(bg, buf, NULL);
   bg->expand_x = 0;
   bg->expand_y = 0;
   bg->show(bg);
   
   pad = elm_pad_new(win);
   pad->expand_x = 0;
   pad->expand_y = 0;
   pad->show(pad);

   frame = elm_frame_new(win);
   pad->child_add(pad, frame);
   frame->text_set(frame, "Time");
   frame->expand_x = 0;
   frame->expand_y = 0;
   frame->show(frame);
/*
   pad = elm_pad_new(win);
   frame->child_add(frame, pad);
   pad->expand_x = 0;
   pad->expand_y = 0;
   pad->show(pad);
*/
   box = elm_box_new(win);
//   pad->child_add(pad, box);
   frame->child_add(frame, box);
   box->expand_x = 0;
   box->expand_y = 0;

   toggle = elm_toggle_new(win);
   toggle->text_set(toggle, "Alarm");
   toggle->states_text_set(toggle, "ON", "OFF");
   toggle->state = 0;
   toggle->layout_update(toggle);
   toggle->cb_add(toggle, ELM_CB_CHANGED, on_toggle_changed, NULL);
   box->pack_end(box, toggle);
   toggle->show(toggle);
   toggle->expand_y = 0;
   toggle->fill_y = 0;
   elm_widget_sizing_update(toggle);

   cloc = elm_clock_new(win);
   cloc->expand_x = 0;
   cloc->fill_x = 0;
   cloc->edit = 1;
   cloc->hrs = 6;
   cloc->min = 54;
   cloc->sec = 32;
   cloc->time_update(cloc);
   cloc->cb_add(cloc, ELM_CB_CHANGED, on_clock_changed, NULL);
   box->pack_end(box, cloc);
   elm_widget_sizing_update(cloc);
   cloc->show(cloc);
   
   cloc = elm_clock_new(win);
   cloc->expand_x = 0;
   cloc->fill_x = 0;
   box->pack_end(box, cloc);
   elm_widget_sizing_update(cloc);
   cloc->show(cloc);
   
   cloc = elm_clock_new(win);
   cloc->am_pm = 0;
   cloc->seconds = 1;
   cloc->time_update(cloc);
   cloc->expand_x = 0;
   cloc->fill_x = 0;
   box->pack_end(box, cloc);
   elm_widget_sizing_update(cloc);
   cloc->show(cloc);
   
   cloc = elm_clock_new(win);
   cloc->am_pm = 0;
   cloc->seconds = 0;
   cloc->time_update(cloc);
   cloc->expand_x = 0;
   cloc->fill_x = 0;
   box->pack_end(box, cloc);
   elm_widget_sizing_update(cloc);
   cloc->show(cloc);

/*   
   cloc = elm_clock_new(win);
   cloc->am_pm = 1;
   cloc->seconds = 0;
   cloc->time_update(cloc);
   cloc->expand_x = 0;
   cloc->fill_x = 0;
   box->pack_end(box, cloc);
   elm_widget_sizing_update(cloc);
   cloc->show(cloc);
 */
   
   elm_widget_sizing_update(box);
   box->show(box);

   win->show(win);
}

int
main(int argc, char **argv)
{
   /* init Elementary (all Elementary calls begin with elm_ and all data
    * types, enums and macros will be Elm_ and ELM_ etc.) */
   elm_init(argc, argv);

   /* setup some windows with test widgets in them */
   win_bg_simple();
   win_bg_image();
   win_scrollable_label();
   win_label_determines_min_size();
   win_box_vert_of_labels();
   win_scrollable_box_vert_of_labels();
   win_table_of_labels();
   win_table_of_buttons();
   win_box_vert_of_toggles();
   win_scrollable_box_vert_of_toggles_and_buttons();
   win_box_vert_of_clock_and_toggles();
   
   elm_run(); /* and run the program now  and handle all events etc. */
   
   elm_shutdown(); /* clean up and shut down */
   return 0; 
}
