static Evas_Object_Protected_Data *
evas_object_above_get_internal(const Evas_Object_Protected_Data *obj)
{
   if ((EINA_INLIST_GET(obj))->next)
     return (Evas_Object_Protected_Data *)((EINA_INLIST_GET(obj))->next);
   else
     {
        Evas_Layer *l = (Evas_Layer *)(EINA_INLIST_GET(obj->layer))->next;

        for (; l; l = (Evas_Layer *)(EINA_INLIST_GET(l))->next)
          {
             if (l->objects)
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

EAPI void
evas_object_raise(Evas_Object *obj)
{
   efl_gfx_stack_raise_to_top((Evas_Object *)obj);
}

EOLIAN void
_efl_canvas_object_efl_gfx_stack_raise_to_top(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_RAISE, 1))
     return;

   if (!((EINA_INLIST_GET(obj))->next))
     {
        evas_object_inform_call_restack(eo_obj, obj);
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
        evas_object_inform_call_restack(eo_obj, obj);
        return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas->evas);
   obj->restack = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_inform_call_restack(eo_obj, obj);
   if (!obj->layer || obj->layer->evas->is_frozen) return;
   if ((!evas_event_passes_through(eo_obj, obj)) &&
       (!evas_event_freezes_through(eo_obj, obj)) &&
       (!evas_object_is_source_invisible(eo_obj, obj)))
     {
        if (!obj->is_smart && obj->cur->visible)
          {
             _evas_canvas_event_pointer_in_rect_mouse_move_feed(obj->layer->evas,
                                                                eo_obj,
                                                                obj, 1, 1,
                                                                EINA_FALSE,
                                                                NULL);
          }
     }
}

EAPI void
evas_object_lower(Evas_Object *obj)
{
   efl_gfx_stack_lower_to_bottom((Evas_Object *)obj);
}

EOLIAN void
_efl_canvas_object_efl_gfx_stack_lower_to_bottom(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_LOWER, 1))
     return;

   if (!((EINA_INLIST_GET(obj))->prev))
     {
        evas_object_inform_call_restack(eo_obj, obj);
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
        evas_object_inform_call_restack(eo_obj, obj);
        return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas->evas);
   obj->restack = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_inform_call_restack(eo_obj, obj);
   if (!obj->layer || obj->layer->evas->is_frozen) return;
   if ((!evas_event_passes_through(eo_obj, obj)) &&
       (!evas_event_freezes_through(eo_obj, obj)) &&
       (!evas_object_is_source_invisible(eo_obj, obj)))
     {
        if (!obj->is_smart && obj->cur->visible)
          {
             _evas_canvas_event_pointer_in_rect_mouse_move_feed(obj->layer->evas,
                                                                eo_obj,
                                                                obj, 1, 1,
                                                                EINA_FALSE,
                                                                NULL);
          }
     }
}

EAPI void
evas_object_stack_above(Evas_Object *obj, Evas_Object *above)
{
   efl_gfx_stack_above((Evas_Object *)obj, above);
}

#define SERR(x, ...) EINA_SAFETY_ERROR(eina_slstr_printf(x, __VA_ARGS__))

EOLIAN void
_efl_canvas_object_efl_gfx_stack_stack_above(Eo *eo_obj, Evas_Object_Protected_Data *obj, Efl_Gfx_Stack *eo_above)
{
   if (!eo_above)
     {
        evas_object_raise(eo_obj);
        return;
     }
   if (eo_obj == eo_above) return;
   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_STACK_ABOVE, 1, eo_above)) return;
   Evas_Object_Protected_Data *above = efl_data_scope_get(eo_above, EFL_CANVAS_OBJECT_CLASS);
   if ((EINA_INLIST_GET(obj))->prev == EINA_INLIST_GET(above))
     {
        evas_object_inform_call_restack(eo_obj, obj);
        return;
     }
   if (obj->smart.parent)
     {
        if (obj->smart.parent != above->smart.parent)
          {
             SERR("Invalid operation: object '%s' %p (parent: '%s' %p) not "
                  "inside same smart parent as above '%s' %p (parent '%s' %p)!",
                  efl_class_name_get(eo_obj), eo_obj, efl_class_name_get(obj->smart.parent),
                  obj->smart.parent, efl_class_name_get(eo_above), eo_above,
                  efl_class_name_get(above->smart.parent), above->smart.parent);
             return;
          }
        evas_object_smart_member_stack_above(eo_obj, eo_above);
     }
   else
     {
        if (above->smart.parent)
          {
             SERR("Invalid operation: '%s' %p has no parent but "
                  "above '%s' %p has parent '%s' %p!",
                  efl_class_name_get(eo_obj), eo_obj,
                  efl_class_name_get(eo_above), eo_above,
                  efl_class_name_get(above->smart.parent), above->smart.parent);
             return;
          }
        if (obj->layer != above->layer)
          {
             SERR("Invalid operation: '%s' %p is on layer %d but "
                  "above '%s' %p is on mismatching layer %d!",
                  efl_class_name_get(eo_obj), eo_obj,
                  obj->layer ? (int) obj->layer->layer : -99999,
                  efl_class_name_get(eo_above), eo_above,
                  above->layer ? (int) above->layer->layer : -99999);
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
        evas_object_inform_call_restack(eo_obj, obj);
        return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas->evas);
   obj->restack = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_inform_call_restack(eo_obj, obj);
   if (!obj->layer || obj->layer->evas->is_frozen) return;
   if ((!evas_event_passes_through(eo_obj, obj)) &&
       (!evas_event_freezes_through(eo_obj, obj)) &&
       (!evas_object_is_source_invisible(eo_obj, obj)))
     {
        if (!obj->is_smart && obj->cur->visible)
          {
             _evas_canvas_event_pointer_in_rect_mouse_move_feed(obj->layer->evas,
                                                                eo_obj,
                                                                obj, 1, 1,
                                                                EINA_FALSE,
                                                                NULL);
          }
     }
}

