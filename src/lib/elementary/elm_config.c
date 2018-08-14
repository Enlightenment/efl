#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include <pwd.h>

#include "efl_config_global.eo.h"

EAPI int ELM_EVENT_CONFIG_ALL_CHANGED = 0;
EAPI void __efl_internal_elm_config_set(Efl_Config *cfg);

Elm_Config *_elm_config = NULL;
Efl_Config *_efl_config_obj = NULL;
static char *_elm_profile = NULL;
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
static const char *_elm_gl_preference = NULL;
static Eina_List  *_font_overlays_del = NULL;
static Eina_List  *_color_overlays_del = NULL;

static Ecore_Poller *_elm_cache_flush_poller = NULL;
static void _elm_config_key_binding_hash(void);

Eina_Bool _config_profile_lock = EINA_FALSE;
static Ecore_Timer *_config_change_delay_timer = NULL;
static Ecore_Timer *_config_profile_change_delay_timer = NULL;
static Ecore_Event_Handler *_monitor_file_created_handler = NULL;
static Ecore_Event_Handler *_monitor_directory_created_handler = NULL;
static Eio_Monitor *_eio_config_monitor = NULL;
static Eio_Monitor *_eio_profile_monitor = NULL;

Eina_Hash *_elm_key_bindings = NULL;

#ifdef HAVE_ELEMENTARY_WL2
Ecore_Wl2_Display *_elm_wl_display = NULL;
#endif

const char *_elm_engines[] = {
   "software_x11",
   "fb",
   "opengl_x11",
   "software_gdi",
   "sdl",
   "opengl_sdl",
   "buffer",
   "ews",
   "opengl_cocoa",
   "wayland_shm",
   "wayland_egl",
   "drm",
   "ddraw",
   NULL
};

/* whenever you want to add a new text class support into Elementary,
   declare it both here and in the (default) theme */
static const Elm_Text_Class _elm_text_classes[] = {
   {"button", "Button"},
   {"calendar_year_text", "Year Text in Title Area"},
   {"calendar_month_text", "Month Text in Title Area"},
   {"calendar_weekday_text", "Weekday Text"},
   {"calendar_day_text", "Day Text"},
   {"calendar_day_text_holiday", "Holiday Text"},
   {"calendar_day_text_today", "Today Text"},
   {"calendar_day_text_disabled", "Disabled Day Text"},
   {"datetime_separator_text", "Datetime Separator Text"},
   {"datetime_separator_text_disabled", "Datetime Separator Disabled Text"},
   {"label", "Label"},
   {"entry_text", "Entry Text"},
   {"entry_text_disabled", "Entry Disabled Text"},
   {"entry_guide_text", "Entry Guide Text"},
   {"entry", "Entry"},
   {"index_highlight_text", "Index Highlight Text"},
   {"index_item_text", "Index Items Text"},
   {"index_item_text_selected", "Index Selected Items Text"},
   {"multibuttonentry_item_text", "Multibuttonentry Items"},
   {"multibuttonentry_item_text_pressed", "Multibuttonentry Pressed Items"},
   {"multibuttonentry_item_text_disabled", "Multibuttonentry Disabled Items"},
   {"tags_item_text", "Tags Items"},
   {"tags_item_text_pressed", "Tags Pressed Items"},
   {"tags_item_text_disabled", "Tags Disabled Items"},
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
   {"calendar_year_text", "Year Text in Title Area"},
   {"calendar_month_text", "Month Text in Title Area"},
   {"calendar_weekday_text", "Weekday Text"},
   {"calendar_day_text", "Day Text"},
   {"calendar_day_text_holiday", "Holiday Text"},
   {"calendar_day_text_today", "Today Text"},
   {"calendar_day_text_disabled", "Disabled Day Text"},
   {"calendar_day_selected", "Selected Day Effect"},
   {"calendar_day_highlighted", "Highlighted Day Effect"},
   {"calendar_day_checked", "Checked Day Effect"},
   {"datetime_bg", "Datetime Background"},
   {"datepicker_bg", "Datepicker Background"},
   {"timepicker_bg", "Timepicker Background"},
   {"datetime_separator_text", "Datetime Separator Text"},
   {"datetime_separator_text_disabled", "Datetime Separator Disabled Text"},
   {"hoversel_item_active", "Hoversel Item Text"},
   {"hoversel_text_disabled", "Hoversel Item Disabled Text"},
   {"radio_text", "Radio Text"},
   {"frame", "Frame Text"},
   {"entry_text", "Entry Text"},
   {"entry_text_disabled", "Entry Disabled Text"},
   {"entry_guide_text", "Entry Guide Text"},
   {"entry_cursor", "Entry Cursor"},
   {"entry_selection_handler", "Entry Selection Handler"},
   {"entry_scrollframe_base", "Entry Scrollframe Base"},
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
   {"index_bg", "Index Background"},
   {"index_item_bg", "Index Item Background"},
   {"index_highlight_text", "Index Highlight Text"},
   {"index_item_text", "Index Items Text"},
   {"index_item_text_selected", "Index Selected Items Text"},
   {"toolbar_item", "Toolbar Item Text"},
   {"toolbar_item_disabled", "Toolbar Item Disabled Text"},
   {"toolbar_item_selected", "Toolbar Item Selected Text"},
   {"toolbar_item_active", "Toolbar Item Active Text"},
   {"slider_text", "Slider Text"},
   {"slider_text_disabled", "Slider Disabled Text"},
   {"slider_indicator", "Slider Indicator Text"},
   {"spinner_bg", "Spinner Background"},
   {"progressbar_text", "Progressbar Text"},
   {"progressbar_text_disabled", "Progressbar Disabled Text"},
   {"progressbar_status", "Progressbar Status Text"},
   {"bubble_text", "Bubble Text"},
   {"bubble_info", "Bubble Info Text"},
   {"menu_item_active", "Menu Item Text"},
   {"menu_item_disabled", "Menu Item Disabled Text"},
   {"multibuttonentry_bg", "Multibuttonentry Background"},
   {"multibuttonentry_item_bg", "Multibuttonentry Item Background"},
   {"multibuttonentry_item_bg_selected", "Multibuttonentry Item Selected Background"},
   {"multibuttonentry_item_text", "Multibuttonentry Item Text"},
   {"multibuttonentry_item_text_pressed", "Multibuttonentry Item Pressed Text"},
   {"multibuttonentry_item_text_disabled", "Multibuttonentry Item Disabled Text"},
   {"tags_bg", "Tags Background"},
   {"tags_item_bg", "Tags Item Background"},
   {"tags_item_bg_selected", "Tags Item Selected Background"},
   {"tags_item_text", "Tags Item Text"},
   {"tags_item_text_pressed", "Tags Item Pressed Text"},
   {"tags_item_text_disabled", "Tags Item Disabled Text"},
   {"border_title", "Border Title Text"},
   {"border_title_active", "Border Title Active Text"},
   {"datetime_text", "Datetime Text"},
   {"multibuttonentry_label", "Multibuttonentry Text"},
   {"tags_label", "Tags Text"},
   {"tags_number", "Tags Number Text"},
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
   ELM_CONFIG_VAL(D, T, accel, T_STRING);
   ELM_CONFIG_VAL(D, T, web_backend, T_STRING);
   ELM_CONFIG_VAL(D, T, accel_override, T_UCHAR);
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
   ELM_CONFIG_VAL(D, T, scroll_animation_disable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, scroll_accel_factor, T_DOUBLE);
//   ELM_CONFIG_VAL(D, T, scroll_smooth_time_interval, T_DOUBLE); // not used anymore
   ELM_CONFIG_VAL(D, T, scroll_smooth_amount, T_DOUBLE);
//   ELM_CONFIG_VAL(D, T, scroll_smooth_history_weight, T_DOUBLE); // not used anymore
//   ELM_CONFIG_VAL(D, T, scroll_smooth_future_time, T_DOUBLE); // not used anymore
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
   ELM_CONFIG_VAL(D, T, focus_autoscroll_mode, T_UCHAR);
   ELM_CONFIG_VAL(D, T, context_menu_disabled, T_UCHAR);
   ELM_CONFIG_VAL(D, T, slider_indicator_visible_mode, T_INT);
   ELM_CONFIG_VAL(D, T, item_select_on_focus_disable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, first_item_focus_on_first_focus_in, T_UCHAR);
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
   ELM_CONFIG_VAL(D, T, glayer_tap_finger_size, T_INT);
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
   ELM_CONFIG_VAL(D, T, atspi_mode, T_UCHAR);
   ELM_CONFIG_VAL(D, T, win_auto_focus_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, win_auto_focus_animate, T_UCHAR);
   ELM_CONFIG_VAL(D, T, transition_duration_factor, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, naviframe_prev_btn_auto_pushed, T_UCHAR);
   ELM_CONFIG_VAL(D, T, popup_horizontal_align, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, popup_vertical_align, T_DOUBLE);
   ELM_CONFIG_VAL(D, T, popup_scrollable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, spinner_min_max_filter_enable, T_UCHAR);
   ELM_CONFIG_VAL(D, T, icon_theme, T_STRING);
   ELM_CONFIG_VAL(D, T, entry_select_allow, T_UCHAR);
   ELM_CONFIG_VAL(D, T, offline, T_UCHAR);
   ELM_CONFIG_VAL(D, T, powersave, T_INT);
   ELM_CONFIG_VAL(D, T, drag_anim_duration, T_DOUBLE);
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
   size_t user_dir_len = 0, off = 0;
   va_list ap;
   char *tmp;
   static int use_xdg_config = -1;

   if (use_xdg_config == -1)
     {
        if (getenv("ELM_CONFIG_DIR_XDG")) use_xdg_config = 1;
        else use_xdg_config = 0;
     }
   if (use_xdg_config)
     tmp = eina_vpath_resolve("(:usr.config:)/elementary");
   else
     tmp = eina_vpath_resolve("(:home:)/" ELEMENTARY_BASE_DIR);

   eina_strlcpy(dst, tmp, size);
   free(tmp);

   user_dir_len = strlen(dst);
   off = user_dir_len + 1;
   if (off >= size) return off;
   dst[user_dir_len] = '/';
   va_start(ap, fmt);
   off = off + vsnprintf(dst + off, size - off, fmt, ap);
   va_end(ap);
   return off;
}

typedef struct _Elm_Config_Derived          Elm_Config_Derived;
typedef struct _Elm_Config_Derived_Profile  Elm_Config_Derived_Profile;

struct _Elm_Config_Derived
{
   Eina_List *profiles;
};

struct _Elm_Config_Derived_Profile
{
   const char *profile;
   const char *derive_options;
};

static Eet_Data_Descriptor *_config_derived_edd = NULL;
static Eet_Data_Descriptor *_config_derived_profile_edd = NULL;

static void
_elm_config_profile_derived_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Config_Derived);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;
   _config_derived_edd = eet_data_descriptor_file_new(&eddc);

   memset(&eddc, 0, sizeof(eddc)); /* just in case... */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Config_Derived_Profile);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;
   _config_derived_profile_edd = eet_data_descriptor_file_new(&eddc);

#define T        Elm_Config_Derived_Profile
#define D        _config_derived_profile_edd
   ELM_CONFIG_VAL(D, T, profile, EET_T_STRING);
   ELM_CONFIG_VAL(D, T, derive_options, EET_T_STRING);
#undef T
#undef D

#define T        Elm_Config_Derived
#define D        _config_derived_edd
   ELM_CONFIG_LIST(D, T, profiles, _config_derived_profile_edd);
#undef T
#undef D
}

static void
_elm_config_profile_derived_shutdown(void)
{
   if (_config_derived_profile_edd)
     {
        eet_data_descriptor_free(_config_derived_profile_edd);
        _config_derived_profile_edd = NULL;
     }
   if (_config_derived_edd)
     {
        eet_data_descriptor_free(_config_derived_edd);
        _config_derived_edd = NULL;
     }
}

static Elm_Config_Derived *
_elm_config_derived_load(const char *profile)
{
   char buf[PATH_MAX];
   Eet_File *ef;
   Elm_Config_Derived *derived;

   if (!profile) profile = _elm_profile;
   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s/derived.cfg",
                                 profile);
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        derived = eet_data_read(ef, _config_derived_edd, "config");
        eet_close(ef);
        if (derived) return derived;
     }
   snprintf(buf, sizeof(buf), "%s/config/%s/derived.cfg",
            _elm_data_dir, profile);
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        derived = eet_data_read(ef, _config_derived_edd, "config");
        eet_close(ef);
        if (derived) return derived;
     }
   return NULL;
}

static void
_elm_config_profile_derived_save(const char *profile, Elm_Config_Derived *derived)
{
   Eet_File *ef;
   char buf[PATH_MAX], buf2[PATH_MAX];
   int ret;

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s",
                                 profile ? profile : _elm_profile);
   ecore_file_mkpath(buf);
   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s/derived.cfg.tmp",
                                 profile ? profile : _elm_profile);
   _elm_config_user_dir_snprintf(buf2, sizeof(buf2), "config/%s/derived.cfg",
                                 profile ? profile : _elm_profile);
   ef = eet_open(buf, EET_FILE_MODE_WRITE);
   if (ef)
     {
        ret = eet_data_write(ef, _config_derived_edd, "config", derived, 1);
        eet_close(ef);
        if (ret)
          {
             ecore_file_mv(buf, buf2);
          }
        else
          {
             ERR("Error saving Elementary's derived configuration profile file");
          }
     }
}

