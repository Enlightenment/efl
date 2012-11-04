#include "evas_common.h"
#include "evas_private.h"

static Evas_Object_Protected_Data *
evas_object_above_get_internal(const Evas_Object_Protected_Data *obj)
{
   if ((EINA_INLIST_GET(obj))->next)
     return (Evas_Object_Protected_Data *)((EINA_INLIST_GET(obj))->next);
   else
     {
        if ((EINA_INLIST_GET(obj->layer))->next)
          {
             Evas_Layer *l;

             l = (Evas_Layer *)((EINA_INLIST_GET(obj->layer))->next);
             return l->objects;
          }
     }
   return NULL;
}

static Evas_Object_Protected_Data *
evas_object_below_get_internal(const Evas_Object_Protected_Data *obj)
{
   if ((EINA_INLIST_GET(obj))->prev)
     return (Evas_Object_Protected_Data *)((EINA_INLIST_GET(obj))->prev);
   else
     {
        if ((EINA_INLIST_GET(obj->layer))->prev)
          {
             Evas_Layer *l;

             l = (Evas_Layer *)((EINA_INLIST_GET(obj->layer))->prev);
             return (Evas_Object_Protected_Data *)((EINA_INLIST_GET((l->objects)))->last);
          }
     }
   return NULL;
}

EAPI void
evas_object_raise(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_raise());
}

