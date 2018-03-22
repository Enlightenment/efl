#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pager.h"
#include "efl_page_transition.h"

#define MY_CLASS EFL_UI_PAGER_CLASS

#define DEBUG 0

static void
_efl_ui_pager_update(Efl_Ui_Pager_Data *pd)
{
   if (pd->cnt == 0) return;

   efl_page_transition_update(pd->transition, pd->curr.pos);
   if (pd->indicator)
     efl_page_indicator_update(pd->indicator, pd->curr.pos);
}

static void
_job(void *data)
{
   Evas_Object *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);

   int prev_page, page_diff;
   double prev_pos, pos_diff;

   prev_page = pd->curr.page;
   prev_pos = pd->curr.pos;

   pd->job = NULL;

   pos_diff = ((double) pd->down.x - (double) pd->mouse_x) / (double) pd->w;
   pd->curr.pos = pd->down.pos + pos_diff;
   page_diff = (int) pd->curr.pos;

   //FIXME what if (prev_pos != 0.0)
   if ((pd->loop == EFL_UI_PAGER_LOOP_DISABLED) &&
       (((prev_page == 0) && (pd->curr.pos < 0)) ||
        ((prev_page == (pd->cnt - 1)) && (pd->curr.pos > 0))))
     {
        pd->curr.page = prev_page;
        pd->curr.pos = prev_pos;
        return;
     }

   pd->curr.page = (pd->down.page + page_diff + pd->cnt) % pd->cnt;
   pd->curr.pos -= page_diff;

   //FIXME what if (page_diff >= 2 || page_diff <= -2)
   if (page_diff != 0)
     {
#if DEBUG
        ERR("page changed %d -> %d, diff %d",
            pd->down.page, pd->curr.page, page_diff);
#endif
        pd->down.x = pd->mouse_x;
        pd->down.y = pd->mouse_y;
        pd->down.page = pd->curr.page;
        pd->down.pos = pd->curr.pos;

        efl_page_transition_curr_page_change(pd->transition, page_diff);
     }
#if DEBUG
   ERR("curr page %d pos %lf", pd->curr.page, pd->curr.pos);
#endif

   _efl_ui_pager_update(pd);

   return;
}

static Eina_Bool
_animator(void *data, double pos)
{
   Eo *obj = data;
   double p;

   EFL_UI_PAGER_DATA_GET(obj, pd);

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   if (pd->curr.pos < 0.0)
     {
        if (pd->curr.pos > -0.5)
          pd->curr.pos = pd->curr.pos * (1 - p);
        else
          pd->curr.pos = (-1) - (-1 - pd->curr.pos) * (1 - p);
     }
   else
     {
        if (pd->curr.pos < 0.5)
          pd->curr.pos = pd->curr.pos * (1 - p);
        else
          pd->curr.pos = 1 - (1 - pd->curr.pos) * (1 - p);
     }

   if (EINA_DBL_EQ(pd->curr.pos, 1.0))
     {
        efl_page_transition_curr_page_change(pd->transition, 1.0);
        pd->curr.page = (pd->curr.page + 1 + pd->cnt) % pd->cnt;
        pd->curr.pos = 0.0;
     }
   else if (EINA_DBL_EQ(pd->curr.pos, -1.0))
     {
        efl_page_transition_curr_page_change(pd->transition, -1.0);
        pd->curr.page = (pd->curr.page - 1 + pd->cnt) % pd->cnt;
        pd->curr.pos = 0.0;
     }

   _efl_ui_pager_update(pd);

   if (EINA_DBL_EQ(pos, 1.0) || EINA_DBL_EQ(pd->curr.pos, 0.0))
     {
        pd->animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_mouse_down_cb(void *data,
               const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, pd);
   Eina_Position2D pos;

   if (efl_input_pointer_button_get(ev) != 1) return;
   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;

   ELM_SAFE_FREE(pd->animator, ecore_animator_del);

   pd->move_started = EINA_FALSE;

   pos = efl_input_pointer_position_get(ev);
   pd->mouse_x = pos.x - pd->x;
   pd->mouse_y = pos.y - pd->y;

   pd->down.enabled = EINA_TRUE;
   pd->down.x = pd->mouse_x;
   pd->down.y = pd->mouse_y;
   pd->down.page = pd->curr.page;
   pd->down.pos = pd->curr.pos;

#if DEBUG
   ERR("curr page %d pos %lf", pd->curr.page, pd->curr.pos);
#endif
}

static void
_mouse_move_cb(void *data,
               const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, pd);
   Eina_Position2D pos;

   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;
   if (!pd->down.enabled) return;

   pos = efl_input_pointer_position_get(ev);

   if (pd->prev_block && (pd->mouse_x < (pos.x - pd->x))) return;
   if (pd->next_block && (pd->mouse_x > (pos.x - pd->x))) return;

   pd->mouse_x = pos.x - pd->x;
   pd->mouse_y = pos.y - pd->y;

   if (!pd->move_started)
     {
        Evas_Coord dx, dy;
        dx = pd->mouse_x - pd->down.x;
        dy = pd->mouse_y - pd->down.y;

        if (((dx * dx) + (dy * dy)) <=
            (_elm_config->finger_size * _elm_config->finger_size / 4))
          return;

        pd->move_started = EINA_TRUE;
     }

   ecore_job_del(pd->job);
   pd->job = ecore_job_add(_job, pc);
}

