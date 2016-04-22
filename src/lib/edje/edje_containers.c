#include "edje_private.h"

#define EFL_CANVAS_LAYOUT_INTERNAL_BOX_PROTECTED

#include "efl_canvas_layout_internal_box.eo.h"
#include "../evas/canvas/evas_box.eo.h"
#include "../evas/canvas/evas_line.eo.h"
#include "../evas/canvas/evas_text.eo.h"

#define BOX_CLASS EFL_CANVAS_LAYOUT_INTERNAL_BOX_CLASS

typedef struct _Edje_Box_Data Edje_Box_Data;
typedef struct _Box_Item_Iterator Box_Item_Iterator;

struct _Edje_Box_Data
{
   Edje           *ed;
   Edje_Real_Part *rp;
   const char     *part;
};

struct _Box_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
};

static Eina_Bool
_del_cb(void *data, const Eo_Event *event EINA_UNUSED)
{
   Edje_Real_Part *rp = data;
   rp->typedata.container->eo_proxy = NULL;
   return EO_CALLBACK_CONTINUE;
}

Eo *
_edje_box_internal_proxy_get(Edje_Object *obj, Edje *ed, Edje_Real_Part *rp)
{
   Efl_Canvas_Layout_Internal_Box *eo = rp->typedata.container->eo_proxy;

   if (eo) return eo;

   eo = eo_add(BOX_CLASS, obj, efl_canvas_layout_internal_box_real_part_set(eo_self, ed, rp, rp->part->name));
   eo_event_callback_add(eo, EO_BASE_EVENT_DEL, _del_cb, rp);

   rp->typedata.container->eo_proxy = eo;
   return eo;
}

EOLIAN static void
_efl_canvas_layout_internal_box_real_part_set(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, void *ed, void *rp, const char *part)
{
   pd->ed = ed;
   pd->rp = rp;
   pd->part = part;
}

EOLIAN static Eo_Base *
_efl_canvas_layout_internal_box_eo_base_finalize(Eo *obj, Edje_Box_Data *pd)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->rp && pd->ed && pd->part, NULL);
   return eo_finalize(eo_super(obj, BOX_CLASS));
}

/* Legacy features */

EOLIAN static void
_efl_canvas_layout_internal_box_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Edje_Box_Data *pd)
{
   _edje_part_box_remove_all(pd->ed, pd->part, EINA_TRUE);
}

