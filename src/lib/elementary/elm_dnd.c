#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#include <Elementary.h>
#include "elm_priv.h"

int ELM_CNP_EVENT_SELECTION_CHANGED;

typedef struct {
   void *enter_data, *leave_data, *pos_data, *drop_data;
   Elm_Drag_State enter_cb;
   Elm_Drag_State leave_cb;
   Elm_Drag_Pos pos_cb;
   Elm_Drop_Cb drop_cb;
   Eina_Array *mime_types;
   Elm_Sel_Format format;
   Elm_Xdnd_Action action;
} Elm_Drop_Target;

static int
_default_seat(const Eo *obj)
{
   return evas_device_seat_id_get(evas_default_device_get(evas_object_evas_get(obj), EVAS_DEVICE_CLASS_SEAT));
}

static const char*
_action_to_string(Elm_Xdnd_Action action)
{
   if (action == ELM_XDND_ACTION_COPY) return "copy";
   if (action == ELM_XDND_ACTION_MOVE) return "move";
   if (action == ELM_XDND_ACTION_PRIVATE) return "private";
   if (action == ELM_XDND_ACTION_ASK) return "ask";
   if (action == ELM_XDND_ACTION_LIST) return "list";
   if (action == ELM_XDND_ACTION_LINK) return "link";
   if (action == ELM_XDND_ACTION_DESCRIPTION) return "description";
   return "unknown";
}

static Elm_Xdnd_Action
_string_to_action(const char* action)
{
   if (eina_streq(action, "copy")) return ELM_XDND_ACTION_COPY;
   else if (eina_streq(action, "move")) return ELM_XDND_ACTION_MOVE;
   else if (eina_streq(action, "private")) return ELM_XDND_ACTION_PRIVATE;
   else if (eina_streq(action, "ask")) return ELM_XDND_ACTION_ASK;
   else if (eina_streq(action, "list")) return ELM_XDND_ACTION_LIST;
   else if (eina_streq(action, "link")) return ELM_XDND_ACTION_LINK;
   else if (eina_streq(action, "description")) return ELM_XDND_ACTION_DESCRIPTION;
   return ELM_XDND_ACTION_UNKNOWN;
}

static void
_enter_cb(void *data, const Efl_Event *ev)
{
   Elm_Drop_Target *target = data;

   if (target->enter_cb)
     target->enter_cb(target->enter_data, ev->object);
}

static void
_leave_cb(void *data, const Efl_Event *ev)
{
   Elm_Drop_Target *target = data;

   if (target->leave_cb)
     target->leave_cb(target->leave_data, ev->object);
}

static void
_pos_cb(void *data, const Efl_Event *ev)
{
   Elm_Drop_Target *target = data;
   Efl_Ui_Drop_Event *event = ev->info;

   if (target->pos_cb)
     target->pos_cb(target->pos_data, ev->object, event->position.x, event->position.y, target->action); //FIXME action
}

static Eina_Value
_deliver_content(Eo *obj, void *data, const Eina_Value value)
{
   Elm_Drop_Target *target = data;
   Elm_Selection_Data sel_data;
   Eina_Content *content = eina_value_to_content(&value);

   sel_data.data = (void*)eina_content_data_get(content).mem;
   sel_data.len = eina_content_data_get(content).len;
   sel_data.action = target->action;
   sel_data.format = target->format;

   if (target->drop_cb)
     target->drop_cb(target->drop_data, obj, &sel_data);

   return EINA_VALUE_EMPTY;
}

static void
_drop_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Drop_Dropped_Event *event = ev->info;
   Elm_Drop_Target *target = data;
   target->action = _string_to_action(event->action);
   efl_future_then(ev->object, efl_ui_dnd_drop_data_get(elm_widget_is(ev->object) ? ev->object : efl_ui_win_get(ev->object), _default_seat(ev->object), eina_array_iterator_new(target->mime_types)),
    .success = _deliver_content,
    .data = target
   );
}

