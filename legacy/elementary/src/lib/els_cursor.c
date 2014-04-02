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

#define _cursor_key "_elm_cursor"

struct _Cursor_Id
{
   const char *name;
#ifdef HAVE_ELEMENTARY_X
   int id;
#endif
};

#ifdef HAVE_ELEMENTARY_X
#define CURSOR(_name, _xid) \
   {_name , _xid}
# else
#define CURSOR(_name, _xid) \
   {_name}
#endif

/* Please keep order in sync with Ecore_X_Cursor.h values! */
struct _Cursor_Id _cursors[] =
{
   CURSOR(ELM_CURSOR_X                  , ECORE_X_CURSOR_X                  ),
   CURSOR(ELM_CURSOR_ARROW              , ECORE_X_CURSOR_ARROW              ),
   CURSOR(ELM_CURSOR_BASED_ARROW_DOWN   , ECORE_X_CURSOR_BASED_ARROW_DOWN   ),
   CURSOR(ELM_CURSOR_BASED_ARROW_UP     , ECORE_X_CURSOR_UP                 ),
   CURSOR(ELM_CURSOR_BOAT               , ECORE_X_CURSOR_BOAT               ),
   CURSOR(ELM_CURSOR_BOGOSITY           , ECORE_X_CURSOR_BOGOSITY           ),
   CURSOR(ELM_CURSOR_BOTTOM_LEFT_CORNER , ECORE_X_CURSOR_BOTTOM_LEFT_CORNER ),
   CURSOR(ELM_CURSOR_BOTTOM_RIGHT_CORNER, ECORE_X_CURSOR_BOTTOM_RIGHT_CORNER),
   CURSOR(ELM_CURSOR_BOTTOM_SIDE        , ECORE_X_CURSOR_BOTTOM_SIDE        ),
   CURSOR(ELM_CURSOR_BOTTOM_TEE         , ECORE_X_CURSOR_BOTTOM_TEE         ),
   CURSOR(ELM_CURSOR_BOX_SPIRAL         , ECORE_X_CURSOR_BOX_SPIRAL         ),
   CURSOR(ELM_CURSOR_CENTER_PTR         , ECORE_X_CURSOR_CENTER_PTR         ),
   CURSOR(ELM_CURSOR_CIRCLE             , ECORE_X_CURSOR_CIRCLE             ),
   CURSOR(ELM_CURSOR_CLOCK              , ECORE_X_CURSOR_CLOCK              ),
   CURSOR(ELM_CURSOR_COFFEE_MUG         , ECORE_X_CURSOR_COFFEE_MUG         ),
   CURSOR(ELM_CURSOR_CROSS              , ECORE_X_CURSOR_CROSS              ),
   CURSOR(ELM_CURSOR_CROSS_REVERSE      , ECORE_X_CURSOR_CROSS_REVERSE      ),
   CURSOR(ELM_CURSOR_CROSSHAIR          , ECORE_X_CURSOR_CROSSHAIR          ),
   CURSOR(ELM_CURSOR_DIAMOND_CROSS      , ECORE_X_CURSOR_DIAMOND_CROSS      ),
   CURSOR(ELM_CURSOR_DOT                , ECORE_X_CURSOR_DOT                ),
   CURSOR(ELM_CURSOR_DOT_BOX_MASK       , ECORE_X_CURSOR_DOT_BOX_MASK       ),
   CURSOR(ELM_CURSOR_DOUBLE_ARROW       , ECORE_X_CURSOR_DOUBLE_ARROW       ),
   CURSOR(ELM_CURSOR_DRAFT_LARGE        , ECORE_X_CURSOR_DRAFT_LARGE        ),
   CURSOR(ELM_CURSOR_DRAFT_SMALL        , ECORE_X_CURSOR_DRAFT_SMALL        ),
   CURSOR(ELM_CURSOR_DRAPED_BOX         , ECORE_X_CURSOR_DRAPED_BOX         ),
   CURSOR(ELM_CURSOR_EXCHANGE           , ECORE_X_CURSOR_EXCHANGE           ),
   CURSOR(ELM_CURSOR_FLEUR              , ECORE_X_CURSOR_FLEUR              ),
   CURSOR(ELM_CURSOR_GOBBLER            , ECORE_X_CURSOR_GOBBLER            ),
   CURSOR(ELM_CURSOR_GUMBY              , ECORE_X_CURSOR_GUMBY              ),
   CURSOR(ELM_CURSOR_HAND1              , ECORE_X_CURSOR_HAND1              ),
   CURSOR(ELM_CURSOR_HAND2              , ECORE_X_CURSOR_HAND2              ),
   CURSOR(ELM_CURSOR_HEART              , ECORE_X_CURSOR_HEART              ),
   CURSOR(ELM_CURSOR_ICON               , ECORE_X_CURSOR_ICON               ),
   CURSOR(ELM_CURSOR_IRON_CROSS         , ECORE_X_CURSOR_IRON_CROSS         ),
   CURSOR(ELM_CURSOR_LEFT_PTR           , ECORE_X_CURSOR_LEFT_PTR           ),
   CURSOR(ELM_CURSOR_LEFT_SIDE          , ECORE_X_CURSOR_LEFT_SIDE          ),
   CURSOR(ELM_CURSOR_LEFT_TEE           , ECORE_X_CURSOR_LEFT_TEE           ),
   CURSOR(ELM_CURSOR_LEFTBUTTON         , ECORE_X_CURSOR_LEFTBUTTON         ),
   CURSOR(ELM_CURSOR_LL_ANGLE           , ECORE_X_CURSOR_LL_ANGLE           ),
   CURSOR(ELM_CURSOR_LR_ANGLE           , ECORE_X_CURSOR_LR_ANGLE           ),
   CURSOR(ELM_CURSOR_MAN                , ECORE_X_CURSOR_MAN                ),
   CURSOR(ELM_CURSOR_MIDDLEBUTTON       , ECORE_X_CURSOR_MIDDLEBUTTON       ),
   CURSOR(ELM_CURSOR_MOUSE              , ECORE_X_CURSOR_MOUSE              ),
   CURSOR(ELM_CURSOR_PENCIL             , ECORE_X_CURSOR_PENCIL             ),
   CURSOR(ELM_CURSOR_PIRATE             , ECORE_X_CURSOR_PIRATE             ),
   CURSOR(ELM_CURSOR_PLUS               , ECORE_X_CURSOR_PLUS               ),
   CURSOR(ELM_CURSOR_QUESTION_ARROW     , ECORE_X_CURSOR_QUESTION_ARROW     ),
   CURSOR(ELM_CURSOR_RIGHT_PTR          , ECORE_X_CURSOR_RIGHT_PTR          ),
   CURSOR(ELM_CURSOR_RIGHT_SIDE         , ECORE_X_CURSOR_RIGHT_SIDE         ),
   CURSOR(ELM_CURSOR_RIGHT_TEE          , ECORE_X_CURSOR_RIGHT_TEE          ),
   CURSOR(ELM_CURSOR_RIGHTBUTTON        , ECORE_X_CURSOR_RIGHTBUTTON        ),
   CURSOR(ELM_CURSOR_RTL_LOGO           , ECORE_X_CURSOR_RTL_LOGO           ),
   CURSOR(ELM_CURSOR_SAILBOAT           , ECORE_X_CURSOR_SAILBOAT           ),
   CURSOR(ELM_CURSOR_SB_DOWN_ARROW      , ECORE_X_CURSOR_SB_DOWN_ARROW      ),
   CURSOR(ELM_CURSOR_SB_H_DOUBLE_ARROW  , ECORE_X_CURSOR_SB_H_DOUBLE_ARROW  ),
   CURSOR(ELM_CURSOR_SB_LEFT_ARROW      , ECORE_X_CURSOR_SB_LEFT_ARROW      ),
   CURSOR(ELM_CURSOR_SB_RIGHT_ARROW     , ECORE_X_CURSOR_SB_RIGHT_ARROW     ),
   CURSOR(ELM_CURSOR_SB_UP_ARROW        , ECORE_X_CURSOR_SB_UP_ARROW        ),
   CURSOR(ELM_CURSOR_SB_V_DOUBLE_ARROW  , ECORE_X_CURSOR_SB_V_DOUBLE_ARROW  ),
   CURSOR(ELM_CURSOR_SHUTTLE            , ECORE_X_CURSOR_SHUTTLE            ),
   CURSOR(ELM_CURSOR_SIZING             , ECORE_X_CURSOR_SIZING             ),
   CURSOR(ELM_CURSOR_SPIDER             , ECORE_X_CURSOR_SPIDER             ),
   CURSOR(ELM_CURSOR_SPRAYCAN           , ECORE_X_CURSOR_SPRAYCAN           ),
   CURSOR(ELM_CURSOR_STAR               , ECORE_X_CURSOR_STAR               ),
   CURSOR(ELM_CURSOR_TARGET             , ECORE_X_CURSOR_TARGET             ),
   CURSOR(ELM_CURSOR_TCROSS             , ECORE_X_CURSOR_TCROSS             ),
   CURSOR(ELM_CURSOR_TOP_LEFT_ARROW     , ECORE_X_CURSOR_TOP_LEFT_ARROW     ),
   CURSOR(ELM_CURSOR_TOP_LEFT_CORNER    , ECORE_X_CURSOR_TOP_LEFT_CORNER    ),
   CURSOR(ELM_CURSOR_TOP_RIGHT_CORNER   , ECORE_X_CURSOR_TOP_RIGHT_CORNER   ),
   CURSOR(ELM_CURSOR_TOP_SIDE           , ECORE_X_CURSOR_TOP_SIDE           ),
   CURSOR(ELM_CURSOR_TOP_TEE            , ECORE_X_CURSOR_TOP_TEE            ),
   CURSOR(ELM_CURSOR_TREK               , ECORE_X_CURSOR_TREK               ),
   CURSOR(ELM_CURSOR_UL_ANGLE           , ECORE_X_CURSOR_UL_ANGLE           ),
   CURSOR(ELM_CURSOR_UMBRELLA           , ECORE_X_CURSOR_UMBRELLA           ),
   CURSOR(ELM_CURSOR_UR_ANGLE           , ECORE_X_CURSOR_UR_ANGLE           ),
   CURSOR(ELM_CURSOR_WATCH              , ECORE_X_CURSOR_WATCH              ),
   CURSOR(ELM_CURSOR_XTERM              , ECORE_X_CURSOR_XTERM              )
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
   Evas_Object *obj;
   Evas_Object *eventarea, *owner;
   const char *style, *cursor_name;
   int hot_x, hot_y;
   Ecore_Evas *ee;
   Evas *evas;
#ifdef HAVE_ELEMENTARY_X
   struct {
     Ecore_X_Cursor cursor;
     Ecore_X_Window win;
   } x;
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   struct {
     Ecore_Wl_Window *win;
   } wl;
#endif

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
     }
}

