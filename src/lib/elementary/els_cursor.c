#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Elementary_Cursor.h>

#include "elm_priv.h"

#ifdef HAVE_ELEMENTARY_X
#include <Ecore_X.h>
#include <Ecore_X_Cursor.h>
#endif

#ifdef HAVE_ELEMENTARY_WIN32
#include <Ecore_Win32.h>
#endif

#define _cursor_key "_elm_cursor"

struct _Cursor_Id
{
   const char *name;

#if defined(HAVE_ELEMENTARY_X) || defined(HAVE_ELEMENTARY_COCOA) || defined(HAVE_ELEMENTARY_WIN32)
   int id; /* For X */
   int cid; /* For Cocoa */
#endif
};

#if defined(HAVE_ELEMENTARY_X)
# if defined(HAVE_ELEMENTARY_COCOA)
#  define CURSOR(_name, _id, _cid) {_name, ECORE_X_CURSOR_##_id, _cid}
# else
#  define CURSOR(_name, _id, _cid) { _name, ECORE_X_CURSOR_##_id, -1 }
# endif
#elif defined(HAVE_ELEMENTARY_COCOA)
#  define CURSOR(_name, _id, _cid) {_name, -1, _cid}
#elif defined(HAVE_ELEMENTARY_WIN32)
#  define CURSOR(_name, _id, _cid) {_name, ECORE_WIN32_CURSOR_X11_SHAPE_##_id, -1 }
#else
#  define CURSOR(_name, _id, _cid) { _name }
#endif

