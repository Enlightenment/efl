/*
 * gcc -o web_example web_example.c `pkg-config --cflags --libs elementary ewebkit` -D_GNU_SOURCE
 */
#include <Elementary.h>
#ifdef HAVE_ELEMENTARY_WEB
#include <EWebKit.h>
#endif
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__ __attribute__((unused))
#endif

typedef struct _Tab_Data Tab_Data;

typedef struct
{
   Evas_Object *win;
   Evas_Object *main_box;
   Evas_Object *pager;
   Evas_Object *url;
   Evas_Object *default_web;
   Evas_Object *tabs;
   Evas_Object *close_tab;
   Evas_Object *search_box;
   Evas_Object *search_entry;

   struct {
        Evas_Object *back;
        Evas_Object *fwd;
        Evas_Object *refresh;
   } nav;

   Tab_Data *current_tab;

   Eina_Bool exiting : 1;
} App_Data;

struct _Tab_Data
{
   Evas_Object *web;
   App_Data *app;
   Elm_Toolbar_Item *tab;
};

static Evas_Object * _web_create_window_cb(void *data, Evas_Object *obj, Eina_Bool js, const Elm_Web_Window_Features *wf);

static void
nav_button_update(App_Data *ad)
{
   Eina_Bool back, fwd;

   back = !elm_web_back_possible(ad->current_tab->web);
   fwd = !elm_web_forward_possible(ad->current_tab->web);

   elm_object_disabled_set(ad->nav.back, back);
   elm_object_disabled_set(ad->nav.fwd, fwd);
}

static void
tab_current_set(Tab_Data *td)
{
   const char *uri;

   if (td == td->app->current_tab)
     return;

   td->app->current_tab = td;

   uri = elm_web_uri_get(td->web);
   elm_object_text_set(td->app->url, uri);

   nav_button_update(td->app);
   elm_entry_icon_visible_set(td->app->url, EINA_TRUE);

   elm_pager_content_promote(td->app->pager, td->web);
}

static void
_tab_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Tab_Data *td = data;
   /* the first toolbar_item_append() calls the select callback before the item
    * is assigned, so we need a workaround for that. */
   if (!td->tab)
     td->tab = event_info;
   tab_current_set(td);
}

static void
_title_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Tab_Data *td = data;
   const char *title = event_info;
   char buf[20] = "";

   if (title)
     strncpy(buf, title, sizeof(buf) - 1);
   elm_toolbar_item_label_set(td->tab, buf);
}

static void
_uri_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Tab_Data *td = data;
   const char *uri = event_info;

   if (td != td->app->current_tab)
     return;

   nav_button_update(td->app);
   elm_object_text_set(td->app->url, uri);
}

static void
_web_free_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Tab_Data *td = data;

   if (td->tab)
     elm_toolbar_item_del(td->tab);

   free(td);
}

static void
_tb_item_del_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Tab_Data *td = data;
   if (!td->app->exiting && !elm_toolbar_selected_item_get(obj))
     {
        td->app->current_tab = NULL;
        elm_entry_icon_visible_set(td->app->url, EINA_FALSE);
        if (td->app->search_box)
          evas_object_del(td->app->search_box);
     }
   td->tab = NULL;
}

Tab_Data *
tab_add(App_Data *ad)
{
   Tab_Data *td;

   td = calloc(1, sizeof(Tab_Data));
   if (!td) return NULL;

   td->web = elm_web_add(ad->win);
   elm_web_window_create_hook_set(td->web, _web_create_window_cb, ad);
   elm_web_inwin_mode_set(td->web, EINA_TRUE);
   evas_object_size_hint_weight_set(td->web, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(td->web, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_pager_content_push(ad->pager, td->web);

   td->app = ad;
   td->tab = elm_toolbar_item_append(td->app->tabs, NULL, "New tab",
                                     _tab_clicked_cb, td);
   elm_toolbar_item_del_cb_set(td->tab, _tb_item_del_cb);

   evas_object_data_set(td->web, "tab_data", td);

   evas_object_smart_callback_add(td->web, "title,changed", _title_changed_cb,
                                  td);
   evas_object_smart_callback_add(td->web, "uri,changed", _uri_changed_cb, td);
   evas_object_event_callback_add(td->web, EVAS_CALLBACK_FREE, _web_free_cb,
                                  td);

   elm_toolbar_item_selected_set(td->tab, EINA_TRUE);
   return td;
}

static char *
uri_sanitize(const char *uri)
{
   char *fixed_uri;
   char *schema;
   char *tmp;

   if (!uri || !*uri) return NULL;

   tmp = strstr(uri, "://");
   if (!tmp || (tmp == uri) || (tmp > (uri + 15)))
     {
        char *new_uri = NULL;
        if (ecore_file_exists(uri))
          {
             schema = "file";
             new_uri = ecore_file_realpath(uri);
          }
        else
          schema = "http";

        if (asprintf(&fixed_uri, "%s://%s", schema, new_uri ? new_uri : uri) >
            0)
          {
             free(new_uri);
             return fixed_uri;
          }
        free(new_uri);
     }
   else
     return strdup(uri);

   return NULL;
}

static void
tab_uri_set(Tab_Data *td, const char *uri)
{
   char *sane_uri = uri_sanitize(uri);
   elm_web_uri_set(td->web, sane_uri);
   free(sane_uri);
}

static void
_url_activated_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   Tab_Data *td;
   const char *uri = eina_stringshare_ref(elm_object_text_get(obj));

   if (!ad->current_tab)
     td = tab_add(ad);
   else
     td = ad->current_tab;
   tab_uri_set(td, uri);
   eina_stringshare_del(uri);
}

static void
_nav_back_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;

   elm_web_back(ad->current_tab->web);
}

