#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_spotlight_plain_manager.eo.h"

typedef struct {
   Efl_Ui_Spotlight_Container * container;
   Eina_Size2D page_size;
   Efl_Ui_Widget *current_content;
   Efl_Gfx_Entity *clipper;
   Eina_Bool animation;
   double last_pos;
} Efl_Ui_Spotlight_Plain_Manager_Data;

#define MY_CLASS EFL_UI_SPOTLIGHT_PLAIN_MANAGER_CLASS

static void
_emit_position(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Plain_Manager_Data *pd)
{
   double absolut_position = efl_pack_index_get(pd->container, pd->current_content);
   if (!EINA_DBL_EQ(pd->last_pos, absolut_position))
     efl_event_callback_call(obj, EFL_UI_SPOTLIGHT_MANAGER_EVENT_POS_UPDATE, &absolut_position);

   pd->last_pos = absolut_position;
}

static void
_geom_sync(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Plain_Manager_Data *pd)
{
   Efl_Gfx_Entity *entity = pd->current_content;
   Eina_Rect group_pos = efl_gfx_entity_geometry_get(pd->container);
   Eina_Rect goal = EINA_RECT_EMPTY();

   goal.size = pd->page_size;
   goal.y = (group_pos.y + group_pos.h/2)-pd->page_size.h/2;
   goal.x = (group_pos.x + group_pos.w/2)-pd->page_size.w/2;
   efl_gfx_entity_geometry_set(pd->clipper, goal);
   efl_gfx_entity_geometry_set(entity, goal);
}

EOLIAN static void
_efl_ui_spotlight_plain_manager_efl_ui_spotlight_manager_bind(Eo *obj, Efl_Ui_Spotlight_Plain_Manager_Data *pd, Efl_Ui_Spotlight_Container *spotlight)
{
   if (spotlight)
     {
        Efl_Ui_Widget *index;

        pd->container = spotlight;

        pd->clipper = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                              evas_object_evas_get(spotlight));
        evas_object_static_clip_set(pd->clipper, EINA_TRUE);
        efl_canvas_group_member_add(spotlight, pd->clipper);

        for (int i = 0; i < efl_content_count(spotlight) ; ++i) {
           Efl_Gfx_Entity *elem = efl_pack_content_get(spotlight, i);
           efl_key_data_set(elem, "_elm_leaveme", spotlight);
           efl_canvas_object_clipper_set(elem, pd->clipper);
           efl_canvas_group_member_add(pd->container, elem);
           efl_gfx_entity_visible_set(elem, EINA_FALSE);
        }
        index = efl_ui_spotlight_active_element_get(spotlight);
        if (index)
          {
             pd->current_content = index;
             efl_gfx_entity_visible_set(pd->current_content, EINA_TRUE);
             _geom_sync(obj, pd);
             _emit_position(obj, pd);
          }
     }
}

static void
_content_changed(Eo *obj, Efl_Ui_Spotlight_Plain_Manager_Data *pd)
{
   if (efl_ui_spotlight_active_element_get(pd->container) != pd->current_content)
     {
        Efl_Ui_Widget *old_current_content = pd->current_content;
        pd->current_content = efl_ui_spotlight_active_element_get(pd->container);
        efl_gfx_entity_visible_set(old_current_content, EINA_FALSE);
        efl_gfx_entity_visible_set(pd->current_content, EINA_TRUE);
        _geom_sync(obj, pd);
     }
   _emit_position(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_plain_manager_efl_ui_spotlight_manager_content_add(Eo *obj, Efl_Ui_Spotlight_Plain_Manager_Data *pd, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   efl_key_data_set(subobj, "_elm_leaveme", pd->container);
   efl_canvas_object_clipper_set(subobj, pd->clipper);
   efl_canvas_group_member_add(pd->container, subobj);
   efl_gfx_entity_visible_set(subobj, EINA_FALSE);
   _content_changed(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_plain_manager_efl_ui_spotlight_manager_content_del(Eo *obj, Efl_Ui_Spotlight_Plain_Manager_Data *pd, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   efl_key_data_set(subobj, "_elm_leaveme", NULL);
   efl_canvas_object_clipper_set(subobj, NULL);
   efl_canvas_group_member_remove(pd->container, subobj);
   if (pd->current_content == subobj)
     pd->current_content = NULL;
   _content_changed(obj, pd);
}
EOLIAN static void
_efl_ui_spotlight_plain_manager_efl_ui_spotlight_manager_switch_to(Eo *obj, Efl_Ui_Spotlight_Plain_Manager_Data *pd, int from EINA_UNUSED, int to, Efl_Ui_Spotlight_Manager_Switch_Reason reason EINA_UNUSED)
{
   Efl_Gfx_Entity *to_obj, *from_obj;
   to_obj = efl_pack_content_get(pd->container, to);
   from_obj = efl_pack_content_get(pd->container, from);
   if (from_obj)
     {
        efl_gfx_entity_visible_set(from_obj, EINA_FALSE);
        pd->current_content = NULL;
     }

   if (to_obj)
     {
        efl_gfx_entity_visible_set(to_obj, EINA_TRUE);
        pd->current_content = efl_pack_content_get(pd->container, to);
     }

   _emit_position(obj, pd);
   _geom_sync(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_plain_manager_efl_ui_spotlight_manager_size_set(Eo *obj, Efl_Ui_Spotlight_Plain_Manager_Data *pd, Eina_Size2D size)
{
   pd->page_size = size;
   _geom_sync(obj, pd);
}

EOLIAN static void
_efl_ui_spotlight_plain_manager_efl_object_destructor(Eo *obj, Efl_Ui_Spotlight_Plain_Manager_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));

   for (int i = 0; i < efl_content_count(pd->container); ++i)
     {
        Efl_Gfx_Stack *elem = efl_pack_content_get(pd->container, i);
        efl_gfx_entity_visible_set(elem, EINA_TRUE);
     }
}

EOLIAN static void
_efl_ui_spotlight_plain_manager_efl_ui_spotlight_manager_animated_transition_set(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Plain_Manager_Data *pd, Eina_Bool animation)
{
   pd->animation = animation;
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_plain_manager_efl_ui_spotlight_manager_animated_transition_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Plain_Manager_Data *pd)
{
   return pd->animation;
}

EOLIAN static void
_efl_ui_spotlight_plain_manager_efl_object_invalidate(Eo *obj, Efl_Ui_Spotlight_Plain_Manager_Data *pd)
{
   efl_del(pd->clipper);

   for (int i = 0; i < efl_content_count(pd->container); ++i)
     {
        efl_canvas_object_clipper_set(efl_pack_content_get(pd->container, i), NULL);
     }

   efl_invalidate(efl_super(obj, MY_CLASS));
}

#include "efl_ui_spotlight_plain_manager.eo.c"
