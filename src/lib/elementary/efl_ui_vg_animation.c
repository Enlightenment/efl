#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_vg_animation_private.h"
#include "efl_ui_vg_animation_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_VG_ANIMATION_CLASS

#define MY_CLASS_NAME "Efl_Ui_Vg_Animation"

#define GROW_SIZE 50
#define QUEUE_SIZE 350
#define T_SEGMENT_N 30
#define C_SEGMENT_N 30


static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_PLAY_START[] = "play,start";
static const char SIG_PLAY_REPEAT[] = "play,repeat";
static const char SIG_PLAY_DONE[] = "play,done";
static const char SIG_PLAY_PAUSE[] = "play,pause";
static const char SIG_PLAY_RESUME[] = "play,resume";
static const char SIG_PLAY_STOP[] = "play,stop";
static const char SIG_PLAY_UPDATE[] = "play,update";

/* smart callbacks coming from Efl_Ui_Vg_Animation objects: */
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

typedef struct
{
   Eina_Stringshare *part;
   Eo *obj;
   Eo *proxy;
   const Efl_VG *node;
} Efl_Ui_Vg_Animation_Sub_Obj_Data;

typedef struct
{
   float x1, x2;
   float y;
} Efl_Ui_Vg_Animation_Span_Data;

typedef struct
{
   float x, y;
} Efl_Ui_Vg_Animation_Point;

static Eo *
_proxy_create(Eo *source)
{
   Eo *proxy = efl_add(EFL_CANVAS_PROXY_CLASS, source);
   if (!proxy) return NULL;

   efl_gfx_fill_auto_set(proxy, EINA_TRUE);
   efl_canvas_proxy_source_clip_set(proxy, EINA_FALSE);
   efl_canvas_proxy_source_set(proxy, source);

   efl_gfx_entity_visible_set(proxy, EINA_FALSE);
   efl_gfx_entity_visible_set(source, EINA_FALSE);

   return proxy;
}

static void
_proxy_map_disable(Efl_Ui_Vg_Animation_Data *pd)
{
   Eina_List *l;
   Efl_Ui_Vg_Animation_Sub_Obj_Data *sub_d;

   EINA_LIST_FOREACH(pd->subs, l, sub_d)
     {
        if (!sub_d->proxy) continue;
        if (efl_gfx_mapping_has(sub_d->proxy))
          efl_gfx_mapping_reset(sub_d->proxy);

        //TODO: remove this call
        evas_object_map_enable_set(sub_d->proxy, EINA_FALSE);
     }
}

static Efl_VG *
_node_get(Efl_VG *node, const char *part)
{
   if (!node) return NULL;

   if (!efl_isa(node, EFL_CANVAS_VG_CONTAINER_CLASS)) return NULL;

   char *name = efl_key_data_get(node, "_lot_node_name");

   //Find the target recursiveldy
   if (!name || strcmp(name, part))
     {
        Eina_Iterator *itr = efl_canvas_vg_container_children_get(node);
        Efl_VG *child;
        Efl_VG *ret;

        EINA_ITERATOR_FOREACH(itr, child)
          {
             if (efl_isa(child, EFL_CANVAS_VG_CONTAINER_CLASS))
               {
                  ret = _node_get(child, part);
                  if (ret) return ret;
               }
          }
        return NULL;
     }

   //Find Shape
   Eina_Iterator *itr = efl_canvas_vg_container_children_get(node);
   Efl_VG *child;
   const Efl_Gfx_Path_Command_Type *cmd;

   EINA_ITERATOR_FOREACH(itr, child)
     {
        //Filter out unacceptable types
        if (!efl_isa(child, EFL_CANVAS_VG_SHAPE_CLASS)) continue;
        if (efl_gfx_shape_stroke_width_get(child) > 0) continue;
        efl_gfx_path_get(child, &cmd, NULL);
        if (!cmd) continue;
        return child;
     }
   return NULL;
}

