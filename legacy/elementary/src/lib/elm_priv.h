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

// FIXME: totally disorganised. clean this up!
// 
// Why EAPI in a private header ?
// EAPI is temporaty - that widget api will change, but makign it EAPI right now to indicate its bound for externalness

struct _Elm_Theme
{
   Eina_List *overlay;
   Eina_List *themes;
   Eina_List *extension;
   Eina_Hash *cache;
   const char *theme;
   int ref;
};

typedef enum _Elm_Engine
{
   ELM_SOFTWARE_X11,
     ELM_SOFTWARE_FB,
     ELM_SOFTWARE_DIRECTFB,
     ELM_SOFTWARE_16_X11,
     ELM_XRENDER_X11,
     ELM_OPENGL_X11,
     ELM_SOFTWARE_WIN32,
     ELM_SOFTWARE_16_WINCE,
     ELM_SOFTWARE_SDL,
     ELM_SOFTWARE_16_SDL,
     ELM_OPENGL_SDL
} Elm_Engine;

typedef struct _Elm_Config Elm_Config;

struct _Elm_Config
{
   int engine;
   int thumbscroll_enable;
   int thumbscroll_threshhold;
   double thumbscroll_momentum_threshhold;
   double thumbscroll_friction;
   double thumbscroll_bounce_friction;
   double page_scroll_friction;
   double bring_in_scroll_friction;
   double zoom_friction;
   int thumbscroll_bounce_enable;
   double scale;
   int bgpixmap;
   int compositing;
   Eina_List *font_dirs;
   int font_hinting;
   int image_cache;
   int font_cache;
   int finger_size;
   double fps;
   const char *theme;
   const char *modules;
};

typedef struct _Elm_Module Elm_Module;

struct _Elm_Module
{
   int version;
   const char *name;
   const char *as;
   const char *so_path;
   const char *data_dir;
   const char *bin_dir;
   void *handle;
   void *data;
   void *api;
   int (*init_func) (Elm_Module *m);
   int (*shutdown_func) (Elm_Module *m);
   int references;
};


enum _elm_sel_type {
   ELM_SEL_PRIMARY,
   ELM_SEL_SECONDARY,
   ELM_SEL_CLIPBOARD,

   ELM_SEL_MAX,
};

enum _elm_sel_format {
   ELM_SEL_MARKUP	= 0x01,
   ELM_SEL_IMAGE	= 0x02,
};



#define ELM_NEW(t) calloc(1, sizeof(t))

void _elm_win_shutdown(void);
void _elm_win_rescale(void);

int _elm_theme_object_set(Evas_Object *parent, Evas_Object *o, const char *clas, const char *group, const char *style);
int _elm_theme_object_icon_set(Evas_Object *parent, Evas_Object *o, const char *group, const char *style);
int _elm_theme_set(Elm_Theme *th, Evas_Object *o, const char *clas, const char *group, const char *style);
int _elm_theme_icon_set(Elm_Theme *th, Evas_Object *o, const char *group, const char *style);
int _elm_theme_parse(Elm_Theme *th, const char *theme);
void _elm_theme_shutdown(void);

void _elm_module_init(void);
void _elm_module_shutdown(void);
void _elm_module_parse(const char *s);
Elm_Module *_elm_module_find_as(const char *as);
Elm_Module *_elm_module_add(const char *name, const char *as);
void _elm_module_del(Elm_Module *m);
const void *_elm_module_symbol_get(Elm_Module *m, const char *name);
    
