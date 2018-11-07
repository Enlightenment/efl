#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#ifdef MAX_PROFILES
# undef MAX_PROFILES
#endif

#define MAX_PROFILES 20

#ifdef LOG
# undef LOG
#endif

#define LOG(m)                                                               \
   do {                                                                      \
        lb = elm_label_add(win);                                             \
        elm_object_text_set(lb, m);                                          \
        evas_object_size_hint_weight_set(lb, 0.0, 0.0);                      \
        evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL); \
        evas_object_show(lb);                                                \
   } while(0)

typedef struct _Prof_Data Prof_Data;
typedef struct _App_Data  App_Data;

struct _Prof_Data
{
   Evas_Object *rdg;
   Eina_List   *cks;
   const char  *profile;
   const char  *available_profiles[MAX_PROFILES];
   int          count;
};

struct _App_Data
{
   Evas_Object *win;
   Eina_List   *profiles;
   Prof_Data    curr;
   Prof_Data    new;
};

void test_config(void *data, Evas_Object *obj, void *event_info);

static void
_profile_clear(Prof_Data *pd)
{
   if (pd->profile)
     eina_stringshare_del(pd->profile);
   pd->profile = NULL;
}

static void
_profiles_clear(Prof_Data *pd)
{
   int i;
   for (i = 0; i < MAX_PROFILES; i++)
     {
        if (pd->available_profiles[i])
          eina_stringshare_del(pd->available_profiles[i]);
        pd->available_profiles[i] = NULL;
     }
}

static void
_profile_update(Evas_Object *win)
{
   Evas_Object *lb = evas_object_data_get(win, "lb");
   char **profiles = NULL;
   const char *profile;
   unsigned int i, n = 0;
   char buf[PATH_MAX];

   profile = elm_win_profile_get(win);
   snprintf(buf, sizeof(buf),
            "Profile: <b>%s</b><br/>"
            "Available profiles: <b>",
            profile);

   elm_win_available_profiles_get(win, &profiles, &n);
   if ((profiles) && (n > 0))
     {
        for (i = 0; i < n; i++)
          {
             if (strlen(buf) >= (sizeof(buf) - 3)) break;
             if (i >= 1) strcat(buf, ", ");
             if (strlen(buf) >= (sizeof(buf) - 1 - strlen(profiles[i]))) break;
             strcat(buf, profiles[i]);
          }
        if (strlen(buf) < (sizeof(buf) - 5)) strcat(buf, "</b>");
     }
   elm_object_text_set(lb, buf);
}

static void
_bt_profile_set(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = evas_object_data_get((Evas_Object *)data, "ad");
   Evas_Object *rd = elm_radio_selected_object_get(ad->curr.rdg);
   const char *profile = elm_object_text_get(rd);
   if (strcmp(profile, "Nothing") != 0)
     elm_win_profile_set(ad->win, elm_object_text_get(rd));
   else
     elm_win_profile_set(ad->win, NULL);
   _profile_update(ad->win);
}

static void
_bt_available_profiles_set(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = evas_object_data_get((Evas_Object *)data, "ad");
   Eina_List *l = NULL;
   const char *str;
   Evas_Object *o;
   int i = 0;

   _profiles_clear(&ad->curr);

   EINA_LIST_FOREACH(ad->curr.cks, l, o)
     {
        if (elm_check_state_get(o))
          {
             str = evas_object_data_get(o, "profile");
             if (str)
               {
                  ad->curr.available_profiles[i] = eina_stringshare_add(str);
                  i++;
               }
          }
     }
   ad->curr.count = i;

   elm_win_available_profiles_set(ad->win,
                                  ad->curr.available_profiles,
                                  ad->curr.count);
   _profile_update(ad->win);
}

static void
_bt_win_add(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = evas_object_data_get((Evas_Object *)data, "ad");
   Evas_Object *rd = elm_radio_selected_object_get(ad->new.rdg);
   const char *profile = elm_object_text_get(rd);
   const char *str;
   Eina_List *l = NULL;
   Evas_Object *o;
   int i = 0;

   _profile_clear(&ad->new);
   _profiles_clear(&ad->new);

   if (strcmp(profile, "Nothing") != 0)
     ad->new.profile = (char *)eina_stringshare_add(profile);

   EINA_LIST_FOREACH(ad->new.cks, l, o)
     {
        if (elm_check_state_get(o))
          {
             str = evas_object_data_get(o, "profile");
             if (str)
               {
                  ad->new.available_profiles[i] = eina_stringshare_add(str);
                  i++;
               }
          }
     }
   ad->new.count = i;

   test_config(&(ad->new), NULL, NULL);
}