static Eina_Bool
_part_draw(Efl_Ui_Vg_Animation_Sub_Obj_Data *sub_d, Eina_Position2D offset, Eina_Size2D tsize)
{
   const Efl_Gfx_Path_Command_Type *cmd, *tcmd;
   const double *points;
   int alpha, inarray_idx, inarray_size, i, pt_idx = 0, pt_cnt = 0;
   double x, y;
   Eina_Bezier bezier;
   float t, min_y = 99999999, max_y = -1;
   float inv_segment = (1 / (float) C_SEGMENT_N);
   float u_segment, v_segment, x1_segment, x2_segment;
   double begin_x = 0, begin_y = 0, end_x = 0, end_y  = 0;
   double ctrl[4];
   Eina_Inarray *inarray;
   Efl_Ui_Vg_Animation_Point *pt, *pt2;
   Eo *target = sub_d->proxy;
   Eina_Bool fast_path = EINA_TRUE;

   efl_gfx_path_get(sub_d->node, &cmd, &points);
   if (!cmd) return EINA_FALSE;

   efl_gfx_entity_visible_set(target, EINA_TRUE);
//   efl_gfx_path_bounds_get(sub_d->node, &tbound);
   efl_gfx_color_get(sub_d->node, NULL, NULL, NULL, &alpha);
   efl_gfx_entity_size_set(target, tsize);

   //TODO: Optimize it, scalable or not?
   efl_gfx_entity_size_set(sub_d->obj, tsize);

   //Fast Path? Shape outlines by consisted of straight lines.
   tcmd = cmd;
   if (*tcmd != EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO) fast_path = EINA_FALSE;
   else
     {
        ++tcmd;
        while (*tcmd == EFL_GFX_PATH_COMMAND_TYPE_LINE_TO)
          {
             ++tcmd;
             if (++pt_cnt > 4)
               {
                  fast_path = EINA_FALSE;
                  break;
               }
          }

        if ((pt_cnt != 4) ||
            (((*tcmd) != EFL_GFX_PATH_COMMAND_TYPE_END) && ((*tcmd) != EFL_GFX_PATH_COMMAND_TYPE_CLOSE)))
          fast_path = EINA_FALSE;
     }

   pt_idx = 0;

   //Fast Path: Rectangle Mapping
   if (fast_path)
     {
        efl_gfx_mapping_point_count_set(target, pt_cnt);

        //Point
        efl_gfx_mapping_coord_absolute_set(target, 1, points[0] + offset.x, points[1] + offset.y, 0);
        efl_gfx_mapping_coord_absolute_set(target, 2, points[2] + offset.x, points[3] + offset.y, 0);
        efl_gfx_mapping_coord_absolute_set(target, 3, points[4] + offset.x, points[5] + offset.y, 0);
        efl_gfx_mapping_coord_absolute_set(target, 0, points[6] + offset.x, points[7] + offset.y, 0);

        //Color
        efl_gfx_mapping_color_set(target, 0, alpha, alpha, alpha, alpha);
        efl_gfx_mapping_color_set(target, 1, alpha, alpha, alpha, alpha);
        efl_gfx_mapping_color_set(target, 2, alpha, alpha, alpha, alpha);
        efl_gfx_mapping_color_set(target, 3, alpha, alpha, alpha, alpha);

        //UV
        efl_gfx_mapping_uv_set(target, 0, 0, 0);
        efl_gfx_mapping_uv_set(target, 1, tsize.w, 0);
        efl_gfx_mapping_uv_set(target, 2, tsize.w, tsize.h);
        efl_gfx_mapping_uv_set(target, 3, 0, tsize.h);

        return EINA_TRUE;
     }

   //Case 2. Arbitrary Geometry mapping
   inarray = eina_inarray_new(sizeof(Efl_Ui_Vg_Animation_Point), GROW_SIZE);
   if (!inarray) return EINA_FALSE;
   eina_inarray_resize(inarray, QUEUE_SIZE);
   inarray_size = QUEUE_SIZE;
   inarray_idx = 0;

   //end 0, move 1, line 2, cubic 3, close 4
   while (*cmd != EFL_GFX_PATH_COMMAND_TYPE_END)
     {
        if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO)
          {
             begin_x = points[pt_idx++] + offset.x;
             begin_y = points[pt_idx++] + offset.y;
             if (begin_y < min_y) min_y = begin_y;
             if (begin_y > max_y) max_y = begin_y;
             ++cmd;
             continue;
          }
        else if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO)
          {

             ctrl[0] = points[pt_idx++] + offset.x;
             ctrl[1] = points[pt_idx++] + offset.y;
             ctrl[2] = points[pt_idx++] + offset.x;
             ctrl[3] = points[pt_idx++] + offset.y;
             end_x = points[pt_idx++] + offset.x;
             end_y = points[pt_idx++] + offset.y;

             eina_bezier_values_set(&bezier,
                                    begin_x, begin_y,
                                    ctrl[0], ctrl[1], ctrl[2], ctrl[3],
                                    end_x, end_y);

             for (i = 0; i < C_SEGMENT_N; ++i)
               {
                  if (inarray_idx >= inarray_size)
                    {
                       inarray_size += GROW_SIZE;
                       eina_inarray_grow(inarray, inarray_size);
                    }

                  t = inv_segment * (float) i;
                  eina_bezier_point_at(&bezier, t, &x, &y);
                  pt = eina_inarray_nth(inarray, inarray_idx);
                  pt->x = x;
                  pt->y = y;

                  if (y < min_y) min_y = y;
                  if (y > max_y) max_y = y;

                  ++inarray_idx;
               }
          }
        else if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_LINE_TO)
          {
             end_x = points[pt_idx++] + offset.x;
             end_y = points[pt_idx++] + offset.y;

             for (i = 0; i < C_SEGMENT_N; ++i)
               {
                  if (inarray_idx >= inarray_size)
                    {
                       inarray_size += GROW_SIZE;
                       eina_inarray_grow(inarray, inarray_size);
                    }

                  t = inv_segment * (float) i;
                  pt = eina_inarray_nth(inarray, inarray_idx);
                  pt->x = begin_x + ((double) (end_x - begin_x)) * t;
                  pt->y = begin_y + ((double) (end_y - begin_y)) * t;
                  ++inarray_idx;
               }
          }

        begin_x = end_x;
        begin_y = end_y;

        if (end_y < min_y) min_y = end_y;
        if (end_y > max_y) max_y = end_y;

        ++cmd;
     }

   if (inarray_idx >= inarray_size)
     {
        inarray_size += 1;
        eina_inarray_grow(inarray, inarray_size);
     }
   pt = eina_inarray_nth(inarray, inarray_idx);
   pt2 = eina_inarray_nth(inarray, 0);
   pt->x = pt2->x;
   pt->y = pt2->y;

   float y_segment = (max_y - min_y) * inv_segment;
   Efl_Ui_Vg_Animation_Span_Data spans[T_SEGMENT_N + 1];
   float min_x, max_x;
   float a, b;

   y = min_y;

   for (int i = 0; i <= T_SEGMENT_N; ++i)
     {
        min_x = 999999;
        max_x = -1;

        for (int j = 0; j < inarray_idx; ++j)
          {
             pt = eina_inarray_nth(inarray, j);
             pt2 = pt + 1;

             //Horizontal Line
             if ((fabs(y  - pt->y) < 0.5) &&
                 (fabs(pt->y - pt2->y) < 0.5))
               {
                  if (pt->x < min_x) min_x = pt->x;
                  if (pt->x > max_x) max_x = pt->x;
                  if (pt2->x < min_x) min_x = pt2->x;
                  if (pt2->x > max_x) max_x = pt2->x;
                  continue;
               }

             //Out of Y range
             if (((y < pt->y) && (y < pt2->y)) ||
                 ((y > pt->y) && (y > pt2->y)))
               continue;

             //Vertical Line
             if (fabs(pt2->x - pt2->x) < 0.5)
               x = pt->x;
             //Diagonal Line
             else
               {
                  a = (pt2->y - pt->y) / (pt2->x - pt->x);
                  b = pt->y - (a * pt->x);
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
        static Eo *lines[T_SEGMENT_N + 1];
        if (!lines[i]) lines[i] = evas_object_line_add(evas_object_evas_get(target));
        evas_object_color_set(lines[i], 255, 255, 255, 255);
        evas_object_resize(lines[i], 1000, 1000);
        evas_object_line_xy_set(lines[i], spans[i].x1, spans[i].y, spans[i].x2, spans[i].y);
        evas_object_show(lines[i]);
#endif
     }

   u_segment = ((float) tsize.w) / ((float) T_SEGMENT_N);
   v_segment = ((float) tsize.h) / ((float) T_SEGMENT_N);

   Evas_Map *map = evas_map_new(T_SEGMENT_N * T_SEGMENT_N * 4);
   pt_idx = 0;
//   efl_gfx_mapping_point_count_set(target, (4 * T_SEGMENT_N * T_SEGMENT_N));

   for (int i = 0; i < T_SEGMENT_N; ++i)
     {
        x1_segment = (spans[i].x2 - spans[i].x1) / ((float) T_SEGMENT_N);
        x2_segment = (spans[i + 1].x2 - spans[i + 1].x1) / ((float) T_SEGMENT_N);

        for (int j = 0; j < T_SEGMENT_N; ++j)
          {
#if 0
             //Point
             efl_gfx_mapping_coord_absolute_set(target, pt_idx + 0, spans[i].x1 + ((float) j) * x1_segment, spans[i].y, 0);
             efl_gfx_mapping_coord_absolute_set(target, pt_idx + 1, spans[i].x1 + ((float) j + 1) * x1_segment, spans[i].y, 0);
             efl_gfx_mapping_coord_absolute_set(target, pt_idx + 2, spans[i + 1].x1 + ((float) j + 1) * x2_segment, spans[i + 1].y, 0);
             efl_gfx_mapping_coord_absolute_set(target, pt_idx + 3, spans[i + 1].x1 + ((float) j) * x2_segment, spans[i + 1].y, 0);

             //UV
             efl_gfx_mapping_uv_set(target, pt_idx + 0, ((float) j) * u_segment, ((float) i) * v_segment);
             efl_gfx_mapping_uv_set(target, pt_idx + 1, ((float) j + 1) * u_segment, ((float) i) * v_segment);
             efl_gfx_mapping_uv_set(target, pt_idx + 2, ((float) j + 1) * u_segment, ((float) i + 1) * v_segment);
             efl_gfx_mapping_uv_set(target, pt_idx + 3, ((float) j) * u_segment, ((float) i + 1) * v_segment);

             //Color
             efl_gfx_mapping_color_set(target, pt_idx + 0, alpha, alpha, alpha, alpha);
             efl_gfx_mapping_color_set(target, pt_idx + 1, alpha, alpha, alpha, alpha);
             efl_gfx_mapping_color_set(target, pt_idx + 2, alpha, alpha, alpha, alpha);
             efl_gfx_mapping_color_set(target, pt_idx + 3, alpha, alpha, alpha, alpha);
#endif
             evas_map_point_coord_set(map, pt_idx + 0, spans[i].x1 + ((float) j) * x1_segment, spans[i].y, 0);
             evas_map_point_coord_set(map, pt_idx + 1, spans[i].x1 + ((float) j + 1) * x1_segment, spans[i].y, 0);
             evas_map_point_coord_set(map, pt_idx + 2, spans[i + 1].x1 + ((float) j + 1) * x2_segment, spans[i + 1].y, 0);
             evas_map_point_coord_set(map, pt_idx + 3, spans[i + 1].x1 + ((float) j) * x2_segment, spans[i + 1].y, 0);

             evas_map_point_image_uv_set(map, pt_idx + 0, ((float) j) * u_segment, ((float) i) * v_segment);
             evas_map_point_image_uv_set(map, pt_idx + 1, ((float) j + 1) * u_segment, ((float) i) * v_segment);
             evas_map_point_image_uv_set(map, pt_idx + 2, ((float) j + 1) * u_segment, ((float) i + 1) * v_segment);
             evas_map_point_image_uv_set(map, pt_idx + 3, ((float) j) * u_segment, ((float) i + 1) * v_segment);

             evas_map_point_color_set(map, pt_idx + 0, alpha, alpha, alpha, alpha);
             evas_map_point_color_set(map, pt_idx + 1, alpha, alpha, alpha, alpha);
             evas_map_point_color_set(map, pt_idx + 2, alpha, alpha, alpha, alpha);
             evas_map_point_color_set(map, pt_idx + 3, alpha, alpha, alpha, alpha);

             pt_idx += 4;
          }
     }
   evas_object_map_enable_set(target, EINA_TRUE);
   evas_object_map_set(target, map);

   evas_map_free(map);
   eina_inarray_free(inarray);

   return EINA_TRUE;
}

static void
_update_part_contents(Eo *obj, Efl_Ui_Vg_Animation_Data *pd)
{
   if (!pd->subs) return;

   //TODO: Update to current frame's node tree immediately.

   Efl_VG *root = evas_object_vg_root_node_get(pd->vg);
   if (!root) return;

   Eina_List *l;
   Eina_Position2D pos = efl_gfx_entity_position_get(pd->vg);
   Efl_Ui_Vg_Animation_Sub_Obj_Data *sub_d;
   Eina_Size2D tsize = efl_gfx_entity_size_get(obj);

   EINA_LIST_FOREACH(pd->subs, l, sub_d)
     {
        if (!sub_d->node)
          sub_d->node = _node_get(root, sub_d->part);

        if (sub_d->node)
          _part_draw(sub_d, pos, tsize);
        else
          ERR("part(%s) is invalid in Efl_Ui_Vg_Animation(%p)", sub_d->part, obj);
     }
}

static void
_frame_set_facade(Eo *obj, Efl_Ui_Vg_Animation_Data *pd, int frame)
{
   int pframe = evas_object_vg_animated_frame_get(pd->vg);
   evas_object_vg_animated_frame_set(pd->vg, frame);
   if (pframe != frame) _update_part_contents(obj, pd);
}

static void
_eo_unparent_helper(Eo *child, Eo *parent)
{
   if (efl_parent_get(child) == parent)
     efl_parent_set(child, evas_object_evas_get(parent));
}

static void
_on_sub_object_size_hint_change(void *data,
                                Evas *e EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED)
{
   if (!efl_alive_get(data)) return;
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   efl_canvas_group_change(data);
   //TODO: Update part contents
}

static void
_sizing_eval(Eo *obj, void *data)
{
   Efl_Ui_Vg_Animation_Data *pd = data;
   if (!efl_file_loaded_get(obj)) return;

   double hw,hh;
   efl_gfx_hint_weight_get(obj, &hw, &hh);

   Eina_Size2D size = efl_canvas_vg_object_default_size_get(pd->vg);

   Eina_Size2D min = {-1, -1};
   if (hw == 0) min.w = size.w;
   if (hh == 0) min.h = size.h;

   efl_gfx_hint_size_min_set(obj, min);
}

static void
_size_hint_event_cb(void *data, const Efl_Event *event)
{
   _sizing_eval(event->object, data);
}

static void
_transit_go_facade(Eo* obj, Efl_Ui_Vg_Animation_Data *pd)
{
   Eina_Bool playing = EINA_TRUE;
   pd->repeat_times = 0;
   if (pd->playing_reverse)
     pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS;
   else
     pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING;
   if (elm_widget_is_legacy(obj))
     evas_object_smart_callback_call(obj, SIG_PLAY_START, NULL);
   else
     efl_event_callback_call(obj, EFL_PLAYER_EVENT_PLAYING_CHANGED, &playing);
   if (pd->transit) elm_transit_go(pd->transit);}

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
_autoplay(Eo *obj, Efl_Ui_Vg_Animation_Data *pd, Eina_Bool vis)
{
   if (!pd->autoplay || !pd->transit) return;

   //Resume Animation
   if (vis)
     {
        if (pd->state == EFL_UI_VG_ANIMATION_STATE_PAUSED && pd->autoplay_pause)
          {
             Eina_Bool paused = EINA_FALSE;
             elm_transit_paused_set(pd->transit, EINA_FALSE);
             if (pd->playing_reverse)
               pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS;
             else
               pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING;
             pd->autoplay_pause = EINA_FALSE;
             if (elm_widget_is_legacy(obj))
               evas_object_smart_callback_call(obj, SIG_PLAY_RESUME, NULL);
             else
               efl_event_callback_call(obj, EFL_PLAYER_EVENT_PAUSED_CHANGED, &paused);
          }
     }
   //Pause Animation
   else
     {
        Eina_Bool paused = EINA_TRUE;
        if ((pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING) ||
            (pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS))
          {
             elm_transit_paused_set(pd->transit, EINA_TRUE);
             pd->state = EFL_UI_VG_ANIMATION_STATE_PAUSED;
             pd->autoplay_pause = EINA_TRUE;
             if (elm_widget_is_legacy(obj))
               evas_object_smart_callback_call(obj, SIG_PLAY_PAUSE, NULL);
             else
               efl_event_callback_call(obj, EFL_PLAYER_EVENT_PAUSED_CHANGED, &paused);
          }
     }
}

static void
_transit_del_cb(Elm_Transit_Effect *effect, Elm_Transit *transit)
{
   Eo *obj = (Eo *) effect;
   EFL_UI_VG_ANIMATION_DATA_GET(obj, pd);
   if (!pd) return;

   if ((pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING && pd->progress == 1) ||
       (pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS && pd->progress == 0))
     {
        if (elm_widget_is_legacy(obj))
          evas_object_smart_callback_call(obj, SIG_PLAY_DONE, NULL);
        else
          efl_event_callback_call(obj, EFL_PLAYER_EVENT_PLAYBACK_FINISHED, NULL);
     }

   if (pd->transit != transit) return;

   Efl_Ui_Vg_Animation_State prev_state = pd->state;
   pd->state = EFL_UI_VG_ANIMATION_STATE_STOPPED;
   pd->transit = NULL;
   pd->autoplay_pause = EINA_FALSE;

   if (prev_state != EFL_UI_VG_ANIMATION_STATE_STOPPED)
     {
        Eina_Bool playing = EINA_FALSE;
        if (elm_widget_is_legacy(obj))
          evas_object_smart_callback_call(obj, SIG_PLAY_STOP, NULL);
        else
          efl_event_callback_call(obj, EFL_PLAYER_EVENT_PLAYING_CHANGED, &playing);
        pd->progress = 0;
     }
}

static void
_transit_cb(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   Eo *obj = (Eo *) effect;
   EFL_UI_VG_ANIMATION_DATA_GET(obj, pd);

   if (!pd || !pd->vg)
     {
        ERR("Vector Object is removed in wrong way!, Efl_Ui_Vg_Animation = %p", obj);
        elm_transit_del(transit);
        return;
     }
   if (pd->playback_direction_changed)
     {
        elm_transit_progress_value_set(pd->transit, 1 - progress);
        progress = 1 - progress ;

        if (pd->playback_speed <= 0) pd->playing_reverse = EINA_TRUE;
        else pd->playing_reverse = EINA_FALSE;

        pd->playback_direction_changed = EINA_FALSE;
     }

   if (pd->playing_reverse)
     {
        pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS;
        progress = 1 - progress;
     }
   else pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING;

   pd->progress = progress;
   int minframe = (pd->frame_cnt - 1) * pd->min_progress;
   int maxframe = (pd->frame_cnt - 1) * pd->max_progress;

   int update_frame = (int)((maxframe - minframe) * progress) + minframe;
   int current_frame = evas_object_vg_animated_frame_get(pd->vg);

   if (pd->playback_speed == 0)
     update_frame = current_frame;

   _frame_set_facade(obj, pd, update_frame);

   if (pd->loop)
     {
        int repeat_times = elm_transit_current_repeat_times_get(pd->transit);
        if (pd->repeat_times != repeat_times)
          {
             if (elm_widget_is_legacy(obj))
               evas_object_smart_callback_call(obj, SIG_PLAY_REPEAT, NULL);
             else
               efl_event_callback_call(obj, EFL_PLAYER_EVENT_PLAYBACK_REPEATED, &repeat_times);
             pd->repeat_times = repeat_times;
          }
     }

   //transit_cb is always called with a progress value 0 ~ 1.
   //SIG_PLAY_UPDATE callback is called only when there is a real change.
   if (update_frame == current_frame) return;
   if (elm_widget_is_legacy(obj))
     evas_object_smart_callback_call(obj, SIG_PLAY_UPDATE, NULL);
   else
     {
        double position = pd->frame_duration * pd->progress;
        efl_event_callback_call(obj, EFL_PLAYER_EVENT_PLAYBACK_POSITION_CHANGED, &position);
        efl_event_callback_call(obj, EFL_PLAYER_EVENT_PLAYBACK_PROGRESS_CHANGED, &pd->progress);
     }
}

EOLIAN static void
_efl_ui_vg_animation_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Vg_Animation_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   // Create vg to render vector animation
   Eo *vg = evas_object_vg_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, vg);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _size_hint_event_cb, priv);

   //temporary
   efl_key_data_set(obj, "vg_obj", vg);
   //

   priv->vg = vg;
   priv->playback_speed = 1;
   priv->frame_duration = 0;
   priv->min_progress = 0.0;
   priv->max_progress = 1.0;
}