/* FIXME: should this be public? for now - private (but public symbols) */
EAPI Evas_Object *elm_widget_add(Evas *evas);
EAPI void         elm_widget_del_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_del_pre_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_focus_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_activate_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_disable_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_theme_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_changed_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_signal_emit_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj, const char *emission, const char *source));
EAPI void         elm_widget_signal_callback_add_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj, const char *emission, const char *source, void (*func_cb) (void *data, Evas_Object *o, const char *emission, const char *source), void *data));
EAPI void         elm_widget_signal_callback_del_hook_set(Evas_Object *obj, void *(*func) (Evas_Object *obj, const char *emission, const char *source, void (*func_cb) (void *data, Evas_Object *o, const char *emission, const char *source)));
EAPI void         elm_widget_theme(Evas_Object *obj);
EAPI void         elm_widget_on_focus_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data);
EAPI void         elm_widget_on_change_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data);
EAPI void         elm_widget_on_show_region_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data);
EAPI void         elm_widget_data_set(Evas_Object *obj, void *data);
EAPI void        *elm_widget_data_get(const Evas_Object *obj);
EAPI void         elm_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_sub_object_del(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_hover_object_set(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_signal_emit(Evas_Object *obj, const char *emission, const char *source);
EAPI void         elm_widget_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data);
EAPI void         *elm_widget_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source));
EAPI void         elm_widget_can_focus_set(Evas_Object *obj, int can_focus);
EAPI int          elm_widget_can_focus_get(const Evas_Object *obj);
EAPI int          elm_widget_focus_get(const Evas_Object *obj);
EAPI Evas_Object *elm_widget_focused_object_get(const Evas_Object *obj);
EAPI Evas_Object *elm_widget_top_get(const Evas_Object *obj);
EAPI int          elm_widget_focus_jump(Evas_Object *obj, int forward);
EAPI void         elm_widget_focus_set(Evas_Object *obj, int first);
EAPI void         elm_widget_focused_object_clear(Evas_Object *obj);
EAPI Evas_Object *elm_widget_parent_get(const Evas_Object *obj);
EAPI void         elm_widget_focus_steal(Evas_Object *obj);
EAPI void         elm_widget_activate(Evas_Object *obj);
EAPI void         elm_widget_change(Evas_Object *obj);
EAPI void         elm_widget_disabled_set(Evas_Object *obj, int disabled);
EAPI int          elm_widget_disabled_get(const Evas_Object *obj);
EAPI void         elm_widget_show_region_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
EAPI void         elm_widget_show_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EAPI void         elm_widget_scroll_hold_push(Evas_Object *obj);
EAPI void         elm_widget_scroll_hold_pop(Evas_Object *obj);
EAPI int          elm_widget_scroll_hold_get(const Evas_Object *obj);
EAPI void         elm_widget_scroll_freeze_push(Evas_Object *obj);
EAPI void         elm_widget_scroll_freeze_pop(Evas_Object *obj);
EAPI int          elm_widget_scroll_freeze_get(const Evas_Object *obj);
EAPI void         elm_widget_scale_set(Evas_Object *obj, double scale);
EAPI double       elm_widget_scale_get(const Evas_Object *obj);
EAPI void         elm_widget_theme_set(Evas_Object *obj, Elm_Theme *th);
EAPI Elm_Theme   *elm_widget_theme_get(const Evas_Object *obj);
EAPI void         elm_widget_style_set(Evas_Object *obj, const char *style);
EAPI const char  *elm_widget_style_get(const Evas_Object *obj);
EAPI void         elm_widget_type_set(Evas_Object *obj, const char *type);
EAPI const char  *elm_widget_type_get(const Evas_Object *obj);
EAPI void         elm_widget_drag_lock_x_set(Evas_Object *obj, Eina_Bool lock);
EAPI void         elm_widget_drag_lock_y_set(Evas_Object *obj, Eina_Bool lock);
EAPI Eina_Bool    elm_widget_drag_lock_x_get(const Evas_Object *obj);
EAPI Eina_Bool    elm_widget_drag_lock_y_get(const Evas_Object *obj);
EAPI int          elm_widget_drag_child_locked_x_get(const Evas_Object *obj);
EAPI int          elm_widget_drag_child_locked_y_get(const Evas_Object *obj);
    
EAPI Eina_Bool    elm_widget_is(const Evas_Object *obj);
EAPI Evas_Object *elm_widget_parent_widget_get(const Evas_Object *obj);

EAPI Eina_List   *_elm_stringlist_get(const char *str);
EAPI void         _elm_stringlist_free(Eina_List *list);

Eina_Bool         _elm_widget_type_check(const Evas_Object *obj, const char *type);

void		  _elm_unneed_ethumb(void);

void              _elm_rescale(void);

void              _elm_config_init(void);
void              _elm_config_sub_init(void);
void              _elm_config_shutdown(void);

Eina_Bool	  elm_selection_set(enum _elm_sel_type selection, Evas_Object *widget, enum _elm_sel_format format, const char *buf);
Eina_Bool	  elm_selection_clear(enum _elm_sel_type selection, Evas_Object *widget);
Eina_Bool	  elm_selection_get(enum _elm_sel_type selection, enum _elm_sel_format format, Evas_Object *widget);

Eina_Bool         _elm_dangerous_call_check(const char *call);

void              _elm_widtype_register(const char **ptr);


#define ELM_SET_WIDTYPE(widtype, type) \
   do { \
      if (!widtype) { \
         widtype = eina_stringshare_add(type); \
         _elm_widtype_register(&widtype); \
      } \
   } while (0)

//#define ELM_CHECK_WIDTYPE(obj, widtype) if (elm_widget_type_get(obj) != widtype) return
#define ELM_CHECK_WIDTYPE(obj, widtype) if (!_elm_widget_type_check((obj), (widtype))) return

extern char *_elm_appname;
extern Elm_Config *_elm_config;
extern const char *_elm_data_dir;
extern const char *_elm_lib_dir;
extern int _elm_log_dom;

extern Eina_List *_elm_win_list;

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_elm_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_elm_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_elm_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_elm_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_elm_log_dom, __VA_ARGS__)

#endif
