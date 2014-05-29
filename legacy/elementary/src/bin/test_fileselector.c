#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#ifndef ENABLE_NLS
# ifndef setlocale
#  define setlocale(c, l)
# endif
# ifndef libintl_setlocale
#  define libintl_setlocale(c, l)
# endif
#endif

#include <Elementary.h>

static void
my_fileselector_done(void            *data,
                     Evas_Object *obj EINA_UNUSED,
                     void            *event_info)
{
   /* event_info conatin the full path of the selected file
    * or NULL if none is selected or cancel is pressed */
    const char *selected = event_info;

    if (selected)
      printf("Selected file: %s\n", selected);
    else
      evas_object_del(data);  /* delete the test window */
}

static void
my_fileselector_selected(void *data   EINA_UNUSED,
                         Evas_Object *obj,
                         void        *event_info)
{
   /* event_info conatin the full path of the selected file */
   const char *selected = event_info;
   printf("Selected file: %s\n", selected);

   /* or you can query the selection */
   if (elm_fileselector_multi_select_get(obj))
     {
        const Eina_List *li;
        const Eina_List *paths = elm_fileselector_selected_paths_get(obj);
        char *path;
        printf("All selected files are:\n");
        EINA_LIST_FOREACH(paths, li, path)
          printf(" %s\n", path);
     }
   else
     printf("or: %s\n", elm_fileselector_selected_get(obj));
}

static void
_popup_close_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
my_fileselector_invalid(void *data   EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void        *event_info)
{
   Evas_Object *popup;
   Evas_Object *btn;
   char error_msg[256];

   snprintf(error_msg, 256, "No such file or directory: %s", (char *)event_info);

   popup = elm_popup_add(data);
   elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_CHAR);
   elm_object_part_text_set(popup, "title,text", "Error");
   elm_object_text_set(popup, error_msg);

   btn = elm_button_add(popup);
   elm_object_text_set(btn, "OK");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   evas_object_show(popup);
}

static void
my_fileselector_activated(void *data EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info)
{
   printf("Activated file: %s\n", (char *)event_info);
}

static void
_is_save_clicked(void            *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   printf("Toggle Is save\n");
   if (elm_fileselector_is_save_get(fs))
     elm_fileselector_is_save_set(fs, EINA_FALSE);
   else
     elm_fileselector_is_save_set(fs, EINA_TRUE);
}

static void
_folder_only_clicked(void            *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   printf("Toggle Folder only\n");
   if (elm_fileselector_folder_only_get(fs))
     elm_fileselector_folder_only_set(fs, EINA_FALSE);
   else
     elm_fileselector_folder_only_set(fs, EINA_TRUE);
}

static void
_expandable_clicked(void            *data,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   printf("Toggle Expandable\n");
   if (elm_fileselector_expandable_get(fs))
     elm_fileselector_expandable_set(fs, EINA_FALSE);
   else
     elm_fileselector_expandable_set(fs, EINA_TRUE);
}

static void
_multi_clicked(void            *data,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Eina_Bool enabled = elm_fileselector_multi_select_get(fs);
   printf("Toggle Multiple selection to: %s\n", !enabled ? "On" : "Off");
   elm_fileselector_multi_select_set(fs, !enabled);
}

static void
_buttons_clicked(void            *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   printf("Toggle Buttons\n");
   if (elm_fileselector_buttons_ok_cancel_get(fs))
     elm_fileselector_buttons_ok_cancel_set(fs, EINA_FALSE);
   else
     elm_fileselector_buttons_ok_cancel_set(fs, EINA_TRUE);
}

static void
_hidden_clicked(void            *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   printf("Toggle visibility of hidden files/directories\n");
   if (elm_fileselector_hidden_visible_get(fs))
     elm_fileselector_hidden_visible_set(fs, EINA_FALSE);
   else
     elm_fileselector_hidden_visible_set(fs, EINA_TRUE);
}

static void
_sel_get_clicked(void            *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   printf("Get Selected: %s\n", elm_fileselector_selected_get(fs));
}