static void
_elm_config_derived_free(Elm_Config_Derived *derived)
{
   Elm_Config_Derived_Profile *dp;

   if (!derived) return;
   EINA_LIST_FREE(derived->profiles, dp)
     {
        eina_stringshare_del(dp->profile);
        eina_stringshare_del(dp->derive_options);
        free(dp);
     }
   free(derived);
}

static void
_elm_config_derived_option_op_apply(Elm_Config *cfg, const char *op, const char *params)
{
   if (!strcmp(op, "scale-mul"))
     {
        int multiplier = atoi(params);
        if (multiplier > 0)
          {
             cfg->scale = cfg->scale * (((double)multiplier) / 100.0);
          }
     }
   // Add more derivation commands here
}

static void
_elm_config_derived_option_apply(Elm_Config *cfg, const char *option)
{
   const char *p;
   char *buf = alloca(strlen(option) + 1);
   char *bp = buf;

   p = option;
   for (;;)
     {
        if ((*p == 0) || (*p == ' '))
          {
             if (*p == ' ') p++;
             *bp = 0;
             _elm_config_derived_option_op_apply(cfg, buf, p);
             return;
          }
        else
          {
             *bp = *p;
             bp++;
          }
        if (*p == 0) break;
        p++;
     }
}

static void
_elm_config_derived_apply(Elm_Config *cfg, const char *derive_options)
{
   // derive_options = "option1 param param2 ...; option2 param1 ..."
   const char *p;
   char *buf = alloca(strlen(derive_options) + 1);
   char *bp = buf;

   p = derive_options;
   for (;;)
     {
        if ((*p == 0) || (*p == ';'))
          {
             if (*p == ';') p++;
             *bp = 0;
             _elm_config_derived_option_apply(cfg, buf);
             bp = buf;
          }
        else
          {
             *bp = *p;
             bp++;
          }
        if (*p == 0) break;
        p++;
     }
}

static void
_elm_config_derived_save(Elm_Config *cfg, Elm_Config_Derived *derived)
{
   Elm_Config_Derived_Profile *dp;
   Eina_List *l;

   if (!derived) return;
   EINA_LIST_FOREACH(derived->profiles, l, dp)
     {
        if ((dp->profile) && (dp->derive_options))
          {
             Elm_Config *cfg2;

             cfg2 = malloc(sizeof(Elm_Config));
             if (cfg2)
               {
                  memcpy(cfg2, cfg, sizeof(Elm_Config));
                  _elm_config_derived_apply(cfg2, dp->derive_options);
                  _elm_config_save(cfg2, dp->profile);
                  free(cfg2);
               }
          }
     }
}

EAPI void
elm_config_profile_derived_add(const char *profile, const char *derive_options)
{
   Elm_Config_Derived *derived;

   derived = _elm_config_derived_load(_elm_profile);
   if (!derived) derived = calloc(1, sizeof(Elm_Config_Derived));
   if (derived)
     {
        Elm_Config_Derived_Profile *dp = calloc(1, sizeof(Elm_Config_Derived_Profile));

        if (dp)
          {
             dp->profile = eina_stringshare_add(profile);
             dp->derive_options = eina_stringshare_add(derive_options);
             derived->profiles = eina_list_append(derived->profiles, dp);
             _elm_config_profile_derived_save(_elm_profile, derived);
             _elm_config_derived_save(_elm_config, derived);
          }
        _elm_config_derived_free(derived);
     }
}

EAPI void
elm_config_profile_derived_del(const char *profile)
{
   Elm_Config_Derived *derived;
   Elm_Config_Derived_Profile *dp;
   Eina_List *l;

   if (!profile) return;
   derived = _elm_config_derived_load(_elm_profile);
   if (derived)
     {
        EINA_LIST_FOREACH(derived->profiles, l, dp)
          {
             if ((dp->profile) && (!strcmp(dp->profile, profile)))
               {
                  char buf[PATH_MAX];

                  _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s",
                                                profile);
                  ecore_file_recursive_rm(buf);
                  derived->profiles = eina_list_remove_list(derived->profiles, l);
                  eina_stringshare_del(dp->profile);
                  eina_stringshare_del(dp->derive_options);
                  free(dp);
                  _elm_config_profile_derived_save(_elm_profile, derived);
                  _elm_config_derived_free(derived);
                  return;
               }
          }
        _elm_config_derived_free(derived);
     }
}

const char *
_elm_config_profile_dir_get(const char *prof,
                            Eina_Bool   is_user)
{
   char buf[PATH_MAX];

   if (!is_user)
     goto not_user;

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s", prof);

   // See elm_config_profile_dir_free: always use strdup+free
   if (ecore_file_is_dir(buf))
     return strdup(buf);

   return NULL;

not_user:
   snprintf(buf, sizeof(buf), "%s/config/%s", _elm_data_dir, prof);

   // See elm_config_profile_dir_free: always use strdup+free
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

static Eina_Bool _elm_config_atspi_mode_get(void)
{
   return _elm_config->atspi_mode;
}

static void _elm_config_atspi_mode_set(Eina_Bool is_enabled)
{
   _elm_config->priv.atspi_mode = EINA_TRUE;
   is_enabled = !!is_enabled;
   if (_elm_config->atspi_mode == is_enabled) return;
   _elm_config->atspi_mode = is_enabled;

   if (!is_enabled) _elm_atspi_bridge_shutdown();
   else _elm_atspi_bridge_init();
}

static Eina_Bool _elm_config_selection_unfocused_clear_get(void)
{
   return _elm_config->selection_clear_enable;
}

static void _elm_config_selection_unfocused_clear_set(Eina_Bool enabled)
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
   if (!efd) return;
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
        if (!eina_streq(ecd->color_class, color_class))
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
        if (!eina_streq(ecd->color_class, color_class)) continue;

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
        if (!color) continue;
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
_elm_config_profiles_list(Eina_Bool hide_profiles)
{
   Eina_File_Direct_Info *info;
   Eina_List *flist = NULL;
   Eina_Iterator *file_it;
   char buf[PATH_MAX];
   const char *dir;
   size_t len;

   len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "config");

   file_it = eina_file_stat_ls(buf);
   if (!file_it) goto sys;

   buf[len] = '/';
   len++;

   len = sizeof(buf) - len;

   EINA_ITERATOR_FOREACH(file_it, info)
     {
        if (info->name_length >= len) continue;
        if ((hide_profiles) && (info->path[info->name_start] == '.')) continue;

        if (info->type == EINA_FILE_DIR)
          {
             flist =
               eina_list_sorted_insert(flist, _sort_files_cb,
                                       eina_stringshare_add
                                         (info->path + info->name_start));
          }
     }

   eina_iterator_free(file_it);

sys:
   len = eina_str_join_len(buf, sizeof(buf), '/', _elm_data_dir,
                           strlen(_elm_data_dir), "config",
                           sizeof("config") - 1);

   file_it = eina_file_stat_ls(buf);
   if (!file_it) goto list_free;

   buf[len] = '/';
   len++;

   len = sizeof(buf) - len;
   EINA_ITERATOR_FOREACH(file_it, info)
     {
        if (info->name_length >= len) continue;
        if ((hide_profiles) && (info->path[info->name_start] == '.')) continue;

        switch (info->type)
          {
           case EINA_FILE_DIR:
               {
                  const Eina_List *l;
                  const char *tmp;

                  EINA_LIST_FOREACH(flist, l, tmp)
                    {
                       if (!strcmp(info->path + info->name_start, tmp)) break;
                    }
                  if (!l)
                    flist = eina_list_sorted_insert(flist, _sort_files_cb,
                                                    eina_stringshare_add
                                                    (info->path +
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
   EINA_LIST_FREE(flist, dir) eina_stringshare_del(dir);
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
             p = strchr(_elm_profile, '\\');
             if (p) *p = 0;
             if (!strcmp(_elm_profile, ".."))
               {
                  free(_elm_profile);
                  _elm_profile = NULL;
               }
             else return;
          }
     }

   for (i = 0; i < 2 && !_use_build_config; i++)
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
                       p = strchr(_elm_profile, '/');
                       if (p) *p = 0;
                    }
                  else free(p);
                  eet_close(ef);
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
   eina_stringshare_del(cfg->accel);
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
               {
                  eina_stringshare_del(mb->mod);
                  free(mb);
               }
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
   eina_stringshare_del(cfg->icon_theme);
   free(cfg);
}

static void
_config_apply(void)
{
   _elm_theme_parse(NULL, _elm_config->theme);
   ecore_animator_frametime_set(1.0 / _elm_config->fps);
   edje_password_show_last_set(_elm_config->password_show_last);
   edje_password_show_last_timeout_set(_elm_config->password_show_last_timeout);
}

static void
_config_sub_apply(void)
{
   edje_frametime_set(1.0 / _elm_config->fps);
   edje_scale_set(_elm_config->scale);
   edje_transition_duration_factor_set(_elm_config->transition_duration_factor);
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

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s",
                          _elm_profile);
   ecore_file_mkpath(buf);
   if (_eio_config_monitor) eio_monitor_del(_eio_config_monitor);
   _eio_config_monitor = eio_monitor_add(buf);

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s/base.cfg",
                          _elm_profile);

   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        cfg = eet_data_read(ef, _config_edd, "config");
        eet_close(ef);
     }

   if (cfg)
     {
        size_t len;

        len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "themes/");
        if (len + 1 < sizeof(buf))
          ecore_file_mkpath(buf);
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
   if (!ef)
     {
        _elm_data_dir_snprintf(buf, sizeof(buf), "config/default/base.cfg");

        ef = eet_open(buf, EET_FILE_MODE_READ);
     }
   if (ef)
     {
        cfg = eet_data_read(ef, _config_edd, "config");
        eet_close(ef);
     }
   return cfg;
}

static void
_efl_config_obj_del(Eo *obj EINA_UNUSED)
{
   ERR("You can not delete the global configuration object!");
}

static void
_config_load(void)
{
   if (_efl_config_obj)
     {
        efl_del_intercept_set(_efl_config_obj, NULL);
        efl_loop_unregister(efl_main_loop_get(), EFL_CONFIG_INTERFACE, _efl_config_obj);
        efl_loop_unregister(efl_main_loop_get(), EFL_CONFIG_GLOBAL_CLASS, _efl_config_obj);
        ELM_SAFE_FREE(_efl_config_obj, efl_del);
        ELM_SAFE_FREE(_elm_config, _config_free);
        _elm_font_overlays_del_free();

        ELM_SAFE_FREE(_elm_key_bindings, eina_hash_free);
     }
   _efl_config_obj = efl_add(EFL_CONFIG_GLOBAL_CLASS, efl_main_loop_get());
   efl_loop_register(efl_main_loop_get(), EFL_CONFIG_INTERFACE, _efl_config_obj);
   efl_loop_register(efl_main_loop_get(), EFL_CONFIG_GLOBAL_CLASS, _efl_config_obj);
   efl_del_intercept_set(_efl_config_obj, _efl_config_obj_del);
   if (!_use_build_config)
     {
        _elm_config = _config_user_load();
        if (_elm_config)
          {
             if ((_elm_config->config_version >> ELM_CONFIG_VERSION_EPOCH_OFFSET) < ELM_CONFIG_EPOCH)
                {
                   WRN("User's elementary config seems outdated and unusable. Fallback to load system config.");
                   _config_free(_elm_config);
                   _elm_config = NULL;
                }
             else
               {
                  if (_elm_config->config_version < ELM_CONFIG_VERSION)
                    _config_update();
                  _env_get();
                  return;
               }
          }
     }

   /* no user config, fallback for system. No need to check version for
    * this one, if it's not the right one, someone screwed up at the time
    * of installing it */
   _elm_config = _config_system_load();
   if (_elm_config)
     {
        _env_get();
        return;
     }
   /* FIXME: config load could have failed because of a non-existent
    * profile. Fallback to default before moving on */

   // config load fail - defaults
   // why are these here? well if they are, it means you can make a gui
   // config recovery app i guess...
   _elm_config = ELM_NEW(Elm_Config);
   if (!_elm_config) return;
   _elm_config->config_version = ELM_CONFIG_VERSION;
   _elm_config->engine = NULL;
   _elm_config->accel = NULL;
   _elm_config->accel_override = 0;
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
   _elm_config->scroll_smooth_start_enable = EINA_TRUE;
   _elm_config->scroll_animation_disable = EINA_FALSE;
   _elm_config->scroll_accel_factor = 7.0;
//   _elm_config->scroll_smooth_time_interval = 0.008; // not used anymore
   _elm_config->scroll_smooth_amount = 1.0;
//   _elm_config->scroll_smooth_history_weight = 0.3; // not used anymore
//   _elm_config->scroll_smooth_future_time = 0.0; // not used anymore
   _elm_config->scroll_smooth_time_window = 0.15;
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
   _elm_config->finger_size = 10;
   _elm_config->fps = 60.0;
   _elm_config->theme = eina_stringshare_add("default");
   _elm_config->modules = NULL;
   _elm_config->tooltip_delay = 1.0;
   _elm_config->cursor_engine_only = EINA_TRUE;
   _elm_config->focus_highlight_enable = EINA_FALSE;
   _elm_config->focus_highlight_animate = EINA_TRUE;
   _elm_config->focus_highlight_clip_disable = EINA_FALSE;
   _elm_config->focus_move_policy = ELM_FOCUS_MOVE_POLICY_CLICK;
   _elm_config->first_item_focus_on_first_focus_in = EINA_FALSE;
   _elm_config->item_select_on_focus_disable = EINA_TRUE;
   _elm_config->toolbar_shrink_mode = 2;
   _elm_config->fileselector_expand_enable = EINA_FALSE;
   _elm_config->fileselector_double_tap_navigation_enable = EINA_FALSE;
   _elm_config->inwin_dialogs_enable = EINA_FALSE;
   _elm_config->icon_size = 32;
   _elm_config->longpress_timeout = 1.0;
   _elm_config->effect_enable = EINA_TRUE;
   _elm_config->desktop_entry = EINA_FALSE;
   _elm_config->context_menu_disabled = EINA_FALSE;
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
   _elm_config->glayer_tap_finger_size = 10;
   _elm_config->access_mode = ELM_ACCESS_MODE_OFF;
   _elm_config->selection_clear_enable = EINA_FALSE;
   _elm_config->week_start = 1; /* monday */
   _elm_config->weekend_start = 6; /* saturday */
   _elm_config->weekend_len = 2;
   _elm_config->year_min = 70;
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
   _elm_config->spinner_min_max_filter_enable = EINA_FALSE;
   _elm_config->magnifier_scale = 1.5;
   _elm_config->audio_mute_effect = 0;
   _elm_config->audio_mute_background = 0;
   _elm_config->audio_mute_music = 0;
   _elm_config->audio_mute_foreground = 0;
   _elm_config->audio_mute_interface = 0;
   _elm_config->audio_mute_input = 0;
   _elm_config->audio_mute_alert = 0;
   _elm_config->audio_mute_all = 0;
   _elm_config->win_auto_focus_enable = 1;
   _elm_config->win_auto_focus_animate = 1;
   _elm_config->atspi_mode = ELM_ATSPI_MODE_OFF;
   _elm_config->gl_depth = 0;
   _elm_config->gl_msaa = 0;
   _elm_config->gl_stencil = 0;
   _elm_config->transition_duration_factor = 1.0;
   _elm_config->naviframe_prev_btn_auto_pushed = EINA_TRUE;
   _elm_config->popup_horizontal_align = 0.5;
   _elm_config->popup_vertical_align = 0.5;
   _elm_config->icon_theme = eina_stringshare_add(ELM_CONFIG_ICON_THEME_ELEMENTARY);
   _elm_config->popup_scrollable = EINA_FALSE;
   _elm_config->entry_select_allow = EINA_TRUE;
   _elm_config->drag_anim_duration = 0.0;
   _env_get();
}

