#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#define MY_CLASS EFL_UI_PAGEINDICATOR_CLASS

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pageindicator.h"
#include "efl_ui_widget_pagecontrol.h"


EOLIAN static void
_efl_ui_pageindicator_efl_gfx_size_set(Eo *obj,
                                       Efl_Ui_Pageindicator_Data *sd,
                                       Evas_Coord w,
                                       Evas_Coord h)
{
   efl_gfx_size_set(sd->box, w, h);

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
}

static void
_updated(void *data, const Efl_Event *event)
{
   Efl_Ui_Pageindicator *obj = data;
   EFL_UI_PAGEINDICATOR_DATA_GET(obj, pd);
   EFL_UI_PAGECONTROL_DATA_GET(event->object, pcd);

   Eina_List *l;
   Pageindicator *item, *curr = NULL, *next = NULL;

   EINA_LIST_FOREACH(pd->items, l, item)
     {
        efl_vg_interpolate(item->vector, pd->off, pd->on, 0.0);
     }

   int p1, p2;
   p1 = pcd->page;
   curr = eina_list_nth(pd->items, p1);
   //ERR("page %d item %p", p1, curr);
   if (curr)
     efl_vg_interpolate(curr->vector, pd->on, pd->off, pcd->ratio);

   if (pcd->ratio != 0)
     {
        p2 = (p1 + 1 + pcd->cnt) % pcd->cnt;
        next = eina_list_nth(pd->items, p2);
        //ERR("page %d item %p", p2, next);
     }
   if (next)
     efl_vg_interpolate(next->vector, pd->off, pd->on, pcd->ratio);
}

EOLIAN static void
_efl_ui_pageindicator_bind(Eo *obj,
                           Efl_Ui_Pageindicator_Data *pd,
                           Efl_Ui_Pagecontrol *target)
{
   //FIXME should add a listener which respond to efl_pack of pagecontrol
   Pageindicator *item;
   int i;

   pd->target.obj = target;
   pd->target.cnt = efl_content_count(target);
   EFL_UI_PAGECONTROL_DATA_GET(pd->target.obj, pcd);

   //FIXME use efl_content_iterate
   for (i = 0; i < pd->target.cnt; i++)
     {
        item = malloc(sizeof(Pageindicator));

        pd->items = eina_list_append(pd->items, item);

        item->object = evas_object_vg_add(evas_object_evas_get(obj));
        efl_gfx_visible_set(item->object, EINA_TRUE);
        evas_object_size_hint_min_set(item->object, 10, 10);

        Efl_VG *r = evas_object_vg_root_node_get(item->object);
        item->vector = efl_add(EFL_VG_SHAPE_CLASS, r);

        if (i == pcd->page)
          efl_vg_interpolate(item->vector, pd->off, pd->on, 1.0);
        else
          efl_vg_interpolate(item->vector, pd->off, pd->on, 0.0);

        efl_pack_end(pd->box, item->object);
     }

   efl_event_callback_add(pd->target.obj, EFL_UI_PAGECONTROL_EVENT_UPDATED,
                          _updated, obj); 
}

EOLIAN static void
_efl_ui_pageindicator_efl_canvas_group_group_add(Eo *obj,
                                                 Efl_Ui_Pageindicator_Data *sd)
{
   sd->box = efl_add(EFL_UI_BOX_CLASS, obj);
   efl_gfx_visible_set(sd->box, EINA_TRUE);

   efl_pack_padding_set(sd->box, 10, 10, EINA_TRUE);

   sd->on = efl_add(EFL_VG_SHAPE_CLASS, NULL);
   evas_vg_shape_append_circle(sd->on, 5, 5, 5);
   evas_vg_node_color_set(sd->on, 255, 255, 255, 255);

   sd->off = efl_add(EFL_VG_SHAPE_CLASS, NULL);
   evas_vg_shape_append_circle(sd->off, 5, 5, 3);
   evas_vg_node_color_set(sd->off, 255, 255, 255, 255);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
}

#define EFL_UI_PAGEINDICATOR_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_pageindicator)

#include "efl_ui_pageindicator.eo.c"