static void
_inv_cb(void *data, const Efl_Event *ev)
{
   Elm_Drop_Target *target = data;
   elm_drop_target_del(ev->object, target->format, target->enter_cb, target->enter_data, target->leave_cb,
                       target->leave_data, target->pos_cb, target->pos_data, target->drop_cb, target->drop_data);
}

EFL_CALLBACKS_ARRAY_DEFINE(drop_target_cb,
  {EFL_UI_DND_EVENT_DROP_ENTERED, _enter_cb},
  {EFL_UI_DND_EVENT_DROP_LEFT, _leave_cb},
  {EFL_UI_DND_EVENT_DROP_POSITION_CHANGED, _pos_cb},
  {EFL_UI_DND_EVENT_DROP_DROPPED, _drop_cb},
  {EFL_EVENT_INVALIDATE, _inv_cb}
)

static Eina_Hash *target_register = NULL;

static Eina_Array*
_format_to_mime_array(Elm_Sel_Format format)
{
   Eina_Array *ret = eina_array_new(10);

   if (format & ELM_SEL_FORMAT_TEXT)
     {
        eina_array_push(ret, "text/plain");
        eina_array_push(ret, "text/plain;charset=utf-8");
        eina_array_push(ret, "text/uri-list");
     }
   if (format & ELM_SEL_FORMAT_MARKUP)
     eina_array_push(ret, "application/x-elementary-markup");
   if (format & ELM_SEL_FORMAT_IMAGE)
     {
        eina_array_push(ret, "image/png");
        eina_array_push(ret, "image/jpeg");
        eina_array_push(ret, "image/x-ms-bmp");
        eina_array_push(ret, "image/gif");
        eina_array_push(ret, "image/tiff");
        eina_array_push(ret, "image/svg+xml");
        eina_array_push(ret, "image/x-xpixmap");
        eina_array_push(ret, "image/x-tga");
        eina_array_push(ret, "image/x-portable-pixmap");
     }
   if (format & ELM_SEL_FORMAT_VCARD)
     eina_array_push(ret, "text/vcard");
   if (format & ELM_SEL_FORMAT_HTML)
     eina_array_push(ret, "text/html");

   return ret;
}

EAPI Eina_Bool
elm_drop_target_add(Evas_Object *obj, Elm_Sel_Format format,
                    Elm_Drag_State enter_cb, void *enter_data,
                    Elm_Drag_State leave_cb, void *leave_data,
                    Elm_Drag_Pos pos_cb, void *pos_data,
                    Elm_Drop_Cb drop_cb, void *drop_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   Elm_Drop_Target *target = calloc(1, sizeof(Elm_Drop_Target));
   target->enter_cb = enter_cb;
   target->enter_data = enter_data;
   target->leave_cb = leave_cb;
   target->leave_data = leave_data;
   target->pos_cb = pos_cb;
   target->pos_data = pos_data;
   target->drop_cb = drop_cb;
   target->drop_data = drop_data;
   target->mime_types = _format_to_mime_array(format);
   target->format = format;

   efl_event_callback_array_add(obj, drop_target_cb(), target);
   if (!efl_isa(obj, EFL_UI_WIDGET_CLASS))
     _drop_event_register(obj); //this is ensuring that we are also supporting none widgets
   if (!target_register)
     target_register = eina_hash_pointer_new(NULL);
   eina_hash_list_append(target_register, &obj, target);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_drop_target_del(Evas_Object *obj, Elm_Sel_Format format,
                    Elm_Drag_State enter_cb, void *enter_data,
                    Elm_Drag_State leave_cb, void *leave_data,
                    Elm_Drag_Pos pos_cb, void *pos_data,
                    Elm_Drop_Cb drop_cb, void *drop_data)
{
   Elm_Drop_Target *target;
   Eina_List *n, *found = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   if (!target_register)
     return EINA_TRUE;
   Eina_List *targets = eina_hash_find(target_register, &obj);

   if (!targets)
     return EINA_TRUE;

   EINA_LIST_FOREACH(targets, n, target)
     {
        if (target->enter_cb == enter_cb && target->enter_data == enter_data &&
            target->leave_cb == leave_cb && target->leave_data == leave_data &&
            target->pos_cb == pos_cb && target->pos_data == pos_data &&
            target->drop_cb == drop_cb && target->drop_data == drop_data &&
            target->format == format)
          {

             found = n;
             break;
          }
     }
   if (found)
     {
        efl_event_callback_array_del(obj, drop_target_cb(), eina_list_data_get(found));
        eina_hash_list_remove(target_register, &obj, target);
        eina_array_free(target->mime_types);
        _drop_event_unregister(obj); //this is ensuring that we are also supporting none widgets
        free(target);
     }

   return EINA_TRUE;
}

struct _Item_Container_Drag_Info
{  /* Info kept for containers to support drag */
   Evas_Object *obj;
   Ecore_Timer *tm;    /* When this expires, start drag */
   double anim_tm;  /* Time period to set tm         */
   double tm_to_drag;  /* Time period to set tm         */
   Elm_Xy_Item_Get_Cb itemgetcb;
   Elm_Item_Container_Data_Get_Cb data_get;

   Evas_Coord x_down;  /* Mouse down x cord when drag starts */
   Evas_Coord y_down;  /* Mouse down y cord when drag starts */

   /* Some extra information needed to impl default anim */
   Evas *e;
   Eina_List *icons;   /* List of icons to animate (Anim_Icon) */
   int final_icon_w; /* We need the w and h of the final icon for the animation */
   int final_icon_h;
   Ecore_Animator *ea;

   Elm_Drag_User_Info user_info;
};
typedef struct _Item_Container_Drag_Info Item_Container_Drag_Info;

struct _Anim_Icon
{
   int start_x;
   int start_y;
   int start_w;
   int start_h;
   Evas_Object *o;
};
typedef struct _Anim_Icon Anim_Icon;
static Eina_List *cont_drag_tg = NULL; /* List of Item_Container_Drag_Info */

static Eina_Bool elm_drag_item_container_del_internal(Evas_Object *obj, Eina_Bool full);
static void _cont_obj_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void _cont_obj_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void _cont_obj_mouse_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info);

