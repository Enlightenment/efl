#include "edje_private.h"

#include <Eo.h>

typedef struct _Edje_Transition_Animation_Data Edje_Transition_Animation_Data;
struct _Edje_Transition_Animation_Data
{
   Evas_Object *obj;
   struct
   {
      Evas_Coord x, y, w, h;
   } start, end;
};

struct _Edje_Part_Box_Animation
{
   struct
   {
      Evas_Object_Box_Layout layout;
      void *data;
      void(*free_data)(void *data);
      Edje_Alignment align;
      Evas_Point padding;
   } start, end;
   Eina_List *objs;
   Eina_Bool recalculate:1;
   Evas_Object *box;
   double progress;
   double start_progress;
   int box_start_w, box_start_h;
};

static void
_edje_box_layout_find_all(const char *name, const char *name_alt, Evas_Object_Box_Layout *cb, void **data, void (**free_data)(void *data))
{
   if (!_edje_box_layout_find(name, cb, data, free_data))
     {
        if ((!name_alt) ||
            (!_edje_box_layout_find(name_alt, cb, data, free_data)))
          {
             ERR("box layout '%s' (fallback '%s') not available, using horizontal.",
                 name, name_alt);
             *cb = evas_object_box_layout_horizontal;
             *free_data = NULL;
             *data = NULL;
          }
     }
}

static void
_edje_box_layout_calculate_coords(Evas_Object *obj, Evas_Object_Box_Data *priv, Edje_Part_Box_Animation *anim)
{
   Eina_List *l;
   Edje_Transition_Animation_Data *tad;
   Evas_Coord x, y;

   evas_object_geometry_get(obj, &x, &y, &anim->box_start_w, &anim->box_start_h);
   EINA_LIST_FOREACH(anim->objs, l, tad)
     {
        evas_object_geometry_get(tad->obj, &tad->start.x, &tad->start.y,
              &tad->start.w, &tad->start.h);
        tad->start.x = tad->start.x - x;
        tad->start.y = tad->start.y - y;
     }
   evas_object_box_padding_set(obj, anim->end.padding.x, anim->end.padding.y);
   evas_object_box_align_set(obj, TO_DOUBLE(anim->end.align.x), TO_DOUBLE(anim->end.align.y));
   if (anim->end.layout)
     anim->end.layout(obj, priv, anim->end.data);
   else if (anim->start.layout)
     anim->start.layout(obj, priv, anim->start.data);

   EINA_LIST_FOREACH(anim->objs, l, tad)
     {
        evas_object_geometry_get(tad->obj, &tad->end.x, &tad->end.y,
              &tad->end.w, &tad->end.h);
        tad->end.x = tad->end.x - x;
        tad->end.y = tad->end.y - y;
     }
}

static void
_edje_box_layout_exec(Evas_Object *obj, Edje_Part_Box_Animation *anim)
{
   Eina_List *l;
   Edje_Transition_Animation_Data *tad;
   Evas_Coord x, y, w, h;
   Evas_Coord cur_x, cur_y, cur_w, cur_h;
   double progress;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   progress = (anim->progress - anim->start_progress) / (1 - anim->start_progress);

   EINA_LIST_FOREACH(anim->objs, l, tad)
     {
        cur_x = x + (tad->start.x + ((tad->end.x - tad->start.x) * progress)) * (w / (double)anim->box_start_w);
        cur_y = y + (tad->start.y + ((tad->end.y - tad->start.y) * progress)) * (h / (double)anim->box_start_h);
        cur_w = (w / (double)anim->box_start_w) * (tad->start.w + ((tad->end.w - tad->start.w) * progress));
        cur_h = (h / (double)anim->box_start_h) * (tad->start.h + ((tad->end.h - tad->start.h) * progress));
        evas_object_move(tad->obj, cur_x, cur_y);
        evas_object_resize(tad->obj, cur_w, cur_h);
     }
}

static void
_edje_box_layout(Evas_Object *obj, Evas_Object_Box_Data *priv, void *data)
{
   Edje_Part_Box_Animation *anim = data;
   if (anim->progress < 0.01)
     {
        if (anim->start.layout)
          {
             evas_object_box_padding_set(obj, anim->start.padding.x, anim->start.padding.y);
             evas_object_box_align_set(obj, TO_DOUBLE(anim->start.align.x), TO_DOUBLE(anim->start.align.y));
             anim->start.layout(obj, priv, anim->start.data);
          }
        return;
     }

   if (anim->recalculate)
     {
        _edje_box_layout_calculate_coords(obj, priv, anim);
        anim->start_progress = anim->progress;
        anim->recalculate = EINA_FALSE;
     }

   if ((anim->progress > 0) && (anim->start_progress < 1))
      _edje_box_layout_exec(obj, anim);
}

void
_edje_box_layout_free_data(void *data)
{
   Edje_Transition_Animation_Data *tad;
   Edje_Part_Box_Animation *anim = data;
   if (anim->start.free_data && anim->start.data)
      anim->start.free_data(anim->start.data);
   if (anim->end.free_data && anim->end.data)
      anim->end.free_data(anim->end.data);
   EINA_LIST_FREE(anim->objs, tad)
      free(tad);
   free(data);
}