static void
_path_get_clicked(void            *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   printf("Get Path: %s\n", elm_fileselector_path_get(fs));
}

static Evas_Object *
_option_create(Evas_Object *parent, Evas_Object *fs)
{
   Evas_Object *frame = NULL, *box = NULL, *hbox = NULL, *bt = NULL;

   frame = elm_frame_add(parent);
   evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, 0);
   elm_object_text_set(frame, "Options");
   evas_object_show(frame);

   // outer vertical box
   box = elm_box_add(frame);
   elm_object_content_set(frame, box);
   evas_object_show(box);

   // first horizontal box
   hbox = elm_box_add(frame);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   bt = elm_check_add(hbox);
   elm_object_text_set(bt, "is save");
   elm_check_state_set(bt, elm_fileselector_is_save_get(fs));
   evas_object_smart_callback_add(bt, "changed", _is_save_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_check_add(hbox);
   elm_object_text_set(bt, "folder only");
   elm_check_state_set(bt, elm_fileselector_folder_only_get(fs));
   evas_object_smart_callback_add(bt, "changed", _folder_only_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_check_add(hbox);
   elm_object_text_set(bt, "expandable");
   elm_check_state_set(bt, elm_fileselector_expandable_get(fs));
   evas_object_smart_callback_add(bt, "changed", _expandable_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   // second horizontal box
   hbox = elm_box_add(frame);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   bt = elm_check_add(hbox);
   elm_object_text_set(bt, "multiple selection");
   elm_check_state_set(bt, elm_fileselector_multi_select_get(fs));
   evas_object_smart_callback_add(bt, "changed", _multi_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_check_add(hbox);
   elm_object_text_set(bt, "buttons");
   elm_check_state_set(bt, elm_fileselector_buttons_ok_cancel_get(fs));
   evas_object_smart_callback_add(bt, "changed", _buttons_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_check_add(hbox);
   elm_object_text_set(bt, "hidden");
   elm_check_state_set(bt, elm_fileselector_hidden_visible_get(fs));
   evas_object_smart_callback_add(bt, "changed", _hidden_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   return frame;
}

static Evas_Object *
_getter_option_create(Evas_Object *parent, Evas_Object *fs)
{
   Evas_Object *frame = NULL, *hbox = NULL, *bt = NULL;

   frame = elm_frame_add(parent);
   elm_object_text_set(frame, "Getter Options");
   evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, 0);
   evas_object_show(frame);

   hbox = elm_box_add(frame);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_object_content_set(frame, hbox);
   evas_object_show(hbox);

   bt = elm_button_add(hbox);
   elm_object_text_set(bt, "selected get");
   evas_object_smart_callback_add(bt, "clicked", _sel_get_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(hbox);
   elm_object_text_set(bt, "path get");
   evas_object_smart_callback_add(bt, "clicked", _path_get_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   return frame;
}

static void
_mode_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Elm_Fileselector_Mode mode;

   mode = (elm_fileselector_mode_get(fs) + 1) % ELM_FILESELECTOR_LAST;
   switch (mode)
     {
      case ELM_FILESELECTOR_LIST:
         printf("Setting mode to: List\n");
         break;
      case ELM_FILESELECTOR_GRID:
      default:
         printf("Setting mode to: Grid\n");
         break;
     }

   elm_radio_value_set(obj, mode);
   elm_fileselector_mode_set(data, mode);
}

static Evas_Object *
_mode_option_create(Evas_Object *parent, Evas_Object *fs)
{
   Evas_Object *frame = NULL, *hbox = NULL, *rd = NULL, *rdg = NULL;

   frame = elm_frame_add(parent);
   evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, 0);
   elm_object_text_set(frame, "Mode Option");
   evas_object_show(frame);

   hbox = elm_box_add(frame);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_object_content_set(frame, hbox);
   evas_object_show(hbox);

   rdg = rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, ELM_FILESELECTOR_LIST);
   elm_object_text_set(rd, "List");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _mode_changed_cb, fs);

   rd = elm_radio_add(hbox);
   elm_radio_group_add(rd, rdg);
   elm_radio_state_value_set(rd, ELM_FILESELECTOR_GRID);
   elm_object_text_set(rd, "Grid");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _mode_changed_cb, fs);

   return frame;
}

static void
_sort_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs = evas_object_data_get(obj, "fileselector");
   const char *selected = elm_object_item_text_get(event_info);

   printf("selected sort method : %s\n", selected);
   elm_object_text_set(obj, selected);
   elm_fileselector_sort_method_set(fs, (Elm_Fileselector_Sort)data);
}

