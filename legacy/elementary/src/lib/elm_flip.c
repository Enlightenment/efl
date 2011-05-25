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
   Evas_Object *obj;
   Ecore_Animator *animator;
   double start, len;
   Elm_Flip_Mode mode;
   Evas_Object *clip;
   Evas_Object *event[4];
   struct {
      Evas_Object *content, *clip;
   } front, back;
   Ecore_Job *job;
   Evas_Coord down_x, down_y, x, y, ox, oy, w, h;
   Elm_Flip_Interaction intmode;
   int dir;
   Eina_Bool dir_enabled[4];
   Eina_Bool state : 1;
   Eina_Bool down : 1;
   Eina_Bool finish : 1;
   Eina_Bool started : 1;
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
   int fingx = 0, fingy = 0;
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

   if (wd->dir_enabled[0]) fingy++;
   if (wd->dir_enabled[1]) fingy++;
   if (wd->dir_enabled[2]) fingx++;
   if (wd->dir_enabled[3]) fingx++;
   
   elm_coords_finger_size_adjust(fingx, &minw, fingy, &minh);
   
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

static void
_flip_do(Evas_Object *obj, double t, Elm_Flip_Mode mode, int lin, int rev)
{
   Evas_Coord x, y, w, h;
   double p, deg, pp;
   Evas_Map *mf, *mb;
   Evas_Coord cx, cy, px, py, foc;
   int lx, ly, lz, lr, lg, lb, lar, lag, lab;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   mf = evas_map_new(4);
   evas_map_smooth_set(mf, 0);
   mb = evas_map_new(4);
   evas_map_smooth_set(mb, 0);

   if (wd->front.content)
     {
        const char *type = evas_object_type_get(wd->front.content);

        // FIXME: only handles filled obj
        if ((type) && (!strcmp(type, "image")))
          {
             int iw, ih;
             evas_object_image_size_get(wd->front.content, &iw, &ih);
             evas_object_geometry_get(wd->front.content, &x, &y, &w, &h);
             evas_map_util_points_populate_from_geometry(mf, x, y, w, h, 0);
             evas_map_point_image_uv_set(mf, 0, 0, 0);
             evas_map_point_image_uv_set(mf, 1, iw, 0);
             evas_map_point_image_uv_set(mf, 2, iw, ih);
             evas_map_point_image_uv_set(mf, 3, 0, ih);
          }
        else
          {
             evas_object_geometry_get(wd->front.content, &x, &y, &w, &h);
             evas_map_util_points_populate_from_geometry(mf, x, y, w, h, 0);
          }
     }
   if (wd->back.content)
     {
        const char *type = evas_object_type_get(wd->back.content);
        
        if ((type) && (!strcmp(type, "image")))
          {
             int iw, ih;
             evas_object_image_size_get(wd->back.content, &iw, &ih);
             evas_object_geometry_get(wd->back.content, &x, &y, &w, &h);
             evas_map_util_points_populate_from_geometry(mb, x, y, w, h, 0);
             evas_map_point_image_uv_set(mb, 0, 0, 0);
             evas_map_point_image_uv_set(mb, 1, iw, 0);
             evas_map_point_image_uv_set(mb, 2, iw, ih);
             evas_map_point_image_uv_set(mb, 3, 0, ih);
          }
        else
          {
             evas_object_geometry_get(wd->back.content, &x, &y, &w, &h);
             evas_map_util_points_populate_from_geometry(mb, x, y, w, h, 0);
          }
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

   switch (mode)
     {
      case ELM_FLIP_ROTATE_Y_CENTER_AXIS:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         if (wd->state) deg = 180.0 * p;
         else deg = 180 + (180.0 * p);
         if (rev) deg = -deg;
         evas_map_util_3d_rotate(mf, 0.0, deg, 0.0, cx, cy, 0);
         evas_map_util_3d_rotate(mb, 0.0, 180 + deg, 0.0, cx, cy, 0);
         break;
      case ELM_FLIP_ROTATE_X_CENTER_AXIS:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         if (wd->state) deg = 180.0 * p;
         else deg = 180 + (180.0 * p);
         if (rev) deg = -deg;
         evas_map_util_3d_rotate(mf, deg, 0.0, 0.0, cx, cy, 0);
         evas_map_util_3d_rotate(mb, 180.0 + deg, 0.0, 0.0, cx, cy, 0);
         break;
      case ELM_FLIP_ROTATE_XZ_CENTER_AXIS:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         if (wd->state) deg = 180.0 * p;
         else deg = 180 + (180.0 * p);
         if (rev) deg = -deg;
         evas_map_util_3d_rotate(mf, deg, 0.0, deg, cx, cy, 0);
         evas_map_util_3d_rotate(mb, 180 + deg, 0.0, 180 + deg, cx, cy, 0);
         break;
      case ELM_FLIP_ROTATE_YZ_CENTER_AXIS:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         if (wd->state) deg = 180.0 * p;
         else deg = 180 + (180.0 * p);
         if (rev) deg = -deg;
         evas_map_util_3d_rotate(mf, 0.0, deg, deg, cx, cy, 0);
         evas_map_util_3d_rotate(mb, 0.0, 180.0 + deg, 180.0 + deg, cx, cy, 0);
         break;
      case ELM_FLIP_CUBE_LEFT:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
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
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
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
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
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
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
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
}

static Eina_Bool
_flip(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double t = ecore_loop_time_get() - wd->start;
   if (!wd) return ECORE_CALLBACK_CANCEL;
   if (!wd->animator) return ECORE_CALLBACK_CANCEL;
   
   t = t / wd->len;
   if (t > 1.0) t = 1.0;
   
   _flip_do(obj, t, wd->mode, 0, 0);

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
   Evas_Coord fsize;
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
   
   if (wd->event[0])
     {
        fsize = 16; // FIXME: 16?
        elm_coords_finger_size_adjust(0, NULL, 1, &fsize);
        evas_object_move(wd->event[0], x, y);
        evas_object_resize(wd->event[0], w, fsize);
     }
   if (wd->event[1])
     {
        fsize = 16; // FIXME: 16?
        elm_coords_finger_size_adjust(0, NULL, 1, &fsize);
        evas_object_move(wd->event[1], x, y + h - fsize);
        evas_object_resize(wd->event[1], w, fsize);
     }
   if (wd->event[2])
     {
        fsize = 16; // FIXME: 16?
        elm_coords_finger_size_adjust(1, &fsize, 0, NULL);
        evas_object_move(wd->event[2], x, y);
        evas_object_resize(wd->event[2], fsize, h);
     }
   if (wd->event[3])
     {
        fsize = 16; // FIXME: 16?
        elm_coords_finger_size_adjust(1, &fsize, 0, NULL);
        evas_object_move(wd->event[3], x + w - fsize, y);
        evas_object_resize(wd->event[3], fsize, h);
     }
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

static double
_pos_get(Widget_Data *wd, int *rev, Elm_Flip_Mode *m)
{
   Evas_Coord x, y, w, h;
   double t = 1.0;
   
   evas_object_geometry_get(wd->obj, &x, &y, &w, &h);
   switch (wd->intmode)
     {
      case ELM_FLIP_INTERACTION_ROTATE:
      case ELM_FLIP_INTERACTION_CUBE:
          {
             if (wd->dir == 0)
               {
                  if (wd->down_x > 0)
                     t = 1.0 - ((double)wd->x / (double)wd->down_x);
                  *rev = 1;
               }
             else if (wd->dir == 1)
               {
                  if (wd->down_x < w)
                     t = 1.0 - ((double)(w - wd->x) / (double)(w - wd->down_x));
               }
             else if (wd->dir == 2)
               {
                  if (wd->down_y > 0)
                     t = 1.0 - ((double)wd->y / (double)wd->down_y);
               }
             else if (wd->dir == 3)
               {
                  if (wd->down_y < h)
                     t = 1.0 - ((double)(h - wd->y) / (double)(h - wd->down_y));
                  *rev = 1;
               }
             
             if (t < 0.0) t = 0.0;
             else if (t > 1.0) t = 1.0;
             
             if ((wd->dir == 0) || (wd->dir == 1))
               {
                  if (wd->intmode == ELM_FLIP_INTERACTION_ROTATE) 
                     *m = ELM_FLIP_ROTATE_Y_CENTER_AXIS;
                  else if (wd->intmode == ELM_FLIP_INTERACTION_CUBE)
                    {
                       if (*rev)
                          *m = ELM_FLIP_CUBE_LEFT;
                       else
                          *m = ELM_FLIP_CUBE_RIGHT;
                    }
               }
             else
               {
                  if (wd->intmode == ELM_FLIP_INTERACTION_ROTATE) 
                     *m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
                  else if (wd->intmode == ELM_FLIP_INTERACTION_CUBE)
                    {
                       if (*rev)
                          *m = ELM_FLIP_CUBE_UP;
                       else
                          *m = ELM_FLIP_CUBE_DOWN;
                    }
               }
          }
      default:
        break;
     }
   return t;
}

static Eina_Bool
_event_anim(void *data, double pos)
{
   Widget_Data *wd = data;
   double p;
   
   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);
   if (wd->finish)
     {
        if (wd->dir == 0)
           wd->x = wd->ox * (1.0 - p);
        else if (wd->dir == 1)
           wd->x = wd->ox + ((wd->w - wd->ox) * p);
        else if (wd->dir == 2)
           wd->y = wd->oy * (1.0 - p);
        else if (wd->dir == 3)
           wd->y = wd->oy + ((wd->h - wd->oy) * p);
     }
   else
     {
        if (wd->dir == 0)
           wd->x = wd->ox + ((wd->w - wd->ox) * p);
        else if (wd->dir == 1)
           wd->x = wd->ox * (1.0 - p);
        else if (wd->dir == 2)
           wd->y = wd->oy + ((wd->h - wd->oy) * p);
        else if (wd->dir == 3)
           wd->y = wd->oy * (1.0 - p);
     }
   switch (wd->intmode)
     {
      case ELM_FLIP_INTERACTION_NONE:
        break;
      case ELM_FLIP_INTERACTION_ROTATE:
      case ELM_FLIP_INTERACTION_CUBE:
          {
             Elm_Flip_Mode m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
             int rev = 0;
             p = _pos_get(wd, &rev, &m);
             _flip_do(wd->obj, p, m, 1, rev);
          }
        break;
      case ELM_FLIP_INTERACTION_PAGE:
        /*
         _state_update(st);
         */
        break;
      default:
        break;
     }
   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   evas_object_map_enable_set(wd->front.content, 0);
   evas_object_map_enable_set(wd->back.content, 0);
   // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow
   evas_object_resize(wd->front.content, 0, 0);
   evas_object_resize(wd->back.content, 0, 0);
   evas_smart_objects_calculate(evas_object_evas_get(wd->obj));
   // FIXME: end hack
   wd->animator = NULL;
   if (wd->finish) wd->state = !wd->state;
   _configure(wd->obj);
   wd->animator = NULL;
   evas_object_smart_callback_call(wd->obj, SIG_ANIMATE_DONE, NULL);
   
   return ECORE_CALLBACK_CANCEL;
}

static void
_update_job(void *data)
{
   Widget_Data *wd = data;
   double p;
   Elm_Flip_Mode m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
   int rev = 0;
   
   wd->job = NULL;
   switch (wd->intmode)
     {
      case ELM_FLIP_INTERACTION_ROTATE:
      case ELM_FLIP_INTERACTION_CUBE:
        p = _pos_get(wd, &rev, &m);
        _flip_do(wd->obj, p, m, 1, rev);
        break;
      case ELM_FLIP_INTERACTION_PAGE:
        /*   
         if (_state_update(st))
         {
         evas_object_hide(st->front);
         evas_object_hide(st->back);
         }
         */
        break;
      default:
        break;
     }
}

static void
_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *fl = data;
   Widget_Data *wd = elm_widget_data_get(fl);
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;
   
   if (!wd) return;
   if (ev->button != 1) return;
   if (wd->animator)
     {
        ecore_animator_del(wd->animator);
        wd->animator = NULL;
     }
   wd->down = EINA_TRUE;
   wd->started = EINA_FALSE;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   wd->x = ev->canvas.x - x;
   wd->y = ev->canvas.y - y;
   wd->w = w;                
   wd->h = h;
   wd->down_x = wd->x;
   wd->down_y = wd->y;
   printf("dn %i %i\n", wd->x, wd->y);
}

static void
_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *fl = data;
   Widget_Data *wd = elm_widget_data_get(fl);
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord x, y, w, h;
   double tm = 0.5;
   
   if (!wd) return;
   if (ev->button != 1) return;
   wd->down = 0;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   wd->x = ev->canvas.x - x;
   wd->y = ev->canvas.y - y;
   wd->w = w;
   wd->h = h;
   wd->ox = wd->x;
   wd->oy = wd->y;
   printf("up %i %i\n", wd->x, wd->y);
   if (wd->job)
     {
        ecore_job_del(wd->job);
        wd->job = NULL;
     }
   wd->finish = EINA_FALSE;
   if (wd->dir == 0)
     {
        tm = (double)wd->x / (double)wd->w;
        if (wd->x < (wd->w / 2)) wd->finish = EINA_TRUE;
     }
   else if (wd->dir == 1)
     {
        if (wd->x > (wd->w / 2)) wd->finish = EINA_TRUE;
        tm = 1.0 - ((double)wd->x / (double)wd->w);
     }
   else if (wd->dir == 2)
     {
        if (wd->y < (wd->h / 2)) wd->finish = EINA_TRUE;
        tm = (double)wd->y / (double)wd->h;
     }
   else if (wd->dir == 3)
     {
        if (wd->y > (wd->h / 2)) wd->finish = EINA_TRUE;
        tm = 1.0 - ((double)wd->y / (double)wd->h);
     }
   if (tm < 0.01) tm = 0.01;
   else if (tm > 0.99) tm = 0.99;
   if (!wd->finish) tm = 1.0 - tm;
   tm *= 1.0; // FIXME: config for anim time
   if (wd->animator) ecore_animator_del(wd->animator);
   wd->animator = ecore_animator_timeline_add(tm, _event_anim, wd);
   _event_anim(wd, 0.0);
}

