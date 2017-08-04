#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED


#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pagecontrol.h"

#define MY_CLASS EFL_UI_PAGECONTROL_CLASS

#define MY_CLASS_NAME "Efl_Ui_Pagecontrol"



static void
_job(void *data)
{
   Evas_Object *obj = data;
   EFL_UI_PAGECONTROL_DATA_GET(obj, sd);

   double t = 0, ratio = 0;
   int page;

   sd->job = NULL;

   // calculate t based on mouse position
   if (sd->dir == 0 || sd->dir == 1)
     t = ((double)sd->mouse_down.x - (double)sd->mouse_x) / (double)sd->w;
   else if (sd->dir == 2 || sd->dir == 3)
     t = ((double)sd->mouse_down.y - (double)sd->mouse_y) / (double)sd->h;

   if (t > 1.0) t = 1.0;
   else if (t < -1.0) t = -1.0;

   t += sd->mouse_down.ratio;
   t += (double) sd->cnt;
   page = (int) t;
   ratio = t - (double) page;
   page = page % sd->cnt;

   sd->page = (sd->mouse_down.page + page) % sd->cnt;
   sd->ratio = ratio;

   efl_ui_pagecontrol_update(obj);
}

static Eina_Bool
_animator(void *data, double pos)
{
   Evas_Object *obj = data;
   EFL_UI_PAGECONTROL_DATA_GET(obj, sd);
   double p;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);

   if (sd->ratio < 0.5)
     sd->ratio = sd->ratio * (1 - p);
   else
     sd->ratio = 1 - (1 - sd->ratio) * (1 - p);

   //efl_pack_layout_update(obj);
   //ERR("%d %.2lf", sd->page, sd->ratio);
   efl_ui_pagecontrol_update(obj);

   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   if (sd->ratio == 1.0)
     {
        sd->page += 1;
        sd->ratio = 0.0;
     }

   sd->animator = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGECONTROL_DATA_GET(pc, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   ELM_SAFE_FREE(sd->animator, ecore_animator_del);

   sd->move_started = EINA_FALSE;
   sd->mouse_down.enabled = EINA_TRUE;

   sd->mouse_x = ev->canvas.x - sd->x;
   sd->mouse_y = ev->canvas.y - sd->y;
   sd->mouse_down.x = sd->mouse_x;
   sd->mouse_down.y = sd->mouse_y;

   sd->mouse_down.page = sd->page;
   sd->mouse_down.ratio = sd->ratio;
   //ERR("%d %.2lf", sd->mouse_down.page, sd->mouse_down.ratio);
}

static void
_mouse_up_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGECONTROL_DATA_GET(pc, sd);

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->mouse_down.enabled) return;

   sd->mouse_down.enabled = EINA_FALSE;

   ELM_SAFE_FREE(sd->job, ecore_job_del);

   double time;
   if (sd->ratio < 0.5) //target = first;
      time = sd->ratio;
   else //target = second;
      time = 1 - sd->ratio;

   if (time < 0.01) time = 0.01;
   else if (time > 0.99) time = 0.99;

   //animation
   ecore_animator_del(sd->animator);
   sd->animator = ecore_animator_timeline_add(time, _animator, pc);
}

static void
_mouse_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *pc = data;
   EFL_UI_PAGECONTROL_DATA_GET(pc, sd);

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->mouse_down.enabled) return;

   sd->mouse_x = ev->cur.canvas.x - sd->x;
   sd->mouse_y = ev->cur.canvas.y - sd->y;

   if (!sd->move_started)
     {
        //direction decision
        Evas_Coord dx, dy;
        dx = sd->mouse_x - sd->mouse_down.x;
        dy = sd->mouse_y - sd->mouse_down.y;

        if (((dx * dx) + (dy * dy)) <=
            (_elm_config->finger_size * _elm_config->finger_size / 4))
          return;

        if ((dx < 0) && (abs(dx) > abs(dy))) sd->dir = 0; //left
        else if ((dx >= 0) && (abs(dx) > abs(dy))) sd->dir = 1; //right
        else if ((dy < 0) && (abs(dy) >= abs(dx))) sd->dir = 2; //up
        else if ((dy >= 0) && (abs(dy) >= abs(dx))) sd->dir = 3; //down
        else return;

        sd->move_started = EINA_TRUE;
     }

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   ecore_job_del(sd->job);
   sd->job = ecore_job_add(_job, pc);
}

