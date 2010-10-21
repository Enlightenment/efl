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

typedef struct _Elm_Config    Elm_Config;
typedef struct _Elm_Module    Elm_Module;

struct _Elm_Theme
{
   Eina_List  *overlay;
   Eina_List  *themes;
   Eina_List  *extension;
   Eina_Hash  *cache;
   const char *theme;
   int         ref;
};

typedef enum _Elm_Engine
{
   ELM_SOFTWARE_X11,
   ELM_SOFTWARE_FB,
   ELM_SOFTWARE_DIRECTFB,
   ELM_SOFTWARE_16_X11,
   ELM_SOFTWARE_8_X11,
   ELM_XRENDER_X11,
   ELM_OPENGL_X11,
   ELM_SOFTWARE_WIN32,
   ELM_SOFTWARE_16_WINCE,
   ELM_SOFTWARE_SDL,
   ELM_SOFTWARE_16_SDL,
   ELM_OPENGL_SDL
} Elm_Engine;



/* increment this whenever we change config enough that you need new
 * defaults for elm to work.
 */
#define ELM_CONFIG_EPOCH           0x0001
/* increment this whenever a new set of config values are added but the users
 * config doesn't need to be wiped - simply new values need to be put in
 */
#define ELM_CONFIG_FILE_GENERATION 0x0001
#define ELM_CONFIG_VERSION         ((ELM_CONFIG_EPOCH << 16) | ELM_CONFIG_FILE_GENERATION)

struct _Elm_Config
{
   int          config_version;
   int          engine;
   Eina_Bool    thumbscroll_enable;
   int          thumbscroll_threshhold;
   double       thumbscroll_momentum_threshhold;
   double       thumbscroll_friction;
   double       thumbscroll_bounce_friction;
   double       page_scroll_friction;
   double       bring_in_scroll_friction;
   double       zoom_friction;
   Eina_Bool    thumbscroll_bounce_enable;
   double       scale;
   int          bgpixmap;
   int          compositing;
   Eina_List   *font_dirs;
   int          font_hinting;
   int          image_cache;
   int          font_cache;
   int          finger_size;
   double       fps;
   const char  *theme;
   const char  *modules;
   double       tooltip_delay;
   Eina_Bool    cursor_engine_only;
   Eina_Bool    focus_highlight_enable;
   Eina_Bool    focus_highlight_animate;
   int          toolbar_shrink_mode;
   Eina_Bool    fileselector_expand_enable;
   Eina_Bool    inwin_dialogs_enable;
   int          icon_size;
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
void                _elm_win_rescale(void);

int                 _elm_theme_object_set(Evas_Object *parent, Evas_Object *o, const char *clas, const char *group, const char *style);
int                 _elm_theme_object_icon_set(Evas_Object *parent, Evas_Object *o, const char *group, const char *style);
int                 _elm_theme_set(Elm_Theme *th, Evas_Object *o, const char *clas, const char *group, const char *style);
int                 _elm_theme_icon_set(Elm_Theme *th, Evas_Object *o, const char *group, const char *style);
int                 _elm_theme_parse(Elm_Theme *th, const char *theme);
void                _elm_theme_shutdown(void);

void                _elm_module_init(void);
void                _elm_module_shutdown(void);
void                _elm_module_parse(const char *s);
Elm_Module         *_elm_module_find_as(const char *as);
Elm_Module         *_elm_module_add(const char *name, const char *as);
void                _elm_module_del(Elm_Module *m);
const void         *_elm_module_symbol_get(Elm_Module *m, const char *name);

void                _elm_widget_type_clear(void);

void		    _elm_unneed_ethumb(void);

void                _elm_rescale(void);

void                _elm_config_init(void);
void                _elm_config_sub_init(void);
void                _elm_config_shutdown(void);

void                 elm_tooltip_theme(Elm_Tooltip *tt);
void                 elm_object_sub_tooltip_content_cb_set(Evas_Object *eventarea, Evas_Object *owner, Elm_Tooltip_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);
void                 elm_cursor_theme(Elm_Cursor *cur);
void                 elm_object_sub_cursor_set(Evas_Object *eventarea, Evas_Object *owner, const char *cursor);

void                 elm_menu_clone(Evas_Object *from_menu, Evas_Object *to_menu, Elm_Menu_Item *parent);

Eina_Bool           _elm_dangerous_call_check(const char *call);

Evas_Object        *_elm_scroller_edje_object_get(Evas_Object *obj);

extern char        *_elm_appname;
extern Elm_Config  *_elm_config;
extern const char  *_elm_data_dir;
extern const char  *_elm_lib_dir;
extern int          _elm_log_dom;
extern Eina_List   *_elm_win_list;

#endif
