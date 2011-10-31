#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *content, *clip;
   Eina_Bool enabled : 1;
   Eina_Bool alpha : 1;
   Eina_Bool smooth : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord maxw = -1, maxh = -1;
   if (!wd) return;
   if (wd->content)
     {
        evas_object_size_hint_min_get(wd->content, &minw, &minh);
        evas_object_size_hint_max_get(wd->content, &maxw, &maxh);
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->content)
     {
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->content = NULL;
        _sizing_eval(obj);
     }
}

static void
_mapbuf(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x, y, w, h;
   if (!wd) return;
   evas_object_geometry_get(wd->clip, &x, &y, &w, &h);
   if (wd->enabled)
     {
        Evas_Map *m;

        m = evas_map_new(4);
        evas_map_util_points_populate_from_geometry(m, x, y, w, h, 0);
        evas_map_smooth_set(m, wd->smooth);
        evas_map_alpha_set(m, wd->alpha);
        evas_object_map_set(wd->content, m);
        evas_object_map_enable_set(wd->content, wd->enabled);
        evas_map_free(m);
     }
   else
     {
        evas_object_map_set(wd->content, NULL);
        evas_object_map_enable_set(wd->content, 0);
        evas_object_move(wd->content, x, y);
        evas_object_resize(wd->content, w, h);
     }
}

static void
_configure(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content)
     {
        Evas_Coord x, y, w, h, x2, y2;

        evas_object_geometry_get(wd->clip, &x, &y, &w, &h);
        evas_object_geometry_get(wd->content, &x2, &y2, NULL, NULL);
        if ((x != x2) || (y != y2))
          {
             if (!wd->enabled)
               evas_object_move(wd->content, x, y);
             else
               {

                  Evas *e = evas_object_evas_get(obj);
                  evas_smart_objects_calculate(e);
                  evas_nochange_push(e);
                  //                  printf("x-------------------- %i %i\n", x, y);
                  evas_object_move(wd->content, x, y);
                  evas_smart_objects_calculate(e);
                  //                  printf("y--------------------\n");
                  evas_nochange_pop(e);
               }
          }
        evas_object_resize(wd->content, w, h);
        _mapbuf(obj);
     }
}

static void
_move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _configure(data);
}

static void
_resize(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _configure(data);
}

static void
_content_set_hook(Evas_Object *obj, const char *part __UNUSED__, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   if (content)
     {
        evas_object_data_set(content, "_elm_leaveme", (void *)1);
        elm_widget_sub_object_add(content, obj);
        evas_object_smart_member_add(content, obj);
        evas_object_clip_set(content, wd->clip);
        evas_object_color_set(wd->clip, 255, 255, 255, 255);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
     }
   else
     evas_object_color_set(wd->clip, 0, 0, 0, 0);
   _sizing_eval(obj);
   _configure(obj);
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content;
   if (!wd) return NULL;
   if (!wd->content) return NULL;
   content = wd->content;
   elm_widget_sub_object_del(obj, content);
   evas_object_event_callback_add(content,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);
   evas_object_smart_member_del(content);
   evas_object_color_set(wd->clip, 0, 0, 0, 0);
   evas_object_clip_unset(content);
   evas_object_data_del(content, "_elm_leaveme");
   wd->content = NULL;
   return content;
}

EAPI Evas_Object *
elm_mapbuf_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "mapbuf");
   elm_widget_type_set(obj, "mapbuf");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->clip = evas_object_rectangle_add(e);
   evas_object_static_clip_set(wd->clip, EINA_TRUE);
   evas_object_pass_events_set(wd->clip, EINA_TRUE);
   evas_object_color_set(wd->clip, 0, 0, 0, 0);

   evas_object_event_callback_add(wd->clip, EVAS_CALLBACK_MOVE, _move, obj);
   evas_object_event_callback_add(wd->clip, EVAS_CALLBACK_RESIZE, _resize, obj);
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   elm_widget_resize_object_set(obj, wd->clip);

   wd->enabled = 0;
   wd->alpha = 1;
   wd->smooth = 1;

   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_mapbuf_content_set(Evas_Object *obj, Evas_Object *content)
{
   _content_set_hook(obj, NULL, content);
}

EAPI Evas_Object *
elm_mapbuf_content_get(const Evas_Object *obj)
{
   return _content_get_hook(obj, NULL);
}

EAPI Evas_Object *
elm_mapbuf_content_unset(Evas_Object *obj)
{
   return _content_unset_hook(obj, NULL);
}

EAPI void
elm_mapbuf_enabled_set(Evas_Object *obj, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->enabled == enabled) return;
   wd->enabled = enabled;
   if (wd->content) evas_object_static_clip_set(wd->content, wd->enabled);
   _configure(obj);
}

EAPI Eina_Bool
elm_mapbuf_enabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->enabled;
}

EAPI void
elm_mapbuf_smooth_set(Evas_Object *obj, Eina_Bool smooth)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->smooth == smooth) return;
   wd->smooth = smooth;
   _configure(obj);
}

EAPI Eina_Bool
elm_mapbuf_smooth_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->smooth;
}

EAPI void
elm_mapbuf_alpha_set(Evas_Object *obj, Eina_Bool alpha)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->alpha == alpha) return;
   wd->alpha = alpha;
   _configure(obj);
}

EAPI Eina_Bool
elm_mapbuf_alpha_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->alpha;
}