static void
_event_handler_create(Eo *obj, Efl_Ui_Pagecontrol_Data *sd)
{
   sd->event = evas_object_rectangle_add(evas_object_evas_get(obj));

   evas_object_color_set(sd->event, 0, 0, 0, 0);
   efl_gfx_position_set(sd->event, sd->x, sd->y);
   efl_gfx_size_set(sd->event, sd->w, sd->h);
   efl_gfx_visible_set(sd->event, EINA_TRUE);

   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, obj);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up_cb, obj);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move_cb, obj);
}

EOLIAN static void
_efl_ui_pagecontrol_efl_canvas_group_group_add(Eo *obj,
                                               Efl_Ui_Pagecontrol_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   sd->viewport.foreclip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(sd->viewport.foreclip, EINA_TRUE);

   sd->viewport.backclip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(sd->viewport.backclip, EINA_TRUE);

   sd->hidden_clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(sd->hidden_clip, EINA_TRUE);

   _event_handler_create(obj, sd);

   sd->cnt = 0;
   sd->page = 0;
   sd->ratio = 0.0;
}

EOLIAN static Eo *
_efl_ui_pagecontrol_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Pagecontrol_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static void
_efl_ui_pagecontrol_efl_gfx_size_set(Eo *obj,
                                     Efl_Ui_Pagecontrol_Data *sd,
                                     Evas_Coord w,
                                     Evas_Coord h)
{
   if ((sd->w == w) && (sd->h == h)) return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   sd->w = w;
   sd->h = h;

   efl_gfx_size_set(sd->event, w, h);
   efl_gfx_size_set(sd->viewport.foreclip, w, h);
   efl_gfx_size_set(sd->viewport.backclip, w, h);

   efl_ui_pagecontrol_update(obj);
}

EOLIAN static void
_efl_ui_pagecontrol_efl_gfx_position_set(Eo *obj,
                                         Efl_Ui_Pagecontrol_Data *sd,
                                         Evas_Coord x,
                                         Evas_Coord y)
{
   if ((sd->x == x) && (sd->y == y)) return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   sd->x = x;
   sd->y = y;

   efl_gfx_position_set(sd->event, x, y);
   efl_gfx_position_set(sd->viewport.foreclip, x, y);
   efl_gfx_position_set(sd->viewport.backclip, x, y);
}

EOLIAN static int
_efl_ui_pagecontrol_efl_container_content_count(Eo *obj EINA_UNUSED,
                                                Efl_Ui_Pagecontrol_Data *pd)
{
   return eina_list_count(pd->content_list);
}

EOLIAN static Eina_Bool
_efl_ui_pagecontrol_efl_pack_linear_pack_end(Eo *obj,
                                             Efl_Ui_Pagecontrol_Data *pd,
                                             Efl_Gfx *subobj)
{
   efl_parent_set(subobj, obj);
   pd->cnt += 1;

   pd->content_list = eina_list_append(pd->content_list, subobj);
   efl_gfx_stack_raise(pd->event);

   efl_ui_pagecontrol_update(obj);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx *
_efl_ui_pagecontrol_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                                     Efl_Ui_Pagecontrol_Data *pd,
                                                     int index)
{
   return eina_list_nth(pd->content_list, index);
}

EOLIAN static int
_efl_ui_pagecontrol_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                                   Efl_Ui_Pagecontrol_Data *pd,
                                                   const Efl_Gfx *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

EOLIAN static void
_efl_ui_pagecontrol_update(Eo *obj,
                           Efl_Ui_Pagecontrol_Data *pd)
{
   ERR("%d %.2lf", pd->page, pd->ratio);
   efl_event_callback_call(obj, EFL_UI_PAGECONTROL_EVENT_UPDATED, NULL);
}


EOAPI EFL_VOID_FUNC_BODY(efl_ui_pagecontrol_update)

#define EFL_UI_PAGECONTROL_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_pagecontrol), \
   EFL_OBJECT_OP_FUNC(efl_ui_pagecontrol_update, _efl_ui_pagecontrol_update)

#include "efl_ui_pagecontrol.eo.c"
