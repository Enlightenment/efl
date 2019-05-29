#ifndef ELM_PRIV_H
# define ELM_PRIV_H
# ifdef HAVE_ELEMENTARY_X
#  include <Ecore_X.h>
# endif
# ifdef HAVE_ELEMENTARY_FB
#  include <Ecore_Fb.h>
# endif
# ifdef HAVE_ELEMENTARY_WL2
#  include <Ecore_Wl2.h>
# endif
# ifdef HAVE_ELEMENTARY_DRM
#  include <Ecore_Drm2.h>
# endif
# ifdef HAVE_ELEMENTARY_COCOA
#  include <Ecore_Cocoa.h>
# endif
# ifdef HAVE_ELEMENTARY_WIN32
#  include <Ecore_Win32.h>
# endif

# include <Eio.h>

// Evas internal EO APIs
# include "Evas.h"
# include "Evas_Internal.h"

#include "Elementary.h"
#include "Efl_Ui.h"
# ifdef EAPI
#  undef EAPI
# endif
# ifdef EWAPI
#  undef EWAPI
# endif

# ifdef _WIN32
#  ifdef EFL_BUILD
#   ifdef DLL_EXPORT
#    define EAPI __declspec(dllexport)
#   else
#    error "no DLL_EXPORT"
#    define EAPI
#   endif
#  else
#   define EAPI __declspec(dllimport)
#  endif
#  define EAPI_WEAK
# else
#  ifdef __GNUC__
#   if __GNUC__ >= 4
#    define EAPI __attribute__ ((visibility("default")))
#    define EAPI_WEAK __attribute__ ((weak))
#   else
#    define EAPI
#    define EAPI_WEAK
#   endif
#  else
#   define EAPI
#   define EAPI_WEAK
#  endif
# endif

# define EWAPI EAPI EAPI_WEAK

# include "elm_widget.h"
# include "elm_code_private.h"
#include "elm_access_eo.h"
#include "elm_actionslider_eo.h"
#include "elm_atspi_app_object_eo.h"
#include "elm_atspi_bridge_eo.h"
#include "elm_box_eo.h"
#include "elm_bubble_eo.h"
#include "elm_calendar_eo.h"
#include "elm_calendar_item_eo.h"
#include "elm_clock_eo.h"
#include "elm_code_widget_legacy_eo.h"
#include "elm_color_item_eo.h"
#include "elm_colorselector_eo.h"
#include "elm_conformant_eo.h"
#include "elm_ctxpopup_eo.h"
#include "elm_ctxpopup_item_eo.h"
#include "elm_dayselector_eo.h"
#include "elm_dayselector_item_eo.h"
#include "elm_diskselector_eo.h"
#include "elm_diskselector_item_eo.h"
#include "elm_entry_eo.h"
#include "elm_fileselector_button_eo.h"
#include "elm_fileselector_entry_eo.h"
#include "elm_fileselector_eo.h"
#include "elm_flipselector_eo.h"
#include "elm_flipselector_item_eo.h"
#include "elm_gengrid_eo.h"
#include "elm_gengrid_item_eo.h"
#include "elm_gengrid_pan_eo.h"
#include "elm_genlist_eo.h"
#include "elm_genlist_item_eo.h"
#include "elm_genlist_pan_eo.h"
#include "elm_gesture_layer_eo.h"
#include "elm_glview_eo.h"
#include "elm_grid_eo.h"
#include "elm_hover_eo.h"
#include "elm_hoversel_eo.h"
#include "elm_hoversel_item_eo.h"
#include "elm_icon_eo.h"
#include "elm_image_eo.h"
#include "elm_index_eo.h"
#include "elm_index_item_eo.h"
#include "elm_interface_fileselector_eo.h"
#include "elm_inwin_eo.h"
#include "elm_label_eo.h"
#include "elm_list_eo.h"
#include "elm_list_item_eo.h"
#include "elm_mapbuf_eo.h"
#include "elm_map_eo.h"
#include "elm_map_pan_eo.h"
#include "elm_menu_eo.h"
#include "elm_menu_item_eo.h"
#include "elm_multibuttonentry_eo.h"
#include "elm_multibuttonentry_item_eo.h"
#include "elm_naviframe_eo.h"
#include "elm_naviframe_item_eo.h"
#include "elm_notify_eo.h"
#include "elm_panel_eo.h"
#include "elm_pan_eo.h"
#include "elm_photo_eo.h"
#include "elm_player_eo.h"
#include "elm_plug_eo.h"
#include "elm_popup_eo.h"
#include "elm_popup_item_eo.h"
#include "elm_prefs_eo.h"
#include "elm_route_eo.h"
#include "elm_scroller_eo.h"
#include "elm_segment_control_eo.h"
#include "elm_segment_control_item_eo.h"
#include "elm_separator_eo.h"
#include "elm_slider_eo.h"
#include "elm_slider_part_indicator_eo.h"
#include "elm_slideshow_eo.h"
#include "elm_slideshow_item_eo.h"
#include "elm_spinner_eo.h"
#include "elm_sys_notify_dbus_eo.h"
#include "elm_sys_notify_eo.h"
#include "elm_sys_notify_interface_eo.h"
#include "elm_systray_eo.h"
#include "elm_table_eo.h"
#include "elm_thumb_eo.h"
#include "elm_toolbar_eo.h"
#include "elm_toolbar_item_eo.h"
#include "elm_web_eo.h"
#include "elm_widget_item_container_eo.h"
#include "elm_widget_item_eo.h"
#include "elm_widget_item_static_focus_eo.h"
#include "elm_win_eo.h"

