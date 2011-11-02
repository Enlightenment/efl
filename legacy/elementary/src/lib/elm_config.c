#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

EAPI int ELM_EVENT_CONFIG_ALL_CHANGED = 0;

Elm_Config *_elm_config = NULL;
char *_elm_profile = NULL;
static Eet_Data_Descriptor *_config_edd = NULL;
static Eet_Data_Descriptor *_config_font_overlay_edd = NULL;

static Ecore_Poller *_elm_cache_flush_poller = NULL;

const char *_elm_engines[] = {
   "software_x11",
   "fb",
   "directfb",
   "software_16_x11",
   "software_8_x11",
   "xrender_x11",
   "opengl_x11",
   "software_gdi",
   "software_16_wince_gdi",
   "sdl",
   "software_16_sdl",
   "opengl_sdl",
   "buffer",
   "ews",
   "opengl_cocoa",
   NULL
};

/* whenever you want to add a new text class support into Elementary,
   declare it both here and in the (default) theme */
static const Elm_Text_Class _elm_text_classes[] = {
   {"button", "Button Labels"},
   {"label", "Text Labels"},
   {"entry", "Text Entries"},
   {"title_bar", "Title Bar"},
   {"list_item", "List Items"},
   {"grid_item", "Grid Items"},
   {"toolbar_item", "Toolbar Items"},
   {"menu_item", "Menu Items"},
   {NULL, NULL}
};

static void        _desc_init(void);
static void        _desc_shutdown(void);
static void        _profile_fetch_from_conf(void);
static void        _config_free(void);
static void        _config_apply(void);
static Elm_Config *_config_user_load(void);
static Elm_Config *_config_system_load(void);
static void        _config_load(void);
static void        _config_update(void);
static void        _env_get(void);
static size_t      _elm_data_dir_snprintf(char       *dst,
                                          size_t      size,
                                          const char *fmt, ...)
                                          EINA_PRINTF(3, 4);
static size_t _elm_user_dir_snprintf(char       *dst,
                                     size_t      size,
                                     const char *fmt, ...)
                                     EINA_PRINTF(3, 4);