static void
_win_profile_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   _profile_update(obj);
}

static void
_win_del_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = evas_object_data_get(obj, "ad");
   Evas_Object *o;
   char *str;

   elm_config_profile_list_free(ad->profiles);
   ad->profiles = NULL;

   EINA_LIST_FREE(ad->curr.cks, o)
     {
        str = evas_object_data_del(o, "profile");
        if (str) eina_stringshare_del(str);
     }

   EINA_LIST_FREE(ad->new.cks, o)
     {
        str = evas_object_data_del(o, "profile");
        if (str) eina_stringshare_del(str);
     }

   _profile_clear(&ad->curr);
   _profiles_clear(&ad->curr);
   _profile_clear(&ad->new);
   _profiles_clear(&ad->new);

   free(ad);
}

static Evas_Object *
_radio_add(Evas_Object *win, Evas_Object *bx)
{
   App_Data *ad = evas_object_data_get(win, "ad");
   Evas_Object *bx2, *rd, *rdg = NULL;
   Eina_List *l = NULL;
   const char *str;
   int i = 0;

   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.0);
   elm_box_align_set(bx2, 0.0, 0.5);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   rdg = rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, i);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Nothing");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);
   i++;

   EINA_LIST_FOREACH(ad->profiles, l, str)
     {
        rd = elm_radio_add(win);
        elm_radio_state_value_set(rd, i);
        elm_radio_group_add(rd, rdg);
        elm_object_text_set(rd, str);
        evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_box_pack_end(bx2, rd);
        evas_object_show(rd);
        i++;
     }

   return rdg;
}

static Eina_List *
_check_add(Evas_Object *win, Evas_Object *bx)
{
   App_Data *ad = evas_object_data_get(win, "ad");
   Evas_Object *bx2, *ck;
   Eina_List *l = NULL, *ll = NULL;
   const char *str;

   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.0);
   elm_box_align_set(bx2, 0.0, 0.5);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   EINA_LIST_FOREACH(ad->profiles, l, str)
     {
        ck = elm_check_add(win);
        elm_object_text_set(ck, str);
        evas_object_data_set(ck, "profile", eina_stringshare_add(str));
        evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_box_pack_end(bx2, ck);
        evas_object_show(ck);

        ll = eina_list_append(ll, ck);
     }

   return ll;
}

static Evas_Object *
_inlined_add(Evas_Object *parent)
{
   Evas_Object *win, *bg, *bx, *lb;

   win = elm_win_add(parent, "inlined", ELM_WIN_INLINED_IMAGE);
   if (!win) return NULL;

   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 110, 210, 120);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   evas_object_show(bx);

   LOG("ELM_WIN_INLINED_IMAGE");
   elm_box_pack_end(bx, lb);

   LOG("Profile: <b>N/A</b>");
   elm_box_pack_end(bx, lb);
   evas_object_data_set(win, "lb", lb);

   evas_object_move(win, 10, 100);
   evas_object_resize(win, 150, 70);
   evas_object_move(elm_win_inlined_image_object_get(win), 10, 100);
   evas_object_resize(elm_win_inlined_image_object_get(win), 150, 70);

   evas_object_smart_callback_add(win, "profile,changed", _win_profile_changed_cb, NULL);
   evas_object_show(win);

   return win;
}

static Evas_Object *
_socket_add(const char *name)
{
   Evas_Object *win, *bg, *bx, *lb;

   win = elm_win_add(NULL, "socket image", ELM_WIN_SOCKET_IMAGE);
   if (!win) return NULL;

   if (elm_win_socket_listen(win, name, 0, EINA_FALSE))
     {
        elm_win_autodel_set(win, EINA_TRUE);

        bg = elm_bg_add(win);
        elm_bg_color_set(bg, 80, 110, 205);
        evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(win, bg);
        evas_object_show(bg);

        bx = elm_box_add(win);
        evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
        evas_object_show(bx);

        LOG("ELM_WIN_SOCKET_IMAGE");
        elm_box_pack_end(bx, lb);

        LOG("Profile: <b>N/A</b>");
        elm_box_pack_end(bx, lb);
        evas_object_data_set(win, "lb", lb);

        _inlined_add(win);

        evas_object_move(win, 0, 0);
        evas_object_resize(win, 150, 200);

        evas_object_smart_callback_add(win, "profile,changed", _win_profile_changed_cb, NULL);
        evas_object_show(win);
     }
   else
     {
        evas_object_del(win);
        win = NULL;
     }

   return win;
}

