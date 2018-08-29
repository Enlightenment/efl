#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_mapbuf.h"
#include "elm_widget_container.h"
#include "elm_mapbuf.eo.h"

#include "elm_mapbuf_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_MAPBUF_CLASS

#define MY_CLASS_NAME "Elm_Mapbuf"
#define MY_CLASS_NAME_LEGACY "elm_mapbuf"

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = 0, minh = 0;
   Evas_Coord maxw = -1, maxh = -1;

   ELM_MAPBUF_DATA_GET(obj, sd);
   if (sd->content)
     {
        evas_object_size_hint_combined_min_get(sd->content, &minw, &minh);
        evas_object_size_hint_max_get(sd->content, &maxw, &maxh);
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

EOLIAN static Efl_Ui_Theme_Apply
_elm_mapbuf_efl_ui_widget_theme_apply(Eo *obj, Elm_Mapbuf_Data *sd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   _sizing_eval(obj);

   return int_ret;
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
}

static void
_elm_mapbuf_content_unset_internal(Elm_Mapbuf_Data *sd, Evas_Object *obj,
                                   Evas_Object *content)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_data_del(content, "_elm_leaveme");
   evas_object_smart_member_del(content);
   evas_object_clip_unset(content);
   evas_object_color_set(wd->resize_obj, 0, 0, 0, 0);
   evas_object_event_callback_del_full
      (content, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints_cb,
       obj);
   sd->content = NULL;
   _sizing_eval(obj);
   ELM_SAFE_FREE(sd->idler, ecore_idler_del);
}

EOLIAN static Eina_Bool
_elm_mapbuf_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Mapbuf_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   if (sobj == sd->content)
     _elm_mapbuf_content_unset_internal(sd, (Evas_Object *)obj, sobj);
   return EINA_TRUE;
}

static void
_configure(Evas_Object *obj)
{
   ELM_MAPBUF_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!sd->content) return;
   if (sd->enabled && !evas_object_visible_get(obj)) return;

   Evas_Coord x, y, w, h;
   int i;
   evas_object_geometry_get(wd->resize_obj, &x, &y, &w, &h);

   if (sd->enabled)
     {
        if (!sd->map) sd->map = evas_map_new(4);
        evas_map_util_points_populate_from_geometry(sd->map, x, y, w, h, 0);
        for (i = 0; i < (int)(sizeof(sd->colors)/sizeof(sd->colors[0])); i++)
          {
             evas_map_point_color_set(sd->map, i, sd->colors[i].r,
                                      sd->colors[i].g, sd->colors[i].b,
                                      sd->colors[i].a);
          }

        evas_map_smooth_set(sd->map, sd->smooth);
        evas_map_alpha_set(sd->map, sd->alpha);
        evas_object_map_set(sd->content, sd->map);
        evas_object_map_enable_set(sd->content, EINA_TRUE);
     }
   else
     evas_object_move(sd->content, x, y);
}

static void
_mapbuf_auto_eval(Evas_Object *obj, Elm_Mapbuf_Data *sd)
{
   Eina_Bool vis;
   Evas_Coord x, y, w, h;
   Evas_Coord vx, vy, vw, vh;
   Eina_Bool on = EINA_FALSE;

   if (!sd->automode) return ;
   vis = evas_object_visible_get(obj);
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_output_viewport_get(evas_object_evas_get(obj), &vx, &vy, &vw, &vh);
   if ((vis) && (ELM_RECTS_INTERSECT(x, y, w, h, vx, vy, vw, vh)))
     on = EINA_TRUE;
   elm_mapbuf_enabled_set(obj, on);
}

static Eina_Bool
_mapbuf_move_end(void *data)
{
   Elm_Mapbuf_Data *sd = data;

   elm_mapbuf_smooth_set(sd->self, sd->smooth_saved);
   sd->idler = NULL;

   return EINA_FALSE;
}

static void
_mapbuf_auto_smooth(Evas_Object *obj EINA_UNUSED, Elm_Mapbuf_Data *sd)
{
   if (!sd->automode) return ;
   if (!sd->idler) sd->idler = ecore_idler_add(_mapbuf_move_end, sd);
   sd->smooth = EINA_FALSE;
}

