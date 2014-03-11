#include "evas_common_private.h"
#include "evas_private.h"

static void _evas_layer_free(Evas_Layer *lay);

void
evas_object_inject(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas *e)
{
   Evas_Layer *lay;

   if (!obj) return;
   if (!e) return;
   if (obj->in_layer) return;
   lay = evas_layer_find(e, obj->cur->layer);
   if (!lay)
     {
        lay = evas_layer_new(e);
        lay->layer = obj->cur->layer;
        evas_layer_add(lay);
     }
   eo_data_ref(eo_obj, NULL);
   lay->objects = (Evas_Object_Protected_Data *)eina_inlist_append(EINA_INLIST_GET(lay->objects), EINA_INLIST_GET(obj));
   lay->usage++;
   obj->layer = lay;
   obj->in_layer = 1;
}

void
evas_object_release(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, int clean_layer)
{
   if (!obj->in_layer) return;
   obj->layer->objects = (Evas_Object_Protected_Data *)eina_inlist_remove(EINA_INLIST_GET(obj->layer->objects), EINA_INLIST_GET(obj));
   eo_data_unref(eo_obj, obj);
   obj->layer->usage--;
   if (clean_layer)
     {
        if (obj->layer->usage <= 0)
          {
             evas_layer_del(obj->layer);
             _evas_layer_free(obj->layer);
          }
     }
   obj->layer = NULL;
   obj->in_layer = 0;
}

Evas_Layer *
evas_layer_new(Evas *eo_e)
{
   Evas_Public_Data *e = eo_data_ref(eo_e, EVAS_CLASS);
   Evas_Layer *lay;

   lay = calloc(1, sizeof(Evas_Layer));
   if (!lay) return NULL;
   lay->evas = e;
   return lay;
}

static void
_evas_layer_free(Evas_Layer *lay)
{
   free(lay);
}

void
evas_layer_pre_free(Evas_Layer *lay)
{
   Evas_Object_Protected_Data *obj;

   EINA_INLIST_FOREACH(lay->objects, obj)
     {
        if ((!obj->smart.parent) && (!obj->delete_me))
          evas_object_del(obj->object);
     }
}

void
evas_layer_free_objects(Evas_Layer *lay)
{
   Evas_Object_Protected_Data *obj;

   EINA_INLIST_FREE(lay->objects, obj)
     {
        evas_object_free(obj->object, 0);
     }
}

void
evas_layer_clean(Evas *eo_e)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Layer *tmp;

   while (e->layers)
     {
        tmp = e->layers;
        evas_layer_del(tmp);
        _evas_layer_free(tmp);
     }
}

Evas_Layer *
evas_layer_find(Evas *eo_e, short layer_num)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
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

   eo_data_unref(e->evas, e);
   lay->evas = NULL;
}

static void
_evas_object_layer_set_child(Evas_Object *eo_obj, Evas_Object *par, short l)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *par_obj = eo_data_scope_get(par, EVAS_OBJ_CLASS);

   if (obj->delete_me) return;
   if (obj->cur->layer == l) return;
   evas_object_release(eo_obj, obj, 1);
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

        contained = (Eina_Inlist *)evas_object_smart_members_get_direct(eo_obj);
        EINA_INLIST_FOREACH(contained, member)
          {
             _evas_object_layer_set_child(member->object, eo_obj, l);
          }
     }
}

/* public functions */

EOLIAN void
_evas_object_layer_set(Eo *eo_obj, Evas_Object_Protected_Data *obj EINA_UNUSED, short l)
{
   Evas *eo_e;

   if (obj->delete_me) return;
   if (evas_object_intercept_call_layer_set(eo_obj, obj, l)) return;
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
        evas_object_inform_call_restack(eo_obj);
        return;
     }
   evas_object_change(eo_obj, obj);
   if (!obj->is_smart)
     {
        if (evas_object_is_in_output_rect(eo_obj, obj,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y, 1, 1) &&
            obj->cur->visible)
          if (eina_list_data_find(obj->layer->evas->pointer.object.in, obj))
            evas_event_feed_mouse_move(obj->layer->evas->evas,
                                       obj->layer->evas->pointer.x,
                                       obj->layer->evas->pointer.y,
                                       obj->layer->evas->last_timestamp,
                                       NULL);
     }
   else
     {
        Eina_Inlist *contained;
        Evas_Object_Protected_Data *member;

        contained = (Eina_Inlist *)evas_object_smart_members_get_direct(eo_obj);
        EINA_INLIST_FOREACH(contained, member)
          {
            _evas_object_layer_set_child(member->object, eo_obj, l);
          }
     }
   evas_object_inform_call_restack(eo_obj);
}

EOLIAN short
_evas_object_layer_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->smart.parent)
     {
        Evas_Object_Protected_Data *smart_parent_obj = eo_data_scope_get(obj->smart.parent, EVAS_OBJ_CLASS);
        return smart_parent_obj->cur->layer;
     }
   return obj->cur->layer;
}

