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
   Eina_Bool show_up : 1;
   Eina_Bool show_down : 1;
   Eina_Bool show_left: 1;
   Eina_Bool show_right : 1;
} Scroll_Connector_Context;

static void
_scroll_connector_bar_direction_show_update(Scroll_Connector_Context *ctx)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);
   Eina_Bool hbar_visible = EINA_FALSE, vbar_visible = EINA_FALSE;
   Eina_Bool show_up = EINA_FALSE, show_down = EINA_FALSE, show_left = EINA_FALSE, show_right = EINA_FALSE;
   double vx = 0.0, vy = 0.0;

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.draggable.vertical_bar", NULL, &vy);

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.draggable.horizontal_bar", &vx, NULL);

   efl_ui_scrollbar_bar_visibility_get(ctx->smanager, &hbar_visible, &vbar_visible);
   if (hbar_visible)
     {
        if (vx < 1.0) show_right = EINA_TRUE;
        if (vx > 0.0) show_left = EINA_TRUE;
     }
   if (vbar_visible)
     {
        if (vy < 1.0) show_down = EINA_TRUE;
        if (vy > 0.0) show_up = EINA_TRUE;
     }
   if (show_right != ctx->show_right)
     {
        if (show_right)
          efl_layout_signal_emit(wd->resize_obj, "efl,action,show,right", "efl");
        else
          efl_layout_signal_emit(wd->resize_obj, "efl,action,hide,right", "efl");
        ctx->show_right = show_right;
     }
   if (show_left != ctx->show_left)
     {
        if (show_left)
          efl_layout_signal_emit(wd->resize_obj, "efl,action,show,left", "efl");
        else
          efl_layout_signal_emit(wd->resize_obj, "efl,action,hide,left", "efl");
        ctx->show_left = show_left;
     }
   if (show_up != ctx->show_up)
     {
        if (show_up)
          efl_layout_signal_emit(wd->resize_obj, "efl,action,show,up", "efl");
        else
          efl_layout_signal_emit(wd->resize_obj, "efl,action,hide,up", "efl");
        ctx->show_up = show_up;
     }
   if (show_down != ctx->show_down)
     {
        if (show_down)
          efl_layout_signal_emit(wd->resize_obj, "efl,action,show,down", "efl");
        else
          efl_layout_signal_emit(wd->resize_obj, "efl,action,hide,down", "efl");
        ctx->show_down = show_down;
     }
}

static void
_scroll_connector_bar_read_and_update(Scroll_Connector_Context *ctx)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);
   double vx = 0.0, vy = 0.0;

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.draggable.vertical_bar", NULL, &vy);

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.draggable.horizontal_bar", &vx, NULL);

   efl_ui_scrollbar_bar_position_set(ctx->smanager, vx, vy);
   _scroll_connector_bar_direction_show_update(ctx);
}

static void
_scroll_connector_reload_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);
   Eina_Bool hbar_visible = EINA_FALSE, vbar_visible = EINA_FALSE;

   efl_ui_scrollbar_bar_visibility_get(ctx->smanager, &hbar_visible, &vbar_visible);

   if (hbar_visible)
     efl_layout_signal_emit(wd->resize_obj, "efl,horizontal_bar,visible,on", "efl");
   else
     efl_layout_signal_emit(wd->resize_obj, "efl,horizontal_bar,visible,off", "efl");

   if (vbar_visible)
     efl_layout_signal_emit(wd->resize_obj, "efl,vertical_bar,visible,on", "efl");
   else
     efl_layout_signal_emit(wd->resize_obj, "efl,vertical_bar,visible,off", "efl");

   efl_ui_scrollbar_bar_visibility_update(ctx->smanager);
   _scroll_connector_bar_direction_show_update(ctx);
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
   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_DRAGGED, &type);
}

static void
_scroll_connector_vbar_press_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;

   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_PRESSED, &type);
}

static void
_scroll_connector_vbar_unpress_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;

   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESSED, &type);
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
   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_DRAGGED, &type);
}

static void
_scroll_connector_hbar_press_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL;

   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_PRESSED, &type);
}

static void
_scroll_connector_hbar_unpress_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   Efl_Ui_Layout_Orientation type = EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL;

   efl_event_callback_call(ctx->obj, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESSED, &type);
}

static void
_scroll_connector_bar_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Scroll_Connector_Context *ctx = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);

   double width = 0.0, height = 0.0;

   edje_object_calc_force(wd->resize_obj);
   efl_ui_scrollbar_bar_size_get(ctx->smanager, &width, &height);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.draggable.horizontal_bar", width, 1.0);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.draggable.vertical_bar", 1.0, height);
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

   edje_object_part_drag_step_set(wd->resize_obj, "efl.draggable.horizontal_bar",
                                  (double)step.x / cs.w, 0.0);
   edje_object_part_drag_step_set(wd->resize_obj, "efl.draggable.vertical_bar",
                                  0.0, (double)step.y / cs.h);

   efl_ui_scrollbar_bar_position_get(ctx->smanager, &posx, &posy);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.draggable.horizontal_bar", posx, 0.0);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.draggable.vertical_bar", 0.0, posy);
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
     efl_layout_signal_emit(wd->resize_obj, "efl,horizontal_bar,visible,on", "efl");
   else if (type == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     efl_layout_signal_emit(wd->resize_obj, "efl,vertical_bar,visible,on", "efl");
   _scroll_connector_bar_direction_show_update(ctx);
}

static void
_scroll_connector_bar_hide_cb(void *data, const Efl_Event *event)
{
   Scroll_Connector_Context *ctx = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(ctx->obj, wd);
   Efl_Ui_Layout_Orientation type = *(Efl_Ui_Layout_Orientation *)(event->info);

   if (type == EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL)
     efl_layout_signal_emit(wd->resize_obj, "efl,horizontal_bar,visible,off", "efl");
   else if (type == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     efl_layout_signal_emit(wd->resize_obj, "efl,vertical_bar,visible,off", "efl");
   _scroll_connector_bar_direction_show_update(ctx);
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
   efl_layout_signal_callback_add(obj, "drag", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_vbar_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,set", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,start", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,stop", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,step", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,page", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,vertical_bar,press", "efl",
                                  ctx, _scroll_connector_vbar_press_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,vbar,unpress", "efl",
                                  ctx, _scroll_connector_vbar_unpress_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_hbar_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,set", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,start", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,stop", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,step", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,page", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,horizontal_bar,press", "efl",
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
   efl_layout_signal_callback_del(obj, "drag", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_vbar_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,set", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,start", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,stop", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,step", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,page", "efl.draggable.vertical_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,vertical_bar,press", "efl",
                                  ctx, _scroll_connector_vbar_press_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,vbar,unpress", "efl",
                                  ctx, _scroll_connector_vbar_unpress_cb, NULL);

   efl_layout_signal_callback_del(obj, "drag", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_hbar_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,set", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,start", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,stop", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,step", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,page", "efl.draggable.horizontal_bar",
                                  ctx, _scroll_connector_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,horizontal_bar,press", "efl",
                                  ctx, _scroll_connector_hbar_press_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,hbar,unpress", "efl",
                                  ctx, _scroll_connector_hbar_unpress_cb, NULL);
   free(ctx);
}