# include "efl_ui_focus_parent_provider.eo.h"
# include "efl_ui_widget_focus_manager.eo.h"
# include "efl_ui_focus_parent_provider_standard.eo.h"
# include "elm_widget_item_static_focus_eo.h"
# include "efl_ui_selection_manager.eo.h"
# include "efl_datetime_manager.eo.h"
# include "efl_ui_size_model.eo.h"
# include "efl_ui_homogeneous_model.eo.h"
# include "efl_ui_exact_model.eo.h"
# include "efl_ui_average_model.eo.h"

extern const char *_efl_model_property_itemw;
extern const char *_efl_model_property_itemh;
extern const char *_efl_model_property_selfw;
extern const char *_efl_model_property_selfh;
extern const char *_efl_model_property_totalw;
extern const char *_efl_model_property_totalh;

# ifdef HAVE_LANGINFO_H
#  include <langinfo.h>
# endif

# define CRI(...)      EINA_LOG_DOM_CRIT(_elm_log_dom, __VA_ARGS__)
# define ERR(...)      EINA_LOG_DOM_ERR(_elm_log_dom, __VA_ARGS__)
# define WRN(...)      EINA_LOG_DOM_WARN(_elm_log_dom, __VA_ARGS__)
# define INF(...)      EINA_LOG_DOM_INFO(_elm_log_dom, __VA_ARGS__)
# define DBG(...)      EINA_LOG_DOM_DBG(_elm_log_dom, __VA_ARGS__)

# ifdef ENABLE_NLS
#  include <libintl.h>
#  define E_(string)    _elm_dgettext(string)
# else
#  ifndef setlocale
#   define setlocale(c, l)
#  endif
#  ifndef libintl_setlocale
#   define libintl_setlocale(c, l)
#  endif
#  ifndef bindtextdomain
#   define bindtextdomain(domain, dir)
#  endif
#  ifndef libintl_bindtextdomain
#   define libintl_bindtextdomain(domain, dir)
#  endif
#  define E_(string) (string)
# endif
# define N_(string) (string)

typedef struct _Elm_Theme_Files          Elm_Theme_Files;
typedef struct _Edje_Signal_Data         Edje_Signal_Data;
typedef struct _Elm_Config_Flags         Elm_Config_Flags;
typedef struct _Elm_Config               Elm_Config;
typedef struct _Elm_Config_Bindings_Widget   Elm_Config_Bindings_Widget;
typedef struct _Elm_Config_Binding_Key   Elm_Config_Binding_Key;
typedef struct _Elm_Config_Binding_Modifier  Elm_Config_Binding_Modifier;
typedef struct _Elm_Module               Elm_Module;
typedef struct _Efl_Ui_Theme_Data        Efl_Ui_Theme_Data;

struct _Edje_Signal_Data
{
   Evas_Object   *obj;
   Edje_Signal_Cb func;
   const char    *emission;
   const char    *source;
   void          *data;
};

typedef struct Elm_Theme_File
{
   EINA_INLIST;
   /*
    * We are conserving a list of path even if that's duplicated
    * because we expose those directly to the outside world :'(
    */
   Eina_Stringshare *item;
   Eina_File *handle;
   Eina_Stringshare *base_theme; // data.item: efl_theme_base name
   Eina_Stringshare *match_theme; // data.item: efl_theme_match name
} Elm_Theme_File;

struct _Elm_Theme
{
   Eina_Inlist *overlay;
   Eina_Inlist *themes;
   Eina_Inlist *extension;

   Eina_Hash  *cache;
   Eina_Hash  *cache_data;
   Elm_Theme  *ref_theme;
   Eina_List  *referrers;
   const char *theme;
   Eina_Hash  *cache_style_load_failed;

   /* these only exist to preserve compat with bad elm_theme_XYZ_list_get() api */
   Eina_List *overlay_items;
   Eina_List *theme_items;
   Eina_List *extension_items;

   Efl_Ui_Theme *eo_theme; //for accessing through the Eo interface
};

struct _Efl_Ui_Theme_Data
{
   Elm_Theme *th;
};

/* increment this whenever we change config enough that you need new
 * defaults for elm to work.
 */
# define ELM_CONFIG_EPOCH           0x0002
/* increment this whenever a new set of config values are added but
 * the users config doesn't need to be wiped - simply new values need
 * to be put in
 */
# define ELM_CONFIG_FILE_GENERATION 0x0015
# define ELM_CONFIG_VERSION_EPOCH_OFFSET 16
# define ELM_CONFIG_VERSION         ((ELM_CONFIG_EPOCH << ELM_CONFIG_VERSION_EPOCH_OFFSET) | \
                                     ELM_CONFIG_FILE_GENERATION)
/* NB: profile configuration files (.src) must have their
 * "config_version" entry's value up-to-date with ELM_CONFIG_VERSION
 * (in decimal)!! */

/* note: always remember to sync it with elm_config.c */
extern const char *_elm_engines[];

# define ELM_SOFTWARE_X11      (_elm_engines[0])
# define ELM_SOFTWARE_FB       (_elm_engines[1])
# define ELM_OPENGL_X11        (_elm_engines[2])
# define ELM_SOFTWARE_WIN32    (_elm_engines[3])
# define ELM_SOFTWARE_SDL      (_elm_engines[4])
# define ELM_OPENGL_SDL        (_elm_engines[5])
# define ELM_BUFFER            (_elm_engines[6])
# define ELM_EWS               (_elm_engines[7])
# define ELM_OPENGL_COCOA      (_elm_engines[8])
# define ELM_WAYLAND_SHM       (_elm_engines[9])
# define ELM_WAYLAND_EGL       (_elm_engines[10])
# define ELM_DRM               (_elm_engines[11])
# define ELM_SOFTWARE_DDRAW    (_elm_engines[12])
# define ELM_GL_DRM            (_elm_engines[13])

# define ELM_FONT_TOKEN_STYLE  ":style="