EOLIAN static void
_efl_ui_vg_animation_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Vg_Animation_Data *pd EINA_UNUSED)
{
   if (pd->transit)
     {
        Elm_Transit *transit = pd->transit;
        pd->transit = NULL;   //Skip perform transit_del_cb()
        elm_transit_del(transit);
     }
   pd->state = EFL_UI_VG_ANIMATION_STATE_NOT_READY;

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_vg_animation_efl_object_destructor(Eo *obj,
                                          Efl_Ui_Vg_Animation_Data *pd EINA_UNUSED)
{
   Efl_Gfx_Vg_Value_Provider *vp;
   EINA_LIST_FREE(pd->vp_list, vp)
     efl_unref(vp);
   eina_list_free(pd->vp_list);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_vg_animation_efl_object_constructor(Eo *obj,
                                           Efl_Ui_Vg_Animation_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   return obj;
}

static void
_update_frame_duration(Efl_Ui_Vg_Animation_Data *pd)
{
   int frame_count = evas_object_vg_animated_frame_count_get(pd->vg);
   int min_frame = (frame_count - 1) * pd->min_progress;
   int max_frame = (frame_count - 1) * pd->max_progress;
   double frame_rate = round((double)frame_count / evas_object_vg_animated_frame_duration_get(pd->vg, 0, 0));
   double speed = pd->playback_speed < 0 ? pd->playback_speed * -1 : pd->playback_speed;

   pd->frame_duration = (double)(max_frame - min_frame) / frame_rate;
   if (pd->transit)
     elm_transit_duration_set(pd->transit, speed != 0 ? pd->frame_duration * (1 / speed) : 0);
}

static Eina_Bool
_ready_play(Eo *obj, Efl_Ui_Vg_Animation_Data *pd)
{
   pd->autoplay_pause = EINA_FALSE;
   pd->state = EFL_UI_VG_ANIMATION_STATE_STOPPED;

   if (pd->transit) elm_transit_del(pd->transit);

   pd->frame_cnt = (double) evas_object_vg_animated_frame_count_get(pd->vg);
   pd->frame_duration = evas_object_vg_animated_frame_duration_get(pd->vg, 0, 0);
   _frame_set_facade(obj, pd, 0);

   if (pd->frame_duration > 0)
     {
        double speed = pd->playback_speed < 0 ? pd->playback_speed * -1 : pd->playback_speed;
        Elm_Transit *transit = elm_transit_add();
        elm_transit_object_add(transit, pd->vg);
        if (pd->loop) elm_transit_repeat_times_set(transit, -1);
        elm_transit_effect_add(transit, _transit_cb, obj, _transit_del_cb);
        elm_transit_progress_value_set(transit, pd->progress);
        elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);
        pd->transit = transit;
        if (pd->min_progress != 0.0 || pd->max_progress != 1.0)
          _update_frame_duration(pd);
        else
          elm_transit_duration_set(transit, speed != 0 ? pd->frame_duration * (1 / speed) : 0);

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_vg_animation_efl_file_unload(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   pd->state = EFL_UI_VG_ANIMATION_STATE_NOT_READY;
   pd->frame_cnt = 0;
   pd->frame_duration = 0;
   if (pd->transit) elm_transit_del(pd->transit);

   //Remove all part exising contents
   Efl_Ui_Vg_Animation_Sub_Obj_Data *sub_d;
   Eo *content;
   Eina_List *l, *ll;

   EINA_LIST_FOREACH_SAFE(pd->subs, l, ll, sub_d)
     {
        content = sub_d->obj;

        /* sub_d will die in _widget_sub_object_del */
        if (!_elm_widget_sub_object_redirect_to_top(obj, content))
          {
             ERR("could not remove sub object %p from %p", content, obj);
             continue;
          }
        _eo_unparent_helper(content, obj);
     }
}

EOLIAN static Eina_Error
_efl_ui_vg_animation_efl_file_load(Eo *obj, Efl_Ui_Vg_Animation_Data *pd)
{
   Eina_Error err;
   Eina_Bool ret;
   const char *file;
   const char *key;

   if (efl_file_loaded_get(obj)) return 0;

   err = efl_file_load(efl_super(obj, MY_CLASS));
   if (err) return err;

   file = efl_file_get(obj);
   key = efl_file_key_get(obj);
   ret = efl_file_simple_load(pd->vg, file, key);
   if (!ret)
     {
        efl_file_unload(obj);
        return eina_error_get();
     }

   pd->progress = 0;

   _sizing_eval(obj, pd);

   if (!_ready_play(obj, pd)) return 1;

   if (pd->autoplay)
     {
        _transit_go_facade(obj, pd);

        if (!_visible_check(obj))
          {
             Eina_Bool paused = EINA_TRUE;
             elm_transit_paused_set(pd->transit, EINA_TRUE);
             pd->state = EFL_UI_VG_ANIMATION_STATE_PAUSED;
             pd->autoplay_pause = EINA_TRUE;
             if (elm_widget_is_legacy(obj))
               evas_object_smart_callback_call(obj, SIG_PLAY_PAUSE, NULL);
             else
               efl_event_callback_call(obj, EFL_PLAYER_EVENT_PAUSED_CHANGED, &paused);
          }
     }
   return 0;
}

EOLIAN static void
_efl_ui_vg_animation_efl_gfx_entity_position_set(Eo *obj,
                                                Efl_Ui_Vg_Animation_Data *pd,
                                                Eina_Position2D pos EINA_UNUSED)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);

   Eina_Bool vis = _visible_check(obj);
   if (!vis) _proxy_map_disable(pd);
   _autoplay(obj, pd, vis);
}

EOLIAN static void
_efl_ui_vg_animation_efl_gfx_entity_size_set(Eo *obj,
                                            Efl_Ui_Vg_Animation_Data *pd,
                                            Eina_Size2D size)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, size.w, size.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), size);

   _sizing_eval(obj, pd);

   _autoplay(obj, pd, _visible_check(obj));
}

