#ifndef ELM_PRIV_H
#define ELM_PRIV_H
#ifdef HAVE_ELEMENTARY_X
#include <Ecore_X.h>
#endif
#ifdef HAVE_ELEMENTARY_FB
#include <Ecore_Fb.h>
#endif
#ifdef HAVE_ELEMENTARY_WL2
#include <Ecore_Wl2.h>
#endif
#ifdef HAVE_ELEMENTARY_DRM
#include <Ecore_Drm2.h>
#endif
#ifdef HAVE_ELEMENTARY_COCOA
# include <Ecore_Cocoa.h>
#endif
#ifdef HAVE_ELEMENTARY_WIN32
#include <Ecore_Win32.h>
#endif

#include <Eio.h>

// Evas internal EO APIs
#include "Evas_Internal.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef ELEMENTARY_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#include "elm_widget.h"
#include "elm_access.eo.h"
#include "elm_code_private.h"

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#define CRI(...)      EINA_LOG_DOM_CRIT(_elm_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_elm_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_elm_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_elm_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_elm_log_dom, __VA_ARGS__)

#ifdef ENABLE_NLS
# include <libintl.h>
# define E_(string)    _elm_dgettext(string)
#else
# ifndef setlocale
#  define setlocale(c, l)
# endif
# ifndef libintl_setlocale
#  define libintl_setlocale(c, l)
# endif
# ifndef bindtextdomain
#  define bindtextdomain(domain, dir)
# endif
# ifndef libintl_bindtextdomain
#  define libintl_bindtextdomain(domain, dir)
# endif
# define E_(string) (string)
#endif
#define N_(string) (string)

typedef struct _Elm_Theme_Files          Elm_Theme_Files;
typedef struct _Edje_Signal_Data         Edje_Signal_Data;
typedef struct _Elm_Config               Elm_Config;
typedef struct _Elm_Config_Bindings_Widget   Elm_Config_Bindings_Widget;
typedef struct _Elm_Config_Binding_Key   Elm_Config_Binding_Key;
typedef struct _Elm_Config_Binding_Modifier  Elm_Config_Binding_Modifier;
typedef struct _Elm_Module               Elm_Module;

struct _Edje_Signal_Data
{
   Evas_Object   *obj;
   Edje_Signal_Cb func;
   const char    *emission;
   const char    *source;
   void          *data;
};

struct _Elm_Theme_Files
{
   /*
    * We are conserving a list of path even if that's duplicated
    * because we expose those directly to the outside world :'(
    */
   Eina_List *items;
   Eina_List *handles;
};

struct _Elm_Theme
{
   Elm_Theme_Files overlay;
   Elm_Theme_Files themes;
   Elm_Theme_Files extension;

   Eina_Hash  *cache;
   Eina_Hash  *cache_data;
   Elm_Theme  *ref_theme;
   Eina_List  *referrers;
   const char *theme;
   int         ref;
   Eina_Hash  *cache_style_load_failed;
};

/* increment this whenever we change config enough that you need new
 * defaults for elm to work.
 */
#define ELM_CONFIG_EPOCH           0x0002
/* increment this whenever a new set of config values are added but
 * the users config doesn't need to be wiped - simply new values need
 * to be put in
 */
#define ELM_CONFIG_FILE_GENERATION 0x000e
#define ELM_CONFIG_VERSION_EPOCH_OFFSET 16
#define ELM_CONFIG_VERSION         ((ELM_CONFIG_EPOCH << ELM_CONFIG_VERSION_EPOCH_OFFSET) | \
                                    ELM_CONFIG_FILE_GENERATION)
/* NB: profile configuration files (.src) must have their
 * "config_version" entry's value up-to-date with ELM_CONFIG_VERSION
 * (in decimal)!! */

/* note: always remember to sync it with elm_config.c */
extern const char *_elm_engines[];

#define ELM_SOFTWARE_X11      (_elm_engines[0])
#define ELM_SOFTWARE_FB       (_elm_engines[1])
#define ELM_OPENGL_X11        (_elm_engines[2])
#define ELM_SOFTWARE_WIN32    (_elm_engines[3])
#define ELM_SOFTWARE_SDL      (_elm_engines[4])
#define ELM_OPENGL_SDL        (_elm_engines[5])
#define ELM_BUFFER            (_elm_engines[6])
#define ELM_EWS               (_elm_engines[7])
#define ELM_OPENGL_COCOA      (_elm_engines[8])
#define ELM_SOFTWARE_PSL1GHT  (_elm_engines[9])
#define ELM_WAYLAND_SHM       (_elm_engines[10])
#define ELM_WAYLAND_EGL       (_elm_engines[11])
#define ELM_DRM               (_elm_engines[12])
#define ELM_SOFTWARE_DDRAW    (_elm_engines[13])

#define ELM_FONT_TOKEN_STYLE  ":style="

#define ELM_ACCESS_MODE_OFF   EINA_FALSE
#define ELM_ACCESS_MODE_ON    EINA_TRUE

#define ELM_ATSPI_MODE_OFF   EINA_FALSE
#define ELM_ATSPI_MODE_ON    EINA_TRUE