static void
_elm_config_reload_do(void)
{
   Elm_Config *prev_config;

   prev_config = _elm_config;
   _elm_config = NULL;
   _config_load();
   if ((prev_config) && (_elm_config))
     {
#define KEEP_VAL(xxx) \
   if (prev_config->priv.xxx) { \
      _elm_config->xxx = prev_config->xxx; \
   }
#define KEEP_STR(xxx) \
   if (prev_config->priv.xxx) { \
      eina_stringshare_replace(&(_elm_config->xxx), prev_config->xxx); \
   }
        Elm_Font_Overlay *fo;
        Elm_Color_Overlay *co;
        Elm_Custom_Palette *palette;
        Elm_Color_RGBA *color;
        Elm_Config_Bindings_Widget *wb;
        Elm_Config_Binding_Key *kb;
        Elm_Config_Binding_Modifier *mb;

        KEEP_STR(engine);
        KEEP_STR(accel);
        KEEP_STR(web_backend);
        KEEP_VAL(accel_override);
        KEEP_VAL(vsync);
        KEEP_VAL(thumbscroll_enable);
        KEEP_VAL(thumbscroll_threshold);
        KEEP_VAL(thumbscroll_hold_threshold);
        KEEP_VAL(thumbscroll_momentum_threshold);
        KEEP_VAL(thumbscroll_flick_distance_tolerance);
        KEEP_VAL(thumbscroll_friction);
        KEEP_VAL(thumbscroll_min_friction);
        KEEP_VAL(thumbscroll_friction_standard);
        KEEP_VAL(thumbscroll_bounce_friction);
        KEEP_VAL(thumbscroll_acceleration_threshold);
        KEEP_VAL(thumbscroll_acceleration_time_limit);
        KEEP_VAL(thumbscroll_acceleration_weight);
        KEEP_VAL(page_scroll_friction);
        KEEP_VAL(bring_in_scroll_friction);
        KEEP_VAL(zoom_friction);
        KEEP_VAL(scroll_animation_disable);
        KEEP_VAL(scroll_accel_factor);
        KEEP_VAL(thumbscroll_bounce_enable);
        KEEP_VAL(thumbscroll_border_friction);
        KEEP_VAL(thumbscroll_sensitivity_friction);
        KEEP_VAL(scroll_smooth_start_enable);
        KEEP_VAL(scroll_smooth_amount);
        KEEP_VAL(scroll_smooth_time_window);
        KEEP_VAL(scale);
        if (prev_config->priv.font_overlays)
          {
             EINA_LIST_FREE(_elm_config->font_overlays, fo)
               {
                  eina_stringshare_del(fo->text_class);
                  eina_stringshare_del(fo->font);
                  free(fo);
               }
             _elm_config->font_overlays = prev_config->font_overlays;
             prev_config->font_overlays = NULL;
          }
        KEEP_VAL(font_hinting);
        KEEP_VAL(cache_flush_poll_interval);

        KEEP_VAL(cache_flush_enable);
        KEEP_VAL(image_cache);
        KEEP_VAL(font_cache);
        KEEP_VAL(edje_cache);
        KEEP_VAL(edje_collection_cache);
        KEEP_VAL(finger_size);
        KEEP_VAL(tooltip_delay);
        KEEP_VAL(cursor_engine_only);
        KEEP_VAL(focus_highlight_enable);
        KEEP_VAL(focus_highlight_animate);
        KEEP_VAL(focus_highlight_clip_disable);
        KEEP_VAL(focus_move_policy);
        KEEP_VAL(item_select_on_focus_disable);
        KEEP_VAL(first_item_focus_on_first_focus_in);
        KEEP_VAL(focus_autoscroll_mode);
        KEEP_VAL(slider_indicator_visible_mode);
        KEEP_VAL(popup_horizontal_align);
        KEEP_VAL(popup_vertical_align);
        KEEP_VAL(popup_scrollable);
        KEEP_VAL(toolbar_shrink_mode);
        KEEP_VAL(fileselector_expand_enable);
        KEEP_VAL(fileselector_double_tap_navigation_enable);
        KEEP_VAL(inwin_dialogs_enable);
        KEEP_VAL(icon_size);
        KEEP_VAL(longpress_timeout);
        KEEP_VAL(effect_enable);
        KEEP_VAL(desktop_entry);
        KEEP_VAL(context_menu_disabled);
        KEEP_VAL(password_show_last);
        KEEP_VAL(password_show_last_timeout);
        KEEP_VAL(glayer_zoom_finger_enable);
        KEEP_VAL(glayer_zoom_finger_factor);
        KEEP_VAL(glayer_zoom_wheel_factor);
        KEEP_VAL(glayer_zoom_distance_tolerance);
        KEEP_VAL(glayer_rotate_finger_enable);
        KEEP_VAL(glayer_rotate_angular_tolerance);
        KEEP_VAL(glayer_line_min_length);
        KEEP_VAL(glayer_line_distance_tolerance);
        KEEP_VAL(glayer_line_angular_tolerance);
        KEEP_VAL(glayer_flick_time_limit_ms);
        KEEP_VAL(glayer_long_tap_start_timeout);
        KEEP_VAL(glayer_double_tap_timeout);
        KEEP_VAL(glayer_tap_finger_size);
        KEEP_VAL(access_mode);
        KEEP_VAL(glayer_continues_enable);
        KEEP_VAL(week_start);
        KEEP_VAL(weekend_start);
        KEEP_VAL(weekend_len);
        KEEP_VAL(year_min);
        KEEP_VAL(year_max);
        if (prev_config->priv.color_overlays)
          {
             EINA_LIST_FREE(_elm_config->color_overlays, co)
               {
                  if (co->color_class) eina_stringshare_del(co->color_class);
                  free(co);
               }
             _elm_config->color_overlays = prev_config->color_overlays;
             prev_config->color_overlays = NULL;
          }
        if (prev_config->priv.color_palette)
          {
             EINA_LIST_FREE(_elm_config->color_palette, palette)
               {
                  eina_stringshare_del(palette->palette_name);
                  EINA_LIST_FREE(palette->color_list, color) free(color);
                  free(palette);
               }
             _elm_config->color_palette = prev_config->color_palette;
             prev_config->color_palette = NULL;
          }

        KEEP_VAL(softcursor_mode);
        KEEP_VAL(auto_norender_withdrawn);
        KEEP_VAL(auto_norender_iconified_same_as_withdrawn);
        KEEP_VAL(auto_flush_withdrawn);
        KEEP_VAL(auto_dump_withdrawn);
        KEEP_VAL(auto_throttle);
        KEEP_VAL(auto_throttle_amount);
        KEEP_STR(indicator_service_0);
        KEEP_STR(indicator_service_90);
        KEEP_STR(indicator_service_180);
        KEEP_STR(indicator_service_270);
        KEEP_VAL(selection_clear_enable);
        KEEP_VAL(disable_external_menu);
        KEEP_VAL(clouseau_enable);
        KEEP_VAL(magnifier_enable);
        KEEP_VAL(magnifier_scale);
        KEEP_VAL(audio_mute_effect);
        KEEP_VAL(audio_mute_background);
        KEEP_VAL(audio_mute_music);
        KEEP_VAL(audio_mute_foreground);
        KEEP_VAL(audio_mute_interface);
        KEEP_VAL(audio_mute_input);
        KEEP_VAL(audio_mute_alert);
        KEEP_VAL(audio_mute_all);
        KEEP_VAL(win_auto_focus_enable);
        KEEP_VAL(win_auto_focus_animate);
        KEEP_VAL(transition_duration_factor);
        KEEP_VAL(naviframe_prev_btn_auto_pushed);
        KEEP_VAL(spinner_min_max_filter_enable);
        if (prev_config->priv.bindings)
          {
             EINA_LIST_FREE(_elm_config->bindings, wb)
               {
                  eina_stringshare_del(wb->name);
                  EINA_LIST_FREE(wb->key_bindings, kb)
                    {
                       eina_stringshare_del(kb->key);
                       eina_stringshare_del(kb->action);
                       eina_stringshare_del(kb->params);
                       EINA_LIST_FREE(kb->modifiers, mb)
                         {
                            eina_stringshare_del(mb->mod);
                            free(mb);
                         }
                       free(kb);
                    }
                  free(wb);
               }
             _elm_config->bindings = prev_config->bindings;
             prev_config->bindings = NULL;
          }
        KEEP_VAL(atspi_mode);
        KEEP_VAL(gl_depth);
        KEEP_VAL(gl_stencil);
        KEEP_VAL(gl_msaa);
        KEEP_STR(icon_theme);
        KEEP_VAL(entry_select_allow);
        KEEP_VAL(drag_anim_duration);

        _elm_config->priv = prev_config->priv;
        _config_free(prev_config);
     }
   if (_elm_config) _env_get();
}

static void
_config_flush_get(void)
{
   Eina_Bool is_mirrored;
   Eina_Bool translate;
   double pre_scale;

   is_mirrored = _elm_config->is_mirrored;
   translate = _elm_config->translate;
   pre_scale = _elm_config->scale;

   _elm_config_font_overlays_cancel();
   _color_overlays_cancel();

   _elm_config_reload_do();

   /* restore prev value which is not part of the EET file */
   _elm_config->is_mirrored = is_mirrored;
   _elm_config->translate = translate;

   _config_apply();
   _config_sub_apply();
   evas_font_reinit();
   _elm_config_font_overlay_apply();
   _elm_config_color_overlay_apply();
   if (pre_scale != _elm_config->scale)
     _elm_rescale();
   _elm_recache();
   _elm_old_clouseau_reload();
   _elm_config_key_binding_hash();
   _elm_win_access(_elm_config->access_mode);
   ecore_event_add(ELM_EVENT_CONFIG_ALL_CHANGED, NULL, NULL, NULL);
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
_elm_config_profile_save(const char *profile)
{
   Elm_Config_Derived *derived;
   char buf[4096], buf2[4096];
   int ok = 0, ret;
   const char *err, *s;
   Eet_File *ef;
   size_t len;

   if ((s = getenv("ELM_PROFILE_NOSAVE")) && atoi(s))
     return EINA_TRUE;

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

   derived = _elm_config_derived_load(profile ? profile : _elm_profile);
   if (derived)
     {
        _elm_config_derived_save(_elm_config, derived);
        _elm_config_derived_free(derived);
     }
   return EINA_TRUE;

err:
   ecore_file_unlink(buf2);
   return EINA_FALSE;
}

Eina_Bool
_elm_config_save(Elm_Config *cfg, const char *profile)
{
   char buf[4096], buf2[4096];
   int ok = 0, ret;
   const char *err;
   Eet_File *ef;
   size_t len;

   len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "themes/");
   if (len + 1 >= sizeof(buf))
     return EINA_FALSE;

   ok = ecore_file_mkpath(buf);
   if (!ok)
     {
        ERR("Problem accessing Elementary's user configuration directory: %s",
            buf);
        return EINA_FALSE;
     }

   len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "config/%s",
                                       profile ? profile : _elm_profile);
   if (len + 1 >= sizeof(buf))
     return EINA_FALSE;

   ok = ecore_file_mkpath(buf);
   if (!ok)
     {
        ERR("Problem accessing Elementary's user configuration directory: %s",
            buf);
        return EINA_FALSE;
     }

   if (!profile)
     {
        if (!_elm_config_profile_save(NULL))
          return EINA_FALSE;
     }

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

   ok = eet_data_write(ef, _config_edd, "config", cfg, 1);
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

