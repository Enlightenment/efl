#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include <pwd.h>

EAPI int ELM_EVENT_CONFIG_ALL_CHANGED = 0;

Elm_Config *_elm_config = NULL;
char *_elm_profile = NULL;
static Eet_Data_Descriptor *_config_edd = NULL;
static Eet_Data_Descriptor *_config_font_overlay_edd = NULL;
static Eet_Data_Descriptor *_config_color_edd = NULL;
static Eet_Data_Descriptor *_config_color_palette_edd = NULL;
static Eet_Data_Descriptor *_config_color_overlay_edd = NULL;
static Eet_Data_Descriptor *_config_bindings_widget_edd = NULL;
static Eet_Data_Descriptor *_config_binding_key_edd = NULL;
static Eet_Data_Descriptor *_config_binding_modifier_edd = NULL;
const char *_elm_preferred_engine = NULL;
const char *_elm_accel_preference = NULL;
Eina_List  *_font_overlays_del = NULL;
Eina_List  *_color_overlays_del = NULL;

static Ecore_Poller *_elm_cache_flush_poller = NULL;
static void _elm_config_key_binding_hash(void);

Eina_Hash *_elm_key_bindings = NULL;

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
   "psl1ght",
   "wayland_shm",
   "wayland_egl",
   "drm",
   NULL
};

/* whenever you want to add a new text class support into Elementary,
   declare it both here and in the (default) theme */
static const Elm_Text_Class _elm_text_classes[] = {
   {"button", "Button"},
   {"label", "Label"},
   {"entry", "Entry"},
   {"title_bar", "Title Bar"},
   {"list_item", "List Items"},
   {"grid_item", "Grid Items"},
   {"toolbar_item", "Toolbar Items"},
   {"menu_item", "Menu Items"},
   {NULL, NULL}
};

/* whenever you want to add a new class class support into Elementary,
   declare it both here and in the (default) theme */
static const Elm_Color_Class _elm_color_classes[] = {
   {"button_text", "Button Text"},
   {"button_text_disabled", "Button Disabled Text"},
   {"button_text_anchor", "Anchor Button Text"},
   {"button_text_anchor_disabled", "Anchor Button Disabled Text"},
   {"hoversel_item_active", "Hoversel Item Text"},
   {"hoversel_text_disabled", "Hoversel Item Disabled Text"},
   {"radio_text", "Radio Text"},
   {"frame", "Frame Text"},
   {"entry", "Entry Text"},
   {"check_text", "Check Text"},
   {"check_on_text", "Check On Text"},
   {"check_off_text", "Check Off Text"},
   {"list_item_base", "List Item Base"},
   {"list_item_base_odd", "List Odd Item Base"},
   {"list_item_disabled", "List Item Disabled Base"},
   {"list_item_selected", "List Item Selected Base"},
   {"grid_item", "Grid Item Text"},
   {"grid_item_disabled", "Grid Item Disabled Text"},
   {"grid_item_selected", "Grid Item Selected Text"},
   {"toolbar_item", "Toolbar Item Text"},
   {"toolbar_item_disabled", "Toolbar Item Disabled Text"},
   {"toolbar_item_selected", "Toolbar Item Selected Text"},
   {"toolbar_item_active", "Toolbar Item Active Text"},
   {"slider_text", "Slider Text"},
   {"slider_text_disabled", "Slider Disabled Text"},
   {"slider_indicator", "Slider Indicator Text"},
   {"progressbar_text", "Progressbar Text"},
   {"progressbar_text_disabled", "Progressbar Disabled Text"},
   {"progressbar_status", "Progressbar Status Text"},
   {"bubble_text", "Bubble Text"},
   {"bubble_info", "Bubble Info Text"},
   {"menu_item_active", "Menu Item Text"},
   {"menu_item_disabled", "Menu Item Disabled Text"},
   {"border_title", "Border Title Text"},
   {"border_title_active", "Border Title Active Text"},
   {"datetime_text", "Datetime Text"},
   {"multibuttonentry_label", "Multibuttonentry Text"},
   {"spinner", "Spinner Text"},
   {"spinner_disabled", "Spinner Disabled Text"},
   {NULL, NULL}
};

static void        _config_free(Elm_Config *cfg);
static void        _config_apply(void);
static void        _config_sub_apply(void);
static void        _config_update(void);
static void        _env_get(void);
static void        _color_overlays_cancel(void);

