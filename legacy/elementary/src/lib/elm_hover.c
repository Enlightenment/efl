#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Content_Info Content_Info;

#ifndef MAX
# define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define ELM_HOVER_PARTS_FOREACH unsigned int i = 0; \
  for (i = 0; i < sizeof(wd->subs) / sizeof(wd->subs[0]); i++)

static const char *_directions[] = {
  "left",
  "top-left",
  "top",
  "top-right",
  "right",
  "bottom-right",
  "bottom",
  "bottom-left",
  "middle"
};

#define _HOV_LEFT (_directions[0])
#define _HOV_TOP_LEFT (_directions[1])
#define _HOV_TOP (_directions[2])
#define _HOV_TOP_RIGHT (_directions[2])
#define _HOV_RIGHT (_directions[4])
#define _HOV_BOTTOM_RIGHT (_directions[5])
#define _HOV_BOTTOM (_directions[6])
#define _HOV_BOTTOM_LEFT (_directions[7])
#define _HOV_MIDDLE (_directions[8])

struct _Content_Info
{
   const char *swallow;
   Evas_Object *obj;
};

struct _Widget_Data
{
   Evas_Object *hov, *cov;
   Evas_Object *offset, *size;
   Evas_Object *parent, *target;
   Evas_Object *smt_sub;
   Content_Info subs[sizeof(_directions)/sizeof(_directions[0])];
};