void
_raise(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
   if (evas_object_intercept_call_raise(eo_obj)) return;

   Evas_Object_Protected_Data *obj = _pd;
   if (!((EINA_INLIST_GET(obj))->next))
     {
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   if (obj->smart.parent)
     evas_object_smart_member_raise(eo_obj);
   else
     {
        if (obj->in_layer)
          obj->layer->objects = (Evas_Object_Protected_Data *)eina_inlist_demote(EINA_INLIST_GET(obj->layer->objects), EINA_INLIST_GET(obj));
     }
   if (obj->clip.clipees)
     {
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas->evas);
   obj->restack = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_inform_call_restack(eo_obj);
   if (obj->layer->evas->is_frozen) return;
   if ((!evas_event_passes_through(eo_obj, obj)) &&
       (!evas_event_freezes_through(eo_obj, obj)) &&
       (!evas_object_is_source_invisible(eo_obj, obj)))
     {
        if (!obj->is_smart)
          {
             if (evas_object_is_in_output_rect(eo_obj, obj,
                                               obj->layer->evas->pointer.x,
                                               obj->layer->evas->pointer.y,
                                               1, 1) && obj->cur.visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
}

EAPI void
evas_object_lower(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_lower());
}

void
_lower(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
   if (evas_object_intercept_call_lower(eo_obj)) return;

   Evas_Object_Protected_Data *obj = _pd;
   if (!((EINA_INLIST_GET(obj))->prev))
     {
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   if (obj->smart.parent)
     evas_object_smart_member_lower(eo_obj);
   else
     {
        if (obj->in_layer)
          obj->layer->objects = (Evas_Object_Protected_Data *)eina_inlist_promote(EINA_INLIST_GET(obj->layer->objects),
                                                                                 EINA_INLIST_GET(obj));
     }
   if (obj->clip.clipees)
     {
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas->evas);
   obj->restack = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_inform_call_restack(eo_obj);
   if (obj->layer->evas->is_frozen) return;
   if ((!evas_event_passes_through(eo_obj, obj)) &&
       (!evas_event_freezes_through(eo_obj, obj)) &&
       (!evas_object_is_source_invisible(eo_obj, obj)))
     {
        if (!obj->is_smart)
          {
             if (evas_object_is_in_output_rect(eo_obj, obj,
                                               obj->layer->evas->pointer.x,
                                               obj->layer->evas->pointer.y,
                                               1, 1) && obj->cur.visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
}

EAPI void
evas_object_stack_above(Evas_Object *eo_obj, Evas_Object *above)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_stack_above(above));
}

void
_stack_above(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object *eo_above = va_arg(*list, Evas_Object *);
   if (!eo_above) return;
   if (eo_obj == eo_above) return;
   if (evas_object_intercept_call_stack_above(eo_obj, eo_above)) return;
   if (!eo_above)
     {
        evas_object_raise(eo_obj);
        return;
     }
   Evas_Object_Protected_Data *obj = _pd;
   Evas_Object_Protected_Data *above = eo_data_get(eo_above, EVAS_OBJ_CLASS);
   if ((EINA_INLIST_GET(obj))->prev == EINA_INLIST_GET(above))
     {
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   if (obj->smart.parent)
     {
        if (obj->smart.parent != above->smart.parent)
          {
             ERR("BITCH! evas_object_stack_above(), %p not inside same smart as %p!", eo_obj, eo_above);
             return;
          }
        evas_object_smart_member_stack_above(eo_obj, eo_above);
     }
   else
     {
        if (above->smart.parent)
          {
             ERR("BITCH! evas_object_stack_above(), %p stack above %p, but above has smart parent, obj does not", eo_obj, eo_above);
             return;
          }
        if (obj->layer != above->layer)
          {
             ERR("BITCH! evas_object_stack_above(), %p stack above %p, not matching layers", eo_obj, eo_above);
             return;
          }
        if (obj->in_layer)
          {
             obj->layer->objects = (Evas_Object_Protected_Data *)eina_inlist_remove(EINA_INLIST_GET(obj->layer->objects),
                                                                                   EINA_INLIST_GET(obj));
             obj->layer->objects = (Evas_Object_Protected_Data *)eina_inlist_append_relative(EINA_INLIST_GET(obj->layer->objects),
                                                                                            EINA_INLIST_GET(obj),
                                                                                            EINA_INLIST_GET(above));
          }
     }
   if (obj->clip.clipees)
     {
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas->evas);
   obj->restack = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_inform_call_restack(eo_obj);
   if (obj->layer->evas->is_frozen) return;
   if ((!evas_event_passes_through(eo_obj, obj)) &&
       (!evas_event_freezes_through(eo_obj, obj)) &&
       (!evas_object_is_source_invisible(eo_obj, obj)))
     {
        if (!obj->is_smart)
          {
             if (evas_object_is_in_output_rect(eo_obj, obj,
                                               obj->layer->evas->pointer.x,
                                               obj->layer->evas->pointer.y,
                                               1, 1) && obj->cur.visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
}

EAPI void
evas_object_stack_below(Evas_Object *eo_obj, Evas_Object *eo_below)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_stack_below(eo_below));
}

void
_stack_below(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object *eo_below = va_arg(*list, Evas_Object *);
   if (!eo_below) return;
   if (eo_obj == eo_below) return;
   if (evas_object_intercept_call_stack_below(eo_obj, eo_below)) return;
   if (!eo_below)
     {
        evas_object_lower(eo_obj);
        return;
     }
   Evas_Object_Protected_Data *obj = _pd;
   Evas_Object_Protected_Data *below = eo_data_get(eo_below, EVAS_OBJ_CLASS);
   if ((EINA_INLIST_GET(obj))->next == EINA_INLIST_GET(below))
     {
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   if (obj->smart.parent)
     {
        if (obj->smart.parent != below->smart.parent)
          {
             ERR("BITCH! evas_object_stack_below(), %p not inside same smart as %p!", eo_obj, eo_below);
             return;
          }
        evas_object_smart_member_stack_below(eo_obj, eo_below);
     }
   else
     {
        if (below->smart.parent)
          {
             ERR("BITCH! evas_object_stack_below(), %p stack below %p, but below has smart parent, obj does not", eo_obj, eo_below);
             return;
          }
        if (obj->layer != below->layer)
          {
             ERR("BITCH! evas_object_stack_below(), %p stack below %p, not matching layers", eo_obj, eo_below);
             return;
          }
        if (obj->in_layer)
          {
             obj->layer->objects = (Evas_Object_Protected_Data *)eina_inlist_remove(EINA_INLIST_GET(obj->layer->objects),
                                                                     EINA_INLIST_GET(obj));
             obj->layer->objects = (Evas_Object_Protected_Data *)eina_inlist_prepend_relative(EINA_INLIST_GET(obj->layer->objects),
                                                                               EINA_INLIST_GET(obj),
                                                                               EINA_INLIST_GET(below));
          }
     }
   if (obj->clip.clipees)
     {
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas->evas);
   obj->restack = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_inform_call_restack(eo_obj);
   if (obj->layer->evas->is_frozen) return;
   if ((!evas_event_passes_through(eo_obj, obj)) &&
       (!evas_event_freezes_through(eo_obj, obj)) &&
       (!evas_object_is_source_invisible(eo_obj, obj)))
     {
        if (!obj->is_smart)
          {
             if (evas_object_is_in_output_rect(eo_obj, obj,
                                               obj->layer->evas->pointer.x,
                                               obj->layer->evas->pointer.y,
                                               1, 1) && obj->cur.visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
}

EAPI Evas_Object *
evas_object_above_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *ret = NULL;
   eo_do((Eo *)eo_obj, evas_obj_above_get(&ret));
   return ret;
}

void
_above_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Protected_Data *obj = _pd;
   Evas_Object **ret = va_arg(*list, Evas_Object **);

   if (obj->smart.parent)
     {
        do
          {
             obj = (Evas_Object_Protected_Data *)((EINA_INLIST_GET(obj))->next);
             if ((obj) && (!obj->delete_me))
               {
                  *ret = obj->object;
                  return;
               }
          }
        while (obj);
        *ret = NULL;
        return;
     }
   obj = evas_object_above_get_internal(obj);
   while (obj)
     {
        if (!obj->delete_me)
          {
             *ret = obj->object;
             return;
          }
        obj = evas_object_above_get_internal(obj);
     }
   *ret = NULL;
}

EAPI Evas_Object *
evas_object_below_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *ret = NULL;
   eo_do((Eo *)eo_obj, evas_obj_below_get(&ret));
   return ret;
}

void
_below_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Protected_Data *obj = _pd;
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;
   if (obj->smart.parent)
     {
        do
          {
             obj = (Evas_Object_Protected_Data *)((EINA_INLIST_GET(obj))->prev);
             if ((obj) && (!obj->delete_me))
               {
                  *ret = obj->object;
                  return;
               }
          }
        while (obj);
        return;
     }
   obj = evas_object_below_get_internal(obj);
   while (obj)
     {
        if (!obj->delete_me)
          {
             *ret = obj->object;
             return;
          }
        obj = evas_object_below_get_internal(obj);
     }
}



EAPI Evas_Object *
evas_object_bottom_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *ret = NULL;
   eo_do((Eo *)e, evas_canvas_object_bottom_get(&ret));
   return ret;
}

void
_canvas_object_bottom_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   const Evas_Public_Data *e = _pd;
   if (e->layers)
     {
        Evas_Object_Protected_Data *obj;

        obj = e->layers->objects;
        while (obj)
          {
             if (!obj->delete_me)
               {
                  *ret = obj->object;
                  return;
               }
             obj = evas_object_above_get_internal(obj);
          }
     }
   *ret = NULL;
}

EAPI Evas_Object *
evas_object_top_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *ret = NULL;
   eo_do((Eo *)eo_e, evas_canvas_object_top_get(&ret));
   return ret;
}

void
_canvas_object_top_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *params_list)
{
   Evas_Object **ret = va_arg(*params_list, Evas_Object **);
   *ret = NULL;
   Evas_Object_Protected_Data *obj = NULL;
   Eina_Inlist *list;
   Evas_Layer *layer;

   const Evas_Public_Data *e = _pd;
   list = EINA_INLIST_GET(e->layers);
   if (!list) return;

   layer = (Evas_Layer *) list->last;
   if (!layer) return;

   list = EINA_INLIST_GET(layer->objects);
   if (!list) return;

   obj = (Evas_Object_Protected_Data *) list->last;
   if (!obj) return;

   while (obj)
     {
        if (!obj->delete_me)
          {
             *ret = obj->object;
             return;
          }
        obj = evas_object_below_get_internal(obj);
     }

   *ret = obj->object;
}
