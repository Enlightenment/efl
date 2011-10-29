/*
 * gcc -o entry_example entry_example.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__ __attribute__((unused))
#endif

typedef struct
{
   Evas_Object *win;
   Evas_Object *edit_buffer;
} App_Data;

typedef struct
{
   Evas_Object *inwin;
   Evas_Object *pager;
   Evas_Object *grid;
   Evas_Object *settings;

   int size;
   int vsize;
   int width, height;

   const char *emo;

   App_Data *ad;
} App_Inwin_Data;

static void
_edit_buffer_insert(Evas_Object *e, const char *text)
{
   elm_entry_entry_insert(e, text);
   elm_object_focus(e);
}

static void
_it_sel_cb(void *data, Evas_Object *obj __UNUSED__, void *event)
{
   App_Inwin_Data *aid = data;
   Elm_Gengrid_Item *it = event;

   aid->emo = elm_gengrid_item_data_get(it);

   elm_pager_content_promote(aid->pager, aid->settings);
}

static char *
_emo_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return strdup(data);
}

static Evas_Object *
_emo_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *o;

   if (strcmp(part, "elm.swallow.icon"))
     return NULL;
   o = elm_layout_add(obj);
   elm_layout_theme_set(o, "entry/emoticon", data, "default");
   return o;
}

static void
_emo_del(void *data, Evas_Object *obj __UNUSED__)
{
   free(data);
}

static Evas_Object *
_page_grid_add(Evas_Object *parent, App_Inwin_Data *aid)
{
   Evas_Object *grid;
   char *theme, *emo;
   Eina_List *emos, *it;
   static Elm_Gengrid_Item_Class it_class = {
        "default",
        {
           _emo_label_get,
           _emo_content_get,
           NULL,
           _emo_del
        }
   };

   theme = elm_theme_list_item_path_get("default", NULL);
   if (!theme) return NULL;
   emos = edje_file_collection_list(theme);
   free(theme);
   if (!emos) return NULL;

   grid = elm_gengrid_add(parent);
   elm_gengrid_item_size_set(grid, 64, 80);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);

   EINA_LIST_FOREACH(emos, it, emo)
     {
        char name[512];

        if (strncmp(emo, "elm/entry/emoticon/", 19))
          continue;
        sscanf(emo, "elm/entry/emoticon/%[^/]/default", name);
        elm_gengrid_item_append(grid, &it_class, strdup(name), _it_sel_cb, aid);
     }
   edje_file_collection_list_free(emos);

   return grid;
}

static void
_btn_insert_cb(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   App_Inwin_Data *aid = data;
   const char *size[] = {
        "size",
        "absize",
        "relsize"
   };
   const char *vsize[] = {
        "full",
        "ascent"
   };
   char buf[512];

   snprintf(buf, sizeof(buf), "<item %s=%dx%d vsize=%s href=emoticon/%s>"
            "</item>", size[aid->size], aid->width, aid->height,
            vsize[aid->vsize], aid->emo);
   _edit_buffer_insert(aid->ad->edit_buffer, buf);

   evas_object_del(aid->inwin);
}

static void
_width_changed_cb(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   App_Inwin_Data *aid = data;

   aid->width = atoi(elm_object_text_get(obj));
}

static void
_height_changed_cb(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   App_Inwin_Data *aid = data;

   aid->height = atoi(elm_object_text_get(obj));
}

static Evas_Object *
_page_settings_add(Evas_Object *parent, App_Inwin_Data *aid)
{
   Evas_Object *box, *sizeopts, *box2, *sizebox, *vsizebox,
               *rsize, *rabsize, *rrelsize, *rvfull, *rvascent,
               *fwidth, *ewidth, *fheight, *eheight,
               *binsert;
   char buf[100];
   static Elm_Entry_Filter_Accept_Set accept_set = {
        .accepted = "0123456789",
        .rejected = NULL
   };
   static Elm_Entry_Filter_Limit_Size limit_size = {
        .max_char_count = 5,
        .max_byte_count = 0
   };

   box = elm_box_add(parent);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

   sizeopts = elm_frame_add(parent);
   elm_object_text_set(sizeopts, "Size");
   evas_object_size_hint_weight_set(sizeopts, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sizeopts, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, sizeopts);
   evas_object_show(sizeopts);

   box2 = elm_box_add(parent);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(sizeopts, box2);
   evas_object_show(box2);

   sizebox = elm_box_add(parent);
   elm_box_horizontal_set(sizebox, EINA_TRUE);
   evas_object_size_hint_weight_set(sizebox, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sizebox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box2, sizebox);
   evas_object_show(sizebox);

   rsize = elm_radio_add(parent);
   elm_object_text_set(rsize, "Scale adjusted (size)");
   elm_radio_state_value_set(rsize, 0);
   elm_radio_value_pointer_set(rsize, &aid->size);
   elm_box_pack_end(sizebox, rsize);
   evas_object_show(rsize);

   rabsize = elm_radio_add(parent);
   elm_object_text_set(rabsize, "Absolute size (absize)");
   elm_radio_state_value_set(rabsize, 1);
   elm_radio_value_pointer_set(rabsize, &aid->size);
   elm_radio_group_add(rabsize, rsize);
   elm_box_pack_end(sizebox, rabsize);
   evas_object_show(rabsize);

   rrelsize = elm_radio_add(parent);
   elm_object_text_set(rrelsize, "Relative to line (relsize)");
   elm_radio_state_value_set(rrelsize, 2);
   elm_radio_value_pointer_set(rrelsize, &aid->size);
   elm_radio_group_add(rrelsize, rsize);
   elm_box_pack_end(sizebox, rrelsize);
   evas_object_show(rrelsize);

   vsizebox = elm_box_add(parent);
   elm_box_horizontal_set(vsizebox, EINA_TRUE);
   evas_object_size_hint_weight_set(vsizebox, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(vsizebox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box2, vsizebox);
   evas_object_show(vsizebox);

   rvfull = elm_radio_add(parent);
   elm_object_text_set(rvfull, "Full height (vsize=full)");
   elm_radio_state_value_set(rvfull, 0);
   elm_radio_value_pointer_set(rvfull, &aid->vsize);
   elm_box_pack_end(vsizebox, rvfull);
   evas_object_show(rvfull);

   rvascent = elm_radio_add(parent);
   elm_object_text_set(rvascent, "Ascent only (vsize=ascent)");
   elm_radio_state_value_set(rvascent, 1);
   elm_radio_value_pointer_set(rvascent, &aid->vsize);
   elm_radio_group_add(rvascent, rvfull);
   elm_box_pack_end(vsizebox, rvascent);
   evas_object_show(rvascent);

   fwidth = elm_frame_add(parent);
   elm_object_text_set(fwidth, "Width");
   evas_object_size_hint_weight_set(fwidth, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fwidth, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box2, fwidth);
   evas_object_show(fwidth);

   snprintf(buf, sizeof(buf), "%d", aid->width);
   ewidth = elm_entry_add(parent);
   elm_entry_single_line_set(ewidth, EINA_TRUE);
   elm_entry_text_filter_append(ewidth, elm_entry_filter_accept_set,
                                &accept_set);
   elm_entry_text_filter_append(ewidth, elm_entry_filter_limit_size,
                                &limit_size);
   elm_object_text_set(ewidth, buf);
   evas_object_size_hint_weight_set(ewidth, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(ewidth, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fwidth, ewidth);
   evas_object_show(ewidth);

   evas_object_smart_callback_add(ewidth, "changed", _width_changed_cb, aid);

   fheight = elm_frame_add(parent);
   elm_object_text_set(fheight, "Height");
   evas_object_size_hint_weight_set(fheight, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fheight, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box2, fheight);
   evas_object_show(fheight);

   snprintf(buf, sizeof(buf), "%d", aid->height);
   eheight = elm_entry_add(parent);
   elm_entry_single_line_set(eheight, EINA_TRUE);
   elm_entry_text_filter_append(eheight, elm_entry_filter_accept_set,
                                &accept_set);
   elm_entry_text_filter_append(eheight, elm_entry_filter_limit_size,
                                &limit_size);
   elm_object_text_set(eheight, buf);
   evas_object_size_hint_weight_set(eheight, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(eheight, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fheight, eheight);
   evas_object_show(eheight);

   evas_object_smart_callback_add(eheight, "changed", _height_changed_cb, aid);

   binsert = elm_button_add(parent);
   elm_object_text_set(binsert, "Insert");
   elm_box_pack_end(box, binsert);
   evas_object_show(binsert);

   evas_object_smart_callback_add(binsert, "clicked", _btn_insert_cb, aid);

   return box;
}

static void
_insert_cancel_cb(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   App_Inwin_Data *aid = data;

   evas_object_del(aid->inwin);
}

static void
_inwin_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   free(data);
}

static void
_image_insert_cb(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   App_Data *ad = data;
   App_Inwin_Data *aid;
   Evas_Object *inwin, *box, *box2, *pager, *o;

   aid = calloc(1, sizeof(App_Inwin_Data));
   if (!aid) return;

   aid->ad = ad;
   aid->size = 1;
   aid->vsize = 1;
   aid->width = 64;
   aid->height = 64;

   inwin = elm_win_inwin_add(ad->win);
   evas_object_size_hint_weight_set(inwin, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(inwin, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(inwin);

   evas_object_event_callback_add(inwin, EVAS_CALLBACK_DEL, _inwin_del_cb,
                                  aid);

   box = elm_box_add(ad->win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_inwin_content_set(inwin, box);
   evas_object_show(box);

   pager = elm_pager_add(ad->win);
   evas_object_size_hint_weight_set(pager, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pager, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, pager);
   evas_object_show(pager);

   o = _page_grid_add(ad->win, aid);
   elm_pager_content_push(pager, o);
   aid->grid = o;

   o = _page_settings_add(ad->win, aid);
   elm_pager_content_push(pager, o);
   aid->settings = o;

   elm_pager_content_promote(pager, aid->grid);

   box2 = elm_box_add(ad->win);
   elm_box_horizontal_set(box2, EINA_TRUE);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   o = elm_button_add(ad->win);
   elm_object_text_set(o, "Cancel");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box2, o);
   evas_object_show(o);

   evas_object_smart_callback_add(o, "clicked", _insert_cancel_cb, aid);

   aid->inwin = inwin;
   aid->pager = pager;
}

static void
_format_change_cb(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   App_Data *ad = data;
   char fmt_open[5], fmt_close[6];
   const char *sel;
   int cursor;
   char *s;
   const char *ptr, *what;
   Eina_Strbuf *buf = NULL;

   sel = elm_object_text_get(obj);
   snprintf(fmt_open, sizeof(fmt_open), "<%s>", sel);
   snprintf(fmt_close, sizeof(fmt_close), "</%s>", sel);

   buf = eina_strbuf_new();

   cursor = elm_entry_cursor_pos_get(ad->edit_buffer);
   elm_entry_cursor_begin_set(ad->edit_buffer);
   elm_entry_cursor_selection_begin(ad->edit_buffer);
   elm_entry_cursor_pos_set(ad->edit_buffer, cursor);
   elm_entry_cursor_selection_end(ad->edit_buffer);
   sel = elm_entry_selection_get(ad->edit_buffer);
   if (!sel || !sel[0])
     {
        eina_strbuf_append(buf, fmt_open);
        goto all_done;
     }
   what = fmt_open;
   ptr = sel;
   while ((s = strstr(ptr, what)))
     {
        ptr = s + strlen(what);
        if (what == fmt_open) what = fmt_close;
        else what = fmt_open;
     }
   eina_strbuf_append(buf, what);
   if (what == fmt_close)
     {
        elm_entry_cursor_selection_begin(ad->edit_buffer);
        elm_entry_cursor_end_set(ad->edit_buffer);
        elm_entry_cursor_selection_end(ad->edit_buffer);
        sel = elm_entry_selection_get(ad->edit_buffer);
        if (sel)
          {
             Eina_Strbuf *buf2 = eina_strbuf_new();
             eina_strbuf_append(buf2, sel);
             eina_strbuf_replace_first(buf2, fmt_close, "");
             elm_entry_entry_insert(ad->edit_buffer,
                                    eina_strbuf_string_get(buf2));
             eina_strbuf_free(buf2);
          }
     }
   elm_entry_select_none(ad->edit_buffer);
   elm_entry_cursor_pos_set(ad->edit_buffer, cursor);

all_done:
   _edit_buffer_insert(ad->edit_buffer, eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

static void
_autosave_change_cb(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   App_Data *ad = data;
   Eina_Bool state = elm_check_state_get(obj);

   elm_entry_autosave_set(ad->edit_buffer, state);
   if (state) elm_entry_file_save(ad->edit_buffer);
}

static void
_edit_dblclick_cb(void *data __UNUSED__, Evas_Object *obj, void *event __UNUSED__)
{
   int current_cursor;

   current_cursor = elm_entry_cursor_pos_get(obj);

   while (elm_entry_cursor_prev(obj))
     {
        const char *content;
        if (elm_entry_cursor_is_visible_format_get(obj))
          break;
        content = elm_entry_cursor_content_get(obj);
        if (content && (content[0] == ' '))
          break;
     }

   if (current_cursor == elm_entry_cursor_pos_get(obj))
     return;

   elm_entry_cursor_next(obj);
   elm_entry_cursor_selection_begin(obj);
   elm_entry_cursor_pos_set(obj, current_cursor);
   while (elm_entry_cursor_next(obj))
     {
        const char *content;
        if (elm_entry_cursor_is_visible_format_get(obj))
          break;
        content = elm_entry_cursor_content_get(obj);
        if (content && (content[0] == ' '))
          break;
     }
   elm_entry_cursor_selection_end(obj);
}

static void
_edit_tplclick_cb(void *data __UNUSED__, Evas_Object *obj, void *event __UNUSED__)
{
   elm_entry_cursor_line_begin_set(obj);
   elm_entry_cursor_selection_begin(obj);
   elm_entry_cursor_line_end_set(obj);
   elm_entry_cursor_selection_end(obj);
}

static void
_win_del_cb(void *data __UNUSED__, Evas_Object *obj, void *event __UNUSED__)
{
   evas_object_del(obj);
   elm_exit();
}

int
elm_main(int argc __UNUSED__, char *argv[] __UNUSED__)
{
   Evas_Object *win, *bg, *box, *tb, *en, *o, *icon;
   App_Data app;

   memset(&app, 0, sizeof(app));

   win = elm_win_add(NULL, "entry-example", ELM_WIN_BASIC);
   elm_win_title_set(win, "Emacs Lite");
   evas_object_show(win);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   tb = elm_box_add(win);
   elm_box_horizontal_set(tb, EINA_TRUE);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, tb);
   evas_object_show(tb);

   o = elm_check_add(win);
   elm_check_state_set(o, EINA_FALSE);
   elm_object_text_set(o, "Autosave");
   elm_box_pack_end(tb, o);
   evas_object_show(o);

   evas_object_smart_callback_add(o, "changed", _autosave_change_cb, &app);

   o = elm_button_add(win);
   elm_object_text_set(o, "b");
   elm_box_pack_end(tb, o);
   evas_object_show(o);

   evas_object_smart_callback_add(o, "clicked", _format_change_cb, &app);

   o = elm_button_add(win);
   elm_object_text_set(o, "em");
   elm_box_pack_end(tb, o);
   evas_object_show(o);

   evas_object_smart_callback_add(o, "clicked", _format_change_cb, &app);

   o = elm_button_add(win);
   elm_box_pack_end(tb, o);
   evas_object_show(o);

   icon = elm_layout_add(win);
   elm_layout_theme_set(icon, "entry", "emoticon/haha", "default");
   elm_object_content_set(o, icon);

   evas_object_smart_callback_add(o, "clicked", _image_insert_cb, &app);

   en = elm_entry_add(win);
   elm_entry_autosave_set(en, EINA_FALSE);
   elm_entry_file_set(en, "/tmp/da_test.txt", ELM_TEXT_FORMAT_MARKUP_UTF8);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, en);
   evas_object_show(en);

   app.win = win;
   app.edit_buffer = en;

   evas_object_smart_callback_add(en, "clicked,double", _edit_dblclick_cb,
                                  NULL);
   evas_object_smart_callback_add(en, "clicked,triple", _edit_tplclick_cb,
                                  NULL);

   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, &app);

   elm_object_focus(app.edit_buffer);

   evas_object_resize(win, 300, 780);

   elm_run();

   return 0;
}
ELM_MAIN();
