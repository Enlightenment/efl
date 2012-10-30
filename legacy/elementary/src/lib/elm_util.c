#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "elm_priv.h"

#define SLICE_CNT 30
#define ZOOM_SEC 0.75

typedef struct _Slice
{
   Evas_Object *obj;
   Evas_Coord x[4];
   Evas_Coord y[4];
   float zoom_end;
   float zoom_end2;
   float zoom_cur;
   float zoom_cur2;
   float zoom_begin;
   float zoom_begin2;
   int color_end;
   int color_end2;
   int color_begin;
   int color_begin2;
   int color_cur;
   int color_cur2;
} Slice;

struct _Scrol_Effect
{
   Evas_Object *target;
   Slice slices[SLICE_CNT];
   Elm_Transit *transit;
   int slice_idx;
};

static void
_slices_new(Scroll_Effect *se)
{
   Evas *e = evas_object_evas_get(se->target);
   for (int i = 0; i < SLICE_CNT; i++)
     {
        if (!se->slices[i].obj)
          se->slices[i].obj = evas_object_image_add(e);
        evas_object_image_smooth_scale_set(se->slices[i].obj, EINA_TRUE);
        se->slices[i].zoom_cur = 0;
        se->slices[i].zoom_cur2 = 0;
        se->slices[i].color_cur = 255;
        se->slices[i].color_cur2 = 255;
     }
}

static void
_transit_del_cb(void *data, Elm_Transit *transit __UNUSED__)
{
   Scroll_Effect *se = data;
   se->transit = NULL;
}

static void
_zoom_out_cb(void *data, Elm_Transit *transit __UNUSED__, double progress)
{
   Scroll_Effect *se = data;
   int i;
   Evas_Coord x, y, w, h;
   int color, color2;

   evas_object_geometry_get(se->target, &x, &y, &w, &h);

   Evas_Coord slice_h = h / SLICE_CNT;

   Evas_Map *map = evas_map_new(4);
   if (!map) return;

   for(i = 0; i < SLICE_CNT; ++i)
     {
        Evas_Coord offset = slice_h * i;
        Evas_Coord offset2 = offset + slice_h;

        if (i == SLICE_CNT - 1)
          offset2 += (h - SLICE_CNT * slice_h);

        evas_object_image_fill_set(se->slices[i].obj, 0, offset, w, offset2);
        evas_object_image_source_set(se->slices[i].obj, se->target);
        evas_object_image_source_visible_set(se->slices[i].obj, EINA_FALSE);
		  evas_object_image_source_events_set(se->slices[i].obj, EINA_TRUE);
		  evas_object_resize(se->slices[i].obj, w, h);
		  evas_object_move(se->slices[i].obj, x, y);

        se->slices[i].zoom_cur = se->slices[i].zoom_begin + (se->slices[i].zoom_end - se->slices[i].zoom_begin) * progress;
        se->slices[i].zoom_cur2 = se->slices[i].zoom_begin2 + (se->slices[i].zoom_end2 - se->slices[i].zoom_begin2) * progress;

        evas_map_point_coord_set(map, 0, x + se->slices[i].zoom_cur, y + offset, 0);
        evas_map_point_coord_set(map, 1, x + w - se->slices[i].zoom_cur, y + offset, 0);
        evas_map_point_coord_set(map, 2, x + w - se->slices[i].zoom_cur2, y + offset2, 0);
        evas_map_point_coord_set(map, 3, x + se->slices[i].zoom_cur2, y + offset2, 0);

        evas_map_point_image_uv_set(map, 0, 0, offset);
        evas_map_point_image_uv_set(map, 1, w, offset);
        evas_map_point_image_uv_set(map, 2, w, offset2);
        evas_map_point_image_uv_set(map, 3, 0, offset2);

        color = se->slices[i].color_begin + (se->slices[i].color_end - se->slices[i].color_begin) * progress;
        color2 = se->slices[i].color_begin2 + (se->slices[i].color_end2 - se->slices[i].color_begin2) * progress;

        if (color < 200) color = 200;
        if (color2 < 200) color2 = 200;

        evas_map_point_color_set(map, 0, color, color, color, 255);
        evas_map_point_color_set(map, 1, color, color, color, 255);
        evas_map_point_color_set(map, 2, color2, color2, color2, 255);
        evas_map_point_color_set(map, 3, color2, color2, color2, 255);

        se->slices[i].color_cur = color;
        se->slices[i].color_cur2 = color2;
        evas_object_map_set(se->slices[i].obj, map);
        evas_object_map_enable_set(se->slices[i].obj, EINA_TRUE);
        evas_object_show(se->slices[i].obj);
     }
   evas_map_free(map);
}