EOLIAN static void
_efl_ui_vg_animation_efl_gfx_entity_visible_set(Eo *obj,
                                                  Efl_Ui_Vg_Animation_Data *pd,
                                                  Eina_Bool vis)
{
  if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);

   vis = _visible_check(obj);
   if (!vis) _proxy_map_disable(pd);
   _autoplay(obj, pd, vis);
}

EOLIAN static void
_efl_ui_vg_animation_efl_gfx_view_view_size_set(Eo *obj EINA_UNUSED,
                                                  Efl_Ui_Vg_Animation_Data *pd,
                                                  Eina_Size2D size)
{
   Eina_Rect viewbox;
   viewbox.x = viewbox.y =0;
   viewbox.w = size.w;
   viewbox.h = size.h;

   efl_canvas_vg_object_viewbox_set(pd->vg, viewbox);
}

EOLIAN Eina_Size2D
_efl_ui_vg_animation_efl_gfx_view_view_size_get(const Eo *obj EINA_UNUSED,
                                                  Efl_Ui_Vg_Animation_Data *pd)
{
   Eina_Rect viewbox = efl_canvas_vg_object_viewbox_get(pd->vg);

   return EINA_SIZE2D(viewbox.w, viewbox.h);
}

EOLIAN static void
_efl_ui_vg_animation_efl_player_playback_loop_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, Eina_Bool loop)
{
   if (pd->loop == loop) return;
   pd->loop = loop;
   if (pd->transit)
     {
        if (loop) elm_transit_repeat_times_set(pd->transit, -1);
        else elm_transit_repeat_times_set(pd->transit, 0);
     }
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_player_playback_loop_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->loop;
}