Edje_Part_Box_Animation *
_edje_box_layout_anim_new(Evas_Object *box)
{
   Edje_Part_Box_Animation *anim = calloc(1, sizeof(Edje_Part_Box_Animation));
   if (!anim)
      return NULL;

   anim->box = box;
   evas_object_box_layout_set(box, _edje_box_layout, anim, NULL);

   return anim;
}

void
_edje_box_recalc_apply(Edje *ed EINA_UNUSED, Edje_Real_Part *ep, Edje_Calc_Params *p3 EINA_UNUSED, Edje_Part_Description_Box *chosen_desc)
{
   Evas_Object_Box_Data *priv;
#if 0
   int min_w, min_h;
#endif
   if ((ep->type != EDJE_RP_TYPE_CONTAINER) ||
       (!ep->typedata.container)) return;
     
   if ((ep->param2) && (ep->description_pos != ZERO))
     {
        Edje_Part_Description_Box *param2_desc = (Edje_Part_Description_Box *)ep->param2->description;
        if (ep->typedata.container->anim->end.layout == NULL)
          {
             _edje_box_layout_find_all(param2_desc->box.layout, param2_desc->box.alt_layout, &ep->typedata.container->anim->end.layout, &ep->typedata.container->anim->end.data, &ep->typedata.container->anim->end.free_data);
             ep->typedata.container->anim->end.padding.x = param2_desc->box.padding.x;
             ep->typedata.container->anim->end.padding.y = param2_desc->box.padding.y;
             ep->typedata.container->anim->end.align.x = param2_desc->box.align.x;
             ep->typedata.container->anim->end.align.y = param2_desc->box.align.y;

             priv = eo_data_scope_get(ep->object, EVAS_BOX_CLASS);
             if (priv == NULL)
                return;

             evas_object_box_padding_set(ep->object, ep->typedata.container->anim->start.padding.x, ep->typedata.container->anim->start.padding.y);
             evas_object_box_align_set(ep->object, TO_DOUBLE(ep->typedata.container->anim->start.align.x), TO_DOUBLE(ep->typedata.container->anim->start.align.y));
             ep->typedata.container->anim->start.layout(ep->object, priv, ep->typedata.container->anim->start.data);
             _edje_box_layout_calculate_coords(ep->object, priv, ep->typedata.container->anim);
             ep->typedata.container->anim->start_progress = 0.0;
          }
        evas_object_smart_changed(ep->object);
     }
   else
     {
        ep->typedata.container->anim->end.layout = NULL;
     }

   if (ep->description_pos < 0.01 || !ep->typedata.container->anim->start.layout)
     {
        _edje_box_layout_find_all(chosen_desc->box.layout, chosen_desc->box.alt_layout, &ep->typedata.container->anim->start.layout, &ep->typedata.container->anim->start.data, &ep->typedata.container->anim->start.free_data);
        ep->typedata.container->anim->start.padding.x = chosen_desc->box.padding.x;
        ep->typedata.container->anim->start.padding.y = chosen_desc->box.padding.y;
        ep->typedata.container->anim->start.align.x = chosen_desc->box.align.x;
        ep->typedata.container->anim->start.align.y = chosen_desc->box.align.y;
        evas_object_smart_changed(ep->object);
     }

   ep->typedata.container->anim->progress = ep->description_pos;

   if (evas_object_smart_need_recalculate_get(ep->object))
     {
	evas_object_smart_need_recalculate_set(ep->object, 0);
	evas_object_smart_calculate(ep->object);
     }
#if 0 /* Why the hell do we affect part size after resize ??? */
   evas_object_size_hint_min_get(ep->object, &min_w, &min_h);
   if (chosen_desc->box.min.h && (p3->w < min_w))
     p3->w = min_w;
   if (chosen_desc->box.min.v && (p3->h < min_h))
     p3->h = min_h;
#endif
}

Eina_Bool
_edje_box_layout_add_child(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Edje_Transition_Animation_Data *tad;
   
   if ((rp->type != EDJE_RP_TYPE_CONTAINER) ||
       (!rp->typedata.container)) return EINA_FALSE;
   tad = calloc(1, sizeof(Edje_Transition_Animation_Data));
   if (!tad) return EINA_FALSE;
   tad->obj = child_obj;
   rp->typedata.container->anim->objs = eina_list_append(rp->typedata.container->anim->objs, tad);
   rp->typedata.container->anim->recalculate = EINA_TRUE;
   return EINA_TRUE;
}

void
_edje_box_layout_remove_child(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Eina_List *l;
   Edje_Transition_Animation_Data *tad;

   if ((rp->type != EDJE_RP_TYPE_CONTAINER) ||
       (!rp->typedata.container)) return;
   EINA_LIST_FOREACH(rp->typedata.container->anim->objs, l, tad)
     {
        if (tad->obj == child_obj)
          {
             free(eina_list_data_get(l));
             rp->typedata.container->anim->objs = eina_list_remove_list(rp->typedata.container->anim->objs, l);
             rp->typedata.container->anim->recalculate = EINA_TRUE;
             break;
          }
     }
   rp->typedata.container->anim->recalculate = EINA_TRUE;
}