static Evas_Object *
_sort_option_create(Evas_Object *win, Evas_Object *parent, Evas_Object *fs)
{
   Evas_Object *frame = NULL, *hbox = NULL, *hoversel;

   frame = elm_frame_add(parent);
   evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, 0);
   elm_object_text_set(frame, "Sort Option");
   evas_object_show(frame);

   hbox = elm_box_add(frame);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_object_content_set(frame, hbox);
   evas_object_show(hbox);

   hoversel = elm_hoversel_add(hbox);
   elm_hoversel_hover_parent_set(hoversel, win);
   evas_object_data_set(hoversel, "fileselector", fs);
   elm_object_text_set(hoversel, "Choose sort method");

   elm_hoversel_item_add(hoversel, "File Name(asc)", NULL, ELM_ICON_NONE,
                         _sort_selected_cb,
                         (const void *)ELM_FILESELECTOR_SORT_BY_FILENAME_ASC);
   elm_hoversel_item_add(hoversel, "File Name(desc)", NULL, ELM_ICON_NONE,
                         _sort_selected_cb,
                         (const void *) ELM_FILESELECTOR_SORT_BY_FILENAME_DESC);
   elm_hoversel_item_add(hoversel, "Type(asc)", NULL, ELM_ICON_NONE,
                         _sort_selected_cb,
                         (const void *) ELM_FILESELECTOR_SORT_BY_TYPE_ASC);
   elm_hoversel_item_add(hoversel, "Type(desc)", NULL, ELM_ICON_NONE,
                         _sort_selected_cb,
                         (const void *) ELM_FILESELECTOR_SORT_BY_TYPE_DESC);
   elm_hoversel_item_add(hoversel, "Size(asc)", NULL, ELM_ICON_NONE,
                         _sort_selected_cb,
                         (const void *) ELM_FILESELECTOR_SORT_BY_SIZE_ASC);
   elm_hoversel_item_add(hoversel, "Size(desc)", NULL, ELM_ICON_NONE,
                         _sort_selected_cb,
                         (const void *) ELM_FILESELECTOR_SORT_BY_SIZE_DESC);
   elm_hoversel_item_add(hoversel, "Modified time(asc)", NULL, ELM_ICON_NONE,
                         _sort_selected_cb,
                         (const void *) ELM_FILESELECTOR_SORT_BY_MODIFIED_ASC);
   elm_hoversel_item_add(hoversel, "Modified time(desc)", NULL, ELM_ICON_NONE,
                         _sort_selected_cb,
                         (const void *) ELM_FILESELECTOR_SORT_BY_MODIFIED_DESC);

   elm_box_pack_end(hbox, hoversel);
   evas_object_show(hoversel);

   return frame;
}

static void
_small_icon_clicked(void *data,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Evas_Coord w, h;

   elm_fileselector_thumbnail_size_get(fs, &w, &h);

   elm_fileselector_thumbnail_size_set(fs, 56, 56);
   printf("Thumbnail icon was changed from %dx%d to 56x56\n", w, h);
}

static void
_middle_icon_clicked(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Evas_Coord w, h;

   elm_fileselector_thumbnail_size_get(fs, &w, &h);

   elm_fileselector_thumbnail_size_set(fs, 78, 78);
   printf("Thumbnail icon was changed from %dx%d to 78x78\n", w, h);
}

