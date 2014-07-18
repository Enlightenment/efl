#include "evas_render2.h"

static void
_obj_basic_process(Evas_Public_Data *e,
                   Evas_Object_Protected_Data *obj, int l)
{
   Evas_Object *eo_obj = obj->object;

   obj->rect_del = EINA_FALSE;
   obj->render_pre = EINA_FALSE;
   if (obj->delete_me == 2) return;
   else if (obj->delete_me != 0) obj->delete_me++;
   evas_object_clip_recalc(obj);
   obj->is_active = 1;
   obj->func->render_pre(eo_obj, obj, obj->private_data);
   obj->pre_render_done = EINA_TRUE;
   int i; for (i = 0; i < l; i++) printf(" ");
   printf("BASIC %p %p [%10s]\n", e, eo_obj, obj->type);
   obj->func->render_post(eo_obj, obj, obj->private_data);
   obj->restack = EINA_FALSE;
   obj->pre_render_done = EINA_FALSE;
   evas_object_change_reset(eo_obj);
}

static void
_obj_process(Evas_Public_Data *e,
             Evas_Object_Protected_Data *obj, int l)
{
   // process object OR walk through child objects if smart and process those
   Evas_Object_Protected_Data *obj2;
   Evas_Object *eo_obj = obj->object;
   const Eina_Inlist *il;

   if (!obj->changed) return;
   il = evas_object_smart_members_get_direct(eo_obj);
   if (il)
     {
        obj->rect_del = EINA_FALSE;
        obj->render_pre = EINA_FALSE;
        if (obj->delete_me == 2) return;
        else if (obj->delete_me != 0) obj->delete_me++;
        evas_object_clip_recalc(obj);
        obj->is_active = 1;
        obj->func->render_pre(eo_obj, obj, obj->private_data);
        obj->pre_render_done = EINA_TRUE;
        int i; for (i = 0; i < l; i++) printf(" ");
        printf("SMART %p %p [%10s] ch %i\n", e, eo_obj, obj->type, obj->changed);

        EINA_INLIST_FOREACH(il, obj2) _obj_process(e, obj2, l + 1);

        obj->func->render_post(eo_obj, obj, obj->private_data);
        obj->restack = EINA_FALSE;
        obj->pre_render_done = EINA_FALSE;
        evas_object_change_reset(eo_obj);
     }
   else _obj_basic_process(e, obj, l);
}

void
_evas_render2_stage_generate_object_updates(Evas_Public_Data *e)
{
   Evas_Layer *lay;
   Evas_Object_Protected_Data *obj;
   double t;

   static int num = 0;
   printf("........... updates # %i\n", num++);
   t = get_time();
   EINA_INLIST_FOREACH(e->layers, lay)
     {
        EINA_INLIST_FOREACH(lay->objects, obj)
          {
             _obj_process(e, obj, 0);
          }
     }
   t = get_time() - t;
   printf("T: update generation: "); out_time(t);
}
