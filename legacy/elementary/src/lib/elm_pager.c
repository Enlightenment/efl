#include <Elementary.h>
#include "elm_priv.h"

/* FIXME:
 * 1. Possibly remove those callbacks and just use the naviframe ones.
 * 2. I can create a different object, but it can wait until inheritance
 * is implemented, too annoying atm. */
static void
_push_finished(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(obj, "show,finished", event_info);
}

static void
_pop_finished(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(obj, "hide,finished", event_info);
}

EINA_DEPRECATED EAPI Evas_Object *
elm_pager_add(Evas_Object *parent)
{
   Evas_Object *nf = elm_naviframe_add(parent);
   evas_object_smart_callback_add(nf, "push,finished", _push_finished, NULL);
   evas_object_smart_callback_add(nf, "pop,finished", _pop_finished, NULL);
   return nf;
}

EINA_DEPRECATED EAPI void
elm_pager_content_push(Evas_Object *obj, Evas_Object *content)
{
   Elm_Object_Item *it;
   it = elm_naviframe_item_push(obj, NULL, NULL, NULL, content, NULL);
   elm_naviframe_item_title_visible_set(it, EINA_FALSE);
}

EINA_DEPRECATED EAPI void
elm_pager_content_pop(Evas_Object *obj)
{
   elm_naviframe_item_pop(obj);
}

EINA_DEPRECATED EAPI void
elm_pager_content_promote(Evas_Object *obj, Evas_Object *content)
{
   Eina_List *items = elm_naviframe_items_get(obj);
   Eina_List *itr;
   Elm_Object_Item *it;
   EINA_LIST_FOREACH(items, itr, it)
     {
        if (elm_object_item_content_get(it) == content)
          {
             elm_naviframe_item_promote(it);
             break;
          }
     }
   eina_list_free(items);
}

EINA_DEPRECATED EAPI Evas_Object *
elm_pager_content_bottom_get(const Evas_Object *obj)
{
   Elm_Object_Item *it;
   it = elm_naviframe_bottom_item_get(obj);
   return elm_object_item_content_get(it);
}

EINA_DEPRECATED EAPI Evas_Object *
elm_pager_content_top_get(const Evas_Object *obj)
{
   Elm_Object_Item *it;
   it = elm_naviframe_top_item_get(obj);
   return elm_object_item_content_get(it);
}

EINA_DEPRECATED EAPI void
elm_pager_item_style_default_set(Evas_Object *obj, const char *style)
{
   elm_object_style_set(obj, style);
}

EINA_DEPRECATED EAPI const char *
elm_pager_item_style_default_get(const Evas_Object *obj)
{
   return elm_object_style_get(obj);
}
