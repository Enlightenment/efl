#include "evas_common_private.h"
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
        Evas_Layer *l = (Evas_Layer *)(EINA_INLIST_GET(obj->layer))->prev;

        for (; l; l = (Evas_Layer *)(EINA_INLIST_GET(l))->prev)
          {
             if (l->objects)
               return (Evas_Object_Protected_Data *)((EINA_INLIST_GET((l->objects)))->last);
          }
     }
   return NULL;
}

EOLIAN void
_evas_object_raise(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (evas_object_intercept_call_raise(eo_obj, obj)) return;

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
                                               1, 1) && obj->cur->visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
}

EOLIAN void
_evas_object_lower(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (evas_object_intercept_call_lower(eo_obj, obj)) return;

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
                                               1, 1) && obj->cur->visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
}

EOLIAN void
_evas_object_stack_above(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object *eo_above)
{
   if (!eo_above)
     {
        evas_object_raise(eo_obj);
        return;
     }
   if (eo_obj == eo_above) return;
   if (evas_object_intercept_call_stack_above(eo_obj, obj, eo_above)) return;
   Evas_Object_Protected_Data *above = eo_data_scope_get(eo_above, EVAS_OBJ_CLASS);
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
                                               1, 1) && obj->cur->visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
}

EOLIAN void
_evas_object_stack_below(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object *eo_below)
{
   if (!eo_below)
     {
        evas_object_lower(eo_obj);
        return;
     }
   if (eo_obj == eo_below) return;
   if (evas_object_intercept_call_stack_below(eo_obj, obj, eo_below)) return;
   Evas_Object_Protected_Data *below = eo_data_scope_get(eo_below, EVAS_OBJ_CLASS);
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
                                               1, 1) && obj->cur->visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
}

EOLIAN Evas_Object *
_evas_object_above_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->smart.parent)
     {
        do
          {
             obj = (Evas_Object_Protected_Data *)((EINA_INLIST_GET(obj))->next);
             if ((obj) && (!obj->delete_me)) return obj->object;
          }
        while (obj);
        return NULL;
     }
   obj = evas_object_above_get_internal(obj);
   while (obj)
     {
        if (!obj->delete_me) return obj->object;
        obj = evas_object_above_get_internal(obj);
     }
   return NULL;
}

EOLIAN Evas_Object *
_evas_object_below_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->smart.parent)
     {
        do
          {
             obj = (Evas_Object_Protected_Data *)((EINA_INLIST_GET(obj))->prev);
             if ((obj) && (!obj->delete_me)) return obj->object;
          }
        while (obj);
        return NULL;
     }
   obj = evas_object_below_get_internal(obj);
   while (obj)
     {
        if (!obj->delete_me) return obj->object;
        obj = evas_object_below_get_internal(obj);
     }
   return NULL;
}

EOLIAN Evas_Object*
_evas_object_bottom_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   if (e->layers)
     {
        Evas_Object_Protected_Data *obj;

        obj = e->layers->objects;
        while (obj)
          {
             if (!obj->delete_me) return obj->object;
             obj = evas_object_above_get_internal(obj);
          }
     }
   return NULL;
}

EOLIAN Evas_Object*
_evas_object_top_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   Evas_Object_Protected_Data *obj = NULL;
   Eina_Inlist *list;
   Evas_Layer *layer;

   list = EINA_INLIST_GET(e->layers);
   if (!list) return NULL;

   layer = (Evas_Layer *) list->last;
   if (!layer) return NULL;

   list = EINA_INLIST_GET(layer->objects);
   if (!list) return NULL;

   obj = (Evas_Object_Protected_Data *) list->last;
   if (!obj) return NULL;

   while (obj)
     {
        if (!obj->delete_me) return obj->object;
        obj = evas_object_below_get_internal(obj);
     }

   return NULL;
}

