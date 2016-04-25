#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static Evas_Object *target = NULL;
static Ecore_Animator *anim = NULL;

static Eina_List *mirrors = NULL;

static Eina_Bool
_cb_anim(void *data EINA_UNUSED, double pos)
{
   Eina_List *l;
   Evas_Object *o;
   double v = ecore_animator_pos_map(pos, ECORE_POS_MAP_SINUSOIDAL, 0.0, 0.0);
   Evas_Coord x, y, w, h;
   Evas_Coord vw, vh;
   int tot;

   evas_output_viewport_get(evas_object_evas_get(target), NULL, NULL, &vw, &vh);
   evas_object_geometry_get(target, &x, &y, &w, &h);
   tot = (eina_list_count(mirrors) + 2) / 2;
   EINA_LIST_FOREACH(mirrors, l, o)
     {
        Evas_Map *m = evas_map_new(4);
        int n = (int)((uintptr_t)evas_object_data_get(o, "stack"));
        double ang = (((double)(n + 1) * 20.0) / (double)tot);
        evas_map_util_points_populate_from_object(m, o);
        evas_map_util_3d_rotate(m, ang * v / 2.0, ang * v, 0,
                                x + w + (w / 2), y + (h / 2), 0);
        evas_map_util_3d_perspective(m, vw / 2, vh / 2, 0, vw + vh);
        evas_object_map_set(o, m);
        evas_map_free(m);
        evas_object_map_enable_set(o, EINA_TRUE);
     }
   if (pos >= 1.0) anim = NULL;
   return EINA_TRUE;
}

static int
explode_obj(Evas_Object *obj, int n)
{
   Eina_List *children = NULL;
   Evas_Object *o;
   Evas *e;

   e = evas_object_evas_get(obj);
   children = evas_object_smart_members_get(obj);
   if (!children)
     {
        Evas_Object *m;
        Evas_Coord x, y, w, h;

        evas_object_geometry_get(obj, &x, &y, &w, &h);

        m = evas_object_image_filled_add(e);
        mirrors = eina_list_append(mirrors, m);
        evas_object_anti_alias_set(m, EINA_TRUE);
        evas_object_image_source_clip_set(m, EINA_FALSE);
        evas_object_pass_events_set(m, EINA_TRUE);
        evas_object_image_source_set(m, obj);
        evas_object_image_source_visible_set(m, EINA_FALSE);
        evas_object_layer_set(m, EVAS_LAYER_MAX - 10);
        evas_object_move(m, x, y);
        evas_object_resize(m, w, h);
        evas_object_show(m);
        evas_object_color_set(m, 0, 0, 0, 128);
        evas_object_data_set(m, "stack", (void *)((uintptr_t)n));

        n++;

        m = evas_object_image_filled_add(e);
        mirrors = eina_list_append(mirrors, m);
        evas_object_anti_alias_set(m, EINA_TRUE);
        evas_object_image_source_clip_set(m, EINA_FALSE);
        evas_object_pass_events_set(m, EINA_TRUE);
        evas_object_image_source_set(m, obj);
        evas_object_image_source_visible_set(m, EINA_FALSE);
        evas_object_layer_set(m, EVAS_LAYER_MAX - 10);
        evas_object_move(m, x, y);
        evas_object_resize(m, w, h);
        evas_object_show(m);
        evas_object_data_set(m, "stack", (void *)((uintptr_t)n));
     }
   else
     {
        EINA_LIST_FREE(children, o)
          {
             if (!evas_object_visible_get(o)) continue;
             if (evas_object_clipees_has(o)) continue;

             n = explode_obj(o, n);
          }
     }
   return n;
}

static void
explode(Evas_Object *obj)
{
   Evas_Object *o;

   EINA_LIST_FREE(mirrors, o) evas_object_del(o);
   if (!obj) return;

   explode_obj(obj, 0);
   if (!anim) anim = ecore_animator_timeline_add(1.0, _cb_anim, NULL);
}

static void
_cb_target_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
               Evas_Object *obj, void *info EINA_UNUSED)
{
   if (obj == target)
     {
        if (anim)
          {
             ecore_animator_del(anim);
             anim = NULL;
          }
        explode(NULL);
        target = NULL;
     }
}

static void
_cb_catcher_down(void *data, Evas *e,
                 Evas_Object *obj EINA_UNUSED, void *info)
{
   Evas_Event_Mouse_Down *ev = info;

   if (target)
     {
        evas_object_event_callback_del_full(target, EVAS_CALLBACK_DEL,
                                            _cb_target_del, data);
        if (anim)
          {
             ecore_animator_del(anim);
             anim = NULL;
          }
        explode(NULL);
        target = NULL;
     }
   if ((ev->button == 2) &&
       (evas_key_modifier_is_set(ev->modifiers, "Control")))
     {
        Eina_List *objs;

        objs = evas_tree_objects_at_xy_get(e, NULL, ev->canvas.x, ev->canvas.y);
        if (objs)
          {
             Eina_List *l;
             Evas_Object *lobj, *parent;

             target = NULL;
             EINA_LIST_FOREACH(objs, l, lobj)
               {
                  if (evas_object_data_get(lobj, "::catcher::")) continue;
                  if (evas_object_repeat_events_get(lobj))
                    {
                       int r, g, b, a;
                       evas_object_color_get(lobj, &r, &g, &b, &a);
                       if ((r == 0) && (g == 0) && (b == 0) && (a == 0)) continue;
                    }
                  if (elm_object_widget_check(lobj))
                    {
                       target = lobj;
                       break;
                    }
                  else
                    {
                       for (parent = lobj; parent != NULL;
                            parent = elm_object_parent_widget_get(parent))
                         {
                            if (elm_object_widget_check(parent))
                              {
                                 target = parent;
                                 break;
                              }
                         }
                       if (target) break;
                    }
               }
             if (target)
               {
                  evas_object_event_callback_add(target, EVAS_CALLBACK_DEL,
                                                 _cb_target_del, data);
                  eina_list_free(objs);
                  explode(target);
               }
          }
     }
}

void
explode_win_enable(Evas_Object *win)
{
   Evas *e = evas_object_evas_get(win);

   Evas_Object *catcher = evas_object_rectangle_add(e);
   evas_object_layer_set(catcher, EVAS_LAYER_MAX);
   evas_object_color_set(catcher, 0, 0, 0, 0);
   evas_object_resize(catcher, 99999, 99999);
   evas_object_repeat_events_set(catcher, EINA_TRUE);
   evas_object_data_set(catcher, "::catcher::", catcher);
   evas_object_show(catcher);

   evas_object_event_callback_add(catcher, EVAS_CALLBACK_MOUSE_DOWN,
                                  _cb_catcher_down, win);
}
