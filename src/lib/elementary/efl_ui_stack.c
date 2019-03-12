#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_stack_private.h"

#define MY_CLASS EFL_UI_STACK_CLASS
#define MY_CLASS_NAME "Efl.Ui.Stack"

static Efl_Canvas_Animation *show_anim = NULL;
static Efl_Canvas_Animation *hide_anim = NULL;

static void
_announce_hiding(Efl_Ui_Stack *obj, Evas_Object *content)
{
   //Deactivated Event
   Efl_Ui_Stack_Event_Deactivated deactivated_info;
   deactivated_info.content = content;
   efl_event_callback_call(obj,
                           EFL_UI_STACK_EVENT_DEACTIVATED,
                           &deactivated_info);

   //Unloaded Event
   Efl_Ui_Stack_Event_Unloaded unloaded_info;
   unloaded_info.content = content;
   efl_event_callback_call(obj,
                           EFL_UI_STACK_EVENT_UNLOADED,
                           &unloaded_info);
}

static void
_show_content_without_anim(Efl_Ui_Stack *obj, Evas_Object *content)
{
   //Loaded Event
   Efl_Ui_Stack_Event_Loaded loaded_info;
   loaded_info.content = content;
   efl_event_callback_call(obj, EFL_UI_STACK_EVENT_LOADED,
                           &loaded_info);

   evas_object_raise(content);
   /* efl_ui_widget_resize_object_set() calls efl_gfx_entity_visible_set()
    * internally.
    * Therefore, efl_ui_widget_resize_object_set() is called after
    * setting animation and efl_gfx_entity_visible_set() is not called. */
   efl_ui_widget_resize_object_set(obj, content);

   //Activated Event
   Efl_Ui_Stack_Event_Activated activated_info;
   activated_info.content = content;
   efl_event_callback_call(obj, EFL_UI_STACK_EVENT_ACTIVATED,
                           &activated_info);
}

static void
_hide_content_without_anim(Efl_Ui_Stack *obj EINA_UNUSED, Evas_Object *content)
{
   efl_gfx_entity_visible_set(content, EINA_FALSE);
}

static void
_content_del_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Content_Data *cd = data;

   //Popped content has already called deactivated event and unloaded event.
   if (cd->popped_hidden) return;

   _announce_hiding(NULL, cd->content);
}

static Content_Data *
_content_data_new(Eo *obj EINA_UNUSED, Eo *content)
{
   Content_Data *cd = calloc(1, sizeof(Content_Data));
   if (!cd)
     {
        ERR("Memory allocation error!");
        return NULL;
     }

   cd->content = content;

   efl_event_callback_add(cd->content, EFL_EVENT_DEL, _content_del_cb, cd);

   return cd;
}

static void
_content_data_del(Content_Data *cd)
{
   if (!cd) return;

   if (cd->content)
     efl_del(cd->content);

   free(cd);
}

static void
_anim_started_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_event_freeze(efl_animation_player_target_get(event->object));
}

static Evas_Object*
_end_anim(Transit_Data *td)
{
   Efl_Canvas_Object *content = td->cd->content;

   efl_event_thaw(content);
   td->cd->on_pushing = EINA_FALSE;
   td->cd->on_popping = EINA_FALSE;
   free(td);

   return content;
}

static void
_hide_anim_ended_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Stack_Data *pd = efl_data_scope_safe_get(data, EFL_UI_STACK_CLASS);
   Efl_Canvas_Object *content;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   content = _end_anim(pd->hide_td);
   pd->hide_td = NULL;
   efl_gfx_entity_visible_set(content, EINA_FALSE);
   _announce_hiding(data, content);
}

static void
_show_anim_ended_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Stack_Data *pd = efl_data_scope_safe_get(data, EFL_UI_STACK_CLASS);
   Efl_Canvas_Object *content;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   content = _end_anim(pd->show_td);
   pd->show_td = NULL;
   //Activated Event
   Efl_Ui_Stack_Event_Activated activated_info;
   activated_info.content = content;
   efl_event_callback_call(data,
                           EFL_UI_STACK_EVENT_ACTIVATED,
                           &activated_info);
}

