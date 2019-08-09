#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_animation_view_private.h"
#include "efl_ui_animation_view_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_ANIMATION_VIEW_CLASS

#define MY_CLASS_NAME "Efl_Ui_Animation_View"
#define MY_CLASS_NAME_LEGACY "efl_ui_animation_view"

#define T_SEGMENT_N 60
#define C_SEGMENT_N 60
#define QUEUE_SIZE 500


static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_PLAY_START[] = "play,start";
static const char SIG_PLAY_REPEAT[] = "play,repeat";
static const char SIG_PLAY_DONE[] = "play,done";
static const char SIG_PLAY_PAUSE[] = "play,pause";
static const char SIG_PLAY_RESUME[] = "play,resume";
static const char SIG_PLAY_STOP[] = "play,stop";
static const char SIG_PLAY_UPDATE[] = "play,update";

/* smart callbacks coming from Efl_Ui_Animation_View objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_PLAY_START, ""},
   {SIG_PLAY_REPEAT, ""},
   {SIG_PLAY_DONE, ""},
   {SIG_PLAY_PAUSE, ""},
   {SIG_PLAY_RESUME, ""},
   {SIG_PLAY_STOP, ""},
   {NULL, NULL}
};

#if 0
typedef struct
{
   float x1, x2;
   float y;

} Span_Data;

typedef struct
{
   float x, y;
} Point;

static Point queue[QUEUE_SIZE];
static Eo *proxy_obj[T_SEGMENT_N][T_SEGMENT_N];

Eina_Bool
map_content(Efl_VG *node, const char *id, int off_x, int off_y, Eo* target)
{
   if (!node) return EINA_FALSE;

   if (!efl_isa(node, EFL_CANVAS_VG_CONTAINER_CLASS)) return EINA_FALSE;

   char *name = efl_key_data_get(node, "_lot_node_name");

   //Find the target recursively
   if (!name || strcmp(name, id))
     {
        Eina_Iterator *itr = efl_canvas_vg_container_children_get(node);
        Efl_VG *child;

        EINA_ITERATOR_FOREACH(itr, child)
          {
             if (efl_isa(child, EFL_CANVAS_VG_CONTAINER_CLASS))
               {
                  if (map_content(child, id, off_x, off_y, target)) break;
               }
          }
        return EINA_FALSE;
     }
printf("name = %s\n", name);
   //Find Shape
   Eina_Iterator *itr = efl_canvas_vg_container_children_get(node);
   Efl_VG *child;

   EINA_ITERATOR_FOREACH(itr, child)
     {
        //Filter out unacceptable types
        if (!efl_isa(child, EFL_CANVAS_VG_SHAPE_CLASS)) continue;
        if (efl_gfx_shape_stroke_width_get(child) > 0) continue;

        const Efl_Gfx_Path_Command_Type *cmd;
        const double *points;

        efl_gfx_path_get(child, &cmd, NULL);
        if (!cmd) continue;

        //Fast Path? Shape outlines by consisted of lines.
        int pt_cnt = 0;
        Eina_Bool fast_path = EINA_TRUE;

        if (*cmd != EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO) fast_path = EINA_FALSE;
        else
          {
             ++cmd;

             while (*cmd == EFL_GFX_PATH_COMMAND_TYPE_LINE_TO)
               {
                  ++cmd;
                  if (++pt_cnt > 4)
                    {
                       fast_path = EINA_FALSE;
                       break;
                    }
               }

             if (((*cmd) != EFL_GFX_PATH_COMMAND_TYPE_END) && ((*cmd) != EFL_GFX_PATH_COMMAND_TYPE_CLOSE))
               fast_path = EINA_FALSE;
          }

        efl_gfx_path_get(child, &cmd, &points);

        int rgba;
        efl_gfx_color_get(child, NULL, NULL, NULL, &rgba);

        int pt_idx = 0;
        double x, y;

        //Case 1. Rectangle Mapping
        if (fast_path)
          {
             int map_idx = 1;
             Evas_Map *map = evas_map_new(pt_cnt);

             for (int i = 0; i < pt_cnt; ++i)
               {
                  x = points[pt_idx++] + off_x;
                  y = points[pt_idx++] + off_y;
                  evas_map_point_coord_set(map, map_idx, x, y, 0);
                  evas_map_point_color_set(map, map_idx++, rgba, rgba, rgba, rgba);
                  map_idx %= pt_cnt;
               }

             //Texture Coordinates
             Eina_Rect geom = efl_gfx_entity_geometry_get(target);
             evas_map_point_image_uv_set(map, 0, 0, 0);
             evas_map_point_image_uv_set(map, 1, geom.w, 0);
             evas_map_point_image_uv_set(map, 2, geom.w, geom.h);
             evas_map_point_image_uv_set(map, 3, 0, geom.h);

             evas_object_map_set(target, map);
             evas_object_map_enable_set(target, EINA_TRUE);

             return EINA_TRUE;
          }

        //Case 2. Arbitrary Geometry mapping

        Eina_Bezier bezier;
        float min_y = 999999, max_y = -1;
        double begin_x, begin_y;
        double end_x, end_y;
        double ctrl[4];
        double inv_segment = (1 / (double) C_SEGMENT_N);
        int queue_idx = 0;
        int i;
        float t;

        //end 0, move 1, line 2, cubic 3, close 4
        while (*cmd != EFL_GFX_PATH_COMMAND_TYPE_END)
          {
             if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO)
               {
                  begin_x = points[pt_idx++] + off_x;
                  begin_y = points[pt_idx++] + off_y;
                  if (begin_y < min_y) min_y = begin_y;
                  if (begin_y > max_y) max_y = begin_y;
                  ++cmd;
                  continue;
               }
             else if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO)
               {

                  ctrl[0] = points[pt_idx++] + off_x;
                  ctrl[1] = points[pt_idx++] + off_y;
                  ctrl[2] = points[pt_idx++] + off_x;
                  ctrl[3] = points[pt_idx++] + off_y;
                  end_x = points[pt_idx++] + off_x;
                  end_y = points[pt_idx++] + off_y;

                  eina_bezier_values_set(&bezier,
                                         begin_x, begin_y,
                                         ctrl[0], ctrl[1], ctrl[2], ctrl[3],
                                         end_x, end_y);

                  for (i = 0; i < C_SEGMENT_N; ++i)
                    {
                       t = inv_segment * (double) i;
                       eina_bezier_point_at(&bezier, t, &x, &y);
                       queue[queue_idx].x = x;
                       queue[queue_idx].y = y;

                       if (y < min_y) min_y = y;
                       if (y > max_y) max_y = y;

                       ++queue_idx;
                    }
               }
             else if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_LINE_TO)
               {
                  end_x = points[pt_idx++] + off_x;
                  end_y = points[pt_idx++] + off_y;

                  for (i = 0; i < C_SEGMENT_N; ++i)
                    {
                       t = inv_segment * (double) i;
                       queue[queue_idx].x = begin_x + ((double) (end_x - begin_x)) * t;
                       queue[queue_idx].y = begin_y + ((double) (end_y - begin_y)) * t;
                       ++queue_idx;
                    }
               }

             begin_x = end_x;
             begin_y = end_y;

             if (end_y < min_y) min_y = end_y;
             if (end_y > max_y) max_y = end_y;

             ++cmd;
          }

        queue[queue_idx].x = queue[0].x;
        queue[queue_idx].y = queue[0].y;

        float y_segment = (max_y - min_y) * inv_segment;
        Span_Data spans[T_SEGMENT_N + 1];
        float min_x, max_x;
        float a, b;

        static Eo *lines[T_SEGMENT_N + 1];

        y = min_y;

        for (int i = 0; i <= T_SEGMENT_N; ++i)
          {
             min_x = 999999;
             max_x = -1;

             for (int j = 0; j < queue_idx; ++j)
               {
                  //Horizontal Line
                  if ((fabs(y  - queue[j].y) < 0.5) &&
                      (fabs(queue[j].y - queue[j + 1].y) < 0.5))
                    {
                       if (queue[j].x < min_x) min_x = queue[j].x;
                       if (queue[j].x > max_x) max_x = queue[j].x;
                       if (queue[j + 1].x < min_x) min_x = queue[j + 1].x;
                       if (queue[j + 1].x > max_x) max_x = queue[j + 1].x;
                       continue;
                    }

                  //Out of Y range
                  if (((y < queue[j].y) && (y < queue[j + 1].y)) ||
                      ((y > queue[j].y) && (y > queue[j + 1].y)))
                    continue;

                  //Vertical Line
                  if (fabs(queue[j + 1].x - queue[j + 1].x) < 0.5)
                    x = queue[j].x;
                  //Diagonal Line
                  else
                    {
                       a = (queue[j + 1].y - queue[j].y) / (queue[j + 1].x - queue[j].x);
                       b = queue[j].y - (a * queue[j].x);
                       x = (y - b) / a;
                    }

                  if (x < min_x) min_x = x;
                  if (x > max_x) max_x = x;
               }
             spans[i].x1 = min_x;
             spans[i].x2 = max_x;
             spans[i].y = y;
             y += y_segment;
#if 0
             if (!lines[i]) lines[i] = evas_object_line_add(evas_object_evas_get(target));
             evas_object_color_set(lines[i], 255, 0, 0, 255);
             evas_object_resize(lines[i], 1000, 1000);
             evas_object_line_xy_set(lines[i], spans[i].x1, spans[i].y, spans[i].x2, spans[i].y);
             evas_object_show(lines[i]);
#endif
          }

        Evas *evas = evas_object_evas_get(target);
        int w, h;
        evas_object_geometry_get(target, NULL, NULL, &w, &h);

        float u_segment = ((float) w) / ((float) T_SEGMENT_N);
        float v_segment = ((float) h) / ((float) T_SEGMENT_N);

        for (int i = 0; i < T_SEGMENT_N; ++i)
          {
             float x1_segment = (spans[i].x2 - spans[i].x1) / ((float) T_SEGMENT_N);
             float x2_segment = (spans[i + 1].x2 - spans[i + 1].x1) / ((float) T_SEGMENT_N);

             for (int j = 0; j < T_SEGMENT_N; ++j)
               {
                  if (!proxy_obj[i][j])
                    {
                       proxy_obj[i][j] = evas_object_image_filled_add(evas);
                       evas_object_image_source_set(proxy_obj[i][j], target);
                       evas_object_image_source_events_set(proxy_obj[i][j], EINA_TRUE);
                       evas_object_move(proxy_obj[i][j], -1000, -1000);
                       evas_object_resize(proxy_obj[i][j], w, h);
                       evas_object_show(proxy_obj[i][j]);
                    }

                  Evas_Map *map = evas_map_new(4);

                  evas_map_point_coord_set(map, 0, spans[i].x1 + ((float) j) * x1_segment, spans[i].y, 0);
                  evas_map_point_coord_set(map, 1, spans[i].x1 + ((float) j + 1) * x1_segment, spans[i].y, 0);
                  evas_map_point_coord_set(map, 2, spans[i + 1].x1 + ((float) j + 1) * x2_segment, spans[i + 1].y, 0);
                  evas_map_point_coord_set(map, 3, spans[i + 1].x1 + ((float) j) * x2_segment, spans[i + 1].y, 0);

                  evas_map_point_image_uv_set(map, 0, ((float) j) * u_segment, ((float) i) * v_segment);
                  evas_map_point_image_uv_set(map, 1, ((float) j + 1) * u_segment, ((float) i) * v_segment);
                  evas_map_point_image_uv_set(map, 2, ((float) j + 1) * u_segment, ((float) i + 1) * v_segment);
                  evas_map_point_image_uv_set(map, 3, ((float) j) * u_segment, ((float) i + 1) * v_segment);

                  evas_map_point_color_set(map, 0, rgba, rgba, rgba, rgba);
                  evas_map_point_color_set(map, 1, rgba, rgba, rgba, rgba);
                  evas_map_point_color_set(map, 2, rgba, rgba, rgba, rgba);
                  evas_map_point_color_set(map, 3, rgba, rgba, rgba, rgba);

                  evas_object_map_enable_set(proxy_obj[i][j], EINA_TRUE);
                  evas_object_map_set(proxy_obj[i][j], map);
               }
          }

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_update_map_content(Eo *vg)
{
   Efl_VG *root = evas_object_vg_root_node_get(vg);
   if (!root) return;
   int x, y;
   evas_object_geometry_get(vg, &x, &y, 0, 0);
   map_content(root, "tizen_mi_obj", x, y, map_obj);
   map_content(root, "tizen_mi_obj2", x, y, map_obj2);
}

#endif


static void
_sizing_eval(void *data)
{
   Efl_Ui_Animation_View_Data *pd = data;
   if (!efl_file_loaded_get(pd->obj)) return;

   double hw,hh;
   efl_gfx_hint_weight_get(pd->obj, &hw, &hh);

   Eina_Size2D size = efl_canvas_vg_object_default_size_get(pd->vg);

   Eina_Size2D min = {-1, -1};
   if (hw == 0) min.w = size.w;
   if (hh == 0) min.h = size.h;

   efl_gfx_hint_size_min_set(pd->obj, min);
}

static void
_size_hint_event_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   _sizing_eval(data);
}

static void
_transit_go_facade(Efl_Ui_Animation_View_Data *pd)
{
   pd->repeat_times = 0;
   if (pd->play_back)
     pd->state = EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK;
   else
     pd->state = EFL_UI_ANIMATION_VIEW_STATE_PLAY;
   evas_object_smart_callback_call(pd->obj, SIG_PLAY_START, NULL);
   if (pd->transit) elm_transit_go(pd->transit);
}

static Eina_Bool
_visible_check(Eo *obj)
{
   if (!efl_gfx_entity_visible_get(obj)) return EINA_FALSE;

   //TODO: Check Smart parents visibilities?

   Eina_Size2D size = efl_gfx_entity_size_get(obj);
   if (size.w == 0 || size.h == 0) return EINA_FALSE;

   Evas_Coord output_w, output_h;
   evas_output_size_get(evas_object_evas_get(obj), &output_w, &output_h);

   Eina_Position2D pos = efl_gfx_entity_position_get(obj);

   //Outside viewport
   if ((pos.x + size.w < 0) || (pos.x > output_w) ||
       (pos.y + size.h < 0) || (pos.y > output_h))
     return EINA_FALSE;

   //Inside viewport
   return EINA_TRUE;
}

static void
_auto_play(Efl_Ui_Animation_View_Data *pd, Eina_Bool vis)
{
   if (!pd->auto_play || !pd->transit) return;

   //Resume Animation
   if (vis)
     {
        if (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PAUSE && pd->auto_play_pause)
          {
             elm_transit_paused_set(pd->transit, EINA_FALSE);
             if (pd->play_back)
               pd->state = EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK;
             else
               pd->state = EFL_UI_ANIMATION_VIEW_STATE_PLAY;
             pd->auto_play_pause = EINA_FALSE;
             evas_object_smart_callback_call(pd->obj, SIG_PLAY_RESUME, NULL);
          }
     }
   //Pause Animation
   else
     {
        if ((pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY) ||
            (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK))
          {
             elm_transit_paused_set(pd->transit, EINA_TRUE);
             pd->state = EFL_UI_ANIMATION_VIEW_STATE_PAUSE;
             pd->auto_play_pause = EINA_TRUE;
             evas_object_smart_callback_call(pd->obj, SIG_PLAY_PAUSE, NULL);
          }
     }
}

static void
_transit_del_cb(Elm_Transit_Effect *effect, Elm_Transit *transit)
{
   Efl_Ui_Animation_View_Data *pd = (Efl_Ui_Animation_View_Data *) effect;

   if ((pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY && pd->progress == 1) ||
       (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK && pd->progress == 0))
     evas_object_smart_callback_call(pd->obj, SIG_PLAY_DONE, NULL);

   if (pd->transit != transit) return;

   Efl_Ui_Animation_View_State prev_state = pd->state;
   pd->state = EFL_UI_ANIMATION_VIEW_STATE_STOP;
   pd->transit = NULL;
   pd->auto_play_pause = EINA_FALSE;

   if (prev_state != EFL_UI_ANIMATION_VIEW_STATE_STOP)
     {
        evas_object_smart_callback_call(pd->obj, SIG_PLAY_STOP, NULL);
        pd->progress = 0;
     }
}

static void
_transit_cb(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   Efl_Ui_Animation_View_Data *pd = (Efl_Ui_Animation_View_Data *) effect;

   if (!pd->vg)
     {
        ERR("Vector Object is removed in wrong way!, Efl_Ui_Animation_View = %p", pd->obj);
        elm_transit_del(transit);
        return;
     }

   if (pd->play_back)
     {
        pd->state = EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK;
        progress = 1 - progress;
     }
   else pd->state = EFL_UI_ANIMATION_VIEW_STATE_PLAY;

   pd->progress = progress;
   int minframe = (pd->frame_cnt - 1) * pd->min_progress;
   int maxframe = (pd->frame_cnt - 1) * pd->max_progress;

   int update_frame = (int)((maxframe - minframe) * progress) + minframe;
   int current_frame = evas_object_vg_animated_frame_get(pd->vg);
   evas_object_vg_animated_frame_set(pd->vg, update_frame);

   if (pd->auto_repeat)
     {
        int repeat_times = elm_transit_current_repeat_times_get(pd->transit);
        if (pd->repeat_times != repeat_times)
          {
             evas_object_smart_callback_call(pd->obj, SIG_PLAY_REPEAT, NULL);
             pd->repeat_times = repeat_times;
          }
     }

   //transit_cb is always called with a progress value 0 ~ 1.
   //SIG_PLAY_UPDATE callback is called only when there is a real change.
   if (update_frame != current_frame)
     evas_object_smart_callback_call(pd->obj, SIG_PLAY_UPDATE, NULL);
}

EOLIAN static void
_efl_ui_animation_view_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Animation_View_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   // Create vg to render vector animation
   Eo *vg = evas_object_vg_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, vg);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _size_hint_event_cb, priv);

   priv->vg = vg;
   priv->speed = 1;
   priv->frame_duration = 0;
   priv->min_progress = 0.0;
   priv->max_progress = 1.0;
}

EOLIAN static void
_efl_ui_animation_view_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Animation_View_Data *pd EINA_UNUSED)
{
   if (pd->transit)
     {
        Elm_Transit *transit = pd->transit;
        pd->transit = NULL;   //Skip perform transit_del_cb()
        elm_transit_del(transit);
     }
   pd->state = EFL_UI_ANIMATION_VIEW_STATE_NOT_READY;

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_animation_view_efl_object_destructor(Eo *obj,
                                          Efl_Ui_Animation_View_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_animation_view_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Animation_View_Data *pd)
{
   pd->obj = obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   return obj;
}

static void
_update_frame_duration(Efl_Ui_Animation_View_Data *pd)
{
   int frame_count = evas_object_vg_animated_frame_count_get(pd->vg);
   int min_frame = (frame_count - 1) * pd->min_progress;
   int max_frame = (frame_count - 1) * pd->max_progress;
   double frame_rate = round((double)frame_count / evas_object_vg_animated_frame_duration_get(pd->vg, 0, 0));

   pd->frame_duration = (double)(max_frame - min_frame) / frame_rate;
   if (pd->transit)
     elm_transit_duration_set(pd->transit, pd->frame_duration * (1/pd->speed));
}

static Eina_Bool
_ready_play(Efl_Ui_Animation_View_Data *pd)
{
   pd->auto_play_pause = EINA_FALSE;
   pd->state = EFL_UI_ANIMATION_VIEW_STATE_STOP;

   if (pd->transit) elm_transit_del(pd->transit);

   pd->frame_cnt = (double) evas_object_vg_animated_frame_count_get(pd->vg);
   pd->frame_duration = evas_object_vg_animated_frame_duration_get(pd->vg, 0, 0);
   evas_object_vg_animated_frame_set(pd->vg, 0);

   if (pd->frame_duration > 0)
     {
        Elm_Transit *transit = elm_transit_add();
        elm_transit_object_add(transit, pd->vg);
        if (pd->auto_repeat) elm_transit_repeat_times_set(transit, -1);
        elm_transit_effect_add(transit, _transit_cb, pd, _transit_del_cb);
        elm_transit_progress_value_set(transit, pd->progress);
        elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);
        pd->transit = transit;
        if (pd->min_progress != 0.0 || pd->max_progress != 1.0)
          _update_frame_duration(pd);
        else
          elm_transit_duration_set(transit, pd->frame_duration * (1/pd->speed));

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_animation_view_efl_file_unload(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   pd->state = EFL_UI_ANIMATION_VIEW_STATE_NOT_READY;
   pd->frame_cnt = 0;
   pd->frame_duration = 0;
   if (pd->transit) elm_transit_del(pd->transit);
}

EOLIAN static Eina_Error
_efl_ui_animation_view_efl_file_load(Eo *obj, Efl_Ui_Animation_View_Data *pd)
{
   Eina_Error err;
   const char *file;
   const char *key;

   if (efl_file_loaded_get(obj)) return 0;

   err = efl_file_load(efl_super(obj, MY_CLASS));
   if (err) return err;

   file = efl_file_get(obj);
   key = efl_file_key_get(obj);
   efl_file_simple_load(pd->vg, file, key);

   pd->progress = 0;

   _sizing_eval(pd);

   if (!_ready_play(pd)) return 1;

   if (pd->auto_play)
     {
        _transit_go_facade(pd);

        if (!_visible_check(obj))
          {
             elm_transit_paused_set(pd->transit, EINA_TRUE);
             pd->state = EFL_UI_ANIMATION_VIEW_STATE_PAUSE;
             pd->auto_play_pause = EINA_TRUE;
             evas_object_smart_callback_call(pd->obj, SIG_PLAY_PAUSE, NULL);
          }
     }
   return 0;
}

EOLIAN static void
_efl_ui_animation_view_efl_gfx_entity_position_set(Eo *obj,
                                                Efl_Ui_Animation_View_Data *pd,
                                                Eina_Position2D pos EINA_UNUSED)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);

   _auto_play(pd, _visible_check(obj));
}

EOLIAN static void
_efl_ui_animation_view_efl_gfx_entity_size_set(Eo *obj,
                                            Efl_Ui_Animation_View_Data *pd,
                                            Eina_Size2D size)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, size.w, size.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), size);

   _sizing_eval(pd);

   _auto_play(pd, _visible_check(obj));
}

EOLIAN static void
_efl_ui_animation_view_efl_gfx_entity_visible_set(Eo *obj,
                                               Efl_Ui_Animation_View_Data *pd,
                                               Eina_Bool vis)
{
  if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);

   _auto_play(pd, _visible_check(obj));
}

EOLIAN static void
_efl_ui_animation_view_auto_repeat_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd, Eina_Bool auto_repeat)
{
   if (pd->auto_repeat == auto_repeat) return;
   pd->auto_repeat = auto_repeat;
   if (pd->transit)
     {
        if (auto_repeat) elm_transit_repeat_times_set(pd->transit, -1);
        else elm_transit_repeat_times_set(pd->transit, 0);
     }
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_auto_repeat_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->auto_repeat;
}

EOLIAN static void
_efl_ui_animation_view_auto_play_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd,
                                  Eina_Bool auto_play)
{
   pd->auto_play = auto_play;
   if (!auto_play) pd->auto_play_pause = EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_auto_play_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->auto_play;
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_play(Eo *obj, Efl_Ui_Animation_View_Data *pd)
{
   if (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY) return EINA_FALSE;

   Eina_Bool rewind = EINA_FALSE;
   if (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK) rewind = EINA_TRUE;

   pd->play_back = EINA_FALSE;
   pd->auto_play_pause = EINA_FALSE;

   if (!efl_file_loaded_get(obj)) return EINA_FALSE;
   if (!pd->transit && !_ready_play(pd)) return EINA_FALSE;

   if (pd->state == EFL_UI_ANIMATION_VIEW_STATE_STOP)
     _transit_go_facade(pd);
   else if (rewind)
     elm_transit_progress_value_set(pd->transit, pd->progress);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_stop(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   if (!pd->transit) return EINA_FALSE;

   if ((pd->state == EFL_UI_ANIMATION_VIEW_STATE_NOT_READY) ||
       (pd->state == EFL_UI_ANIMATION_VIEW_STATE_STOP))
     return EINA_FALSE;

   evas_object_vg_animated_frame_set(pd->vg, 0);
   pd->progress = 0;
   pd->state = EFL_UI_ANIMATION_VIEW_STATE_STOP;
   evas_object_smart_callback_call(pd->obj, SIG_PLAY_STOP, NULL);
   elm_transit_del(pd->transit);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_pause(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   if (!pd->transit) return EINA_FALSE;

   if ((pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY) ||
       (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK))
     {
        elm_transit_paused_set(pd->transit, EINA_TRUE);
        pd->state = EFL_UI_ANIMATION_VIEW_STATE_PAUSE;
        pd->auto_play_pause = EINA_FALSE;
        evas_object_smart_callback_call(pd->obj, SIG_PLAY_PAUSE, NULL);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_resume(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   if (!pd->transit) return EINA_FALSE;

   if (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PAUSE)
     {
        elm_transit_paused_set(pd->transit, EINA_FALSE);
        if (pd->play_back)
          pd->state = EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK;
        else
          pd->state = EFL_UI_ANIMATION_VIEW_STATE_PLAY;
        pd->auto_play_pause = EINA_FALSE;

        evas_object_smart_callback_call(pd->obj, SIG_PLAY_RESUME, NULL);

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_play_back(Eo *obj, Efl_Ui_Animation_View_Data *pd)
{
   if (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK) return EINA_FALSE;

   Eina_Bool rewind = EINA_FALSE;
   if (pd->state == EFL_UI_ANIMATION_VIEW_STATE_PLAY) rewind = EINA_TRUE;

   pd->play_back = EINA_TRUE;
   pd->auto_play_pause = EINA_FALSE;

   if (!efl_file_loaded_get(obj)) return EINA_FALSE;
   if (!pd->transit && !_ready_play(pd)) return EINA_FALSE;

   if (pd->state == EFL_UI_ANIMATION_VIEW_STATE_STOP)
     {
        if (pd->progress == 0) pd->progress = 1.0;
        _transit_go_facade(pd);
     }
   else if (rewind)
     elm_transit_progress_value_set(pd->transit, 1 - pd->progress);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_speed_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd, double speed)
{
   if (speed <= 0) return EINA_FALSE;
   pd->speed = speed;

   if (pd->transit)
     elm_transit_duration_set(pd->transit, pd->frame_duration * (1/pd->speed));

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_animation_view_progress_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd, double progress)
{
   if (progress < 0) progress = 0;
   else if (progress > 1) progress = 1;
   if (pd->progress == progress) return;

   pd->progress = progress;

   if (pd->frame_cnt > 0)
     evas_object_vg_animated_frame_set(pd->vg, (int) ((pd->frame_cnt - 1) * progress));

   if (pd->transit)
     {
        if (pd->play_back)
          elm_transit_progress_value_set(pd->transit, 1 - progress);
        else
          elm_transit_progress_value_set(pd->transit, progress);
     }
}

EOLIAN static double
_efl_ui_animation_view_progress_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->progress;
}

EOLIAN static void
_efl_ui_animation_view_frame_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd, int frame_num)
{
   efl_ui_animation_view_progress_set(obj, (double) frame_num / (double) (evas_object_vg_animated_frame_count_get(pd->vg) - 1));
}

EOLIAN static int
_efl_ui_animation_view_frame_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   double progress = (pd->progress * (pd->max_progress - pd->min_progress)) +  pd->min_progress;
   return (int) ((double) (evas_object_vg_animated_frame_count_get(pd->vg) - 1) * progress);
}

EOLIAN static double
_efl_ui_animation_view_speed_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->speed;
}

EOLIAN static double
_efl_ui_animation_view_duration_time_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->frame_duration;
}

EOLIAN static Eina_Size2D
_efl_ui_animation_view_default_size_get(const Eo *obj EINA_UNUSED,
                                     Efl_Ui_Animation_View_Data *pd EINA_UNUSED)
{
   return efl_canvas_vg_object_default_size_get(pd->vg);
}

EOLIAN static Efl_Ui_Animation_View_State
_efl_ui_animation_view_state_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->state;
}

EOLIAN static Eina_Bool
_efl_ui_animation_view_is_playing_back(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->play_back;
}

EOLIAN static int
_efl_ui_animation_view_frame_count_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return evas_object_vg_animated_frame_count_get(pd->vg);
}

EOLIAN static void
_efl_ui_animation_view_min_progress_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd, double min_progress)
{
   if (min_progress < 0.0 || min_progress > 1.0 || min_progress > pd->max_progress) return;

   pd->min_progress = min_progress;
   _update_frame_duration(pd);
}

EOLIAN static double
_efl_ui_animation_view_min_progress_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->min_progress;
}

EOLIAN static void
_efl_ui_animation_view_max_progress_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd, double max_progress)
{
   if (max_progress < 0.0 || max_progress > 1.0 || max_progress < pd->min_progress) return;

   pd->max_progress = max_progress;
   _update_frame_duration(pd);
}

EOLIAN static double
_efl_ui_animation_view_max_progress_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->max_progress;
}

EOLIAN static void
_efl_ui_animation_view_min_frame_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd, int min_frame)
{
   int frame_count = evas_object_vg_animated_frame_count_get(pd->vg);
   if (min_frame < 0) min_frame = 0;
   else
     {
        int max_frame = (frame_count - 1) * pd->max_progress;
        if (min_frame > max_frame) min_frame = max_frame;
     }

   pd->min_progress = (double)min_frame / (double)(frame_count - 1);
   _update_frame_duration(pd);
}

EOLIAN static int
_efl_ui_animation_view_min_frame_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->min_progress * (evas_object_vg_animated_frame_count_get(pd->vg) - 1);
}

EOLIAN static void
_efl_ui_animation_view_max_frame_set(Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd, int max_frame)
{
   int frame_count = evas_object_vg_animated_frame_count_get(pd->vg);
   if (max_frame > frame_count - 1) max_frame = frame_count - 1;
   else
     {
        int min_frame = (frame_count - 1) * pd->min_progress;
        if (min_frame > max_frame) max_frame = min_frame;
     }

   pd->max_progress = (double)max_frame / (double)(frame_count - 1);
   _update_frame_duration(pd);
}

EOLIAN static int
_efl_ui_animation_view_max_frame_get(const Eo *obj EINA_UNUSED, Efl_Ui_Animation_View_Data *pd)
{
   return pd->max_progress * (evas_object_vg_animated_frame_count_get(pd->vg) - 1);
}

EOLIAN static Efl_Object *
_efl_ui_animation_view_efl_part_part_get(const Eo* obj, Efl_Ui_Animation_View_Data *pd, const char *part)
{
   return ELM_PART_IMPLEMENT(EFL_UI_ANIMATION_VIEW_PART_CLASS, obj, part);
}

EAPI Elm_Animation_View*
elm_animation_view_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EAPI Eina_Bool
elm_animation_view_file_set(Elm_Animation_View *obj, const char *file, const char *key)
{
   return efl_file_simple_load(obj, file, key);
}

EAPI Elm_Animation_View_State
elm_animation_view_state_get(Elm_Animation_View *obj)
{
   return efl_ui_animation_view_state_get(obj);
}

static Eina_Bool
_efl_ui_animation_view_content_set(Eo *obj, Efl_Ui_Animation_View_Data *pd, const char *part, Efl_Gfx_Entity *content)
{
   return EINA_FALSE;
}

static Efl_Gfx_Entity *
_efl_ui_animation_view_content_get(const Eo *obj, Efl_Ui_Animation_View_Data *pd, const char *part)
{
   return NULL;
}

static Efl_Gfx_Entity *
_efl_ui_animation_view_content_unset(Eo *obj, Efl_Ui_Animation_View_Data *pd, const char *part)
{
   return NULL;
}

/* Efl.Part begin */
ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_animation_view, EFL_UI_ANIMATION_VIEW, Efl_Ui_Animation_View_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_animation_view, EFL_UI_ANIMATION_VIEW, Efl_Ui_Animation_View_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_animation_view, EFL_UI_ANIMATION_VIEW, Efl_Ui_Animation_View_Data)
#include "efl_ui_animation_view_part.eo.c"
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_ANIMATION_VIEW_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_animation_view)

#include "efl_ui_animation_view_eo.legacy.c"
#include "efl_ui_animation_view.eo.c"