static void
_big_icon_clicked(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Evas_Coord w, h;

   elm_fileselector_thumbnail_size_get(fs, &w, &h);

   elm_fileselector_thumbnail_size_set(fs, 131, 131);
   printf("Thumbnail icon was changed from %dx%d to 131x131\n", w, h);
}

static Evas_Object *
_thumbnail_size_option_create(Evas_Object *parent, Evas_Object *fs)
{
   Evas_Object *frame = NULL, *hbox = NULL, *bt = NULL;

   frame = elm_frame_add(parent);
   elm_object_text_set(frame, "Thumbnail Size");
   evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, 0);
   evas_object_show(frame);

   hbox = elm_box_add(frame);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_object_content_set(frame, hbox);
   evas_object_show(hbox);

   bt = elm_button_add(hbox);
   elm_object_text_set(bt, "Small icon");
   evas_object_smart_callback_add(bt, "clicked", _small_icon_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(hbox);
   elm_object_text_set(bt, "Middle icon");
   evas_object_smart_callback_add(bt, "clicked", _middle_icon_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(hbox);
   elm_object_text_set(bt, "Big icon");
   evas_object_smart_callback_add(bt, "clicked", _big_icon_clicked, fs);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   return frame;
}

static Eina_Bool
_all_filter(const char *path  EINA_UNUSED,
            Eina_Bool dir     EINA_UNUSED,
            void *data        EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_edje_filter(const char *path, Eina_Bool dir,
             void *data EINA_UNUSED)
{
   if (dir) return EINA_TRUE;

   if (eina_str_has_extension(path, ".edc") ||
       eina_str_has_extension(path, ".edj"))
     return EINA_TRUE;
   return EINA_FALSE;
}

void
test_fileselector(void *data       EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fs, *box, *vbox, *sep;

   /* Set the locale according to the system pref.
    * If you don't do so the file selector will order the files list in
    * a case sensitive manner
    */
   setlocale(LC_ALL, "");

   elm_need_ethumb();
   elm_need_efreet();

   win = elm_win_util_standard_add("fileselector", "File Selector");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   fs = elm_fileselector_add(box);
   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, fs);
   evas_object_show(fs);

   /* enable the fs file name entry */
   elm_fileselector_is_save_set(fs, EINA_TRUE);
   /* make the file list a tree with dir expandable in place */
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   /* start the fileselector in the home dir */
   elm_fileselector_path_set(fs, getenv("HOME"));

   /* add filesters */
   elm_fileselector_mime_types_filter_append(fs, "text/*", "Text Files");
   elm_fileselector_mime_types_filter_append(fs, "image/*", "Image Files");
   elm_fileselector_custom_filter_append(fs, _all_filter, NULL, "All Files");
   elm_fileselector_custom_filter_append(fs, _edje_filter, NULL, "Edje Files");

   evas_object_smart_callback_add(fs, "done", my_fileselector_done, win);
   evas_object_smart_callback_add(fs, "selected", my_fileselector_selected,
                                  win);
   evas_object_smart_callback_add(fs, "selected,invalid",
                                  my_fileselector_invalid, win);
   evas_object_smart_callback_add(fs, "activated", my_fileselector_activated,
                                  win);

   sep = elm_separator_add(box);
   elm_box_pack_end(box, sep);
   evas_object_show(sep);

   vbox = elm_box_add(box);
   elm_box_pack_end(box, vbox);
   evas_object_show(vbox);

   /* test options */
   elm_box_pack_end(vbox, _option_create(vbox, fs));
   elm_box_pack_end(vbox, _getter_option_create(vbox, fs));
   elm_box_pack_end(vbox, _mode_option_create(vbox, fs));
   elm_box_pack_end(vbox, _sort_option_create(win, vbox, fs));
   elm_box_pack_end(vbox, _thumbnail_size_option_create(vbox, fs));

   evas_object_resize(win, 550, 500);
   evas_object_show(win);
}