/* Please keep order in sync with Ecore_X_Cursor.h values! */
static struct _Cursor_Id _cursors[] =
{
   CURSOR(ELM_CURSOR_X                  , X                  , ECORE_COCOA_CURSOR_CROSSHAIR),
   CURSOR(ELM_CURSOR_ARROW              , ARROW              , ECORE_COCOA_CURSOR_ARROW),
   CURSOR(ELM_CURSOR_BASED_ARROW_DOWN   , BASED_ARROW_DOWN   , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_BASED_ARROW_UP     , UP                 , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_BOAT               , BOAT               , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_BOGOSITY           , BOGOSITY           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_BOTTOM_LEFT_CORNER , BOTTOM_LEFT_CORNER , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_BOTTOM_RIGHT_CORNER, BOTTOM_RIGHT_CORNER, ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_BOTTOM_SIDE        , BOTTOM_SIDE        , ECORE_COCOA_CURSOR_RESIZE_DOWN),
   CURSOR(ELM_CURSOR_BOTTOM_TEE         , BOTTOM_TEE         , ECORE_COCOA_CURSOR_RESIZE_DOWN),
   CURSOR(ELM_CURSOR_BOX_SPIRAL         , BOX_SPIRAL         , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_CENTER_PTR         , CENTER_PTR         , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_CIRCLE             , CIRCLE             , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_CLOCK              , CLOCK              , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_COFFEE_MUG         , COFFEE_MUG         , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_CROSS              , CROSS              , ECORE_COCOA_CURSOR_CROSSHAIR),
   CURSOR(ELM_CURSOR_CROSS_REVERSE      , CROSS_REVERSE      , ECORE_COCOA_CURSOR_CROSSHAIR),
   CURSOR(ELM_CURSOR_CROSSHAIR          , CROSSHAIR          , ECORE_COCOA_CURSOR_CROSSHAIR),
   CURSOR(ELM_CURSOR_DIAMOND_CROSS      , DIAMOND_CROSS      , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_DOT                , DOT                , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_DOT_BOX_MASK       , DOT_BOX_MASK       , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_DOUBLE_ARROW       , DOUBLE_ARROW       , ECORE_COCOA_CURSOR_RESIZE_UP_DOWN),
   CURSOR(ELM_CURSOR_DRAFT_LARGE        , DRAFT_LARGE        , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_DRAFT_SMALL        , DRAFT_SMALL        , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_DRAPED_BOX         , DRAPED_BOX         , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_EXCHANGE           , EXCHANGE           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_FLEUR              , FLEUR              , ECORE_COCOA_CURSOR_CLOSED_HAND),
   CURSOR(ELM_CURSOR_GOBBLER            , GOBBLER            , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_GUMBY              , GUMBY              , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_HAND1              , HAND1              , ECORE_COCOA_CURSOR_POINTING_HAND),
   CURSOR(ELM_CURSOR_HAND2              , HAND2              , ECORE_COCOA_CURSOR_POINTING_HAND),
   CURSOR(ELM_CURSOR_HEART              , HEART              , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_ICON               , ICON               , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_IRON_CROSS         , IRON_CROSS         , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_LEFT_PTR           , LEFT_PTR           , ECORE_COCOA_CURSOR_ARROW),
   CURSOR(ELM_CURSOR_LEFT_SIDE          , LEFT_SIDE          , ECORE_COCOA_CURSOR_RESIZE_LEFT),
   CURSOR(ELM_CURSOR_LEFT_TEE           , LEFT_TEE           , ECORE_COCOA_CURSOR_RESIZE_LEFT),
   CURSOR(ELM_CURSOR_LEFTBUTTON         , LEFTBUTTON         , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_LL_ANGLE           , LL_ANGLE           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_LR_ANGLE           , LR_ANGLE           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_MAN                , MAN                , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_MIDDLEBUTTON       , MIDDLEBUTTON       , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_MOUSE              , MOUSE              , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_PENCIL             , PENCIL             , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_PIRATE             , PIRATE             , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_PLUS               , PLUS               , ECORE_COCOA_CURSOR_CROSSHAIR),
   CURSOR(ELM_CURSOR_QUESTION_ARROW     , QUESTION_ARROW     , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_RIGHT_PTR          , RIGHT_PTR          , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_RIGHT_SIDE         , RIGHT_SIDE         , ECORE_COCOA_CURSOR_RESIZE_RIGHT),
   CURSOR(ELM_CURSOR_RIGHT_TEE          , RIGHT_TEE          , ECORE_COCOA_CURSOR_RESIZE_RIGHT),
   CURSOR(ELM_CURSOR_RIGHTBUTTON        , RIGHTBUTTON        , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_RTL_LOGO           , RTL_LOGO           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_SAILBOAT           , SAILBOAT           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_SB_DOWN_ARROW      , SB_DOWN_ARROW      , ECORE_COCOA_CURSOR_RESIZE_DOWN),
   CURSOR(ELM_CURSOR_SB_H_DOUBLE_ARROW  , SB_H_DOUBLE_ARROW  , ECORE_COCOA_CURSOR_RESIZE_LEFT_RIGHT),
   CURSOR(ELM_CURSOR_SB_LEFT_ARROW      , SB_LEFT_ARROW      , ECORE_COCOA_CURSOR_RESIZE_LEFT),
   CURSOR(ELM_CURSOR_SB_RIGHT_ARROW     , SB_RIGHT_ARROW     , ECORE_COCOA_CURSOR_RESIZE_RIGHT),
   CURSOR(ELM_CURSOR_SB_UP_ARROW        , SB_UP_ARROW        , ECORE_COCOA_CURSOR_RESIZE_UP),
   CURSOR(ELM_CURSOR_SB_V_DOUBLE_ARROW  , SB_V_DOUBLE_ARROW  , ECORE_COCOA_CURSOR_RESIZE_UP_DOWN),
   CURSOR(ELM_CURSOR_SHUTTLE            , SHUTTLE            , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_SIZING             , SIZING             , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_SPIDER             , SPIDER             , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_SPRAYCAN           , SPRAYCAN           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_STAR               , STAR               , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_TARGET             , TARGET             , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_TCROSS             , TCROSS             , ECORE_COCOA_CURSOR_CROSSHAIR),
   CURSOR(ELM_CURSOR_TOP_LEFT_ARROW     , TOP_LEFT_ARROW     , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_TOP_LEFT_CORNER    , TOP_LEFT_CORNER    , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_TOP_RIGHT_CORNER   , TOP_RIGHT_CORNER   , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_TOP_SIDE           , TOP_SIDE           , ECORE_COCOA_CURSOR_RESIZE_UP),
   CURSOR(ELM_CURSOR_TOP_TEE            , TOP_TEE            , ECORE_COCOA_CURSOR_RESIZE_UP),
   CURSOR(ELM_CURSOR_TREK               , TREK               , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_UL_ANGLE           , UL_ANGLE           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_UMBRELLA           , UMBRELLA           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_UR_ANGLE           , UR_ANGLE           , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_WATCH              , WATCH              , ECORE_COCOA_CURSOR_DEFAULT),
   CURSOR(ELM_CURSOR_XTERM              , XTERM              , ECORE_COCOA_CURSOR_IBEAM)
};
static const int _cursors_count = sizeof(_cursors)/sizeof(struct _Cursor_Id);