static const char *widtype = NULL;
static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _reval_content(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _hov_show_do(Evas_Object *obj);
static void _hov_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_show(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_hide(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _elm_hover_sub_obj_placement_eval_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _elm_hover_sub_obj_placement_eval(Evas_Object *obj);

static const char SIG_CLICKED[] = "clicked";
static const char SIG_SMART_LOCATION_CHANGED[] = "smart,changed";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_CLICKED, ""},
       {SIG_SMART_LOCATION_CHANGED, ""},
       {NULL, NULL}
};

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (evas_object_visible_get(obj))
     evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
   elm_hover_target_set(obj, NULL);
   elm_hover_parent_set(obj, NULL);
   evas_object_event_callback_del_full(wd->hov, EVAS_CALLBACK_MOVE, _hov_move, obj);
   evas_object_event_callback_del_full(wd->hov, EVAS_CALLBACK_RESIZE, _hov_resize, obj);
   evas_object_event_callback_del_full(wd->hov, EVAS_CALLBACK_SHOW, _hov_show, obj);
   evas_object_event_callback_del_full(wd->hov, EVAS_CALLBACK_HIDE, _hov_hide, obj);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->cov, "elm,action,focus", "elm");
        evas_object_focus_set(wd->cov, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->cov, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->cov, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->cov, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   // FIXME: hover contents doesn't seem to propagate resizes properly
   _elm_theme_object_set(obj, wd->cov, "hover", "base", elm_widget_style_get(obj));
   edje_object_scale_set(wd->cov, elm_widget_scale_get(obj) *
                         _elm_config->scale);

   if (wd->smt_sub)
     _elm_hover_sub_obj_placement_eval(obj);
   else
     _reval_content(obj);
   _sizing_eval(obj);
   if (evas_object_visible_get(wd->cov)) _hov_show_do(obj);
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   edje_object_signal_emit(wd->cov, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   edje_object_signal_callback_add(wd->hov, emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);

   edje_object_signal_callback_del_full(wd->hov, emission, source, func_cb,
                                        data);
}

static void
_elm_hover_left_space_calc(Widget_Data *wd, Evas_Coord *spc_l, Evas_Coord *spc_t, Evas_Coord *spc_r, Evas_Coord *spc_b)
{
   Evas_Coord x = 0, y = 0, w = 0, h = 0, x2 = 0, y2 = 0, w2 = 0, h2 = 0;

   if (wd->parent)
     evas_object_geometry_get(wd->parent, &x, &y, &w, &h);
   if (wd->target)
     evas_object_geometry_get(wd->target, &x2, &y2, &w2, &h2);

   *spc_l = x2 - x;
   *spc_r = (x + w) - (x2 + w2);
   if (*spc_l < 0)
     *spc_l = 0;
   if (*spc_r < 0)
     *spc_r = 0;

   *spc_t = y2 - y;
   *spc_b = (y + h) - (y2 + h2);
   if (*spc_t < 0)
     *spc_t = 0;
   if (*spc_b < 0)
     *spc_b = 0;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord ofs_x, x = 0, y = 0, w = 0, h = 0, x2 = 0, y2 = 0, w2 = 0, h2 = 0;
   if (!wd) return;
   if (wd->parent) evas_object_geometry_get(wd->parent, &x, &y, &w, &h);
   if (wd->hov) evas_object_geometry_get(wd->hov, &x2, &y2, &w2, &h2);

   if (elm_widget_mirrored_get(obj))
     ofs_x = w - (x2 - x) - w2;
   else
     ofs_x = x2 - x;

   evas_object_move(wd->cov, x, y);
   evas_object_resize(wd->cov, w, h);
   evas_object_size_hint_min_set(wd->offset, ofs_x, y2 - y);
   evas_object_size_hint_min_set(wd->size, w2, h2);
   edje_object_part_swallow(wd->cov, "elm.swallow.offset", wd->offset);
   edje_object_part_swallow(wd->cov, "elm.swallow.size", wd->size);
}

static void
_reval_content(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   ELM_HOVER_PARTS_FOREACH
     {
        char buf[1024];
        snprintf(buf, sizeof(buf), "elm.swallow.slot.%s", wd->subs[i].swallow);
        edje_object_part_swallow(wd->cov, buf, wd->subs[i].obj);
     }
}

static const char *
_elm_hover_smart_content_location_get(Widget_Data *wd,  Evas_Coord spc_l, Evas_Coord spc_t, Evas_Coord spc_r, Evas_Coord spc_b)
{
   Evas_Coord c_w = 0, c_h = 0, mid_w, mid_h;
   int max;

   evas_object_size_hint_min_get(wd->smt_sub, &c_w, &c_h);
   mid_w = c_w / 2;
   mid_h = c_h / 2;

   if (spc_l > spc_r)
     goto left;

   max = MAX(spc_t, spc_r);
   max = MAX(max, spc_b);

   if (max == spc_t)
     {
        if (mid_w > spc_l)
          return _HOV_TOP_RIGHT;

        return _HOV_TOP;
     }

   if (max == spc_r)
     {
        if (mid_h > spc_t)
          return _HOV_BOTTOM_RIGHT;
        else if (mid_h > spc_b)
          return _HOV_TOP_RIGHT;

        return _HOV_RIGHT;
     }

   if (mid_h > spc_l)
     return _HOV_BOTTOM_RIGHT;

   return _HOV_BOTTOM;

left:
   max = MAX(spc_t, spc_l);
   max = MAX(max, spc_b);

   if (max == spc_t)
     {
        if (mid_w > spc_r)
          return _HOV_TOP_LEFT;

        return _HOV_TOP;
     }

   if (max == spc_l)
     {
        if (mid_h > spc_t)
          return _HOV_BOTTOM_LEFT;
        else if (mid_h > spc_b)
          return _HOV_TOP_LEFT;

        return _HOV_LEFT;
     }

   if (mid_h > spc_r)
     return _HOV_BOTTOM_LEFT;

   return _HOV_BOTTOM;
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd;
   Evas_Object *sub;

   sub = event_info;
   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (wd->smt_sub)
     {
        if (wd->smt_sub == sub)
          wd->smt_sub = NULL;
     }
   else
     {
        ELM_HOVER_PARTS_FOREACH
          {
             if (wd->subs[i].obj == sub)
               {
                  wd->subs[i].obj = NULL;
                  break;
               }
          }
     }
}

static void
_hov_show_do(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (wd->cov)
     {
        evas_object_show(wd->cov);
        edje_object_signal_emit(wd->cov, "elm,action,show", "elm");
     }

   ELM_HOVER_PARTS_FOREACH
     {
        char buf[1024];

        if (wd->subs[i].obj)
          {
             snprintf(buf, sizeof(buf), "elm,action,slot,%s,show",
                      wd->subs[i].swallow);
             edje_object_signal_emit(wd->cov, buf, "elm");
          }
     }
}

static void
_hov_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_hov_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_hov_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _hov_show_do(data);
}

static void
_hov_hide(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->cov)
     {
        edje_object_signal_emit(wd->cov, "elm,action,hide", "elm");
        evas_object_hide(wd->cov);
     }

   ELM_HOVER_PARTS_FOREACH
     {
        char buf[1024];

        if (wd->subs[i].obj)
          {
             snprintf(buf, sizeof(buf), "elm,action,slot,%s,hide",
                      wd->subs[i].swallow);
             edje_object_signal_emit(wd->cov, buf, "elm");
          }
     }
}

static void
_target_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->target = NULL;
}

static void
_target_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd)
     return;

   _sizing_eval(data);
   _elm_hover_sub_obj_placement_eval(data);
}