static void
_cont_drag_done_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Item_Container_Drag_Info *st = data;
   elm_widget_scroll_freeze_pop(st->obj);
   if (st->user_info.dragdone)
     st->user_info.dragdone(st->user_info.donecbdata, NULL, EINA_FALSE); /*FIXME*/
}

static Eina_Bool
_cont_obj_drag_start(void *data)
{  /* Start a drag-action when timer expires */
   Item_Container_Drag_Info *st = data;
   st->tm = NULL;
   Elm_Drag_User_Info *info = &st->user_info;
   if (info->dragstart) info->dragstart(info->startcbdata, st->obj);
   elm_widget_scroll_freeze_push(st->obj);
   evas_object_event_callback_del_full
      (st->obj, EVAS_CALLBACK_MOUSE_MOVE, _cont_obj_mouse_move, st);
   elm_drag_start(  /* Commit the start only if data_get successful */
         st->obj, info->format,
         info->data, info->action,
         info->createicon, info->createdata,
         info->dragpos, info->dragdata,
         info->acceptcb, info->acceptdata,
         _cont_drag_done_cb, st);
   ELM_SAFE_FREE(info->data, free);

   return ECORE_CALLBACK_CANCEL;
}

static inline Eina_List *
_anim_icons_make(Eina_List *icons)
{  /* Make local copies of all icons, add them to list */
   Eina_List *list = NULL, *itr;
   Evas_Object *o;

   EINA_LIST_FOREACH(icons, itr, o)
     {  /* Now add icons to animation window */
        Anim_Icon *st = calloc(1, sizeof(*st));

        if (!st)
          {
             ERR("Failed to allocate memory for icon!");
             continue;
          }

        evas_object_geometry_get(o, &st->start_x, &st->start_y, &st->start_w, &st->start_h);
        evas_object_show(o);
        st->o = o;
        list = eina_list_append(list, st);
     }

   return list;
}