# define ELM_ACCESS_MODE_OFF   EINA_FALSE
# define ELM_ACCESS_MODE_ON    EINA_TRUE

# define ELM_ATSPI_MODE_OFF   EINA_FALSE
# define ELM_ATSPI_MODE_ON    EINA_TRUE

/* convenience macro to compress code and avoid typos */
# undef CEIL
# define CEIL(a)   (((a) % 2 != 0) ? ((a) / 2 + 1) : ((a) / 2))
# undef IS_INSIDE
# define IS_INSIDE(x, y, xx, yy, ww, hh) \
   (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && \
    ((x) >= (xx)) && ((y) >= (yy)))


# define ELM_SAFE_FREE(_h, _fn) do { _fn((void*)_h); _h = NULL; } while (0)
# define ELM_SAFE_DEL(_h) do { efl_del(_h); _h = NULL; } while (0)

# define ELM_PRIV_STATIC_VARIABLE_DECLARE(name, signal, type) \
   static const char name[] = signal;

# define ELM_PRIV_SMART_CALLBACKS_DESC(name, signal, type) \
   {name, type},

struct _Elm_Config_Flags
{
   Eina_Bool engine : 1;
   Eina_Bool accel : 1;
   Eina_Bool web_backend : 1;
   Eina_Bool accel_override : 1;
   Eina_Bool vsync : 1;
   Eina_Bool thumbscroll_enable : 1;
   Eina_Bool thumbscroll_threshold : 1;
   Eina_Bool thumbscroll_hold_threshold : 1;
   Eina_Bool thumbscroll_momentum_threshold : 1;
   Eina_Bool thumbscroll_bounce_friction : 1;
   Eina_Bool thumbscroll_acceleration_threshold : 1;
   Eina_Bool thumbscroll_acceleration_time_limit : 1;
   Eina_Bool thumbscroll_acceleration_weight : 1;
   //new
   Eina_Bool thumbscroll_momentum_friction : 1;
   Eina_Bool thumbscroll_momentum_distance_max : 1;
   Eina_Bool thumbscroll_momentum_animation_duration_min_limit : 1;
   Eina_Bool thumbscroll_momentum_animation_duration_max_limit : 1;
   //deprecated
   Eina_Bool thumbscroll_flick_distance_tolerance : 1;
   Eina_Bool thumbscroll_friction : 1;
   Eina_Bool thumbscroll_min_friction : 1;
   Eina_Bool thumbscroll_friction_standard : 1;
   //
   Eina_Bool page_scroll_friction : 1;
   Eina_Bool bring_in_scroll_friction : 1;
   Eina_Bool zoom_friction : 1;
   Eina_Bool scroll_animation_disable : 1;
   Eina_Bool scroll_accel_factor : 1;
   Eina_Bool thumbscroll_bounce_enable : 1;
   Eina_Bool thumbscroll_border_friction : 1;
   Eina_Bool thumbscroll_sensitivity_friction : 1;
   Eina_Bool scroll_smooth_start_enable : 1;
   Eina_Bool scroll_smooth_amount : 1;
   Eina_Bool scroll_smooth_time_window : 1;
   Eina_Bool scale : 1;
   Eina_Bool bgpixmap : 1; // unused
   Eina_Bool compositing : 1; // unused
   Eina_Bool font_dirs : 1; // unused
   Eina_Bool font_overlays : 1;
   Eina_Bool font_hinting : 1;
   Eina_Bool cache_flush_poll_interval : 1;
   Eina_Bool cache_flush_enable : 1;
   Eina_Bool image_cache : 1;
   Eina_Bool font_cache : 1;
   Eina_Bool edje_cache : 1;
   Eina_Bool edje_collection_cache : 1;
   Eina_Bool finger_size : 1;
   Eina_Bool fps : 1; // unused
   Eina_Bool theme : 1; // unused
   Eina_Bool modules : 1; // unused
   Eina_Bool tooltip_delay : 1;
   Eina_Bool cursor_engine_only : 1;
   Eina_Bool focus_highlight_enable : 1;
   Eina_Bool focus_highlight_animate : 1;
   Eina_Bool focus_highlight_clip_disable : 1;
   Eina_Bool focus_move_policy : 1;
   Eina_Bool item_select_on_focus_disable : 1;
   Eina_Bool first_item_focus_on_first_focus_in : 1;
   Eina_Bool focus_autoscroll_mode : 1;
   Eina_Bool slider_indicator_visible_mode : 1;
   Eina_Bool popup_horizontal_align : 1; // unused
   Eina_Bool popup_vertical_align : 1; // unused
   Eina_Bool popup_scrollable : 1;
   Eina_Bool toolbar_shrink_mode : 1; // unused
   Eina_Bool fileselector_expand_enable : 1; // unused
   Eina_Bool fileselector_double_tap_navigation_enable : 1; // unused
   Eina_Bool inwin_dialogs_enable : 1; // unused
   Eina_Bool icon_size : 1; // unused
   Eina_Bool longpress_timeout : 1;
   Eina_Bool effect_enable : 1; // unused
   Eina_Bool desktop_entry : 1; // unused
   Eina_Bool context_menu_disabled : 1;
   Eina_Bool password_show_last : 1;
   Eina_Bool password_show_last_timeout : 1;
   Eina_Bool glayer_zoom_finger_enable : 1; // unused
   Eina_Bool glayer_zoom_finger_factor : 1; // unused
   Eina_Bool glayer_zoom_wheel_factor : 1; // unused
   Eina_Bool glayer_zoom_distance_tolerance : 1; // unused
   Eina_Bool glayer_rotate_finger_enable : 1; // unused
   Eina_Bool glayer_rotate_angular_tolerance : 1; // unused
   Eina_Bool glayer_line_min_length : 1; // unused
   Eina_Bool glayer_line_distance_tolerance : 1; // unused
   Eina_Bool glayer_line_angular_tolerance : 1; // unused
   Eina_Bool glayer_flick_time_limit_ms : 1; // unused
   Eina_Bool glayer_long_tap_start_timeout : 1;
   Eina_Bool glayer_double_tap_timeout : 1;
   Eina_Bool glayer_tap_finger_size : 1;
   Eina_Bool access_mode : 1;
   Eina_Bool glayer_continues_enable : 1; // unused
   Eina_Bool week_start : 1; // unused
   Eina_Bool weekend_start : 1; // unused
   Eina_Bool weekend_len : 1; // unused
   Eina_Bool year_min : 1; // unused
   Eina_Bool year_max : 1; // unused
   Eina_Bool color_overlays : 1;
   Eina_Bool color_palette : 1; // unused
   Eina_Bool softcursor_mode : 1;
   Eina_Bool auto_norender_withdrawn : 1; // unused
   Eina_Bool auto_norender_iconified_same_as_withdrawn : 1; // unused
   Eina_Bool auto_flush_withdrawn : 1; // unused
   Eina_Bool auto_dump_withdrawn : 1; // unused
   Eina_Bool auto_throttle : 1; // unused
   Eina_Bool auto_throttle_amount : 1; // unused
   Eina_Bool indicator_service_0 : 1; // unused
   Eina_Bool indicator_service_90 : 1; // unused
   Eina_Bool indicator_service_180 : 1; // unused
   Eina_Bool indicator_service_270 : 1; // unused
   Eina_Bool selection_clear_enable : 1;
   Eina_Bool disable_external_menu : 1;
   Eina_Bool clouseau_enable : 1;
   Eina_Bool magnifier_enable : 1;
   Eina_Bool magnifier_scale : 1;
   Eina_Bool audio_mute_effect : 1;
   Eina_Bool audio_mute_background : 1;
   Eina_Bool audio_mute_music : 1;
   Eina_Bool audio_mute_foreground : 1;
   Eina_Bool audio_mute_interface : 1;
   Eina_Bool audio_mute_input : 1;
   Eina_Bool audio_mute_alert : 1;
   Eina_Bool audio_mute_all : 1;
   Eina_Bool win_auto_focus_enable : 1;
   Eina_Bool win_auto_focus_animate : 1;
   Eina_Bool transition_duration_factor : 1;
   Eina_Bool naviframe_prev_btn_auto_pushed : 1; // unused
   Eina_Bool spinner_min_max_filter_enable : 1; // unused
   Eina_Bool bindings : 1; // unused
   Eina_Bool atspi_mode : 1;
   Eina_Bool gl_depth : 1;
   Eina_Bool gl_stencil : 1;
   Eina_Bool gl_msaa : 1;
   Eina_Bool icon_theme : 1;
   Eina_Bool entry_select_allow : 1; // unused
   Eina_Bool drag_anim_duration : 1;
   Eina_Bool win_no_border : 1;
};