static void
_signal_dismiss(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_parent_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_parent_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_parent_show(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
}

static void
_parent_hide(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_hide(wd->cov);
}

static void
_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   elm_hover_parent_set(data, NULL);
   _sizing_eval(data);
}

EAPI Evas_Object *
elm_hover_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "hover");
   elm_widget_type_set(obj, "hover");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);

   ELM_HOVER_PARTS_FOREACH
      wd->subs[i].swallow = _directions[i];

   wd->hov = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->hov, EINA_TRUE);
   evas_object_color_set(wd->hov, 0, 0, 0, 0);
   elm_widget_resize_object_set(obj, wd->hov);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_MOVE, _hov_move, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_RESIZE, _hov_resize, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_SHOW, _hov_show, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_HIDE, _hov_hide, obj);

   wd->cov = edje_object_add(e);
   _elm_theme_object_set(obj, wd->cov, "hover", "base", "default");
   elm_widget_sub_object_add(obj, wd->cov);
   edje_object_signal_callback_add(wd->cov, "elm,action,dismiss", "",
                                   _signal_dismiss, obj);

   wd->offset = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->offset, EINA_TRUE);
   evas_object_color_set(wd->offset, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->offset);

   wd->size = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->size, EINA_TRUE);
   evas_object_color_set(wd->size, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->size);

   edje_object_part_swallow(wd->cov, "elm.swallow.offset", wd->offset);
   edje_object_part_swallow(wd->cov, "elm.swallow.size", wd->size);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   elm_hover_parent_set(obj, parent);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_hover_target_set(Evas_Object *obj, Evas_Object *target)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->target)
     {
        evas_object_event_callback_del_full(wd->target, EVAS_CALLBACK_DEL,
                                            _target_del, obj);
        evas_object_event_callback_del_full(wd->target, EVAS_CALLBACK_MOVE,
                                            _target_move, obj);
     }
   wd->target = target;
   if (wd->target)
     {
        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_DEL,
                                       _target_del, obj);
        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_MOVE,
                                       _target_move, obj);
        elm_widget_hover_object_set(target, obj);
        _sizing_eval(obj);
     }
}


EAPI void
elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->parent)
     {
        evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_MOVE,
                                            _parent_move, obj);
        evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_RESIZE,
                                            _parent_resize, obj);
        evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_SHOW,
                                            _parent_show, obj);
        evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_HIDE,
                                            _parent_hide, obj);
        evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_DEL,
                                            _parent_del, obj);
     }
   wd->parent = parent;
   if (wd->parent)
     {
        evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_MOVE,
                                       _parent_move, obj);
        evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_RESIZE,
                                       _parent_resize, obj);
        evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_SHOW,
                                       _parent_show, obj);
        evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_HIDE,
                                       _parent_hide, obj);
        evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_DEL,
                                       _parent_del, obj);
        //	elm_widget_sub_object_add(parent, obj);
     }
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_hover_target_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->target;
}

EAPI Evas_Object *
elm_hover_parent_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->parent;
}

static void
_elm_hover_subs_del(Widget_Data *wd)
{
   ELM_HOVER_PARTS_FOREACH
     {
        if (wd->subs[i].obj)
          {
             evas_object_del(wd->subs[i].obj);
             wd->subs[i].obj = NULL;
          }
     }
}

static void
_elm_hover_sub_obj_placement_eval(Evas_Object *obj)
{
   Evas_Coord spc_l, spc_r, spc_t, spc_b;
   const char *smart_dir;
   Widget_Data *wd;
   char buf[1024];

   wd = elm_widget_data_get(obj);
   if (!wd->smt_sub)
     return;

   _elm_hover_left_space_calc(wd, &spc_l, &spc_t, &spc_r, &spc_b);

   edje_object_part_unswallow(wd->cov, wd->smt_sub);

   smart_dir = _elm_hover_smart_content_location_get(wd, spc_l, spc_t, spc_r,
                                                     spc_b);
   evas_object_smart_callback_call(obj, SIG_SMART_LOCATION_CHANGED,
                                   (void *)smart_dir);

   if (elm_widget_mirrored_get(obj))
     {
        if (smart_dir == _HOV_BOTTOM_LEFT)
          smart_dir = _HOV_BOTTOM_RIGHT;
        else if (smart_dir == _HOV_BOTTOM_RIGHT)
          smart_dir = _HOV_BOTTOM_LEFT;
        else if (smart_dir == _HOV_RIGHT)
          smart_dir = _HOV_LEFT;
        else if (smart_dir == _HOV_LEFT)
          smart_dir = _HOV_RIGHT;
        else if (smart_dir == _HOV_TOP_RIGHT)
          smart_dir = _HOV_TOP_LEFT;
        else if (smart_dir == _HOV_TOP_LEFT)
          smart_dir = _HOV_TOP_RIGHT;
     }
   snprintf(buf, sizeof(buf), "elm.swallow.slot.%s", smart_dir);
   edje_object_part_swallow(wd->cov, buf, wd->smt_sub);
}

