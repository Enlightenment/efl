#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Elementary.h>
#include <Efl_Ui.h>
#include "elm_priv.h"

typedef struct {
   Eo *obj;
   Eo *smanager;
   int freeze_want;
   Eina_Bool scroll_count : 1;
   Eina_Bool need_scroll : 1;
} Scroll_Connector_Context;

static void
_scroll_connector_bar_read_and_update(Scroll_Connector_Context *ctx)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);
   double vx = 0.0, vy = 0.0;

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.dragable.vbar", NULL, &vy);

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.dragable.hbar", &vx, NULL);

   efl_ui_scrollbar_bar_position_set(ctx->smanager, vx, vy);
}

static void
_scroll_connector_reload_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;

   efl_ui_scrollbar_bar_visibility_update(ctx->smanager);
}


static void
_scroll_connector_edje_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;

   _scroll_connector_bar_read_and_update(ctx);
}


static void
_scroll(void *data,
                          Evas_Object *obj,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;

   ctx->scroll_count = EINA_FALSE;
   if (!ctx->need_scroll) return;
   ctx->need_scroll = EINA_FALSE;
   efl_layout_signal_emit(obj, "efl,action,scroll", "efl");
}

static void
_scroll_connector_edje_drag_start_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;

   _scroll_connector_bar_read_and_update(ctx);

   ctx->freeze_want = efl_ui_scrollable_scroll_freeze_get(ctx->smanager);
   efl_ui_scrollable_scroll_freeze_set(ctx->smanager, EINA_TRUE);
   efl_event_callback_call(ctx->obj, EFL_UI_EVENT_SCROLL_DRAG_STARTED, NULL);
}

static void
_scroll_connector_edje_drag_stop_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission EINA_UNUSED,
                               const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;

   _scroll_connector_bar_read_and_update(ctx);
   EINA_SAFETY_ON_TRUE_RETURN(ctx->freeze_want == -1);
   efl_ui_scrollable_scroll_freeze_set(ctx->smanager, ctx->freeze_want);
   ctx->freeze_want = -1;
   efl_event_callback_call(ctx->obj, EFL_UI_EVENT_SCROLL_DRAG_FINISHED, NULL);
}

static void
_scroll_connector_vbar_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type;

   _scroll_connector_bar_read_and_update(ctx);

   type = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;
   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_scroll_connector_vbar_press_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;

   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_scroll_connector_vbar_unpress_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;

   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_scroll_connector_hbar_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL;

   _scroll_connector_bar_read_and_update(ctx);
   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_scroll_connector_hbar_press_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL;

   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_scroll_connector_hbar_unpress_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL;

   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_scroll_connector_bar_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);

   double width = 0.0, height = 0.0;

   edje_object_calc_force(wd->resize_obj);
   efl_ui_scrollbar_bar_size_get(ctx->smanager, &width, &height);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.hbar", width, 1.0);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.vbar", 1.0, height);
}

static void
_scroll_connector_bar_pos_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);

   double posx = 0.0, posy = 0.0;
   Eina_Size2D cs;
   Eina_Position2D step;

   step = efl_ui_scrollable_step_size_get(ctx->smanager);
   cs = efl_ui_scrollable_content_size_get(ctx->smanager);

   edje_object_part_drag_step_set(wd->resize_obj, "efl.dragable.hbar",
                                  (double)step.x / cs.w, 0.0);
   edje_object_part_drag_step_set(wd->resize_obj, "efl.dragable.vbar",
                                  0.0, (double)step.y / cs.h);

   efl_ui_scrollbar_bar_position_get(ctx->smanager, &posx, &posy);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.hbar", posx, 0.0);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.vbar", 0.0, posy);
   if (ctx->scroll_count)
     ctx->need_scroll = EINA_TRUE;
   else
     {
        efl_layout_signal_emit(wd->resize_obj, "efl,action,scroll", "efl");
        ctx->scroll_count = EINA_TRUE;
     }
}

static void
_scroll_connector_bar_show_cb(void *data, const Efl_Event *event)
{
   Scroll_Connector_Context *ctx = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);
   Efl_Ui_Layout_Orientation type = *(Efl_Ui_Layout_Orientation *)(event->info);

   if (type == EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL)
     efl_layout_signal_emit(wd->resize_obj, "efl,action,show,hbar", "efl");
   else if (type == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     efl_layout_signal_emit(wd->resize_obj, "efl,action,show,vbar", "efl");
}

static void
_scroll_connector_bar_hide_cb(void *data, const Efl_Event *event)
{
   Scroll_Connector_Context *ctx = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);
   Efl_Ui_Layout_Orientation type = *(Efl_Ui_Layout_Orientation *)(event->info);

   if (type == EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL)
     efl_layout_signal_emit(wd->resize_obj, "efl,action,hide,hbar", "efl");
   else if (type == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     efl_layout_signal_emit(wd->resize_obj, "efl,action,hide,vbar", "efl");
}

void
efl_ui_scroll_connector_bind(Eo *obj, Eo *manager)
{
   Scroll_Connector_Context *ctx = calloc(1, sizeof(Scroll_Connector_Context));
   if (!ctx) return;
   ctx->obj = obj;
   ctx->smanager = manager;
   efl_key_data_set(obj, "__context", ctx);

   //from the theme to the object
   efl_layout_signal_callback_add(obj, "reload", "efl",
                                  ctx, _scroll_connector_reload_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag", "efl.dragable.vbar",
                                  ctx, _scroll_connector_vbar_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,set", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,start", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,stop", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,step", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,page", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,vbar,press", "efl",
                                  ctx, _scroll_connector_vbar_press_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,vbar,unpress", "efl",
                                  ctx, _scroll_connector_vbar_unpress_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag", "efl.dragable.hbar",
                                  ctx, _scroll_connector_hbar_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,set", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,start", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,stop", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,step", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,page", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,hbar,press", "efl",
                                  ctx, _scroll_connector_hbar_press_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,hbar,unpress", "efl",
                                  ctx, _scroll_connector_hbar_unpress_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,action,scroll", "efl",
                                  ctx, _scroll, NULL);

   //from the object to the theme
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _scroll_connector_bar_size_changed_cb, ctx);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _scroll_connector_bar_pos_changed_cb, ctx);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _scroll_connector_bar_show_cb, ctx);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _scroll_connector_bar_hide_cb, ctx);
}

void
efl_ui_scroll_connector_unbind(Eo *obj)
{
   Scroll_Connector_Context *ctx;

   ctx = efl_key_data_get(obj, "__context");
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   efl_layout_signal_callback_del(obj, "reload", "efl",
                                  ctx, _scroll_connector_reload_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag", "efl.dragable.vbar",
                                  ctx, _scroll_connector_vbar_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,set", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,start", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,stop", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,step", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,page", "efl.dragable.vbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,vbar,press", "efl",
                                  ctx, _scroll_connector_vbar_press_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,vbar,unpress", "efl",
                                  ctx, _scroll_connector_vbar_unpress_cb, NULL);

   efl_layout_signal_callback_del(obj, "drag", "efl.dragable.hbar",
                                  ctx, _scroll_connector_hbar_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,set", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,start", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,stop", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,step", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,page", "efl.dragable.hbar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,hbar,press", "efl",
                                  ctx, _scroll_connector_hbar_press_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,hbar,unpress", "efl",
                                  ctx, _scroll_connector_hbar_unpress_cb, NULL);
   free(ctx);
}
