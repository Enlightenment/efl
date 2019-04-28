#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_active_view_view_manager_plain.eo.h"

typedef struct {
   Efl_Ui_Active_View_Container * container;
   Efl_Gfx_Entity *group;
   Eina_Size2D page_size;
   int current_content;
   Eina_Bool animation;
} Efl_Ui_Active_View_View_Manager_Plain_Data;

#define MY_CLASS EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_PLAIN_CLASS

static void
_emit_position(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_View_Manager_Plain_Data *pd)
{
   double absolut_position = pd->current_content;
   efl_event_callback_call(obj, EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_EVENT_POS_UPDATE, &absolut_position);
}

static void
_geom_sync(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_View_Manager_Plain_Data *pd)
{
   Efl_Gfx_Entity *entity = efl_pack_content_get(pd->container, pd->current_content);
   Eina_Rect group_pos = efl_gfx_entity_geometry_get(pd->group);
   Eina_Rect goal = EINA_RECT_EMPTY();

   goal.size = pd->page_size;
   goal.y = (group_pos.y + group_pos.h/2)-pd->page_size.h/2;
   goal.x = (group_pos.x + group_pos.w/2)-pd->page_size.w/2;
   efl_gfx_entity_geometry_set(entity, goal);
}

static void
_resize_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _geom_sync(data, efl_data_scope_get(data, MY_CLASS));
}

static void
_move_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _geom_sync(data, efl_data_scope_get(data, MY_CLASS));
}

EFL_CALLBACKS_ARRAY_DEFINE(group_callback,
  {EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _resize_cb},
  {EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _move_cb},
)

EOLIAN static void
_efl_ui_active_view_view_manager_plain_efl_ui_active_view_view_manager_bind(Eo *obj, Efl_Ui_Active_View_View_Manager_Plain_Data *pd, Efl_Ui_Active_View_Container *active_view, Efl_Canvas_Group *group)
{
   if (active_view && group)
     {
        int index;

        pd->container = active_view;
        pd->group = group;

        efl_event_callback_array_add(pd->group, group_callback(), obj);

        for (int i = 0; i < efl_content_count(active_view) ; ++i) {
           Efl_Gfx_Entity *elem = efl_pack_content_get(active_view, i);
           efl_canvas_group_member_add(pd->group, elem);
           efl_gfx_entity_visible_set(elem, EINA_FALSE);
        }
        index = efl_ui_active_view_active_index_get(active_view);
        if (index != -1)
          {
             pd->current_content = index;
             efl_gfx_entity_visible_set(efl_pack_content_get(pd->container, pd->current_content), EINA_TRUE);
             _geom_sync(obj, pd);
             _emit_position(obj, pd);
          }
     }
}

static void
_content_changed(Eo *obj, Efl_Ui_Active_View_View_Manager_Plain_Data *pd)
{
   if (efl_ui_active_view_active_index_get(pd->container) != pd->current_content)
     {
        pd->current_content = efl_ui_active_view_active_index_get(pd->container);
        efl_gfx_entity_visible_set(efl_pack_content_get(pd->container, pd->current_content), EINA_FALSE);
        _geom_sync(obj, pd);
        _emit_position(obj, pd);
     }
}

EOLIAN static void
_efl_ui_active_view_view_manager_plain_efl_ui_active_view_view_manager_content_add(Eo *obj, Efl_Ui_Active_View_View_Manager_Plain_Data *pd, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   efl_canvas_group_member_add(pd->group, subobj);
   efl_gfx_entity_visible_set(subobj, EINA_FALSE);
   _content_changed(obj, pd);
}

EOLIAN static void
_efl_ui_active_view_view_manager_plain_efl_ui_active_view_view_manager_content_del(Eo *obj, Efl_Ui_Active_View_View_Manager_Plain_Data *pd, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   efl_canvas_group_member_remove(pd->group, subobj);
   _content_changed(obj, pd);
}
EOLIAN static void
_efl_ui_active_view_view_manager_plain_efl_ui_active_view_view_manager_switch_to(Eo *obj, Efl_Ui_Active_View_View_Manager_Plain_Data *pd, int from EINA_UNUSED, int to)
{
   Efl_Gfx_Entity *to_obj, *from_obj;
   to_obj = efl_pack_content_get(pd->container, to);
   from_obj = efl_pack_content_get(pd->container, from);
   if (from_obj)
     {
        efl_gfx_entity_visible_set(from_obj, EINA_FALSE);
        pd->current_content = -1;
     }

   if (to_obj)
     {
        efl_gfx_entity_visible_set(to_obj, EINA_TRUE);
        pd->current_content = to;
     }

   _emit_position(obj, pd);
   _geom_sync(obj, pd);
}

EOLIAN static void
_efl_ui_active_view_view_manager_plain_efl_ui_active_view_view_manager_view_size_set(Eo *obj, Efl_Ui_Active_View_View_Manager_Plain_Data *pd, Eina_Size2D size)
{
   pd->page_size = size;
   _geom_sync(obj, pd);
}

EOLIAN static void
_efl_ui_active_view_view_manager_plain_efl_object_destructor(Eo *obj, Efl_Ui_Active_View_View_Manager_Plain_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));

   efl_event_callback_array_del(pd->group, group_callback(), obj);

   for (int i = 0; i < efl_content_count(pd->container); ++i)
     {
        Efl_Gfx_Stack *elem = efl_pack_content_get(pd->container, i);
        efl_gfx_entity_visible_set(elem, EINA_TRUE);
     }
}

EOLIAN static void
_efl_ui_active_view_view_manager_plain_efl_ui_active_view_view_manager_animation_enabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_View_Manager_Plain_Data *pd, Eina_Bool animation)
{
   pd->animation = animation;
}

EOLIAN static Eina_Bool
_efl_ui_active_view_view_manager_plain_efl_ui_active_view_view_manager_animation_enabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Active_View_View_Manager_Plain_Data *pd)
{
   return pd->animation;
}


#include "efl_ui_active_view_view_manager_plain.eo.c"