static Evas_Object *
_plug_add(Evas_Object *win, Evas_Object *bx, const char *name)
{
   Evas_Object *plug, *ly;
   Eina_Bool res = EINA_FALSE;
   char buf[PATH_MAX];

   plug = elm_plug_add(win);
   if (plug) res = elm_plug_connect(plug, name, 0, EINA_FALSE);

   if (res)
     {
        ly = elm_layout_add(win);
        snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
        elm_layout_file_set(ly, buf, "win_config");
        evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_fill_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_show(ly);

        evas_object_size_hint_weight_set(plug, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_box_pack_end(bx, ly);
        elm_object_part_content_set(ly, "swallow", plug);
        evas_object_show(plug);
     }
   else
     {
        if (plug) evas_object_del(plug);
        plug = NULL;
     }

   return plug;
}

#ifdef FRAME
# undef FRAME
#endif

#define FRAME(t)                                                                  \
   do {                                                                           \
        fr = elm_frame_add(bx);                                                   \
        elm_object_text_set(fr, t);                                               \
        evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); \
        evas_object_size_hint_fill_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);       \
        elm_box_pack_end(bx, fr);                                                 \
        evas_object_show(fr);                                                     \
        bx2 = elm_box_add(win);                                                   \
        evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);             \
        evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.0);                \
        elm_box_align_set(bx2, 0.0, 0.5);                                         \
        elm_object_content_set(fr, bx2);                                          \
        evas_object_show(bx2);                                                    \
   } while(0)

void
test_config(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad;
   Prof_Data *pd = (Prof_Data *)data;
   Evas_Object *win, *sc, *bx, *fr, *bx2, *lb, *bt;
   Ecore_Evas *ee;
   const char *siname = "_TestConfigSocketImage_";
   char buf[PATH_MAX];

   if (!(ad = calloc(1, sizeof(App_Data)))) return;

   win = elm_win_util_standard_add("config", "Configuration");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_data_set(win, "ad", ad);
   ad->win = win;
   ad->profiles = elm_config_profile_list_get();

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(sc, bx);

   FRAME("Current window profile");
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win));
   snprintf(buf, sizeof(buf),
            "Virtual desktop window profile support: <b>%s</b>",
            ecore_evas_window_profile_supported_get(ee) ? "Yes" : "No");
   LOG(buf);
   elm_box_pack_end(bx2, lb);

   LOG("Profile: <b>N/A</b><br/>Available profiles:");
   elm_box_pack_end(bx2, lb);
   evas_object_data_set(win, "lb", lb);

   LOG("<br/>Window profile");
   elm_box_pack_end(bx2, lb);
   ad->curr.rdg = _radio_add(win, bx2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Set");
   evas_object_smart_callback_add(bt, "clicked", _bt_profile_set, win);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   LOG("Window available profiles");
   elm_box_pack_end(bx2, lb);
   ad->curr.cks = _check_add(win, bx2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Set");
   evas_object_smart_callback_add(bt, "clicked", _bt_available_profiles_set, win);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   FRAME("Socket");
   if (_socket_add(siname))
     {
        LOG("Starting socket image.");
        elm_box_pack_end(bx2, lb);
     }
   else
     {
        LOG("Failed to create socket.<br/>"
            "Please check whether another test configuration window is<br/>"
            "already running and providing socket image.");
        elm_box_pack_end(bx2, lb);
     }

   FRAME("Plug");
   if (!_plug_add(win, bx2, siname))
     {
        LOG("Failed to connect to server.");
        elm_box_pack_end(bx2, lb);
     }

   FRAME("Create new window with profile");
   LOG("Window profile");
   elm_box_pack_end(bx2, lb);
   ad->new.rdg = _radio_add(win, bx2);

   LOG("Window available profiles");
   elm_box_pack_end(bx2, lb);
   ad->new.cks = _check_add(win, bx2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Create");
   evas_object_smart_callback_add(bt, "clicked", _bt_win_add, win);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_smart_callback_add(win, "profile,changed", _win_profile_changed_cb, NULL);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, NULL);

   if (pd && !obj) //obj is NULL when called by _bt_win_add but not when user clicks this test
     {
        if (pd->available_profiles[0])
          elm_win_available_profiles_set(win,
                                         pd->available_profiles,
                                         pd->count);
        if (pd->profile)
          elm_win_profile_set(win, pd->profile);

        _profile_update(win);
     }

   evas_object_show(bx);
   evas_object_show(sc);

   evas_object_resize(win, 400, 500);
   evas_object_show(win);
}

static void
_font_overlay_page_next(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *layout;
   Evas_Object *nf = (Evas_Object *)data;
   char buf[255];

   layout = elm_layout_add(nf);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(layout, buf, "font_overlay_layout");
   elm_layout_text_set(layout, "elm.text", "TEXTBLOCK part of test_class");
   evas_object_show(layout);

   elm_naviframe_item_push(nf, "Font Overlay", NULL, NULL, layout, NULL);
}

static void
_apply_font_overlay_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *nf = (Evas_Object *)data;
   Evas_Object *entry;
   const char *font;
   const char *font_size_temp;
   int font_size;

   entry = (Evas_Object *)evas_object_data_get(nf, "font_entry");
   font = elm_entry_entry_get(entry);
   entry = (Evas_Object *)evas_object_data_get(nf, "font_size_entry");
   font_size_temp = elm_entry_entry_get(entry);
   font_size = atoi(font_size_temp);

   printf("Font overlay set: Font [%s], FontSize [%d]\n", font, font_size);
   elm_config_font_overlay_set("font_overlay_test", font, font_size);
   elm_config_font_overlay_apply();
}

