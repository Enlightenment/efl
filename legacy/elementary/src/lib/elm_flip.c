#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Flip Flip
 *
 * This holds 2 content objects: one on the front and one on the back. It
 * allows you to flip from front to back and vice-versa using various effects.
 *
 * Supported flip types:
 * ELM_FLIP_ROTATE_Y_CENTER_AXIS
 * ELM_FLIP_ROTATE_X_CENTER_AXIS
 * ELM_FLIP_ROTATE_XZ_CENTER_AXIS
 * ELM_FLIP_ROTATE_YZ_CENTER_AXIS
 *
 * Signals that you can add callbacks for are:
 *
 * "animate,done" - when a flip animation is finished
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Ecore_Animator *animator;
   double start, len;
   Elm_Flip_Mode mode;
   struct {
      Evas_Object *content, *clip;
   } front, back;
   Eina_Bool state : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void _configure(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->animator) ecore_animator_del(wd->animator);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, minw2 = -1, minh2 = -1;
   Evas_Coord maxw = -1, maxh = -1, maxw2 = -1, maxh2 = -1;
   if (!wd) return;
   if (wd->front.content)
     evas_object_size_hint_min_get(wd->front.content, &minw, &minh);
   if (wd->back.content)
     evas_object_size_hint_min_get(wd->back.content, &minw2, &minh2);
   if (wd->front.content)
     evas_object_size_hint_max_get(wd->front.content, &maxw, &maxh);
   if (wd->back.content)
     evas_object_size_hint_max_get(wd->back.content, &maxw2, &maxh2);
   
   if (minw2 > minw) minw = minw2;
   if (minh2 > minh) minh = minh2;
   if ((maxw2 >= 0) && (maxw2 < maxw)) maxw = maxw2;
   if ((maxh2 >= 0) && (maxh2 < maxh)) maxh = maxh2;
   
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
   if (sub == wd->front.content)
     {
	evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
	wd->front.content = NULL;
        evas_object_hide(wd->front.clip);
	_sizing_eval(obj);
     }
   else if (sub == wd->back.content)
     {
	evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
	wd->back.content = NULL;
        evas_object_hide(wd->back.clip);
	_sizing_eval(obj);
     }
}