#define ELM_CURSOR_GET_OR_RETURN(cur, obj, ...)         \
  Elm_Cursor *cur;                                      \
  do                                                    \
    {                                                   \
       if (!(obj))                                      \
         {                                              \
            CRI("Null pointer: " #obj);            \
            return __VA_ARGS__;                         \
         }                                              \
       cur = evas_object_data_get((obj), _cursor_key);  \
       if (!cur)                                        \
         {                                              \
            ERR("Object does not have cursor: " #obj);  \
            return __VA_ARGS__;                         \
         }                                              \
    }                                                   \
  while (0)

struct _Elm_Cursor
{
   Evas_Object *obj, *hotobj;
   Evas_Object *eventarea, *owner;
   const char *style, *cursor_name;
   int hot_x, hot_y;
   Ecore_Evas *ee;
   Evas *evas;
   Ecore_Job *hotupdate_job;
#ifdef HAVE_ELEMENTARY_X
   struct {
     Ecore_X_Cursor cursor;
     Ecore_X_Window win;
   } x;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   struct {
     Ecore_Wl2_Window *win;
   } wl;
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   struct {
     Ecore_Win32_Cursor *cursor;
     Ecore_Win32_Window *win;
   } win32;
#endif

#ifdef HAVE_ELEMENTARY_COCOA
   struct {
      Ecore_Cocoa_Cursor  cursor;
      Ecore_Cocoa_Window *win;
   } cocoa;
#endif
   struct
   {
      Evas_Object *obj;
      int layer;
      int x, y;
   } prev;

   Eina_Bool visible:1;
   Eina_Bool use_engine:1;
   Eina_Bool theme_search:1;
};

static void
_elm_cursor_obj_del(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Cursor *cur = data;

   if (cur)
     {
        evas_object_event_callback_del_full(cur->obj, EVAS_CALLBACK_DEL,
                                            _elm_cursor_obj_del, cur);
        cur->obj = NULL;
        ELM_SAFE_FREE(cur->hotobj, evas_object_del);
     }
}

static void
_elm_cursor_set_hot_spots(Elm_Cursor *cur)
{
   const char *str;
   Evas_Coord cx, cy, cw, ch, x, y, w, h;
   int prev_hot_x, prev_hot_y;

   prev_hot_x = cur->hot_x;
   prev_hot_y = cur->hot_y;

   evas_object_geometry_get(cur->obj, &cx, &cy, &cw, &ch);
   evas_object_geometry_get(cur->hotobj, &x, &y, &w, &h);
   cur->hot_x = (x + (w / 2)) - cx;
   cur->hot_y = (y + (h / 2)) - cy;

   str = edje_object_data_get(cur->obj, "hot_x");
   if (str) cur->hot_x = atoi(str);
   str = edje_object_data_get(cur->obj, "hot_y");
   if (str) cur->hot_y = atoi(str);

   if ((cur->visible) &&
       ((prev_hot_x != cur->hot_x) || (prev_hot_y != cur->hot_y)))
     {
        ecore_evas_object_cursor_set(cur->ee, cur->obj,
                                     ELM_OBJECT_LAYER_CURSOR,
                                     cur->hot_x, cur->hot_y);
     }
}

static void
_elm_cursor_set_hot_spots_job(void *data)
{
   Elm_Cursor *cur = data;

   cur->hotupdate_job = NULL;

   _elm_cursor_set_hot_spots(cur);
}

static void
_elm_cursor_hot_change(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Cursor *cur = data;
   if (cur->hotupdate_job) ecore_job_del(cur->hotupdate_job);
   cur->hotupdate_job = ecore_job_add(_elm_cursor_set_hot_spots_job, data);
}

static Eina_Bool
_elm_cursor_obj_add(Evas_Object *obj, Elm_Cursor *cur)
{
   int x, y;

#ifdef HAVE_ELEMENTARY_WL2
   const char *engine_name;

   engine_name = ecore_evas_engine_name_get(cur->ee);
   if ((engine_name) &&
       ((!strcmp(engine_name, ELM_WAYLAND_SHM)) ||
           (!strcmp(engine_name, ELM_WAYLAND_EGL))))
     return EINA_FALSE;
#endif

   cur->obj = edje_object_add(cur->evas);
   if (!cur->obj) return EINA_FALSE;

   if (!_elm_theme_object_set(obj, cur->obj, "cursor", cur->cursor_name,
                             cur->style ? cur->style : "default"))
     {
        ELM_SAFE_FREE(cur->obj, evas_object_del);
        return EINA_FALSE;
     }
   evas_object_data_set(cur->obj, "elm-cursor", (void*)1);
   cur->hotobj = evas_object_rectangle_add(cur->evas);
   evas_object_color_set(cur->hotobj, 0, 0, 0, 0);
   evas_object_event_callback_add(cur->obj, EVAS_CALLBACK_MOVE,
                                  _elm_cursor_hot_change, cur);
   evas_object_event_callback_add(cur->obj, EVAS_CALLBACK_RESIZE,
                                  _elm_cursor_hot_change, cur);
   evas_object_event_callback_add(cur->hotobj, EVAS_CALLBACK_MOVE,
                                  _elm_cursor_hot_change, cur);
   evas_object_event_callback_add(cur->hotobj, EVAS_CALLBACK_RESIZE,
                                  _elm_cursor_hot_change, cur);
   if (edje_object_part_exists(cur->obj, "elm.swallow.hotspot"))
     edje_object_part_swallow(cur->obj, "elm.swallow.hotspot", cur->hotobj);
   else if (edje_object_part_exists(cur->obj, "elm.content.hotspot"))
     edje_object_part_swallow(cur->obj, "elm.content.hotspot", cur->hotobj);
   else
     {
        ELM_SAFE_FREE(cur->hotobj, evas_object_del);
        ELM_SAFE_FREE(cur->obj, evas_object_del);
        return EINA_FALSE;
     }

   evas_object_event_callback_add(cur->obj, EVAS_CALLBACK_DEL,
                                  _elm_cursor_obj_del, cur);
   edje_object_size_min_get(cur->obj, &x, &y);
   edje_object_size_min_restricted_calc(cur->obj, &x, &y, x, y);
   if ((x < 8) || (y < 8))
     {
        x = 8;
        y = 8;
     }
   evas_object_resize(cur->obj, x, y);
   return EINA_TRUE;
}

static void
_elm_cursor_set(Elm_Cursor *cur)
{
   evas_event_freeze(cur->evas);
   if (!cur->use_engine)
     {
        if (cur->visible) goto end;

        if (!cur->obj)
          _elm_cursor_obj_add(cur->owner, cur);
        if (cur->obj)
          {
             ecore_evas_cursor_get(cur->ee, &cur->prev.obj, &cur->prev.layer, &cur->prev.x, &cur->prev.y);
             if (cur->prev.obj)
               {
                  if (evas_object_data_get(cur->prev.obj, "elm-cursor"))
                    memset(&cur->prev, 0, sizeof(cur->prev));
                  else
                    ecore_evas_cursor_unset(cur->ee);
               }
             ecore_evas_object_cursor_set(cur->ee, cur->obj,
                                       ELM_OBJECT_LAYER_CURSOR, cur->hot_x,
                                       cur->hot_y);
          }
        cur->visible = !!cur->obj;
     }
   else
     {
        cur->visible = EINA_TRUE;
        if (cur->obj)
          {
             evas_object_del(cur->obj);
             cur->obj = NULL;
             ELM_SAFE_FREE(cur->hotobj, evas_object_del);
          }
#ifdef HAVE_ELEMENTARY_X
        if (cur->x.win)
          ecore_x_window_cursor_set(cur->x.win, cur->x.cursor);
#endif
#ifdef HAVE_ELEMENTARY_WL2
        if (cur->wl.win)
          {
             Evas_Object *top;

             top = elm_widget_top_get(cur->owner);
             if ((top) && (efl_isa(top, EFL_UI_WIN_CLASS)))
               _elm_win_wl_cursor_set(top, cur->cursor_name);
          }
#endif

#ifdef HAVE_ELEMENTARY_COCOA
        if (cur->cocoa.win)
          ecore_cocoa_window_cursor_set(cur->cocoa.win, cur->cocoa.cursor);
#endif

#ifdef HAVE_ELEMENTARY_WIN32
        if (cur->win32.win)
          ecore_win32_window_cursor_set(cur->win32.win, cur->win32.cursor);
#endif
     }
end:
   evas_event_thaw(cur->evas);
}

static void
_elm_cursor_mouse_in(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Cursor *cur = data;

   Evas_Event_Mouse_In *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   _elm_cursor_set(cur);
}

static void
_elm_cursor_mouse_out(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Object *sobj_parent;
   Elm_Cursor *pcur = NULL;
   Elm_Cursor *cur = data;

   Evas_Event_Mouse_Out *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   if (!cur->visible) return;
   evas_event_freeze(cur->evas);
   cur->visible = EINA_FALSE;

   sobj_parent = evas_object_data_get(cur->eventarea, "elm-parent");
   while (sobj_parent)
     {
        pcur = evas_object_data_get((sobj_parent), _cursor_key);
        if ((pcur) && (pcur->visible)) break;
        sobj_parent = evas_object_data_get(sobj_parent, "elm-parent");
     }

   if (pcur)
     {
        pcur->visible = EINA_FALSE;
        evas_event_thaw(cur->evas);
        _elm_cursor_set(pcur);
        return;
     }

   if (!cur->use_engine)
     {
        if (cur->prev.obj)
          ecore_evas_object_cursor_set(cur->ee, cur->prev.obj, cur->prev.layer,
                                       cur->prev.x, cur->prev.y);
        else
          ecore_evas_object_cursor_set(cur->ee, NULL, ELM_OBJECT_LAYER_CURSOR,
                                       cur->hot_x, cur->hot_y);
        memset(&cur->prev, 0, sizeof(cur->prev));
     }
   else
     {
#ifdef HAVE_ELEMENTARY_X
        if (cur->x.win)
          ecore_x_window_cursor_set(cur->x.win, ECORE_X_CURSOR_X);
#endif
#ifdef HAVE_ELEMENTARY_WL2
        if (cur->wl.win)
          {
             Evas_Object *top;

             top = elm_widget_top_get(cur->owner);
             if ((top) && (efl_isa(top, EFL_UI_WIN_CLASS)))
               _elm_win_wl_cursor_set(top, NULL);
          }
#endif

#ifdef HAVE_ELEMENTARY_COCOA
        if (cur->cocoa.win)
          ecore_cocoa_window_cursor_set(cur->cocoa.win, ECORE_COCOA_CURSOR_DEFAULT);
#endif

#ifdef HAVE_ELEMENTARY_WIN32
        if (cur->win32.win)
          ecore_win32_window_cursor_set(cur->win32.win, ecore_win32_cursor_shaped_new(ECORE_WIN32_CURSOR_SHAPE_ARROW));
#endif
     }
   evas_event_thaw(cur->evas);
}

static void
_elm_cursor_del(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_object_cursor_unset(obj);
}

#if defined(HAVE_ELEMENTARY_X) || defined(HAVE_ELEMENTARY_COCOA) || defined(HAVE_ELEMENTARY_WIN32)
static int
_elm_cursor_strcmp(const void *data1, const void *data2)
{
   const struct _Cursor_Id *c1 = data1;
   const struct _Cursor_Id *c2 = data2;
   return strcmp (c1->name, c2->name);
}
#endif

static void
_elm_cursor_cur_set(Elm_Cursor *cur)
{
   if (!cur->theme_search)
     {
        INF("Using only engine cursors");
        cur->use_engine = EINA_TRUE;
     }
   else if (_elm_cursor_obj_add(cur->owner, cur))
     {
        _elm_cursor_set_hot_spots(cur);
        cur->use_engine = EINA_FALSE;
        elm_widget_cursor_add(cur->owner, cur);
     }
   else
     {
        INF("Cursor couldn't be found on theme: %s", cur->cursor_name);
        cur->use_engine = EINA_TRUE;
     }

#ifdef HAVE_ELEMENTARY_DRM
   const char *engine_name;

   engine_name = ecore_evas_engine_name_get(cur->ee);
   if ((engine_name) && (!strcmp(engine_name, ELM_DRM)))
     cur->use_engine = EINA_FALSE;
#endif

   if (cur->use_engine)
     {
        Evas_Object *top;

        top = elm_widget_top_get(cur->owner);
        if ((top) && (efl_isa(top, EFL_UI_WIN_CLASS)))
          {
#ifdef HAVE_ELEMENTARY_X
             cur->x.win = elm_win_xwindow_get(top);
             if (cur->x.win)
               {
                  struct _Cursor_Id *cur_id;

                  cur_id = bsearch(&(cur->cursor_name), _cursors, _cursors_count,
                                   sizeof(struct _Cursor_Id), _elm_cursor_strcmp);

                  if (!cur_id)
                    {
                       INF("X cursor couldn't be found: %s. Using default.",
                           cur->cursor_name);
                       cur->x.cursor = ecore_x_cursor_shape_get(ECORE_X_CURSOR_X);
                    }
                  else
                    cur->x.cursor = ecore_x_cursor_shape_get(cur_id->id);
               }
#endif

#ifdef HAVE_ELEMENTARY_COCOA
             cur->cocoa.win = elm_win_cocoa_window_get(top);
             if (cur->cocoa.win)
               {
                  struct _Cursor_Id *cur_id;

                  cur_id = bsearch(&(cur->cursor_name), _cursors, _cursors_count,
                                   sizeof(struct _Cursor_Id), _elm_cursor_strcmp);
                  if (!cur_id)
                    {
                       INF("Cocoa Cursor couldn't be found: %s. Using default...",
                           cur->cursor_name);
                       cur->cocoa.cursor = ECORE_COCOA_CURSOR_DEFAULT;
                    }
                  else
                    cur->cocoa.cursor = cur_id->cid;
               }
#endif
#ifdef HAVE_ELEMENTARY_WL2
             cur->wl.win = elm_win_wl_window_get(top);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
             cur->win32.win = elm_win_win32_window_get(top);
             if (cur->win32.win)
               {
                  struct _Cursor_Id *cur_id;

                  cur_id = bsearch(&(cur->cursor_name), _cursors, _cursors_count,
                                   sizeof(struct _Cursor_Id), _elm_cursor_strcmp);

                  if (!cur_id)
                    {
                       INF("Win32 X cursor couldn't be found: %s. Using default.",
                           cur->cursor_name);
                       cur->win32.cursor = ecore_win32_cursor_shaped_new(ECORE_WIN32_CURSOR_SHAPE_ARROW);
                    }
                  else
                    cur->win32.cursor = (Ecore_Win32_Cursor *)ecore_win32_cursor_x11_shaped_get(cur_id->id);
               }
#endif
          }
     }

   if (efl_canvas_object_pointer_in_get(cur->eventarea))
     _elm_cursor_set(cur);
}

/**
 * Set the cursor to be shown when mouse is over the object
 *
 * Set the cursor that will be displayed when mouse is over the
 * object. The object can have only one cursor set to it, so if
 * this function is called twice for an object, the previous set
 * will be unset.
 * If using X cursors, a definition of all the valid cursor names
 * is listed on Elementary_Cursors.h. If an invalid name is set
 * the default cursor will be used.
 *
 * This is an internal function that is used by objects with sub-items
 * that want to provide different cursors for each of them. The @a
 * owner object should be an elm_widget and will be used to track
 * theme changes and to feed @a func and @a del_cb. The @a eventarea
 * may be any object and is the one that should be used later on with
 * elm_object_cursor apis, such as elm_object_cursor_unset().
 *
 * @param eventarea the object being attached a cursor.
 * @param owner the elm_widget that owns this object, will be used to
 *        track theme changes and to be used in @a func or @a del_cb.
 * @param cursor the cursor name to be used.
 *
 * @internal
 * @ingroup Elm_Cursors
 */
void
elm_object_sub_cursor_set(Evas_Object *eventarea, Evas_Object *owner, const char *cursor)
{
   Elm_Cursor *cur = NULL;

   cur = evas_object_data_get(eventarea, _cursor_key);
   if (cur)
     elm_object_cursor_unset(eventarea);

   if (!cursor) return;

   cur = ELM_NEW(Elm_Cursor);
   if (!cur) return;

   cur->owner = owner;
   cur->eventarea = eventarea;
   cur->theme_search = !_elm_config->cursor_engine_only;
   cur->visible = EINA_FALSE;
   cur->style = eina_stringshare_add("default");

   cur->cursor_name = eina_stringshare_add(cursor);
   if (!cur->cursor_name)
     ERR("Could not store cursor name %s", cursor);

   cur->evas = evas_object_evas_get(eventarea);
   cur->ee = ecore_evas_ecore_evas_get(cur->evas);

   _elm_cursor_cur_set(cur);

   evas_object_data_set(eventarea, _cursor_key, cur);

   evas_object_event_callback_add(eventarea, EVAS_CALLBACK_MOUSE_IN,
                                  _elm_cursor_mouse_in, cur);
   evas_object_event_callback_add(eventarea, EVAS_CALLBACK_MOUSE_OUT,
                                  _elm_cursor_mouse_out, cur);
   evas_object_event_callback_add(eventarea, EVAS_CALLBACK_DEL,
                                  _elm_cursor_del, cur);
}

EOLIAN void
_elm_widget_efl_ui_cursor_cursor_set(Evas_Object *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED,
                                     const char *cursor)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_object_sub_cursor_set(obj, obj, cursor);
}

EOLIAN const char *
_elm_widget_efl_ui_cursor_cursor_get(const Evas_Object *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj, NULL);
   return cur->cursor_name;
}