EOLIAN static void
_elm_mapbuf_efl_gfx_entity_position_set(Eo *obj, Elm_Mapbuf_Data *sd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);

   _mapbuf_auto_eval(obj, sd);
   _mapbuf_auto_smooth(obj, sd);
   _configure(obj);
}

EOLIAN static void
_elm_mapbuf_efl_gfx_entity_size_set(Eo *obj, Elm_Mapbuf_Data *sd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
   if (sd->content)
     efl_gfx_entity_size_set(sd->content, sz);

   _mapbuf_auto_eval(obj, sd);
   _configure(obj);
}

EOLIAN static void
_elm_mapbuf_efl_gfx_entity_visible_set(Eo *obj, Elm_Mapbuf_Data *sd, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);

   _mapbuf_auto_eval(obj, sd);
   _configure(obj);
}

static Eina_Bool
_elm_mapbuf_content_set(Eo *obj, Elm_Mapbuf_Data *sd, const char *part, Evas_Object *content)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (part && strcmp(part, "default")) return EINA_FALSE;
   if (sd->content == content) return EINA_TRUE;

   evas_object_del(sd->content);
   sd->content = content;

   if (content)
     {
        evas_object_data_set(content, "_elm_leaveme", (void *)1);
        elm_widget_sub_object_add(obj, content);
        evas_object_smart_member_add(content, obj);
        evas_object_clip_set(content, wd->resize_obj);
        evas_object_color_set
          (wd->resize_obj, 255, 255, 255, 255);
        evas_object_event_callback_add
          (content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _changed_size_hints_cb, obj);
     }
   else
     evas_object_color_set(wd->resize_obj, 0, 0, 0, 0);

   _sizing_eval(obj);
   _configure(obj);

   return EINA_TRUE;
}

static Evas_Object*
_elm_mapbuf_content_get(const Eo *obj EINA_UNUSED, Elm_Mapbuf_Data *sd, const char *part)
{
   if (part && strcmp(part, "default")) return NULL;
   return sd->content;
}

static Evas_Object*
_elm_mapbuf_content_unset(Eo *obj, Elm_Mapbuf_Data *sd, const char *part)
{
   Evas_Object *content;
   if (part && strcmp(part, "default")) return NULL;
   if (!sd->content) return NULL;

   content = sd->content;
   _elm_widget_sub_object_redirect_to_top(obj, content);
   _elm_mapbuf_content_unset_internal(sd, obj, content);
   return content;
}

EOLIAN static Eina_Bool
_elm_mapbuf_efl_content_content_set(Eo *obj, Elm_Mapbuf_Data *sd, Evas_Object *content)
{
   return _elm_mapbuf_content_set(obj, sd, NULL, content);
}

EOLIAN static Evas_Object*
_elm_mapbuf_efl_content_content_get(const Eo *obj, Elm_Mapbuf_Data *sd)
{
   return _elm_mapbuf_content_get(obj, sd, NULL);
}

EOLIAN static Evas_Object*
_elm_mapbuf_efl_content_content_unset(Eo *obj, Elm_Mapbuf_Data *sd)
{
   return _elm_mapbuf_content_unset(obj, sd, NULL);
}