static void
_mouse_up_cb(void *data,
             const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Evas_Object *pc = data;
   EFL_UI_PAGER_DATA_GET(pc, pd);
   double time;

   if (efl_input_event_flags_get(ev) & EFL_INPUT_FLAGS_PROCESSED) return;
   if (!pd->down.enabled) return;

   pd->down.enabled = EINA_FALSE;

   ELM_SAFE_FREE(pd->job, ecore_job_del);

   if (EINA_DBL_EQ(pd->curr.pos, 0.0)) return;

   if (pd->curr.pos < 0.0)
     {
        if (pd->curr.pos > -0.5) time = (-1) * pd->curr.pos;
        else time = 1 + pd->curr.pos;
     }
   else
     {
        if (pd->curr.pos < 0.5) time = pd->curr.pos;
        else time = 1 - pd->curr.pos;
     }

   if (time < 0.01) time = 0.01;
   else if (time > 0.99) time = 0.99;

   ecore_animator_del(pd->animator);
   pd->animator = ecore_animator_timeline_add(time, _animator, pc);
}

//FIXME sub_object_parent_add? destruction
static void
_event_handler_create(Eo *obj, Efl_Ui_Pager_Data *pd)
{
   pd->event = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                       evas_object_evas_get(obj));
   evas_object_color_set(pd->event, 0, 0, 0, 0);
   evas_object_repeat_events_set(pd->event, EINA_TRUE);

   efl_content_set(efl_part(obj, "event"), pd->event);

   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_DOWN,
                          _mouse_down_cb, obj);
   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_UP,
                          _mouse_up_cb, obj);
   efl_event_callback_add(pd->event, EFL_EVENT_POINTER_MOVE,
                          _mouse_move_cb, obj);
}

EOLIAN static Eo *
_efl_ui_pager_efl_object_constructor(Eo *obj,
                                     Efl_Ui_Pager_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   elm_widget_sub_object_parent_add(obj);

   pd->cnt = 0;
   pd->loop = EFL_UI_PAGER_LOOP_DISABLED;

   pd->curr.page = 0;
   pd->curr.pos = 0.0;

   elm_widget_can_focus_set(obj, EINA_TRUE);

   return obj;
}

EOLIAN static Eo *
_efl_ui_pager_efl_object_finalize(Eo *obj,
                                  Efl_Ui_Pager_Data *pd)
{
   Efl_Ui_Theme_Apply theme_apply;
   Eo *page_root;

   obj = efl_finalize(efl_super(obj, MY_CLASS));

   theme_apply = efl_ui_layout_theme_set(obj, "pager", "base",
                                         efl_ui_widget_style_get(obj));

   if (theme_apply == EFL_UI_THEME_APPLY_FAILED)
     CRI("Failed to set layout!");

   page_root = efl_add(EFL_CANVAS_GROUP_CLASS, evas_object_evas_get(obj));
   pd->page_root = page_root;
   efl_content_set(efl_part(obj, "page_root"), page_root);

   _event_handler_create(obj, pd);

   return obj;
}

EOLIAN static void
_efl_ui_pager_efl_gfx_size_set(Eo *obj,
                               Efl_Ui_Pager_Data *pd,
                               Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   if ((pd->w == sz.w) && (pd->h == sz.h)) return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), sz);

   pd->w = sz.w;
   pd->h = sz.h;
}

