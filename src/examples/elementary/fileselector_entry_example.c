/**
 * Simple Elementary's <b>file selector entry widget</b> example,
 * illustrating its usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g fileselector_entry_example.c -o fileselector_entry_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

/* hook on the file,chosen smart callback */
static void
_file_chosen(void        *data,
             Evas_Object *obj,
             void        *event_info)
{
   const char *file = event_info;

   if (file)
     printf("File chosen: %s\n", file);
   else
     printf("File selection canceled.\n");
}

/* toggle inwin mode */
static void
_inwin_mode_toggle(void        *data,
                   Evas_Object *obj,
                   void        *event_info)
{
   Evas_Object *fs_entry = data;
   Eina_Bool old_val = elm_fileselector_entry_inwin_mode_get(fs_entry);

   elm_fileselector_entry_inwin_mode_set(fs_entry, !old_val);
   printf("Inwin mode set to: %s\n", old_val ? "false" : "true");
}

static void
_current_sel_toggle(void        *data,
                    Evas_Object *obj,
                    void        *event_info)
{
   Evas_Object *fs_entry = data;
   Eina_Bool old_val = elm_fileselector_is_save_get(fs_entry);
   elm_fileselector_is_save_set(fs_entry, !old_val);

   printf("%s text entry with selected item's name\n",
          old_val ? "Disabling" : "Enabling");
}

static void
_folder_only_toggle(void        *data,
                    Evas_Object *obj,
                    void        *event_info)
{
   Evas_Object *fs_entry = data;
   Eina_Bool old_val = elm_fileselector_folder_only_get(fs_entry);
   elm_fileselector_folder_only_set(fs_entry, !old_val);

   printf("Folder only mode set to: %s\n", old_val ? "false" : "true");
}

static void
_expandable_toggle(void        *data,
                   Evas_Object *obj,
                   void        *event_info)
{
   Evas_Object *fs_entry = data;
   Eina_Bool old_val = elm_fileselector_expandable_get(fs_entry);
   elm_fileselector_expandable_set(fs_entry, !old_val);

   printf("Expandable folders mode set to: %s\n", old_val ? "false" : "true");
}

EAPI_MAIN int
elm_main(int    argc,
         char **argv)
{
   Evas_Object *win, *vbox, *hbox, *ic, *ck, *fs_entry, *sep;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("fileselector-entry", "File Selector Entry Example");
   elm_win_autodel_set(win, EINA_TRUE);

   vbox = elm_box_add(win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, vbox);
   evas_object_show(vbox);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "file");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   /* file selector entry */
   fs_entry = elm_fileselector_entry_add(win);
   evas_object_size_hint_align_set(fs_entry, EVAS_HINT_FILL, 0);
   elm_fileselector_path_set(fs_entry, "/tmp");
   elm_object_text_set(fs_entry, "Select a file");
   elm_object_part_content_set(fs_entry, "button icon", ic);

   elm_box_pack_end(vbox, fs_entry);
   evas_object_show(fs_entry);
   evas_object_show(ic);

   /* attribute setting knobs */
   sep = elm_separator_add(win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(vbox, sep);
   evas_object_show(sep);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "editable selection");
   elm_check_state_set(ck, elm_fileselector_is_save_get(fs_entry));
   evas_object_smart_callback_add(ck, "changed", _current_sel_toggle, fs_entry);
   elm_box_pack_end(hbox, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "\"inwin\" mode");
   elm_check_state_set(ck, elm_fileselector_entry_inwin_mode_get(fs_entry));
   evas_object_smart_callback_add(ck, "changed", _inwin_mode_toggle, fs_entry);
   elm_box_pack_end(hbox, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "folders only");
   elm_check_state_set(ck, elm_fileselector_folder_only_get(fs_entry));
   evas_object_smart_callback_add(ck, "changed", _folder_only_toggle, fs_entry);
   elm_box_pack_end(hbox, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "expandable");
   elm_check_state_set(ck, elm_fileselector_expandable_get(fs_entry));
   evas_object_smart_callback_add(ck, "changed", _expandable_toggle, fs_entry);
   elm_box_pack_end(hbox, ck);
   evas_object_show(ck);

   evas_object_smart_callback_add(fs_entry, "file,chosen", _file_chosen, NULL);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