#define ELM_CONFIG_VAL(edd, type, member, dtype) \
  EET_DATA_DESCRIPTOR_ADD_BASIC(edd, type, #member, member, dtype)
#define ELM_CONFIG_LIST(edd, type, member, eddtype) \
  EET_DATA_DESCRIPTOR_ADD_LIST(edd, type, #member, member, eddtype)

#ifdef HAVE_ELEMENTARY_X
static Ecore_Event_Handler *_prop_change_handler = NULL;
static Ecore_Timer *_prop_all_update_timer = NULL;
static Ecore_Timer *_prop_change_delay_timer = NULL;
static Ecore_X_Window _root_1st = 0;
#define ATOM_COUNT 2
static Ecore_X_Atom _atom[ATOM_COUNT];
static Ecore_X_Atom _atom_config = 0;
static const char *_atom_names[ATOM_COUNT] =
{
   "ELM_PROFILE",
   "ELM_CONFIG"
};
#define ATOM_E_PROFILE                              0
#define ATOM_E_CONFIG                               1

static Eina_Bool _prop_all_update_cb(void *data __UNUSED__);
static Eina_Bool _prop_config_get(void);
static void      _prop_config_set(void);
static Eina_Bool _prop_change(void *data  __UNUSED__,
                              int ev_type __UNUSED__,
                              void       *ev);

static Eina_Bool
_prop_all_update_cb(void *data __UNUSED__)
{
   _prop_config_set();
   ecore_x_window_prop_string_set(_root_1st, _atom[ATOM_E_PROFILE],
                                  _elm_profile);
   _prop_all_update_timer = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_prop_config_get(void)
{
   int size = 0;
   Ecore_X_Atom atom;
   char buf[512];
   unsigned char *data = NULL;
   Elm_Config *config_data;

   snprintf(buf, sizeof(buf), "ELM_CONFIG_%s", _elm_profile);
   atom = ecore_x_atom_get(buf);
   _atom_config = atom;
   if (!ecore_x_window_prop_property_get(_root_1st,
                                         atom, _atom[ATOM_E_CONFIG],
                                         8, &data, &size))
     {
        if (!ecore_x_window_prop_property_get(_root_1st,
                                              _atom[ATOM_E_CONFIG],
                                              _atom[ATOM_E_CONFIG],
                                              8, &data, &size))
          return EINA_FALSE;
        else
          _atom_config = _atom[ATOM_E_CONFIG];
     }
   else
     _atom_config = atom;
   if (size < 1)
     {
        free(data);
        return EINA_FALSE;
     }
   config_data = eet_data_descriptor_decode(_config_edd, data, size);
   free(data);
   if (!config_data) return EINA_FALSE;

   /* What do we do on version mismatch when someone changes the
    * config in the rootwindow? */
   /* Most obvious case, new version and we are still linked to
    * whatever was there before, we just ignore until user restarts us */
   if (config_data->config_version > ELM_CONFIG_VERSION)
     return EINA_TRUE;
   /* What in the case the version is older? Do we even support those
    * cases or we only check for equality above? */

   _config_free();
   _elm_config = config_data;
   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_rescale();
   _elm_recache();
   ecore_event_add(ELM_EVENT_CONFIG_ALL_CHANGED, NULL, NULL, NULL);
   return EINA_TRUE;
}

static void
_prop_config_set(void)
{
   unsigned char *config_data = NULL;
   int size = 0;

   config_data = eet_data_descriptor_encode(_config_edd, _elm_config, &size);
   if (config_data)
     {
        Ecore_X_Atom atom;
        char buf[512];

        snprintf(buf, sizeof(buf), "ELM_CONFIG_%s", _elm_profile);
        atom = ecore_x_atom_get(buf);
        _atom_config = atom;

        ecore_x_window_prop_property_set(_root_1st, _atom_config,
                                         _atom[ATOM_E_CONFIG], 8,
                                         config_data, size);
        free(config_data);
     }
}

static Eina_Bool
_prop_change_delay_cb(void *data __UNUSED__)
{
   char *s;

   s = ecore_x_window_prop_string_get(_root_1st, _atom[ATOM_E_PROFILE]);
   if (s)
     {
        if (_elm_profile) free(_elm_profile);
        _elm_profile = s;
     }
   _prop_config_get();
   _prop_change_delay_timer = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_prop_change(void *data  __UNUSED__,
             int ev_type __UNUSED__,
             void       *ev)
{
   Ecore_X_Event_Window_Property *event = ev;

   if (event->win == _root_1st)
     {
        if (event->atom == _atom[ATOM_E_PROFILE])
          {
             if (_prop_change_delay_timer) ecore_timer_del(_prop_change_delay_timer);
             _prop_change_delay_timer = ecore_timer_add(0.1, _prop_change_delay_cb, NULL);
          }
        else if (((_atom_config > 0) && (event->atom == _atom_config)) ||
                 (event->atom == _atom[ATOM_E_CONFIG]))
          {
             if (_prop_change_delay_timer) ecore_timer_del(_prop_change_delay_timer);
             _prop_change_delay_timer = ecore_timer_add(0.1, _prop_change_delay_cb, NULL);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

#endif

static void
_desc_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Config);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_edd = eet_data_descriptor_file_new(&eddc);
   if (!_config_edd)
     {
        printf("EEEK! eet_data_descriptor_file_new() failed\n");
        return;
     }

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Font_Overlay);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_font_overlay_edd = eet_data_descriptor_stream_new(&eddc);
   if (!_config_font_overlay_edd)
     {
        printf("EEEK! eet_data_descriptor_stream_new() failed\n");
        eet_data_descriptor_free(_config_edd);
        return;
     }
#define T_INT    EET_T_INT
#define T_DOUBLE EET_T_DOUBLE
#define T_STRING EET_T_STRING
#define T_UCHAR  EET_T_UCHAR

#define T        Elm_Font_Overlay
#define D        _config_font_overlay_edd
   ELM_CONFIG_VAL(D, T, text_class, EET_T_STRING);
   ELM_CONFIG_VAL(D, T, font, EET_T_STRING);
   ELM_CONFIG_VAL(D, T, size, EET_T_INT);
#undef T
#undef D

#define T Elm_Config
#define D _config_edd
   ELM_CONFIG_VAL(D, T, config_version, T_INT);
   ELM_CONFIG_VAL(D, T, engine, T_STRING);
   ELM_CONFIG_VAL(D, T, vsync, T_UCHAR);
   ELM_CONFIG_VAL(D, T, thumbscroll_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, thumbscroll_threshold, T_INT);
   ELM_CONFIG_VAL(D, T, thumbscroll_momentum_threshold, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_bounce_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_border_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_sensitivity_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, page_scroll_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, bring_in_scroll_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, zoom_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_bounce_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, scroll_smooth_amount, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, scroll_smooth_history_weight, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, scroll_smooth_future_time, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, scroll_smooth_time_window, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, scale, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, bgpixmap, T_INT);
   ELM_CONFIG_VAL(D, T, compositing, T_INT);
   /* EET_DATA_DESCRIPTOR_ADD_LIST(D, T, "font_dirs", font_dirs, sub_edd); */
   ELM_CONFIG_LIST(D, T, font_overlays, _config_font_overlay_edd);
   ELM_CONFIG_VAL(D, T, font_hinting, T_INT);
   ELM_CONFIG_VAL(D, T, cache_flush_poll_interval, T_INT);
   ELM_CONFIG_VAL(D, T, cache_flush_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, image_cache, T_INT);
   ELM_CONFIG_VAL(D, T, font_cache, T_INT);
   ELM_CONFIG_VAL(D, T, edje_cache, T_INT);
   ELM_CONFIG_VAL(D, T, edje_collection_cache, T_INT);
   ELM_CONFIG_VAL(D, T, finger_size, T_INT);
   ELM_CONFIG_VAL(D, T, fps, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, theme, T_STRING);
   ELM_CONFIG_VAL(D, T, modules, T_STRING);
   ELM_CONFIG_VAL(D, T, tooltip_delay, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, cursor_engine_only, T_UCHAR);
   ELM_CONFIG_VAL(D, T, focus_highlight_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, focus_highlight_animate, T_UCHAR);
   ELM_CONFIG_VAL(D, T, toolbar_shrink_mode, T_INT);
   ELM_CONFIG_VAL(D, T, fileselector_expand_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, inwin_dialogs_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, icon_size, T_INT);
   ELM_CONFIG_VAL(D, T, longpress_timeout, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, effect_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, desktop_entry, T_UCHAR);
   ELM_CONFIG_VAL(D, T, password_show_last, T_UCHAR);
   ELM_CONFIG_VAL(D, T, password_show_last_timeout, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_zoom_finger_factor, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_zoom_wheel_factor, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_zoom_distance_tolerance, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_rotate_angular_tolerance, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_line_min_length, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_line_distance_tolerance, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_line_angular_tolerance, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_flick_time_limit_ms, T_INT);
   ELM_CONFIG_VAL(D, T, glayer_long_tap_start_timeout, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, access_mode, T_INT);
   ELM_CONFIG_VAL(D, T, glayer_continues_enable, T_UCHAR);
#undef T
#undef D
#undef T_INT
#undef T_DOUBLE
#undef T_STRING
#undef T_UCHAR
}

static void
_desc_shutdown(void)
{
   if (_config_edd)
     {
        eet_data_descriptor_free(_config_edd);
        _config_edd = NULL;
     }

   if (_config_font_overlay_edd)
     {
        eet_data_descriptor_free(_config_font_overlay_edd);
        _config_font_overlay_edd = NULL;
     }
}

static int
_sort_files_cb(const void *f1,
               const void *f2)
{
   return strcmp(f1, f2);
}

const char *
_elm_config_current_profile_get(void)
{
   return _elm_profile;
}

static size_t
_elm_data_dir_snprintf(char       *dst,
                       size_t      size,
                       const char *fmt,
                       ...)
{
   size_t data_dir_len, off;
   va_list ap;

   data_dir_len = eina_strlcpy(dst, _elm_data_dir, size);

   off = data_dir_len + 1;
   if (off >= size)
     goto end;

   va_start(ap, fmt);
   dst[data_dir_len] = '/';

   off = off + vsnprintf(dst + off, size - off, fmt, ap);
   va_end(ap);

end:
   return off;
}

static size_t
_elm_user_dir_snprintf(char       *dst,
                       size_t      size,
                       const char *fmt,
                       ...)
{
   const char *home;
   size_t user_dir_len, off;
   va_list ap;

#ifdef _WIN32
   home = evil_homedir_get();
#else
   home = getenv("HOME");
#endif
   if (!home)
     home = "/";

   user_dir_len = eina_str_join_len(dst, size, '/', home, strlen(home),
                                    ".elementary", sizeof(".elementary") - 1);

   off = user_dir_len + 1;
   if (off >= size)
     goto end;

   va_start(ap, fmt);
   dst[user_dir_len] = '/';

   off = off + vsnprintf(dst + off, size - off, fmt, ap);
   va_end(ap);

end:
   return off;
}

const char *
_elm_config_profile_dir_get(const char *prof,
                            Eina_Bool   is_user)
{
   char buf[PATH_MAX];

   if (!is_user)
     goto not_user;

   _elm_user_dir_snprintf(buf, sizeof(buf), "config/%s", prof);

   if (ecore_file_is_dir(buf))
     return strdup(buf);

   return NULL;

not_user:
   snprintf(buf, sizeof(buf), "%s/config/%s", _elm_data_dir, prof);

   if (ecore_file_is_dir(buf))
     return strdup(buf);

   return NULL;
}

Eina_List *
_elm_config_font_overlays_list(void)
{
   return _elm_config->font_overlays;
}

void
_elm_config_font_overlay_set(const char    *text_class,
                             const char    *font,
                             Evas_Font_Size size)
{
   Elm_Font_Overlay *efd;
   Eina_List *l;

   EINA_LIST_FOREACH(_elm_config->font_overlays, l, efd)
     {
        if (strcmp(efd->text_class, text_class))
          continue;

        if (efd->font) eina_stringshare_del(efd->font);
        efd->font = eina_stringshare_add(font);
        efd->size = size;
        _elm_config->font_overlays =
          eina_list_promote_list(_elm_config->font_overlays, l);
        return;
     }

   /* the text class doesn't exist */
   efd = calloc(1, sizeof(Elm_Font_Overlay));
   efd->text_class = eina_stringshare_add(text_class);
   efd->font = eina_stringshare_add(font);
   efd->size = size;

   _elm_config->font_overlays = eina_list_prepend(_elm_config->font_overlays,
                                                  efd);
}

void
_elm_config_font_overlay_remove(const char *text_class)
{
   Elm_Font_Overlay *efd;
   Eina_List *l;

   EINA_LIST_FOREACH(_elm_config->font_overlays, l, efd)
     {
        if (strcmp(efd->text_class, text_class))
          continue;

        _elm_config->font_overlays =
          eina_list_remove_list(_elm_config->font_overlays, l);
        if (efd->text_class) eina_stringshare_del(efd->text_class);
        if (efd->font) eina_stringshare_del(efd->font);
        free(efd);

        return;
     }
}

void
_elm_config_font_overlay_apply(void)
{
   Elm_Font_Overlay *efd;
   Eina_List *l;
   int i;

   for (i = 0; _elm_text_classes[i].desc; i++)
     edje_text_class_del(_elm_text_classes[i].name);

   EINA_LIST_FOREACH(_elm_config->font_overlays, l, efd)
     edje_text_class_set(efd->text_class, efd->font, efd->size);
}

Eina_List *
_elm_config_text_classes_get(void)
{
   Eina_List *ret = NULL;
   int i;

   for (i = 0; _elm_text_classes[i].desc; i++)
     {
        Elm_Text_Class *tc;
        tc = malloc(sizeof(*tc));

        *tc = _elm_text_classes[i];

        ret = eina_list_append(ret, tc);
     }

   return ret;
}

void
_elm_config_text_classes_free(Eina_List *l)
{
   Elm_Text_Class *tc;

   EINA_LIST_FREE(l, tc)
     free(tc);
}

Eina_List *
_elm_config_profiles_list(void)
{
   const Eina_File_Direct_Info *info;
   Eina_List *flist = NULL;
   Eina_Iterator *file_it;
   char buf[PATH_MAX];
   const char *dir;
   size_t len;

   len = _elm_user_dir_snprintf(buf, sizeof(buf), "config");

   file_it = eina_file_direct_ls(buf);
   if (!file_it)
     goto sys;

   buf[len] = '/';
   len++;

   len = sizeof(buf) - len;

   EINA_ITERATOR_FOREACH(file_it, info)
     {
        if (info->name_length >= len)
          continue;

        if (info->type == EINA_FILE_DIR)
          {
             flist =
               eina_list_sorted_insert(flist, _sort_files_cb,
                                       eina_stringshare_add(info->path +
                                                            info->name_start));
          }
     }

   eina_iterator_free(file_it);

sys:
   len = eina_str_join_len(buf, sizeof(buf), '/', _elm_data_dir,
                           strlen(_elm_data_dir), "config",
                           sizeof("config") - 1);

   file_it = eina_file_direct_ls(buf);
   if (!file_it)
     goto list_free;

   buf[len] = '/';
   len++;

   len = sizeof(buf) - len;
   EINA_ITERATOR_FOREACH(file_it, info)
     {
        if (info->name_length >= len)
          continue;

        switch (info->type)
          {
           case EINA_FILE_DIR:
           {
              const Eina_List *l;
              const char *tmp;

              EINA_LIST_FOREACH(flist, l, tmp)
                if (!strcmp(info->path + info->name_start, tmp))
                  break;

              if (!l)
                flist =
                  eina_list_sorted_insert(flist, _sort_files_cb,
                                          eina_stringshare_add(info->path +
                                                               info->name_start));
           }
           break;

           default:
             continue;
          }
     }
   eina_iterator_free(file_it);
   return flist;

list_free:
   EINA_LIST_FREE(flist, dir)
     eina_stringshare_del(dir);

   return NULL;
}

static void
_profile_fetch_from_conf(void)
{
   char buf[PATH_MAX], *p, *s;
   Eet_File *ef = NULL;
   int len = 0;

   _elm_profile = strdup("default");

   // if env var - use profile without question
   s = getenv("ELM_PROFILE");
   if (s)
     {
        free(_elm_profile);
        _elm_profile = strdup(s);
        return;
     }

   // user profile
   _elm_user_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        p = eet_read(ef, "config", &len);
        if (p)
          {
             free(_elm_profile);
             _elm_profile = malloc(len + 1);
             memcpy(_elm_profile, p, len);
             _elm_profile[len] = 0;
             free(p);
          }
        eet_close(ef);
        if (!p) ef = NULL;
     }
   if (ef) return;

   // system profile
   _elm_data_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        p = eet_read(ef, "config", &len);
        if (p)
          {
             free(_elm_profile);
             _elm_profile = malloc(len + 1);
             memcpy(_elm_profile, p, len);
             _elm_profile[len] = 0;
             free(p);
          }
        eet_close(ef);
     }
}

static void
_config_free(void)
{
   Elm_Font_Overlay *fo;
   const char *fontdir;

   if (!_elm_config) return;
   EINA_LIST_FREE(_elm_config->font_dirs, fontdir)
     {
        eina_stringshare_del(fontdir);
     }
   if (_elm_config->engine) eina_stringshare_del(_elm_config->engine);
   EINA_LIST_FREE(_elm_config->font_overlays, fo)
     {
        if (fo->text_class) eina_stringshare_del(fo->text_class);
        if (fo->font) eina_stringshare_del(fo->font);
        free(fo);
     }
   if (_elm_config->theme) eina_stringshare_del(_elm_config->theme);
   if (_elm_config->modules) eina_stringshare_del(_elm_config->modules);
   free(_elm_config);
   _elm_config = NULL;
}

static void
_config_apply(void)
{
   _elm_theme_parse(NULL, _elm_config->theme);
   ecore_animator_frametime_set(1.0 / _elm_config->fps);
}

static void
_config_sub_apply(void)
{
   edje_frametime_set(1.0 / _elm_config->fps);
   edje_scale_set(_elm_config->scale);
   edje_password_show_last_set(_elm_config->password_show_last);
   edje_password_show_last_timeout_set(_elm_config->password_show_last_timeout);
   if (_elm_config->modules) _elm_module_parse(_elm_config->modules);
}

static Eina_Bool
_elm_cache_flush_cb(void *data __UNUSED__)
{
   elm_all_flush();
   return ECORE_CALLBACK_RENEW;
}

/* kind of abusing this call right now -- shared between all of those
 * properties -- but they are not meant to be called that periodically
 * anyway */
void
_elm_recache(void)
{
   Eina_List *l;
   Evas_Object *win;

   elm_all_flush();

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        Evas *e = evas_object_evas_get(win);
        evas_image_cache_set(e, _elm_config->image_cache * 1024);
        evas_font_cache_set(e, _elm_config->font_cache * 1024);
     }
   edje_file_cache_set(_elm_config->edje_cache);
   edje_collection_cache_set(_elm_config->edje_collection_cache);

   if (_elm_cache_flush_poller)
     {
        ecore_poller_del(_elm_cache_flush_poller);
        _elm_cache_flush_poller = NULL;
     }
   if (_elm_config->cache_flush_enable)
     {
        if (_elm_config->cache_flush_poll_interval > 0)
          {
             _elm_cache_flush_poller =
                ecore_poller_add(ECORE_POLLER_CORE,
                                 _elm_config->cache_flush_poll_interval,
                                 _elm_cache_flush_cb, NULL);
          }
     }
}

static Elm_Config *
_config_user_load(void)
{
   Elm_Config *cfg = NULL;
   Eet_File *ef;
   char buf[PATH_MAX];

   _elm_user_dir_snprintf(buf, sizeof(buf), "config/%s/base.cfg",
                          _elm_profile);

   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        cfg = eet_data_read(ef, _config_edd, "config");
        eet_close(ef);
     }
   return cfg;
}