EOLIAN static void
_efl_ui_vg_animation_efl_player_autoplay_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd,
                                  Eina_Bool autoplay)
{
   pd->autoplay = autoplay;
   if (!autoplay) pd->autoplay_pause = EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_player_autoplay_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->autoplay;
}

Eina_Bool _efl_ui_vg_animation_playing_sector(Eo *obj, Efl_Ui_Vg_Animation_Data *pd, const char *start, const char *end)
{
   int start_frame = 0;
   int end_frame = evas_object_vg_animated_frame_count_get(pd->vg) - 1;

   if (start && end)
     {
        efl_gfx_frame_controller_sector_get(pd->vg, start, &start_frame, NULL);
        efl_gfx_frame_controller_sector_get(pd->vg, end, &end_frame, NULL);
     }
   else
     {
        if (start)
          {
             efl_gfx_frame_controller_sector_get(pd->vg, start, &start_frame, &end_frame);
          }
        else if (end)
          {
             efl_gfx_frame_controller_sector_get(pd->vg, end, &end_frame, NULL);
          }
     }

   efl_ui_vg_animation_min_frame_set(obj, start_frame);
   if (start_frame < end_frame)
      efl_ui_vg_animation_max_frame_set(obj, end_frame);

   if (!efl_player_playing_set(obj, EINA_TRUE))
      return EINA_FALSE;
   return EINA_TRUE;
}

