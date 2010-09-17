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
typedef struct _Elm_Drop_Data Elm_Drop_Data;

typedef Eina_Bool (*Elm_Drop_Cb) (void *d, Evas_Object *o, Elm_Drop_Data *data);

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
   ELM_XRENDER_X11,
   ELM_OPENGL_X11,
   ELM_SOFTWARE_WIN32,
   ELM_SOFTWARE_16_WINCE,
   ELM_SOFTWARE_SDL,
   ELM_SOFTWARE_16_SDL,
   ELM_OPENGL_SDL
} Elm_Engine;

typedef enum _Elm_Sel_Type
{
   ELM_SEL_PRIMARY,
   ELM_SEL_SECONDARY,
   ELM_SEL_CLIPBOARD,
   ELM_SEL_XDND,

   ELM_SEL_MAX,
} Elm_Sel_Type;

typedef enum _Elm_Sel_Format
{
   /** Plain unformated text: Used for things that don't want rich markup */
   ELM_SEL_FORMAT_TEXT   = 0x01,
   /** Edje textblock markup, including inline images */
   ELM_SEL_FORMAT_MARKUP = 0x02,
   /** Images */
   ELM_SEL_FORMAT_IMAGE	 = 0x04,
   /** Vcards */
   ELM_SEL_FORMAT_VCARD =  0x08,
} Elm_Sel_Format;

struct _Elm_Drop_Data
{
   int                   x, y;
   Elm_Sel_Format        format;
   void                 *data;
   int                   len;
};

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
   int          thumbscroll_enable;
   int          thumbscroll_threshhold;
   double       thumbscroll_momentum_threshhold;
   double       thumbscroll_friction;
   double       thumbscroll_bounce_friction;
   double       page_scroll_friction;
   double       bring_in_scroll_friction;
   double       zoom_friction;
   int          thumbscroll_bounce_enable;
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

Eina_Bool            elm_selection_set(Elm_Sel_Type selection, Evas_Object *widget, Elm_Sel_Format format, const char *buf);
Eina_Bool            elm_selection_clear(Elm_Sel_Type selection, Evas_Object *widget);
Eina_Bool            elm_selection_get(Elm_Sel_Type selection, Elm_Sel_Format format, Evas_Object *widget);
Eina_Bool            elm_drop_target_add(Evas_Object *widget, Elm_Sel_Type, Elm_Drop_Cb, void *);
Eina_Bool            elm_drop_target_del(Evas_Object *widget);
Eina_Bool            elm_drag_start(Evas_Object *, Elm_Sel_Format, const char *, void (*)(void *,Evas_Object*),void*);

Eina_Bool           _elm_dangerous_call_check(const char *call);

extern char        *_elm_appname;
extern Elm_Config  *_elm_config;
extern const char  *_elm_data_dir;
extern const char  *_elm_lib_dir;
extern int          _elm_log_dom;
extern Eina_List   *_elm_win_list;

#endif
