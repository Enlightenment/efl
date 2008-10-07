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
};

#define ELM_NEW(t) calloc(1, sizeof(t))

void _elm_win_shutdown(void);

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
EAPI void         elm_widget_min_size_set(Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);
EAPI void         elm_widget_min_size_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);
EAPI void         elm_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_sub_object_del(Evas_Object *obj, Evas_Object *sobj);
EAPI void         elm_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj);
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
EAPI void         elm_widget_min_size_resize(Evas_Object *obj);

/*
void _elm_obj_init(Elm_Obj *obj);
void _elm_obj_nest_push(void);
void _elm_obj_nest_pop(void);
int _elm_obj_del_defer(Elm_Obj *obj);
Elm_Cb *_elm_cb_new(void);
void _elm_cb_call(Elm_Obj *obj, Elm_Cb_Type, void *info);
int _elm_theme_set(Evas_Object *o, const char *clas, const char *group);
void _elm_widget_init(Elm_Widget *wid);
void _elm_widget_post_init(Elm_Widget *wid);
*/  

extern char *_elm_appname;

extern Elm_Config *_elm_config;

/*
extern Elm_Obj_Class _elm_obj_class;
extern Elm_Win_Class _elm_win_class;
extern Elm_Widget_Class _elm_widget_class;
extern Elm_Bg_Class _elm_bg_class;
extern Elm_Scroller_Class _elm_scroller_class;
extern Elm_Label_Class _elm_label_class;
extern Elm_Box_Class _elm_box_class;
extern Elm_Table_Class _elm_table_class;
extern Elm_Button_Class _elm_button_class;
extern Elm_Icon_Class _elm_icon_class;
extern Elm_Toggle_Class _elm_toggle_class;
extern Elm_Clock_Class _elm_clock_class;
extern Elm_Frame_Class _elm_frame_class;
extern Elm_Pad_Class _elm_pad_class;
extern Elm_Contactlist_Class _elm_contactlist_class;
*/

#endif