EAPI void
evas_object_stack_below(Evas_Object *obj, Evas_Object *below)
{
   efl_gfx_stack_below((Evas_Object *)obj, below);
}

EOLIAN void
_efl_canvas_object_efl_gfx_stack_stack_below(Eo *eo_obj, Evas_Object_Protected_Data *obj, Efl_Gfx_Stack *eo_below)
{
   if (!eo_below)
     {
        evas_object_lower(eo_obj);
        return;
     }
   if (eo_obj == eo_below) return;
   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_STACK_BELOW, 1, eo_below)) return;
   Evas_Object_Protected_Data *below = efl_data_scope_get(eo_below, EFL_CANVAS_OBJECT_CLASS);
   if ((EINA_INLIST_GET(obj))->next == EINA_INLIST_GET(below))
     {
        evas_object_inform_call_restack(eo_obj, obj);
        return;
     }
   if (obj->smart.parent)
     {
        if (obj->smart.parent != below->smart.parent)
          {
             SERR("Invalid operation: object '%s' %p (parent: '%s' %p) not "
                  "inside same smart parent as below '%s' %p (parent '%s' %p)!",
                  efl_class_name_get(eo_obj), eo_obj, efl_class_name_get(obj->smart.parent),
                  obj->smart.parent, efl_class_name_get(eo_below), eo_below,
                  efl_class_name_get(below->smart.parent), below->smart.parent);
             return;
          }
        evas_object_smart_member_stack_below(eo_obj, eo_below);
     }
   else
     {
        if (below->smart.parent)
          {
             SERR("Invalid operation: object '%s' %p has no parent but "
                  "below '%s' %p has different parent '%s' %p!",
                  efl_class_name_get(eo_obj), eo_obj,
                  efl_class_name_get(eo_below), eo_below,
                  efl_class_name_get(below->smart.parent), below->smart.parent);
             return;
          }
        if (obj->layer != below->layer)
          {
             SERR("Invalid operation: object '%s' %p is on layer %d but "
                  "below '%s' %p is on mismatching layer %d!",
                  efl_class_name_get(eo_obj), eo_obj,
                  obj->layer ? (int) obj->layer->layer : -99999,
                  efl_class_name_get(eo_below), eo_below,
                  below->layer ? (int) below->layer->layer : -99999);
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
        evas_object_inform_call_restack(eo_obj, obj);
        return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas->evas);
   obj->restack = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_inform_call_restack(eo_obj, obj);
   if (!obj->layer || obj->layer->evas->is_frozen) return;
   if ((!evas_event_passes_through(eo_obj, obj)) &&
       (!evas_event_freezes_through(eo_obj, obj)) &&
       (!evas_object_is_source_invisible(eo_obj, obj)))
     {
        if (!obj->is_smart && obj->cur->visible)
          {
             _evas_canvas_event_pointer_in_rect_mouse_move_feed(obj->layer->evas,
                                                                eo_obj,
                                                                obj, 1, 1,
                                                                EINA_FALSE,
                                                                NULL);
          }
     }
}

EAPI Evas_Object *
evas_object_above_get(const Evas_Object *obj)
{
   return efl_gfx_stack_above_get((Evas_Object *)obj);
}

EOLIAN Efl_Gfx_Stack *
_efl_canvas_object_efl_gfx_stack_above_get(Eo *eo_obj EINA_UNUSED,
                                     Evas_Object_Protected_Data *obj)
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

EAPI Evas_Object *
evas_object_below_get(const Evas_Object *obj)
{
   return efl_gfx_stack_below_get((Evas_Object *)obj);
}

EOLIAN Efl_Gfx_Stack *
_efl_canvas_object_efl_gfx_stack_below_get(Eo *eo_obj EINA_UNUSED,
                                     Evas_Object_Protected_Data *obj)
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
_evas_canvas_object_bottom_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
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
_evas_canvas_object_top_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
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