EAPI void
elm_object_cursor_unset(Evas_Object *obj)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj);

   if (cur->hotupdate_job) ecore_job_del(cur->hotupdate_job);
   cur->hotupdate_job = NULL;

   eina_stringshare_del(cur->cursor_name);
   cur->cursor_name = NULL;
   eina_stringshare_del(cur->style);
   cur->style = NULL;

   if (cur->owner)
     elm_widget_cursor_del(cur->owner, cur);

   if (cur->obj)
     {
        evas_object_event_callback_del_full(cur->obj, EVAS_CALLBACK_DEL,
                                            _elm_cursor_obj_del, cur);
        ELM_SAFE_FREE(cur->obj, evas_object_del);
     }

   if (cur->visible)
     {
        if (!cur->use_engine)
          ecore_evas_object_cursor_set(cur->ee, NULL, ELM_OBJECT_LAYER_CURSOR,
                                       cur->hot_x, cur->hot_y);
#ifdef HAVE_ELEMENTARY_X
        else if (cur->x.win)
          ecore_x_window_cursor_set(cur->x.win, ECORE_X_CURSOR_X);
#endif
#ifdef HAVE_ELEMENTARY_COCOA
        else if (cur->cocoa.win)
          ecore_cocoa_window_cursor_set(cur->cocoa.win, ECORE_COCOA_CURSOR_DEFAULT);
#endif
#ifdef HAVE_ELEMENTARY_WL2
        else if (cur->wl.win)
          {
             Evas_Object *top;

             top = elm_widget_top_get(cur->owner);
             if ((top) && (efl_isa(top, EFL_UI_WIN_CLASS)))
               _elm_win_wl_cursor_set(top, NULL);
          }
#endif
#ifdef HAVE_ELEMENTARY_WIN32
        else
          ecore_win32_window_cursor_set(cur->win32.win, ecore_win32_cursor_shaped_new(ECORE_WIN32_CURSOR_SHAPE_ARROW));
#endif
     }

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOUSE_IN,
                                  _elm_cursor_mouse_in, cur);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOUSE_OUT,
                                  _elm_cursor_mouse_out, cur);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _elm_cursor_del, cur);
   evas_object_data_del(obj, _cursor_key);
   free(cur);
}