struct _Elm_Config
{
   int           config_version;

   const char   *engine;
   const char   *accel;
   const char   *web_backend;
   unsigned char accel_override;
   unsigned char vsync;
   unsigned char thumbscroll_enable;
   int           thumbscroll_threshold;
   int           thumbscroll_hold_threshold;
   double        thumbscroll_momentum_threshold;
   double        thumbscroll_bounce_friction;
   double        thumbscroll_acceleration_threshold;
   double        thumbscroll_acceleration_time_limit;
   double        thumbscroll_acceleration_weight;
   //
   int           thumbscroll_momentum_distance_max;
   double        thumbscroll_momentum_friction;
   double        thumbscroll_momentum_animation_duration_min_limit;
   double        thumbscroll_momentum_animation_duration_max_limit;
   //deprecated
   int           thumbscroll_flick_distance_tolerance;
   double        thumbscroll_friction;
   double        thumbscroll_min_friction;
   double        thumbscroll_friction_standard;
   //
   double        page_scroll_friction;
   double        bring_in_scroll_friction;
   double        zoom_friction;
   Eina_Bool     scroll_animation_disable;
   double        scroll_accel_factor;
   unsigned char thumbscroll_bounce_enable;
   double        thumbscroll_border_friction;
   double        thumbscroll_sensitivity_friction;
   unsigned char scroll_smooth_start_enable;
//   double        scroll_smooth_time_interval;; // not used anymore
   double        scroll_smooth_amount;
//   double        scroll_smooth_history_weight;; // not used anymore
//   double        scroll_smooth_future_time;; // not used anymore
   double        scroll_smooth_time_window;
   double        scale;
   int           bgpixmap;
   int           compositing;
   Eina_List    *font_dirs; // priv flags for Elm_Config_Flags -> up to here...
   Eina_List    *font_overlays;
   int           font_hinting;
   int           cache_flush_poll_interval;
   unsigned char cache_flush_enable;
   int           image_cache;
   int           font_cache;
   int           edje_cache;
   int           edje_collection_cache;
   int           finger_size;
   double        fps;
   const char   *theme;
   const char   *modules;
   double        tooltip_delay;
   unsigned char cursor_engine_only;
   unsigned char focus_highlight_enable;
   unsigned char focus_highlight_animate;
   unsigned char focus_highlight_clip_disable; /**< This shows disabled status of focus highlight clip feature. This value is false by default so the focus highlight is clipped. */
   unsigned char focus_move_policy; /**< This show how the elementary focus is moved to another object. Focus can be moved by click or mouse_in. */
   unsigned char item_select_on_focus_disable; /**< This shows the disabled status of select on focus feature. This value is false by default so that select on focus feature is enabled by default.*/
   unsigned char first_item_focus_on_first_focus_in;  /**< This sets the first item focus on first focus in feature*/
   Elm_Focus_Autoscroll_Mode focus_autoscroll_mode; /**< This shows the focus auto scroll mode. By default, @c ELM_FOCUS_AUTOSCROLL_MODE_SHOW is set. */
   Elm_Slider_Indicator_Visible_Mode  slider_indicator_visible_mode;  /**< this sets the slider indicator visible mode */
   double        popup_horizontal_align;
   double        popup_vertical_align;
   Eina_Bool     popup_scrollable;
   int           toolbar_shrink_mode;
   unsigned char fileselector_expand_enable;
   unsigned char fileselector_double_tap_navigation_enable;
   unsigned char inwin_dialogs_enable;
   int           icon_size;
   double        longpress_timeout;
   unsigned char effect_enable;
   unsigned char desktop_entry;
   unsigned char context_menu_disabled;
   unsigned char password_show_last;
   double        password_show_last_timeout;
   unsigned char glayer_zoom_finger_enable;
   double        glayer_zoom_finger_factor;
   double        glayer_zoom_wheel_factor;
   double        glayer_zoom_distance_tolerance;
   double        glayer_rotate_finger_enable;
   double        glayer_rotate_angular_tolerance;
   double        glayer_line_min_length;
   double        glayer_line_distance_tolerance;
   double        glayer_line_angular_tolerance;
   unsigned int  glayer_flick_time_limit_ms;
   double        glayer_long_tap_start_timeout;
   double        glayer_double_tap_timeout;
   int           glayer_tap_finger_size;
   Eina_Bool     access_mode;
   unsigned char glayer_continues_enable;
   int           week_start;
   int           weekend_start;
   int           weekend_len;
   int           year_min;
   int           year_max;
   Eina_List    *color_overlays;
   Eina_List    *color_palette;
   unsigned char softcursor_mode;
   unsigned char auto_norender_withdrawn;
   unsigned char auto_norender_iconified_same_as_withdrawn;
   unsigned char auto_flush_withdrawn;
   unsigned char auto_dump_withdrawn;
   unsigned char auto_throttle;
   double        auto_throttle_amount;
   const char   *indicator_service_0;
   const char   *indicator_service_90;
   const char   *indicator_service_180;
   const char   *indicator_service_270;
   unsigned char selection_clear_enable;
   unsigned char disable_external_menu;
   unsigned char clouseau_enable;
   unsigned char magnifier_enable;
   double        magnifier_scale;
   unsigned char audio_mute_effect;
   unsigned char audio_mute_background;
   unsigned char audio_mute_music;
   unsigned char audio_mute_foreground;
   unsigned char audio_mute_interface;
   unsigned char audio_mute_input;
   unsigned char audio_mute_alert;
   unsigned char audio_mute_all;
   unsigned char win_auto_focus_enable;
   unsigned char win_auto_focus_animate;
   double        transition_duration_factor;
   unsigned char naviframe_prev_btn_auto_pushed;
   unsigned char spinner_min_max_filter_enable;
   Eina_List    *bindings;
   Eina_Bool     atspi_mode;
   int           gl_depth;
   int           gl_stencil;
   int           gl_msaa;
   const char   *icon_theme;
   unsigned char entry_select_allow;
   Eina_Bool     offline;
   int  powersave;
   double        drag_anim_duration;
   unsigned char win_no_border;