// those widgets changed name over time, breaking key bindings
typedef struct _Widget_Class_Name_Map {
   const char *const legacy;
   const char *const newname;
} Widget_Class_Name_Map;

static const Widget_Class_Name_Map _widget_class_name_map [] = {
{ "elm_win", "Efl.Ui.Win" },
{ "elm_image", "Efl.Ui.Image" },
{ "elm_video", "Efl.Ui.Video" },
{ "elm_photocam", "Efl.Ui.Image_Zoomable" },
};

static void
_elm_key_bindings_update(Elm_Config *cfg, Elm_Config *syscfg EINA_UNUSED)
{
   for (size_t k = 0; k < EINA_C_ARRAY_LENGTH(_widget_class_name_map); k++)
     {
        const Widget_Class_Name_Map *cmap = &_widget_class_name_map[k];
        Elm_Config_Bindings_Widget *wb;
        Eina_List *l;

        // find old name in local memory and replace with new name
        EINA_LIST_FOREACH(cfg->bindings, l, wb)
          {
             if (wb->name && !strcasecmp(wb->name, cmap->legacy))
               {
                  eina_stringshare_replace(&wb->name, cmap->newname);
                  break;
               }
          }
     }
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
#define COPYVAL(x) do {_elm_config->x = tcfg->x; } while (0)
#define COPYPTR(x) do {_elm_config->x = tcfg->x; tcfg->x = NULL; } while (0)
#define COPYSTR(x) COPYPTR(x)

   /* we also need to update for property changes in the root window
    * if needed, but that will be dependent on new properties added
    * with each version */
   IFCFG(0x0002)
   _elm_config->win_auto_focus_enable = tcfg->win_auto_focus_enable;;
   _elm_config->win_auto_focus_animate = tcfg->win_auto_focus_animate;
   IFCFGEND

   IFCFG(0x0003)
   _elm_config->transition_duration_factor = tcfg->transition_duration_factor;
   IFCFGEND

   IFCFG(0x0004)
   Elm_Config_Bindings_Widget *wb, *twb = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(tcfg->bindings, l, wb)
     {
        if (wb->name && !strcmp(wb->name, "Elm_Hoversel"))
          {
             twb = wb;
             break;
          }
     }
   if (twb)
     {
        EINA_LIST_FOREACH(_elm_config->bindings, l, wb)
           {
              if (wb->name && !strcmp(wb->name, "Elm_Hoversel"))
                {
                   // simply swap bindngs for Elm_Hoversel with system ones
                   Eina_List *tmp = wb->key_bindings;
                   wb->key_bindings = twb->key_bindings;
                   twb->key_bindings = tmp;
                   break;
                }
           }
     }
   IFCFGEND

   IFCFG(0x0005)
   _elm_config->naviframe_prev_btn_auto_pushed = tcfg->naviframe_prev_btn_auto_pushed;
   IFCFGEND

   IFCFG(0x0006)
   _elm_config->cursor_engine_only = 0;
   IFCFGEND

   IFCFG(0x0007)
   Elm_Config_Bindings_Widget *wb, *twb = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(tcfg->bindings, l, wb)
     {
        if (wb->name && !strcmp(wb->name, "Elm_Combobox"))
          {
             twb = wb;
             break;
          }
     }
   if (twb)
     {
        EINA_LIST_FOREACH(_elm_config->bindings, l, wb)
           {
              if (wb->name && !strcmp(wb->name, "Elm_Combobox"))
                {
                   // simply swap bindngs for Elm_Combobox with system ones
                   Eina_List *tmp = wb->key_bindings;
                   wb->key_bindings = twb->key_bindings;
                   twb->key_bindings = tmp;
                   break;
                }
           }
     }
   IFCFGEND

   IFCFG(0x0008)
   _elm_config->popup_horizontal_align = 0.5;
   _elm_config->popup_vertical_align = 0.5;
   IFCFGEND

   IFCFG(0x0009)
   _elm_config->scroll_accel_factor = 7.0;
   IFCFGEND

   IFCFG(0x000a)
   _elm_config->icon_theme = eina_stringshare_add(ELM_CONFIG_ICON_THEME_ELEMENTARY);
   IFCFGEND

   IFCFG(0x000b)
   eina_stringshare_refplace(&_elm_config->modules, tcfg->modules);
   IFCFGEND

   IFCFG(0x000e)
   _elm_config->entry_select_allow = EINA_TRUE;
   IFCFGEND

   IFCFG(0x000f)
   _elm_key_bindings_update(_elm_config, tcfg);
   IFCFGEND

   /**
    * Fix user config for current ELM_CONFIG_EPOCH here.
    **/

#undef COPYSTR
#undef COPYPTR
#undef COPYVAL
#undef IFCFGEND
#undef IFCFGELSE
#undef IFCFG

   _elm_config->config_version = ELM_CONFIG_VERSION;
   /* after updating user config, we must save */
   _config_free(tcfg);
   _elm_config_save(_elm_config, NULL);
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
        else if ((!strcasecmp(s, "wayland_shm")) || 
                 (!strcasecmp(s, "wayland-shm")))
          eina_stringshare_replace(&_elm_config->engine, ELM_WAYLAND_SHM);
        else if ((!strcasecmp(s, "wayland_egl")) || 
                 (!strcasecmp(s, "wayland-egl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_WAYLAND_EGL);
        else if ((!strcasecmp(s, "drm")))
          eina_stringshare_replace(&_elm_config->engine, ELM_DRM);
        else if ((!strcasecmp(s, "ddraw")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_DDRAW);
        else
          ERR("Unknown engine '%s'.", s);
        if (_elm_config->engine)
          eina_stringshare_replace(&_elm_preferred_engine, _elm_config->engine);
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
   s = getenv("ELM_SCROLL_ANIMATION_DISABLE");
   if (s) _elm_config->scroll_animation_disable = !!atoi(s);
   s = getenv("ELM_SCROLL_ACCEL_FACTOR");
   if (s) _elm_config->scroll_accel_factor = atof(s);
//   s = getenv("ELM_SCROLL_SMOOTH_TIME_INTERVAL"); // not used anymore
//   if (s) _elm_config->scroll_smooth_time_interval = atof(s); // not used anymore
   s = getenv("ELM_SCROLL_SMOOTH_AMOUNT");
   if (s) _elm_config->scroll_smooth_amount = _elm_atof(s);
//   s = getenv("ELM_SCROLL_SMOOTH_HISTORY_WEIGHT"); // not used anymore
//   if (s) _elm_config->scroll_smooth_history_weight = _elm_atof(s); // not used anymore
//   s = getenv("ELM_SCROLL_SMOOTH_FUTURE_TIME"); // not used anymore
//   if (s) _elm_config->scroll_smooth_future_time = _elm_atof(s); // not used anymore
   s = getenv("ELM_SCROLL_SMOOTH_TIME_WINDOW");
   if (s) _elm_config->scroll_smooth_time_window = _elm_atof(s);
   s = getenv("ELM_FOCUS_AUTOSCROLL_MODE");
   if (s)
     {
        if (!strcmp(s, "ELM_FOCUS_AUTOSCROLL_MODE_NONE"))
          _elm_config->focus_autoscroll_mode = ELM_FOCUS_AUTOSCROLL_MODE_NONE;
        else if (!strcmp(s, "ELM_FOCUS_AUTOSCROLL_MODE_BRING_IN"))
          _elm_config->focus_autoscroll_mode = ELM_FOCUS_AUTOSCROLL_MODE_BRING_IN;
        else
          _elm_config->focus_autoscroll_mode = ELM_FOCUS_AUTOSCROLL_MODE_SHOW;
     }
   s = getenv("ELM_SLIDER_INDICATOR_VISIBLE_MODE");
   if (s)
     {
        if (!strcmp(s, "ELM_SLIDER_INDICATOR_VISIBLE_MODE_DEFAULT"))
          _elm_config->slider_indicator_visible_mode = ELM_SLIDER_INDICATOR_VISIBLE_MODE_DEFAULT;
        else if (!strcmp(s, "ELM_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS"))
          _elm_config->slider_indicator_visible_mode = ELM_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS;
        else if (!strcmp(s, "ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS"))
          _elm_config->slider_indicator_visible_mode = ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS;
        else
          _elm_config->slider_indicator_visible_mode = ELM_SLIDER_INDICATOR_VISIBLE_MODE_NONE;
     }
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

   s = getenv("ELM_FIRST_ITEM_FOCUS_ON_FIRST_FOCUS_IN");
   if (s) _elm_config->first_item_focus_on_first_focus_in = !!atoi(s);

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

   s = getenv("ELM_CONTEXT_MENU_DISABLED");
   if (s) _elm_config->context_menu_disabled = !!atoi(s);

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
   s = getenv("ELM_ATSPI_MODE");
   if (s) _elm_config->atspi_mode = ELM_ATSPI_MODE_ON;
   s = getenv("ELM_SPINNER_MIN_MAX_FILTER_ENABLE");
   if (s) _elm_config->spinner_min_max_filter_enable = !!atoi(s);

   s = getenv("ELM_TRANSITION_DURATION_FACTOR");
   if (s) _elm_config->transition_duration_factor = atof(s);

   s = getenv("ELM_POPUP_HORIZONTAL_ALIGN");
   if (s) _elm_config->popup_horizontal_align = _elm_atof(s);
   s = getenv("ELM_POPUP_VERTICAL_ALIGN");
   if (s) _elm_config->popup_vertical_align = _elm_atof(s);
   s = getenv("ELM_POPUP_SCROLLABLE");
   if (s) _elm_config->popup_scrollable = atoi(s);

   s = getenv("ELM_GLAYER_TAP_FINGER_SIZE");
   if (s) _elm_config->glayer_tap_finger_size = atoi(s);

   s = getenv("EFL_UI_DND_DRAG_ANIM_DURATION");
   if (s) _elm_config->drag_anim_duration = _elm_atof(s);
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
        if (wb->name)
          {
             char *namelower = alloca(strlen(wb->name) + 1);

             strcpy(namelower, wb->name);
             eina_str_tolower(&namelower);
             eina_hash_add(_elm_key_bindings, namelower, wb->key_bindings);
          }
     }
}

static Eina_Bool
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
                             const char *name,
                             const Evas_Event_Key_Down *ev,
                             const Elm_Action *actions)
{
   Elm_Config_Binding_Key *binding;
   Eina_List *binding_list, *l;
   char *namelower;
   int i = 0;

   namelower = alloca(strlen(name) + 1);
   strcpy(namelower, name);
   eina_str_tolower(&namelower);

   binding_list = eina_hash_find(_elm_key_bindings, namelower);

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
   _elm_config->priv.cursor_engine_only = EINA_TRUE;
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
   _elm_config->priv.tooltip_delay = EINA_TRUE;
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
   _elm_config->priv.scale = EINA_TRUE;
   if (scale < 0.0) return;
   if (_elm_config->scale == scale) return;
   _elm_config->scale = scale;
   _elm_rescale();
}

EAPI const char *
elm_config_icon_theme_get(void)
{
   if (!_elm_config->icon_theme)
     return ELM_CONFIG_ICON_THEME_ELEMENTARY;

   return _elm_config->icon_theme;
}

EAPI void
elm_config_icon_theme_set(const char *theme)
{
   _elm_config->priv.icon_theme = EINA_TRUE;
   eina_stringshare_del(_elm_config->icon_theme);

   if (theme)
     _elm_config->icon_theme = eina_stringshare_add(theme);
   else
     _elm_config->icon_theme = eina_stringshare_add(ELM_CONFIG_ICON_THEME_ELEMENTARY);
}

EAPI Eina_Bool
elm_config_password_show_last_get(void)
{
   return _elm_config->password_show_last;
}

EAPI void
elm_config_password_show_last_set(Eina_Bool password_show_last)
{
   _elm_config->priv.password_show_last = EINA_TRUE;
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
   _elm_config->priv.password_show_last_timeout = EINA_TRUE;
   if (password_show_last_timeout < 0.0) return;
   if (_elm_config->password_show_last_timeout == password_show_last_timeout) return;
   _elm_config->password_show_last_timeout = password_show_last_timeout;
   edje_password_show_last_timeout_set(_elm_config->password_show_last_timeout);
}

EAPI Eina_Bool
elm_config_save(void)
{
   return _elm_config_save(_elm_config, NULL);
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
   // Don't change this: EO relies on free() to be used.
   free((void *)p_dir);
}

EAPI Eina_List *
elm_config_profile_list_get(void)
{
   return _elm_config_profiles_list(EINA_TRUE);
}

EAPI Eina_List *
elm_config_profile_list_full_get(void)
{
   return _elm_config_profiles_list(EINA_FALSE);
}

EAPI void
elm_config_profile_list_free(Eina_List *l)
{
   const char *dir;

   EINA_LIST_FREE(l, dir)
     eina_stringshare_del(dir);
}

EAPI Eina_Bool
elm_config_profile_exists(const char *profile)
{
   char buf[PATH_MAX], buf2[PATH_MAX];

   if (!profile) return EINA_FALSE;

   _elm_config_user_dir_snprintf(buf, sizeof(buf),
                                 "config/%s/base.cfg", profile);
   if (ecore_file_exists(buf)) return EINA_TRUE;

   snprintf(buf2, sizeof(buf2), "config/%s/base.cfg", profile);
   eina_str_join_len(buf, sizeof(buf), '/',
                     _elm_data_dir, strlen(_elm_data_dir),
                     buf2, strlen(buf2));
   if (ecore_file_exists(buf)) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI void
elm_config_profile_set(const char *profile)
{
   EINA_SAFETY_ON_NULL_RETURN(profile);
   _elm_config_profile_set(profile);
}

EAPI void
elm_config_profile_save(const char *profile)
{
   EINA_SAFETY_ON_NULL_RETURN(profile);
   _elm_config_save(_elm_config, profile);
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
   _elm_config->priv.access_mode = EINA_TRUE;
   _elm_config_access_set(is_access);
}

EAPI Eina_Bool
elm_config_atspi_mode_get(void)
{
   return _elm_config_atspi_mode_get();
}

EAPI void
elm_config_atspi_mode_set(Eina_Bool is_enable)
{
   _elm_config_atspi_mode_set(is_enable);
}

EAPI Eina_Bool
elm_config_selection_unfocused_clear_get(void)
{
   return _elm_config_selection_unfocused_clear_get();
}

EAPI void
elm_config_selection_unfocused_clear_set(Eina_Bool enabled)
{
   _elm_config->priv.selection_clear_enable = EINA_TRUE;
   _elm_config_selection_unfocused_clear_set(enabled);
}

EAPI void
elm_config_font_overlay_set(const char    *text_class,
                     const char    *font,
                     Evas_Font_Size size)
{
   _elm_config->priv.font_overlays = EINA_TRUE;
   EINA_SAFETY_ON_NULL_RETURN(text_class);
   _elm_config_font_overlay_set(text_class, font, size);
}

EAPI void
elm_config_font_overlay_unset(const char *text_class)
{
   _elm_config->priv.font_overlays = EINA_TRUE;
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
   _elm_config->priv.color_overlays = EINA_TRUE;
   EINA_SAFETY_ON_NULL_RETURN(color_class);
   _elm_config_color_overlay_set(color_class,
                                 r, g, b, a,
                                 r2, g2, b2, a2,
                                 r3, g3, b3, a3);
}

EAPI void
elm_config_color_overlay_unset(const char *color_class)
{
   _elm_config->priv.color_overlays = EINA_TRUE;
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
   _elm_config->priv.finger_size = EINA_TRUE;
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
   _elm_config->priv.cache_flush_poll_interval = EINA_TRUE;
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
   _elm_config->priv.cache_flush_enable = EINA_TRUE;
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
   _elm_config->priv.font_cache = EINA_TRUE;
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
   _elm_config->priv.image_cache = EINA_TRUE;
   if (size < 0) return;
   if (_elm_config->image_cache == size) return;
   _elm_config->image_cache = size;

   _elm_recache();
}

EAPI void
elm_config_font_hint_type_set(int type)
{
   Eina_List *l;
   Evas_Object *win;

   _elm_config->priv.font_hinting = EINA_TRUE;
   if ((type < 0) || (type > 2)) return;
   _elm_config->font_hinting = type;

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        Evas *e = evas_object_evas_get(win);

        if (!_elm_config->font_hinting)
          evas_font_hinting_set(e, EVAS_FONT_HINTING_NONE);
        else if (_elm_config->font_hinting == 1)
          evas_font_hinting_set(e, EVAS_FONT_HINTING_AUTO);
        else if (_elm_config->font_hinting == 2)
          evas_font_hinting_set(e, EVAS_FONT_HINTING_BYTECODE);
     }
}

EAPI int
elm_config_cache_edje_file_cache_size_get()
{
   return _elm_config->edje_cache;
}

EAPI void
elm_config_cache_edje_file_cache_size_set(int size)
{
   _elm_config->priv.edje_cache = EINA_TRUE;
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
   _elm_config->priv.edje_collection_cache = EINA_TRUE;
   if (_elm_config->edje_collection_cache == size) return;
   _elm_config->edje_collection_cache = size;

   _elm_recache();
}

EAPI Eina_Bool
elm_config_vsync_get(void)
{
   return _elm_config->vsync;
}

EAPI void
elm_config_vsync_set(Eina_Bool enabled)
{
   _elm_config->priv.vsync = EINA_TRUE;
   _elm_config->vsync = enabled;
}

EAPI Eina_Bool
elm_config_agressive_withdrawn_get(void)
{
   return _elm_config->auto_norender_withdrawn &&
     _elm_config->auto_norender_iconified_same_as_withdrawn &&
     _elm_config->auto_flush_withdrawn &&
     _elm_config->auto_dump_withdrawn;
}

EAPI void
elm_config_agressive_withdrawn_set(Eina_Bool enabled)
{
   _elm_config->auto_norender_withdrawn = enabled;
   _elm_config->auto_norender_iconified_same_as_withdrawn = enabled;
   _elm_config->auto_flush_withdrawn = enabled;
   _elm_config->auto_dump_withdrawn = enabled;
}

EAPI Eina_Bool
elm_config_accel_preference_override_get(void)
{
   return _elm_config->accel_override;
}

EAPI void
elm_config_accel_preference_override_set(Eina_Bool enabled)
{
   _elm_config->priv.accel_override = EINA_TRUE;
   _elm_config->accel_override = enabled;
}

EAPI Eina_Bool
elm_config_focus_highlight_enabled_get(void)
{
   return _elm_config->focus_highlight_enable;
}

EAPI void
elm_config_focus_highlight_enabled_set(Eina_Bool enable)
{
   _elm_config->priv.focus_highlight_enable = EINA_TRUE;
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
   _elm_config->priv.focus_highlight_animate = EINA_TRUE;
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
   _elm_config->priv.focus_highlight_clip_disable = EINA_TRUE;
   _elm_config->focus_highlight_clip_disable = !!disable;
}

EAPI Elm_Focus_Move_Policy
elm_config_focus_move_policy_get(void)
{
   return _elm_config->focus_move_policy;
}

static void
_efl_ui_widget_config_reload(Efl_Ui_Widget *obj)
{
   Elm_Focus_Move_Policy focus_move_policy = elm_config_focus_move_policy_get();
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, sd);
   Efl_Ui_Widget *w;
   Eina_List *n;

   //reload focus move policy
   if (efl_ui_widget_focus_move_policy_automatic_get(obj) &&
       (sd->focus_move_policy != focus_move_policy))
     {
        sd->focus_move_policy = focus_move_policy;
     }

   EINA_LIST_FOREACH(sd->subobjs, n, w)
     {
        if (efl_isa(w, EFL_UI_WIDGET_CLASS))
          _efl_ui_widget_config_reload(w);
     }
}

void
_elm_win_focus_reconfigure(void)
{
   const Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     {
        _efl_ui_widget_config_reload(obj);
     }
}

EAPI void
elm_config_focus_move_policy_set(Elm_Focus_Move_Policy policy)
{
   _elm_config->priv.focus_move_policy = EINA_TRUE;
   _elm_config->focus_move_policy = policy;
   _elm_win_focus_reconfigure();
}

EAPI Eina_Bool
elm_config_item_select_on_focus_disabled_get(void)
{
   return _elm_config->item_select_on_focus_disable;
}

EAPI void
elm_config_item_select_on_focus_disabled_set(Eina_Bool disable)
{
   _elm_config->priv.item_select_on_focus_disable = EINA_TRUE;
   _elm_config->item_select_on_focus_disable = !!disable;
}

EAPI Eina_Bool
elm_config_first_item_focus_on_first_focusin_get(void)
{
   return _elm_config->first_item_focus_on_first_focus_in;
}

EAPI void
elm_config_first_item_focus_on_first_focusin_set(Eina_Bool enable)
{
   _elm_config->priv.first_item_focus_on_first_focus_in = EINA_TRUE;
   _elm_config->first_item_focus_on_first_focus_in = !!enable;
}

EAPI Eina_Bool
elm_config_scroll_bounce_enabled_get(void)
{
   return _elm_config->thumbscroll_bounce_enable;
}

EAPI void
elm_config_scroll_bounce_enabled_set(Eina_Bool enabled)
{
   _elm_config->priv.thumbscroll_bounce_enable = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_bounce_friction = EINA_TRUE;
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
   _elm_config->priv.page_scroll_friction = EINA_TRUE;
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
   _elm_config->priv.bring_in_scroll_friction = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_enable = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_threshold = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_hold_threshold = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_momentum_threshold = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_flick_distance_tolerance = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_friction = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_min_friction = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_friction_standard = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_border_friction = EINA_TRUE;
   if (friction < 0.0) friction = 0.0;
   if (friction > 1.0) friction = 1.0;
   _elm_config->thumbscroll_border_friction = friction;
}

EAPI double
elm_config_scroll_thumbscroll_sensitivity_friction_get(void)
{
   return _elm_config->thumbscroll_sensitivity_friction;
}

EAPI Eina_Bool
elm_config_context_menu_disabled_get(void)
{
   return _elm_config->context_menu_disabled;
}

EAPI void
elm_config_context_menu_disabled_set(Eina_Bool disabled)
{
   _elm_config->priv.context_menu_disabled = EINA_TRUE;
   _elm_config->context_menu_disabled = !!disabled;
}

EAPI void
elm_config_scroll_thumbscroll_sensitivity_friction_set(double friction)
{
   _elm_config->priv.thumbscroll_sensitivity_friction = EINA_TRUE;
   if (friction < 0.1) friction = 0.1;
   if (friction > 1.0) friction = 1.0;
   _elm_config->thumbscroll_sensitivity_friction = friction;
}

EAPI Eina_Bool
elm_config_scroll_thumbscroll_smooth_start_get(void)
{
   return _elm_config->scroll_smooth_start_enable;
}

EAPI void
elm_config_scroll_thumbscroll_smooth_start_set(Eina_Bool enable)
{
   _elm_config->priv.scroll_smooth_start_enable = EINA_TRUE;
   _elm_config->scroll_smooth_start_enable = enable;
}

EAPI Eina_Bool
elm_config_scroll_animation_disabled_get(void)
{
   return _elm_config->scroll_animation_disable;
}

EAPI void
elm_config_scroll_animation_disabled_set(Eina_Bool disable)
{
   _elm_config->priv.scroll_animation_disable = EINA_TRUE;
   _elm_config->scroll_animation_disable = !!disable;
}

EAPI void
elm_config_scroll_accel_factor_set(double factor)
{
   _elm_config->priv.scroll_accel_factor = EINA_TRUE;
   if (factor < 0.0) factor = 0.0;
   if (factor > 10.0) factor = 10.0;
   _elm_config->scroll_accel_factor = factor;
}

EAPI double
elm_config_scroll_accel_factor_get(void)
{
   return _elm_config->scroll_accel_factor;
}

EAPI void
elm_config_scroll_thumbscroll_smooth_amount_set(double amount)
{
   _elm_config->priv.scroll_smooth_amount = EINA_TRUE;
   if (amount < 0.0) amount = 0.0;
   if (amount > 1.0) amount = 1.0;
   _elm_config->scroll_smooth_amount = amount;
}

EAPI double
elm_config_scroll_thumbscroll_smooth_amount_get(void)
{
   return _elm_config->scroll_smooth_amount;
}

EAPI void
elm_config_scroll_thumbscroll_smooth_time_window_set(double amount)
{
   _elm_config->priv.scroll_smooth_time_window = EINA_TRUE;
   if (amount < 0.0) amount = 0.0;
   if (amount > 1.0) amount = 1.0;
   _elm_config->scroll_smooth_time_window = amount;
}

EAPI double
elm_config_scroll_thumbscroll_smooth_time_window_get(void)
{
   return _elm_config->scroll_smooth_time_window;
}

EAPI double
elm_config_scroll_thumbscroll_acceleration_threshold_get(void)
{
   return _elm_config->thumbscroll_acceleration_threshold;
}

EAPI void
elm_config_scroll_thumbscroll_acceleration_threshold_set(double threshold)
{
   _elm_config->priv.thumbscroll_acceleration_threshold = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_acceleration_time_limit = EINA_TRUE;
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
   _elm_config->priv.thumbscroll_acceleration_weight = EINA_TRUE;
   _elm_config->thumbscroll_acceleration_weight = weight;
}

EAPI Elm_Focus_Autoscroll_Mode
elm_config_focus_autoscroll_mode_get(void)
{
   return _elm_config->focus_autoscroll_mode;
}

EAPI void
elm_config_slider_indicator_visible_mode_set(Elm_Slider_Indicator_Visible_Mode mode)
{
   _elm_config->priv.slider_indicator_visible_mode = EINA_TRUE;
   _elm_config->slider_indicator_visible_mode = mode;
}

EAPI Elm_Slider_Indicator_Visible_Mode
elm_config_slider_indicator_visible_mode_get(void)
{
    return _elm_config->slider_indicator_visible_mode;
}

EAPI void
elm_config_focus_autoscroll_mode_set(Elm_Focus_Autoscroll_Mode mode)
{
   _elm_config->priv.focus_autoscroll_mode = EINA_TRUE;
   _elm_config->focus_autoscroll_mode = mode;
}

EAPI void
elm_config_longpress_timeout_set(double longpress_timeout)
{
   _elm_config->priv.longpress_timeout = EINA_TRUE;
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
   _elm_config->priv.softcursor_mode = EINA_TRUE;
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
   _elm_config->priv.disable_external_menu = EINA_TRUE;
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
   _elm_config->priv.clouseau_enable = EINA_TRUE;
   _elm_config->clouseau_enable = !!enable;
   _elm_old_clouseau_reload();
}

EAPI double
elm_config_glayer_long_tap_start_timeout_get(void)
{
   return _elm_config->glayer_long_tap_start_timeout;
}

EAPI void
elm_config_glayer_long_tap_start_timeout_set(double long_tap_timeout)
{
   _elm_config->priv.glayer_long_tap_start_timeout = EINA_TRUE;
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
   _elm_config->priv.glayer_double_tap_timeout = EINA_TRUE;
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
   _elm_config->priv.magnifier_enable = EINA_TRUE;
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
   _elm_config->priv.magnifier_scale = EINA_TRUE;
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
        _elm_config->priv.audio_mute_effect = EINA_TRUE;
        _elm_config->audio_mute_effect = mute;
        break;
      case EDJE_CHANNEL_BACKGROUND:
        _elm_config->priv.audio_mute_background = EINA_TRUE;
        _elm_config->audio_mute_background = mute;
        break;
      case EDJE_CHANNEL_MUSIC:
        _elm_config->priv.audio_mute_music = EINA_TRUE;
        _elm_config->audio_mute_music = mute;
        break;
      case EDJE_CHANNEL_FOREGROUND:
        _elm_config->priv.audio_mute_foreground = EINA_TRUE;
        _elm_config->audio_mute_foreground = mute;
        break;
      case EDJE_CHANNEL_INTERFACE:
        _elm_config->priv.audio_mute_interface = EINA_TRUE;
        _elm_config->audio_mute_interface = mute;
        break;
      case EDJE_CHANNEL_INPUT:
        _elm_config->priv.audio_mute_input = EINA_TRUE;
        _elm_config->audio_mute_input = mute;
        break;
      case EDJE_CHANNEL_ALERT:
        _elm_config->priv.audio_mute_alert = EINA_TRUE;
        _elm_config->audio_mute_alert = mute;
        break;
      case EDJE_CHANNEL_ALL:
        _elm_config->priv.audio_mute_all = EINA_TRUE;
        _elm_config->audio_mute_all = mute;
        break;
      default:
        break;
     }
   edje_audio_channel_mute_set(channel, mute);
}