static void
_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *fl = data;
   Widget_Data *wd = elm_widget_data_get(fl);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x, y, w, h;
   
   if (!wd) return;
   if (!wd->down) return;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   wd->x = ev->cur.canvas.x - x;
   wd->y = ev->cur.canvas.y - y;
   wd->w = w;                
   wd->h = h;
   if (!wd->started)
     {
        Evas_Coord dx, dy;
        
        dx = wd->x - wd->down_x;
        dy = wd->y - wd->down_y;
        if (((dx * dx) + (dy * dy)) > (_elm_config->finger_size * _elm_config->finger_size / 4))
          {
             wd->dir = 0;
             if      ((wd->x > (w / 2)) && (dx <  0) && (abs(dx) >  abs(dy))) wd->dir = 0; // left
             else if ((wd->x < (w / 2)) && (dx >= 0) && (abs(dx) >  abs(dy))) wd->dir = 1; // right
             else if ((wd->y > (h / 2)) && (dy <  0) && (abs(dy) >= abs(dx))) wd->dir = 2; // up
             else if ((wd->y < (h / 2)) && (dy >= 0) && (abs(dy) >= abs(dx))) wd->dir = 3; // down
             wd->started = EINA_TRUE;
             printf("START\n");
             flip_show_hide(data);
             evas_smart_objects_calculate(evas_object_evas_get(data));
             _flip(data);
             // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow)
             evas_object_map_enable_set(wd->front.content, 0);
             evas_object_map_enable_set(wd->back.content, 0);
//             evas_object_resize(wd->front.content, 0, 0);
//             evas_object_resize(wd->back.content, 0, 0);
             evas_smart_objects_calculate(evas_object_evas_get(data));
             _configure(obj);
             // FIXME: end hack
          }
        else return;
     }
   printf("mv %i %i\n", wd->x, wd->y);
   if (wd->job) ecore_job_del(wd->job);
   wd->job = ecore_job_add(_update_job, wd);
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

   wd->obj = obj;
   
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
   wd->intmode = ELM_FLIP_INTERACTION_NONE;
   
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
   int i;
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
   if (wd->intmode != ELM_FLIP_INTERACTION_NONE)
     {
        for (i = 0; i < 4; i++) evas_object_raise(wd->event[i]);
     }
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
   int i;
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
   if (wd->intmode != ELM_FLIP_INTERACTION_NONE)
     {
        for (i = 0; i < 4; i++) evas_object_raise(wd->event[i]);
     }
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