static void
_show_content_with_anim(Efl_Ui_Stack *obj, Efl_Ui_Stack_Data *pd, Content_Data *cd)
{
   //immidiatly stop hiding animation
   efl_player_stop(pd->show);
   if (pd->show_td)
     EINA_SAFETY_ERROR("td is set but it should not");

   //attach new content target
   efl_animation_player_target_set(pd->show, cd->content);

   Transit_Data *td = calloc(1, sizeof(Transit_Data));
   td->cd = cd;
   pd->show_td = td;

   /* efl_ui_widget_resize_object_set() calls efl_gfx_entity_visible_set()
    * internally.
    * Therefore, efl_ui_widget_resize_object_set() is called after
    * setting animation and efl_gfx_entity_visible_set() is not called. */
   efl_ui_widget_resize_object_set(obj, cd->content);
   efl_player_start(pd->show);
}

static void
_hide_content_with_anim(Efl_Ui_Stack *obj EINA_UNUSED, Efl_Ui_Stack_Data *pd, Content_Data *cd)
{
   //immidiatly stop hiding animation
   efl_player_stop(pd->hide);
   if (pd->hide_td)
     EINA_SAFETY_ERROR("td is set but it should not");

   //attach new content target
   efl_animation_player_target_set(pd->hide, cd->content);

   Transit_Data *td = calloc(1, sizeof(Transit_Data));
   td->cd = cd;
   pd->hide_td = td;

   efl_player_start(pd->hide);
}

EOLIAN static void
_efl_ui_stack_push(Eo *obj, Efl_Ui_Stack_Data *pd, Eo *content)
{
   if (!content) return;

   //If the given content exists in the stack, promote the given content to the top.
   Content_Data *cd = NULL;
   EINA_INLIST_FOREACH(pd->stack, cd)
      if (cd->content == content)
        break;

   Content_Data *top_cd = NULL;
   if (pd->stack)
     top_cd = EINA_INLIST_CONTAINER_GET(pd->stack->last, Content_Data);

   if (cd)
     {
        //If given content is already the top content, then do nothing.
        if (cd == top_cd)
          return;

        //Remove the given content(existing content) to promote it to the top.
        pd->stack = eina_inlist_remove(pd->stack, EINA_INLIST_GET(cd));
     }
   else
     {
        cd = _content_data_new(obj, content);
        if (!cd) return;

        evas_object_smart_member_add(content, obj);
     }

   pd->stack = eina_inlist_append(pd->stack, EINA_INLIST_GET(cd));

   //Loaded Event
   Efl_Ui_Stack_Event_Loaded loaded_info;
   loaded_info.content = content;
   efl_event_callback_call(obj, EFL_UI_STACK_EVENT_LOADED, &loaded_info);

   /* Apply transition to top content.
    * Hide top content with animation. */
   if (top_cd)
     {
        Eo *top_content = top_cd->content;

        /* If content is being pushed now, then finish current animation and hide
         * the content without animation. */
        if (top_cd->on_pushing)
          {
             _hide_content_without_anim(obj, top_content);
             _announce_hiding(obj, top_content);
          }
        else
          {
             top_cd->on_pushing = EINA_TRUE;

             _hide_content_with_anim(obj, pd, top_cd);
          }
     }

   /* Prepare transition for new content.
    * Hide new content without animation. */
   cd->on_pushing = EINA_TRUE;
   _hide_content_without_anim(obj, content);

   /* Apply transition to new content.
    * Show new content with animation. */
   _show_content_with_anim(obj, pd, cd);
}

static void
_pop_content_hide_cb(void *data, const Efl_Event *event)
{
   Content_Data *cd = data;

   /* object is being shown */
   if (event->info) return;

   cd->popped_hidden = EINA_TRUE;

   _content_data_del(cd);
}