static Elm_Config *
_config_system_load(void)
{
   Elm_Config *cfg = NULL;
   Eet_File *ef;
   char buf[PATH_MAX];

   _elm_data_dir_snprintf(buf, sizeof(buf), "config/%s/base.cfg",
                          _elm_profile);

   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        cfg = eet_data_read(ef, _config_edd, "config");
        eet_close(ef);
     }
   return cfg;
}

static void
_config_load(void)
{
   _elm_config = _config_user_load();
   if (_elm_config)
     {
        if (_elm_config->config_version < ELM_CONFIG_VERSION)
          _config_update();
        return;
     }

   /* no user config, fallback for system. No need to check version for
    * this one, if it's not the right one, someone screwed up at the time
    * of installing it */
   _elm_config = _config_system_load();
   if (_elm_config) return;
   /* FIXME: config load could have failed because of a non-existent
    * profile. Fallback to default before moving on */

   // config load fail - defaults
   /* XXX: do these make sense? Only if it's valid to install the lib
    * without the config, but do we want that? */
   _elm_config = ELM_NEW(Elm_Config);
   _elm_config->config_version = ELM_CONFIG_VERSION;
   _elm_config->engine = eina_stringshare_add("software_x11");
   _elm_config->vsync = 0;
   _elm_config->thumbscroll_enable = EINA_TRUE;
   _elm_config->thumbscroll_threshold = 24;
   _elm_config->thumbscroll_momentum_threshold = 100.0;
   _elm_config->thumbscroll_friction = 1.0;
   _elm_config->thumbscroll_bounce_friction = 0.5;
   _elm_config->thumbscroll_bounce_enable = EINA_TRUE;
   _elm_config->page_scroll_friction = 0.5;
   _elm_config->bring_in_scroll_friction = 0.5;
   _elm_config->zoom_friction = 0.5;
   _elm_config->thumbscroll_border_friction = 0.5;
   _elm_config->thumbscroll_sensitivity_friction = 0.25; // magic number! just trial and error shows this makes it behave "nicer" and not run off at high speed all the time
   _elm_config->scroll_smooth_amount = 1.0;
   _elm_config->scroll_smooth_history_weight = 0.3;
   _elm_config->scroll_smooth_future_time = 0.0;
   _elm_config->scroll_smooth_time_window = 0.2;
   _elm_config->scale = 1.0;
   _elm_config->bgpixmap = 0;
   _elm_config->compositing = 1;
   _elm_config->font_hinting = 2;
   _elm_config->cache_flush_poll_interval = 512;
   _elm_config->cache_flush_enable = EINA_TRUE;
   _elm_config->font_dirs = NULL;
   _elm_config->image_cache = 4096;
   _elm_config->font_cache = 512;
   _elm_config->edje_cache = 32;
   _elm_config->edje_collection_cache = 64;
   _elm_config->finger_size = 40;
   _elm_config->fps = 60.0;
   _elm_config->theme = eina_stringshare_add("default");
   _elm_config->modules = NULL;
   _elm_config->tooltip_delay = 1.0;
   _elm_config->cursor_engine_only = EINA_TRUE;
   _elm_config->focus_highlight_enable = EINA_FALSE;
   _elm_config->focus_highlight_animate = EINA_TRUE;
   _elm_config->toolbar_shrink_mode = 2;
   _elm_config->fileselector_expand_enable = EINA_FALSE;
   _elm_config->inwin_dialogs_enable = EINA_FALSE;
   _elm_config->icon_size = 32;
   _elm_config->longpress_timeout = 1.0;
   _elm_config->effect_enable = EINA_TRUE;
   _elm_config->desktop_entry = EINA_FALSE;
   _elm_config->is_mirrored = EINA_FALSE; /* Read sys value in env_get() */
   _elm_config->password_show_last = EINA_FALSE;
   _elm_config->password_show_last_timeout = 2.0;
   _elm_config->glayer_zoom_finger_factor = 1.0;
   _elm_config->glayer_zoom_wheel_factor = 0.05;
   _elm_config->glayer_zoom_distance_tolerance = 1.0; /* 1 times elm_finger_size_get() */
   _elm_config->glayer_rotate_angular_tolerance = 0.034906585;     /* Represents 2 DEG */
   _elm_config->glayer_line_min_length = 1.0;         /* 1 times elm_finger_size_get() */
   _elm_config->glayer_line_distance_tolerance = 3.0; /* 3 times elm_finger_size_get() */
   _elm_config->glayer_line_angular_tolerance = 0.34906585;       /* Represents 20 DEG */
   _elm_config->glayer_flick_time_limit_ms = 60;              /* 60 ms to finish flick */
   _elm_config->glayer_long_tap_start_timeout = 1.2;   /* 1.2 second to start long-tap */
   _elm_config->glayer_continues_enable = EINA_TRUE;      /* Continue gestures default */
}

