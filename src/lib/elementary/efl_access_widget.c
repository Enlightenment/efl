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

#include "efl_access_widget.eo.h"

typedef struct 
{
   Efl_Access_Object *forced_parent;
   Efl_Ui_Widget *widget;
} Efl_Access_Widget_Data;

EOLIAN static void
_efl_access_widget_efl_access_component_screen_position_get(const Eo *obj, Efl_Access_Widget_Data *_pd EINA_UNUSED, int *x, int *y)
{
   Eina_Rect r = efl_access_component_extents_get(obj, EINA_TRUE);
   if (x) *x = r.x;
   if (y) *y = r.y;
}

EOLIAN static Eina_Bool
_efl_access_widget_efl_access_component_screen_position_set(Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *_pd EINA_UNUSED, int x, int y)
{
   Eina_Rect r = efl_access_component_extents_get(obj, EINA_TRUE);
   r.x = x;
   r.y = y;
   return efl_access_component_extents_set(obj, EINA_TRUE, r);
}

EOLIAN static Eina_Bool
_efl_access_widget_efl_access_component_contains(Eo *obj, Efl_Access_Widget_Data *_pd EINA_UNUSED, Eina_Bool type, int x, int y)
{
   Eina_Rect r = efl_access_component_extents_get(obj, type);
   return eina_rectangle_coords_inside(&r.rect, x, y);
}

EOLIAN static Eo *
_efl_access_widget_efl_access_component_accessible_at_point_get(Eo *obj, Efl_Access_Widget_Data *_pd EINA_UNUSED, Eina_Bool screen_coords, int x, int y)
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
_efl_access_widget_efl_access_component_extents_get(const Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *_pd EINA_UNUSED, Eina_Bool screen_coords)
{
   Eina_Rect r = EINA_RECT(-1, -1, -1, -1);

   if (!_pd->widget)
     return r;

   r = efl_gfx_entity_geometry_get(_pd->widget);
   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(_pd->widget));
        if (ee)
          {
             int ee_x = 0, ee_y = 0;
             ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
             r.x += ee_x;
             r.y += ee_y;
          }
     }
   return r;
}

EOLIAN static Eina_Bool
_efl_access_widget_efl_access_component_extents_set(Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *_pd EINA_UNUSED, Eina_Bool screen_coords, Eina_Rect r)
{
   int wx, wy;

   if ((r.x < 0) || (r.y < 0) || (r.w < 0) || (r.h < 0)) return EINA_FALSE;

   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(_pd->widget));
        if (!ee) return EINA_FALSE;
        evas_object_geometry_get(_pd->widget, &wx, &wy, NULL, NULL);
        ecore_evas_move(ee, r.x - wx, r.y - wy);
     }
   else
     evas_object_move(_pd->widget, r.x, r.y);

   evas_object_resize(_pd->widget, r.w, r.h);
   return EINA_TRUE;
}

EOLIAN static int
_efl_access_widget_efl_access_component_z_order_get(const Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *_pd EINA_UNUSED)
{
   // Currently not used.
   return 0;
}

EOLIAN static Eina_Bool
_efl_access_widget_efl_access_component_focus_grab(Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *pd)
{
   if (!pd->widget)
     return EINA_FALSE;

   if (elm_object_focus_allow_get(pd->widget))
     {
       Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(pd->widget));
       if (!ee) return EINA_FALSE;
       ecore_evas_activate(ee);
       elm_object_focus_set(pd->widget, EINA_TRUE);
       return EINA_TRUE;
     }
   return EINA_FALSE;
}

const char*
_elm_access_widget_accessible_plain_name_get(const Evas_Object *obj, const char* name)
{
   char *accessible_plain_name;

   Elm_Widget_Smart_Data *sd = efl_data_scope_get(obj, EFL_UI_WIDGET_CLASS);
   if (!sd) return NULL;

   accessible_plain_name = _elm_util_mkup_to_text(name);
   eina_stringshare_del(sd->accessible_name);
   sd->accessible_name =  eina_stringshare_add(accessible_plain_name);
   free(accessible_plain_name);
   return sd->accessible_name;
}