static Eina_Bool
_elm_cursor_obj_add(Evas_Object *obj, Elm_Cursor *cur)
{
   int x, y;

   cur->obj = edje_object_add(cur->evas);

   if (!cur->obj)
     return EINA_FALSE;

   if (!_elm_theme_object_set(obj, cur->obj, "cursor", cur->cursor_name,
                             cur->style ? cur->style : "default"))
     {
        ELM_SAFE_FREE(cur->obj, evas_object_del);
        return EINA_FALSE;
     }

   evas_object_event_callback_add(cur->obj, EVAS_CALLBACK_DEL,
                                  _elm_cursor_obj_del, cur);

   edje_object_size_min_get(cur->obj, &x, &y);
   evas_object_resize(cur->obj, x, y);
   return EINA_TRUE;
}

static void
_elm_cursor_set_hot_spots(Elm_Cursor *cur)
{
   const char *str;

   str = edje_object_data_get(cur->obj, "hot_x");
   if (str) cur->hot_x = atoi(str);
   else cur->hot_x = 0;

   str = edje_object_data_get(cur->obj, "hot_y");
   if (str) cur->hot_y = atoi(str);
   else cur->hot_y = 0;
}

static void
_elm_cursor_set(Elm_Cursor *cur)
{
   if (cur->visible) return;

   evas_event_freeze(cur->evas);
   cur->visible = EINA_TRUE;
   if (!cur->use_engine)
     {
        if (!cur->obj)
          _elm_cursor_obj_add(cur->eventarea, cur);
        ecore_evas_object_cursor_set(cur->ee, cur->obj,
                                     ELM_OBJECT_LAYER_CURSOR, cur->hot_x,
                                     cur->hot_y);
     }
   else
     {
        if (cur->obj)
          {
             evas_object_del(cur->obj);
             cur->obj = NULL;
          }
        ecore_evas_object_cursor_set(cur->ee, NULL,
                                     ELM_OBJECT_LAYER_CURSOR, cur->hot_x,
                                     cur->hot_y);
#ifdef HAVE_ELEMENTARY_X
        if (cur->x.win)
          ecore_x_window_cursor_set(cur->x.win, cur->x.cursor);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
        if (cur->wl.win)
          ecore_wl_window_cursor_from_name_set(cur->wl.win, cur->cursor_name);
#endif
     }
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
     ecore_evas_object_cursor_set(cur->ee, NULL, ELM_OBJECT_LAYER_CURSOR,
                                  cur->hot_x, cur->hot_y);
   else
     {
#ifdef HAVE_ELEMENTARY_X
        if (cur->x.win)
          ecore_x_window_cursor_set(cur->x.win, ECORE_X_CURSOR_X);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
        if (cur->wl.win)
          ecore_wl_window_cursor_default_restore(cur->wl.win);
#endif
     }
   evas_event_thaw(cur->evas);
}