Eina_Bool
_playing_stop(Eo* obj, Efl_Ui_Vg_Animation_Data *pd)
{
   Eina_Bool playing = EINA_FALSE;
   if (!pd->transit) return EINA_FALSE;

   if ((pd->state == EFL_UI_VG_ANIMATION_STATE_NOT_READY) ||
       (pd->state == EFL_UI_VG_ANIMATION_STATE_STOPPED))
     return EINA_FALSE;

   _frame_set_facade(obj, pd, 0);

   pd->progress = 0;
   pd->state = EFL_UI_VG_ANIMATION_STATE_STOPPED;
   if (elm_widget_is_legacy(obj))
     evas_object_smart_callback_call(obj, SIG_PLAY_STOP, NULL);
   else
     efl_event_callback_call(obj, EFL_PLAYER_EVENT_PLAYING_CHANGED, &playing);

   elm_transit_del(pd->transit);
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_vg_animation_frame_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, int frame_num)
{
   efl_player_playback_progress_set(obj, (double) frame_num / (double) (evas_object_vg_animated_frame_count_get(pd->vg) - 1));
}

EOLIAN static int
_efl_ui_vg_animation_frame_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   double progress = (pd->progress * (pd->max_progress - pd->min_progress)) +  pd->min_progress;
   return (int) ((double) (evas_object_vg_animated_frame_count_get(pd->vg) - 1) * progress);
}

EOLIAN static Eina_Size2D
_efl_ui_vg_animation_default_view_size_get(const Eo *obj EINA_UNUSED,
                                             Efl_Ui_Vg_Animation_Data *pd EINA_UNUSED)
{
   return efl_canvas_vg_object_default_size_get(pd->vg);
}

EOLIAN static Efl_Ui_Vg_Animation_State
_efl_ui_vg_animation_state_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->state;
}

EOLIAN static int
_efl_ui_vg_animation_frame_count_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return efl_gfx_frame_controller_frame_count_get(pd->vg);
}

EOLIAN static void
_efl_ui_vg_animation_min_progress_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, double min_progress)
{
   if (min_progress < 0.0 || min_progress > 1.0 || min_progress > pd->max_progress) return;

   pd->min_progress = min_progress;
   _update_frame_duration(pd);
}

EOLIAN static double
_efl_ui_vg_animation_min_progress_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->min_progress;
}

EOLIAN static void
_efl_ui_vg_animation_max_progress_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, double max_progress)
{
   if (max_progress < 0.0 || max_progress > 1.0 || max_progress < pd->min_progress) return;

   pd->max_progress = max_progress;
   _update_frame_duration(pd);
}

EOLIAN static double
_efl_ui_vg_animation_max_progress_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->max_progress;
}

EOLIAN static void
_efl_ui_vg_animation_min_frame_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, int min_frame)
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
_efl_ui_vg_animation_min_frame_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->min_progress * (evas_object_vg_animated_frame_count_get(pd->vg) - 1);
}

EOLIAN static void
_efl_ui_vg_animation_max_frame_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, int max_frame)
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
_efl_ui_vg_animation_max_frame_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->max_progress * (evas_object_vg_animated_frame_count_get(pd->vg) - 1);
}