static Eina_Bool
_drag_anim_play(void *data, double pos)
{  /* Impl of the animation of icons, called on frame time */
   Item_Container_Drag_Info *st = data;
   Eina_List *l;
   Anim_Icon *sti;

   if (st->ea)
     {
        if (pos > 0.99)
          {
             st->ea = NULL;  /* Avoid deleting on mouse up */
             EINA_LIST_FOREACH(st->icons, l, sti)
                evas_object_hide(sti->o);

             _cont_obj_drag_start(st);  /* Start dragging */
             return ECORE_CALLBACK_CANCEL;
          }

        Evas_Coord xm, ym;
        evas_pointer_canvas_xy_get(st->e, &xm, &ym);
        EINA_LIST_FOREACH(st->icons, l, sti)
          {
             int x, y, h, w;
             w = sti->start_w + ((st->final_icon_w - sti->start_w) * pos);
             h = sti->start_h + ((st->final_icon_h - sti->start_h) * pos);
             x = sti->start_x - (pos * ((sti->start_x + (w/2) - xm)));
             y = sti->start_y - (pos * ((sti->start_y + (h/2) - ym)));
             evas_object_move(sti->o, x, y);
             evas_object_resize(sti->o, w, h);
          }

        return ECORE_CALLBACK_RENEW;
     }

   return ECORE_CALLBACK_CANCEL;
}

static inline Eina_Bool
_drag_anim_start(void *data)
{  /* Start default animation */
   Item_Container_Drag_Info *st = data;

   st->tm = NULL;
   /* Now we need to build an (Anim_Icon *) list */
   st->icons = _anim_icons_make(st->user_info.icons);
   if (st->user_info.createicon)
     {
        Evas_Object *temp_win = elm_win_add(NULL, "Temp", ELM_WIN_DND);
        Evas_Object *final_icon = st->user_info.createicon(st->user_info.createdata, temp_win, NULL, NULL);
        evas_object_geometry_get(final_icon, NULL, NULL, &st->final_icon_w, &st->final_icon_h);
        evas_object_del(final_icon);
        evas_object_del(temp_win);
     }
   st->ea = ecore_animator_timeline_add(st->anim_tm, _drag_anim_play, st);

   return EINA_FALSE;
}

static Eina_Bool
_cont_obj_anim_start(void *data)
{  /* Start a drag-action when timer expires */
   Item_Container_Drag_Info *st = data;
   int xposret, yposret;  /* Unused */
   Elm_Object_Item *it = (st->itemgetcb) ?
      (st->itemgetcb(st->obj, st->x_down, st->y_down, &xposret, &yposret))
      : NULL;

   st->tm = NULL;
   st->user_info.format = ELM_SEL_FORMAT_TARGETS; /* Default */
   st->icons = NULL;
   st->user_info.data = NULL;
   st->user_info.action = ELM_XDND_ACTION_COPY;  /* Default */

   if (!it)   /* Failed to get mouse-down item, abort drag */
     return ECORE_CALLBACK_CANCEL;

   if (st->data_get)
     {  /* collect info then start animation or start dragging */
        if (st->data_get(    /* Collect drag info */
                 st->obj,      /* The container object */
                 it,           /* Drag started on this item */
                 &st->user_info))
          {
             if (st->user_info.icons)
               _drag_anim_start(st);
             else
               {
                  if (EINA_DBL_NONZERO(st->anim_tm))
                    {
                       // even if we don't manage the icons animation, we have
                       // to wait until it is finished before beginning drag.
                       st->tm = ecore_timer_add(st->anim_tm, _cont_obj_drag_start, st);
                    }
                  else
                    _cont_obj_drag_start(st);  /* Start dragging, no anim */
               }
          }
     }

   return ECORE_CALLBACK_CANCEL;
}

static int
_drag_item_container_cmp(const void *d1,
               const void *d2)
{
   const Item_Container_Drag_Info *st = d1;
   return (((uintptr_t) (st->obj)) - ((uintptr_t) d2));
}

void
_anim_st_free(Item_Container_Drag_Info *st)
{  /* Stops and free mem of ongoing animation */
   if (st)
     {
        ELM_SAFE_FREE(st->ea, ecore_animator_del);
        Anim_Icon *sti;

        EINA_LIST_FREE(st->icons, sti)
          {
             evas_object_del(sti->o);
             free(sti);
          }

        st->icons = NULL;
     }
}