EOLIAN static Eo *
_efl_ui_stack_pop(Eo *obj, Efl_Ui_Stack_Data *pd)
{
   if (!pd->stack)
     {
        ERR("There is no content in the stack!");
        return NULL;
     }

   Content_Data *top_cd = EINA_INLIST_CONTAINER_GET(pd->stack->last, Content_Data);

   pd->stack = eina_inlist_remove(pd->stack, EINA_INLIST_GET(top_cd));

   /* Apply transition to top content.
    * Hide top content with animation. */
     {
        Eo *top_content = top_cd->content;

        /* If content is being popped now, then finish current animation and show
         * the content without animation. */
        if (top_cd->on_popping)
          {
             _hide_content_without_anim(obj, top_content);
             _announce_hiding(obj, top_content);
          }
        else
          {
             top_cd->on_popping = EINA_TRUE;

             //Deallocate content data when hide animation is finished.
             efl_event_callback_add(top_content, EFL_GFX_ENTITY_EVENT_VISIBILITY_CHANGED,
                                    _pop_content_hide_cb, top_cd);

             _hide_content_with_anim(obj, pd, top_cd);
          }
     }

   if (pd->stack)
     {
        Content_Data *prev_cd = EINA_INLIST_CONTAINER_GET(pd->stack->last,
                                                          Content_Data);
        Eo *prev_content = prev_cd->content;

        prev_cd->on_popping = EINA_TRUE;

        //Loaded Event
        Efl_Ui_Stack_Event_Loaded loaded_info;
        loaded_info.content = prev_content;
        efl_event_callback_call(obj, EFL_UI_STACK_EVENT_LOADED,
                                &loaded_info);

        /* Apply transition to previous content.
         * Show previous content with animation. */
        _show_content_with_anim(obj, pd, prev_cd);
     }

   return NULL;
}

EOLIAN static void
_efl_ui_stack_insert_before(Eo *obj, Efl_Ui_Stack_Data *pd,
                            Eo *base_content, Eo *content)
{
   if (!content) return;

   Content_Data *base_cd = NULL;
   EINA_INLIST_FOREACH(pd->stack, base_cd)
      if (base_cd->content == base_content)
        break;

   if (!base_cd)
     {
        ERR("The given base content is not found in the stack!");
        return;
     }

   Content_Data *cd = _content_data_new(obj, content);
   if (!cd) return;

   pd->stack = eina_inlist_prepend_relative(pd->stack,
                                            EINA_INLIST_GET(cd),
                                            EINA_INLIST_GET(base_cd));
   evas_object_smart_member_add(content, obj);
}

EOLIAN static void
_efl_ui_stack_insert_after(Eo *obj, Efl_Ui_Stack_Data *pd,
                           Eo *base_content, Eo *content)
{
   if (!content) return;

   Content_Data *base_cd = NULL;
   EINA_INLIST_FOREACH(pd->stack, base_cd)
      if (base_cd->content == base_content)
        break;

   if (!base_cd)
     {
        ERR("The given base content is not found in the stack!");
        return;
     }

   Content_Data *cd = _content_data_new(obj, content);
   if (!cd) return;

   pd->stack = eina_inlist_append_relative(pd->stack,
                                           EINA_INLIST_GET(cd),
                                           EINA_INLIST_GET(base_cd));
   evas_object_smart_member_add(content, obj);

   if (pd->stack->last == EINA_INLIST_GET(cd))
     {
        /* Do not apply transition for insert.
         * Hide top content without animation. */
        _hide_content_without_anim(obj, base_cd->content);
        _announce_hiding(obj, base_cd->content);

        /* Do not apply transition for insert.
         * Show new content without animation. */
        _show_content_without_anim(obj, content);
     }
}

