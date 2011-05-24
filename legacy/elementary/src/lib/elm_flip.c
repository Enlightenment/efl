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
 * ELM_FLIP_CUBE_LEFT
 * ELM_FLIP_CUBE_RIGHT
 * ELM_FLIP_CUBE_UP
 * ELM_FLIP_CUBE_DOWN
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
   Evas_Object *clip;
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

static const char SIG_ANIMATE_DONE[] = "animate,done";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_ANIMATE_DONE, ""},
   {NULL, NULL}
};

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

static Eina_Bool
_elm_flip_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd)
     return EINA_FALSE;

   /* Try Focus cycle in subitem */
   if (wd->state)
     return elm_widget_focus_next_get(wd->front.content, dir, next);
   else
     return elm_widget_focus_next_get(wd->back.content, dir, next);

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

static void
flip_show_hide(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (elm_flip_front_get(obj))
     {
        if (wd->front.content)
          evas_object_show(wd->front.clip);
        else
          evas_object_hide(wd->front.clip);
        if (wd->back.content)
          evas_object_hide(wd->back.clip);
        else
          evas_object_hide(wd->back.clip);
     }
   else
     {
        if (wd->front.content)
          evas_object_hide(wd->front.clip);
        else
          evas_object_hide(wd->front.clip);
        if (wd->back.content)
          evas_object_show(wd->back.clip);
        else
          evas_object_hide(wd->back.clip);
     }
}

static Eina_Bool
_flip(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double t = ecore_loop_time_get() - wd->start;
   Evas_Coord x, y, w, h;
   double p, deg;
   Evas_Map *mf, *mb;
   Evas_Coord cx, cy, px, py, foc;
   int lx, ly, lz, lr, lg, lb, lar, lag, lab;
   if (!wd->animator) return ECORE_CALLBACK_CANCEL;
   t = t / wd->len;
   if (t > 1.0) t = 1.0;

   if (!wd) return ECORE_CALLBACK_CANCEL;

   mf = evas_map_new(4);
   evas_map_smooth_set(mf, 0);
   mb = evas_map_new(4);
   evas_map_smooth_set(mb, 0);

   if (wd->front.content)
     {
        evas_object_geometry_get(wd->front.content, &x, &y, &w, &h);
        evas_map_util_points_populate_from_geometry(mf, x, y, w, h, 0);
     }
   if (wd->back.content)
     {
        evas_object_geometry_get(wd->back.content, &x, &y, &w, &h);
        evas_map_util_points_populate_from_geometry(mb, x, y, w, h, 0);
     }

   evas_object_geometry_get(obj, &x, &y, &w, &h);

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
      case ELM_FLIP_CUBE_LEFT:
         p = 1.0 - t;
         p = 1.0 - (p * p);
         deg = -90.0 * p;
         if (wd->state)
           {
              evas_map_util_3d_rotate(mf, 0.0, deg, 0.0, cx, cy, w / 2);
              evas_map_util_3d_rotate(mb, 0.0, deg + 90, 0.0, cx, cy, w / 2);
           }
         else
           {
              evas_map_util_3d_rotate(mf, 0.0, deg + 90, 0.0, cx, cy, w / 2);
              evas_map_util_3d_rotate(mb, 0.0, deg, 0.0, cx, cy, w / 2);
           }
         break;
      case ELM_FLIP_CUBE_RIGHT:
         p = 1.0 - t;
         p = 1.0 - (p * p);
         deg = 90.0 * p;
         if (wd->state)
           {
              evas_map_util_3d_rotate(mf, 0.0, deg, 0.0, cx, cy, w / 2);
              evas_map_util_3d_rotate(mb, 0.0, deg - 90, 0.0, cx, cy, w / 2);
           }
         else
           {
              evas_map_util_3d_rotate(mf, 0.0, deg - 90, 0.0, cx, cy, w / 2);
              evas_map_util_3d_rotate(mb, 0.0, deg, 0.0, cx, cy, w / 2);
           }
         break;
      case ELM_FLIP_CUBE_UP:
         p = 1.0 - t;
         p = 1.0 - (p * p);
         deg = -90.0 * p;
         if (wd->state)
           {
              evas_map_util_3d_rotate(mf, deg, 0.0, 0.0, cx, cy, h / 2);
              evas_map_util_3d_rotate(mb, deg + 90, 0.0, 0.0, cx, cy, h / 2);
           }
         else
           {
              evas_map_util_3d_rotate(mf, deg + 90, 0.0, 0.0, cx, cy, h / 2);
              evas_map_util_3d_rotate(mb, deg, 0.0, 0.0, cx, cy, h / 2);
           }
         break;
      case ELM_FLIP_CUBE_DOWN:
         p = 1.0 - t;
         p = 1.0 - (p * p);
         deg = 90.0 * p;
         if (wd->state)
           {
              evas_map_util_3d_rotate(mf, deg, 0.0, 0.0, cx, cy, h / 2);
              evas_map_util_3d_rotate(mb, deg - 90, 0.0, 0.0, cx, cy, h / 2);
           }
         else
           {
              evas_map_util_3d_rotate(mf, deg - 90, 0.0, 0.0, cx, cy, h / 2);
              evas_map_util_3d_rotate(mb, deg, 0.0, 0.0, cx, cy, h / 2);
           }
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
        // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow
        evas_object_resize(wd->front.content, 0, 0);
        evas_object_resize(wd->back.content, 0, 0);
        evas_smart_objects_calculate(evas_object_evas_get(obj));
        // FIXME: end hack
        wd->animator = NULL;
        wd->state = !wd->state;
        _configure(obj);
        evas_object_smart_callback_call(obj, SIG_ANIMATE_DONE, NULL);
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
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

static Eina_Bool
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

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "flip");
   elm_widget_type_set(obj, "flip");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_focus_next_hook_set(obj, _elm_flip_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->clip = evas_object_rectangle_add(e);
   evas_object_static_clip_set(wd->clip, 1);
   evas_object_color_set(wd->clip, 255, 255, 255, 255);
   evas_object_move(wd->clip, -49999, -49999);
   evas_object_resize(wd->clip, 99999, 99999);
   elm_widget_sub_object_add(obj, wd->clip);
   evas_object_clip_set(wd->clip, evas_object_clip_get(obj));
   evas_object_smart_member_add(wd->clip, obj);

   wd->front.clip = evas_object_rectangle_add(e);
   evas_object_static_clip_set(wd->front.clip, 1);
   evas_object_data_set(wd->front.clip, "_elm_leaveme", obj);
   evas_object_color_set(wd->front.clip, 255, 255, 255, 255);
   evas_object_move(wd->front.clip, -49999, -49999);
   evas_object_resize(wd->front.clip, 99999, 99999);
   elm_widget_sub_object_add(obj, wd->front.clip);
   evas_object_smart_member_add(wd->front.clip, obj);
   evas_object_clip_set(wd->front.clip, wd->clip);

   wd->back.clip = evas_object_rectangle_add(e);
   evas_object_static_clip_set(wd->back.clip, 1);
   evas_object_data_set(wd->back.clip, "_elm_leaveme", obj);
   evas_object_color_set(wd->back.clip, 255, 255, 255, 255);
   evas_object_move(wd->back.clip, -49999, -49999);
   evas_object_resize(wd->back.clip, 99999, 99999);
   elm_widget_sub_object_add(wd->back.clip, obj);
   evas_object_smart_member_add(obj, wd->back.clip);
   evas_object_clip_set(wd->back.clip, wd->clip);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, NULL);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   wd->state = 1;

   _sizing_eval(obj);

   return obj;
}