void
test_config_font_overlay(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *nf, *box, *horizon_box, *label, *btn, *entry;
   Elm_Object_Item *it;

   win = elm_win_util_standard_add("naviframe", "Naviframe");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   nf = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, nf);
   evas_object_show(nf);

   box = elm_box_add(nf);
   evas_object_show(box);

   horizon_box = elm_box_add(box);
   elm_box_horizontal_set(horizon_box, EINA_TRUE);
   evas_object_size_hint_align_set(horizon_box, EVAS_HINT_FILL, -1);
   evas_object_size_hint_weight_set(horizon_box, EVAS_HINT_EXPAND, -1);
   elm_box_pack_end(box, horizon_box);
   evas_object_show(horizon_box);

   label = elm_label_add(horizon_box);
   elm_object_text_set(label, "Font:");
   elm_box_pack_end(horizon_box, label);
   evas_object_show(label);

   entry = elm_entry_add(horizon_box);
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, -1);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, -1);
   elm_object_part_text_set(entry, "elm.guide", "Input Font");
   elm_box_pack_end(horizon_box, entry);
   evas_object_show(entry);
   evas_object_data_set(nf, "font_entry", entry);

   horizon_box = elm_box_add(box);
   elm_box_horizontal_set(horizon_box, EINA_TRUE);
   evas_object_size_hint_align_set(horizon_box, EVAS_HINT_FILL, -1);
   evas_object_size_hint_weight_set(horizon_box, EVAS_HINT_EXPAND, -1);
   elm_box_pack_end(box, horizon_box);
   evas_object_show(horizon_box);

   label = elm_label_add(horizon_box);
   elm_object_text_set(label, "Size:");
   elm_box_pack_end(horizon_box, label);
   evas_object_show(label);

   entry = elm_entry_add(horizon_box);
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, -1);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, -1);
   elm_object_part_text_set(entry, "elm.guide", "Input Font Size");
   elm_box_pack_end(horizon_box, entry);
   evas_object_show(entry);
   evas_object_data_set(nf, "font_size_entry", entry);

   btn = elm_button_add(box);
   elm_object_text_set(btn, "Apply Font Overlay");
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, -1);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, -1);
   evas_object_smart_callback_add(btn, "clicked", _apply_font_overlay_btn_clicked_cb, nf);
   elm_box_pack_end(box, btn);
   evas_object_show(btn);

   btn = elm_button_add(nf);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(btn, "clicked", _font_overlay_page_next, nf);
   elm_object_text_set(btn, "Next");
   evas_object_show(btn);

   it = elm_naviframe_item_push(nf, "Font Overlay", NULL, btn, box, NULL);
   evas_object_data_set(nf, "page1", it);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