EOLIAN static void
_efl_ui_stack_insert_at(Eo *obj, Efl_Ui_Stack_Data *pd,
                        int index, Eo *content)
{
   if (!content)
     {
        ERR("The given content is NULL!");
        return;
     }

   int count = eina_inlist_count(pd->stack);
   if ((index < 0) || (index > count))
     {
        ERR("The index(%d) should be from 0 to #contents in the stack(%d)!",
            index, count);
        return;
     }

   Content_Data *base_cd = NULL;

   if (index == count)
     {
        base_cd = EINA_INLIST_CONTAINER_GET(pd->stack->last, Content_Data);
     }
   else
     {
        int i = 0;
        EINA_INLIST_FOREACH(pd->stack, base_cd)
          {
             if (i == index)
               break;

             i++;
          }
     }

   Content_Data *cd = _content_data_new(obj, content);
   if (!cd) return;

   if (index == count)
     pd->stack = eina_inlist_append_relative(pd->stack,
                                             EINA_INLIST_GET(cd),
                                             EINA_INLIST_GET(base_cd));
   else
     pd->stack = eina_inlist_prepend_relative(pd->stack,
                                              EINA_INLIST_GET(cd),
                                              EINA_INLIST_GET(base_cd));

   evas_object_smart_member_add(content, obj);

   if (pd->stack->last == EINA_INLIST_GET(cd))
     {

        /* Do not apply transition for insert.
         * Hide top content without animation. */
        _hide_content_without_anim(obj, base_cd->content);
        _announce_hiding(obj, base_cd->content);

        /* Do not apply transition for insert.
         * Show new content without animation. */
        _show_content_without_anim(obj, content);
     }
}

EOLIAN static void
_efl_ui_stack_remove(Eo *obj, Efl_Ui_Stack_Data *pd, Eo *content)
{
   if (!pd->stack)
     {
        ERR("There is no content in the stack!");
        return;
     }

   if (!content)
     {
        ERR("The given content is NULL!");
        return;
     }

   Content_Data *cd = NULL;

   EINA_INLIST_FOREACH(pd->stack, cd)
     {
        if (cd->content == content)
          break;
     }
   if (!cd)
     {
        ERR("The given content does not exist in the stack!");
        return;
     }

   Eina_Bool remove_top = EINA_FALSE;
   if (pd->stack->last == EINA_INLIST_GET(cd))
     remove_top = EINA_TRUE;

   pd->stack = eina_inlist_remove(pd->stack, EINA_INLIST_GET(cd));

   _announce_hiding(obj, cd->content);
   _content_data_del(cd);

   if (remove_top)
     {
        if (pd->stack)
          {
             Content_Data *new_top_cd = EINA_INLIST_CONTAINER_GET(pd->stack->last,
                                                                  Content_Data);

             /* Do not apply transition for insert.
              * Show new content without animation. */
             _show_content_without_anim(obj, new_top_cd->content);
          }
     }
}

EOLIAN static void
_efl_ui_stack_remove_at(Eo *obj, Efl_Ui_Stack_Data *pd,
                        int index)
{
   if (!pd->stack)
     {
        ERR("There is no content in the stack!");
        return;
     }

   int count = eina_inlist_count(pd->stack);
   if ((index < 0) || (index >= count))
     {
        ERR("The index(%d) should be from 0 to (#contents - 1) in the stack(%d)!",
            index, count);
        return;
     }

   Content_Data *cd = NULL;
   int i = 0;
   EINA_INLIST_FOREACH(pd->stack, cd)
     {
        if (i == index)
          break;
        i++;
     }

   Eina_Bool remove_top = EINA_FALSE;
   if (pd->stack->last == EINA_INLIST_GET(cd))
     remove_top = EINA_TRUE;

   pd->stack = eina_inlist_remove(pd->stack, EINA_INLIST_GET(cd));

   _announce_hiding(NULL, cd->content);
   _content_data_del(cd);

   //FIXME: Apply transition here.
   if (remove_top)
     {
        if (pd->stack)
          {
             Content_Data *new_top_cd = EINA_INLIST_CONTAINER_GET(pd->stack->last,
                                                                  Content_Data);
             /* Do not apply transition for insert.
              * Show new content without animation. */
             _show_content_without_anim(obj, new_top_cd->content);
          }
     }
}

