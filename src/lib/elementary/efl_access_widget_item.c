#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_COMPONENT_PROTECTED
#define EFL_ACCESS_ACTION_PROTECTED
#define EFL_ACCESS_ACTION_BETA

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#include "efl_access_widget_item.eo.h"

EOLIAN static Eina_List *
_efl_access_widget_item_efl_access_object_attributes_get(const Eo *eo_item, void *pd  EINA_UNUSED)
{
   const char *style = NULL;
   Eina_List *attr_list = NULL;
   Efl_Access_Attribute *attr = NULL;

   attr_list = efl_access_object_attributes_get(efl_super(eo_item, EFL_ACCESS_WIDGET_ITEM_CLASS));

   style = elm_object_item_style_get(eo_item);
   if (style)
     {
        attr = calloc(1, sizeof(Efl_Access_Attribute));
        if (attr)
          {
             attr->key = eina_stringshare_add("style");
             attr->value = eina_stringshare_add(style);
             attr_list = eina_list_append(attr_list, attr);
          }
     }
   return attr_list;
}

EAPI Eina_Bool
_elm_widget_item_onscreen_is(const Elm_Object_Item *item)
{
   Eina_Rectangle r1, r2;
   Elm_Widget_Item_Data *id = efl_data_scope_get(item, ELM_WIDGET_ITEM_CLASS);
   if (!id || !id->view) return EINA_FALSE;

   if (!evas_object_visible_get(id->view))
     return EINA_FALSE;

   if (!_elm_widget_onscreen_is(id->widget))
     return EINA_FALSE;

   evas_object_geometry_get(id->view, &r1.x, &r1.y, &r1.w, &r1.h);
   if (eina_rectangle_is_empty(&r1))
     return EINA_FALSE;

   evas_object_geometry_get(id->widget, &r2.x, &r2.y, &r2.w, &r2.h);
   if (!eina_rectangles_intersect(&r1, &r2))
     return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static Efl_Access_State_Set
_efl_access_widget_item_efl_access_object_state_set_get(const Eo *eo_item,
                                                 void *item EINA_UNUSED)
{
   Efl_Access_State_Set states = 0;

   STATE_TYPE_SET(states, EFL_ACCESS_STATE_FOCUSABLE);

   if (elm_object_item_focus_get(eo_item))
     STATE_TYPE_SET(states, EFL_ACCESS_STATE_FOCUSED);
   if (!elm_object_item_disabled_get(eo_item))
     {
        STATE_TYPE_SET(states, EFL_ACCESS_STATE_ENABLED);
        STATE_TYPE_SET(states, EFL_ACCESS_STATE_SENSITIVE);
        STATE_TYPE_SET(states, EFL_ACCESS_STATE_VISIBLE);
     }
   if (_elm_widget_item_onscreen_is(eo_item))
     STATE_TYPE_SET(states, EFL_ACCESS_STATE_SHOWING);

   return states;
}

EOLIAN static void
_efl_access_widget_item_efl_access_component_screen_position_get(const Eo *obj, void *_pd EINA_UNUSED, int *x, int *y)
{
   Eina_Rect r = efl_access_component_extents_get(obj, EINA_TRUE);
   if (x) *x = r.x;
   if (y) *y = r.y;
}

EOLIAN static Eina_Bool
_efl_access_widget_item_efl_access_component_screen_position_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, int x, int y)
{
   Eina_Rect r = efl_access_component_extents_get(obj, EINA_TRUE);
   r.x = x;
   r.y = y;
   return efl_access_component_extents_set(obj, EINA_TRUE, r);
}

EOLIAN static Eina_Bool
_efl_access_widget_item_efl_access_component_contains(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool type, int x, int y)
{
   Eina_Rect r = efl_access_component_extents_get(obj, type);
   return eina_rectangle_coords_inside(&r.rect, x, y);
}

EOLIAN static Eo *
_efl_access_widget_item_efl_access_component_accessible_at_point_get(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool screen_coords, int x, int y)
{
   Eina_List *l, *children;
   Eo *ret = NULL, *child;

   children = efl_access_object_access_children_get(obj);

   EINA_LIST_FOREACH(children, l, child)
     {
        Eina_Bool contains;
        if (efl_isa(child, EFL_ACCESS_COMPONENT_MIXIN))
          {
              contains = efl_access_component_contains(child, screen_coords, x, y);
              if (contains)
                {
                   ret = child;
                   break;
                }
          }
     }

   eina_list_free(children);
   return ret;
}

EOLIAN static Eina_Rect
_efl_access_widget_item_efl_access_component_extents_get(const Eo *obj EINA_UNUSED, void *sd EINA_UNUSED, Eina_Bool screen_coords)
{
   Eina_Rect r = EINA_RECT(-1, -1, -1, -1);
   int ee_x, ee_y;

   r = efl_gfx_entity_geometry_get(obj);
   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        if (ee)
          {
             ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
             r.x += ee_x;
             r.y += ee_y;
          }
     }
   return r;
}

EOLIAN static Eina_Bool
_efl_access_widget_item_efl_access_component_extents_set(Eo *obj EINA_UNUSED, void *sd EINA_UNUSED, Eina_Bool screen_coords EINA_UNUSED, Eina_Rect r EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static int
_efl_access_widget_item_efl_access_component_z_order_get(const Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   // Currently not used.
   return 0;
}

EOLIAN static Eina_Bool
_efl_access_widget_item_efl_access_component_focus_grab(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   elm_object_item_focus_set(obj, EINA_TRUE);
   return elm_object_item_focus_get(obj);
}

#include "efl_access_widget_item.eo.c"