/* convenience macro to compress code and avoid typos */
#undef CEIL
#define CEIL(a)   (((a) % 2 != 0) ? ((a) / 2 + 1) : ((a) / 2))
#undef IS_INSIDE
#define IS_INSIDE(x, y, xx, yy, ww, hh) \
  (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && \
  ((x) >= (xx)) && ((y) >= (yy)))


#define ELM_SAFE_FREE(_h, _fn) do { _fn((void*)_h); _h = NULL; } while (0)
#define ELM_SAFE_DEL(_h) do { efl_del(_h); _h = NULL; } while (0)

#define ELM_PRIV_STATIC_VARIABLE_DECLARE(name, signal, type) \
   static const char name[] = signal;

#define ELM_PRIV_SMART_CALLBACKS_DESC(name, signal, type) \
   {name, type},

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
   int           thumbscroll_flick_distance_tolerance;
   double        thumbscroll_friction;
   double        thumbscroll_min_friction;
   double        thumbscroll_friction_standard;
   double        thumbscroll_bounce_friction;
   double        thumbscroll_acceleration_threshold;
   double        thumbscroll_acceleration_time_limit;
   double        thumbscroll_acceleration_weight;
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
   Eina_List    *font_dirs;
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

   /* Not part of the EET file */
   Eina_Bool     is_mirrored : 1;
   Eina_Bool     translate : 1;
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

void                 _elm_emotion_init(void);
void                 _elm_emotion_shutdown(void);

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

Elm_Theme_Apply      _elm_theme_object_set(Evas_Object *parent,
                                           Evas_Object *o,
                                           const char *clas,
                                           const char *group,
                                           const char *style);
Eina_Bool            _elm_theme_object_icon_set(Evas_Object *o,
                                                const char *group,
                                                const char *style);
Elm_Theme_Apply      _elm_theme_set(Elm_Theme *th,
                                    Evas_Object *o,
                                    const char *clas,
                                    const char *group,
                                    const char *style);
Eina_Bool            _elm_theme_icon_set(Elm_Theme *th,
                                         Evas_Object *o,
                                         const char *group,
                                         const char *style);
Eina_File           *_elm_theme_group_file_find(Elm_Theme *th,
                                                const char *group);
void                 _elm_theme_parse(Elm_Theme *th,
                                      const char *theme);
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
Eina_Bool            _elm_clouseau_reload(void);

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

#ifdef HAVE_ELEMENTARY_WL2
void                 _elm_win_wl_cursor_set(Evas_Object *obj, const char *cursor);
#endif

void _efl_ui_focus_manager_redirect_events_del(Efl_Ui_Focus_Manager *manager, Eo *obj);
void _efl_ui_focus_manager_redirect_events_add(Efl_Ui_Focus_Manager *manager, Eo *obj);

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

extern Eina_Bool _config_profile_lock;

#ifdef HAVE_ELEMENTARY_WL2
extern Ecore_Wl2_Display *_elm_wl_display;
#endif

#ifdef ENABLE_NLS
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

#endif

/* Used by the paste handler */
void   _elm_entry_entry_paste(Evas_Object *obj, const char *entry);

double _elm_atof(const char *s);

// elm_layout and elm_entry LEGACY signal API (returned the user data pointer)
void _elm_layout_signal_callback_add_legacy(Eo *obj, Eo *edje, Eina_List **p_edje_signals, const char *emission, const char *source, Edje_Signal_Cb func, void *data);
void *_elm_layout_signal_callback_del_legacy(Eo *obj, Eo *edje, Eina_List **p_edje_signals, const char *emission, const char *source, Edje_Signal_Cb func);
void _elm_entry_signal_callback_add_legacy(Eo *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data);
void *_elm_entry_signal_callback_del_legacy(Eo *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb);


/* Internal EO APIs */
const Elm_Layout_Part_Alias_Description *elm_layout_content_aliases_get(const Eo *obj);
const Elm_Layout_Part_Alias_Description *elm_layout_text_aliases_get(const Eo *obj);

#define _ELM_LAYOUT_ALIASES_IMPLEMENT(_pfx, _typ) \
   static const Elm_Layout_Part_Alias_Description * \
   _##_pfx##_##_typ##_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED) \
   { \
      return _##_typ##_aliases; \
   }

#define _ELM_LAYOUT_ALIASES_OPS(_pfx, _typ) \
   EFL_OBJECT_OP_FUNC(elm_layout_##_typ##_aliases_get, _##_pfx##_##_typ##_aliases_get)

#define ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT() _ELM_LAYOUT_ALIASES_IMPLEMENT(MY_CLASS_PFX, content)
#define ELM_LAYOUT_TEXT_ALIASES_IMPLEMENT()  _ELM_LAYOUT_ALIASES_IMPLEMENT(MY_CLASS_PFX, text)

#define ELM_LAYOUT_CONTENT_ALIASES_OPS() _ELM_LAYOUT_ALIASES_OPS(MY_CLASS_PFX, content)
#define ELM_LAYOUT_TEXT_ALIASES_OPS() _ELM_LAYOUT_ALIASES_OPS(MY_CLASS_PFX, text)

#endif