EOLIAN static int
_efl_ui_stack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Stack_Data *pd, Efl_Canvas_Object *content)
{
   if (!pd->stack)
     {
        ERR("There is no content in the stack!");
        return -1;
     }

   if (!content)
     {
        ERR("The given content is NULL!");
        return -1;
     }

   Content_Data *cd = NULL;
   int index = 0;
   int count = eina_inlist_count(pd->stack);

   EINA_INLIST_FOREACH(pd->stack, cd)
     {
        if (cd->content == content)
          break;
        index++;
     }

   //The given content is not found.
   if (index == count) return -1;

   return index;
}

EOLIAN static Eo *
_efl_ui_stack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Stack_Data *pd, int index)
{
   if (!pd->stack)
     {
        ERR("There is no content in the stack!");
        return NULL;
     }

   int count = eina_inlist_count(pd->stack);
   if ((index < 0) || (index >= count))
     {
        ERR("The index(%d) should be from 0 to (#contents - 1) in the stack(%d)!",
            index, count);
        return NULL;
     }

   Content_Data *cd = NULL;
   int i = 0;
   EINA_INLIST_FOREACH(pd->stack, cd)
     {
        if (i == index)
          break;
        i++;
     }

   if (cd)
     return cd->content;

   return NULL;
}

EOLIAN static Eo *
_efl_ui_stack_top(Eo *obj EINA_UNUSED, Efl_Ui_Stack_Data *pd)
{
   if (!pd->stack) return NULL;

   Content_Data *cd = EINA_INLIST_CONTAINER_GET(pd->stack->last, Content_Data);

   return cd->content;
}

EFL_CALLBACKS_ARRAY_DEFINE(_anim_show_event_cb,
  {EFL_ANIMATION_PLAYER_EVENT_STARTED, _anim_started_cb},
  {EFL_ANIMATION_PLAYER_EVENT_ENDED, _show_anim_ended_cb},
)

EFL_CALLBACKS_ARRAY_DEFINE(_anim_hide_event_cb,
  {EFL_ANIMATION_PLAYER_EVENT_STARTED, _anim_started_cb},
  {EFL_ANIMATION_PLAYER_EVENT_ENDED, _hide_anim_ended_cb},
)

EOLIAN static Eo *
_efl_ui_stack_efl_object_constructor(Eo *obj, Efl_Ui_Stack_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Animation *sh, *hi;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   //Default Show Animation
   sh = show_anim = efl_add(EFL_CANVAS_ANIMATION_ALPHA_CLASS, obj);
   efl_animation_alpha_set(show_anim, 0.0, 1.0);
   efl_animation_duration_set(show_anim, 0.5);
   efl_animation_final_state_keep_set(show_anim, EINA_TRUE);

   pd->show = efl_add(EFL_CANVAS_ANIMATION_PLAYER_CLASS, obj);
   efl_animation_player_animation_set(pd->show, sh);
   efl_player_play_set(pd->show, EINA_FALSE);
   efl_event_callback_array_add(pd->show, _anim_show_event_cb(), obj);

   //Default Hide Animation
   hi = hide_anim = efl_add(EFL_CANVAS_ANIMATION_ALPHA_CLASS, obj);
   efl_animation_alpha_set(hide_anim, 1.0, 0.0);
   efl_animation_duration_set(hide_anim, 0.5);
   efl_animation_final_state_keep_set(hide_anim, EINA_TRUE);

   pd->hide = efl_add(EFL_CANVAS_ANIMATION_PLAYER_CLASS, obj);
   efl_animation_player_animation_set(pd->hide, hi);
   efl_player_play_set(pd->hide, EINA_FALSE);
   efl_event_callback_array_add(pd->hide, _anim_hide_event_cb(), obj);

   return obj;
}

#include "efl_ui_stack.eo.c"
