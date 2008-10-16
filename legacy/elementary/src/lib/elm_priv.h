#ifndef ELM_PRIV_H
#define ELM_PRIV_H

#include "els_pan.h"
#include "els_scroller.h"
#include "els_box.h"
#include "els_table.h"
#include "els_icon.h"

// FIXME: totally disorganised. clean this up!

typedef enum _Elm_Engine
{
   ELM_SOFTWARE_X11,
     ELM_SOFTWARE_FB,
     ELM_SOFTWARE_16_X11,
     ELM_XRENDER_X11,
     ELM_OPENGL_X11
} Elm_Engine;

typedef struct _Elm_Config Elm_Config;

struct _Elm_Config
{
   int engine;
   int thumbscroll_enable;
   int thumbscroll_threshhold;
   double thumbscroll_momentum_threshhold;
   double thumbscroll_friction;
   double scale;
   int bgpixmap;
   int compositing;
};

#define ELM_NEW(t) calloc(1, sizeof(t))

void _elm_win_shutdown(void);

int _elm_theme_set(Evas_Object *o, const char *clas, const char *group, const char *style);
int _elm_theme_icon_set(Evas_Object *o, const char *group, const char *style);
    
/* FIXME: should this be public? for now - private (but public symbols) */
EAPI Evas_Object *elm_widget_add(Evas *evas);
EAPI void         elm_widget_del_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_focus_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_activate_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_disable_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj));
EAPI void         elm_widget_on_focus_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data);
EAPI void         elm_widget_on_change_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data);
EAPI void         elm_widget_data_set(Evas_Object *obj, void *data);
EAPI void        *elm_widget_data_get(Evas_Object *obj);
EAPI void         elm_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_sub_object_del(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_hover_object_set(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_can_focus_set(Evas_Object *obj, int can_focus);
EAPI int          elm_widget_can_focus_get(Evas_Object *obj);
EAPI int          elm_widget_focus_get(Evas_Object *obj);
EAPI Evas_Object *elm_widget_focused_object_get(Evas_Object *obj);
EAPI int          elm_widget_focus_jump(Evas_Object *obj, int forward);
EAPI void         elm_widget_focus_set(Evas_Object *obj, int first);
EAPI void         elm_widget_focused_object_clear(Evas_Object *obj);
EAPI Evas_Object *elm_widget_parent_get(Evas_Object *obj);
EAPI void         elm_widget_focus_steal(Evas_Object *obj);
EAPI void         elm_widget_activate(Evas_Object *obj);
EAPI void         elm_widget_change(Evas_Object *obj);
EAPI void         elm_widget_disabled_set(Evas_Object *obj, int disabled);
EAPI int          elm_widget_disabled_get(Evas_Object *obj);

extern char *_elm_appname;

extern Elm_Config *_elm_config;

#endif