   /* Not part of the EET file */
   Eina_Bool     is_mirrored : 1;
   Eina_Bool     translate : 1;

   Elm_Config_Flags priv;
};

struct _Elm_Config_Bindings_Widget
{
   const char   *name;
   Eina_List    *key_bindings;
};

struct _Elm_Config_Binding_Key
{
   int           context;
   const char   *key;
   const char   *action;
   const char   *params;
   Eina_List    *modifiers;
};

struct _Elm_Config_Binding_Modifier
{
   const char   *mod;
   unsigned char flag;
};

struct _Elm_Module
{
   int          version;
   const char  *name;
   const char  *as;
   const char  *so_path;
   const char  *data_dir;
   const char  *bin_dir;
   Eina_Module *module;
   void        *data;
   void        *api;
   int          (*init_func)(Elm_Module *m);
   int          (*shutdown_func)(Elm_Module *m);
   int          references;
};

Eina_Bool            _elm_web_init(const char *engine);

Eo                   *_elm_atspi_bridge_get(void);
void                 _elm_atspi_bridge_init(void);
void                 _elm_atspi_bridge_shutdown(void);

Eina_Bool            _elm_prefs_init(void);
void                 _elm_prefs_shutdown(void);

/* these already issued by the two above, respectively */
void                 _elm_prefs_data_init(void);
void                 _elm_prefs_data_shutdown(void);

/* init functions for dnd and cnp */
Eo*                  _efl_ui_selection_manager_get(Eo *obj);
void                 _efl_ui_dnd_shutdown(void);

int                  _elm_ews_wm_init(void);
void                 _elm_ews_wm_shutdown(void);
void                 _elm_ews_wm_rescale(Elm_Theme *th,
                                         Eina_Bool use_theme);

void                 _elm_win_shutdown(void);
void                 _elm_win_rescale(Elm_Theme *th,
                                      Eina_Bool use_theme);
void                 _elm_win_access(Eina_Bool is_access);
void                 _elm_win_translate(void);
void                 _elm_win_focus_reconfigure(void);
void                 _elm_win_standard_init(Eo *win);

Ecore_X_Window       _elm_ee_xwin_get(const Ecore_Evas *ee);

Eina_Error      _elm_theme_object_set(Evas_Object *parent,
                                           Evas_Object *o,
                                           const char *clas,
                                           const char *group,
                                           const char *style);
