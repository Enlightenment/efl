#ifndef ELM_PRIV_H
#define ELM_PRIV_H
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifdef HAVE_ELEMENTARY_X
# include <Ecore_X.h>
#endif
#ifdef HAVE_ELEMENTARY_FB
# include <Ecore_Fb.h>
#endif
#ifdef HAVE_ELEMENTARY_WINCE
# include <Ecore_WinCE.h>
#endif

#include "els_pan.h"
#include "els_scroller.h"
#include "els_box.h"
#include "els_icon.h"

#include "elm_widget.h"

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_elm_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR (_elm_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_elm_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_elm_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG (_elm_log_dom, __VA_ARGS__)

#ifdef ENABLE_NLS
# include <libintl.h>
# define E_(string) _elm_dgettext(string)
#else
# define bindtextdomain(domain,dir)
# define E_(string) (string)
#endif


typedef struct _Elm_Config    Elm_Config;
typedef struct _Elm_Module    Elm_Module;

struct _Elm_Theme
{
   Eina_List  *overlay;
   Eina_List  *themes;
   Eina_List  *extension;
   Eina_Hash  *cache;
   Eina_Hash  *cache_data;
   Elm_Theme  *ref_theme;
   Eina_List  *referrers;
   const char *theme;
   int         ref;
};

/* increment this whenever we change config enough that you need new
 * defaults for elm to work.
 */
#define ELM_CONFIG_EPOCH           0x0001
/* increment this whenever a new set of config values are added but the users
 * config doesn't need to be wiped - simply new values need to be put in
 */
#define ELM_CONFIG_FILE_GENERATION 0x0003
#define ELM_CONFIG_VERSION         ((ELM_CONFIG_EPOCH << 16) | ELM_CONFIG_FILE_GENERATION)
/* NB: profile configuration files (.src) must have their
 * "config_version" entry's value up-to-date with ELM_CONFIG_VERSION
 * (in decimal)!! */

/* note: always remember to sync it with elm_config.c */
extern const char *_elm_engines[];

#define ELM_SOFTWARE_X11      (_elm_engines[0])
#define ELM_SOFTWARE_FB       (_elm_engines[1])
#define ELM_SOFTWARE_DIRECTFB (_elm_engines[2])
#define ELM_SOFTWARE_16_X11   (_elm_engines[3])
#define ELM_SOFTWARE_8_X11    (_elm_engines[4])
#define ELM_XRENDER_X11       (_elm_engines[5])
#define ELM_OPENGL_X11        (_elm_engines[6])
#define ELM_SOFTWARE_WIN32    (_elm_engines[7])
#define ELM_SOFTWARE_16_WINCE (_elm_engines[8])
#define ELM_SOFTWARE_SDL      (_elm_engines[9])
#define ELM_SOFTWARE_16_SDL   (_elm_engines[10])
#define ELM_OPENGL_SDL        (_elm_engines[11])
#define ELM_BUFFER            (_elm_engines[12])

#define ELM_FONT_TOKEN_STYLE ":style="

#define ELM_ACCESS_MODE_OFF 0
#define ELM_ACCESS_MODE_ON 1

struct _Elm_Config
{
   int            config_version;
   const char    *engine;
   unsigned char  vsync;
   unsigned char  thumbscroll_enable;
   int            thumbscroll_threshold;
   double         thumbscroll_momentum_threshold;
   double         thumbscroll_friction;
   double         thumbscroll_bounce_friction;
   double         page_scroll_friction;
   double         bring_in_scroll_friction;
   double         zoom_friction;
   unsigned char  thumbscroll_bounce_enable;
   double         thumbscroll_border_friction;
   double         scroll_smooth_amount;
   double         scroll_smooth_history_weight;
   double         scroll_smooth_future_time;
   double         scroll_smooth_time_window;
   double         scale;
   int            bgpixmap;
   int            compositing;
   Eina_List     *font_dirs;
   Eina_List     *font_overlays;
   int            font_hinting;
   int            cache_flush_poll_interval;
   unsigned char  cache_flush_enable;
   int            image_cache;
   int            font_cache;
   int            edje_cache;
   int            edje_collection_cache;
   int            finger_size;
   double         fps;
   const char    *theme;
   const char    *modules;
   double         tooltip_delay;
   unsigned char  cursor_engine_only;
   unsigned char  focus_highlight_enable;
   unsigned char  focus_highlight_animate;
   int            toolbar_shrink_mode;
   unsigned char  fileselector_expand_enable;
   unsigned char  inwin_dialogs_enable;
   int            icon_size;
   double         longpress_timeout;
   unsigned char  effect_enable;
   unsigned char  desktop_entry;
   Eina_Bool      password_show_last;
   double         password_show_last_timeout;
   double         glayer_zoom_finger_factor;
   double         glayer_zoom_wheel_factor;
   double         glayer_zoom_distance_tolerance;
   double         glayer_rotate_angular_tolerance;
   double         glayer_line_min_length;
   double         glayer_line_distance_tolerance;
   double         glayer_line_angular_tolerance;
   unsigned int   glayer_flick_time_limit_ms;
   double         glayer_long_tap_start_timeout;
   int            access_mode;
   Eina_Bool      glayer_continues_enable;

   /* Not part of the EET file */
   Eina_Bool      is_mirrored : 1;
   Eina_Bool      translate : 1;
};

struct _Elm_Module
{
   int          version;
   const char  *name;
   const char  *as;
   const char  *so_path;
   const char  *data_dir;
   const char  *bin_dir;
   void        *handle;
   void        *data;
   void        *api;
   int        (*init_func) (Elm_Module *m);
   int        (*shutdown_func) (Elm_Module *m);
   int          references;
};

void                _elm_win_shutdown(void);
void                _elm_win_rescale(Elm_Theme *th, Eina_Bool use_theme);

Eina_Bool           _elm_theme_object_set(Evas_Object *parent, Evas_Object *o, const char *clas, const char *group, const char *style);
Eina_Bool           _elm_theme_object_icon_set(Evas_Object *parent, Evas_Object *o, const char *group, const char *style);
Eina_Bool           _elm_theme_set(Elm_Theme *th, Evas_Object *o, const char *clas, const char *group, const char *style);
Eina_Bool           _elm_theme_icon_set(Elm_Theme *th, Evas_Object *o, const char *group, const char *style);
Eina_Bool           _elm_theme_parse(Elm_Theme *th, const char *theme);
void                _elm_theme_shutdown(void);

void                _elm_module_init(void);
void                _elm_module_shutdown(void);
void                _elm_module_parse(const char *s);
Elm_Module         *_elm_module_find_as(const char *as);
Elm_Module         *_elm_module_add(const char *name, const char *as);
void                _elm_module_del(Elm_Module *m);
Eina_Bool           _elm_module_load(Elm_Module *m);
void                _elm_module_unload(Elm_Module *m);
const void         *_elm_module_symbol_get(Elm_Module *m, const char *name);

void                _elm_widget_type_clear(void);
void                _elm_widget_focus_region_show(const Evas_Object *obj);
void                _elm_widget_top_win_focused_set(Evas_Object *obj, Eina_Bool top_win_focused);
Eina_Bool           _elm_widget_top_win_focused_get(const Evas_Object *obj);

void                _elm_unneed_ethumb(void);

void                _elm_rescale(void);
void                _elm_widget_mirrored_reload(Evas_Object *obj);

void                _elm_config_init(void);
void                _elm_config_sub_init(void);
void                _elm_config_shutdown(void);
Eina_Bool           _elm_config_save(void);
void                _elm_config_reload(void);

void                _elm_recache(void);

const char         *_elm_config_current_profile_get(void);
const char         *_elm_config_profile_dir_get(const char *prof, Eina_Bool is_user);
Eina_List          *_elm_config_profiles_list(void);
void                _elm_config_profile_set(const char *profile);

void                _elm_config_engine_set(const char *engine);

Eina_List          *_elm_config_font_overlays_list(void);
void                _elm_config_font_overlay_set(const char *text_class, const char *font, Evas_Font_Size size);
void                _elm_config_font_overlay_remove(const char *text_class);
void                _elm_config_font_overlay_apply(void);
Eina_List          *_elm_config_text_classes_get(void);
void                _elm_config_text_classes_free(Eina_List *l);

Elm_Font_Properties *_elm_font_properties_get(Eina_Hash **font_hash, const char *font);
Eina_Hash           *_elm_font_available_hash_add(Eina_Hash *font_hash, const char *full_name);
void                 _elm_font_available_hash_del(Eina_Hash *hash);

void                 elm_tooltip_theme(Elm_Tooltip *tt);
void                 elm_object_sub_tooltip_content_cb_set(Evas_Object *eventarea, Evas_Object *owner, Elm_Tooltip_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);
void                 elm_cursor_theme(Elm_Cursor *cur);
void                 elm_object_sub_cursor_set(Evas_Object *eventarea, Evas_Object *owner, const char *cursor);

void                 elm_menu_clone(Evas_Object *from_menu, Evas_Object *to_menu, Elm_Menu_Item *parent);

Eina_Bool           _elm_dangerous_call_check(const char *call);

Evas_Object        *_elm_scroller_edje_object_get(Evas_Object *obj);

char               *_elm_util_mkup_to_text(const char *mkup);
char               *_elm_util_text_to_mkup(const char *text);

Eina_Bool           _elm_video_check(Evas_Object *video);


extern char        *_elm_appname;
extern Elm_Config  *_elm_config;
extern const char  *_elm_data_dir;
extern const char  *_elm_lib_dir;
extern int          _elm_log_dom;
extern Eina_List   *_elm_win_list;
extern int          _elm_win_deferred_free;

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


/* Used by the paste handler */
void _elm_entry_entry_paste(Evas_Object *obj, const char *entry);

#endif
