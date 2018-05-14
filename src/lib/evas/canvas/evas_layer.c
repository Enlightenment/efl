#include "evas_common_private.h"
#include "evas_private.h"

void
evas_object_inject(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas *e)
{
   Evas_Layer *lay;
   Evas_Public_Data *evas;

   if (!obj) return;
   if (!e) return;
   if (obj->in_layer) return;
   evas = efl_data_scope_get(e, EVAS_CANVAS_CLASS);
   if (!evas) return;
   evas_canvas_async_block(evas);
   lay = evas_layer_find(e, obj->cur->layer);
   if (!lay)
     {
        lay = evas_layer_new(e);
        lay->layer = obj->cur->layer;
        evas_layer_add(lay);
     }
   efl_data_ref(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   lay->objects = (Evas_Object_Protected_Data *)eina_inlist_append(EINA_INLIST_GET(lay->objects), EINA_INLIST_GET(obj));
   lay->usage++;
   obj->layer = lay;
   obj->in_layer = 1;
}

void
evas_object_release(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, int clean_layer)
{
   if (!obj->in_layer) return;
   if (!obj->layer->walking_objects)
     obj->layer->objects = (Evas_Object_Protected_Data *)eina_inlist_remove(EINA_INLIST_GET(obj->layer->objects), EINA_INLIST_GET(obj));
   efl_data_unref(eo_obj, obj);
   if (!obj->layer->walking_objects)
     {
        obj->layer->usage--;
        if (clean_layer)
          {
             if (obj->layer->usage <= 0)
               {
                  evas_layer_del(obj->layer);
               }
          }
        obj->layer = NULL;
        obj->in_layer = 0;
     }
   else
     obj->layer->removes = eina_list_append(obj->layer->removes, obj);
}

Evas_Layer *
evas_layer_new(Evas *eo_e)
{
   Evas_Public_Data *e = efl_data_ref(eo_e, EVAS_CANVAS_CLASS);
   Evas_Layer *lay;

   lay = calloc(1, sizeof(Evas_Layer));
   if (!lay) return NULL;
   lay->evas = e;
   return lay;
}

void
_evas_layer_flush_removes(Evas_Layer *lay)
{
   Evas_Object_Protected_Data *obj;

   if (lay->walking_objects) return;
   EINA_LIST_FREE(lay->removes, obj)
     {
        lay->objects = (Evas_Object_Protected_Data *)
          eina_inlist_remove(EINA_INLIST_GET(lay->objects),
                             EINA_INLIST_GET(obj));
        obj->layer = NULL;
        obj->in_layer = 0;
        if (lay->usage > 0) lay->usage--;
     }
   if (lay->usage <= 0)
     {
        evas_layer_del(lay);
     }
}

void
evas_layer_pre_free(Evas_Layer *lay)
{
   Evas_Object_Protected_Data *obj;

   lay->walking_objects++;
   EINA_INLIST_FOREACH(lay->objects, obj)
     {
        if ((!obj->smart.parent) && (!obj->delete_me))
          evas_object_del(obj->object);
     }
   lay->walking_objects--;
   _evas_layer_flush_removes(lay);
}

void
evas_layer_free_objects(Evas_Layer *lay)
{
   Evas_Object_Protected_Data *obj;

   EINA_INLIST_FREE(lay->objects, obj)
     {
        if (obj->object == NULL)
          {
             ERR("Object still present in the canvas stack, but without a valid object pointer (%s@%p).", obj->type, obj);
          }
        evas_object_free(obj, EINA_FALSE);
     }
}

void
evas_layer_clean(Evas *eo_e)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Layer *tmp;

   while (e->layers)
     {
        tmp = e->layers;
        evas_layer_del(tmp);
     }
}

Evas_Layer *
evas_layer_find(Evas *eo_e, short layer_num)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Layer *layer;

   EINA_INLIST_FOREACH(e->layers, layer)
     {
        if (layer->layer == layer_num) return layer;
     }
   return NULL;
}