Eina_Bool            _elm_theme_object_icon_set(Evas_Object *o,
                                                const char *group,
                                                const char *style);
Eina_Error      _elm_theme_set(Elm_Theme *th,
                                    Evas_Object *o,
                                    const char *clas,
                                    const char *group,
                                    const char *style,
                                    Eina_Bool is_legacy);
Eina_Bool            _elm_theme_icon_set(Elm_Theme *th,
                                         Evas_Object *o,
                                         const char *group,
                                         const char *style);
Eina_File           *_elm_theme_group_file_find(Elm_Theme *th,
                                                const char *group);
void                 _elm_theme_parse(Elm_Theme *th,
                                      const char *theme);
void                 _elm_theme_init(void);
void                 _elm_theme_shutdown(void);

void                 _elm_module_init(void);
void                 _elm_module_shutdown(void);
void                 _elm_module_parse(const char *s);
Elm_Module          *_elm_module_find_as(const char *as);
Elm_Module          *_elm_module_add(const char *name,
                                     const char *as);
void                 _elm_module_del(Elm_Module *m);
Eina_Bool            _elm_module_load(Elm_Module *m);
void                 _elm_module_unload(Elm_Module *m);
const void          *_elm_module_symbol_get(Elm_Module *m,
                                            const char *name);

void                 _elm_widget_focus_auto_show(Evas_Object *obj);
void                 _elm_widget_top_win_focused_set(Evas_Object *obj,
                                                     Eina_Bool top_win_focused);
Eina_Bool            _elm_widget_top_win_focused_get(const Evas_Object *obj);

void                 _elm_unneed_systray(void);
void                 _elm_unneed_sys_notify(void);
void                 _elm_unneed_ethumb(void);
void                 _elm_unneed_web(void);

void                 _elm_rescale(void);
Eina_Bool            _elm_old_clouseau_reload(void);

void                 _elm_config_init(void);
void                 _elm_config_sub_init(void);
void                 _elm_config_shutdown(void);
void                 _elm_config_sub_shutdown(void);
Eina_Bool            _elm_config_save(Elm_Config *cfg, const char *profile);
void                 _elm_config_reload(void);
size_t               _elm_config_user_dir_snprintf(char *dst, size_t size,
                                                   const char *fmt, ...)
                                                   EINA_PRINTF(3, 4);
void                 elm_color_class_init(void);
void                 elm_color_class_shutdown(void);
void                 _elm_recache(void);

const char          *_elm_config_current_profile_get(void);
const char          *_elm_config_profile_dir_get(const char *prof,
                                                 Eina_Bool is_user);
Eina_List           *_elm_config_profiles_list(Eina_Bool hide_profiles);
void                 _elm_config_all_update(void);
void                 _elm_config_profile_set(const char *profile);

void                 _elm_config_engine_set(const char *engine);

Eina_List           *_elm_config_font_overlays_list(void);
void                 _elm_config_font_overlay_set(const char *text_class,
                                                  const char *font,
                                                  Evas_Font_Size size);
void                 _elm_config_font_overlay_remove(const char *text_class);
void                 _elm_config_font_overlay_apply(void);
Eina_List           *_elm_config_text_classes_get(void);
void                 _elm_config_text_classes_free(Eina_List *l);

Eina_List           *_elm_config_color_classes_get(void);
void                 _elm_config_color_classes_free(Eina_List *l);
Eina_List           *_elm_config_color_overlays_list(void);
void                 _elm_config_color_overlay_set(const char *color_class,
                                                   int r, int g, int b, int a,
                                                   int r2, int g2, int b2, int a2,
                                                   int r3, int g3, int b3, int a3);
void                 _elm_config_color_overlay_remove(const char *color_class);
void                 _elm_config_color_overlay_apply(void);

Eina_Bool            _elm_config_access_get(void);
void                 _elm_config_access_set(Eina_Bool is_access);

Eina_Bool            _elm_config_key_binding_call(Evas_Object *obj,
                                                  const char *name,
                                                  const Evas_Event_Key_Down *ev,
                                                  const Elm_Action *actions);

Elm_Font_Properties *_elm_font_properties_get(Eina_Hash **font_hash,
                                              const char *font);
Eina_Hash           *_elm_font_available_hash_add(Eina_Hash *font_hash,
                                                  const char *full_name);
void                 _elm_font_available_hash_del(Eina_Hash *hash);

void                 elm_tooltip_theme(Elm_Tooltip *tt);
void                 elm_object_sub_tooltip_content_cb_set(Evas_Object *eventarea,
                                                           Evas_Object *owner,
                                                           Elm_Tooltip_Content_Cb func,
                                                           const void *data,
                                                           Evas_Smart_Cb del_cb);
void                 elm_cursor_theme(Elm_Cursor *cur);
void                 elm_object_sub_cursor_set(Evas_Object *eventarea,
                                               Evas_Object *owner,
                                               const char *cursor);
const char *         elm_object_sub_cursor_get(const Evas_Object *obj);
Eina_Bool            elm_object_sub_cursor_style_set(Evas_Object *obj,
                                                     const char *style);
const char *         elm_object_sub_cursor_style_get(const Evas_Object *obj);
Eina_Bool            elm_object_sub_cursor_theme_search_enabled_set(Evas_Object *obj,
                                                                    Eina_Bool theme_search);
Eina_Bool            elm_object_sub_cursor_theme_search_enabled_get(const Evas_Object *obj);

void                 elm_menu_clone(Evas_Object *from_menu,
                                    Evas_Object *to_menu,
                                    Elm_Object_Item *parent);

char                *_elm_util_mkup_to_text(const char *mkup);
char                *_elm_util_text_to_mkup(const char *text);

Eina_Bool            _elm_video_check(Evas_Object *video);

