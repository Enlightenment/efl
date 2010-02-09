#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Conformant Conformant
 * 
 * The aim is to provide a widget that can be used in elementary apps to 
 * account for space taken up by the indicator & softkey windows when running 
 * the illume2 module of E17.
 */

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data 
{
   Evas_Object *base;
   Evas_Object *shelf, *panel;
   Evas_Object *content;
   Ecore_Event_Handler *prop_hdl;
   struct {
      Ecore_Animator *animator; // animaton timer
      double start; // time started
      Evas_Coord auto_x, auto_y; // desired delta
      Evas_Coord x, y; // current delta
   } delta;
};

/* local function prototypes */
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static int _prop_change(void *data, int type, void *event);

/* local functions */
static void 
_del_hook(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->prop_hdl) ecore_event_handler_del(wd->prop_hdl);
   free(wd);
}

static void 
_theme_hook(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   _elm_theme_set(wd->base, "conformant", "base", elm_widget_style_get(obj));
   if (wd->content)
     edje_object_part_swallow(wd->base, "elm.swallow.content", wd->content);
   edje_object_scale_set(wd->base, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void 
_sizing_eval(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw = -1, mh = -1;

   edje_object_size_min_calc(wd->base, &mw, &mh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   
   if (sub == wd->content)
     {
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->content = NULL;
        _sizing_eval(obj);
     }
}

/* unused now - but meant to be for making sure the focused widget is always
 * visible when the vkbd comes and goes by moving the conformant obj (and thus
 * its children) to  show the focused widget (and if focus changes follow)

static Evas_Object *
_focus_object_get(const Evas_Object *obj)
{
   Evas_Object *win, *foc;
   
   win = elm_widget_top_get(obj);
   if (!win) return NULL;
   foc = elm_widget_top_get(win);
}

static void
_focus_object_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   evas_object_geometry_get(obj, x, y, w, h);
}

static void
_focus_change_del(void *data, Evas_Object *obj, void *event_info)
{
   // called from toplevel when the focused window shanges
}

static void
_autoscroll_move(Evas_Object *obj)
{
   // move conformant edje by delta to show focused widget
}

static void
_autoscroll_mode_enable(Evas_Object *obj)
{
   // called when autoscroll mode should be on - content area smaller than
   // its min size
   // 1. get focused object
   // 2. if not in visible conformant area calculate delta needed to
   //    get it in
   // 3. store delta and call _autoscroll_move() which either asanimates
   //    or jumps right there
}

static void
_autoscroll_mode_disable(Evas_Object *obj)
{
   // called when autoscroll mode should be off - set delta to 0 and
   // call _autoscroll_move()
}
*/

static int 
_prop_change(void *data, int type, void *event) 
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Event_Window_Property *ev;
   Widget_Data *wd = elm_widget_data_get(data);

   ev = event;
   if (ev->win != ecore_x_window_root_first_get()) return 1;
   if (ev->atom == ECORE_X_ATOM_E_ILLUME_TOP_SHELF_GEOMETRY) 
     {
        int sh = -1;

        ecore_x_e_illume_top_shelf_geometry_get(ecore_x_window_root_first_get(), 
                                                NULL, NULL, NULL, &sh);
        if (sh < 0) sh = 0;
        evas_object_size_hint_min_set(wd->shelf, -1, sh);
        evas_object_size_hint_max_set(wd->shelf, -1, sh);
     }
   if (ev->atom == ECORE_X_ATOM_E_ILLUME_BOTTOM_PANEL_GEOMETRY) 
     {
        int sh = -1;

        ecore_x_e_illume_bottom_panel_geometry_get(ecore_x_window_root_first_get(), 
                                                   NULL, NULL, NULL, &sh);
        if (sh < 0) sh = 0;
        evas_object_size_hint_min_set(wd->panel, -1, sh);
        evas_object_size_hint_max_set(wd->panel, -1, sh);
     }
   // FIXME: listen to kbd region property changes too.
#endif

   return 1;
}

/**
 * Add a new Conformant object
 * 
 * @param parent The parent object
 * @return The new conformant object or NULL if it cannot be created
 * 
 * @ingroup Conformant
 */
EAPI Evas_Object *
elm_conformant_add(Evas_Object *parent) 
{
   Evas_Object *obj;
   Evas *evas;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);

   evas = evas_object_evas_get(parent);

   obj = elm_widget_add(evas);
   elm_widget_type_set(obj, "conformant");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->base = edje_object_add(evas);
   _elm_theme_set(wd->base, "conformant", "base", "default");
   elm_widget_resize_object_set(obj, wd->base);

#ifdef HAVE_ELEMENTARY_X
   int sh = -1;

   ecore_x_e_illume_top_shelf_geometry_get(ecore_x_window_root_first_get(), 
                                           NULL, NULL, NULL, &sh);
   if (sh < 0) sh = 0;

   wd->shelf = evas_object_rectangle_add(evas);
   evas_object_color_set(wd->shelf, 0, 0, 0, 0);
   evas_object_size_hint_min_set(wd->shelf, -1, sh);
   evas_object_size_hint_max_set(wd->shelf, -1, sh);
   edje_object_part_swallow(wd->base, "elm.swallow.shelf", wd->shelf);

   sh = -1;
   ecore_x_e_illume_bottom_panel_geometry_get(ecore_x_window_root_first_get(), 
                                              NULL, NULL, NULL, &sh);
   if (sh < 0) sh = 0;

   wd->panel = evas_object_rectangle_add(evas);
   evas_object_color_set(wd->panel, 0, 0, 0, 0);
   evas_object_size_hint_min_set(wd->panel, -1, sh);
   evas_object_size_hint_max_set(wd->panel, -1, sh);
   edje_object_part_swallow(wd->base, "elm.swallow.panel", wd->panel);

   wd->prop_hdl = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY, 
                                          _prop_change, obj);
   // FIXME: get kbd region prop
#endif
   
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

/**
 * Set the content of the conformant widget
 * 
 * @param obj The conformant object
 * @param content The content that will be used inside this conformant object
 * 
 * @ingroup Conformant
 */
EAPI void 
elm_conformant_content_set(Evas_Object *obj, Evas_Object *content) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((wd->content != content) && (wd->content))
     elm_widget_sub_object_del(obj, wd->content);
   wd->content = content;
   if (content) 
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->base, "elm.swallow.content", content);
        _sizing_eval(obj);
     }
}
