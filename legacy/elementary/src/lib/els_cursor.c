#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#ifdef HAVE_ELEMENTARY_X
#include <Ecore_X.h>
#include <Ecore_X_Cursor.h>
#endif

/**
 * @defgroup Cursors Cursors
 *
 * The Cursor is an internal smart object used to customize the
 * cursor displayed over objects (or widgets).
 * It can use default X cursors (if using X), or cursors from a
 * theme.
 */

static const char _cursor_key[] = "_elm_cursor";

struct _Cursor_Id {
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
            CRITICAL("Null pointer: " #obj);            \
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
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Cursor cursor;
   Ecore_X_Window win;
#endif
   Eina_Bool visible:1;
};

static void
_cursor_mouse_in(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Cursor *cur = data;
#ifdef HAVE_ELEMENTARY_X
   ecore_x_window_cursor_set(cur->win, cur->cursor);
   cur->visible = EINA_TRUE;
#endif
}

static void
_cursor_mouse_out(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Cursor *cur = data;
#ifdef HAVE_ELEMENTARY_X
   cur->visible = EINA_FALSE;
   ecore_x_window_cursor_set(cur->win, ECORE_X_CURSOR_X);
#endif
}

static void
_cursor_del(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_object_cursor_unset(obj);
}

static int
_cur_strcmp(const void *data1, const void *data2)
{
   const struct _Cursor_Id *c1 = data1;
   const struct _Cursor_Id *c2 = data2;
   return strcmp (c1->name, c2->name);
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
 * @param obj the object being set a cursor.
 * @param cursor the cursor name to be used.
 *
 * @ingroup Cursors
 */
EAPI void
elm_object_cursor_set(Evas_Object *obj, const char *cursor)
{
   Elm_Cursor *cur = NULL;
   struct _Cursor_Id cur_search, *cur_id;

   EINA_SAFETY_ON_NULL_RETURN(obj);

   cur = evas_object_data_get(obj, _cursor_key);
   if (cur)
     elm_object_cursor_unset(obj);

   if (!cursor) return;

   cur_search.name = cursor;
   cur_id = bsearch(&cursor, _cursors, _cursors_count,
	 sizeof(struct _Cursor_Id), _cur_strcmp);

   if (!cur_id) {
     INF("Cursor couldn't be found");
     return;
   }

   cur = ELM_NEW(Elm_Cursor);
   if (!cur) return;

#ifdef HAVE_ELEMENTARY_X
   cur->win = elm_win_xwindow_get(obj);
   cur->cursor = ecore_x_cursor_shape_get(cur_id->id);
#endif

   cur->visible = EINA_FALSE;
   evas_object_data_set(obj, _cursor_key, cur);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_IN,
				  _cursor_mouse_in, cur);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_OUT,
				  _cursor_mouse_out, cur);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
				  _cursor_del, cur);
}

/**
 * Unset cursor for object
 *
 * Unset cursor for object, and set the cursor to default if the mouse
 * was over this object.
 *
 * @param obj Target object
 * @see elm_object_cursor_set()
 *
 * @ingroup Cursors
 */
EAPI void
elm_object_cursor_unset(Evas_Object *obj)
{
   ELM_CURSOR_GET_OR_RETURN(cur, obj);
#ifdef HAVE_ELEMENTARY_X
   if (cur->visible)
     ecore_x_window_cursor_set(cur->win, ECORE_X_CURSOR_X);
#endif
   evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_IN,
				  _cursor_mouse_in);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_OUT,
				  _cursor_mouse_out);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _cursor_mouse_out);
   evas_object_data_del(obj, _cursor_key);
   free(cur);
}