static void
_elm_hover_sub_obj_placement_eval_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _elm_hover_sub_obj_placement_eval(data);
}

EAPI void
elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (!strcmp(swallow, "smart"))
     {
        if (wd->smt_sub != content)
          {
             _elm_hover_subs_del(wd);
             wd->smt_sub = content;
          }

        if (content)
          {
             elm_widget_sub_object_add(obj, content);
             evas_object_event_callback_add(wd->smt_sub,
                                            EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _elm_hover_sub_obj_placement_eval_cb,
                                            obj);

             _elm_hover_sub_obj_placement_eval(obj);
          }

        goto end;
     }

   if (wd->smt_sub)
     {
        evas_object_del(wd->smt_sub);
        wd->smt_sub = NULL;
     }

   ELM_HOVER_PARTS_FOREACH
     {
        if (!strcmp(swallow, wd->subs[i].swallow))
          {
             if (content == wd->subs[i].obj)
               return;
             evas_object_del(wd->subs[i].obj);
             wd->subs[i].obj = NULL;

             if (content)
               {
                  char buf[1024];

                  snprintf(buf, sizeof(buf), "elm.swallow.slot.%s", swallow);
                  elm_widget_sub_object_add(obj, content);
                  edje_object_part_swallow(wd->cov, buf, content);
                  wd->subs[i].obj = content;
               }
             break;
          }
     }

end:
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_hover_content_get(const Evas_Object *obj, const char *swallow)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return NULL;

   if (!strcmp(swallow, "smart"))
     return wd->smt_sub;

   ELM_HOVER_PARTS_FOREACH
      if (!strcmp(swallow, wd->subs[i].swallow))
        return wd->subs[i].obj;

   return NULL;
}

static void
_elm_hover_sub_obj_unparent(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);

   elm_widget_sub_object_del(obj, wd->smt_sub);
   evas_object_event_callback_del_full(wd->smt_sub,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _elm_hover_sub_obj_placement_eval_cb,
                                       obj);
   edje_object_part_unswallow(wd->cov, wd->smt_sub);
   wd->smt_sub = NULL;
}

EAPI Evas_Object *
elm_hover_content_unset(Evas_Object *obj, const char *swallow)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return NULL;

   if (!strcmp(swallow, "smart"))
     {
        Evas_Object *content;

        if (!wd->smt_sub)
          return NULL;

        content = wd->smt_sub;
        _elm_hover_sub_obj_unparent(obj);
        return content;
     }

   ELM_HOVER_PARTS_FOREACH
     {
        if (!strcmp(swallow, wd->subs[i].swallow))
          {
             Evas_Object *content;

             if (!wd->subs[i].obj)
               return NULL;

             content = wd->subs[i].obj;
             elm_widget_sub_object_del(obj, wd->subs[i].obj);
             edje_object_part_unswallow(wd->cov, wd->subs[i].obj);
             wd->subs[i].obj = NULL;

             return content;
          }
     }

   return NULL;
}

EAPI const char *
elm_hover_best_content_location_get(const Evas_Object *obj, Elm_Hover_Axis pref_axis)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Evas_Coord spc_l, spc_r, spc_t, spc_b;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return NULL;

   _elm_hover_left_space_calc(wd, &spc_l, &spc_t, &spc_r, &spc_b);

   if (pref_axis == ELM_HOVER_AXIS_HORIZONTAL)
     {
        if (spc_l < spc_r) return _HOV_RIGHT;
        else return _HOV_LEFT;
     }
   else if (pref_axis == ELM_HOVER_AXIS_VERTICAL)
     {
        if (spc_t < spc_b) return _HOV_BOTTOM;
        else return _HOV_TOP;
     }

   if (spc_l < spc_r)
     {
        if (spc_t > spc_r) return _HOV_TOP;
        else if (spc_b > spc_r) return _HOV_BOTTOM;
        return _HOV_RIGHT;
     }
   if (spc_t > spc_r) return _HOV_TOP;
   else if (spc_b > spc_r) return _HOV_BOTTOM;
   return _HOV_LEFT;
}