static void
_nav_refresh_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   const Evas_Modifier *mods = evas_key_modifier_get(evas_object_evas_get(obj));

   if (evas_key_modifier_is_set(mods, "Shift"))
     elm_web_reload_full(ad->current_tab->web);
   else
     elm_web_reload(ad->current_tab->web);
}

static void
_nav_fwd_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;

   elm_web_forward(ad->current_tab->web);
}

static void
_close_tab_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;

   if (!ad->current_tab)
     return;
   evas_object_del(ad->current_tab->web);
}

static void
_add_tab_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   tab_add(ad);
   elm_object_focus_set(ad->url, EINA_TRUE);
}

static Evas_Object *
_web_create_window_cb(void *data, Evas_Object *obj __UNUSED__, Eina_Bool js __UNUSED__, const Elm_Web_Window_Features *wf __UNUSED__)
{
   App_Data *ad = data;
   Tab_Data *td;

   td = tab_add(ad);
   return td->web;
}

static void
_win_del_request_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   ad->exiting = EINA_TRUE;
}

static void
_win_free_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

static void
_search_entry_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   const char *text;

   text = elm_object_text_get(obj);
   elm_web_text_search(ad->current_tab->web, text, EINA_FALSE, EINA_TRUE,
                       EINA_TRUE);
   elm_web_text_matches_unmark_all(ad->current_tab->web);
   elm_web_text_matches_mark(ad->current_tab->web, text, EINA_FALSE, EINA_TRUE,
                             0);
}

static void
_search_entry_activate_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   const char *text;

   text = elm_object_text_get(obj);
   elm_web_text_search(ad->current_tab->web, text, EINA_FALSE, EINA_TRUE,
                       EINA_TRUE);
}

static void
_search_next_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   const char *text;

   text = elm_object_text_get(ad->search_entry);
   elm_web_text_search(ad->current_tab->web, text, EINA_FALSE, EINA_TRUE,
                       EINA_TRUE);
}

static void
_search_prev_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   const char *text;

   text = elm_object_text_get(ad->search_entry);
   elm_web_text_search(ad->current_tab->web, text, EINA_FALSE, EINA_FALSE,
                       EINA_TRUE);
}

static void
_search_close_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   evas_object_del(ad->search_box);
}

static void
_search_box_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   App_Data *ad = data;
   ad->search_box = NULL;
   ad->search_entry = NULL;
}

static void
_win_search_trigger_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   App_Data *ad = data;
   Evas_Object *box, *box2, *entry, *btn, *ic;

   if (strcmp(ev->keyname, "f") ||
       !evas_key_modifier_is_set(ev->modifiers, "Control"))
     return;
   if (ad->search_box || !ad->current_tab)
     return;

   box = elm_box_add(ad->win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_after(ad->main_box, box, ad->url);
   evas_object_show(box);

   evas_object_event_callback_add(box, EVAS_CALLBACK_DEL, _search_box_del_cb,
                                  ad);

   entry = elm_entry_add(ad->win);
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, entry);
   evas_object_show(entry);

   evas_object_smart_callback_add(entry, "changed", _search_entry_changed_cb,
                                  ad);
   evas_object_smart_callback_add(entry, "activated", _search_entry_activate_cb,
                                  ad);

   box2 = elm_box_add(ad->win);
   elm_box_horizontal_set(box2, EINA_TRUE);
   elm_entry_end_set(entry, box2);

   btn = elm_button_add(ad->win);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   ic = elm_icon_add(ad->win);
   elm_icon_standard_set(ic, "arrow_up");
   elm_button_icon_set(btn, ic);

   evas_object_smart_callback_add(btn, "clicked", _search_prev_cb, ad);

   btn = elm_button_add(ad->win);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   ic = elm_icon_add(ad->win);
   elm_icon_standard_set(ic, "arrow_down");
   elm_button_icon_set(btn, ic);

   evas_object_smart_callback_add(btn, "clicked", _search_next_cb, ad);

   btn = elm_button_add(ad->win);
   elm_box_pack_end(box, btn);
   evas_object_show(btn);

   ic = elm_icon_add(ad->win);
   elm_icon_standard_set(ic, "close");
   elm_button_icon_set(btn, ic);

   evas_object_smart_callback_add(btn, "clicked", _search_close_cb, ad);

   ad->search_box = box;
   ad->search_entry = entry;

   elm_object_focus_set(entry, EINA_TRUE);
}