static void
_cont_obj_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{  /* Cancel any drag waiting to start on timeout */
   Item_Container_Drag_Info *st = data;

   if (((Evas_Event_Mouse_Up *)event_info)->button != 1)
     return;  /* We only process left-click at the moment */

   evas_object_event_callback_del_full
      (st->obj, EVAS_CALLBACK_MOUSE_MOVE, _cont_obj_mouse_move, st);
   evas_object_event_callback_del_full
      (st->obj, EVAS_CALLBACK_MOUSE_UP, _cont_obj_mouse_up, st);

   ELM_SAFE_FREE(st->tm, ecore_timer_del);

   _anim_st_free(st);
}

static void
_cont_obj_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{  /* Cancel any drag waiting to start on timeout */
   if (((Evas_Event_Mouse_Move *)event_info)->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        Item_Container_Drag_Info *st = data;

        evas_object_event_callback_del_full
           (st->obj, EVAS_CALLBACK_MOUSE_MOVE, _cont_obj_mouse_move, st);
        evas_object_event_callback_del_full
           (st->obj, EVAS_CALLBACK_MOUSE_UP, _cont_obj_mouse_up, st);
        elm_drag_item_container_del_internal(obj, EINA_FALSE);

        ELM_SAFE_FREE(st->tm, ecore_timer_del);

        _anim_st_free(st);
     }
}

static Eina_Bool
elm_drag_item_container_del_internal(Evas_Object *obj, Eina_Bool full)
{
   Item_Container_Drag_Info *st =
      eina_list_search_unsorted(cont_drag_tg, _drag_item_container_cmp, obj);

   if (st)
     {
        ELM_SAFE_FREE(st->tm, ecore_timer_del); /* Cancel drag-start timer */

        if (st->ea)  /* Cancel ongoing default animation */
          _anim_st_free(st);

        if (full)
          {
             st->itemgetcb = NULL;
             st->data_get = NULL;
             evas_object_event_callback_del_full
                (obj, EVAS_CALLBACK_MOUSE_DOWN, _cont_obj_mouse_down, st);

             cont_drag_tg = eina_list_remove(cont_drag_tg, st);
             ELM_SAFE_FREE(st->user_info.data, free);
             free(st);
          }

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_cont_obj_mouse_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{  /* Launch a timer to start dragging */
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->button != 1)
     return;  /* We only process left-click at the moment */

   Item_Container_Drag_Info *st = data;
   evas_object_event_callback_add(st->obj, EVAS_CALLBACK_MOUSE_MOVE,
         _cont_obj_mouse_move, st);

   evas_object_event_callback_add(st->obj, EVAS_CALLBACK_MOUSE_UP,
         _cont_obj_mouse_up, st);

   ecore_timer_del(st->tm);

   st->e = e;
   st->x_down = ev->canvas.x;
   st->y_down = ev->canvas.y;
   st->tm = ecore_timer_add(st->tm_to_drag, _cont_obj_anim_start, st);
}
EAPI Eina_Bool
elm_drag_item_container_del(Evas_Object *obj)
{
   return elm_drag_item_container_del_internal(obj, EINA_TRUE);
}

EAPI Eina_Bool
elm_drag_item_container_add(Evas_Object *obj,
                            double anim_tm,
                            double tm_to_drag,
                            Elm_Xy_Item_Get_Cb itemgetcb,
                            Elm_Item_Container_Data_Get_Cb data_get)
{
   Item_Container_Drag_Info *st;

   if (elm_drag_item_container_del_internal(obj, EINA_FALSE))
     {  /* Updating info of existing obj */
        st = eina_list_search_unsorted(cont_drag_tg, _drag_item_container_cmp, obj);
        if (!st) return EINA_FALSE;
     }
   else
     {
        st = calloc(1, sizeof(*st));
        if (!st) return EINA_FALSE;

        st->obj = obj;
        cont_drag_tg = eina_list_append(cont_drag_tg, st);

        /* Register for mouse callback for container to start/abort drag */
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                       _cont_obj_mouse_down, st);
     }

   st->tm = NULL;
   st->anim_tm = anim_tm;
   st->tm_to_drag = tm_to_drag;
   st->itemgetcb = itemgetcb;
   st->data_get = data_get;
   return EINA_TRUE;
}

