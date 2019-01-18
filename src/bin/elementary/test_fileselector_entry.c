#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   void *box;           /* Use this to get box content     */
};
typedef struct _api_data api_data;

enum _api_state
{
   ICON_UNSET,
   WINDOW_TITLE_SET,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   const Eina_List *items = elm_box_children_get(api->box);
   if (!eina_list_count(items))
     return;

   /* Get first item of list of vbox children */
   Evas_Object *fs_bt = eina_list_nth(items, 0);

   /* use elm_box_children_get() to get list of children */
   switch(api->state)
     { /* Put all api-changes under switch */
      case ICON_UNSET:
         elm_object_part_content_unset(fs_bt, NULL);
         break;

      case WINDOW_TITLE_SET:
         elm_fileselector_entry_window_title_set(fs_bt, "Custom title from API");
         break;

      case API_STATE_LAST:

         break;
      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{  /* Will add here a SWITCH command containing code to modify test-object */
   /* in accordance a->state value. */
   api_data *a = data;
   char str[128];

   printf("clicked event on API Button: api_state=<%d>\n", a->state);
   set_api_state(a);
   a->state++;
   sprintf(str, "Next API function (%u)", a->state);
   elm_object_text_set(obj, str);
   elm_object_disabled_set(obj, a->state == API_STATE_LAST);
}

static void
_create_dir_struct(void)
{
   FILE *fp;
   int ret = 0;

   ret = mkdir("/tmp/test_fs_en", S_IRWXU);
   if (ret < 0) return;
   fp = fopen("/tmp/test_fs_en/a_file.txt", "w");
   if (fp) fclose(fp);
   fp = fopen("/tmp/test_fs_en/k_file.txt", "w");
   if (fp) fclose(fp);
   fp = fopen("/tmp/test_fs_en/m_file.txt", "w");
   if (fp) fclose(fp);

   ret = mkdir("/tmp/test_fs_en/a_subdir", S_IRWXU);
   if (ret < 0) return;
   fp = fopen("/tmp/test_fs_en/a_subdir/d_sub_file.txt", "w");
   if (fp) fclose(fp);
   fp = fopen("/tmp/test_fs_en/a_subdir/j_sub_file.txt", "w");
   if (fp) fclose(fp);
}

static void
_file_chosen(void            *data,
             Evas_Object *obj EINA_UNUSED,
             void            *event_info)
{
   Evas_Object *entry = data;
   char *file = event_info;
   elm_object_text_set(entry, file);
   printf("File chosen: %s\n", file);
}

static void
_inwin_mode_toggle(void            *data,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Evas_Object *fs_en = data;
   Eina_Bool value = elm_fileselector_entry_inwin_mode_get(fs_en);
   elm_fileselector_entry_inwin_mode_set(fs_en, !value);
   printf("Inwin mode set to: %s\n", value ? "false" : "true");
}

static void
_folder_only_toggle(void            *data,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *fs_en = data;
   Evas_Object *ic = elm_object_part_content_get(fs_en, "button icon");
   Eina_Bool value = elm_fileselector_folder_only_get(fs_en);
   elm_fileselector_folder_only_set(fs_en, !value);
   printf("Folder only flag set to: %s\n", value ? "false" : "true");
   if (!value)
     {
        if (ic) elm_icon_standard_set(ic, "folder");
        elm_object_text_set(fs_en, "Select a folder");
     }
   else
     {
        if (ic) elm_icon_standard_set(ic, "file");
        elm_object_text_set(fs_en, "Select a file");
     }
}

static void
_expandable_toggle(void            *data,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Evas_Object *fs_en = data;
   Eina_Bool value = elm_fileselector_expandable_get(fs_en);
   elm_fileselector_expandable_set(fs_en, !value);
   printf("Expandable flag set to: %s\n", value ? "false" : "true");
}

static void
_disabled_toggle(void            *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Evas_Object *fs_en = data;
   Eina_Bool value = elm_object_disabled_get(fs_en);
   elm_object_disabled_set(fs_en, !value);
   printf("Disabled flag set to: %s\n", value ? "false" : "true");
}

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   free(data);
}

void
test_fileselector_entry(void *data       EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Evas_Object *win, *vbox, *hbox, *ic, *bt, *fs_en, *en, *lb, *bxx;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("fileselector-entry", "File Selector Entry");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   vbox = elm_box_add(win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   api->box = vbox;
   evas_object_show(vbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   elm_box_pack_end(bxx, vbox);

   _create_dir_struct(); /* Create a dir struct in /tmp */

   /* file selector entry */
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "file");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   fs_en = elm_fileselector_entry_add(win);
   elm_fileselector_path_set(fs_en, "/tmp/test_fs_en");
   elm_object_text_set(fs_en, "Select a file");
   elm_object_part_content_set(fs_en, "button icon", ic);
   evas_object_size_hint_weight_set(fs_en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fs_en, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(vbox, fs_en);
   evas_object_show(fs_en);
   evas_object_show(ic);

   /* attribute setting buttons */
   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Toggle inwin mode");
   evas_object_smart_callback_add(bt, "clicked", _inwin_mode_toggle, fs_en);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Toggle folder only mode");
   evas_object_smart_callback_add(bt, "clicked", _folder_only_toggle, fs_en);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Toggle expandable mode");
   evas_object_smart_callback_add(bt, "clicked", _expandable_toggle, fs_en);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Last selection:");
   elm_box_pack_end(vbox, lb);
   evas_object_show(lb);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Toggle disabled");
   evas_object_smart_callback_add(bt, "clicked", _disabled_toggle, fs_en);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   en = elm_entry_add(win);
   elm_entry_line_wrap_set(en, ELM_WRAP_NONE);
   elm_entry_editable_set(en, EINA_FALSE);
   evas_object_smart_callback_add(fs_en, "file,chosen", _file_chosen, en);
   elm_box_pack_end(vbox, en);
   evas_object_show(en);

   evas_object_resize(win, 400, 500);
   evas_object_show(win);
}