Eina_List           *_elm_config_color_list_get(const char *palette_name);
void                 _elm_config_color_set(const char *palette_name,
                                           int r,
                                           int g,
                                           int b,
                                           int a);
void                 _elm_config_colors_free(const char *palette_name);

typedef struct _Elm_DBus_Menu Elm_DBus_Menu;

const char          *_elm_dbus_menu_register(Eo *obj);
void                 _elm_dbus_menu_unregister(Eo *obj);
int                  _elm_dbus_menu_item_add(Elm_DBus_Menu *dbus_menu,
                                             Elm_Object_Item *item);
void                 _elm_dbus_menu_update(Elm_DBus_Menu *dbus_menu);
void                 _elm_dbus_menu_item_delete(Elm_DBus_Menu *dbus_menu,
                                                int id);

void                 _elm_dbus_menu_app_menu_register(Ecore_X_Window xid, Eo *obj,
                                                      void (*result_cb)(Eina_Bool, void *),
                                                      void *data);
void                 _elm_dbus_menu_app_menu_unregister(Eo *obj);
void                 _elm_dbus_menu_item_select_cb(Elm_Object_Item *obj_item);

void                 _elm_menu_menu_bar_set(Eo *obj, Eina_Bool menu_bar);
void                 _elm_menu_menu_bar_hide(Eo *obj);

# ifdef HAVE_ELEMENTARY_WL2
void                 _elm_win_wl_cursor_set(Evas_Object *obj, const char *cursor);
# endif

void _efl_ui_focus_manager_redirect_events_del(Efl_Ui_Focus_Manager *manager, Eo *obj);
void _efl_ui_focus_manager_redirect_events_add(Efl_Ui_Focus_Manager *manager, Eo *obj);

void _efl_access_shutdown(void);

/* Combobox: no proper support for Efl.Part API yet. */
void        _elm_combobox_part_text_set(Eo *obj, const char * part, const char *label);
const char *_elm_combobox_part_text_get(const Eo *obj, const char *part);

/* DEPRECATED, will be removed on next release */
void                 _elm_icon_signal_emit(Evas_Object *obj,
                                           const char *emission,
                                           const char *source);
void                 _elm_icon_signal_callback_add(Evas_Object *obj,
                                                   const char *emission,
                                                   const char *source,
                                                   Edje_Signal_Cb func_cb,
                                                   void *data);
void                *_elm_icon_signal_callback_del(Evas_Object *obj,
                                                   const char *emission,
                                                   const char *source,
                                                   Edje_Signal_Cb func_cb);
 void                _efl_ui_image_sizing_eval(Evas_Object *obj);
/* end of DEPRECATED */

/* DO NOT USE THIS this is only for performance optimization! */
void                 _elm_widget_full_eval(Eo *obj);
void                 _elm_widget_full_eval_children(Eo *obj, Elm_Widget_Smart_Data *pd);

EOAPI void			 efl_page_transition_page_size_set(Eo *obj, Eina_Size2D sz);
EOAPI void			 efl_page_transition_padding_size_set(Eo *obj, int padding);
EOAPI void			 efl_page_transition_update(Eo *obj, double pos);
EOAPI void			 efl_page_transition_pack(Eo *obj, int index);
EOAPI void			 efl_page_transition_unpack_all(Eo *obj);
EOAPI void			 efl_page_transition_curr_page_change(Eo *obj, int diff);
EOAPI Eina_Bool		 efl_page_transition_loop_set(Eo *obj, Efl_Ui_Pager_Loop loop);

EOAPI void			 efl_page_indicator_update(Eo *obj, double pos);
EOAPI void			 efl_page_indicator_pack(Eo *obj, int index);
EOAPI void			 efl_page_indicator_unpack(Eo *obj, int index);
EOAPI void			 efl_page_indicator_unpack_all(Eo *obj);

Eina_Bool _elm_config_accel_preference_parse(const char *pref, Eina_Stringshare **accel, int *gl_depth, int *gl_stencil, int *gl_msaa);

extern char *_elm_appname;
extern Elm_Config *_elm_config;
extern Efl_Config *_efl_config_obj;
extern const char *_elm_data_dir;
extern const char *_elm_lib_dir;
extern int _elm_log_dom;
extern Eina_List *_elm_win_list;
extern int _elm_win_deferred_free;
extern const char *_elm_preferred_engine;
extern const char *_elm_accel_preference;
extern const char SIG_WIDGET_FOCUSED[];
extern const char SIG_WIDGET_UNFOCUSED[];
extern const char SIG_WIDGET_LANG_CHANGED[];
extern const char SIG_WIDGET_ACCESS_CHANGED[];
extern const char SIG_LAYOUT_FOCUSED[];
extern const char SIG_LAYOUT_UNFOCUSED[];

extern Eina_Stringshare *_property_style_ss;

extern Eina_Bool _config_profile_lock;

extern Eina_FreeQ *postponed_fq;

# ifdef HAVE_ELEMENTARY_WL2
extern Ecore_Wl2_Display *_elm_wl_display;
# endif

# ifdef ENABLE_NLS
/* Our gettext wrapper, used to disable translation of elm if the app
 * is not translated. */
static inline const char *
_elm_dgettext(const char *string)
{
   if (EINA_UNLIKELY(_elm_config->translate == EINA_FALSE))
     {
        return string;
     }

   return dgettext(PACKAGE, string);
}

# endif

extern Eina_Bool _use_build_config;

/* Used by the paste handler */
void   _elm_entry_entry_paste(Evas_Object *obj, const char *entry);

double _elm_atof(const char *s);