static void
_zoom_out(Scroll_Effect *se)
{
   for(int i = 0; i < SLICE_CNT; ++i)
     {
        se->slices[i].zoom_begin = se->slices[i].zoom_cur;
        se->slices[i].zoom_begin2 = se->slices[i].zoom_cur2;
        se->slices[i].color_begin = se->slices[i].color_cur;
        se->slices[i].color_begin2 = se->slices[i].color_cur2;
     }

   if (se->transit) elm_transit_del(se->transit);
   se->transit = elm_transit_add();
   elm_transit_del_cb_set(se->transit, _transit_del_cb, se);
   elm_transit_effect_add(se->transit, _zoom_out_cb, se, NULL);
   elm_transit_duration_set(se->transit, ZOOM_SEC);
   elm_transit_tween_mode_set(se->transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   elm_transit_event_enabled_set(se->transit, EINA_TRUE);
   elm_transit_go(se->transit);
}

static void
_zoom_in_cb(void *data, Elm_Transit *transit __UNUSED__, double progress)
{
   Scroll_Effect *se = data;

   int i;
   Evas_Coord x, y, w, h;
   int color, color2;

   evas_object_geometry_get(se->target, &x, &y, &w, &h);
   Evas_Coord slice_h = h / SLICE_CNT;

   Evas_Map *map = evas_map_new(4);
   if (!map) return;

   for(i = 0; i < SLICE_CNT; ++i)
     {
        Evas_Coord offset = slice_h * i;
        Evas_Coord offset2 = offset + slice_h;

        if (i == SLICE_CNT - 1)
          offset2 += (h - SLICE_CNT * slice_h);

        evas_object_image_fill_set(se->slices[i].obj, 0, offset, w, offset2);
        evas_object_image_source_set(se->slices[i].obj, se->target);

        se->slices[i].zoom_cur = se->slices[i].zoom_begin + se->slices[i].zoom_end * progress;
        se->slices[i].zoom_cur2 = se->slices[i].zoom_begin2 + se->slices[i].zoom_end2 * progress;

        evas_map_point_coord_set(map, 0, x + se->slices[i].zoom_cur, y + offset, 0);
        evas_map_point_coord_set(map, 1, x + w - se->slices[i].zoom_cur, y + offset, 0);
        evas_map_point_coord_set(map, 2, x + w - se->slices[i].zoom_cur2, y + offset2, 0);
        evas_map_point_coord_set(map, 3, x + se->slices[i].zoom_cur2, y + offset2, 0);


        evas_map_point_image_uv_set(map, 0, 0, offset);
        evas_map_point_image_uv_set(map, 1, w, offset);
        evas_map_point_image_uv_set(map, 2, w, offset2);
        evas_map_point_image_uv_set(map, 3, 0, offset2);

        color = se->slices[i].color_begin + se->slices[i].color_end * progress;
        color2 = se->slices[i].color_begin2 + se->slices[i].color_end2 * progress;

        if (color < 200) color = 200;
        if (color2 < 200) color2 = 200;

        evas_map_point_color_set(map, 0, color, color, color, 255);
        evas_map_point_color_set(map, 1, color, color, color, 255);
        evas_map_point_color_set(map, 2, color2, color2, color2, 255);
        evas_map_point_color_set(map, 3, color2, color2, color2, 255);

        se->slices[i].color_cur = color;
        se->slices[i].color_cur2 = color2;

        evas_object_map_enable_set(se->slices[i].obj, EINA_TRUE);
        evas_object_map_set(se->slices[i].obj, map);
        evas_object_show(se->slices[i].obj);
     }
   evas_map_free(map);
}

static void
_zoom_in(Scroll_Effect *se)
{
   for(int i = 0; i < SLICE_CNT; ++i)
     {
        se->slices[i].zoom_begin = se->slices[i].zoom_cur;
        se->slices[i].zoom_begin2 = se->slices[i].zoom_cur2;
        se->slices[i].zoom_end = -se->slices[i].zoom_cur;
        se->slices[i].zoom_end2 = -se->slices[i].zoom_cur2;
        se->slices[i].color_begin = se->slices[i].color_cur;
        se->slices[i].color_begin2 = se->slices[i].color_cur2;
        se->slices[i].color_end = 255 - se->slices[i].color_cur;
        se->slices[i].color_end2 = 255 - se->slices[i].color_cur2;
     }

   if (se->transit) elm_transit_del(se->transit);
   se->transit = elm_transit_add();
   elm_transit_del_cb_set(se->transit, _transit_del_cb, se);
   elm_transit_effect_add(se->transit, _zoom_in_cb, se, NULL);
   elm_transit_duration_set(se->transit, ZOOM_SEC);
   elm_transit_event_enabled_set(se->transit, EINA_TRUE);
   elm_transit_tween_mode_set(se->transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   elm_transit_go(se->transit);
}

float degree_to_radian( const float degree )
{
   const int PI = 3.141592;
   return degree / 180 * PI;
}

static void
_lookup_index(Scroll_Effect *se, Evas_Coord pt)
{
   Evas_Coord y, h;
   evas_object_geometry_get(se->target, NULL, &y, NULL, &h);

   y = pt - y;

   Evas_Coord slice_h = h / SLICE_CNT;

   y -= (y % slice_h);
   y /= slice_h;

   se->slice_idx = y;
}

static void
_update_slices_property(Scroll_Effect *se, Eina_Bool move)
{
   float degree;
   Evas_Coord w;
   evas_object_geometry_get(se->target, NULL, NULL, &w, NULL);

   Evas_Coord half_w = (w >> 1);
   float depth = 0.2 * half_w;

   //First Slice
   degree = 90 - (((fabs(se->slice_idx)) / (SLICE_CNT)) * 90);
   se->slices[0].zoom_end = sin(degree_to_radian(degree)) * depth;

   degree = 90 - (((fabs(se->slice_idx - 1)) / (SLICE_CNT)) * 90);
   se->slices[0].zoom_end2 = sin(degree_to_radian(degree)) * depth;

   se->slices[0].color_end = 255 * ((se->slices[0].zoom_end / depth) * 0.5 + 0.5);
   se->slices[0].color_end2 = 255 * ((se->slices[0].zoom_end2 / depth) * 0.5 + 0.5);

   if (move)
     {
        se->slices[0].zoom_begin = se->slices[0].zoom_cur = se->slices[0].zoom_end;
        se->slices[0].zoom_begin2 = se->slices[0].zoom_cur2 = se->slices[0].zoom_end2;
     }

   //Last Slices
   for(int i = 1; i < SLICE_CNT; ++i)
     {
        degree = 90 - (((fabs(se->slice_idx - (i + 1))) / (SLICE_CNT)) * 90);

        se->slices[i].zoom_end = se->slices[i - 1].zoom_end2;
        se->slices[i].zoom_end2 = sin(degree_to_radian(degree)) * depth;

        se->slices[i].color_end = se->slices[i - 1].color_end2;
        se->slices[i].color_end2 = 255 * ((se->slices[i].zoom_end2 / depth) * 0.5 + 0.5);

        if (move)
          {
             se->slices[i].zoom_begin = se->slices[i].zoom_cur = se->slices[i].zoom_end;
             se->slices[i].zoom_begin2 = se->slices[i].zoom_cur2 = se->slices[i].zoom_end2;
          }
     }
}

static void
_target_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
	printf("mouse down!!\n");
   Evas_Event_Mouse_Down *ev = event_info;
   _lookup_index(data, ev->canvas.y);
   _update_slices_property(data, EINA_FALSE);
   _zoom_out(data);
}

static void
_target_mouse_move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Event_Mouse_Move *ev = event_info;
	printf("mouse move!!\n");

   if (!ev->buttons) return;

   Scroll_Effect *se = data;

   _lookup_index(data, ev->cur.canvas.y);

   if (se->transit)
     _update_slices_property(data, EINA_FALSE);
   else
     {
        _update_slices_property(data, EINA_TRUE);
        _zoom_out_cb(se, NULL, 1);
     }
}