/**
 * Set the front content of the flip widget.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_flip_content_front_unset() function.
 *
 * @param obj The flip object
 * @param content The new front content object
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
   if (wd->front.content) evas_object_del(wd->back.content);
   wd->front.content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_smart_member_add(content, obj);
        evas_object_clip_set(content, wd->front.clip);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        _sizing_eval(obj);
     }
   // force calc to contents are the right size before transition
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   flip_show_hide(obj);
   _configure(obj);
}

/**
 * Set the back content of the flip widget.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_flip_content_back_unset() function.
 *
 * @param obj The flip object
 * @param content The new back content object
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
   if (wd->back.content) evas_object_del(wd->back.content);
   wd->back.content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_smart_member_add(content, obj);
        evas_object_clip_set(content, wd->back.clip);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        _sizing_eval(obj);
     }
   // force calc to contents are the right size before transition
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   flip_show_hide(obj);
   _configure(obj);
}

/**
 * Get the front content used for the flip
 *
 * Return the front content object which is set for this widget.
 *
 * @param obj The flip object
 * @return The front content object that is being used
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
 * Get the back content used for the flip
 *
 * Return the back content object which is set for this widget.
 *
 * @param obj The flip object
 * @return The back content object that is being used
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
 * Unset the front content used for the flip
 *
 * Unparent and return the front content object which was set for this widget.
 *
 * @param obj The flip object
 * @return The front content object that was being used
 *
 * @ingroup Flip
 */
EAPI Evas_Object *
elm_flip_content_front_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->front.content) return NULL;
   Evas_Object *content = wd->front.content;
   evas_object_clip_unset(content);
   elm_widget_sub_object_del(obj, content);
   evas_object_smart_member_del(content);
   wd->front.content = NULL;
   return content;
}

/**
 * Unset the back content used for the flip
 *
 * Unparent and return the back content object which was set for this widget.
 *
 * @param obj The flip object
 * @return The back content object that was being used
 *
 * @ingroup Flip
 */
EAPI Evas_Object *
elm_flip_content_back_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->back.content) return NULL;
   Evas_Object *content = wd->back.content;
   evas_object_clip_unset(content);
   elm_widget_sub_object_del(obj, content);
   evas_object_smart_member_del(content);
   wd->back.content = NULL;
   return content;
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
 * ELM_FLIP_CUBE_LEFT
 * ELM_FLIP_CUBE_RIGHT
 * ELM_FLIP_CUBE_UP
 * ELM_FLIP_CUBE_DOWN
 * 
 * FIXME: add - ELM_FLIP_CUBE_UP
 * FIXME: add - ELM_FLIP_CUBE_DOWN
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
   flip_show_hide(obj);
   wd->mode = mode;
   wd->start = ecore_loop_time_get();
   wd->len = 0.5;
   // force calc to contents are the right size before transition
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   _flip(obj);
   // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow)
   evas_object_map_enable_set(wd->front.content, 0);
   evas_object_map_enable_set(wd->back.content, 0);
   evas_object_resize(wd->front.content, 0, 0);
   evas_object_resize(wd->back.content, 0, 0);
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   _configure(obj);
   // FIXME: end hack
}
