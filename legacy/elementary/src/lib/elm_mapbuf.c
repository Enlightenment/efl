#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_mapbuf.h"
#include "elm_widget_container.h"

EAPI Eo_Op ELM_OBJ_MAPBUF_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_MAPBUF_CLASS

#define MY_CLASS_NAME "Elm_Mapbuf"
#define MY_CLASS_NAME_LEGACY "elm_mapbuf"

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord maxw = -1, maxh = -1;

   ELM_MAPBUF_DATA_GET(obj, sd);
   if (sd->content)
     {
        evas_object_size_hint_min_get(sd->content, &minw, &minh);
        evas_object_size_hint_max_get(sd->content, &maxw, &maxh);
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_elm_mapbuf_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   _sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
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
_elm_mapbuf_content_unset(Elm_Mapbuf_Smart_Data *sd, Evas_Object *obj,
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

static void
_elm_mapbuf_smart_sub_object_del(Eo *obj, void *_pd, va_list *list)
{
   Elm_Mapbuf_Smart_Data *sd = _pd;
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

   if (sobj == sd->content)
     _elm_mapbuf_content_unset(sd, (Evas_Object *)obj, sobj);
   if (ret) *ret = EINA_TRUE;
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
_mapbuf_auto_eval(Evas_Object *obj, Elm_Mapbuf_Smart_Data *sd)
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
   Elm_Mapbuf_Smart_Data *sd = data;

   elm_mapbuf_smooth_set(sd->self, sd->smooth_saved);
   sd->idler = NULL;

   return EINA_FALSE;
}

static void
_mapbuf_auto_smooth(Evas_Object *obj EINA_UNUSED, Elm_Mapbuf_Smart_Data *sd)
{
   if (!sd->automode) return ;
   if (!sd->idler) sd->idler = ecore_idler_add(_mapbuf_move_end, sd);
   sd->smooth = EINA_FALSE;
}

static void
_elm_mapbuf_smart_move(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   _mapbuf_auto_eval(obj, _pd);
   _mapbuf_auto_smooth(obj, _pd);
   _configure(obj);
}

static void
_elm_mapbuf_smart_resize(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   ELM_MAPBUF_DATA_GET(obj, sd);
   if (sd->content) evas_object_resize(sd->content, w, h);
   _mapbuf_auto_eval(obj, _pd);
   _configure(obj);
}

static void
_elm_mapbuf_smart_show(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());

   _mapbuf_auto_eval(obj, _pd);
   _configure(obj);
}

static void
_elm_mapbuf_smart_hide(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());

   _mapbuf_auto_eval(obj, _pd);
   _configure(obj);
}

static void
_elm_mapbuf_smart_content_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Mapbuf_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (part && strcmp(part, "default")) return;
   if (sd->content == content)
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }

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

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_mapbuf_smart_content_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Mapbuf_Smart_Data *sd = _pd;
   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;

   if (part && strcmp(part, "default")) return;
   *ret = sd->content;
}

static void
_elm_mapbuf_smart_content_unset(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *content;
   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (ret) *ret = NULL;

   Elm_Mapbuf_Smart_Data *sd = _pd;

   if (part && strcmp(part, "default")) return;
   if (!sd->content) return;

   content = sd->content;
   elm_widget_sub_object_del(obj, content);
   _elm_mapbuf_content_unset(sd, obj, content);
   if (ret) *ret = content;
}

static void
_elm_mapbuf_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Mapbuf_Smart_Data *priv = _pd;

   ELM_SAFE_FREE(priv->idler, ecore_idler_del);
   ELM_SAFE_FREE(priv->map, evas_map_free);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static void
_elm_mapbuf_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Mapbuf_Smart_Data *priv = _pd;
   Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   int i;

   elm_widget_resize_object_set(obj, rect, EINA_TRUE);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
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
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

EAPI void
elm_mapbuf_enabled_set(Evas_Object *obj,
                       Eina_Bool enabled)
{
   ELM_MAPBUF_CHECK(obj);
   eo_do(obj, elm_obj_mapbuf_enabled_set(enabled));
}

static void
_internal_enable_set(Eo *obj, Elm_Mapbuf_Smart_Data *sd, Eina_Bool enabled)
{
   if (sd->enabled == enabled) return;
   sd->enabled = enabled;

   if (sd->content) evas_object_static_clip_set(sd->content, sd->enabled);

   if (!sd->enabled && sd->content)
     {
        evas_object_map_set(sd->content, NULL);
        evas_object_map_enable_set(sd->content, EINA_FALSE);
     }
   _configure(obj);
}

static void
_enabled_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool enabled = va_arg(*list, int);
   Elm_Mapbuf_Smart_Data *sd = _pd;

   _internal_enable_set(obj, sd, enabled);
}

EAPI Eina_Bool
elm_mapbuf_enabled_get(const Evas_Object *obj)
{
   ELM_MAPBUF_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_mapbuf_enabled_get(&ret));
   return ret;
}

static void
_enabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Mapbuf_Smart_Data *sd = _pd;
   *ret = sd->enabled;
}

EAPI void
elm_mapbuf_smooth_set(Evas_Object *obj,
                      Eina_Bool smooth)
{
   ELM_MAPBUF_CHECK(obj);
   eo_do(obj, elm_obj_mapbuf_smooth_set(smooth));
}