#define ELM_CONFIG_VAL(edd, type, member, dtype) \
  EET_DATA_DESCRIPTOR_ADD_BASIC(edd, type, #member, member, dtype)
#define ELM_CONFIG_LIST(edd, type, member, eddtype) \
  EET_DATA_DESCRIPTOR_ADD_LIST(edd, type, #member, member, eddtype)

#ifdef HAVE_ELEMENTARY_X
static Ecore_Event_Handler *_prop_change_handler = NULL;
static Ecore_Timer *_prop_change_delay_timer = NULL;
static Ecore_X_Window _config_win = 0;
#define ATOM_COUNT 3
static Ecore_X_Atom _atom[ATOM_COUNT];
static Ecore_X_Atom _atom_config = 0;
static const char *_atom_names[ATOM_COUNT] =
{
   "ELM_PROFILE",
   "ELM_CONFIG",
   "ELM_CONFIG_WIN"
};
#define ATOM_E_PROFILE    0
#define ATOM_E_CONFIG     1
#define ATOM_E_CONFIG_WIN 2

static Eina_Bool _prop_config_get(void);
static void      _prop_config_set(void);
static Eina_Bool _prop_change(void *data  EINA_UNUSED,
                              int ev_type EINA_UNUSED,
                              void       *ev);

static void
_elm_font_overlays_del_free(void)
{
   char *text_class;
   Eina_List *l;
   EINA_LIST_FOREACH(_font_overlays_del, l, text_class)
     eina_stringshare_del(text_class);
   _font_overlays_del = eina_list_free(_font_overlays_del);
}

static void
_elm_config_font_overlays_cancel(void)
{
   Elm_Font_Overlay *efd;
   Eina_List *l;
   EINA_LIST_FOREACH(_elm_config->font_overlays, l, efd)
     edje_text_class_del(efd->text_class);
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
   if (!ecore_x_window_prop_property_get(_config_win,
                                         atom, _atom[ATOM_E_CONFIG],
                                         8, &data, &size))
     {
        if (!ecore_x_window_prop_property_get(_config_win,
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

   _elm_config_font_overlays_cancel();
   _color_overlays_cancel();
   _config_free(_elm_config);
   _elm_config = NULL;
   _elm_config = config_data;
   _env_get();
   _config_apply();
   _config_sub_apply();
   _elm_config_font_overlay_apply();
   _elm_config_color_overlay_apply();
   _elm_rescale();
   _elm_recache();
   _elm_clouseau_reload();
   _elm_config_key_binding_hash();
   _elm_win_access(_elm_config->access_mode);
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

        ecore_x_window_prop_property_set(_config_win, _atom_config,
                                         _atom[ATOM_E_CONFIG], 8,
                                         config_data, size);
        free(config_data);
     }
}

static Eina_Bool
_prop_change_delay_cb(void *data EINA_UNUSED)
{
   char *s;

   if (!getenv("ELM_PROFILE"))
     {
        s = ecore_x_window_prop_string_get(_config_win, _atom[ATOM_E_PROFILE]);
        if (s)
          {
             free(_elm_profile);
             _elm_profile = s;
          }
     }
   _prop_config_get();
   _prop_change_delay_timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_prop_change(void *data  EINA_UNUSED,
             int ev_type EINA_UNUSED,
             void       *ev)
{
   Ecore_X_Event_Window_Property *event = ev;

   if (event->win == _config_win)
     {
        if (event->atom == _atom[ATOM_E_PROFILE])
          {
             ecore_timer_del(_prop_change_delay_timer);
             _prop_change_delay_timer = ecore_timer_add(0.1, _prop_change_delay_cb, NULL);
          }
        else if (((_atom_config > 0) && (event->atom == _atom_config)) ||
                 (event->atom == _atom[ATOM_E_CONFIG]))
          {
             ecore_timer_del(_prop_change_delay_timer);
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
        ERR("EEEK! eet_data_descriptor_file_new() failed.");
        return;
     }

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Font_Overlay);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_font_overlay_edd = eet_data_descriptor_stream_new(&eddc);
   if (!_config_font_overlay_edd)
     {
        ERR("EEEK! eet_data_descriptor_stream_new() failed.");
        eet_data_descriptor_free(_config_edd);
        return;
     }

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Color_RGBA);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_color_edd = eet_data_descriptor_stream_new(&eddc);
   if (!_config_color_edd)
     {
        ERR("EEEK! eet_data_descriptor_stream_new() failed.");
        eet_data_descriptor_free(_config_edd);
        return;
     }

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Custom_Palette);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_color_palette_edd = eet_data_descriptor_stream_new(&eddc);
   if (!_config_color_palette_edd)
     {
        ERR("EEEK! eet_data_descriptor_stream_new() failed.");
        eet_data_descriptor_free(_config_edd);
        return;
     }

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Color_Overlay);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_color_overlay_edd = eet_data_descriptor_stream_new(&eddc);
   if (!_config_color_overlay_edd)
     {
        ERR("EEEK! eet_data_descriptor_stream_new() failed.");
        eet_data_descriptor_free(_config_edd);
        return;
     }

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Config_Bindings_Widget);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_bindings_widget_edd = eet_data_descriptor_stream_new(&eddc);
   if (!_config_bindings_widget_edd)
     {
        ERR("EEEK! eet_data_descriptor_stream_new() failed.");
        eet_data_descriptor_free(_config_edd);
        return;
     }

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Config_Binding_Key);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_binding_key_edd = eet_data_descriptor_stream_new(&eddc);
   if (!_config_binding_key_edd)
     {
        ERR("EEEK! eet_data_descriptor_stream_new() failed.");
        eet_data_descriptor_free(_config_edd);
        return;
     }

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Config_Binding_Modifier);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_binding_modifier_edd = eet_data_descriptor_stream_new(&eddc);
   if (!_config_binding_modifier_edd)
     {
        ERR("EEEK! eet_data_descriptor_stream_new() failed.");
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

#define T Elm_Color_RGBA
#define D _config_color_edd
   ELM_CONFIG_VAL(D, T, r, EET_T_UINT);
   ELM_CONFIG_VAL(D, T, g, EET_T_UINT);
   ELM_CONFIG_VAL(D, T, b, EET_T_UINT);
   ELM_CONFIG_VAL(D, T, a, EET_T_UINT);
#undef T
#undef D

#define T Elm_Custom_Palette
#define D _config_color_palette_edd
   ELM_CONFIG_VAL(D, T, palette_name, EET_T_STRING);
   ELM_CONFIG_LIST(D, T, color_list, _config_color_edd);
#undef T
#undef D

#define T Elm_Color_Overlay
#define D _config_color_overlay_edd
   ELM_CONFIG_VAL(D, T, color_class, EET_T_STRING);
   ELM_CONFIG_VAL(D, T, color.r, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, color.g, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, color.b, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, color.a, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, outline.r, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, outline.g, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, outline.b, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, outline.a, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, shadow.r, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, shadow.g, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, shadow.b, EET_T_UCHAR);
   ELM_CONFIG_VAL(D, T, shadow.a, EET_T_UCHAR);
#undef T
#undef D

#define T        Elm_Config_Bindings_Widget
#define D        _config_bindings_widget_edd
   ELM_CONFIG_VAL(D, T, name, EET_T_STRING);
   ELM_CONFIG_LIST(D, T, key_bindings, _config_binding_key_edd);
#undef T
#undef D

#define T        Elm_Config_Binding_Key
#define D        _config_binding_key_edd
   ELM_CONFIG_VAL(D, T, context, EET_T_INT);
   ELM_CONFIG_VAL(D, T, key, EET_T_STRING);
   ELM_CONFIG_VAL(D, T, action, EET_T_STRING);
   ELM_CONFIG_VAL(D, T, params, EET_T_STRING);
   ELM_CONFIG_LIST(D, T, modifiers, _config_binding_modifier_edd);
#undef T
#undef D

#define T        Elm_Config_Binding_Modifier
#define D        _config_binding_modifier_edd
   ELM_CONFIG_VAL(D, T, mod, EET_T_STRING);
   ELM_CONFIG_VAL(D, T, flag, EET_T_UCHAR);
#undef T
#undef D

#define T Elm_Config
#define D _config_edd
   ELM_CONFIG_VAL(D, T, config_version, T_INT);
   ELM_CONFIG_VAL(D, T, engine, T_STRING);
   ELM_CONFIG_VAL(D, T, vsync, T_UCHAR);
   ELM_CONFIG_VAL(D, T, thumbscroll_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, thumbscroll_threshold, T_INT);
   ELM_CONFIG_VAL(D, T, thumbscroll_hold_threshold, T_INT);
   ELM_CONFIG_VAL(D, T, thumbscroll_momentum_threshold, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_flick_distance_tolerance, T_INT);
   ELM_CONFIG_VAL(D, T, thumbscroll_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_min_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_friction_standard, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_bounce_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_border_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_sensitivity_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_acceleration_threshold, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_acceleration_time_limit, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_acceleration_weight, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, page_scroll_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, bring_in_scroll_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, zoom_friction, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, thumbscroll_bounce_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, scroll_smooth_start_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, scroll_smooth_time_interval, T_DOUBLE);
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
   ELM_CONFIG_VAL(D, T, focus_highlight_clip_disable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, focus_move_policy, T_UCHAR);
   ELM_CONFIG_VAL(D, T, item_select_on_focus_disable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, toolbar_shrink_mode, T_INT);
   ELM_CONFIG_VAL(D, T, fileselector_expand_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, fileselector_double_tap_navigation_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, inwin_dialogs_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, icon_size, T_INT);
   ELM_CONFIG_VAL(D, T, longpress_timeout, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, effect_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, desktop_entry, T_UCHAR);
   ELM_CONFIG_VAL(D, T, password_show_last, T_UCHAR);
   ELM_CONFIG_VAL(D, T, password_show_last_timeout, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_zoom_finger_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, glayer_zoom_finger_factor, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_zoom_wheel_factor, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_zoom_distance_tolerance, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_rotate_finger_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, glayer_rotate_angular_tolerance, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_line_min_length, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_line_distance_tolerance, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_line_angular_tolerance, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_flick_time_limit_ms, T_INT);
   ELM_CONFIG_VAL(D, T, glayer_long_tap_start_timeout, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, glayer_double_tap_timeout, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, access_mode, T_UCHAR);
   ELM_CONFIG_VAL(D, T, selection_clear_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, glayer_continues_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, week_start, T_INT);
   ELM_CONFIG_VAL(D, T, weekend_start, T_INT);
   ELM_CONFIG_VAL(D, T, weekend_len, T_INT);
   ELM_CONFIG_VAL(D, T, year_min, T_INT);
   ELM_CONFIG_VAL(D, T, year_max, T_INT);
   ELM_CONFIG_LIST(D, T, color_overlays, _config_color_overlay_edd);
   ELM_CONFIG_LIST(D, T, color_palette, _config_color_palette_edd);
   ELM_CONFIG_VAL(D, T, softcursor_mode, T_UCHAR);
   ELM_CONFIG_VAL(D, T, auto_norender_withdrawn, T_UCHAR);
   ELM_CONFIG_VAL(D, T, auto_norender_iconified_same_as_withdrawn, T_UCHAR);
   ELM_CONFIG_VAL(D, T, auto_flush_withdrawn, T_UCHAR);
   ELM_CONFIG_VAL(D, T, auto_dump_withdrawn, T_UCHAR);
   ELM_CONFIG_VAL(D, T, auto_throttle, T_UCHAR);
   ELM_CONFIG_VAL(D, T, auto_throttle_amount, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, indicator_service_0, T_STRING);
   ELM_CONFIG_VAL(D, T, indicator_service_90, T_STRING);
   ELM_CONFIG_VAL(D, T, indicator_service_180, T_STRING);
   ELM_CONFIG_VAL(D, T, indicator_service_270, T_STRING);
   ELM_CONFIG_VAL(D, T, disable_external_menu, T_UCHAR);
   ELM_CONFIG_VAL(D, T, clouseau_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, magnifier_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, magnifier_scale, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, audio_mute_effect, T_UCHAR);
   ELM_CONFIG_VAL(D, T, audio_mute_background, T_UCHAR);
   ELM_CONFIG_VAL(D, T, audio_mute_music, T_UCHAR);
   ELM_CONFIG_VAL(D, T, audio_mute_foreground, T_UCHAR);
   ELM_CONFIG_VAL(D, T, audio_mute_interface, T_UCHAR);
   ELM_CONFIG_VAL(D, T, audio_mute_input, T_UCHAR);
   ELM_CONFIG_VAL(D, T, audio_mute_alert, T_UCHAR);
   ELM_CONFIG_VAL(D, T, audio_mute_all, T_UCHAR);
   ELM_CONFIG_LIST(D, T, bindings, _config_bindings_widget_edd);
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

   if (_config_color_edd)
     {
        eet_data_descriptor_free(_config_color_edd);
        _config_color_edd = NULL;
     }

   if (_config_color_palette_edd)
     {
        eet_data_descriptor_free(_config_color_palette_edd);
        _config_color_palette_edd = NULL;
     }

   if (_config_color_overlay_edd)
     {
        eet_data_descriptor_free(_config_color_overlay_edd);
        _config_color_overlay_edd = NULL;
     }

   if (_config_bindings_widget_edd)
     {
        eet_data_descriptor_free(_config_bindings_widget_edd);
        _config_bindings_widget_edd = NULL;
     }

   if (_config_binding_key_edd)
     {
        eet_data_descriptor_free(_config_binding_key_edd);
        _config_binding_key_edd = NULL;
     }

   if (_config_binding_modifier_edd)
     {
        eet_data_descriptor_free(_config_binding_modifier_edd);
        _config_binding_modifier_edd = NULL;
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

size_t
_elm_config_user_dir_snprintf(char       *dst,
                              size_t      size,
                              const char *fmt,
                              ...)
{
   const char *home = NULL;
   size_t user_dir_len = 0, off = 0;
   va_list ap;

#ifdef _WIN32
   home = evil_homedir_get();
   user_dir_len = eina_str_join_len
     (dst, size, '/', home, strlen(home),
         ELEMENTARY_BASE_DIR, sizeof(ELEMENTARY_BASE_DIR) - 1);
#else
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
#ifdef DOXDG
        home = getenv("XDG_CONFIG_HOME");
        if (home)
          {
             user_dir_len = eina_str_join_len
             (dst, size, '/', home, strlen(home),
                 "elementary", sizeof("elementary") - 1);
          }
        else
#endif
          {
             home = getenv("HOME");
             if (!home) home = "/";
#ifdef DOXDG
             user_dir_len = eina_str_join_len
             (dst, size, '/', home, strlen(home),
                 ".config", sizeof(".config") - 1,
                 "elementary", sizeof("elementary") - 1);
#else
             user_dir_len = eina_str_join_len
             (dst, size, '/', home, strlen(home),
                 ELEMENTARY_BASE_DIR, sizeof(ELEMENTARY_BASE_DIR) - 1);
#endif
          }
     }
#if !defined(HAVE_GETUID) || !defined(HAVE_GETEUID)
   else
     {
        struct passwd *pw = getpwent();

        if ((!pw) || (!pw->pw_dir)) goto end;
#ifdef DOXDG
        user_dir_len = eina_str_join_len
          (dst, size, '/', pw->pw_dir, strlen(pw->pw_dir),
           ".config", sizeof(".config") - 1,
           "elementary", sizeof("elementary") - 1);
#else
        user_dir_len = eina_str_join_len
          (dst, size, '/', pw->pw_dir, strlen(pw->pw_dir),
           ELEMENTARY_BASE_DIR, sizeof(ELEMENTARY_BASE_DIR) - 1);
#endif
     }
#endif
#endif

   off = user_dir_len + 1;
   if (off >= size) goto end;

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

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s", prof);

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

Eina_Bool _elm_config_access_get(void)
{
   return _elm_config->access_mode;
}

void _elm_config_access_set(Eina_Bool is_access)
{
   is_access = !!is_access;
   if (_elm_config->access_mode == is_access) return;
   _elm_config->access_mode = is_access;
   _elm_win_access(is_access);

   if (!is_access) _elm_access_shutdown();
}

Eina_Bool _elm_config_selection_unfocused_clear_get(void)
{
   return _elm_config->selection_clear_enable;
}

void _elm_config_selection_unfocused_clear_set(Eina_Bool enabled)
{
   enabled = !!enabled;
   if (_elm_config->selection_clear_enable == enabled) return;
   _elm_config->selection_clear_enable = enabled;
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

        eina_stringshare_del(efd->font);
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
        if (!efd->text_class) continue;
        if (strcmp(efd->text_class, text_class)) continue;

        _font_overlays_del =
           eina_list_append(_font_overlays_del,
                            eina_stringshare_add(text_class));
        _elm_config->font_overlays =
          eina_list_remove_list(_elm_config->font_overlays, l);
        eina_stringshare_del(efd->text_class);
        eina_stringshare_del(efd->font);
        free(efd);

        return;
     }
}

void
_elm_config_font_overlay_apply(void)
{
   Elm_Font_Overlay *efd;
   Eina_List *l;
   char *text_class;
   EINA_LIST_FOREACH(_font_overlays_del, l, text_class)
     {
        edje_text_class_del(text_class);
        eina_stringshare_del(text_class);
     }
   _font_overlays_del = eina_list_free(_font_overlays_del);

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
        if (!tc) continue;

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
_elm_config_color_classes_get(void)
{
   Eina_List *ret = NULL;
   int i;

   for (i = 0; _elm_color_classes[i].desc; i++)
     {
        Elm_Color_Class *cc;
        cc = malloc(sizeof(*cc));
        if (!cc) continue;

        *cc = _elm_color_classes[i];

        ret = eina_list_append(ret, cc);
     }

   return ret;
}

void
_elm_config_color_classes_free(Eina_List *l)
{
   Elm_Color_Class *cc;

   EINA_LIST_FREE(l, cc)
     free(cc);
}

static void
_color_overlays_cancel(void)
{
   Elm_Color_Overlay *ecd;
   Eina_List *l;
   EINA_LIST_FOREACH(_elm_config->color_overlays, l, ecd)
     edje_color_class_del(ecd->color_class);
}

Eina_List *
_elm_config_color_overlays_list(void)
{
   return _elm_config->color_overlays;
}

void
_elm_config_color_overlay_set(const char *color_class,
                              int r, int g, int b, int a,
                              int r2, int g2, int b2, int a2,
                              int r3, int g3, int b3, int a3)
{
   Elm_Color_Overlay *ecd;
   Eina_List *l;

#define CHECK_COLOR_VAL(v) v = (v > 255)? 255 : (v < 0)? 0: v
   CHECK_COLOR_VAL(r);
   CHECK_COLOR_VAL(g);
   CHECK_COLOR_VAL(b);
   CHECK_COLOR_VAL(a);
   CHECK_COLOR_VAL(r2);
   CHECK_COLOR_VAL(g2);
   CHECK_COLOR_VAL(b2);
   CHECK_COLOR_VAL(a2);
   CHECK_COLOR_VAL(r3);
   CHECK_COLOR_VAL(g3);
   CHECK_COLOR_VAL(b3);
   CHECK_COLOR_VAL(a3);
#undef CHECK_COLOR_VAL

   EINA_LIST_FOREACH(_elm_config->color_overlays, l, ecd)
     {
        if (strcmp(ecd->color_class, color_class))
          continue;

        ecd->color.r = r;
        ecd->color.g = g;
        ecd->color.b = b;
        ecd->color.a = a;
        ecd->outline.r = r2;
        ecd->outline.g = g2;
        ecd->outline.b = b2;
        ecd->outline.a = a2;
        ecd->shadow.r = r3;
        ecd->shadow.g = g3;
        ecd->shadow.b = b3;
        ecd->shadow.a = a3;

        _elm_config->color_overlays =
           eina_list_promote_list(_elm_config->color_overlays, l);
        return;
     }

   /* the color class doesn't exist */
   ecd = calloc(1, sizeof(Elm_Color_Overlay));
   if (!ecd) return;

   ecd->color_class = eina_stringshare_add(color_class);
   ecd->color.r = r;
   ecd->color.g = g;
   ecd->color.b = b;
   ecd->color.a = a;
   ecd->outline.r = r2;
   ecd->outline.g = g2;
   ecd->outline.b = b2;
   ecd->outline.a = a2;
   ecd->shadow.r = r3;
   ecd->shadow.g = g3;
   ecd->shadow.b = b3;
   ecd->shadow.a = a3;

   _elm_config->color_overlays =
      eina_list_prepend(_elm_config->color_overlays, ecd);
}

void
_elm_config_color_overlay_remove(const char *color_class)
{
   Elm_Color_Overlay *ecd;
   Eina_List *l;

   EINA_LIST_FOREACH(_elm_config->color_overlays, l, ecd)
     {
        if (!ecd->color_class) continue;
        if (strcmp(ecd->color_class, color_class)) continue;

        _color_overlays_del =
           eina_list_append(_color_overlays_del,
                            eina_stringshare_add(color_class));
        _elm_config->color_overlays =
          eina_list_remove_list(_elm_config->color_overlays, l);
        eina_stringshare_del(ecd->color_class);
        free(ecd);

        return;
     }
}

void
_elm_config_color_overlay_apply(void)
{
   Elm_Color_Overlay *ecd;
   Eina_List *l;
   char *color_class;

   EINA_LIST_FREE(_color_overlays_del, color_class)
     {
        edje_color_class_del(color_class);
        eina_stringshare_del(color_class);
     }

   EINA_LIST_FOREACH(_elm_config->color_overlays, l, ecd)
     edje_color_class_set(ecd->color_class,
                ecd->color.r, ecd->color.g, ecd->color.b, ecd->color.a,
                ecd->outline.r, ecd->outline.g, ecd->outline.b, ecd->outline.a,
                ecd->shadow.r, ecd->shadow.g, ecd->shadow.b, ecd->shadow.a);
}

Eina_List *
_elm_config_color_list_get(const char *palette_name)
{
    Eina_List *plist;
    Elm_Custom_Palette *cpalette;
    EINA_LIST_FOREACH(_elm_config->color_palette, plist, cpalette)
      {
         if (strcmp(cpalette->palette_name, palette_name))
           continue;
         return cpalette->color_list;
      }
    return NULL;
}

void
_elm_config_color_set(const char *palette_name,
                      int r,
                      int g,
                      int b,
                      int a)
{
   Eina_List *plist;
   Elm_Custom_Palette *cpalette;
   Elm_Color_RGBA *color;
   EINA_LIST_FOREACH(_elm_config->color_palette, plist, cpalette)
     {
        if (strcmp(cpalette->palette_name, palette_name))
          continue;

        color = calloc(1, sizeof(Elm_Color_RGBA));
        color->r = r;
        color->g = g;
        color->b = b;
        color->a = a;
        cpalette->color_list = eina_list_prepend(cpalette->color_list,
                                                       color);
     }
}

void
_elm_config_colors_free(const char *palette_name)
{
   Eina_List *plist;
   Elm_Custom_Palette *cpalette;
   Elm_Color_RGBA *color;
   EINA_LIST_FOREACH(_elm_config->color_palette, plist, cpalette)
     {
        if (strcmp(cpalette->palette_name, palette_name))
          continue;

        EINA_LIST_FREE(cpalette->color_list, color)
          {
             free(color);
          }
     }
}

Eina_List *
_elm_config_profiles_list(void)
{
   Eina_File_Direct_Info *info;
   Eina_List *flist = NULL;
   Eina_Iterator *file_it;
   char buf[PATH_MAX];
   const char *dir;
   size_t len;

   len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "config");

   file_it = eina_file_stat_ls(buf);
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

   file_it = eina_file_stat_ls(buf);
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
   int len = 0, i;

   // if env var - use profile without question
   s = getenv("ELM_PROFILE");
   if (s)
     {
        _elm_profile = strdup(s);
        if (_elm_profile)
          {
             p = strchr(_elm_profile, '/');
             if (p) *p = 0;
          }
        return;
     }

   for (i = 0; i < 2; i++)
     {
        // user profile
        if (i == 0)
          _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
        // system profile
        else if (i == 1)
          _elm_data_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
        ef = eet_open(buf, EET_FILE_MODE_READ);
        if (ef)
          {
             p = eet_read(ef, "config", &len);
             if (p)
               {
                  _elm_profile = malloc(len + 1);
                  if (_elm_profile)
                    {
                       memcpy(_elm_profile, p, len);
                       _elm_profile[len] = 0;
                       free(p);
                    }
                  else free(p);
                  eet_close(ef);
                  p = strchr(_elm_profile, '/');
                  if (p) *p = 0;
                  return;
               }
             eet_close(ef);
          }
     }

   _elm_profile = strdup("default");
}

static void
_config_free(Elm_Config *cfg)
{
   Elm_Font_Overlay *fo;
   const char *fontdir;
   Elm_Color_Overlay *co;
   Elm_Custom_Palette *palette;
   Elm_Color_RGBA *color;
   char *color_class;
   Elm_Config_Bindings_Widget *wb;
   Elm_Config_Binding_Key *kb;
   Elm_Config_Binding_Modifier *mb;

   if (!cfg) return;
   EINA_LIST_FREE(cfg->font_dirs, fontdir)
     {
        eina_stringshare_del(fontdir);
     }
   eina_stringshare_del(cfg->engine);
   EINA_LIST_FREE(cfg->font_overlays, fo)
     {
        eina_stringshare_del(fo->text_class);
        eina_stringshare_del(fo->font);
        free(fo);
     }
   EINA_LIST_FREE(_color_overlays_del, color_class)
     eina_stringshare_del(color_class);
   EINA_LIST_FREE(cfg->color_overlays, co)
     {
        if (co->color_class) eina_stringshare_del(co->color_class);
        free(co);
     }
   EINA_LIST_FREE(cfg->color_palette, palette)
     {
        eina_stringshare_del(palette->palette_name);
        EINA_LIST_FREE(palette->color_list, color) free(color);
        free(palette);
     }
   EINA_LIST_FREE(cfg->bindings, wb)
     {
        eina_stringshare_del(wb->name);
        EINA_LIST_FREE(wb->key_bindings, kb)
          {
             eina_stringshare_del(kb->key);
             eina_stringshare_del(kb->action);
             eina_stringshare_del(kb->params);
             EINA_LIST_FREE(kb->modifiers, mb)
                eina_stringshare_del(mb->mod);
             free(kb);
          }
        free(wb);
     }
   eina_stringshare_del(cfg->theme);
   eina_stringshare_del(cfg->modules);
   eina_stringshare_del(cfg->indicator_service_0);
   eina_stringshare_del(cfg->indicator_service_90);
   eina_stringshare_del(cfg->indicator_service_180);
   eina_stringshare_del(cfg->indicator_service_270);
   free(cfg);
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
   edje_audio_channel_mute_set(EDJE_CHANNEL_EFFECT, _elm_config->audio_mute_effect);
   edje_audio_channel_mute_set(EDJE_CHANNEL_BACKGROUND, _elm_config->audio_mute_background);
   edje_audio_channel_mute_set(EDJE_CHANNEL_MUSIC, _elm_config->audio_mute_music);
   edje_audio_channel_mute_set(EDJE_CHANNEL_FOREGROUND, _elm_config->audio_mute_foreground);
   edje_audio_channel_mute_set(EDJE_CHANNEL_INTERFACE, _elm_config->audio_mute_interface);
   edje_audio_channel_mute_set(EDJE_CHANNEL_INPUT, _elm_config->audio_mute_input);
   edje_audio_channel_mute_set(EDJE_CHANNEL_ALERT, _elm_config->audio_mute_alert);
   edje_audio_channel_mute_set(EDJE_CHANNEL_ALL, _elm_config->audio_mute_all);
}

static Eina_Bool
_elm_cache_flush_cb(void *data EINA_UNUSED)
{
   elm_cache_all_flush();
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

   elm_cache_all_flush();

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        Evas *e = evas_object_evas_get(win);
        evas_image_cache_set(e, _elm_config->image_cache * 1024);
        evas_font_cache_set(e, _elm_config->font_cache * 1024);
     }
   edje_file_cache_set(_elm_config->edje_cache);
   edje_collection_cache_set(_elm_config->edje_collection_cache);

   ELM_SAFE_FREE(_elm_cache_flush_poller, ecore_poller_del);
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

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s/base.cfg",
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

        /* set the default value if the configuration was just added and the
         * value is zero which means it was not supported before and invalid. */
        if (_elm_config->thumbscroll_min_friction == 0.0)
          _elm_config->thumbscroll_min_friction = 0.5;
        if (_elm_config->thumbscroll_friction_standard == 0.0)
          _elm_config->thumbscroll_friction_standard = 1000.0;
        if (_elm_config->thumbscroll_flick_distance_tolerance == 0)
          _elm_config->thumbscroll_flick_distance_tolerance = 1000;

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
   // why are these here? well if they are, it means you can make a gui
   // config recovery app i guess...
   _elm_config = ELM_NEW(Elm_Config);
   _elm_config->config_version = ELM_CONFIG_VERSION;
   _elm_config->engine = eina_stringshare_add("software_x11");
   _elm_config->vsync = 0;
   _elm_config->thumbscroll_enable = EINA_TRUE;
   _elm_config->thumbscroll_threshold = 24;
   _elm_config->thumbscroll_hold_threshold = 24;
   _elm_config->thumbscroll_momentum_threshold = 100.0;
   _elm_config->thumbscroll_flick_distance_tolerance = 1000;
   _elm_config->thumbscroll_friction = 1.0;
   _elm_config->thumbscroll_min_friction = 0.5;
   _elm_config->thumbscroll_friction_standard = 1000.0;
   _elm_config->thumbscroll_bounce_friction = 0.5;
   _elm_config->thumbscroll_bounce_enable = EINA_TRUE;
   _elm_config->thumbscroll_acceleration_threshold = 500.0;
   _elm_config->thumbscroll_acceleration_time_limit = 0.7;
   _elm_config->thumbscroll_acceleration_weight = 1.5;
   _elm_config->page_scroll_friction = 0.5;
   _elm_config->bring_in_scroll_friction = 0.5;
   _elm_config->zoom_friction = 0.5;
   _elm_config->thumbscroll_border_friction = 0.5;
   _elm_config->thumbscroll_sensitivity_friction = 0.25; // magic number! just trial and error shows this makes it behave "nicer" and not run off at high speed all the time
   _elm_config->scroll_smooth_start_enable = EINA_FALSE;
   _elm_config->scroll_smooth_time_interval = 0.008;
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
   _elm_config->focus_highlight_clip_disable = EINA_FALSE;
   _elm_config->focus_move_policy = ELM_FOCUS_MOVE_POLICY_CLICK;
   _elm_config->item_select_on_focus_disable = EINA_TRUE;
   _elm_config->toolbar_shrink_mode = 2;
   _elm_config->fileselector_expand_enable = EINA_FALSE;
   _elm_config->fileselector_double_tap_navigation_enable = EINA_FALSE;
   _elm_config->inwin_dialogs_enable = EINA_FALSE;
   _elm_config->icon_size = 32;
   _elm_config->longpress_timeout = 1.0;
   _elm_config->effect_enable = EINA_TRUE;
   _elm_config->desktop_entry = EINA_FALSE;
   _elm_config->is_mirrored = EINA_FALSE; /* Read sys value in env_get() */
   _elm_config->password_show_last = EINA_FALSE;
   _elm_config->password_show_last_timeout = 2.0;
   _elm_config->glayer_zoom_finger_enable = EINA_TRUE;
   _elm_config->glayer_zoom_finger_factor = 1.0;
   _elm_config->glayer_zoom_wheel_factor = 0.05;
   _elm_config->glayer_zoom_distance_tolerance = 1.0; /* 1 times elm_config_finger_size_get() */
   _elm_config->glayer_rotate_finger_enable = EINA_TRUE;
   _elm_config->glayer_rotate_angular_tolerance = 2.0; /* 2 DEG */
   _elm_config->glayer_line_min_length = 1.0;         /* 1 times elm_config_finger_size_get() */
   _elm_config->glayer_line_distance_tolerance = 3.0; /* 3 times elm_config_finger_size_get() */
   _elm_config->glayer_line_angular_tolerance = 20.0; /* 20 DEG */
   _elm_config->glayer_flick_time_limit_ms = 120;              /* ms to finish flick */
   _elm_config->glayer_long_tap_start_timeout = 1.2;   /* 1.2 second to start long-tap */
   _elm_config->glayer_double_tap_timeout = 0.25;   /* 0.25 seconds between 2 mouse downs of a tap. */
   _elm_config->glayer_continues_enable = EINA_TRUE;      /* Continue gestures default */
   _elm_config->access_mode = ELM_ACCESS_MODE_OFF;
   _elm_config->selection_clear_enable = EINA_FALSE;
   _elm_config->week_start = 1; /* monday */
   _elm_config->weekend_start = 6; /* saturday */
   _elm_config->weekend_len = 2;
   _elm_config->year_min = 2;
   _elm_config->year_max = 137;
   _elm_config->softcursor_mode = 0; /* 0 = auto, 1 = on, 2 = off */
   _elm_config->color_palette = NULL;
   _elm_config->auto_norender_withdrawn = 0;
   _elm_config->auto_norender_iconified_same_as_withdrawn = 1;
   _elm_config->auto_flush_withdrawn = 0;
   _elm_config->auto_dump_withdrawn = 0;
   _elm_config->auto_throttle = 0;
   _elm_config->auto_throttle_amount = 0.1;
   _elm_config->indicator_service_0 = eina_stringshare_add("elm_indicator_portrait");
   _elm_config->indicator_service_90 = eina_stringshare_add("elm_indicator_landscape");
   _elm_config->indicator_service_180 = eina_stringshare_add("elm_indicator_portrait");
   _elm_config->indicator_service_270 = eina_stringshare_add("elm_indicator_landscape");
   _elm_config->disable_external_menu = EINA_FALSE;
   _elm_config->magnifier_enable = EINA_TRUE;
   _elm_config->magnifier_scale = 1.5;
   _elm_config->audio_mute_effect = 0;
   _elm_config->audio_mute_background = 0;
   _elm_config->audio_mute_music = 0;
   _elm_config->audio_mute_foreground = 0;
   _elm_config->audio_mute_interface = 0;
   _elm_config->audio_mute_input = 0;
   _elm_config->audio_mute_alert = 0;
   _elm_config->audio_mute_all = 0;
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

   len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
   if (len + 1 >= sizeof(buf))
     return EINA_FALSE;

   len = _elm_config_user_dir_snprintf(buf2, sizeof(buf2),
                                       "config/profile.cfg.tmp");
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
        ERR("Error saving Elementary's configuration profile file");
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

   len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s",
                                       _elm_profile);
   if (len + 1 >= sizeof(buf))
     return EINA_FALSE;

   ok = ecore_file_mkpath(buf);
   if (!ok)
     {
        ERR("Problem accessing Elementary's user configuration directory: %s",
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
   const char *s = NULL;

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
#define COPYVAL(x) do {_elm_config->x = tcfg->x; } while (0)
#define COPYPTR(x) do {_elm_config->x = tcfg->x; tcfg->x = NULL; } while (0)
#define COPYSTR(x) COPYPTR(x)

     /* we also need to update for property changes in the root window
      * if needed, but that will be dependent on new properties added
      * with each version */

     IFCFG(0x0003);
     COPYVAL(longpress_timeout);
     IFCFGEND;

     IFCFG(0x0004);
#define PREFS_IFACE_MODULE_STR "prefs>prefs_iface"
     if (!_elm_config->modules)
       s = eina_stringshare_add(PREFS_IFACE_MODULE_STR);
     else
       {
          if (!strstr(_elm_config->modules, PREFS_IFACE_MODULE_STR))
            s = eina_stringshare_printf
            ("%s:%s", _elm_config->modules, PREFS_IFACE_MODULE_STR);
       }
     if (s)
       {
          eina_stringshare_del(_elm_config->modules);
          _elm_config->modules = s;
       }
     IFCFGEND;

     IFCFG(0x0005);
     COPYVAL(magnifier_scale);
     if (!_elm_config->bindings)
       {
          Elm_Config_Bindings_Widget *wb;
          Eina_List *l;
          
          EINA_LIST_FOREACH(tcfg->bindings, l, wb)
            {
               Elm_Config_Bindings_Widget *wb2;

               wb2 = calloc(1, sizeof(Elm_Config_Bindings_Widget));
               if (wb2)
                 {
                    Elm_Config_Binding_Key *kb;
                    Eina_List *l2;
                    
                    *wb2 = *wb;
#define DUPSHARE(x) if (wb->x) wb2->x = eina_stringshare_add(wb->x)
                    DUPSHARE(name);
#undef DUPSHARE
                    wb->key_bindings = NULL;
                    EINA_LIST_FOREACH(wb->key_bindings, l2, kb)
                      {
                         Elm_Config_Binding_Key *kb2;

                         kb2 = calloc(1, sizeof(Elm_Config_Binding_Key));
                         if (kb2)
                           {
                              Elm_Config_Binding_Modifier *mb;
                              Eina_List *l3;

#define DUPSHARE(x) if (kb->x) kb2->x = eina_stringshare_add(kb->x)
                              DUPSHARE(key);
                              DUPSHARE(action);
                              DUPSHARE(params);
#undef DUPSHARE
                              EINA_LIST_FOREACH(kb2->modifiers, l3, mb)
                                {
                                   Elm_Config_Binding_Modifier *mb2;

                                   mb2 = calloc(1, sizeof(Elm_Config_Bindings_Widget));
                                   if (mb2)
                                     {
#define DUPSHARE(x) if (mb->x) mb2->x = eina_stringshare_add(mb->x)
                                        DUPSHARE(mod);
#undef DUPSHARE
                                        kb->modifiers = eina_list_append(kb->modifiers, mb2);
                                     }
                                }
                              wb->key_bindings = eina_list_append(wb->key_bindings, kb2);
                           }
                      }
                    _elm_config->bindings = eina_list_append(_elm_config->bindings, wb2);
                 }
            }
       }
     IFCFGEND;

#undef COPYSTR
#undef COPYPTR
#undef COPYVAL
#undef IFCFGEND
#undef IFCFGELSE
#undef IFCFG

   _elm_config->config_version = ELM_CONFIG_VERSION;
   /* after updating user config, we must save */
   _config_free(tcfg);
   _elm_config_save();
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
        else if ((!strcasecmp(s, "fb")) ||
                 (!strcasecmp(s, "software-fb")) ||
                 (!strcasecmp(s, "software_fb")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_FB);
        else if ((!strcasecmp(s, "psl1ght")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_PSL1GHT);
        else if ((!strcasecmp(s, "sdl")) ||
                 (!strcasecmp(s, "software-sdl")) ||
                 (!strcasecmp(s, "software_sdl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_SDL);
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
        else if (!strcasecmp(s, "buffer"))
          eina_stringshare_replace(&_elm_config->engine, ELM_BUFFER);
        else if ((!strncmp(s, "shot:", 5)))
          eina_stringshare_replace(&_elm_config->engine, s);
        else if ((!strcasecmp(s, "ews")))
          eina_stringshare_replace(&_elm_config->engine, ELM_EWS);
        else if ((!strcasecmp(s, "wayland_shm")))
          eina_stringshare_replace(&_elm_config->engine, ELM_WAYLAND_SHM);
        else if ((!strcasecmp(s, "wayland_egl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_WAYLAND_EGL);
        else if ((!strcasecmp(s, "drm")))
          eina_stringshare_replace(&_elm_config->engine, ELM_DRM);
        else
          ERR("Unknown engine '%s'.", s);
     }

   s = getenv("ELM_VSYNC");
   if (s) _elm_config->vsync = !!atoi(s);

   s = getenv("ELM_THUMBSCROLL_ENABLE");
   if (s) _elm_config->thumbscroll_enable = !!atoi(s);
   s = getenv("ELM_THUMBSCROLL_THRESHOLD");
   if (s) _elm_config->thumbscroll_threshold = atoi(s);
   s = getenv("ELM_THUMBSCROLL_HOLD_THRESHOLD");
   if (s) _elm_config->thumbscroll_hold_threshold = atoi(s);
   // FIXME: floatformat locale issues here 1.0 vs 1,0 - should just be 1.0
   s = getenv("ELM_THUMBSCROLL_MOMENTUM_THRESHOLD");
   if (s) _elm_config->thumbscroll_momentum_threshold = _elm_atof(s);
   s = getenv("ELM_THUMBSCROLL_FLICK_DISTANCE_TOLERANCE");
   if (s) _elm_config->thumbscroll_flick_distance_tolerance = atoi(s);
   s = getenv("ELM_THUMBSCROLL_FRICTION");
   if (s) _elm_config->thumbscroll_friction = _elm_atof(s);
   s = getenv("ELM_THUMBSCROLL_MIN_FRICTION");
   if (s) _elm_config->thumbscroll_min_friction = _elm_atof(s);
   s = getenv("ELM_THUMBSCROLL_FRICTION_STANDARD");
   if (s) _elm_config->thumbscroll_friction_standard = _elm_atof(s);
   s = getenv("ELM_THUMBSCROLL_BOUNCE_ENABLE");
   if (s) _elm_config->thumbscroll_bounce_enable = !!atoi(s);
   s = getenv("ELM_THUMBSCROLL_BOUNCE_FRICTION");
   if (s) _elm_config->thumbscroll_bounce_friction = _elm_atof(s);
   s = getenv("ELM_THUMBSCROLL_ACCELERATION_THRESHOLD");
   if (s) _elm_config->thumbscroll_acceleration_threshold = _elm_atof(s);
   s = getenv("ELM_THUMBSCROLL_ACCELERATION_TIME_LIMIT");
   if (s) _elm_config->thumbscroll_acceleration_time_limit = _elm_atof(s);
   s = getenv("ELM_THUMBSCROLL_ACCELERATION_WEIGHT");
   if (s) _elm_config->thumbscroll_acceleration_weight = _elm_atof(s);
   s = getenv("ELM_PAGE_SCROLL_FRICTION");
   if (s) _elm_config->page_scroll_friction = _elm_atof(s);
   s = getenv("ELM_BRING_IN_SCROLL_FRICTION");
   if (s) _elm_config->bring_in_scroll_friction = _elm_atof(s);
   s = getenv("ELM_ZOOM_FRICTION");
   if (s) _elm_config->zoom_friction = _elm_atof(s);
   s = getenv("ELM_THUMBSCROLL_BORDER_FRICTION");
   if (s)
     {
        friction = _elm_atof(s);
        if (friction < 0.0)
          friction = 0.0;

        if (friction > 1.0)
          friction = 1.0;

        _elm_config->thumbscroll_border_friction = friction;
     }
   s = getenv("ELM_THUMBSCROLL_SENSITIVITY_FRICTION");
   if (s)
     {
        friction = _elm_atof(s);
        if (friction < 0.1)
          friction = 0.1;

        if (friction > 1.0)
          friction = 1.0;

        _elm_config->thumbscroll_sensitivity_friction = friction;
     }
   s = getenv("ELM_SCROLL_SMOOTH_START_ENABLE");
   if (s) _elm_config->scroll_smooth_start_enable = !!atoi(s);
   s = getenv("ELM_SCROLL_SMOOTH_TIME_INTERVAL");
   if (s) _elm_config->scroll_smooth_time_interval = atof(s);
   s = getenv("ELM_SCROLL_SMOOTH_AMOUNT");
   if (s) _elm_config->scroll_smooth_amount = _elm_atof(s);
   s = getenv("ELM_SCROLL_SMOOTH_HISTORY_WEIGHT");
   if (s) _elm_config->scroll_smooth_history_weight = _elm_atof(s);
   s = getenv("ELM_SCROLL_SMOOTH_FUTURE_TIME");
   if (s) _elm_config->scroll_smooth_future_time = _elm_atof(s);
   s = getenv("ELM_SCROLL_SMOOTH_TIME_WINDOW");
   if (s) _elm_config->scroll_smooth_time_window = _elm_atof(s);
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
   if (s) _elm_config->scale = _elm_atof(s);

   s = getenv("ELM_FINGER_SIZE");
   if (s) _elm_config->finger_size = atoi(s);

   s = getenv("ELM_PASSWORD_SHOW_LAST");
   if (s) _elm_config->password_show_last = !!atoi(s);

   s = getenv("ELM_PASSWORD_SHOW_LAST_TIMEOUT");
   if (s)
     {
        double pw_show_last_timeout = _elm_atof(s);
        if (pw_show_last_timeout >= 0.0)
          _elm_config->password_show_last_timeout = pw_show_last_timeout;
     }

   s = getenv("ELM_FPS");
   if (s) _elm_config->fps = _elm_atof(s);
   if (_elm_config->fps < 1.0) _elm_config->fps = 1.0;

   s = getenv("ELM_MODULES");
   if (s) eina_stringshare_replace(&_elm_config->modules, s);

   s = getenv("ELM_TOOLTIP_DELAY");
   if (s)
     {
        double delay = _elm_atof(s);
        if (delay >= 0.0)
          _elm_config->tooltip_delay = delay;
     }

   s = getenv("ELM_CURSOR_ENGINE_ONLY");
   if (s) _elm_config->cursor_engine_only = !!atoi(s);

   s = getenv("ELM_FOCUS_HIGHLIGHT_ENABLE");
   if (s) _elm_config->focus_highlight_enable = !!atoi(s);

   s = getenv("ELM_FOCUS_HIGHLIGHT_ANIMATE");
   if (s) _elm_config->focus_highlight_animate = !!atoi(s);

   s = getenv("ELM_FOCUS_HIGHLIGHT_CLIP_DISABLE");
   if (s) _elm_config->focus_highlight_clip_disable = !!atoi(s);

   s = getenv("ELM_FOCUS_MOVE_POLICY");
   if (s) _elm_config->focus_move_policy = !!atoi(s);

   s = getenv("ELM_ITEM_SELECT_ON_FOCUS_DISABLE");
   if (s) _elm_config->item_select_on_focus_disable = !!atoi(s);

   s = getenv("ELM_TOOLBAR_SHRINK_MODE");
   if (s) _elm_config->toolbar_shrink_mode = atoi(s);

   s = getenv("ELM_FILESELECTOR_EXPAND_ENABLE");
   if (s) _elm_config->fileselector_expand_enable = !!atoi(s);

   s = getenv("ELM_FILESELECTOR_DOUBLE_TAP_NAVIGATION_ENABLE");
   if (s) _elm_config->fileselector_double_tap_navigation_enable = !!atoi(s);

   s = getenv("ELM_INWIN_DIALOGS_ENABLE");
   if (s) _elm_config->inwin_dialogs_enable = !!atoi(s);

   s = getenv("ELM_ICON_SIZE");
   if (s) _elm_config->icon_size = atoi(s);

   s = getenv("ELM_LONGPRESS_TIMEOUT");
   if (s) _elm_config->longpress_timeout = _elm_atof(s);
   if (_elm_config->longpress_timeout < 0.0)
     _elm_config->longpress_timeout = 0.0;

   s = getenv("ELM_EFFECT_ENABLE");
   if (s) _elm_config->effect_enable = !!atoi(s);

   s = getenv("ELM_DESKTOP_ENTRY");
   if (s) _elm_config->desktop_entry = !!atoi(s);
   s = getenv("ELM_ACCESS_MODE");
   if (s) _elm_config->access_mode = ELM_ACCESS_MODE_ON;

   s = getenv("ELM_SELECTION_CLEAR_ENABLE");
   if (s) _elm_config->selection_clear_enable = !!atoi(s);

   s = getenv("ELM_AUTO_THROTTLE");
   if (s) _elm_config->auto_throttle = EINA_TRUE;
   s = getenv("ELM_AUTO_THROTTLE_AMOUNT");
   if (s) _elm_config->auto_throttle_amount = _elm_atof(s);
   s = getenv("ELM_AUTO_NORENDER_WITHDRAWN");
   if (s) _elm_config->auto_norender_withdrawn = EINA_TRUE;
   s = getenv("ELM_AUTO_NORENDER_ICONIFIED_SAME_AS_WITHDRAWN");
   if (s) _elm_config->auto_norender_iconified_same_as_withdrawn = EINA_TRUE;
   s = getenv("ELM_AUTO_FLUSH_WITHDRAWN");
   if (s) _elm_config->auto_flush_withdrawn = EINA_TRUE;
   s = getenv("ELM_AUTO_DUMP_WIDTHDRAWN");
   if (s) _elm_config->auto_dump_withdrawn = EINA_TRUE;

   s = getenv("ELM_INDICATOR_SERVICE_0");
   if (s) eina_stringshare_replace(&_elm_config->indicator_service_0, s);
   s = getenv("ELM_INDICATOR_SERVICE_90");
   if (s) eina_stringshare_replace(&_elm_config->indicator_service_90, s);
   s = getenv("ELM_INDICATOR_SERVICE_180");
   if (s) eina_stringshare_replace(&_elm_config->indicator_service_180, s);
   s = getenv("ELM_INDICATOR_SERVICE_270");
   if (s) eina_stringshare_replace(&_elm_config->indicator_service_270, s);
   s = getenv("ELM_DISABLE_EXTERNAL_MENU");
   if (s) _elm_config->disable_external_menu = !!atoi(s);

   s = getenv("ELM_CLOUSEAU");
   if (s) _elm_config->clouseau_enable = atoi(s);
   s = getenv("ELM_MAGNIFIER_ENABLE");
   if (s) _elm_config->magnifier_enable = !!atoi(s);
   s = getenv("ELM_MAGNIFIER_SCALE");
   if (s) _elm_config->magnifier_scale = _elm_atof(s);
}

static void
_elm_config_key_binding_hash(void)
{
   Elm_Config_Bindings_Widget *wb;
   Eina_List *l;

   if (_elm_key_bindings)
     eina_hash_free(_elm_key_bindings);

   _elm_key_bindings = eina_hash_string_superfast_new(NULL);
   EINA_LIST_FOREACH(_elm_config->bindings, l, wb)
     {
        eina_hash_add(_elm_key_bindings, wb->name, wb->key_bindings);
     }
}

Eina_Bool
_elm_config_modifier_check(const Evas_Modifier *m,
                           Eina_List *mod_list)
{
   Eina_List *l;
   Elm_Config_Binding_Modifier *mod;
   EINA_LIST_FOREACH(mod_list, l, mod)
     {
        if ((evas_key_modifier_is_set(m, mod->mod)) ^ (mod->flag))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
_elm_config_key_binding_call(Evas_Object *obj,
                             const Evas_Event_Key_Down *ev,
                             const Elm_Action *actions)
{
   Elm_Config_Binding_Key *binding;
   Eina_List *binding_list, *l;
   int i = 0;

   binding_list = eina_hash_find(_elm_key_bindings, elm_widget_type_get(obj));

   if (binding_list)
     {
        EINA_LIST_FOREACH(binding_list, l, binding)
          {
             if (binding->key && (!strcmp(binding->key, ev->key))
                 && _elm_config_modifier_check(ev->modifiers, binding->modifiers))
               {
                  while (actions[i].name)
                    {
                       if (!strcmp(binding->action, actions[i].name))
                         return actions[i].func(obj, binding->params);
                       i++;
                    }
                  break;
               }
          }
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
elm_config_mirrored_get(void)
{
   return _elm_config->is_mirrored;
}

EAPI void
elm_config_mirrored_set(Eina_Bool mirrored)
{
   mirrored = !!mirrored;
   if (_elm_config->is_mirrored == mirrored) return;
   _elm_config->is_mirrored = mirrored;
   _elm_rescale();
}

EAPI Eina_Bool
elm_config_cursor_engine_only_get(void)
{
   return _elm_config->cursor_engine_only;
}

EAPI void
elm_config_cursor_engine_only_set(Eina_Bool engine_only)
{
   engine_only = !!engine_only;
   _elm_config->cursor_engine_only = engine_only;
}

EAPI double
elm_config_tooltip_delay_get(void)
{
   return _elm_config->tooltip_delay;
}

EAPI void
elm_config_tooltip_delay_set(double delay)
{
   if (delay < 0.0) return;
   _elm_config->tooltip_delay = delay;
}

EAPI double
elm_config_scale_get(void)
{
   return _elm_config->scale;
}

EAPI void
elm_config_scale_set(double scale)
{
   if (scale < 0.0) return;
   if (_elm_config->scale == scale) return;
   _elm_config->scale = scale;
   _elm_rescale();
}

EAPI Eina_Bool
elm_config_password_show_last_get(void)
{
   return _elm_config->password_show_last;
}

EAPI void
elm_config_password_show_last_set(Eina_Bool password_show_last)
{
   if (_elm_config->password_show_last == password_show_last) return;
   _elm_config->password_show_last = password_show_last;
   edje_password_show_last_set(_elm_config->password_show_last);
}

EAPI double
elm_config_password_show_last_timeout_get(void)
{
   return _elm_config->password_show_last_timeout;
}

EAPI void
elm_config_password_show_last_timeout_set(double password_show_last_timeout)
{
   if (password_show_last_timeout < 0.0) return;
   if (_elm_config->password_show_last_timeout == password_show_last_timeout) return;
   _elm_config->password_show_last_timeout = password_show_last_timeout;
   edje_password_show_last_timeout_set(_elm_config->password_show_last_timeout);
}

EAPI Eina_Bool
elm_config_save(void)
{
   return _elm_config_save();
}

EAPI void
elm_config_reload(void)
{
   _elm_config_reload();
}

EAPI const char *
elm_config_profile_get(void)
{
   return _elm_config_current_profile_get();
}

EAPI const char *
elm_config_profile_dir_get(const char *profile,
                    Eina_Bool   is_user)
{
   return _elm_config_profile_dir_get(profile, is_user);
}

EAPI void
elm_config_profile_dir_free(const char *p_dir)
{
   free((void *)p_dir);
}

EAPI Eina_List *
elm_config_profile_list_get(void)
{
   return _elm_config_profiles_list();
}

EAPI void
elm_config_profile_list_free(Eina_List *l)
{
   const char *dir;

   EINA_LIST_FREE(l, dir)
     eina_stringshare_del(dir);
}

EAPI void
elm_config_profile_set(const char *profile)
{
   EINA_SAFETY_ON_NULL_RETURN(profile);
   _elm_config_profile_set(profile);
}

EAPI const char *
elm_config_engine_get(void)
{
   return _elm_config->engine;
}

EAPI void
elm_config_engine_set(const char *engine)
{
   EINA_SAFETY_ON_NULL_RETURN(engine);

   _elm_config_engine_set(engine);
}

EAPI Eina_List *
elm_config_text_classes_list_get(void)
{
   return _elm_config_text_classes_get();
}

EAPI void
elm_config_text_classes_list_free(Eina_List *list)
{
   _elm_config_text_classes_free(list);
}

EAPI const Eina_List *
elm_config_font_overlay_list_get(void)
{
   return _elm_config_font_overlays_list();
}

EAPI Eina_Bool
elm_config_access_get(void)
{
   return _elm_config_access_get();
}

EAPI void
elm_config_access_set(Eina_Bool is_access)
{
   _elm_config_access_set(is_access);
}

EAPI Eina_Bool
elm_config_selection_unfocused_clear_get(void)
{
   return _elm_config_selection_unfocused_clear_get();
}

EAPI void
elm_config_selection_unfocused_clear_set(Eina_Bool enabled)
{
   _elm_config_selection_unfocused_clear_set(enabled);
}

EAPI void
elm_config_font_overlay_set(const char    *text_class,
                     const char    *font,
                     Evas_Font_Size size)
{
   EINA_SAFETY_ON_NULL_RETURN(text_class);
   _elm_config_font_overlay_set(text_class, font, size);
}

EAPI void
elm_config_font_overlay_unset(const char *text_class)
{
   EINA_SAFETY_ON_NULL_RETURN(text_class);
   _elm_config_font_overlay_remove(text_class);
}

EAPI void
elm_config_font_overlay_apply(void)
{
   _elm_config_font_overlay_apply();
   _elm_rescale();
}

EAPI Eina_List *
elm_config_color_classes_list_get(void)
{
   return _elm_config_color_classes_get();
}

EAPI void
elm_config_color_classes_list_free(Eina_List *list)
{
   _elm_config_color_classes_free(list);
}

EAPI const Eina_List *
elm_config_color_overlay_list_get(void)
{
   return _elm_config_color_overlays_list();
}

EAPI void
elm_config_color_overlay_set(const char *color_class,
                             int r, int g, int b, int a,
                             int r2, int g2, int b2, int a2,
                             int r3, int g3, int b3, int a3)
{
   EINA_SAFETY_ON_NULL_RETURN(color_class);
   _elm_config_color_overlay_set(color_class,
                                 r, g, b, a,
                                 r2, g2, b2, a2,
                                 r3, g3, b3, a3);
}

EAPI void
elm_config_color_overlay_unset(const char *color_class)
{
   EINA_SAFETY_ON_NULL_RETURN(color_class);
   _elm_config_color_overlay_remove(color_class);
}

EAPI void
elm_config_color_overlay_apply(void)
{
   _elm_config_color_overlay_apply();
}

EAPI Evas_Coord
elm_config_finger_size_get(void)
{
   return _elm_config->finger_size;
}

EAPI void
elm_config_finger_size_set(Evas_Coord size)
{
   if (size < 0) return;
   if (_elm_config->finger_size == size) return;
   _elm_config->finger_size = size;
   _elm_rescale();
}

EAPI int
elm_config_cache_flush_interval_get(void)
{
   return _elm_config->cache_flush_poll_interval;
}

EAPI void
elm_config_cache_flush_interval_set(int size)
{
   EINA_SAFETY_ON_FALSE_RETURN(size > 0);
   if (_elm_config->cache_flush_poll_interval == size) return;
   _elm_config->cache_flush_poll_interval = size;

   _elm_recache();
}

EAPI Eina_Bool
elm_config_cache_flush_enabled_get(void)
{
   return _elm_config->cache_flush_enable;
}

EAPI void
elm_config_cache_flush_enabled_set(Eina_Bool enabled)
{
   enabled = !!enabled;
   if (_elm_config->cache_flush_enable == enabled) return;
   _elm_config->cache_flush_enable = enabled;

   _elm_recache();
}

EAPI int
elm_config_cache_font_cache_size_get(void)
{
   return _elm_config->font_cache;
}

EAPI void
elm_config_cache_font_cache_size_set(int size)
{
   if (size < 0) return;
   if (_elm_config->font_cache == size) return;
   _elm_config->font_cache = size;

   _elm_recache();
}

EAPI int
elm_config_cache_image_cache_size_get(void)
{
   return _elm_config->image_cache;
}

EAPI void
elm_config_cache_image_cache_size_set(int size)
{
   if (size < 0) return;
   if (_elm_config->image_cache == size) return;
   _elm_config->image_cache = size;

   _elm_recache();
}

EAPI int
elm_config_cache_edje_file_cache_size_get()
{
   return _elm_config->edje_cache;
}

EAPI void
elm_config_cache_edje_file_cache_size_set(int size)
{
   if (size < 0) return;
   if (_elm_config->edje_cache == size) return;
   _elm_config->edje_cache = size;

   _elm_recache();
}

EAPI int
elm_config_cache_edje_collection_cache_size_get(void)
{
   return _elm_config->edje_collection_cache;
}

EAPI void
elm_config_cache_edje_collection_cache_size_set(int size)
{
   if (_elm_config->edje_collection_cache == size) return;
   _elm_config->edje_collection_cache = size;

   _elm_recache();
}

EAPI Eina_Bool
elm_config_focus_highlight_enabled_get(void)
{
   return _elm_config->focus_highlight_enable;
}

EAPI void
elm_config_focus_highlight_enabled_set(Eina_Bool enable)
{
   _elm_config->focus_highlight_enable = !!enable;
}

EAPI Eina_Bool
elm_config_focus_highlight_animate_get(void)
{
   return _elm_config->focus_highlight_animate;
}

EAPI void
elm_config_focus_highlight_animate_set(Eina_Bool animate)
{
   _elm_config->focus_highlight_animate = !!animate;
}

EAPI Eina_Bool
elm_config_focus_highlight_clip_disabled_get(void)
{
   return _elm_config->focus_highlight_clip_disable;
}

EAPI void
elm_config_focus_highlight_clip_disabled_set(Eina_Bool disable)
{
   _elm_config->focus_highlight_clip_disable = !!disable;
}

EAPI Elm_Focus_Move_Policy
elm_config_focus_move_policy_get(void)
{
   return _elm_config->focus_move_policy;
}

EAPI void
elm_config_focus_move_policy_set(Elm_Focus_Move_Policy policy)
{
   _elm_config->focus_move_policy = policy;
}

EAPI Eina_Bool
elm_config_item_select_on_focus_disabled_get(void)
{
   return _elm_config->item_select_on_focus_disable;
}

EAPI void
elm_config_item_select_on_focus_disabled_set(Eina_Bool disable)
{
   _elm_config->item_select_on_focus_disable = !!disable;
}

EAPI Eina_Bool
elm_config_scroll_bounce_enabled_get(void)
{
   return _elm_config->thumbscroll_bounce_enable;
}

EAPI void
elm_config_scroll_bounce_enabled_set(Eina_Bool enabled)
{
   _elm_config->thumbscroll_bounce_enable = enabled;
}

EAPI double
elm_config_scroll_bounce_friction_get(void)
{
   return _elm_config->thumbscroll_bounce_friction;
}

EAPI void
elm_config_scroll_bounce_friction_set(double friction)
{
   _elm_config->thumbscroll_bounce_friction = friction;
}

EAPI double
elm_config_scroll_page_scroll_friction_get(void)
{
   return _elm_config->page_scroll_friction;
}

EAPI void
elm_config_scroll_page_scroll_friction_set(double friction)
{
   _elm_config->page_scroll_friction = friction;
}

EAPI double
elm_config_scroll_bring_in_scroll_friction_get(void)
{
   return _elm_config->bring_in_scroll_friction;
}

EAPI void
elm_config_scroll_bring_in_scroll_friction_set(double friction)
{
   _elm_config->bring_in_scroll_friction = friction;
}

EAPI double
elm_config_scroll_zoom_friction_get(void)
{
   return _elm_config->zoom_friction;
}

EAPI void
elm_config_scroll_zoom_friction_set(double friction)
{
   _elm_config->zoom_friction = friction;
}

EAPI Eina_Bool
elm_config_scroll_thumbscroll_enabled_get(void)
{
   return _elm_config->thumbscroll_enable;
}

EAPI void
elm_config_scroll_thumbscroll_enabled_set(Eina_Bool enabled)
{
   _elm_config->thumbscroll_enable = enabled;
}

EAPI unsigned int
elm_config_scroll_thumbscroll_threshold_get(void)
{
   return _elm_config->thumbscroll_threshold;
}

EAPI void
elm_config_scroll_thumbscroll_threshold_set(unsigned int threshold)
{
   _elm_config->thumbscroll_threshold = threshold;
}

EAPI unsigned int
elm_config_scroll_thumbscroll_hold_threshold_get(void)
{
   return _elm_config->thumbscroll_hold_threshold;
}

EAPI void
elm_config_scroll_thumbscroll_hold_threshold_set(unsigned int threshold)
{
   _elm_config->thumbscroll_hold_threshold = threshold;
}

EAPI double
elm_config_scroll_thumbscroll_momentum_threshold_get(void)
{
   return _elm_config->thumbscroll_momentum_threshold;
}

EAPI void
elm_config_scroll_thumbscroll_momentum_threshold_set(double threshold)
{
   _elm_config->thumbscroll_momentum_threshold = threshold;
}

EAPI unsigned int
elm_config_scroll_thumbscroll_flick_distance_tolerance_get(void)
{
   return _elm_config->thumbscroll_flick_distance_tolerance;
}

EAPI void
elm_config_scroll_thumbscroll_flick_distance_tolerance_set(unsigned int distance)
{
   _elm_config->thumbscroll_flick_distance_tolerance = distance;
}

EAPI double
elm_config_scroll_thumbscroll_friction_get(void)
{
   return _elm_config->thumbscroll_friction;
}

EAPI void
elm_config_scroll_thumbscroll_friction_set(double friction)
{
   _elm_config->thumbscroll_friction = friction;
}

EAPI double
elm_config_scroll_thumbscroll_min_friction_get(void)
{
   return _elm_config->thumbscroll_min_friction;
}

EAPI void
elm_config_scroll_thumbscroll_min_friction_set(double friction)
{
   _elm_config->thumbscroll_min_friction = friction;
}

EAPI double
elm_config_scroll_thumbscroll_friction_standard_get(void)
{
   return _elm_config->thumbscroll_friction_standard;
}

EAPI void
elm_config_scroll_thumbscroll_friction_standard_set(double standard)
{
   _elm_config->thumbscroll_friction_standard = standard;
}

EAPI double
elm_config_scroll_thumbscroll_border_friction_get(void)
{
   return _elm_config->thumbscroll_border_friction;
}

EAPI void
elm_config_scroll_thumbscroll_border_friction_set(double friction)
{
   if (friction < 0.0) friction = 0.0;
   if (friction > 1.0) friction = 1.0;
   _elm_config->thumbscroll_border_friction = friction;
}

EAPI double
elm_config_scroll_thumbscroll_sensitivity_friction_get(void)
{
   return _elm_config->thumbscroll_sensitivity_friction;
}

EAPI void
elm_config_scroll_thumbscroll_sensitivity_friction_set(double friction)
{
   if (friction < 0.1) friction = 0.1;
   if (friction > 1.0) friction = 1.0;
   _elm_config->thumbscroll_sensitivity_friction = friction;
}

EAPI double
elm_config_scroll_thumbscroll_acceleration_threshold_get(void)
{
   return _elm_config->thumbscroll_acceleration_threshold;
}

EAPI void
elm_config_scroll_thumbscroll_acceleration_threshold_set(double threshold)
{
   _elm_config->thumbscroll_acceleration_threshold = threshold;
}

EAPI double
elm_config_scroll_thumbscroll_acceleration_time_limit_get(void)
{
   return _elm_config->thumbscroll_acceleration_time_limit;
}

EAPI void
elm_config_scroll_thumbscroll_acceleration_time_limit_set(double time_limit)
{
   _elm_config->thumbscroll_acceleration_time_limit = time_limit;
}

EAPI double
elm_config_scroll_thumbscroll_acceleration_weight_get(void)
{
   return _elm_config->thumbscroll_acceleration_weight;
}

EAPI void
elm_config_scroll_thumbscroll_acceleration_weight_set(double weight)
{
   _elm_config->thumbscroll_acceleration_weight = weight;
}

EAPI void
elm_config_longpress_timeout_set(double longpress_timeout)
{
   _elm_config->longpress_timeout = longpress_timeout;
}

EAPI double
elm_config_longpress_timeout_get(void)
{
   return _elm_config->longpress_timeout;
}

EAPI void
elm_config_softcursor_mode_set(Elm_Softcursor_Mode mode)
{
   _elm_config->softcursor_mode = mode;
}

EAPI Elm_Softcursor_Mode
elm_config_softcursor_mode_get(void)
{
   return _elm_config->softcursor_mode;
}

EAPI Eina_Bool
elm_config_disable_external_menu_get(void)
{
   return _elm_config->disable_external_menu;
}

EAPI void
elm_config_disable_external_menu_set(Eina_Bool disable)
{
   _elm_config->disable_external_menu = !!disable;
}

EAPI Eina_Bool
elm_config_clouseau_enabled_get(void)
{
   return _elm_config->clouseau_enable;
}

EAPI void
elm_config_clouseau_enabled_set(Eina_Bool enable)
{
   _elm_config->clouseau_enable = !!enable;
   _elm_clouseau_reload();
}

EAPI double
elm_config_glayer_long_tap_start_timeout_get(void)
{
   return _elm_config->glayer_long_tap_start_timeout;
}

EAPI void
elm_config_glayer_long_tap_start_timeout_set(double long_tap_timeout)
{
   _elm_config->glayer_long_tap_start_timeout = long_tap_timeout;
}

EAPI double
elm_config_glayer_double_tap_timeout_get(void)
{
   return _elm_config->glayer_double_tap_timeout;
}

EAPI void
elm_config_glayer_double_tap_timeout_set(double double_tap_timeout)
{
   _elm_config->glayer_double_tap_timeout = double_tap_timeout;
}

EAPI Eina_Bool
elm_config_magnifier_enable_get(void)
{
   return _elm_config->magnifier_enable;
}

EAPI void
elm_config_magnifier_enable_set(Eina_Bool enable)
{
   _elm_config->magnifier_enable = !!enable;
}

EAPI double
elm_config_magnifier_scale_get(void)
{
   return _elm_config->magnifier_scale;
}

EAPI void
elm_config_magnifier_scale_set(double scale)
{
   _elm_config->magnifier_scale = scale;
}

EAPI Eina_Bool
elm_config_audio_mute_get(Edje_Channel channel)
{
   switch (channel)
     {
      case EDJE_CHANNEL_EFFECT:
        return _elm_config->audio_mute_effect;
        break;
      case EDJE_CHANNEL_BACKGROUND:
        return _elm_config->audio_mute_background;
        break;
      case EDJE_CHANNEL_MUSIC:
        return _elm_config->audio_mute_music;
        break;
      case EDJE_CHANNEL_FOREGROUND:
        return _elm_config->audio_mute_foreground;
        break;
      case EDJE_CHANNEL_INTERFACE:
        return _elm_config->audio_mute_interface;
        break;
      case EDJE_CHANNEL_INPUT:
        return _elm_config->audio_mute_input;
        break;
      case EDJE_CHANNEL_ALERT:
        return _elm_config->audio_mute_alert;
        break;
      case EDJE_CHANNEL_ALL:
        return _elm_config->audio_mute_all;
        break;
      default:
        break;
     }
   return EINA_FALSE;
}

EAPI void
elm_config_audio_mute_set(Edje_Channel channel, Eina_Bool mute)
{
   switch (channel)
     {
      case EDJE_CHANNEL_EFFECT:
        _elm_config->audio_mute_effect = mute;
        break;
      case EDJE_CHANNEL_BACKGROUND:
        _elm_config->audio_mute_background = mute;
        break;
      case EDJE_CHANNEL_MUSIC:
        _elm_config->audio_mute_music = mute;
        break;
      case EDJE_CHANNEL_FOREGROUND:
        _elm_config->audio_mute_foreground = mute;
        break;
      case EDJE_CHANNEL_INTERFACE:
        _elm_config->audio_mute_interface = mute;
        break;
      case EDJE_CHANNEL_INPUT:
        _elm_config->audio_mute_input = mute;
        break;
      case EDJE_CHANNEL_ALERT:
        _elm_config->audio_mute_alert = mute;
        break;
      case EDJE_CHANNEL_ALL:
        _elm_config->audio_mute_all = mute;
        break;
      default:
        break;
     }
   edje_audio_channel_mute_set(channel, mute);
}

EAPI void
elm_config_all_flush(void)
{
#ifdef HAVE_ELEMENTARY_X
   _prop_config_set();
   ecore_x_window_prop_string_set(_config_win, _atom[ATOM_E_PROFILE],
                                  _elm_profile);
#endif
}

static void
_translation_init()
{
#ifdef ENABLE_NLS

   /* How does it decide translation with current domain??
      Application could use their own text domain.
      This is insane to me.

   const char *cur_dom = textdomain(NULL);
   const char *trans_comment = gettext("");
   const char *msg_locale = setlocale(LC_MESSAGES, NULL);
   */

   /* Same concept as what glib does:
    * We shouldn't translate if there are no translations for the
    * application in the current locale + domain. (Unless locale is
    * en_/C where translating only parts of the interface make some
    * sense).
    */
   /*
      _elm_config->translate = !(strcmp (cur_dom, "messages") &&
      !*trans_comment && strncmp (msg_locale, "en_", 3) &&
      strcmp (msg_locale, "C"));
    */
   /* Get RTL orientation from system */
   if (_elm_config->translate)
     {
        bindtextdomain(PACKAGE, LOCALE_DIR);
        _elm_config->is_mirrored = !strcmp(E_("default:LTR"), "default:RTL");
     }

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
   _env_get();
   ELM_SAFE_FREE(_elm_preferred_engine, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_accel_preference, eina_stringshare_del);
   _translation_init();
   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_config_color_overlay_apply();
   _elm_recache();
   _elm_clouseau_reload();
   _elm_config_key_binding_hash();
}

void
_elm_config_sub_shutdown(void)
{
#ifdef HAVE_ELEMENTARY_X
   ELM_SAFE_FREE(_prop_change_delay_timer, ecore_timer_del);
#endif

#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
   if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
       ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
       ENGINE_COMPARE(ELM_XRENDER_X11) ||
       ENGINE_COMPARE(ELM_OPENGL_X11) ||
       ENGINE_COMPARE(ELM_OPENGL_COCOA))
#undef ENGINE_COMPARE
     {
#ifdef HAVE_ELEMENTARY_X
        ecore_x_shutdown();
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
        if (ecore_x_init(NULL))
          {
             Ecore_X_Window win = 0, win2 = 0, root;

             if (!ecore_x_screen_is_composited(0))
               _elm_config->compositing = 0;
             ecore_x_atoms_get(_atom_names, ATOM_COUNT, _atom);
             root = ecore_x_window_root_first_get();
             if (ecore_x_window_prop_window_get(root,
                                                _atom[ATOM_E_CONFIG_WIN],
                                                &win, 1) == 1)
               {
                  if (ecore_x_window_prop_window_get(win,
                                                     _atom[ATOM_E_CONFIG_WIN],
                                                     &win2, 1) == 1)
                    {
                       if (win2 == win) _config_win = win;
                    }
               }
             if (_config_win == 0)
               _config_win = ecore_x_window_permanent_new
                             (root, _atom[ATOM_E_CONFIG_WIN]);

             ecore_x_event_mask_set(_config_win,
                                    ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
             _prop_change_handler = ecore_event_handler_add
               (ECORE_X_EVENT_WINDOW_PROPERTY, _prop_change, NULL);
             if (!getenv("ELM_PROFILE"))
               {
                  char *s;

                  s = ecore_x_window_prop_string_get(_config_win,
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
                       s = strchr(_elm_profile, '/');
                       if (s) *s = 0;
                    }
               }
          }
        else
          ERR("Cannot connect to X11 display. check $DISPLAY variable");
#endif
     }
   _config_sub_apply();
}

void
_elm_config_reload(void)
{
   _config_free(_elm_config);
   _elm_config = NULL;
   _config_load();
   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_config_color_overlay_apply();
   _elm_rescale();
   _elm_recache();
   _elm_clouseau_reload();
   _elm_config_key_binding_hash();
   ecore_event_add(ELM_EVENT_CONFIG_ALL_CHANGED, NULL, NULL, NULL);
}

void
_elm_config_engine_set(const char *engine)
{
   if (_elm_config->engine && strcmp(_elm_config->engine, engine))
     eina_stringshare_del(_elm_config->engine);

   _elm_config->engine = eina_stringshare_add(engine);
}

EAPI const char *
elm_config_preferred_engine_get(void)
{
   return _elm_preferred_engine;
}

EAPI void
elm_config_preferred_engine_set(const char *engine)
{
   if (engine)
     eina_stringshare_replace(&(_elm_preferred_engine), engine);
   else
     {
        ELM_SAFE_FREE(_elm_preferred_engine, eina_stringshare_del);
     }
}

EAPI const char *
elm_config_accel_preference_get(void)
{
   return _elm_accel_preference;
}

EAPI void
elm_config_accel_preference_set(const char *pref)
{
   if (pref)
     eina_stringshare_replace(&(_elm_accel_preference), pref);
   else
     {
        ELM_SAFE_FREE(_elm_accel_preference, eina_stringshare_del);
     }
}

EAPI const char *
elm_config_indicator_service_get(int rotation)
{
   switch (rotation)
     {
      case 0:
        return _elm_config->indicator_service_0;
      case 90:
        return _elm_config->indicator_service_90;
      case 180:
        return _elm_config->indicator_service_180;
      case 270:
        return _elm_config->indicator_service_270;
      default:
        return NULL;
     }
}

void
_elm_config_profile_set(const char *profile)
{
   if (!profile) return;

   if (_elm_profile)
     {
        if (!strcmp(_elm_profile, profile))
          return;

        free(_elm_profile);
     }

   _elm_profile = strdup(profile);

   _color_overlays_cancel();
   _config_free(_elm_config);
   _elm_config = NULL;
   _config_load();
   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_config_color_overlay_apply();
   _elm_rescale();
   _elm_recache();
   _elm_clouseau_reload();
   _elm_config_key_binding_hash();
}

void
_elm_config_shutdown(void)
{
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
   if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
       ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
       ENGINE_COMPARE(ELM_XRENDER_X11) ||
       ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
     {
#ifdef HAVE_ELEMENTARY_X
        ELM_SAFE_FREE(_prop_change_handler, ecore_event_handler_del);
#endif
     }
   ELM_SAFE_FREE(_elm_config, _config_free);
   ELM_SAFE_FREE(_elm_preferred_engine, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_accel_preference, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_profile, free);

#ifdef HAVE_ELEMENTARY_X
   _elm_font_overlays_del_free();
#endif

   _desc_shutdown();

   if (_elm_key_bindings)
     eina_hash_free(_elm_key_bindings);
}