static void
_target_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	printf("mouse up!!\n");
	_zoom_in(data);
}

static void
_target_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Scroll_Effect *se = data;

   for (int i = 0; i < SLICE_CNT; i++)
     {
        if (!se->slices[i].obj) continue;
        evas_object_del(se->slices[i].obj);
        se->slices[i].obj = NULL;
     }
}

Scroll_Effect *
elm_scrl_effect_add(Evas_Object *target)
{
   Scroll_Effect *se = calloc(1, sizeof(Scroll_Effect));
   if (!se) return NULL;

   se->target = target;
   _slices_new(se);

   evas_object_event_callback_add(target, EVAS_CALLBACK_MOUSE_DOWN, _target_mouse_down, se);
   evas_object_event_callback_add(target, EVAS_CALLBACK_MOUSE_MOVE, _target_mouse_move, se);
   evas_object_event_callback_add(target, EVAS_CALLBACK_MOUSE_UP, _target_mouse_up, se);
   evas_object_event_callback_add(target, EVAS_CALLBACK_DEL, _target_del, se);

   return se;
}

void
elm_scrl_effect_del(Scroll_Effect *se)
{
   for (int i = 0; i < SLICE_CNT; i++)
     {
        if (!se->slices[i].obj) continue;
        evas_object_del(se->slices[i].obj);
     }

   free(se);
}