// FIXME: add ambient and lighting control

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

EAPI void
elm_flip_interaction_set(Evas_Object *obj, Elm_Flip_Interaction mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->intmode == mode) return;
   wd->intmode = mode;
}

EAPI Elm_Flip_Interaction
elm_flip_interaction_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_FLIP_INTERACTION_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_FLIP_INTERACTION_NONE;
   return wd->intmode;
}

EAPI void
elm_flip_interacton_direction_enabled_set(Evas_Object *obj, Elm_Flip_Direction dir, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   int i = -1;
   if (!wd) return;
   enabled = !!enabled;
   if      (dir == ELM_FLIP_DIRECTION_UP)    i = 0;
   else if (dir == ELM_FLIP_DIRECTION_DOWN)  i = 1;
   else if (dir == ELM_FLIP_DIRECTION_LEFT)  i = 2;
   else if (dir == ELM_FLIP_DIRECTION_RIGHT) i = 3;
   if (i < 0) return;
   if (wd->dir_enabled[i] == enabled) return;
   wd->dir_enabled[i] = enabled;
   if (wd->dir_enabled[i]) 
     {
        wd->event[i] = evas_object_rectangle_add(evas_object_evas_get(obj));
        elm_widget_sub_object_add(obj, wd->event[i]);
        evas_object_clip_set(wd->event[i], evas_object_clip_get(obj));
        evas_object_color_set(wd->event[i], 0, 0, 0, 0);
        evas_object_show(wd->event[i]);
        evas_object_smart_member_add(wd->event[i], obj);
        evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_DOWN, _down_cb, obj);
        evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_UP, _up_cb, obj);
        evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_MOVE, _move_cb, obj);
     }
   else
     {
        evas_object_del(wd->event[i]);
        wd->event[i] = NULL;
     }
   _sizing_eval(obj);
}

EAPI Eina_Bool
elm_flip_interacton_direction_enabled_get(Evas_Object *obj, Elm_Flip_Direction dir)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   int i = -1;
   if (!wd) return EINA_FALSE;
   if      (dir == ELM_FLIP_DIRECTION_UP)    i = 0;
   else if (dir == ELM_FLIP_DIRECTION_DOWN)  i = 1;
   else if (dir == ELM_FLIP_DIRECTION_LEFT)  i = 2;
   else if (dir == ELM_FLIP_DIRECTION_RIGHT) i = 3;
   if (i < 0) return EINA_FALSE;
   return wd->dir_enabled[i];
}
