#include "edje_private.h"
#include "edje_part_helper.h"

#define EFL_CANVAS_LAYOUT_INTERNAL_BOX_PROTECTED
#include "efl_canvas_layout_internal_box.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_INTERNAL_BOX_CLASS

#include "../evas/canvas/evas_box.eo.h"

PROXY_IMPLEMENTATION(box, BOX, Edje_Box_Data)
#undef PROXY_IMPLEMENTATION

typedef struct _Part_Item_Iterator Part_Item_Iterator;

/* Legacy features */

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_pack_clear(Eo *obj, Edje_Box_Data *pd)
{
   RETURN_VAL(_edje_part_box_remove_all(pd->ed, pd->part, EINA_TRUE));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_unpack_all(Eo *obj, Edje_Box_Data *pd)
{
   RETURN_VAL(_edje_part_box_remove_all(pd->ed, pd->part, EINA_FALSE));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_unpack(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   Evas_Object *removed;
   removed = _edje_part_box_remove(pd->ed, pd->part, subobj);
   RETURN_VAL((removed == subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_pack(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   RETURN_VAL(_edje_part_box_append(pd->ed, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_begin(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   RETURN_VAL(_edje_part_box_prepend(pd->ed, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_end(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   RETURN_VAL(_edje_part_box_append(pd->ed, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_before(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj, const Efl_Gfx *existing)
{
   RETURN_VAL(_edje_part_box_insert_before(pd->ed, pd->part, subobj, existing));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_after(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj, const Efl_Gfx *existing)
{
   RETURN_VAL(_edje_part_box_insert_after(pd->ed, pd->part, subobj, existing));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_at(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj, int index)
{
   int cnt = PROXY_CALL(efl_content_count(obj));
   if ((index < 0) && ((-index) <= (cnt + 1)))
     index = cnt + index + 1;
   if ((index >= 0) && (index < cnt))
     RETURN_VAL(_edje_part_box_insert_at(pd->ed, pd->part, subobj, index));
   else
     RETURN_VAL(_edje_part_box_append(pd->ed, pd->part, subobj));
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_box_efl_pack_linear_pack_unpack_at(Eo *obj, Edje_Box_Data *pd, int index)
{
   if (index < 0) index += PROXY_CALL(efl_content_count(obj));
   RETURN_VAL(_edje_part_box_remove_at(pd->ed, pd->part, index));
}

/* New APIs with Eo */

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_box_efl_pack_linear_pack_content_get(Eo *obj, Edje_Box_Data *pd, int index)
{
   if (index < 0) index += PROXY_CALL(efl_content_count(obj));
   RETURN_VAL(_edje_part_box_content_at(pd->ed, pd->part, index));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_container_content_remove(Eo *obj, Edje_Box_Data *pd EINA_UNUSED, Efl_Gfx *subobj)
{
   RETURN_VAL(PROXY_CALL(efl_pack_unpack(obj, subobj)));
}

EOLIAN static int
_efl_canvas_layout_internal_box_efl_pack_linear_pack_index_get(Eo *obj, Edje_Box_Data *pd, const Efl_Gfx * subobj)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;
   Eina_List *l;
   int k = 0;

   priv = efl_data_scope_get(pd->rp->object, EVAS_BOX_CLASS);
   if (!priv) RETURN_VAL(-1);
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        if (opt->obj == subobj)
          RETURN_VAL(k);
        k++;
     }
   RETURN_VAL(-1);
}

/* this iterator is the same as efl_ui_box */
static Eina_Bool
_part_item_iterator_next(Part_Item_Iterator *it, void **data)
{
   Efl_Gfx *sub;

   if (!it->object) return EINA_FALSE;
   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_part_item_iterator_get_container(Part_Item_Iterator *it)
{
   return it->object;
}

static void
_part_item_iterator_free(Part_Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   efl_wref_del(it->object, &it->object);
   free(it);
}

static Eina_Iterator *
_part_item_iterator_create(Eo *obj, Eina_Iterator *real_iterator)
{
   Part_Item_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = real_iterator;
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_part_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_part_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_part_item_iterator_free);
   efl_wref_add(obj, &it->object);

   return &it->iterator;
}

EOLIAN static Eina_Iterator *
_efl_canvas_layout_internal_box_efl_container_content_iterate(Eo *obj, Edje_Box_Data *pd)
{
   Eina_Iterator *it;

   if (!pd->rp->typedata.container) RETURN_VAL(NULL);
   it = evas_object_box_iterator_new(pd->rp->object);

   RETURN_VAL(_part_item_iterator_create(pd->rp->object, it));
}

EOLIAN static int
_efl_canvas_layout_internal_box_efl_container_content_count(Eo *obj, Edje_Box_Data *pd)
{
   RETURN_VAL(evas_obj_box_count(pd->rp->object));
}

EOLIAN static Efl_Orient
_efl_canvas_layout_internal_box_efl_orientation_orientation_get(Eo *obj, Edje_Box_Data *pd)
{
   const Edje_Part_Description_Box *desc =
         (Edje_Part_Description_Box *) pd->rp->chosen_description;

   if (!desc || !desc->box.layout)
     RETURN_VAL(EFL_ORIENT_NONE);

   if (!strncmp(desc->box.layout, "vertical", 8))
     RETURN_VAL(EFL_ORIENT_VERTICAL);
   else if (!strncmp(desc->box.layout, "horizontal", 10))
     RETURN_VAL(EFL_ORIENT_HORIZONTAL);

   WRN("unknown orientation '%s'", desc->box.layout);
   RETURN_VAL(EFL_ORIENT_NONE);
}

#include "efl_canvas_layout_internal_box.eo.c"