char *
_str_ncpy(char *dest, const char *src, size_t count)
{
   if ((!dest) || (!src)) return NULL;
   return strncpy(dest, src, count);
}

char *
_str_append(char *str, const char *txt, int *len, int *alloc)
{
   int txt_len = strlen(txt);

   if (txt_len <= 0) return str;
   if ((*len + txt_len) >= *alloc)
     {
        char *str2;
        int alloc2;

        alloc2 = *alloc + txt_len + 128;
        str2 = realloc(str, alloc2);
        if (!str2) return str;
        *alloc = alloc2;
        str = str2;
     }
   strcpy(str + *len, txt);
   *len += txt_len;
   return str;
}

char *
_elm_util_mkup_to_text(const char *mkup)
{
   return evas_textblock_text_markup_to_utf8(NULL, mkup);
}

char *
_elm_util_text_to_mkup(const char *text)
{
   return evas_textblock_text_utf8_to_markup(NULL, text);
}

double
_elm_atof(const char *s)
{
   char *cradix, *buf, *p;
   
   if ((!s) || (!s[0])) return 0.0;
   cradix = nl_langinfo(RADIXCHAR);
   if (!cradix) return atof(s);
   buf = alloca(strlen(s) + 1);
   strcpy(buf, s);
   for (p = buf; *p; p++)
     {
        if (*p == '.') *p = *cradix;
     }
   return atof(buf);
}