EOLIAN static const char*
_efl_access_widget_efl_access_object_i18n_name_get(const Eo *obj, Efl_Access_Widget_Data *_pd)
{
   const char *ret, *name;
   name = efl_access_object_i18n_name_get(efl_super(obj, EFL_ACCESS_WIDGET_CLASS));

   if (name) return name;

   ret = elm_object_text_get(_pd->widget);
   if (!ret) return NULL;

   return _elm_access_widget_accessible_plain_name_get(_pd->widget, ret);
}

EOLIAN static Efl_Access_State_Set
_efl_access_widget_efl_access_object_state_set_get(const Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *pd)
{
   Efl_Access_State_Set states = 0;

   if (!pd->widget)
     {
        STATE_TYPE_SET(states, EFL_ACCESS_STATE_DEFUNCT);
        return states;
     }

   if (evas_object_visible_get(pd->widget))
     {
        STATE_TYPE_SET(states, EFL_ACCESS_STATE_VISIBLE);
        if (_elm_widget_onscreen_is(pd->widget))
          STATE_TYPE_SET(states, EFL_ACCESS_STATE_SHOWING);
     }
   if (!elm_widget_child_can_focus_get(pd->widget))
     {
        if (elm_object_focus_allow_get(pd->widget))
          STATE_TYPE_SET(states, EFL_ACCESS_STATE_FOCUSABLE);
        if (efl_ui_focus_object_focus_get(pd->widget))
          STATE_TYPE_SET(states, EFL_ACCESS_STATE_FOCUSED);
     }
   if (!elm_object_disabled_get(pd->widget))
     {
        STATE_TYPE_SET(states, EFL_ACCESS_STATE_ENABLED);
        STATE_TYPE_SET(states, EFL_ACCESS_STATE_SENSITIVE);
     }

   return states;
}

EOLIAN static Eina_List*
_efl_access_widget_efl_access_object_attributes_get(const Eo *obj, Efl_Access_Widget_Data *pd)
{
   const char *type = NULL;
   const char *style = NULL;
   Eina_List *attr_list = NULL;
   Efl_Access_Attribute *attr = NULL;

   attr_list = efl_access_object_attributes_get(efl_super(obj, EFL_ACCESS_WIDGET_CLASS));

   //Add type and style information in addition.
   type = elm_widget_type_get(pd->widget);
   if (type)
     {
        attr = calloc(1, sizeof(Efl_Access_Attribute));
        if (attr)
          {
             attr->key = eina_stringshare_add("type");
             attr->value = eina_stringshare_add(type);
             attr_list = eina_list_append(attr_list, attr);
           }
     }

   style = elm_widget_style_get(pd->widget);
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

EOLIAN static Efl_Access_Object*
_efl_access_widget_efl_access_object_access_parent_get(const Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *pd)
{
   if (pd->forced_parent)
     return pd->forced_parent;

   Elm_Widget_Smart_Data *sd = efl_data_scope_get(pd->widget, EFL_UI_WIDGET_CLASS);
   if (!sd) return NULL;

   return sd->parent_obj ? efl_access_object_query(EFL_ACCESS_OBJECT_MIXIN, sd->parent_obj) : NULL;
}

EOLIAN static Eina_List*
_efl_access_widget_efl_access_object_access_children_get(const Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *pd)
{
   Eina_List *l, *accs = NULL;
   Evas_Object *subobj = NULL;

   Elm_Widget_Smart_Data *sd = efl_data_scope_get(pd->widget, EFL_UI_WIDGET_CLASS);
   if (!sd) return NULL;

   EINA_LIST_FOREACH(sd->subobjs, l, subobj)
     {
        Efl_Access_Object *acc = efl_access_object_query(EFL_ACCESS_OBJECT_MIXIN, subobj);
        if (acc) accs = eina_list_append(accs, acc);
     }
   return accs;
}

EOLIAN static Efl_Ui_Widget*
_efl_access_widget_real_widget_get(const Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *pd)
{
   return pd->widget;
}

EOLIAN static void
_efl_access_widget_real_widget_set(Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *pd, Efl_Ui_Widget *widget)
{
   pd->widget = widget;
}

EOLIAN static Efl_Access_Object*
_efl_access_widget_forced_parent_get(const Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *pd)
{
   return pd->forced_parent;
}

EOLIAN static void
_efl_access_widget_forced_parent_set(Eo *obj EINA_UNUSED, Efl_Access_Widget_Data *pd, Efl_Access_Object *parent)
{
   pd->forced_parent = parent;
}

#include "efl_access_widget.eo.c"