EOLIAN void
_elm_widget_efl_ui_cursor_cursor_style_set(Evas_Object *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED,
                                           const char *style)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj);

   if (!eina_stringshare_replace(&cur->style, style))
     ERR("Could not set current style=%s", style);

   if (cur->use_engine) return;

   if (!cur->obj)
     {
        if (!_elm_cursor_obj_add(cur->owner, cur))
          ERR("Could not create cursor object");
        else
          _elm_cursor_set_hot_spots(cur);
     }
   else
     {
        if (!_elm_theme_object_set(obj, cur->obj, "cursor", cur->cursor_name,
                                   style))
          ERR("Could not apply the theme to the cursor style=%s", style);
        else
          _elm_cursor_set_hot_spots(cur);
     }
}

EOLIAN const char *
_elm_widget_efl_ui_cursor_cursor_style_get(const Evas_Object *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj, NULL);
   return cur->style ? cur->style : "default";
}

/**
 * Notify cursor should recalculate its theme.
 * @internal
 */
void
elm_cursor_theme(Elm_Cursor *cur)
{
   if ((!cur) || (!cur->obj)) return;
   if (!_elm_theme_object_set(cur->owner, cur->obj, "cursor",
                              cur->cursor_name, cur->style))
     ERR("Could not apply the theme to the cursor style=%s", cur->style);
   else
     _elm_cursor_set_hot_spots(cur);
}

EOLIAN void
_elm_widget_efl_ui_cursor_cursor_theme_search_enabled_set(Evas_Object *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED,
                                                          Eina_Bool theme_search)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj);
   cur->theme_search = theme_search;
   ELM_SAFE_FREE(cur->obj, evas_object_del);
   _elm_cursor_cur_set(cur);
}

EOLIAN Eina_Bool
_elm_widget_efl_ui_cursor_cursor_theme_search_enabled_get(const Evas_Object *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj, EINA_FALSE);
   return cur->theme_search;
}

#include "efl_ui_cursor.eo.c"