EOLIAN static void
_efl_ui_pager_efl_gfx_position_set(Eo *obj,
                                   Efl_Ui_Pager_Data *pd,
                                   Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   if ((pd->x == pos.x) && (pd->y == pos.y)) return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), pos);

   pd->x = pos.x;
   pd->y = pos.y;
}

EOLIAN static int
_efl_ui_pager_efl_container_content_count(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Pager_Data *pd)
{
   return pd->cnt;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_begin(Eo *obj,
                                         Efl_Ui_Pager_Data *pd,
                                         Efl_Gfx *subobj)
{
   efl_parent_set(subobj, obj);
   elm_widget_sub_object_add(obj, subobj);

   pd->content_list = eina_list_prepend(pd->content_list, subobj);

   pd->cnt += 1;
   pd->curr.page += 1;

   efl_page_transition_update(pd->transition, pd->curr.pos);

   if (pd->indicator)
     {
        efl_page_indicator_pack(pd->indicator, 0);
        efl_page_indicator_update(pd->indicator, pd->curr.pos);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_end(Eo *obj,
                                       Efl_Ui_Pager_Data *pd,
                                       Efl_Gfx *subobj)
{
   efl_parent_set(subobj, obj);
   elm_widget_sub_object_add(obj, subobj);

   pd->content_list = eina_list_append(pd->content_list, subobj);

   pd->cnt += 1;

   efl_page_transition_update(pd->transition, pd->curr.pos);

   if (pd->indicator)
     {
        efl_page_indicator_pack(pd->indicator, (pd->cnt - 1));
        efl_page_indicator_update(pd->indicator, pd->curr.pos);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_before(Eo *obj,
                                          Efl_Ui_Pager_Data *pd,
                                          Efl_Gfx *subobj,
                                          const Efl_Gfx *existing)
{
   int index;

   efl_parent_set(subobj, obj);
   elm_widget_sub_object_add(obj, subobj);

   index = eina_list_data_idx(pd->content_list, (void *)existing);
   pd->content_list = eina_list_prepend_relative(pd->content_list, subobj, existing);

   pd->cnt += 1;
   if (pd->curr.page >= index) pd->curr.page += 1;

   efl_page_transition_update(pd->transition, pd->curr.pos);

   if (pd->indicator)
     {
        efl_page_indicator_pack(pd->indicator, index);
        efl_page_indicator_update(pd->indicator, pd->curr.pos);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_after(Eo *obj,
                                         Efl_Ui_Pager_Data *pd,
                                         Efl_Gfx *subobj,
                                         const Efl_Gfx *existing)
{
   int index;

   efl_parent_set(subobj, obj);
   elm_widget_sub_object_add(obj, subobj);

   index = eina_list_data_idx(pd->content_list, (void *)existing);
   pd->content_list = eina_list_append_relative(pd->content_list, subobj, existing);

   pd->cnt += 1;
   if (pd->curr.page > index) pd->curr.page += 1;

   efl_page_transition_update(pd->transition, pd->curr.pos);

   if (pd->indicator)
     {
        efl_page_indicator_pack(pd->indicator, (index + 1));
        efl_page_indicator_update(pd->indicator, pd->curr.pos);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_pager_efl_pack_linear_pack_at(Eo *obj,
                                      Efl_Ui_Pager_Data *pd,
                                      Efl_Gfx *subobj,
                                      int index)
{
   Efl_Gfx *existing = NULL;

   efl_parent_set(subobj, obj);
   elm_widget_sub_object_add(obj, subobj);

   existing = eina_list_nth(pd->content_list, index);
   pd->content_list = eina_list_prepend_relative(pd->content_list, subobj, existing);

   pd->cnt += 1;
   if (pd->curr.page >= index) pd->curr.page += 1;

   efl_page_transition_update(pd->transition, pd->curr.pos);

   if (pd->indicator)
     {
        efl_page_indicator_pack(pd->indicator, index);
        efl_page_indicator_update(pd->indicator, pd->curr.pos);
     }

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx *
_efl_ui_pager_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                               Efl_Ui_Pager_Data *pd,
                                               int index)
{
   return eina_list_nth(pd->content_list, index);
}

EOLIAN static int
_efl_ui_pager_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Pager_Data *pd,
                                             const Efl_Gfx *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

static Eina_Bool
_change_animator(void *data, double pos)
{
   Eo *obj = data;
   EFL_UI_PAGER_DATA_GET(obj, pd);

   double p, d, temp_pos;
   int temp_page;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   d = pd->change.src + pd->change.delta * p;
   temp_page = d;
   temp_pos = d - temp_page;

   if ((pd->change.delta < 0) && (temp_pos > 0))
     {
        temp_page += 1;
        temp_pos -= 1.0;
     }

   if (pd->curr.page != temp_page)
     {
        if (pd->change.delta < 0)
          efl_page_transition_curr_page_change(pd->transition, -1.0);
        else
          efl_page_transition_curr_page_change(pd->transition, 1.0);
        temp_pos = 0.0;
     }

   pd->curr.page = temp_page;
   pd->curr.pos = temp_pos;

#if DEBUG
   ERR("curr page %d pos %lf", pd->curr.page, pd->curr.pos);
#endif

   _efl_ui_pager_update(pd);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   pd->change.animator = NULL;

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static void
_efl_ui_pager_current_page_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd,
                               int index)
{
   double time;

#if DEBUG
   ERR("curr page %d pos %lf", pd->curr.page, pd->curr.pos);
#endif

   pd->change.src = pd->curr.page + pd->curr.pos;
   pd->change.dst = index;

   pd->change.delta = pd->change.dst - pd->change.src;

#if DEBUG
   ERR("curr page %d pos %lf delta %lf",
       pd->curr.page, pd->curr.pos, pd->change.delta);
#endif

   if (pd->change.delta == 0) return;

   time = pd->change.delta;
   if (pd->change.delta < 0) time *= (-1);
   time /= pd->cnt; //FIXME

   ecore_animator_del(pd->change.animator);
   pd->change.animator = ecore_animator_timeline_add(time, _change_animator, obj);
}

EOLIAN static int
_efl_ui_pager_current_page_get(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd)
{
   return pd->curr.page;
}

EOLIAN static void
_efl_ui_pager_transition_set(Eo *obj EINA_UNUSED,
                             Efl_Ui_Pager_Data *pd,
                             Efl_Page_Transition *transition)
{
   efl_page_transition_bind(transition, obj, pd->page_root);
   pd->transition = transition;
}

EOLIAN static void
_efl_ui_pager_indicator_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd,
                            const Efl_Class *klass)
{
   if (pd->indicator)
     {
        efl_del(pd->indicator);
        pd->indicator = NULL;
     }

   if (!klass)
     {
        efl_del(pd->idbox);
        pd->idbox = NULL;
        return;
     }

   if (!pd->idbox)
     {
        pd->idbox = efl_add(EFL_UI_BOX_CLASS, obj);
        efl_content_set(efl_part(obj, "indicator"), pd->idbox);
     }

   pd->indicator = efl_add(klass, pd->idbox);
}

EOLIAN Eina_Size2D
_efl_ui_pager_page_size_get(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd)
{
   return pd->page_spec.sz;
}

EOLIAN static void
_efl_ui_pager_page_size_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd,
                            Eina_Size2D sz)
{
   if (sz.w < 0 || sz.h < 0) return;

   pd->page_spec.sz = sz;

   efl_page_transition_page_size_set(pd->transition, sz);
}

EOLIAN static int
_efl_ui_pager_padding_get(Eo *obj EINA_UNUSED,
                          Efl_Ui_Pager_Data *pd)
{
   return pd->page_spec.padding;
}

EOLIAN static void
_efl_ui_pager_padding_set(Eo *obj EINA_UNUSED,
                          Efl_Ui_Pager_Data *pd,
                          int padding)
{
   pd->page_spec.padding = padding;

   efl_page_transition_padding_size_set(pd->transition, padding);
}

EOLIAN static void
_efl_ui_pager_scroll_block_get(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd,
                               Eina_Bool *prev,
                               Eina_Bool *next)
{
   if (prev) *prev = pd->prev_block;
   if (next) *next = pd->next_block;
}

EOLIAN static void
_efl_ui_pager_scroll_block_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Pager_Data *pd,
                               Eina_Bool prev,
                               Eina_Bool next)
{
   pd->prev_block = prev;
   pd->next_block = next;
}

EOLIAN static void
_efl_ui_pager_loop_mode_set(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd,
                            Efl_Ui_Pager_Loop loop)
{
   pd->loop = loop;

   efl_page_transition_loop_set(pd->transition, loop);
}

EOLIAN static Efl_Ui_Pager_Loop
_efl_ui_pager_loop_mode_get(Eo *obj EINA_UNUSED,
                            Efl_Ui_Pager_Data *pd)
{
   return pd->loop;
}


#include "efl_ui_pager.eo.c"
