#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_LIST_DEFAULT_ITEM_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_GROUP_ITEM_CLASS
#define MY_CLASS_PFX  efl_ui_group_item

#define MY_CLASS_NAME "Efl.Ui.Grid_Default_Item"


typedef struct {
   Eina_List *registered_items;
} Efl_Ui_Group_Item_Data;

EOLIAN static Efl_Object*
_efl_ui_group_item_efl_object_constructor(Eo *obj, Efl_Ui_Group_Item_Data *pd EINA_UNUSED)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "group_item");

   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_group_item_efl_object_invalidate(Eo *obj, Efl_Ui_Group_Item_Data *pd EINA_UNUSED)
{
   efl_pack_clear(obj);
   efl_invalidate(efl_super(obj, MY_CLASS));
}

static void _unregister_item(Eo *obj EINA_UNUSED, Efl_Ui_Group_Item_Data *pd, Efl_Gfx_Entity *subobj);

static void
_invalidate(void *data, const Efl_Event *ev)
{
   Efl_Ui_Group_Item_Data *pd = efl_data_scope_safe_get(data, MY_CLASS);
   if (!pd) return;
   _unregister_item(data, pd, ev->object);
}

static void
_register_item(Eo *obj, Efl_Ui_Group_Item_Data *pd, Efl_Gfx_Entity *subobj)
{
   efl_ui_item_parent_set(subobj, obj);
   efl_event_callback_add(subobj, EFL_EVENT_INVALIDATE, _invalidate, obj);
   pd->registered_items = eina_list_append(pd->registered_items, subobj);
}

static void
_unregister_item(Eo *obj EINA_UNUSED, Efl_Ui_Group_Item_Data *pd, Efl_Gfx_Entity *subobj)
{
   efl_ui_item_container_set(subobj, NULL);
   efl_event_callback_del(subobj, EFL_EVENT_INVALIDATE, _invalidate, obj);
   pd->registered_items = eina_list_remove(pd->registered_items, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Group_Item_Data *pd)
{
   Eo *item;

   while(pd->registered_items)
     {
        item = eina_list_data_get(pd->registered_items);
        efl_del(item);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_unpack_all(Eo *obj, Efl_Ui_Group_Item_Data *pd)
{
   Eo *item;
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, EINA_FALSE);

   EINA_LIST_FREE(pd->registered_items, item)
     {
        efl_pack_unpack(container, item);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_unpack(Eo *obj, Efl_Ui_Group_Item_Data *pd, Efl_Gfx_Entity *subobj)
{
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, EINA_FALSE);

   if (!efl_pack_unpack(container, subobj))
     return EINA_FALSE;

   _unregister_item(obj, pd, subobj);
   return EINA_TRUE;
}

#define HANDLE_REG_CALL(cond) \
  if (!(cond)) \
    { \
      _unregister_item(obj, pd, subobj); \
      return EINA_FALSE; \
    } \
  return EINA_TRUE;

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_pack(Eo *obj, Efl_Ui_Group_Item_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(subobj == obj, EINA_FALSE);
   return efl_pack_end(obj, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Group_Item_Data *pd, Efl_Gfx_Entity *subobj)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(subobj == obj, EINA_FALSE);
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, EINA_FALSE);
   int group_index = efl_pack_index_get(container, obj);

   _register_item(obj, pd, subobj);
   HANDLE_REG_CALL(efl_pack_at(container, subobj, group_index + eina_list_count(pd->registered_items)));
}

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Group_Item_Data *pd, Efl_Gfx_Entity *subobj)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(subobj == obj, EINA_FALSE);
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, EINA_FALSE);
   int group_index = efl_pack_index_get(container, obj);

   _register_item(obj, pd, subobj);
   HANDLE_REG_CALL(efl_pack_at(container, subobj, group_index + 1));
}

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Group_Item_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(subobj == obj, EINA_FALSE);
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, EINA_FALSE);
   int group_index = efl_pack_index_get(container, obj);

   //FIXME, maybe we should check if existing is really part of this group
   _register_item(obj, pd, subobj);
   if (existing)
     {
        HANDLE_REG_CALL(efl_pack_before(container, subobj, existing));
     }
   else
     {
        HANDLE_REG_CALL(efl_pack_at(container, subobj, group_index + 1));
     }

}

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Group_Item_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(subobj == obj, EINA_FALSE);
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, EINA_FALSE);
   int group_index = efl_pack_index_get(container, obj);

   //FIXME, maybe we should check if existing is really part of this group
   _register_item(obj, pd, subobj);
   if (existing)
     {
        HANDLE_REG_CALL(efl_pack_after(container, subobj, existing));
     }
   else
     {
        HANDLE_REG_CALL(efl_pack_at(container, subobj, group_index + eina_list_count(pd->registered_items)));
     }
}