EAPI Eina_Bool
elm_config_window_auto_focus_enable_get(void)
{
   return _elm_config->win_auto_focus_enable;
}

EAPI void
elm_config_window_auto_focus_enable_set(Eina_Bool enable)
{
   _elm_config->priv.win_auto_focus_enable = EINA_TRUE;
   _elm_config->win_auto_focus_enable = enable;
}

EAPI Eina_Bool
elm_config_window_auto_focus_animate_get(void)
{
   return _elm_config->win_auto_focus_animate;
}

EAPI void
elm_config_window_auto_focus_animate_set(Eina_Bool enable)
{
   _elm_config->priv.win_auto_focus_animate = EINA_TRUE;
   _elm_config->win_auto_focus_animate = enable;
}

EAPI Eina_Bool
elm_config_popup_scrollable_get(void)
{
   return _elm_config->popup_scrollable;
}

EAPI void
elm_config_popup_scrollable_set(Eina_Bool scrollable)
{
   _elm_config->priv.popup_scrollable = EINA_TRUE;
   _elm_config->popup_scrollable = scrollable;
}

EAPI void
elm_config_all_flush(void)
{
   char buf[PATH_MAX];
   int ok = 0;
   size_t len;

   len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "themes/");
   if (len + 1 >= sizeof(buf))
     return;

   ok = ecore_file_mkpath(buf);
   if (!ok)
     {
        ERR("Problem accessing Elementary's user configuration directory: %s",
            buf);
        return;
     }

   len = _elm_config_user_dir_snprintf(buf, sizeof(buf), "config");
   if (len + 1 >= sizeof(buf))
     return;

   ok = ecore_file_mkpath(buf);
   if (!ok)
     {
        ERR("Problem accessing Elementary's user configuration directory: %s",
            buf);
        return;
     }

   if (!_elm_config_profile_save(NULL))
     {
        ERR("Failed to save profile");
        return;
     }

   elm_config_save();
   return;
}

