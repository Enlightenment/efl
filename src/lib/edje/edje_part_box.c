#include "edje_private.h"
#include "edje_part_helper.h"
#include "efl_canvas_layout_part_box.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_PART_BOX_CLASS

#include "../evas/canvas/evas_box.eo.h"

PROXY_IMPLEMENTATION(box, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

/* Legacy features */

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_pack_clear(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_box_remove_all(pd->ed, pd->part, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_unpack_all(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_box_remove_all(pd->ed, pd->part, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_unpack(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   Evas_Object *removed;
   PROXY_DATA_GET(obj, pd);
   removed = _edje_part_box_remove(pd->ed, pd->part, subobj);
   return (removed == subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_pack(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_linear_pack_begin(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_box_prepend(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_linear_pack_end(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_linear_pack_before(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_box_insert_before(pd->ed, pd->part, subobj, existing);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_linear_pack_after(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_box_insert_after(pd->ed, pd->part, subobj, existing);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_box_efl_pack_linear_pack_at(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index)
{
   PROXY_DATA_GET(obj, pd);
   int cnt = efl_content_count(obj);
   if ((index < 0) && ((-index) <= (cnt + 1)))
     index = cnt + index + 1;
   if ((index >= 0) && (index < cnt))
     return _edje_part_box_insert_at(pd->ed, pd->part, subobj, index);
   else
     return _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static Efl_Gfx_Entity *
_efl_canvas_layout_part_box_efl_pack_linear_pack_unpack_at(Eo *obj, void *_pd EINA_UNUSED, int index)
{
   PROXY_DATA_GET(obj, pd);
   if (index < 0) index += efl_content_count(obj);
   return _edje_part_box_remove_at(pd->ed, pd->part, index);
}

/* New APIs with Eo */

EOLIAN static Efl_Gfx_Entity *
_efl_canvas_layout_part_box_efl_pack_linear_pack_content_get(Eo *obj, void *_pd EINA_UNUSED, int index)
{
   PROXY_DATA_GET(obj, pd);
   if (index < 0) index += efl_content_count(obj);
   return _edje_part_box_content_at(pd->ed, pd->part, index);
}

EOLIAN static int
_efl_canvas_layout_part_box_efl_pack_linear_pack_index_get(Eo *obj, void *_pd EINA_UNUSED, const Efl_Gfx_Entity * subobj)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;
   Eina_List *l;
   int k = 0;

   PROXY_DATA_GET(obj, pd);
   priv = efl_data_scope_get(pd->rp->object, EVAS_BOX_CLASS);
   if (!priv) return -1;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        if (opt->obj == subobj)
          return k;
        k++;
     }
   return -1;
}


EOLIAN static Eina_Iterator *
_efl_canvas_layout_part_box_efl_container_content_iterate(Eo *obj, void *_pd EINA_UNUSED)
{
   Eina_Iterator *it;

   PROXY_DATA_GET(obj, pd);
   if (!pd->rp->typedata.container) return NULL;
   it = evas_object_box_iterator_new(pd->rp->object);

   return efl_canvas_iterator_create(pd->rp->object, it, NULL);
}

EOLIAN static int
_efl_canvas_layout_part_box_efl_container_content_count(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return evas_obj_box_count(pd->rp->object);
}

EOLIAN static Efl_Ui_Dir
_efl_canvas_layout_part_box_efl_ui_direction_direction_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   const Edje_Part_Description_Box *desc =
         (Edje_Part_Description_Box *) pd->rp->chosen_description;

   if (!desc || !desc->box.layout)
     return EFL_UI_DIR_DEFAULT;

   if (!strncmp(desc->box.layout, "vertical", 8))
     return EFL_UI_DIR_VERTICAL;
   else if (!strncmp(desc->box.layout, "horizontal", 10))
     return EFL_UI_DIR_HORIZONTAL;

   WRN("unknown orientation '%s'", desc->box.layout);
   return EFL_UI_DIR_DEFAULT;
}

#include "efl_canvas_layout_part_box.eo.c"
