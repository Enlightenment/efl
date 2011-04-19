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
   struct
   {
      Ecore_Animator *animator; // animaton timer
      double start; // time started
      Evas_Coord auto_x, auto_y; // desired delta
      Evas_Coord x, y; // current delta
   } delta;
};

/* local function prototypes */
static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static Eina_Bool _prop_change(void *data, int type, void *event);

/* local functions */
static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->prop_hdl) ecore_event_handler_del(wd->prop_hdl);
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   edje_object_mirrored_set(wd->base, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->base, "conformant", "base",
                         elm_widget_style_get(obj));

   if (wd->content)
     edje_object_part_swallow(wd->base, "elm.swallow.content", wd->content);
   edje_object_scale_set(wd->base, elm_widget_scale_get(obj)
                         * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw = -1, mh = -1;

   if (!wd) return;
   edje_object_size_min_calc(wd->base, &mw, &mh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;

   if (!wd) return;
   if (sub == wd->content)
     {
        evas_object_event_callback_del_full(sub,
                                            EVAS_CALLBACK_CHANGED_SIZE_HINTS,
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

static Eina_Bool
_prop_change(void *data, int type __UNUSED__, void *event)
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Event_Window_Property *ev;
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return ECORE_CALLBACK_PASS_ON;
   ev = event;
   if (ev->atom == ECORE_X_ATOM_E_ILLUME_ZONE)
     {
        Ecore_X_Window zone;
        int sh = -1;

        zone = ecore_x_e_illume_zone_get(ev->win);
        ecore_x_e_illume_indicator_geometry_get(zone, NULL, NULL, NULL, &sh);
        if (sh < 0) sh = 0;
        evas_object_size_hint_min_set(wd->shelf, -1, sh);
        evas_object_size_hint_max_set(wd->shelf, -1, sh);
        sh = -1;
        ecore_x_e_illume_softkey_geometry_get(zone, NULL, NULL, NULL, &sh);
        if (sh < 0) sh = 0;
        evas_object_size_hint_min_set(wd->panel, -1, sh);
        evas_object_size_hint_max_set(wd->panel, -1, sh);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY)
     {
        Ecore_X_Window zone;
        int sh = -1;

        zone = ecore_x_e_illume_zone_get(ev->win);
        ecore_x_e_illume_indicator_geometry_get(zone, NULL, NULL, NULL, &sh);
        if (sh < 0) sh = 0;
        evas_object_size_hint_min_set(wd->shelf, -1, sh);
        evas_object_size_hint_max_set(wd->shelf, -1, sh);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY)
     {
        Ecore_X_Window zone;
        int sh = -1;

        zone = ecore_x_e_illume_zone_get(ev->win);
        ecore_x_e_illume_softkey_geometry_get(zone, NULL, NULL, NULL, &sh);
        if (sh < 0) sh = 0;
        evas_object_size_hint_min_set(wd->panel, -1, sh);
        evas_object_size_hint_max_set(wd->panel, -1, sh);
     }
   else if (ev->atom == ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY)
     {
        Ecore_X_Window zone;
        int ky = -1;

        printf("Keyboard Geometry Changed\n");
        zone = ecore_x_e_illume_zone_get(ev->win);
        ecore_x_e_illume_keyboard_geometry_get(zone, NULL, &ky, NULL, NULL);
        printf("\tGeom: %d\n", ky);
     }
#endif

   return ECORE_CALLBACK_PASS_ON;
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
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "conformant");
   elm_widget_type_set(obj, "conformant");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->base = edje_object_add(e);
   _elm_theme_object_set(obj, wd->base, "conformant", "base", "default");
   elm_widget_resize_object_set(obj, wd->base);

#ifdef HAVE_ELEMENTARY_X
   Evas_Object *top = elm_widget_top_get(obj);
   Ecore_X_Window zone, xwin = elm_win_xwindow_get(top);

   if ((xwin) && (!elm_win_inlined_image_object_get(top)))
     {
        int sh = -1;

        zone = ecore_x_e_illume_zone_get(xwin);

        ecore_x_e_illume_indicator_geometry_get(zone, NULL, NULL, NULL, &sh);
        if (sh < 0) sh = 0;
        wd->shelf = evas_object_rectangle_add(e);
        evas_object_color_set(wd->shelf, 0, 0, 0, 0);
        evas_object_size_hint_min_set(wd->shelf, -1, sh);
        evas_object_size_hint_max_set(wd->shelf, -1, sh);
        edje_object_part_swallow(wd->base, "elm.swallow.shelf", wd->shelf);

        sh = -1;
        ecore_x_e_illume_softkey_geometry_get(zone, NULL, NULL, NULL, &sh);
        if (sh < 0) sh = 0;
        wd->panel = evas_object_rectangle_add(e);
        evas_object_color_set(wd->panel, 0, 0, 0, 0);
        evas_object_size_hint_min_set(wd->panel, -1, sh);
        evas_object_size_hint_max_set(wd->panel, -1, sh);
        edje_object_part_swallow(wd->base, "elm.swallow.panel", wd->panel);

        wd->prop_hdl = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY,
                                               _prop_change, obj);
     }
   // FIXME: get kbd region prop
#endif

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

/**
 * Set the content of the conformant widget
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_conformat_content_unset() function.
 *
 * @param obj The conformant object
 * @return The content that was being used
 *
 * @ingroup Conformant
 */
EAPI void
elm_conformant_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->base, "elm.swallow.content", content);
     }
   _sizing_eval(obj);
}

/**
 * Get the content of the conformant widget
 *
 * Return the content object which is set for this widget;
 *
 * @param obj The conformant object
 * @return The content that is being used
 *
 * @ingroup Conformant
 */
EAPI Evas_Object *
elm_conformant_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   return wd->content;
}

/**
 * Unset the content of the conformant widget
 *
 * Unparent and return the content object which was set for this widget;
 *
 * @param obj The conformant object
 * @return The content that was being used
 *
 * @ingroup Conformant
 */
EAPI Evas_Object *
elm_conformant_content_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content;

   if (!wd) return NULL;
   if (!wd->content) return NULL;
   content = wd->content;
   elm_widget_sub_object_del(obj, wd->content);
   edje_object_part_unswallow(wd->base, wd->content);
   wd->content = NULL;
   return content;
}