static void
_translation_init(void)
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
   if (msg_locale && cur_dom)
     _elm_config->translate = !(strcmp (cur_dom, "messages") &&
           !*trans_comment && strncmp (msg_locale, "en_", 3) &&
           strcmp (msg_locale, "C"));
   /* Get RTL orientation from system */
   if (_elm_config->translate)
     {
        bindtextdomain(PACKAGE, LOCALE_DIR);
        /* TRANSLATORS: This string is special and defines if the language
           by default is a Left To Right or Right To Left language. Most
           languages are LTR and thus the default string is "default:LTR"
           but if this is a right-to-left language like arabic, Farsi,
           Hebrew, etc. then change this string to be "default:RTL"
         */
        _elm_config->is_mirrored = !strcmp(E_("default:LTR"), "default:RTL");
     }

#endif
}

void
_elm_config_init(void)
{
   ELM_EVENT_CONFIG_ALL_CHANGED = ecore_event_type_new();
   _desc_init();
   _elm_config_profile_derived_init();
   _profile_fetch_from_conf();
   _config_load();
   if (_elm_config) _env_get();
   ELM_SAFE_FREE(_elm_accel_preference, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_gl_preference, eina_stringshare_del);
   _translation_init();
   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_config_color_overlay_apply();
   _elm_recache();
   _elm_old_clouseau_reload();
   _elm_config_key_binding_hash();
}

void
_elm_config_sub_shutdown(void)
{
   ecore_event_type_flush(ELM_EVENT_CONFIG_ALL_CHANGED);

#ifdef HAVE_ELEMENTARY_WIN32
   ecore_win32_shutdown();
#endif
   ELM_SAFE_FREE(_eio_config_monitor, eio_monitor_del);
   ELM_SAFE_FREE(_eio_profile_monitor, eio_monitor_del);
   ELM_SAFE_FREE(_config_change_delay_timer, ecore_timer_del);
   ELM_SAFE_FREE(_config_profile_change_delay_timer, ecore_timer_del);
   ELM_SAFE_FREE(_monitor_file_created_handler, ecore_event_handler_del);
   ELM_SAFE_FREE(_monitor_directory_created_handler, ecore_event_handler_del);
}