static const char *
_elm_config_eet_close_error_get(Eet_File *ef,
                                char     *file)
{
   Eet_Error err;
   const char *erstr = NULL;

   err = eet_close(ef);
   switch (err)
     {
      case EET_ERROR_WRITE_ERROR:
        erstr = "An error occurred while saving Elementary's "
                "settings to disk. The error could not be "
                "deterimined. The file where the error occurred was: "
                "%s. This file has been deleted to avoid corrupt data.";
        break;

      case EET_ERROR_WRITE_ERROR_FILE_TOO_BIG:
        erstr = "Elementary's settings files are too big "
                "for the file system they are being saved to. "
                "This error is very strange as the files should "
                "be extremely small. Please check the settings "
                "for your home directory. "
                "The file where the error occurred was: %s ."
                "This file has been deleted to avoid corrupt data.";
        break;

      case EET_ERROR_WRITE_ERROR_IO_ERROR:
        erstr = "An output error occurred when writing the settings "
                "files for Elementary. Your disk is having troubles "
                "and possibly needs replacement. "
                "The file where the error occurred was: %s ."
                "This file has been deleted to avoid corrupt data.";
        break;

      case EET_ERROR_WRITE_ERROR_OUT_OF_SPACE:
        erstr = "Elementary cannot write its settings file "
                "because it ran out of space to write the file. "
                "You have either run out of disk space or have "
                "gone over your quota limit. "
                "The file where the error occurred was: %s ."
                "This file has been deleted to avoid corrupt data.";
        break;

      case EET_ERROR_WRITE_ERROR_FILE_CLOSED:
        erstr = "Elementary unexpectedly had the settings file "
                "it was writing closed on it. This is very unusual. "
                "The file where the error occurred was: %s "
                "This file has been deleted to avoid corrupt data.";
        break;

      default:
        break;
     }
   if (erstr)
     {
        /* delete any partially-written file */
         ecore_file_unlink(file);
         return strdup(erstr);
     }

   return NULL;
}