EOLIAN static void
_efl_canvas_layout_internal_box_efl_pack_unpack_all(Eo *obj EINA_UNUSED, Edje_Box_Data *pd)
{
   _edje_part_box_remove_all(pd->ed, pd->part, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_unpack(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx_Base *subobj)
{
   Evas_Object *removed;
   removed = _edje_part_box_remove(pd->ed, pd->part, subobj);
   return (removed == subobj);
}

EOLIAN static void
_efl_canvas_layout_internal_box_efl_pack_pack(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx_Base *subobj)
{
   _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static void
_efl_canvas_layout_internal_box_efl_pack_linear_pack_begin(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx_Base *subobj)
{
   _edje_part_box_prepend(pd->ed, pd->part, subobj);
}

EOLIAN static void
_efl_canvas_layout_internal_box_efl_pack_linear_pack_end(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx_Base *subobj)
{
   _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_before(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx_Base *subobj, const Efl_Gfx_Base *existing)
{
   return _edje_part_box_insert_before(pd->ed, pd->part, subobj, existing);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_after(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx_Base *subobj, const Efl_Gfx_Base *existing)
{
   return _edje_part_box_insert_after(pd->ed, pd->part, subobj, existing);
}

EOLIAN static void
_efl_canvas_layout_internal_box_efl_pack_linear_pack_insert(Eo *obj, Edje_Box_Data *pd, Efl_Gfx_Base *subobj, int index)
{
   int cnt = efl_content_count(obj);
   if ((index >= 0) && (index < cnt))
     _edje_part_box_insert_at(pd->ed, pd->part, subobj, index);
   else
     _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static Efl_Gfx_Base *
_efl_canvas_layout_internal_box_efl_pack_linear_content_at_remove(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, int index)
{
   if (index < 0) index += efl_content_count(obj);
   return _edje_part_box_remove_at(pd->ed, pd->part, index);
}

/* New APIs with Eo */

EOLIAN static Efl_Gfx_Base *
_efl_canvas_layout_internal_box_efl_pack_linear_content_at_get(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, int index)
{
   if (index < 0) index += efl_content_count(obj);
   return _edje_part_box_content_at(pd->ed, pd->part, index);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_container_content_remove(Eo *obj, Edje_Box_Data *pd EINA_UNUSED, Efl_Gfx_Base *subobj)
{
   return efl_pack_unpack(obj, subobj);
}

EOLIAN static int
_efl_canvas_layout_internal_box_efl_pack_linear_content_index_get(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx_Base * subobj)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;
   Eina_List *l;
   int k = 0;

   priv = eo_data_scope_get(pd->rp->object, EVAS_BOX_CLASS);
   if (!priv) return -1;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        if (opt->obj == subobj)
          return k;
        k++;
     }
   return -1;
}

/* this iterator is the same as efl_ui_box */
static Eina_Bool
_box_item_iterator_next(Box_Item_Iterator *it, void **data)
{
   Efl_Gfx_Base *sub;

   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_box_item_iterator_get_container(Box_Item_Iterator *it)
{
   return it->object;
}

static void
_box_item_iterator_free(Box_Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   eina_list_free(it->list);
   eo_unref(it->object);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_canvas_layout_internal_box_efl_container_content_iterate(Eo *obj, Edje_Box_Data *pd)
{
   Box_Item_Iterator *it;

   if (!pd->rp->typedata.container) return NULL;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = evas_object_box_children_get(pd->rp->object);
   it->real_iterator = eina_list_iterator_new(it->list);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_box_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_box_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_box_item_iterator_free);
   it->object = eo_ref(obj);

   return &it->iterator;
}

EOLIAN static int
_efl_canvas_layout_internal_box_efl_container_content_count(Eo *obj EINA_UNUSED, Edje_Box_Data *pd)
{
   if (!pd->rp->typedata.container) return 0;
   return evas_obj_box_count(pd->rp->object);
}

/* Legacy API implementation */

#define PART_BOX_GET(obj, part, ...) ({ \
   Eo *__box = efl_content_get(obj, part); \
   if (!__box || !eo_isa(__box, EFL_CANVAS_LAYOUT_INTERNAL_BOX_CLASS)) \
     { \
        ERR("No such box part '%s' in layout %p", part, obj); \
        return __VA_ARGS__; \
     } \
   __box; })

EAPI Eina_Bool
edje_object_part_box_append(Edje_Object *obj, const char *part, Evas_Object *child)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   efl_pack_end(box, child);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_box_prepend(Edje_Object *obj, const char *part, Evas_Object *child)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   efl_pack_begin(box, child);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_box_insert_before(Edje_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   return efl_pack_before(box, child, reference);
}

EAPI Eina_Bool
edje_object_part_box_insert_after(Edje_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   return efl_pack_after(box, child, reference);
}

EAPI Eina_Bool
edje_object_part_box_insert_at(Edje_Object *obj, const char *part, Evas_Object *child, unsigned int pos)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   efl_pack_insert(box, child, pos);
   return EINA_TRUE;
}

EAPI Evas_Object *
edje_object_part_box_remove_at(Edje_Object *obj, const char *part, unsigned int pos)
{
   Eo *box = PART_BOX_GET(obj, part, NULL);
   return efl_pack_content_at_remove(box, pos);
}

EAPI Evas_Object *
edje_object_part_box_remove(Edje_Object *obj, const char *part, Evas_Object *child)
{
   Eo *box = PART_BOX_GET(obj, part, NULL);
   if (efl_pack_unpack(box, child))
     return child;
   return NULL;
}

EAPI Eina_Bool
edje_object_part_box_remove_all(Edje_Object *obj, const char *part, Eina_Bool clear)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   if (clear)
     efl_pack_clear(box);
   else
     efl_pack_unpack_all(box);
   return EINA_TRUE;
}

#include "efl_canvas_layout_internal_box.eo.c"