EOLIAN static Efl_Object *
_efl_ui_vg_animation_efl_part_part_get(const Eo* obj, Efl_Ui_Vg_Animation_Data *pd EINA_UNUSED, const char *part)
{
   return ELM_PART_IMPLEMENT(EFL_UI_VG_ANIMATION_PART_CLASS, obj, part);
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_ui_widget_widget_sub_object_add(Eo *obj, Efl_Ui_Vg_Animation_Data *pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   if (evas_object_data_get(sobj, "elm-parent") == obj) return EINA_TRUE;

   int_ret = elm_widget_sub_object_add(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   evas_object_event_callback_add
         (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _on_sub_object_size_hint_change, obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_ui_widget_widget_sub_object_del(Eo *obj, Efl_Ui_Vg_Animation_Data *pd, Evas_Object *sobj)
{
   Eina_List *l;
   Efl_Ui_Vg_Animation_Sub_Obj_Data *sub_d;
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   evas_object_event_callback_del_full
     (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_sub_object_size_hint_change, obj);

   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;
//   if (pd->destructed_is) return EINA_TRUE;

   EINA_LIST_FOREACH(pd->subs, l, sub_d)
     {
        if (sub_d->obj != sobj) continue;
        pd->subs = eina_list_remove_list(pd->subs, l);
        if (sub_d->proxy) efl_del(sub_d->proxy);
        eina_stringshare_del(sub_d->part);
        free(sub_d);
        break;
     }

   // No need to resize object during destruction
   if (wd->resize_obj && efl_alive_get(obj))
     efl_canvas_group_change(obj);

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_vg_animation_value_provider_override(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, Efl_Gfx_Vg_Value_Provider *value_provider)
{
   if (!value_provider) return;

   if (pd->vp_list)
     {
        const char *keypath1 = efl_gfx_vg_value_provider_keypath_get(value_provider);
        if (!keypath1)
          {
             ERR("Couldn't override Value Provider(%p). Keypath is NULL.", value_provider);
             return;
          }
        const Eina_List *l;
        Efl_Gfx_Vg_Value_Provider *_vp;
        EINA_LIST_FOREACH(pd->vp_list, l, _vp)
          {
             const char *keypath2 = efl_gfx_vg_value_provider_keypath_get(_vp);
             if (!strcmp(keypath1, keypath2))
               {
                  pd->vp_list = eina_list_remove(pd->vp_list, _vp);
                  efl_unref(_vp);
                  break;
               }
          }
     }

   efl_ref(value_provider);
   pd->vp_list = eina_list_append(pd->vp_list, value_provider);
   efl_key_data_set(pd->vg, "_vg_value_providers", pd->vp_list);
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_player_playing_set(Eo *obj, Efl_Ui_Vg_Animation_Data *pd, Eina_Bool playing)
{
   if (playing)
     {
        if ((pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING && pd->playback_speed > 0)
           || (pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS && pd->playback_speed <= 0))
          return EINA_FALSE;

        Eina_Bool rewind = EINA_FALSE;
        if ((pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING && pd->playback_speed <= 0)
           || (pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS && pd->playback_speed > 0))
          rewind = EINA_TRUE;

        if (pd->playback_speed <= 0)
          pd->playing_reverse = EINA_TRUE;
        else
          pd->playing_reverse = EINA_FALSE;
        pd->autoplay_pause = EINA_FALSE;


        if (!efl_file_loaded_get(obj)) return EINA_FALSE;
        if (!pd->transit && !_ready_play(obj, pd)) return EINA_FALSE;


        if (pd->state == EFL_UI_VG_ANIMATION_STATE_STOPPED)
          {
             if (pd->playing_reverse && pd->progress == 0) pd->progress = 1.0;
             _transit_go_facade(obj, pd);
          }
        else if (rewind)
          {
             elm_transit_progress_value_set(pd->transit, pd->playing_reverse ? 1 - pd->progress : pd->progress);
             pd->playback_direction_changed = EINA_FALSE;
          }
     }
   else
     {
        return _playing_stop(obj, pd);
     }
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_player_playing_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   if (pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING)
     return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_player_paused_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, Eina_Bool paused)
{
   paused = !!paused;
   if (paused)
     {
        if ((pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING) ||
            (pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS))
          {
             elm_transit_paused_set(pd->transit, paused);
             pd->state = EFL_UI_VG_ANIMATION_STATE_PAUSED;
             pd->autoplay_pause = EINA_FALSE;
             if (elm_widget_is_legacy(obj))
               evas_object_smart_callback_call(obj, SIG_PLAY_PAUSE, NULL);
             else
               efl_event_callback_call(obj, EFL_PLAYER_EVENT_PAUSED_CHANGED, &paused);
          }
     }
   else
     {
        if (pd->transit && pd->state == EFL_UI_VG_ANIMATION_STATE_PAUSED)
          {
             elm_transit_paused_set(pd->transit, paused);
             if (pd->playing_reverse)
               pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS;
             else
               pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING;
             pd->autoplay_pause = EINA_FALSE;
             if (elm_widget_is_legacy(obj))
               evas_object_smart_callback_call(obj, SIG_PLAY_RESUME, NULL);
             else
               efl_event_callback_call(obj, EFL_PLAYER_EVENT_PAUSED_CHANGED, &paused);
          }
     }
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_player_paused_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   if (pd->state == EFL_UI_VG_ANIMATION_STATE_PAUSED)
     return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_vg_animation_efl_player_playback_position_set(Eo *obj, Efl_Ui_Vg_Animation_Data *pd, double sec)
{
   EINA_SAFETY_ON_TRUE_RETURN(sec < 0);
   EINA_SAFETY_ON_TRUE_RETURN(sec > pd->frame_duration);

   efl_player_playback_progress_set(obj, pd->frame_duration != 0 ? sec / pd->frame_duration : 0);
}

EOLIAN static double
_efl_ui_vg_animation_efl_player_playback_position_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->frame_duration * pd->progress;
}

EOLIAN static double
_efl_ui_vg_animation_efl_player_playback_progress_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->progress;
}

EOLIAN static void
_efl_ui_vg_animation_efl_player_playback_progress_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, double progress)
{
   if (progress < 0) progress = 0;
   else if (progress > 1) progress = 1;
   if (pd->progress == progress) return;

   pd->progress = progress;

   if (pd->frame_cnt > 0)
     evas_object_vg_animated_frame_set(pd->vg, (int) ((pd->frame_cnt - 1) * progress));

   if (pd->transit)
     {
        if (pd->playing_reverse)
          elm_transit_progress_value_set(pd->transit, 1 - progress);
        else
          elm_transit_progress_value_set(pd->transit, progress);
     }
}

EOLIAN static void
_efl_ui_vg_animation_efl_player_playback_speed_set(Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, double speed)
{
   Eina_Bool rewind = EINA_FALSE;

   if ((pd->playback_speed > 0 && speed < 0) || (pd->playback_speed < 0 && speed > 0))
     rewind = EINA_TRUE;

   // pd->playback_direction_changed is used only during playback.
   if (pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING && rewind)
     {
        pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS;
        pd->playback_direction_changed = EINA_TRUE;
     }
   else if (pd->state == EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS && rewind)
     {
        pd->state = EFL_UI_VG_ANIMATION_STATE_PLAYING;
        pd->playback_direction_changed = EINA_TRUE;
     }

   pd->playback_speed = speed;
   speed = speed < 0 ? speed * -1 : speed;
   if (pd->transit)
     elm_transit_duration_set(pd->transit, pd->playback_speed != 0 ? pd->frame_duration * (1 / speed) : 0);
}

EOLIAN static double
_efl_ui_vg_animation_efl_player_playback_speed_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->playback_speed;
}

EOLIAN static double
_efl_ui_vg_animation_efl_playable_length_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd)
{
   return pd->frame_duration;
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_playable_playable_get(const Eo *obj, Efl_Ui_Vg_Animation_Data *pd EINA_UNUSED)
{
   if (!efl_file_loaded_get(obj)) return EINA_FALSE;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_vg_animation_efl_playable_seekable_get(const Eo *obj, Efl_Ui_Vg_Animation_Data *pd EINA_UNUSED)
{
   if (!efl_file_loaded_get(obj)) return EINA_FALSE;
   return EINA_TRUE;
}

#define MY_CLASS_NAME_LEGACY "elm_animation_view"

static void
_efl_ui_vg_animation_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_vg_animation_legacy_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_VG_ANIMATION_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   return obj;
}

EAPI Elm_Animation_View*
elm_animation_view_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_VG_ANIMATION_LEGACY_CLASS, parent);
}

EAPI Eina_Bool
elm_animation_view_file_set(Elm_Animation_View *obj, const char *file, const char *key)
{
   return efl_file_simple_load(obj, file, key);
}

EAPI Elm_Animation_View_State
elm_animation_view_state_get(Elm_Animation_View *obj)
{
   Elm_Animation_View_State state = ELM_ANIMATION_VIEW_STATE_NOT_READY;

   switch (efl_ui_vg_animation_state_get(obj))
     {
      case EFL_UI_VG_ANIMATION_STATE_PLAYING:
         state = ELM_ANIMATION_VIEW_STATE_PLAY;
         break;
      case EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS:
         state = ELM_ANIMATION_VIEW_STATE_PLAY_BACK;
         break;
      case EFL_UI_VG_ANIMATION_STATE_PAUSED:
         state = ELM_ANIMATION_VIEW_STATE_PAUSE;
         break;
      case EFL_UI_VG_ANIMATION_STATE_STOPPED:
         state = ELM_ANIMATION_VIEW_STATE_STOP;
         break;
      case EFL_UI_VG_ANIMATION_STATE_NOT_READY:
      default:
         break;
     }
   return state;
}

static Eina_Bool
_efl_ui_vg_animation_content_set(Eo *obj, Efl_Ui_Vg_Animation_Data *pd, const char *part, Efl_Gfx_Entity *content)
{
   Efl_Ui_Vg_Animation_Sub_Obj_Data *sub_d;
   Eina_List *l;

   if (!efl_file_loaded_get(obj)) return EINA_FALSE;

   EINA_LIST_FOREACH(pd->subs, l, sub_d)
     {
        if (!strcmp(part, sub_d->part))
          {
             if (content == sub_d->obj) goto end;
             if (efl_alive_get(sub_d->obj))
               {
                  if (sub_d->proxy) efl_del(sub_d->proxy);
                  _eo_unparent_helper(sub_d->obj, obj);
                  efl_del(sub_d->obj);
               }
             break;
          }
        else if (content == sub_d->obj)
          {
             pd->subs = eina_list_remove_list(pd->subs, l);
             eina_stringshare_del(sub_d->part);
             if (sub_d->proxy) efl_del(sub_d->proxy);
             free(sub_d);
             _elm_widget_sub_object_redirect_to_top(obj, content);
             break;
          }
     }

   if (content)
     {
        if (!elm_widget_sub_object_add(obj, content)) return EINA_FALSE;

        sub_d = ELM_NEW(Efl_Ui_Vg_Animation_Sub_Obj_Data);
        if (!sub_d)
          {
             ERR("failed to allocate memory!");
             _elm_widget_sub_object_redirect_to_top(obj, content);
             return EINA_FALSE;
          }
        sub_d->part = eina_stringshare_add(part);
        sub_d->obj = content;
        sub_d->proxy = _proxy_create(sub_d->obj);
        pd->subs = eina_list_append(pd->subs, sub_d);
        efl_parent_set(content, obj);
     }

   efl_canvas_group_change(obj);

   _update_part_contents(obj, pd);

end:
   return EINA_TRUE;
}

static Efl_Gfx_Entity *
_efl_ui_vg_animation_content_get(const Eo *obj EINA_UNUSED, Efl_Ui_Vg_Animation_Data *pd, const char *part)
{
   const Eina_List *l;
   Efl_Ui_Vg_Animation_Sub_Obj_Data *sub_d;

   EINA_LIST_FOREACH(pd->subs, l, sub_d)
     {
        if (strcmp(part, sub_d->part)) continue;
        return sub_d->obj;
     }

   return NULL;
}

static Efl_Gfx_Entity *
_efl_ui_vg_animation_content_unset(Eo *obj, Efl_Ui_Vg_Animation_Data *pd, const char *part)
{
   Efl_Ui_Vg_Animation_Sub_Obj_Data *sub_d;
   Eina_List *l;
   Eo *content;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   EINA_LIST_FOREACH(pd->subs, l, sub_d)
     {
        if (!strcmp(part, sub_d->part))
          {
             if (!sub_d->obj)
               {
                  ERR("Sub Object Data doens't have valid object, Something wrong....");
                  return NULL;
               }

             content = sub_d->obj;

             /* sub_d will die in _widget_sub_object_del */
             if (!_elm_widget_sub_object_redirect_to_top(obj, content))
               {
                  ERR("could not remove sub object %p from %p", content, obj);
                  return NULL;
               }
             _eo_unparent_helper(content, obj);

             return content;
          }
     }

   return NULL;
}

#undef GROW_SIZE
#undef QUEUE_SIZE
#undef T_SEGMENT_N
#undef C_SEGMENT_N


/* Efl.Part begin */
ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_vg_animation, EFL_UI_VG_ANIMATION, Efl_Ui_Vg_Animation_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_vg_animation, EFL_UI_VG_ANIMATION, Efl_Ui_Vg_Animation_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_vg_animation, EFL_UI_VG_ANIMATION, Efl_Ui_Vg_Animation_Data)
#include "efl_ui_vg_animation_part.eo.c"
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_VG_ANIMATION_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_vg_animation)

#include "efl_ui_vg_animation_eo.legacy.c"
#include "efl_ui_vg_animation.eo.c"