static int
_flip(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double t = ecore_loop_time_get() - wd->start;
   Evas_Coord x, y, w, h;
   double p, deg;
   Evas_Map *mf, *mb;
   Evas_Coord cx, cy, px, py, foc;
   int lx, ly, lz, lr, lg, lb, lar, lag, lab;
   if (!wd->animator) return 0;
   t = t / wd->len;
   if (t > 1.0) t = 1.0;

   if (!wd) return 0;
   evas_object_geometry_get(obj, &x, &y, &w, &h);

   mf = evas_map_new(4);
   evas_map_smooth_set(mf, 0);
   mb = evas_map_new(4);
   evas_map_smooth_set(mb, 0);

   if (wd->front.content)
     evas_map_util_points_populate_from_object_full(mf, wd->front.content, 0);
   if (wd->back.content)
     evas_map_util_points_populate_from_object_full(mb, wd->back.content, 0);
   
   cx = x + (w / 2);
   cy = y + (h / 2);

   px = x + (w / 2);
   py = y + (h / 2);
   foc = 2048;
   
   lx = cx;
   ly = cy;
   lz = -10000;
   lr = 255;
   lg = 255;
   lb = 255;
   lar = 0;
   lag = 0;
   lab = 0;
   
   switch (wd->mode)
     {
     case ELM_FLIP_ROTATE_Y_CENTER_AXIS:
        p = 1.0 - t;
        p = 1.0 - (p * p);
        if (wd->state) deg = 180.0 * p;
        else deg = 180 + (180.0 * p);
        evas_map_util_3d_rotate(mf, 0.0, deg, 0.0, cx, cy, 0);
        evas_map_util_3d_rotate(mb, 0.0, deg + 180.0, 0.0, cx, cy, 0);
        break;
     case ELM_FLIP_ROTATE_X_CENTER_AXIS:
        p = 1.0 - t;
        p = 1.0 - (p * p);
        if (wd->state) deg = 180.0 * p;
        else deg = 180 + (180.0 * p);
        evas_map_util_3d_rotate(mf, deg, 0.0, 0.0, cx, cy, 0);
        evas_map_util_3d_rotate(mb, deg + 180.0, 0.0, 0.0, cx, cy, 0);
        break;
     case ELM_FLIP_ROTATE_XZ_CENTER_AXIS:
        p = 1.0 - t;
        p = 1.0 - (p * p);
        if (wd->state) deg = 180.0 * p;
        else deg = 180 + (180.0 * p);
        evas_map_util_3d_rotate(mf, deg, 0.0, deg, cx, cy, 0);
        evas_map_util_3d_rotate(mb, deg + 180.0, 0.0, deg + 180.0, cx, cy, 0);
        break;
     case ELM_FLIP_ROTATE_YZ_CENTER_AXIS:
        p = 1.0 - t;
        p = 1.0 - (p * p);
        if (wd->state) deg = 180.0 * p;
        else deg = 180 + (180.0 * p);
        evas_map_util_3d_rotate(mf, 0.0, deg, deg, cx, cy, 0);
        evas_map_util_3d_rotate(mb, 0.0, deg + 180.0, deg + 180.0, cx, cy, 0);
        break;
     default:
        break;
     }

   
   if (wd->front.content)
     {
        evas_map_util_3d_lighting(mf, lx, ly, lz, lr, lg, lb, lar, lag, lab);
        evas_map_util_3d_perspective(mf, px, py, 0, foc);
        evas_object_map_set(wd->front.content, mf);
        evas_object_map_enable_set(wd->front.content, 1);
        if (evas_map_util_clockwise_get(mf)) evas_object_show(wd->front.clip);
        else evas_object_hide(wd->front.clip);
     }
      
   if (wd->back.content)
     {
        evas_map_util_3d_lighting(mb, lx, ly, lz, lr, lg, lb, lar, lag, lab);
        evas_map_util_3d_perspective(mb, px, py, 0, foc);
        evas_object_map_set(wd->back.content, mb);
        evas_object_map_enable_set(wd->back.content, 1);
        if (evas_map_util_clockwise_get(mb)) evas_object_show(wd->back.clip);
        else evas_object_hide(wd->back.clip);
     }
   
   evas_map_free(mf);
   evas_map_free(mb);
   
   if (t >= 1.0)
     {
        evas_object_map_enable_set(wd->front.content, 0);
        evas_object_map_enable_set(wd->back.content, 0);
        wd->animator = NULL;
        wd->state = !wd->state;
        _configure(obj);
        evas_object_smart_callback_call(obj, "animate,done", NULL);
        return 0;
     }
   return 1;
}

static void
_configure(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x, y, w, h;
   if (!wd) return;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   if (wd->front.content)
     {
        if (!wd->animator)
          evas_object_move(wd->front.content, x, y);
        evas_object_resize(wd->front.content, w, h);
     }
   if (wd->back.content)
     {
        if (!wd->animator)
          evas_object_move(wd->back.content, x, y);
        evas_object_resize(wd->back.content, w, h);
     }
   _flip(obj);
}

static void
_move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   _configure(obj);
}

static void
_resize(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{    
   _configure(obj);
}

static int
_animate(void *data)
{
   return _flip(data);
}

/**
 * Add a new flip to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Flip
 */
EAPI Evas_Object *
elm_flip_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "flip");
   elm_widget_type_set(obj, "flip");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->front.clip = evas_object_rectangle_add(e);
   evas_object_color_set(wd->front.clip, 255, 255, 255, 255);
   evas_object_move(wd->front.clip, -49999, -49999);
   evas_object_resize(wd->front.clip, 99999, 99999);
   elm_widget_sub_object_add(wd->front.clip, obj);
   evas_object_smart_member_add(wd->front.clip, obj);
   
   wd->back.clip = evas_object_rectangle_add(e);
   evas_object_color_set(wd->back.clip, 255, 255, 255, 255);
   evas_object_move(wd->back.clip, -49999, -49999);
   evas_object_resize(wd->back.clip, 99999, 99999);
   elm_widget_sub_object_add(wd->back.clip, obj);
   evas_object_smart_member_add(wd->back.clip, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, NULL);
   
   wd->state = 1;
     
   _sizing_eval(obj);
   return obj;
}

