#include <Elementary.h>

static void
my_bt_40_done(void *data, Evas_Object *obj, void *event_info)
{
   /* event_info conatin the full path of the selected file
    * or NULL if none is selected or cancel is pressed */
   const char *selected = event_info;
   
   if (selected)
     printf("Selected file: %s\n", selected);
   else
     evas_object_del(data); /* delete the test window */
}

static void
my_bt_40_selected(void *data, Evas_Object *obj, void *event_info)
{
   /* event_info conatin the full path of the selected file */
   const char *selected = event_info;
   printf("Selected file: %s\n", selected);
   
   /* or you can query the selection */
   printf("or: %s\n", elm_fileselector_selected_get(obj));
}

void
test_fileselector(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *fs, *bg;
   char buf[PATH_MAX];

   /* Set the locale according to the system pref.
    * If you dont do so the file selector will order the files list in
    * a case sensitive manner
    */
   setlocale(LC_ALL, "");

   win = elm_win_add(NULL, "fileselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "File Selector");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   fs = elm_fileselector_add(win);
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   /* start the fileselector in the home dir */
   elm_fileselector_path_set(fs, getenv("HOME"));
   /* allow fs to expand in x & y */
   evas_object_size_hint_weight_set(fs, 1.0, 1.0);
   elm_win_resize_object_add(win, fs);
   evas_object_show(fs);
   /* the 'done' cb is called when the user press ok/cancel */
   evas_object_smart_callback_add(fs, "done", my_bt_40_done, win);
   evas_object_smart_callback_add(fs, "selected", my_bt_40_selected, win);

   evas_object_resize(win, 240, 350);
   evas_object_show(win);
}