// elm_layout and elm_entry LEGACY signal API (returned the user data pointer)
void _elm_layout_signal_callback_add_legacy(Eo *obj, Eo *edje, Eina_List **p_edje_signals, const char *emission, const char *source, Edje_Signal_Cb func, void *data);
void *_elm_layout_signal_callback_del_legacy(Eo *obj, Eo *edje, Eina_List **p_edje_signals, const char *emission, const char *source, Edje_Signal_Cb func);
void _elm_entry_signal_callback_add_legacy(Eo *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data);
void *_elm_entry_signal_callback_del_legacy(Eo *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb);

void efl_ui_win_inlined_parent_set(Eo *obj, Efl_Canvas_Object *parent);

/* Internal EO APIs */
const Elm_Layout_Part_Alias_Description *elm_layout_content_aliases_get(const Eo *obj);
const Elm_Layout_Part_Alias_Description *elm_layout_text_aliases_get(const Eo *obj);
void efl_ui_slider_val_fetch(Evas_Object *obj, Eina_Bool user_event);
void efl_ui_slider_val_set(Evas_Object *obj);
void efl_ui_slider_down_knob(Evas_Object *obj, double button_x, double button_y);
void efl_ui_slider_move_knob(Evas_Object *obj, double button_x, double button_y);
//void elm_layout_sizing_eval_eoapi(Eo *obj);

# define _ELM_LAYOUT_ALIASES_IMPLEMENT(_pfx, _typ) \
   static const Elm_Layout_Part_Alias_Description * \
   _##_pfx##_##_typ##_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED) \
   { \
      return _##_typ##_aliases; \
   }

# define _ELM_LAYOUT_ALIASES_OPS(_pfx, _typ) \
   EFL_OBJECT_OP_FUNC(elm_layout_##_typ##_aliases_get, _##_pfx##_##_typ##_aliases_get)

# define ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(_pfx) _ELM_LAYOUT_ALIASES_IMPLEMENT(_pfx, content)
# define ELM_LAYOUT_TEXT_ALIASES_IMPLEMENT(_pfx) _ELM_LAYOUT_ALIASES_IMPLEMENT(_pfx, text)

# define ELM_LAYOUT_CONTENT_ALIASES_OPS(_pfx) _ELM_LAYOUT_ALIASES_OPS(_pfx, content)
# define ELM_LAYOUT_TEXT_ALIASES_OPS(_pfx) _ELM_LAYOUT_ALIASES_OPS(_pfx, text)

# define ELM_LAYOUT_SIZING_EVAL_OPS(_pfx) \
   EFL_OBJECT_OP_FUNC(elm_layout_sizing_eval, _##_pfx##_elm_layout_sizing_eval)

# define ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(_pfx, _typ) \
EOLIAN static Eina_Bool \
_##_pfx##_efl_ui_widget_widget_input_event_handler(Eo *obj, _typ *_pd EINA_UNUSED, const Efl_Event *eo_event, Evas_Object *src EINA_UNUSED) \
{ \
   Evas_Event_Key_Down *ev; \
   if (eo_event->desc != EFL_EVENT_KEY_DOWN) return EINA_FALSE; \
   if (efl_input_processed_get(eo_event->info)) return EINA_FALSE; \
   if (elm_widget_disabled_get(obj)) return EINA_FALSE; \
   ev = efl_input_legacy_info_get(eo_event->info); \
   if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions)) return EINA_FALSE; \
   efl_input_processed_set(eo_event->info, EINA_TRUE); \
   return EINA_TRUE; \
}

static inline Efl_Ui_Layout_Orientation
efl_ui_layout_orientation_axis_get(Efl_Ui_Layout_Orientation orient)
{
   return orient & EFL_UI_LAYOUT_ORIENTATION_AXIS_BITMASK;
}

static inline Eina_Bool
efl_ui_layout_orientation_is_inverted(Efl_Ui_Layout_Orientation orient)
{
   return orient & EFL_UI_LAYOUT_ORIENTATION_INVERTED;
}

static inline Eina_Bool
efl_ui_layout_orientation_is_horizontal(Efl_Ui_Layout_Orientation orient, Eina_Bool def_val)
{
   switch (orient & EFL_UI_LAYOUT_ORIENTATION_AXIS_BITMASK)
     {
      case EFL_UI_LAYOUT_ORIENTATION_DEFAULT: return !!def_val;
      case EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL: return EINA_TRUE;
      case EFL_UI_LAYOUT_ORIENTATION_VERTICAL: return EINA_FALSE;
      default: return !!def_val;
     }
}

/**
 * This function sets up handlers for emitting "unfocused" / "focused" events.
 * Once the passed manager object is active, "focused" is emitted, if it gets inactive "unfocused" is emitted
 */
void legacy_efl_ui_focus_manager_widget_legacy_signals(Efl_Ui_Focus_Manager *manager, Efl_Ui_Focus_Manager *emittee);

/**
 * This function sets up handlers for emitting "unfocused" / "focused" events.
 * Once the children of the passed object are getting focus, "focused" will be emitted on the object, "unfocused" otherwise.
 */
void legacy_child_focus_handle(Efl_Ui_Focus_Object *object);

/**
 * This function sets up handlers for emitting "unfocused" / "focused" events.
 * Once the passed object is getting focus, "focused" will be emitted on the object, "unfocused" otherwise.
 */
void legacy_object_focus_handle(Efl_Ui_Focus_Object *object);

void _efl_ui_focus_event_redirector(Efl_Ui_Focus_Object *obj, Efl_Ui_Focus_Object *goal);

/**
 * With this flag you can indicate that this widget is used internally.
 * Indicating that a widget is internal, can be used by the implementing widget, that the parent property of the added sub-object should not be adjusted or altered.
 * There is no direct promise that any widget behaves like the above, every case should be handchecked.
 */
void efl_ui_widget_internal_set(Eo *obj, Eina_Bool internal);
Eina_Bool efl_ui_widget_internal_get(Eo *obj);


#endif