static void
_elm_cursor_del(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_object_cursor_unset(obj);
}

#ifdef HAVE_ELEMENTARY_X
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
   else if (_elm_cursor_obj_add(cur->eventarea, cur))
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

   if (cur->use_engine)
     {
        Evas_Object *top;

        top = elm_widget_top_get(cur->owner);
        if ((top) && (eo_isa(top, ELM_OBJ_WIN_CLASS)))
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
#ifdef HAVE_ELEMENTARY_WAYLAND
             cur->wl.win = elm_win_wl_window_get(top);
#endif
          }
     }
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
 * @ingroup Cursors
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

EAPI void
elm_object_cursor_set(Evas_Object *obj, const char *cursor)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_object_sub_cursor_set(obj, obj, cursor);
}

EAPI const char *
elm_object_cursor_get(const Evas_Object *obj)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj, NULL);
   return cur->cursor_name;
}

EAPI void
elm_object_cursor_unset(Evas_Object *obj)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj);

   eina_stringshare_del(cur->cursor_name);
   eina_stringshare_del(cur->style);

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

EAPI void
elm_object_cursor_style_set(Evas_Object *obj, const char *style)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj);

   if (!eina_stringshare_replace(&cur->style, style))
     ERR("Could not set current style=%s", style);

   if (cur->use_engine) return;

   if (!cur->obj)
     {
        if (!_elm_cursor_obj_add(obj, cur))
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

EAPI const char *
elm_object_cursor_style_get(const Evas_Object *obj)
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
   if (!_elm_theme_object_set(cur->eventarea, cur->obj, "cursor",
                              cur->cursor_name, cur->style))
     ERR("Could not apply the theme to the cursor style=%s", cur->style);
   else
     _elm_cursor_set_hot_spots(cur);
}

EAPI void
elm_object_cursor_theme_search_enabled_set(Evas_Object *obj, Eina_Bool theme_search)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj);
   cur->theme_search = theme_search;
   ELM_SAFE_FREE(cur->obj, evas_object_del);
   _elm_cursor_cur_set(cur);
}

EAPI Eina_Bool
elm_object_cursor_theme_search_enabled_get(const Evas_Object *obj)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj, EINA_FALSE);
   return cur->theme_search;
}