static Eina_Bool
_config_profile_change_delay_cb(void *data EINA_UNUSED)
{
   char *pprof = NULL;

   if (_elm_profile) pprof = strdup(_elm_profile);
   _profile_fetch_from_conf();
   if ((!pprof) || (!(!strcmp(pprof, _elm_profile))))
     {
        _config_flush_get();
     }
   free(pprof);
   _config_profile_change_delay_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_config_change_delay_cb(void *data EINA_UNUSED)
{
   _elm_config_reload();

   _config_change_delay_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_elm_config_file_monitor_cb(void *data EINA_UNUSED,
                            int type,
                            void *event)
{
   Eio_Monitor_Event *ev = event;
   const char *file = ecore_file_file_get(ev->filename);
   char buf[PATH_MAX];

   if (ev->monitor == _eio_config_monitor)
     {
        if (type == EIO_MONITOR_FILE_CREATED)
          {
             if (!strcmp(file, "base.cfg"))
               {
                  if (_config_change_delay_timer)
                    ecore_timer_del(_config_change_delay_timer);
                  _config_change_delay_timer = ecore_timer_add(0.1, _config_change_delay_cb, NULL);
               }
          }
     }
   if (ev->monitor == _eio_profile_monitor)
     {
        if (type == EIO_MONITOR_FILE_CREATED)
          {
             if ((!_config_profile_lock) && (!strcmp(file, "profile.cfg")))
               {
                  if (_config_profile_change_delay_timer)
                    ecore_timer_del(_config_profile_change_delay_timer);
                  _config_profile_change_delay_timer = ecore_timer_add(0.1, _config_profile_change_delay_cb, NULL);
               }
          }
        else if (type == EIO_MONITOR_DIRECTORY_CREATED)
          {
             if (!_eio_config_monitor)
               {
                  _eio_config_monitor = eio_monitor_add(ev->filename);
                  snprintf(buf, sizeof(buf), "%s/base.cfg", ev->filename);
                  if (ecore_file_exists(buf))
                    {
                       if (_config_change_delay_timer)
                         ecore_timer_del(_config_change_delay_timer);
                       _config_change_delay_timer = ecore_timer_add(0.1, _config_change_delay_cb, NULL);
                    }
               }
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

void
_elm_config_sub_init(void)
{
#ifdef HAVE_ELEMENTARY_COCOA
   ecore_cocoa_init();
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   ecore_win32_init();
#endif
   char buf[PATH_MAX];
   int ok = 0;

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "config");
   ok = ecore_file_mkpath(buf);
   if (!ok)
     {
        ERR("Problem accessing Elementary's user configuration directory: %s",
            buf);
        goto end;
     }
   _eio_profile_monitor = eio_monitor_add(buf);
   _monitor_file_created_handler = ecore_event_handler_add
     (EIO_MONITOR_FILE_CREATED, _elm_config_file_monitor_cb, NULL);
   _monitor_directory_created_handler = ecore_event_handler_add
     (EIO_MONITOR_DIRECTORY_CREATED, _elm_config_file_monitor_cb, NULL);

end:
   _config_sub_apply();
}

void
_elm_config_reload(void)
{
   Eina_Bool is_mirrored;
   Eina_Bool translate;

   Eina_Stringshare *ptheme = NULL, *pmodules = NULL, *picon_theme = NULL;
   double pscale;
   int pfinger_size, picon_size;
   unsigned char ppassword_show_last;
   double ppassword_show_last_timeout;
   int pweek_start, pweekend_start, pweekend_len, pyear_min, pyear_max;

#define STO(x) if (_elm_config->x) p##x = eina_stringshare_add(_elm_config->x)
   STO(theme);
   STO(modules);
   STO(icon_theme);
#undef STO

#define STO(x) p##x = _elm_config->x
   STO(scale);
   STO(finger_size);
   STO(icon_size);
   STO(password_show_last);
   STO(password_show_last_timeout);
   STO(week_start);
   STO(weekend_start);
   STO(weekend_len);
   STO(year_min);
   STO(year_max);
#undef STO

   is_mirrored = _elm_config->is_mirrored;
   translate = _elm_config->translate;

   _elm_config_reload_do();
   
   /* restore prev value which is not part of the EET file */
   _elm_config->is_mirrored = is_mirrored;
   _elm_config->translate = translate;

   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_config_color_overlay_apply();
#define CMP(x) (p##x != _elm_config->x)
   if (
          CMP(scale)
       || CMP(finger_size)
       || CMP(icon_size)
       || CMP(password_show_last)
       || CMP(password_show_last_timeout)
       || CMP(week_start)
       || CMP(weekend_start)
       || CMP(weekend_len)
       || CMP(year_min)
       || CMP(year_max)
       // these are string ptr compares but this is right because they are
       // stringshare strings and thus an identical string has the same
       // exact pointer guaranteed
       || CMP(theme)
       || CMP(modules)
       || CMP(icon_theme)
      )
     _elm_rescale();
#undef CMP
   _elm_recache();
   _elm_old_clouseau_reload();
   _elm_config_key_binding_hash();
   ecore_event_add(ELM_EVENT_CONFIG_ALL_CHANGED, NULL, NULL, NULL);
   if (ptheme) eina_stringshare_del(ptheme);
   if (pmodules) eina_stringshare_del(pmodules);
   if (picon_theme) eina_stringshare_del(picon_theme);
}

void
_elm_config_engine_set(const char *engine)
{
   _elm_config->priv.engine = EINA_TRUE;
   eina_stringshare_replace(&(_elm_config->engine), engine);
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
   if (_elm_gl_preference) return _elm_gl_preference;
   if (_elm_accel_preference) return _elm_accel_preference;
   return _elm_config->accel;
}

Eina_Bool
_elm_config_accel_preference_parse(const char *pref, Eina_Stringshare **accel,
                                   int *gl_depth, int *gl_stencil, int *gl_msaa)
{
   Eina_Bool is_hw_accel = EINA_FALSE;
   unsigned int tokens = 0, i;
   char **arr;

   /* Accel preference's string has the window surface configuration as a hw accel, depth, stencil and msaa.
    * The string format is   "{HW Accel}:depth{value}:stencil{value}:msaa{msaa string}"
    * Especially, msaa string is related Evas GL MSAA enum value(low, mid, high)
    * so msaa string has four types as msaa, msaa_low, msaa_mid, msaa_high
    * For instance, "opengl:depth24:stencil8:msaa_high".
    * It means that using hw accelation, window surface depth buffer's size is 24, stencil buffer's size 8 and msaa bits is the highest.
    * The other use-case is  "opengl:depth24".
    * It measn that using hw accelation, depth buffer size is 24. stencil and msaa are not used.
    * The simple case is  "opengl:depth:stencil:msaa".
    * It means that depth, stencil and msaa are setted by pre-defined value(depth:24, stencil:8, msaa:low)
    */

   DBG("accel preference's string: %s", pref);

   /* split GL items (hw accel, gl depth, gl stencil, gl msaa */
   arr = eina_str_split_full(pref, ":", 0, &tokens);
   for (i = 0; arr && arr[i]; i++)
     {
        if ((!strcasecmp(arr[i], "gl")) ||
            (!strcasecmp(arr[i], "opengl")) ||
            (!strcasecmp(arr[i], "3d")) ||
            (!strcasecmp(arr[i], "hw")) ||
            (!strcasecmp(arr[i], "accel")) ||
            (!strcasecmp(arr[i], "hardware"))
            )
          {
             eina_stringshare_replace(accel, arr[i]);
             is_hw_accel = EINA_TRUE;
             *gl_depth = 0;
             *gl_stencil = 0;
             *gl_msaa = 0;
          }
        else if (!strncmp(arr[i], "depth", 5))
          {
             char *value_str = arr[i] + 5;
             if ((value_str) && (isdigit(*value_str)))
               *gl_depth = atoi(value_str);
             else
               *gl_depth = 24;
          }
        else if (!strncmp(arr[i], "stencil", 7))
          {
             char *value_str = arr[i] + 7;
             if ((value_str) && (isdigit(*value_str)))
               *gl_stencil = atoi(value_str);
             else
               *gl_stencil = 8;
          }
        else if (!strncmp(arr[i], "msaa_low", 8))
          *gl_msaa = 1;             // 1 means msaa low
        else if (!strncmp(arr[i], "msaa_mid", 8))
          *gl_msaa = 2;             // 2 means msaa mid
        else if (!strncmp(arr[i], "msaa_high", 9))
          *gl_msaa = 4;             // 4 means msaa high
        else if (!strncmp(arr[i], "msaa", 4))
          *gl_msaa = 1;            // 1 means msaa low
     }

   DBG("accel: %s", *accel);
   DBG("gl depth: %d", *gl_depth);
   DBG("gl stencil: %d", *gl_stencil);
   DBG("gl msaa: %d", *gl_msaa);
   if (arr)
     {
        free(arr[0]);
        free(arr);
     }

   return is_hw_accel;
}

EAPI void
elm_config_accel_preference_set(const char *pref)
{
   _elm_config->priv.accel = EINA_TRUE;
   _elm_config->priv.gl_depth = EINA_TRUE;
   _elm_config->priv.gl_stencil = EINA_TRUE;
   _elm_config->priv.gl_msaa = EINA_TRUE;
   if (pref)
     {
        Eina_Bool hw;

        /* full string */
        eina_stringshare_replace(&(_elm_gl_preference), pref);
        ELM_SAFE_FREE(_elm_accel_preference, eina_stringshare_del);
        ELM_SAFE_FREE(_elm_config->accel, eina_stringshare_del);

        hw = _elm_config_accel_preference_parse(pref, &_elm_config->accel,
                                                &_elm_config->gl_depth,
                                                &_elm_config->gl_stencil,
                                                &_elm_config->gl_msaa);

        if (hw)
          {
             eina_stringshare_replace(&(_elm_accel_preference), _elm_config->accel);
          }
        else
          {
             ELM_SAFE_FREE(_elm_config->accel, eina_stringshare_del);
          }
     }
   else
     {
        ELM_SAFE_FREE(_elm_gl_preference, eina_stringshare_del);
        ELM_SAFE_FREE(_elm_accel_preference, eina_stringshare_del);
        ELM_SAFE_FREE(_elm_config->accel, eina_stringshare_del);
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

EAPI void
elm_config_transition_duration_factor_set(double factor)
{
    _elm_config->priv.transition_duration_factor = EINA_TRUE;
    if (factor < 0.0) return;
    if (_elm_config->transition_duration_factor == factor) return;
    _elm_config->transition_duration_factor = factor;
    edje_transition_duration_factor_set(_elm_config->transition_duration_factor);
}

EAPI double
elm_config_transition_duration_factor_get(void)
{
    return _elm_config->transition_duration_factor;
}

EAPI void
elm_config_web_backend_set(const char *backend)
{
   _elm_config->priv.web_backend = EINA_TRUE;
   if (_elm_web_init(backend))
     eina_stringshare_replace(&_elm_config->web_backend, backend);
}

EAPI const char *
elm_config_web_backend_get(void)
{
   if (!_elm_config) return NULL;
   return _elm_config->web_backend;
}

EAPI Eina_Bool
elm_config_offline_get(void)
{
   if (!_elm_config) return EINA_FALSE;
   return _elm_config->offline;
}

EAPI void
elm_config_offline_set(Eina_Bool set)
{
   if (!_elm_config) return;
   _elm_config->offline = !!set;
}

EAPI int
elm_config_powersave_get(void)
{
   if (!_elm_config) return EINA_FALSE;
   return _elm_config->powersave;
}

EAPI void
elm_config_powersave_set(int set)
{
   if (!_elm_config) return;
   _elm_config->powersave = set;
}

EAPI double
elm_config_drag_anim_duration_get(void)
{
   if (!_elm_config) return 0.0;
   return _elm_config->drag_anim_duration;
}

EAPI void
elm_config_drag_anim_duration_set(double set)
{
   if (!_elm_config) return;
   _elm_config->drag_anim_duration = set;
}

void
_elm_config_profile_set(const char *profile)
{
   Eina_Bool is_mirrored;
   Eina_Bool translate;
   is_mirrored = _elm_config->is_mirrored;
   translate = _elm_config->translate;

   if (!profile) return;

   if (_elm_profile)
     {
        if (!strcmp(_elm_profile, profile))
          return;

        free(_elm_profile);
     }

   _elm_profile = strdup(profile);

   _color_overlays_cancel();

   _elm_config_reload_do();

   /* restore prev value which is not part of the EET file */
   _elm_config->is_mirrored = is_mirrored;
   _elm_config->translate = translate;

   _config_apply();
   _elm_config_font_overlay_apply();
   _elm_config_color_overlay_apply();
   _elm_rescale();
   _elm_recache();
   _elm_old_clouseau_reload();
   _elm_config_key_binding_hash();
}

void
_elm_config_shutdown(void)
{
   efl_del_intercept_set(_efl_config_obj, NULL);
   efl_loop_unregister(efl_main_loop_get(), EFL_CONFIG_INTERFACE, _efl_config_obj);
   efl_loop_unregister(efl_main_loop_get(), EFL_CONFIG_GLOBAL_CLASS, _efl_config_obj);
   ELM_SAFE_FREE(_efl_config_obj, efl_del);
   ELM_SAFE_FREE(_elm_config, _config_free);
   ELM_SAFE_FREE(_elm_preferred_engine, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_accel_preference, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_cache_flush_poller, ecore_poller_del);
   ELM_SAFE_FREE(_elm_profile, free);
   _elm_font_overlays_del_free();

   _elm_config_profile_derived_shutdown();
   _desc_shutdown();

   ELM_SAFE_FREE(_elm_key_bindings, eina_hash_free);
}


/* Efl.Config implementation */

typedef const char * cstring;

static inline Eina_Bool
_eina_value_to_int(const Eina_Value *val, int *i)
{
   Eina_Value *ival;
   Eina_Bool ret;

   if (eina_value_type_get(val) == EINA_VALUE_TYPE_INT)
     {
        return eina_value_get(val, i);
     }

   ival = eina_value_new(EINA_VALUE_TYPE_INT);
   if (!eina_value_convert(val, ival))
     ret = EINA_FALSE;
   else
     ret = eina_value_get(ival, i);
   eina_value_free(ival);
   return ret;
}

static inline Eina_Bool
_eina_value_to_cstring(const Eina_Value *val, cstring *s)
{
   Eina_Value *sval = eina_value_new(EINA_VALUE_TYPE_STRING);
   Eina_Bool ret = EINA_TRUE;
   if (!eina_value_convert(val, sval))
     ret = EINA_FALSE;
   else
     ret = eina_value_get(sval, s);
   eina_value_free(sval);
   return ret;
}

static inline Eina_Bool
_edje_channel_get(const char *channel, Edje_Channel *chan)
{

   if (!*channel)
     *chan = EDJE_CHANNEL_ALL;
   else
     {
        if (!strcmp("_effect", channel))
          *chan = EDJE_CHANNEL_EFFECT;
        else if (!strcmp("_background", channel))
          *chan = EDJE_CHANNEL_BACKGROUND;
        else if (!strcmp("_music", channel))
          *chan = EDJE_CHANNEL_MUSIC;
        else if (!strcmp("_foreground", channel))
          *chan = EDJE_CHANNEL_FOREGROUND;
        else if (!strcmp("_interface", channel))
          *chan = EDJE_CHANNEL_INTERFACE;
        else if (!strcmp("_input", channel))
          *chan = EDJE_CHANNEL_INPUT;
        else if (!strcmp("_alert", channel))
          *chan = EDJE_CHANNEL_ALERT;
        else if (!strcmp("_all", channel))
          *chan = EDJE_CHANNEL_ALL;
        else
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

static const struct {
   Efl_Ui_Focus_Autoscroll_Mode  val;
   const char                   *str;
} _enum_map_focus_autoscroll_mode[] = {
{ EFL_UI_FOCUS_AUTOSCROLL_MODE_SHOW, "show" },
{ EFL_UI_FOCUS_AUTOSCROLL_MODE_NONE, "none" },
{ EFL_UI_FOCUS_AUTOSCROLL_MODE_BRING_IN, "bring_in" }
};

static const struct {
   Efl_Ui_Softcursor_Mode  val;
   const char             *str;
} _enum_map_softcursor_mode[] = {
{ EFL_UI_SOFTCURSOR_MODE_AUTO, "auto" },
{ EFL_UI_SOFTCURSOR_MODE_ON, "on" },
{ EFL_UI_SOFTCURSOR_MODE_OFF, "off" }
};

static const struct {
   Efl_Ui_Slider_Indicator_Visible_Mode  val;
   const char                           *str;
} _enum_map_slider_indicator_visible_mode[] = {
{ EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_ON_DRAG, "on_drag" },
{ EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS, "always" },
{ EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS, "on_focus" },
{ EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_NONE, "none" },
};

static const struct {
   Efl_Ui_Focus_Move_Policy  val;
   const char               *str;
} _enum_map_focus_move_policy[] = {
{ EFL_UI_FOCUS_MOVE_POLICY_CLICK, "click" },
{ EFL_UI_FOCUS_MOVE_POLICY_IN, "in" },
{ EFL_UI_FOCUS_MOVE_POLICY_KEY_ONLY, "key_only" }
};

EOLIAN static Eina_Bool
_efl_config_global_efl_config_config_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED,
                                         const char *name, const Eina_Value *val)
{
   if (!name) return EINA_FALSE;
   // TODO: if (!val) reset to default

#define CONFIG_SET(opt, primityp, valtyp, alttyp) do { \
   if (!strcmp(name, #opt)) \
     { \
        primityp v = 0; \
        alttyp alt = 0; \
        const Eina_Value_Type *typ = EINA_VALUE_TYPE_ ## valtyp; \
        if (eina_value_type_get(val) == typ)  \
          { \
             if (!eina_value_get(val, &v)) return EINA_FALSE; \
          } \
        else if (_eina_value_to_ ## alttyp(val, &alt)) \
          { \
             v = alt; \
          } \
        else \
          { \
             ERR("Invalid value type for config '%s' (got %s wanted %s)", \
                 name, eina_value_type_name_get(eina_value_type_get(val)), \
                 eina_value_type_name_get(EINA_VALUE_TYPE_ ## valtyp)); \
             return EINA_FALSE; \
          } \
        elm_config_ ## opt ## _set(v); \
        return EINA_TRUE; \
     } \
   } while (0)

#define CONFIG_SETB(opt) CONFIG_SET(opt, Eina_Bool, UCHAR, int)
#define CONFIG_SETI(opt) CONFIG_SET(opt, int, INT, int)
#define CONFIG_SETD(opt) CONFIG_SET(opt, double, DOUBLE, int)
#define CONFIG_SETS(opt) CONFIG_SET(opt, const char *, STRING, cstring)

#define CONFIG_SETE(opt) do { \
   if (!strcmp(name, #opt)) \
     { \
        int v = -1; \
        if (eina_value_type_get(val) == EINA_VALUE_TYPE_STRING) \
          { \
             const char *str; \
             if (eina_value_get(val, &str)) \
               { \
                  for (unsigned i = 0; i < (sizeof(_enum_map_ ## opt) / sizeof(_enum_map_ ## opt[0])); i++) \
                    { \
                       if (eina_streq(_enum_map_ ## opt[i].str, str)) { v = _enum_map_ ## opt[i].val; break; } \
                    } \
               } \
             if (v == -1) \
               { \
                 ERR("Invalid value for config '%s' (got '%s')", #opt, str); \
                 return EINA_FALSE; \
               } \
          } \
        else if (!_eina_value_to_int(val, &v)) \
          { \
             ERR("Invalid value type for config '%s' (got %s wanted int or string)", \
                 name, eina_value_type_name_get(eina_value_type_get(val))); \
             return EINA_FALSE; \
          } \
        elm_config_ ## opt ## _set(v); \
        return EINA_TRUE; \
     } \
   } while (0)

   CONFIG_SETB(scroll_bounce_enabled);
   CONFIG_SETD(scroll_bounce_friction);
   CONFIG_SETD(scroll_page_scroll_friction);
   CONFIG_SETB(context_menu_disabled);
   CONFIG_SETD(scroll_bring_in_scroll_friction);
   CONFIG_SETD(scroll_zoom_friction);
   CONFIG_SETB(scroll_thumbscroll_enabled);
   CONFIG_SETI(scroll_thumbscroll_threshold);
   CONFIG_SETI(scroll_thumbscroll_hold_threshold);
   CONFIG_SETD(scroll_thumbscroll_momentum_threshold);
   CONFIG_SETI(scroll_thumbscroll_flick_distance_tolerance);
   CONFIG_SETD(scroll_thumbscroll_friction);
   CONFIG_SETD(scroll_thumbscroll_min_friction);
   CONFIG_SETD(scroll_thumbscroll_friction_standard);
   CONFIG_SETD(scroll_thumbscroll_border_friction);
   CONFIG_SETD(scroll_thumbscroll_sensitivity_friction);
   CONFIG_SETB(scroll_thumbscroll_smooth_start);
   CONFIG_SETB(scroll_animation_disabled);
   CONFIG_SETD(scroll_accel_factor);
   CONFIG_SETD(scroll_thumbscroll_smooth_amount);
   CONFIG_SETD(scroll_thumbscroll_smooth_time_window);
   CONFIG_SETD(scroll_thumbscroll_acceleration_threshold);
   CONFIG_SETD(scroll_thumbscroll_acceleration_time_limit);
   CONFIG_SETD(scroll_thumbscroll_acceleration_weight);
   CONFIG_SETE(focus_autoscroll_mode);
   CONFIG_SETE(slider_indicator_visible_mode);
   CONFIG_SETD(longpress_timeout);
   CONFIG_SETE(softcursor_mode);
   CONFIG_SETD(tooltip_delay);
   CONFIG_SETB(cursor_engine_only);
   CONFIG_SETD(scale);
   CONFIG_SETS(icon_theme);
   CONFIG_SETB(password_show_last);
   CONFIG_SETD(password_show_last_timeout);
   CONFIG_SETS(preferred_engine);
   CONFIG_SETS(accel_preference);
   //font_overlay const char *text_class, const char *font, Evas_Font_Size size);
   CONFIG_SETB(access);
   CONFIG_SETB(selection_unfocused_clear);
   //elm_config.h:EAPI void             elm_config_font_overlay_unset(const char *text_class);
   CONFIG_SETI(font_hint_type);
   CONFIG_SETI(finger_size);
   CONFIG_SETI(cache_flush_interval);
   CONFIG_SETB(cache_flush_enabled);
   CONFIG_SETI(cache_font_cache_size);
   CONFIG_SETI(cache_image_cache_size);
   CONFIG_SETI(cache_edje_file_cache_size);
   CONFIG_SETI(cache_edje_collection_cache_size);
   CONFIG_SETB(vsync);
   CONFIG_SETB(accel_preference_override);
   CONFIG_SETB(focus_highlight_enabled);
   CONFIG_SETB(focus_highlight_animate);
   CONFIG_SETB(focus_highlight_clip_disabled);
   CONFIG_SETE(focus_move_policy);
   CONFIG_SETB(item_select_on_focus_disabled);
   CONFIG_SETB(first_item_focus_on_first_focusin);
   CONFIG_SETB(mirrored);
   CONFIG_SETB(clouseau_enabled);
   CONFIG_SETD(glayer_long_tap_start_timeout);
   CONFIG_SETD(glayer_double_tap_timeout);
   //color_overlay const char *color_class,
   //elm_config.h:EAPI void      elm_config_color_overlay_unset(const char *color_class);
   CONFIG_SETB(magnifier_enable);
   CONFIG_SETD(magnifier_scale);
   CONFIG_SETB(window_auto_focus_enable);
   CONFIG_SETB(window_auto_focus_animate);
   CONFIG_SETB(popup_scrollable);
   CONFIG_SETB(atspi_mode);
   CONFIG_SETD(transition_duration_factor);
   CONFIG_SETS(web_backend);
   CONFIG_SETB(offline);
   CONFIG_SETI(powersave);

   const size_t len = sizeof("audio_mute") - 1;
   if (!strncmp(name, "audio_mute", len))
     {
        const char *channel = name + len;
        Edje_Channel chan;
        int v;
        if (!_eina_value_to_int(val, &v))
          {
             ERR("Invalid value type for config '%s' (got %s wanted int)",
                 name, eina_value_type_name_get(eina_value_type_get(val)));
             return EINA_FALSE;
          }
        if (!_edje_channel_get(channel, &chan))
          {
             ERR("Unknown audio channel '%s'", channel);
             return EINA_FALSE;
          }
        elm_config_audio_mute_set(chan, !!v);
#ifdef ENABLE_MULTISENSE
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
     }

   ERR("Config '%s' does not exist", name);
   return EINA_FALSE;
}

EOLIAN static Eina_Value *
_efl_config_global_efl_config_config_get(const Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED,
                                         const char *name)
{
   Eina_Value *val = NULL;

   if (!name) return NULL;

   // NOTE: returning INT instead of UINT for unsigned int types

#define CONFIG_GET(opt, primityp, valtyp) do { \
   if (!strcmp(name, #opt)) \
     { \
        val = eina_value_new(EINA_VALUE_TYPE_ ## valtyp); \
        eina_value_set(val, elm_config_ ## opt ## _get()); \
        return val; \
     } \
   } while (0)

#define CONFIG_GETB(opt) CONFIG_GET(opt, Eina_Bool, UCHAR)
#define CONFIG_GETI(opt) CONFIG_GET(opt, int, INT)
#define CONFIG_GETD(opt) CONFIG_GET(opt, double, DOUBLE)
#define CONFIG_GETS(opt) CONFIG_GET(opt, const char *, STRING)

#define CONFIG_GETE(opt) do { \
   if (!strcmp(name, #opt)) \
     { \
        int v = elm_config_ ## opt ## _get(); \
        if ((v < 0) || (v > (int)(sizeof(_enum_map_ ## opt) / sizeof(_enum_map_ ## opt[0])))) \
          v = 0; \
        val = eina_value_new(EINA_VALUE_TYPE_STRING); \
        eina_value_set(val, _enum_map_ ## opt[v].str); \
        return val; \
     } \
   } while (0)

   CONFIG_GETB(scroll_bounce_enabled);
   CONFIG_GETD(scroll_bounce_friction);
   CONFIG_GETD(scroll_page_scroll_friction);
   CONFIG_GETB(context_menu_disabled);
   CONFIG_GETD(scroll_bring_in_scroll_friction);
   CONFIG_GETD(scroll_zoom_friction);
   CONFIG_GETB(scroll_thumbscroll_enabled);
   CONFIG_GETI(scroll_thumbscroll_threshold);
   CONFIG_GETI(scroll_thumbscroll_hold_threshold);
   CONFIG_GETD(scroll_thumbscroll_momentum_threshold);
   CONFIG_GETI(scroll_thumbscroll_flick_distance_tolerance);
   CONFIG_GETD(scroll_thumbscroll_friction);
   CONFIG_GETD(scroll_thumbscroll_min_friction);
   CONFIG_GETD(scroll_thumbscroll_friction_standard);
   CONFIG_GETD(scroll_thumbscroll_border_friction);
   CONFIG_GETD(scroll_thumbscroll_sensitivity_friction);
   CONFIG_GETB(scroll_thumbscroll_smooth_start);
   CONFIG_GETB(scroll_animation_disabled);
   CONFIG_GETD(scroll_accel_factor);
   CONFIG_GETD(scroll_thumbscroll_smooth_amount);
   CONFIG_GETD(scroll_thumbscroll_smooth_time_window);
   CONFIG_GETD(scroll_thumbscroll_acceleration_threshold);
   CONFIG_GETD(scroll_thumbscroll_acceleration_time_limit);
   CONFIG_GETD(scroll_thumbscroll_acceleration_weight);
   CONFIG_GETE(focus_autoscroll_mode);
   CONFIG_GETE(slider_indicator_visible_mode);
   CONFIG_GETD(longpress_timeout);
   CONFIG_GETE(softcursor_mode);
   CONFIG_GETD(tooltip_delay);
   CONFIG_GETB(cursor_engine_only);
   CONFIG_GETD(scale);
   CONFIG_GETS(icon_theme);
   CONFIG_GETB(password_show_last);
   CONFIG_GETD(password_show_last_timeout);
   CONFIG_GETS(preferred_engine);
   CONFIG_GETS(accel_preference);
   //font_overlay
   CONFIG_GETB(access);
   CONFIG_GETB(selection_unfocused_clear);
   //elm_config_font_overlay_unset
   //CONFIG_GETI(font_hint_type); // this has no get!
   CONFIG_GETI(finger_size);
   CONFIG_GETI(cache_flush_interval);
   CONFIG_GETB(cache_flush_enabled);
   CONFIG_GETI(cache_font_cache_size);
   CONFIG_GETI(cache_image_cache_size);
   CONFIG_GETI(cache_edje_file_cache_size);
   CONFIG_GETI(cache_edje_collection_cache_size);
   CONFIG_GETB(vsync);
   CONFIG_GETB(accel_preference_override);
   CONFIG_GETB(focus_highlight_enabled);
   CONFIG_GETB(focus_highlight_animate);
   CONFIG_GETB(focus_highlight_clip_disabled);
   CONFIG_GETE(focus_move_policy);
   CONFIG_GETB(item_select_on_focus_disabled);
   CONFIG_GETB(first_item_focus_on_first_focusin);
   CONFIG_GETB(mirrored);
   CONFIG_GETB(clouseau_enabled);
   CONFIG_GETD(glayer_long_tap_start_timeout);
   CONFIG_GETD(glayer_double_tap_timeout);
   //color_overlay
   //color_overlay_unset
   CONFIG_GETB(magnifier_enable);
   CONFIG_GETD(magnifier_scale);
   CONFIG_GETB(window_auto_focus_enable);
   CONFIG_GETB(window_auto_focus_animate);
   CONFIG_GETB(popup_scrollable);
   CONFIG_GETB(atspi_mode);
   CONFIG_GETD(transition_duration_factor);
   CONFIG_GETS(web_backend);
   CONFIG_GETB(offline);
   CONFIG_GETI(powersave);
   CONFIG_GETD(drag_anim_duration);

   const size_t len = sizeof("audio_mute") - 1;
   if (!strncmp(name, "audio_mute", len))
     {
        const char *channel = name + len;
        Edje_Channel chan;
        Eina_Bool b;
        if (!_edje_channel_get(channel, &chan))
          {
             ERR("Unknown audio channel '%s'", channel);
             return NULL;
          }
        val = eina_value_new(EINA_VALUE_TYPE_UCHAR);
        b = elm_config_audio_mute_get(chan);
        if (b != edje_audio_channel_mute_get(chan))
          ERR("config state for audio channel '%s' does not match active state!", channel);
        eina_value_set(val, b);
        return val;
     }

   ERR("Config '%s' does not exist", name);
   return NULL;
}

EOLIAN static void
_efl_config_global_profile_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, const char *profile)
{
   elm_config_profile_set(profile);
}

EOLIAN static const char *
_efl_config_global_profile_get(const Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return elm_config_profile_get();
}

EOLIAN static Eina_Bool
_efl_config_global_save(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, const char *profile)
{
   return _elm_config_save(_elm_config, profile);
}

typedef struct
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
} Profile_Iterator;

static Eina_Bool
_profile_iterator_next(Profile_Iterator *it, void **data)
{
   Efl_Gfx_Entity *sub;

   if (!it->object) return EINA_FALSE;
   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_profile_iterator_get_container(Profile_Iterator *it)
{
   return it->object;
}

static void
_profile_iterator_free(Profile_Iterator *it)
{
   efl_wref_del(it->object, &it->object);
   eina_iterator_free(it->real_iterator);
   elm_config_profile_list_free(it->list);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_config_global_profile_iterate(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, Eina_Bool hidden)
{
   Profile_Iterator *it;
   Eina_List *list;

   list = _elm_config_profiles_list(!hidden);
   if (!list) return NULL;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = list;
   it->real_iterator = eina_list_iterator_new(it->list);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_profile_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_profile_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_profile_iterator_free);
   efl_wref_add(obj, &it->object);

   return &it->iterator;
}

EOLIAN static Eina_Bool
_efl_config_global_profile_exists(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, const char *profile)
{
   return elm_config_profile_exists(profile);
}

EOLIAN static Eina_Stringshare *
_efl_config_global_profile_dir_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, const char *profile, Eina_Bool is_user)
{
   Eina_Stringshare *str;
   const char *dir;

   dir = elm_config_profile_dir_get(profile, is_user);
   str = eina_stringshare_add(dir);
   elm_config_profile_dir_free(dir);

   return str;
}

EOLIAN static void
_efl_config_global_profile_derived_add(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, const char *profile, const char *options)
{
   elm_config_profile_derived_add(profile, options);
}

EOLIAN static void
_efl_config_global_profile_derived_del(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, const char *profile)
{
   elm_config_profile_derived_del(profile);
}

#include "efl_config_global.eo.c"