static Eina_List *cont_drop_tg = NULL; /* List of Item_Container_Drop_Info */

struct _Item_Container_Drop_Info
{  /* Info kept for containers to support drop */
   Evas_Object *obj;
   Elm_Xy_Item_Get_Cb itemgetcb;
   Elm_Drop_Item_Container_Cb dropcb;
   Elm_Drag_Item_Container_Pos poscb;
};
typedef struct _Item_Container_Drop_Info Item_Container_Drop_Info;


typedef struct
{
   Evas_Object    *obj;
   /* FIXME: Cache window */
   Eina_Inlist    *cbs_list; /* List of Dropable_Cbs * */
   struct {
      Evas_Coord      x, y;
      Eina_Bool       in : 1;
      const char     *type;
      Elm_Sel_Format  format;
   } last;
} Dropable;

static int
_drop_item_container_cmp(const void *d1,
               const void *d2)
{
   const Item_Container_Drop_Info *st = d1;
   return (((uintptr_t) (st->obj)) - ((uintptr_t) d2));
}

static void
_elm_item_container_pos_cb(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y, Elm_Xdnd_Action action)
{  /* obj is the container pointer */
   Elm_Object_Item *it = NULL;
   int xposret = 0;
   int yposret = 0;

   Item_Container_Drop_Info *st =
      eina_list_search_unsorted(cont_drop_tg, _drop_item_container_cmp, obj);

   if (st && st->poscb)
     {  /* Call container drop func with specific item pointer */
        int xo = 0;
        int yo = 0;

        evas_object_geometry_get(obj, &xo, &yo, NULL, NULL);
        if (st->itemgetcb)
          it = st->itemgetcb(obj, x+xo, y+yo, &xposret, &yposret);

        st->poscb(data, obj, it, x, y, xposret, yposret, action);
     }
}

static Eina_Bool
_elm_item_container_drop_cb(void *data, Evas_Object *obj , Elm_Selection_Data *ev)
{  /* obj is the container pointer */
   Elm_Object_Item *it = NULL;
   int xposret = 0;
   int yposret = 0;

   Item_Container_Drop_Info *st =
      eina_list_search_unsorted(cont_drop_tg, _drop_item_container_cmp, obj);

   if (st && st->dropcb)
     {  /* Call container drop func with specific item pointer */
        int xo = 0;
        int yo = 0;

        evas_object_geometry_get(obj, &xo, &yo, NULL, NULL);
        if (st->itemgetcb)
          it = st->itemgetcb(obj, ev->x+xo, ev->y+yo, &xposret, &yposret);

        return st->dropcb(data, obj, it, ev, xposret, yposret);
     }

   return EINA_FALSE;
}