/**
 * Set the flip front content
 *
 * @param obj The flip object
 * @param content The content to be used in this flip object
 *
 * @ingroup Flip
 */
EAPI void
elm_flip_content_front_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->front.content == content) return;
   if ((wd->front.content != content) && (wd->front.content))
     {
        evas_object_clip_set(wd->front.content, NULL);
        elm_widget_sub_object_del(obj, wd->front.content);
        evas_object_smart_member_del(wd->front.content);
     }
   wd->front.content = content;
   if (content)
     {
	elm_widget_sub_object_add(content, obj);
        evas_object_smart_member_add(content, obj);
        evas_object_clip_set(content, wd->front.clip);
	evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
        if (!elm_flip_front_get(obj)) evas_object_hide(wd->front.clip);
        else evas_object_show(wd->front.clip);
     }
   else
     evas_object_hide(wd->front.clip);     
   _configure(obj);
}

/**
 * Set the flip back content
 *
 * @param obj The flip object
 * @param content The content to be used in this flip object
 *
 * @ingroup Flip
 */
EAPI void
elm_flip_content_back_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->back.content == content) return;
   if ((wd->back.content != content) && (wd->back.content))
     {
        evas_object_clip_set(wd->back.content, NULL);
        elm_widget_sub_object_del(obj, wd->back.content);
        evas_object_smart_member_del(wd->back.content);
     }
   wd->back.content = content;
   if (content)
     {
	elm_widget_sub_object_add(content, obj);
        evas_object_smart_member_add(content, obj);
        evas_object_clip_set(content, wd->back.clip);
	evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
        if (elm_flip_front_get(obj)) evas_object_hide(wd->back.clip);
        else evas_object_show(wd->back.clip);
     }
   else
     evas_object_hide(wd->back.clip);
   _configure(obj);
}

/**
 * Get the flip front content
 *
 * @param obj The flip object
 * @return The content to be used in this flip object front
 *
 * @ingroup Flip
 */
EAPI Evas_Object *
elm_flip_content_front_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->front.content;
}

/**
 * Get the flip back content
 *
 * @param obj The flip object
 * @return The content to be used in this flip object back
 *
 * @ingroup Flip
 */
EAPI Evas_Object *
elm_flip_content_back_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->back.content;
}

/**
 * Get flip front visibility state
 *
 * @param obj The flip object
 * @return If front front is showing or not currently
 *
 * @ingroup Flip
 */
EAPI Eina_Bool
elm_flip_front_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->state;
}

/**
 * Set flip perspective
 *
 * @param obj The flip object
 * @param foc The coordinate to set the focus on
 * @param x The X coordinate
 * @param y The Y coordinate
 *
 * NOTE: This function currently does nothing.
 *
 * @ingroup Flip
 */
EAPI void
elm_flip_perspective_set(Evas_Object *obj, Evas_Coord foc __UNUSED__, Evas_Coord x __UNUSED__, Evas_Coord y __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
}

/**
 * Runs the flip animation
 *
 * @param obj The flip object
 * @param mode The mode type.  Currently accepted modes are:
 *
 * ELM_FLIP_ROTATE_Y_CENTER_AXIS
 * ELM_FLIP_ROTATE_X_CENTER_AXIS
 * ELM_FLIP_ROTATE_XZ_CENTER_AXIS
 * ELM_FLIP_ROTATE_YZ_CENTER_AXIS
 *
 * @ingroup Flip
 */
EAPI void
elm_flip_go(Evas_Object *obj, Elm_Flip_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!wd->animator) wd->animator = ecore_animator_add(_animate, obj);
   wd->mode = mode;
   wd->start = ecore_loop_time_get();
   wd->len = 0.5;
}