EOLIAN static Eina_Bool
_efl_ui_group_item_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Group_Item_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(subobj == obj, EINA_FALSE);
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, EINA_FALSE);
   int group_index = efl_pack_index_get(container, obj);

   int count = eina_list_count(pd->registered_items);

   if (index < -count)
     return efl_pack_begin(obj, subobj);

   if (index >= count)
     return efl_pack_end(obj, subobj);

   if (index < 0)
     index += count;

   _register_item(obj, pd, subobj);
   HANDLE_REG_CALL(efl_pack_at(container, subobj, group_index + 1 + index));
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_group_item_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Group_Item_Data *pd, int index)
{
   Eo *result, *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, NULL);
   int group_index = efl_pack_index_get(container, obj);
   int count = eina_list_count(pd->registered_items);

   if (index <= -count)
     index = 0;
   else if (index >= count)
     index = eina_list_count(pd->registered_items);
   else if (index < 0)
     index += count;

   result = efl_pack_unpack_at(container, group_index + 1 + index);
   if (result)
     _unregister_item(obj, pd, result);

   return result;
}

EOLIAN static int
_efl_ui_group_item_efl_pack_linear_pack_index_get(Eo *obj, Efl_Ui_Group_Item_Data *pd, const Efl_Gfx_Entity *subobj)
{
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, -1);
   int first_index = efl_pack_index_get(container, obj) + 1;
   int subobj_index = efl_pack_index_get(container, subobj);

   if (subobj_index == -1) return -1;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(first_index <= subobj_index && subobj_index < (first_index + (int)eina_list_count(pd->registered_items)), -1);

   return subobj_index - (first_index);
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_group_item_efl_pack_linear_pack_content_get(Eo *obj, Efl_Ui_Group_Item_Data *pd EINA_UNUSED, int index)
{
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, NULL);
   int group_index = efl_pack_index_get(container, obj);

   int count = eina_list_count(pd->registered_items);

   if (index <= -count)
     return eina_list_data_get(pd->registered_items);

   if (index >= count)
     return eina_list_last_data_get(pd->registered_items);

   if (index < 0)
     index += count;


   return efl_pack_content_get(container, group_index + 1 + index);
}

typedef struct {
   Eina_Iterator iterator;
   unsigned int current;
   unsigned int max;
   Eo *container;
} Efl_Ui_Group_Item_Iterator;

static Eina_Bool
_next_item(Efl_Ui_Group_Item_Iterator *it, void **data)
{
   if (it->current >= it->max) return EINA_FALSE;

   *data = efl_pack_content_get(it->container, it->current);
   it->current++;

   return EINA_TRUE;
}

EOLIAN static Eina_Iterator*
_efl_ui_group_item_efl_container_content_iterate(Eo *obj, Efl_Ui_Group_Item_Data *pd EINA_UNUSED)
{
   Eo *container = efl_ui_item_container_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(container, NULL);
   Efl_Ui_Group_Item_Iterator *it;

   it = calloc(1, sizeof (Efl_Ui_Group_Item_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->current = efl_pack_index_get(container, obj) + 1;
   it->max = it->current + eina_list_count(pd->registered_items);
   it->container = container;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_next_item);
   it->iterator.get_container = NULL;
   it->iterator.free = FUNC_ITERATOR_FREE(free);

   return &it->iterator;
}

EOLIAN static int
_efl_ui_group_item_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Group_Item_Data *pd)
{
   return eina_list_count(pd->registered_items);
}




#include "efl_ui_group_item.eo.c"