static void
_smooth_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool smooth = va_arg(*list, int);
   Elm_Mapbuf_Smart_Data *sd = _pd;

   if (sd->smooth == smooth) return;
   sd->smooth = smooth;
   sd->smooth_saved = smooth;
   _configure(obj);
}

EAPI Eina_Bool
elm_mapbuf_smooth_get(const Evas_Object *obj)
{
   ELM_MAPBUF_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_mapbuf_smooth_get(&ret));
   return ret;
}

static void
_smooth_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Mapbuf_Smart_Data *sd = _pd;
   *ret = sd->smooth;
}

EAPI void
elm_mapbuf_alpha_set(Evas_Object *obj,
                     Eina_Bool alpha)
{
   ELM_MAPBUF_CHECK(obj);
   eo_do(obj, elm_obj_mapbuf_alpha_set(alpha));
}

static void
_alpha_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool alpha = va_arg(*list, int);
   Elm_Mapbuf_Smart_Data *sd = _pd;

   if (sd->alpha == alpha) return;
   sd->alpha = alpha;
   _configure(obj);
}

EAPI Eina_Bool
elm_mapbuf_alpha_get(const Evas_Object *obj)
{
   ELM_MAPBUF_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_mapbuf_alpha_get(&ret));
   return ret;
}

static void
_alpha_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Mapbuf_Smart_Data *sd = _pd;
   *ret = sd->alpha;
}

EAPI void
elm_mapbuf_auto_set(Evas_Object *obj,
                     Eina_Bool on)
{
   ELM_MAPBUF_CHECK(obj);
   eo_do(obj, elm_obj_mapbuf_auto_set(on));
}

static void
_auto_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool on = va_arg(*list, int);
   Elm_Mapbuf_Smart_Data *sd = _pd;

   if (sd->automode == on) return;
   sd->automode = on;
   if (on)
     {
        _mapbuf_auto_eval(obj, sd);
     }
   else
     {
        ELM_SAFE_FREE(sd->idler, ecore_idler_del);

        _internal_enable_set(obj, _pd, EINA_FALSE);
     }
   _configure(obj);
}

EAPI Eina_Bool
elm_mapbuf_auto_get(const Evas_Object *obj)
{
   ELM_MAPBUF_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_mapbuf_auto_get(&ret));
   return ret;
}

static void
_auto_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Mapbuf_Smart_Data *sd = _pd;
   *ret = sd->automode;
}

static void
_point_color_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int idx = va_arg(*list, int);
   int *r = va_arg(*list, int *);
   int *g = va_arg(*list, int *);
   int *b = va_arg(*list, int *);
   int *a = va_arg(*list, int *);

   Elm_Mapbuf_Smart_Data *sd = _pd;

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

EAPI void
elm_mapbuf_point_color_get(Evas_Object *obj, int idx,
                           int *r, int *g, int *b, int *a)
{
   ELM_MAPBUF_CHECK(obj);

   eo_do(obj, elm_obj_mapbuf_point_color_get(idx, r, g, b, a));
}

static void
_point_color_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int idx = va_arg(*list, int);
   int r = va_arg(*list, int);
   int g = va_arg(*list, int);
   int b = va_arg(*list, int);
   int a = va_arg(*list, int);

   Elm_Mapbuf_Smart_Data *sd = _pd;

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

EAPI void
elm_mapbuf_point_color_set(Evas_Object *obj, int idx,
                           int r, int g, int b, int a)
{
   ELM_MAPBUF_CHECK(obj);

   eo_do(obj, elm_obj_mapbuf_point_color_set(idx, r, g, b, a));
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_mapbuf_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_mapbuf_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_mapbuf_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_mapbuf_smart_move),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _elm_mapbuf_smart_show),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _elm_mapbuf_smart_hide),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_mapbuf_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_mapbuf_smart_sub_object_del),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_mapbuf_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_mapbuf_smart_content_get),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_mapbuf_smart_content_unset),

        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_ENABLED_SET), _enabled_set),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_ENABLED_GET), _enabled_get),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_SMOOTH_SET), _smooth_set),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_SMOOTH_GET), _smooth_get),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_ALPHA_SET), _alpha_set),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_ALPHA_GET), _alpha_get),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_AUTO_SET), _auto_set),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_AUTO_GET), _auto_get),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_POINT_COLOR_SET), _point_color_set),
        EO_OP_FUNC(ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_POINT_COLOR_GET), _point_color_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_ENABLED_SET, "Enable or disable the map."),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_ENABLED_GET, "Get a value whether map is enabled or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_SMOOTH_SET, "Enable or disable smooth map rendering."),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_SMOOTH_GET, "Get a value whether smooth map rendering is enabled or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_ALPHA_SET, "Set or unset alpha flag for map rendering."),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_ALPHA_GET, "Get a value whether alpha blending is enabled or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_AUTO_SET, "Turn map on or off automatically based on object visibility."),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_AUTO_GET, "Get automatic mode state."),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_POINT_COLOR_SET, "Set a vertex color of the mapbuf"),
     EO_OP_DESCRIPTION(ELM_OBJ_MAPBUF_SUB_ID_POINT_COLOR_GET, "Get a vertex color of the mapbuf"),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_MAPBUF_BASE_ID, op_desc, ELM_OBJ_MAPBUF_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Mapbuf_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_mapbuf_class_get, &class_desc, ELM_OBJ_CONTAINER_CLASS, NULL);