EOLIAN static void
_elm_mapbuf_efl_canvas_group_group_del(Eo *obj, Elm_Mapbuf_Data *priv)
{
   ELM_SAFE_FREE(priv->idler, ecore_idler_del);
   ELM_SAFE_FREE(priv->map, evas_map_free);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_mapbuf_efl_canvas_group_group_add(Eo *obj, Elm_Mapbuf_Data *priv)
{
   Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   int i;

   elm_widget_resize_object_set(obj, rect);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   evas_object_static_clip_set(rect, EINA_TRUE);
   evas_object_pass_events_set(rect, EINA_TRUE);
   evas_object_color_set(rect, 0, 0, 0, 0);

   for (i = 0; i < (int)(sizeof(priv->colors)/sizeof(priv->colors[0])); i++)
     {
        priv->colors[i].r = 255;
        priv->colors[i].g = 255;
        priv->colors[i].b = 255;
        priv->colors[i].a = 255;
     }

   priv->self = obj;
   priv->alpha = EINA_TRUE;
   priv->smooth = EINA_TRUE;

   elm_widget_can_focus_set(obj, EINA_FALSE);

   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_mapbuf_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_mapbuf_efl_object_constructor(Eo *obj, Elm_Mapbuf_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_IMAGE_MAP);

   return obj;
}

static void
_internal_enable_set(Eo *obj, Elm_Mapbuf_Data *sd, Eina_Bool enabled)
{
   if (sd->enabled == enabled) return;
   sd->enabled = enabled;

   if (!sd->enabled && sd->content)
     {
        evas_object_map_set(sd->content, NULL);
        evas_object_map_enable_set(sd->content, EINA_FALSE);
     }
   _configure(obj);
}

EOLIAN static void
_elm_mapbuf_enabled_set(Eo *obj, Elm_Mapbuf_Data *sd, Eina_Bool enabled)
{
   _internal_enable_set(obj, sd, enabled);
}

EOLIAN static Eina_Bool
_elm_mapbuf_enabled_get(const Eo *obj EINA_UNUSED, Elm_Mapbuf_Data *sd)
{
   return sd->enabled;
}

EOLIAN static void
_elm_mapbuf_smooth_set(Eo *obj, Elm_Mapbuf_Data *sd, Eina_Bool smooth)
{
   if (sd->smooth == smooth) return;
   sd->smooth = smooth;
   sd->smooth_saved = smooth;
   _configure(obj);
}

EOLIAN static Eina_Bool
_elm_mapbuf_smooth_get(const Eo *obj EINA_UNUSED, Elm_Mapbuf_Data *sd)
{
   return sd->smooth;
}

EOLIAN static void
_elm_mapbuf_alpha_set(Eo *obj, Elm_Mapbuf_Data *sd, Eina_Bool alpha)
{
   if (sd->alpha == alpha) return;
   sd->alpha = alpha;
   _configure(obj);
}

EOLIAN static Eina_Bool
_elm_mapbuf_alpha_get(const Eo *obj EINA_UNUSED, Elm_Mapbuf_Data *sd)
{
   return sd->alpha;
}

EOLIAN static void
_elm_mapbuf_auto_set(Eo *obj, Elm_Mapbuf_Data *sd, Eina_Bool on)
{
   if (sd->automode == on) return;
   sd->automode = on;
   if (on)
     {
        _mapbuf_auto_eval(obj, sd);
     }
   else
     {
        ELM_SAFE_FREE(sd->idler, ecore_idler_del);

        _internal_enable_set(obj, sd, EINA_FALSE);
     }
   _configure(obj);
}

EOLIAN static Eina_Bool
_elm_mapbuf_auto_get(const Eo *obj EINA_UNUSED, Elm_Mapbuf_Data *sd)
{
   return sd->automode;
}

EOLIAN static void
_elm_mapbuf_point_color_get(const Eo *obj EINA_UNUSED, Elm_Mapbuf_Data *sd, int idx, int *r, int *g, int *b, int *a)
{
   if ((idx < 0) || (idx >= 4))
     {
        ERR("idx value should be 0 ~ 4");
        return;
     }
   *r = sd->colors[idx].r;
   *g = sd->colors[idx].g;
   *b = sd->colors[idx].b;
   *a =sd->colors[idx].a;
}

EOLIAN static void
_elm_mapbuf_point_color_set(Eo *obj EINA_UNUSED, Elm_Mapbuf_Data *sd, int idx, int r, int g, int b, int a)
{
   if ((idx < 0) || (idx >= 4))
     {
        ERR("idx value should be 0 ~ 4");
        return;
     }
   sd->colors[idx].r = r;
   sd->colors[idx].g = g;
   sd->colors[idx].b = b;
   sd->colors[idx].a = a;

   _configure(obj);
}

static void
_elm_mapbuf_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_mapbuf, ELM_MAPBUF, Elm_Mapbuf_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_mapbuf, ELM_MAPBUF, Elm_Mapbuf_Data)
ELM_PART_OVERRIDE_CONTENT_GET(elm_mapbuf, ELM_MAPBUF, Elm_Mapbuf_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_mapbuf, ELM_MAPBUF, Elm_Mapbuf_Data)
ELM_PART_CONTENT_DEFAULT_GET(elm_mapbuf, "default")
#include "elm_mapbuf_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define ELM_MAPBUF_EXTRA_OPS \
   ELM_PART_CONTENT_DEFAULT_OPS(elm_mapbuf), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_mapbuf)

#include "elm_mapbuf.eo.c"