void
evas_layer_add(Evas_Layer *lay)
{
   Evas_Layer *layer;

   EINA_INLIST_FOREACH(lay->evas->layers, layer)
     {
        if (layer->layer > lay->layer)
          {
             lay->evas->layers = (Evas_Layer *)eina_inlist_prepend_relative(EINA_INLIST_GET(lay->evas->layers),
                                                                            EINA_INLIST_GET(lay),
                                                                            EINA_INLIST_GET(layer));
             return;
          }
     }
   lay->evas->layers = (Evas_Layer *)eina_inlist_append(EINA_INLIST_GET(lay->evas->layers), EINA_INLIST_GET(lay));
}

void
evas_layer_del(Evas_Layer *lay)
{
   Evas_Public_Data *e;

   e = lay->evas;
   e->layers = (Evas_Layer *)eina_inlist_remove(EINA_INLIST_GET(e->layers), EINA_INLIST_GET(lay));
   efl_data_unref(e->evas, e);
   eina_freeq_ptr_main_add(lay, free, sizeof(*lay));
}

static void
_evas_object_layer_set_child(Evas_Object_Protected_Data *obj, Evas_Object_Protected_Data *par_obj, short l)
{
   if (obj->delete_me) return;
   if (obj->cur->layer == l) return;
   if (EINA_UNLIKELY(obj->in_layer))
     {
        ERR("Invalid internal state of object %p (child marked as being a "
            "top-level object)!", obj->object);
        evas_object_release(obj->object, obj, 1);
     }
   else if ((--obj->layer->usage) == 0)
     {
        evas_layer_del(obj->layer);
     }

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
       state_write->layer = l;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   obj->layer = par_obj->layer;
   obj->layer->usage++;
   if (obj->is_smart)
     {
        Eina_Inlist *contained;
        Evas_Object_Protected_Data *member;

        contained = (Eina_Inlist *)evas_object_smart_members_get_direct(obj->object);
        EINA_INLIST_FOREACH(contained, member)
          {
             _evas_object_layer_set_child(member, obj, l);
          }
     }
}

/* public functions */

EAPI void
evas_object_layer_set(Evas_Object *obj, short l)
{
   efl_gfx_stack_layer_set((Evas_Object *)obj, l);
}

EOLIAN void
_efl_canvas_object_efl_gfx_stack_layer_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, short l)
{
   Evas *eo_e;

   if (obj->delete_me) return;
   evas_object_async_block(obj);
   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_LAYER_SET, 1, l)) return;
   if (obj->smart.parent) return;
   if (obj->cur->layer == l)
     {
        evas_object_raise(eo_obj);
        return;
     }
   eo_e = obj->layer->evas->evas;
   evas_object_release(eo_obj, obj, 1);
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
       state_write->layer = l;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_inject(eo_obj, obj, eo_e);
   obj->restack = 1;
   evas_object_change(eo_obj, obj);
   if (obj->clip.clipees)
     {
        evas_object_inform_call_restack(eo_obj, obj);
        return;
     }
   evas_object_change(eo_obj, obj);
   if (!obj->is_smart && obj->cur->visible)
     {
        _evas_canvas_event_pointer_in_rect_mouse_move_feed(obj->layer->evas, eo_obj, obj,
                                                           1, 1, EINA_TRUE,
                                                           NULL);
     }
   else if (obj->is_smart)
     {
        Eina_Inlist *contained;
        Evas_Object_Protected_Data *member;

        contained = (Eina_Inlist *)evas_object_smart_members_get_direct(eo_obj);
        EINA_INLIST_FOREACH(contained, member)
          {
            _evas_object_layer_set_child(member, obj, l);
          }
     }
   evas_object_inform_call_restack(eo_obj, obj);
}

EAPI short
evas_object_layer_get(const Evas_Object *obj)
{
   return efl_gfx_stack_layer_get((Evas_Object *)obj);
}

EOLIAN short
_efl_canvas_object_efl_gfx_stack_layer_get(const Eo *eo_obj EINA_UNUSED,
                                     Evas_Object_Protected_Data *obj)
{
   if (obj->smart.parent)
     {
        Evas_Object_Protected_Data *smart_parent_obj = efl_data_scope_get(obj->smart.parent, EFL_CANVAS_OBJECT_CLASS);
        return smart_parent_obj->cur->layer;
     }
   return obj->cur->layer;
}