static Eina_Bool
_elm_config_profile_save(void)
{
   char buf[4096], buf2[4096];
   int ok = 0, ret;
   const char *err;
   Eet_File *ef;
   size_t len;

   len = _elm_user_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
   if (len + 1 >= sizeof(buf))
     return EINA_FALSE;

   len = _elm_user_dir_snprintf(buf2, sizeof(buf2), "config/profile.cfg.tmp");
   if (len + 1 >= sizeof(buf2))
     return EINA_FALSE;

   ef = eet_open(buf2, EET_FILE_MODE_WRITE);
   if (!ef)
     return EINA_FALSE;

   ok = eet_write(ef, "config", _elm_profile, strlen(_elm_profile), 0);
   if (!ok)
     goto err;

   err = _elm_config_eet_close_error_get(ef, buf2);
   if (err)
     {
        ERR("%s", err);
        free((void *)err);
        goto err;
     }

   ret = ecore_file_mv(buf2, buf);
   if (!ret)
     {
        ERR("Error saving Elementary's configuration file");
        goto err;
     }

   ecore_file_unlink(buf2);
   return EINA_TRUE;

err:
   ecore_file_unlink(buf2);
   return EINA_FALSE;
}

Eina_Bool
_elm_config_save(void)
{
   char buf[4096], buf2[4096];
   int ok = 0, ret;
   const char *err;
   Eet_File *ef;
   size_t len;

   len = _elm_user_dir_snprintf(buf, sizeof(buf), "config/%s", _elm_profile);
   if (len + 1 >= sizeof(buf))
     return EINA_FALSE;

   ok = ecore_file_mkpath(buf);
   if (!ok)
     {
        ERR("Problem acessing Elementary's user configuration directory: %s",
            buf);
        return EINA_FALSE;
     }

   if (!_elm_config_profile_save())
     return EINA_FALSE;

   buf[len] = '/';
   len++;

   if (len + sizeof("base.cfg") >= sizeof(buf) - len)
     return EINA_FALSE;

   memcpy(buf + len, "base.cfg", sizeof("base.cfg"));
   len += sizeof("base.cfg") - 1;

   if (len + sizeof(".tmp") >= sizeof(buf))
     return EINA_FALSE;

   memcpy(buf2, buf, len);
   memcpy(buf2 + len, ".tmp", sizeof(".tmp"));

   ef = eet_open(buf2, EET_FILE_MODE_WRITE);
   if (!ef)
     return EINA_FALSE;

   ok = eet_data_write(ef, _config_edd, "config", _elm_config, 1);
   if (!ok)
     goto err;

   err = _elm_config_eet_close_error_get(ef, buf2);
   if (err)
     {
        ERR("%s", err);
        free((void *)err);
        goto err;
     }

   ret = ecore_file_mv(buf2, buf);
   if (!ret)
     {
        ERR("Error saving Elementary's configuration file");
        goto err;
     }

   ecore_file_unlink(buf2);
   return EINA_TRUE;

err:
   ecore_file_unlink(buf2);
   return EINA_FALSE;
}