static Eina_Bool
elm_drop_item_container_del_internal(Evas_Object *obj, Eina_Bool full)
{
   Item_Container_Drop_Info *st =
      eina_list_search_unsorted(cont_drop_tg, _drop_item_container_cmp, obj);

   if (st)
     {
        // temp until st is stored inside data of obj.
        //FIXME delete this drop container
        st->itemgetcb= NULL;
        st->poscb = NULL;
        st->dropcb = NULL;

        if (full)
          {
             cont_drop_tg = eina_list_remove(cont_drop_tg, st);
             free(st);
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_drop_item_container_add(Evas_Object *obj,
                            Elm_Sel_Format format,
                            Elm_Xy_Item_Get_Cb itemgetcb,
                            Elm_Drag_State entercb, void *enterdata,
                            Elm_Drag_State leavecb, void *leavedata,
                            Elm_Drag_Item_Container_Pos poscb, void *posdata,
                            Elm_Drop_Item_Container_Cb dropcb, void *dropdata)
{
   Item_Container_Drop_Info *st;

   if (elm_drop_item_container_del_internal(obj, EINA_FALSE))
     {  /* Updating info of existing obj */
        st = eina_list_search_unsorted(cont_drop_tg, _drop_item_container_cmp, obj);
        if (!st) return EINA_FALSE;
     }
   else
     {
        st = calloc(1, sizeof(*st));
        if (!st) return EINA_FALSE;

        st->obj = obj;
        cont_drop_tg = eina_list_append(cont_drop_tg, st);
     }

   st->itemgetcb = itemgetcb;
   st->poscb = poscb;
   st->dropcb = dropcb;
   elm_drop_target_add(obj, format,
                       entercb, enterdata,
                       leavecb, leavedata,
                       _elm_item_container_pos_cb, posdata,
                       _elm_item_container_drop_cb, dropdata);
   return EINA_TRUE;
}


EAPI Eina_Bool
elm_drop_item_container_del(Evas_Object *obj)
{
   return elm_drop_item_container_del_internal(obj, EINA_TRUE);
}

typedef struct {
  void *dragdata, *acceptdata, *donecbdata;
  Elm_Drag_Pos dragposcb;
  Elm_Drag_Accept acceptcb;
  Elm_Drag_State dragdonecb;
} Elm_Drag_Data;

static void
_drag_finished_cb(void *data, const Efl_Event *ev)
{
   Elm_Drag_Data *dd = data;
   Eina_Bool *accepted = ev->info;

   if (dd->acceptcb)
     dd->acceptcb(dd->acceptdata, ev->object, *accepted);

   if (dd->dragdonecb)
     dd->dragdonecb(dd->donecbdata, ev->object);

   efl_event_callback_del(ev->object, EFL_UI_DND_EVENT_DRAG_FINISHED, _drag_finished_cb, dd);
   free(dd);
}

EAPI Eina_Bool
elm_drag_start(Evas_Object *obj, Elm_Sel_Format format,
               const char *data, Elm_Xdnd_Action action,
               Elm_Drag_Icon_Create_Cb createicon,
               void *createdata,
               Elm_Drag_Pos dragpos, void *dragdata,
               Elm_Drag_Accept acceptcb, void *acceptdata,
               Elm_Drag_State dragdone, void *donecbdata)
{
   Eina_Array *mime_types;
   Eina_Content *content;
   Efl_Content *ui;
   int x, y, w, h;
   Efl_Ui_Widget *widget;
   Elm_Drag_Data *dd;
   const char *str_action;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   //it should return EINA_TRUE to keep backward compatibility
   if (!data)
     return EINA_TRUE;

   str_action = _action_to_string(action);
   dd = calloc(1, sizeof(Elm_Drag_Data));
   dd->dragposcb = dragpos;
   dd->dragdata = dragdata;
   dd->acceptcb = acceptcb;
   dd->acceptdata = acceptdata;
   dd->dragdonecb = dragdone;
   dd->donecbdata = donecbdata;
   mime_types = _format_to_mime_array(format);
   if (eina_array_count(mime_types) != 1)
     {
        WRN("You passed more than one format, this is not going to work well");
     }
   content = eina_content_new((Eina_Slice) EINA_SLICE_STR_FULL(data), eina_array_data_get(mime_types, 0));
   ui = efl_ui_dnd_drag_start(obj, content, str_action, _default_seat(obj));
   widget = createicon(createdata, ui, &x, &y);
   evas_object_geometry_get(widget, NULL, NULL, &w, &h);
   evas_object_show(widget);
   efl_content_set(ui, widget);
   efl_gfx_entity_size_set(ui, EINA_SIZE2D(w, h));
   eina_array_free(mime_types);

   efl_event_callback_add(obj, EFL_UI_DND_EVENT_DRAG_FINISHED, _drag_finished_cb, dd);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_drag_cancel(Evas_Object *obj)
{
   efl_ui_dnd_drag_cancel(obj, _default_seat(obj));

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_drag_action_set(Evas_Object *obj EINA_UNUSED, Elm_Xdnd_Action action EINA_UNUSED)
{
   ERR("This operation is not supported anymore.");
   return EINA_FALSE;
}