static void
default_content_set(Evas_Object *web)
{
   Evas_Object *view, *frame;
   const char contents[] = ""
      "<html>\n"
      "  <head>\n"
      "    <title>Nothing to see here, move along</title>\n"
      "  </head>\n"
      "  <body>\n"
      "    <a href=\"http://www.enlightenment.org\" target=\"_blank\">E</a>\n"
      "    <br />\n"
      "    <a href=\"http://www.google.com\" target=\"_blank\">Google</a>\n"
      "    <br />\n"
      "  </body>\n"
      "</html>\n";

#ifdef HAVE_ELEMENTARY_WEB
   view = elm_web_webkit_view_get(web);
   frame = ewk_view_frame_main_get(view);
   ewk_frame_contents_set(frame, contents, sizeof(contents) - 1, "text/html",
                          "UTF-8", NULL);
#endif
}

int
elm_main(int argc __UNUSED__, char *argv[] __UNUSED__)
{
   Evas_Object *win, *bg, *box, *box2, *btn, *ic, *url, *pager, *tabs, *web;
   Evas *e;
   Evas_Modifier_Mask ctrl_mask;
   App_Data *ad;

   if (!elm_need_web())
     return -1;

   ad = calloc(1, sizeof(App_Data));
   if (!ad) return -1;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "example-web", ELM_WIN_BASIC);
   elm_win_autodel_set(win, EINA_TRUE);

   e = evas_object_evas_get(win);
   ctrl_mask = evas_key_modifier_mask_get(e, "Control");
   if (!evas_object_key_grab(win, "f", ctrl_mask, 0, EINA_TRUE))
     fprintf(stderr, "Could not grab trigger for search dialog\n");

   evas_object_smart_callback_add(win, "delete,request", _win_del_request_cb,
                                  ad);
   evas_object_event_callback_add(win, EVAS_CALLBACK_KEY_DOWN,
                                  _win_search_trigger_cb, ad);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _win_free_cb, ad);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   url = elm_entry_add(win);
   elm_entry_single_line_set(url, EINA_TRUE);
   elm_entry_scrollable_set(url, EINA_TRUE);
   evas_object_size_hint_weight_set(url, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(url, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, url);
   evas_object_show(url);

   evas_object_smart_callback_add(url, "activated", _url_activated_cb, ad);

   box2 = elm_box_add(win);
   elm_box_horizontal_set(box2, EINA_TRUE);
   elm_entry_icon_set(url, box2);
   elm_entry_icon_visible_set(url, EINA_FALSE);

   btn = elm_button_add(win);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   ad->nav.back = btn;

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "arrow_left");
   elm_button_icon_set(btn, ic);

   evas_object_smart_callback_add(btn, "clicked", _nav_back_cb, ad);

   btn = elm_button_add(win);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   ad->nav.refresh = btn;

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "refresh");
   elm_button_icon_set(btn, ic);

   evas_object_smart_callback_add(btn, "clicked", _nav_refresh_cb, ad);

   btn = elm_button_add(win);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   ad->nav.fwd = btn;

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "arrow_right");
   elm_button_icon_set(btn, ic);

   evas_object_smart_callback_add(btn, "clicked", _nav_fwd_cb, ad);

   box2 = elm_box_add(win);
   elm_box_horizontal_set(box2, EINA_TRUE);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   btn = elm_button_add(win);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "file");
   elm_button_icon_set(btn, ic);

   evas_object_smart_callback_add(btn, "clicked", _add_tab_cb, ad);

   tabs = elm_toolbar_add(win);
   elm_toolbar_align_set(tabs, 0.0);
   elm_toolbar_always_select_mode_set(tabs, EINA_TRUE);
   elm_toolbar_homogeneous_set(tabs, EINA_FALSE);
   elm_toolbar_mode_shrink_set(tabs, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tabs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tabs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box2, tabs);
   evas_object_show(tabs);

   btn = elm_button_add(win);
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   evas_object_smart_callback_add(btn, "clicked", _close_tab_cb, ad);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "close");
   elm_button_icon_set(btn, ic);

   pager = elm_pager_add(win);
   evas_object_size_hint_weight_set(pager, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pager, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, pager);
   evas_object_show(pager);

   elm_toolbar_menu_parent_set(tabs, pager);

   web = elm_web_add(win);
   elm_web_window_create_hook_set(web, _web_create_window_cb, ad);
   elm_web_history_enable_set(web, EINA_FALSE);
   evas_object_size_hint_weight_set(web, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(web, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_pager_content_push(pager, web);

   default_content_set(web);

   ad->win = win;
   ad->main_box = box;
   ad->pager = pager;
   ad->url = url;
   ad->default_web = web;
   ad->tabs = tabs;
   ad->close_tab = btn;

   evas_object_resize(win, 480, 640);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN();