static void
_config_update(void)
{
   Elm_Config *tcfg;

   tcfg = _config_system_load();
   if (!tcfg)
     {
        /* weird profile or something? We should probably fill
         * with hardcoded defaults, or get from default previx */
          return;
     }
#define IFCFG(v)   if ((_elm_config->config_version & 0xffff) < (v)) {
#define IFCFGELSE } else {
#define IFCFGEND  }
#define COPYVAL(x) do {_elm_config->x = tcfg->x; } while(0)
#define COPYPTR(x) do {_elm_config->x = tcfg->x; tcfg->x = NULL; } while(0)
#define COPYSTR(x) COPYPTR(x)

     /* we also need to update for property changes in the root window
      * if needed, but that will be dependent on new properties added
      * with each version */

     IFCFG(0x0003);
     COPYVAL(longpress_timeout);
     IFCFGEND;

#undef COPYSTR
#undef COPYPTR
#undef COPYVAL
#undef IFCFGEND
#undef IFCFGELSE
#undef IFCFG

     /* after updating user config, we must save */
}

static void
_env_get(void)
{
   char *s;
   double friction;

   s = getenv("ELM_ENGINE");
   if (s)
     {
        if ((!strcasecmp(s, "x11")) ||
            (!strcasecmp(s, "x")) ||
            (!strcasecmp(s, "software-x11")) ||
            (!strcasecmp(s, "software_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_X11);
        else if ((!strcasecmp(s, "opengl")) ||
                 (!strcasecmp(s, "gl")) ||
                 (!strcasecmp(s, "opengl-x11")) ||
                 (!strcasecmp(s, "opengl_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_OPENGL_X11);
        else if ((!strcasecmp(s, "x11-8")) ||
                 (!strcasecmp(s, "x8")) ||
                 (!strcasecmp(s, "software-8-x11")) ||
                 (!strcasecmp(s, "software_8_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_8_X11);
        else if ((!strcasecmp(s, "x11-16")) ||
                 (!strcasecmp(s, "x16")) ||
                 (!strcasecmp(s, "software-16-x11")) ||
                 (!strcasecmp(s, "software_16_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_16_X11);
/*
        else if ((!strcasecmp(s, "xrender")) ||
                 (!strcasecmp(s, "xr")) ||
                 (!strcasecmp(s, "xrender-x11")) ||
                 (!strcasecmp(s, "xrender_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_XRENDER_X11);
 */
        else if ((!strcasecmp(s, "fb")) ||
                 (!strcasecmp(s, "software-fb")) ||
                 (!strcasecmp(s, "software_fb")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_FB);
        else if ((!strcasecmp(s, "directfb")) ||
                 (!strcasecmp(s, "dfb")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_DIRECTFB);
        else if ((!strcasecmp(s, "sdl")) ||
                 (!strcasecmp(s, "software-sdl")) ||
                 (!strcasecmp(s, "software_sdl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_SDL);
        else if ((!strcasecmp(s, "sdl-16")) ||
                 (!strcasecmp(s, "software-16-sdl")) ||
                 (!strcasecmp(s, "software_16_sdl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_16_SDL);
        else if ((!strcasecmp(s, "opengl-sdl")) ||
                 (!strcasecmp(s, "opengl_sdl")) ||
                 (!strcasecmp(s, "gl-sdl")) ||
                 (!strcasecmp(s, "gl_sdl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_OPENGL_SDL);
        else if ((!strcasecmp(s, "opengl-cocoa")) ||
                 (!strcasecmp(s, "opengl_cocoa")) ||
                 (!strcasecmp(s, "gl-cocoa")) ||
                 (!strcasecmp(s, "gl_cocoa")))
          eina_stringshare_replace(&_elm_config->engine, ELM_OPENGL_COCOA);
        else if ((!strcasecmp(s, "gdi")) ||
                 (!strcasecmp(s, "software-gdi")) ||
                 (!strcasecmp(s, "software_gdi")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_WIN32);
        else if ((!strcasecmp(s, "wince-gdi")) ||
                 (!strcasecmp(s, "software-16-wince-gdi")) ||
                 (!strcasecmp(s, "software_16_wince_gdi")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_16_WINCE);
        else if (!strcasecmp(s, "buffer"))
          eina_stringshare_replace(&_elm_config->engine, ELM_BUFFER);
        else if ((!strncmp(s, "shot:", 5)))
          eina_stringshare_replace(&_elm_config->engine, s);
        else if ((!strcasecmp(s, "ews")))
          eina_stringshare_replace(&_elm_config->engine, ELM_EWS);
     }

   s = getenv("ELM_VSYNC");
   if (s) _elm_config->vsync = !!atoi(s);

   s = getenv("ELM_THUMBSCROLL_ENABLE");
   if (s) _elm_config->thumbscroll_enable = !!atoi(s);
   s = getenv("ELM_THUMBSCROLL_THRESHOLD");
   if (s) _elm_config->thumbscroll_threshold = atoi(s);
   // FIXME: floatformat locale issues here 1.0 vs 1,0 - should just be 1.0
   s = getenv("ELM_THUMBSCROLL_MOMENTUM_THRESHOLD");
   if (s) _elm_config->thumbscroll_momentum_threshold = atof(s);
   s = getenv("ELM_THUMBSCROLL_FRICTION");
   if (s) _elm_config->thumbscroll_friction = atof(s);
   s = getenv("ELM_THUMBSCROLL_BOUNCE_ENABLE");
   if (s) _elm_config->thumbscroll_bounce_enable = !!atoi(s);
   s = getenv("ELM_THUMBSCROLL_BOUNCE_FRICTION");
   if (s) _elm_config->thumbscroll_bounce_friction = atof(s);
   s = getenv("ELM_PAGE_SCROLL_FRICTION");
   if (s) _elm_config->page_scroll_friction = atof(s);
   s = getenv("ELM_BRING_IN_SCROLL_FRICTION");
   if (s) _elm_config->bring_in_scroll_friction = atof(s);
   s = getenv("ELM_ZOOM_FRICTION");
   if (s) _elm_config->zoom_friction = atof(s);
   s = getenv("ELM_THUMBSCROLL_BORDER_FRICTION");
   if (s)
     {
        friction = atof(s);
        if (friction < 0.0)
          friction = 0.0;

        if (friction > 1.0)
          friction = 1.0;

        _elm_config->thumbscroll_border_friction = friction;
     }
   s = getenv("ELM_THUMBSCROLL_SENSITIVITY_FRICTION");
   if (s)
     {
        friction = atof(s);
        if (friction < 0.1)
          friction = 0.1;

        if (friction > 1.0)
          friction = 1.0;

        _elm_config->thumbscroll_sensitivity_friction = friction;
     }
   s = getenv("ELM_SCROLL_SMOOTH_AMOUNT");
   if (s) _elm_config->scroll_smooth_amount = atof(s);
   s = getenv("ELM_SCROLL_SMOOTH_HISTORY_WEIGHT");
   if (s) _elm_config->scroll_smooth_history_weight = atof(s);
   s = getenv("ELM_SCROLL_SMOOTH_FUTURE_TIME");
   if (s) _elm_config->scroll_smooth_future_time = atof(s);
   s = getenv("ELM_SCROLL_SMOOTH_TIME_WINDOW");
   if (s) _elm_config->scroll_smooth_time_window = atof(s);
   s = getenv("ELM_THEME");
   if (s) eina_stringshare_replace(&_elm_config->theme, s);

   s = getenv("ELM_FONT_HINTING");
   if (s)
     {
        if      (!strcasecmp(s, "none")) _elm_config->font_hinting = 0;
        else if (!strcasecmp(s, "auto"))
          _elm_config->font_hinting = 1;
        else if (!strcasecmp(s, "bytecode"))
          _elm_config->font_hinting = 2;
     }

   s = getenv("ELM_FONT_PATH");
   if (s)
     {
        const char *p, *pp;
        char *buf2;

        EINA_LIST_FREE(_elm_config->font_dirs, p)
          {
             eina_stringshare_del(p);
          }

        buf2 = alloca(strlen(s) + 1);
        p = s;
        pp = p;
        for (;; )
          {
             if ((*p == ':') || (*p == 0))
               {
                  int len;

                  len = p - pp;
                  strncpy(buf2, pp, len);
                  buf2[len] = 0;
                  _elm_config->font_dirs =
                    eina_list_append(_elm_config->font_dirs,
                                     eina_stringshare_add(buf2));
                  if (*p == 0) break;
                  p++;
                  pp = p;
               }
             else
               {
                  if (*p == 0) break;
                  p++;
               }
          }
     }

   s = getenv("ELM_IMAGE_CACHE");
   if (s) _elm_config->image_cache = atoi(s);

   s = getenv("ELM_FONT_CACHE");
   if (s) _elm_config->font_cache = atoi(s);

   s = getenv("ELM_SCALE");
   if (s) _elm_config->scale = atof(s);

   _elm_config->finger_size =
     (double)_elm_config->finger_size * _elm_config->scale;
   s = getenv("ELM_FINGER_SIZE");
   if (s) _elm_config->finger_size = atoi(s);

   s = getenv("ELM_PASSWORD_SHOW_LAST");
   if (s) _elm_config->password_show_last = !!atoi(s);

   s = getenv("ELM_PASSWORD_SHOW_LAST_TIMEOUT");
   if (s)
     {
        double pw_show_last_timeout = atof(s);
        if (pw_show_last_timeout >= 0.0)
          _elm_config->password_show_last_timeout = pw_show_last_timeout;
     }

   s = getenv("ELM_FPS");
   if (s) _elm_config->fps = atof(s);
   if (_elm_config->fps < 1.0) _elm_config->fps = 1.0;

   s = getenv("ELM_MODULES");
   if (s) eina_stringshare_replace(&_elm_config->modules, s);

   /* Get RTL orientation from system */
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALE_DIR);
   _elm_config->is_mirrored = !strcmp(E_("default:LTR"), "default:RTL");

   s = getenv("ELM_TOOLTIP_DELAY");
   if (s)
     {
        double delay = atof(s);
        if (delay >= 0.0)
          _elm_config->tooltip_delay = delay;
     }

   s = getenv("ELM_CURSOR_ENGINE_ONLY");
   if (s) _elm_config->cursor_engine_only = !!atoi(s);

   s = getenv("ELM_FOCUS_HIGHLIGHT_ENABLE");
   if (s) _elm_config->focus_highlight_enable = !!atoi(s);

   s = getenv("ELM_FOCUS_HIGHLIGHT_ANIMATE");
   if (s) _elm_config->focus_highlight_animate = !!atoi(s);

   s = getenv("ELM_TOOLBAR_SHRINK_MODE");
   if (s) _elm_config->toolbar_shrink_mode = atoi(s);

   s = getenv("ELM_FILESELECTOR_EXPAND_ENABLE");
   if (s) _elm_config->fileselector_expand_enable = !!atoi(s);

   s = getenv("ELM_INWIN_DIALOGS_ENABLE");
   if (s) _elm_config->inwin_dialogs_enable = !!atoi(s);

   s = getenv("ELM_ICON_SIZE");
   if (s) _elm_config->icon_size = atoi(s);

   s = getenv("ELM_LONGPRESS_TIMEOUT");
   if (s) _elm_config->longpress_timeout = atof(s);
   if (_elm_config->longpress_timeout < 0.0)
     _elm_config->longpress_timeout = 0.0;

   s = getenv("ELM_EFFECT_ENABLE");
   if (s) _elm_config->effect_enable = !!atoi(s);

   s = getenv("ELM_DESKTOP_ENTRY");
   if (s) _elm_config->desktop_entry = !!atoi(s);
   s = getenv("ELM_ACCESS_MODE");
   if (s) _elm_config->access_mode = ELM_ACCESS_MODE_ON;
}

EAPI Eina_Bool
elm_mirrored_get(void)
{
   return _elm_config->is_mirrored;
}

EAPI void
elm_mirrored_set(Eina_Bool mirrored)
{
   _elm_config->is_mirrored = mirrored;
   _elm_rescale();
}

static void
_translation_init()
{
#ifdef ENABLE_NLS
   const char *cur_dom = textdomain(NULL);
   const char *trans_comment = gettext("");
   const char *msg_locale = setlocale(LC_MESSAGES, NULL);

   /* Same concept as what glib does:
    * We shouldn't translate if there are no translations for the
    * application in the current locale + domain. (Unless locale is
    * en_/C where translating only parts of the interface make some
    * sense).
    */
   _elm_config->translate = !(strcmp (cur_dom, "messages") &&
         !*trans_comment && strncmp (msg_locale, "en_", 3) &&
         strcmp (msg_locale, "C"));
#endif
}

void
_elm_config_init(void)
{
   if (!ELM_EVENT_CONFIG_ALL_CHANGED)
      ELM_EVENT_CONFIG_ALL_CHANGED = ecore_event_type_new();
   _desc_init();
   _profile_fetch_from_conf();
   _config_load();
   _translation_init();
   _env_get();
   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_recache();
}

void
_elm_config_sub_shutdown(void)
{
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
   if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
       ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
       ENGINE_COMPARE(ELM_XRENDER_X11) ||
       ENGINE_COMPARE(ELM_OPENGL_X11) ||
       ENGINE_COMPARE(ELM_OPENGL_COCOA))
#undef ENGINE_COMPARE
     {
#ifdef HAVE_ELEMENTARY_X
        ecore_x_disconnect();
#endif
     }
}

void
_elm_config_sub_init(void)
{
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
   if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
       ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
       ENGINE_COMPARE(ELM_XRENDER_X11) ||
       ENGINE_COMPARE(ELM_OPENGL_X11) ||
       ENGINE_COMPARE(ELM_OPENGL_COCOA))
#undef ENGINE_COMPARE
     {
#ifdef HAVE_ELEMENTARY_X
        if (!ecore_x_init(NULL))
          {
             ERR("Cannot connect to X11 display. check $DISPLAY variable");
             exit(1);
          }
        _root_1st = ecore_x_window_root_first_get();

        if (!ecore_x_screen_is_composited(0))
          _elm_config->compositing = 0;

        ecore_x_atoms_get(_atom_names, ATOM_COUNT, _atom);
        ecore_x_event_mask_set(_root_1st,
                               ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
        _prop_change_handler = ecore_event_handler_add
            (ECORE_X_EVENT_WINDOW_PROPERTY, _prop_change, NULL);
        if (!getenv("ELM_PROFILE"))
          {
             char *s;

             s = ecore_x_window_prop_string_get(_root_1st,
                                                _atom[ATOM_E_PROFILE]);
             if (s)
               {
                  int changed = 0;

                  if (_elm_profile)
                    {
                       if (strcmp(_elm_profile, s)) changed = 1;
                       free(_elm_profile);
                    }
                  _elm_profile = s;
                  if (changed) _prop_config_get();
               }
          }
#endif
     }
   _config_sub_apply();
}

void
_elm_config_reload(void)
{
   _config_free();
   _config_load();
   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_rescale();
   _elm_recache();
}

void
_elm_config_engine_set(const char *engine)
{
   if (_elm_config->engine && strcmp(_elm_config->engine, engine))
     eina_stringshare_del(_elm_config->engine);

   _elm_config->engine = eina_stringshare_add(engine);
}

void
_elm_config_all_update(void)
{
#ifdef HAVE_ELEMENTARY_X
   if (_prop_all_update_timer) ecore_timer_del(_prop_all_update_timer);
   _prop_all_update_timer = ecore_timer_add(0.1, _prop_all_update_cb, NULL);
   _prop_config_set();
   ecore_x_window_prop_string_set(_root_1st, _atom[ATOM_E_PROFILE],
                                  _elm_profile);
#endif
}

void
_elm_config_profile_set(const char *profile)
{
   Eina_Bool changed = EINA_FALSE;

   if (_elm_profile)
     {
        if (strcmp(_elm_profile, profile))
          changed = 1;
        free(_elm_profile);
     }

   _elm_profile = strdup(profile);

   if (changed)
     {
        _config_free();
        _config_load();
        _config_apply();
        _elm_config_font_overlay_apply();
        _elm_rescale();
        _elm_recache();
     }
}

void
_elm_config_shutdown(void)
{
#ifdef HAVE_ELEMENTARY_X
   if (_prop_all_update_timer)
     {
        ecore_timer_del(_prop_all_update_timer);
        _prop_all_update_cb(NULL);
     }
   _prop_all_update_timer = NULL;
   if (_prop_change_delay_timer) ecore_timer_del(_prop_change_delay_timer);
   _prop_change_delay_timer = NULL;
#endif

#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
   if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
       ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
       ENGINE_COMPARE(ELM_XRENDER_X11) ||
       ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
     {
#ifdef HAVE_ELEMENTARY_X
        ecore_event_handler_del(_prop_change_handler);
        _prop_change_handler = NULL;
#endif
     }
   _config_free();
   if (_elm_profile)
     {
        free(_elm_profile);
        _elm_profile = NULL;
     }
   _desc_shutdown();
}

