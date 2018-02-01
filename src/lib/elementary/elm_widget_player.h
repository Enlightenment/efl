#ifndef ELM_WIDGET_PLAYER_H
#define ELM_WIDGET_PLAYER_H

#include "Elementary.h"

#include "elm_player.eo.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-player-class The Elementary Player Class
 *
 * Elementary, besides having the @ref Player widget, exposes its
 * foundation -- the Elementary Player Class -- in order to create other
 * widgets which are a player with some more logic on top.
 */

/**
 * Base layout smart data extended with player instance data.
 */
typedef struct _Elm_Player_Data Elm_Player_Data;
struct _Elm_Player_Data
{
   Evas_Object          *video;
   Evas_Object          *emotion;

   /* tracking those to ease disabling/enabling them back */
   Evas_Object          *forward;
   Evas_Object          *info;
   Evas_Object          *next;
   Evas_Object          *pause;
   Evas_Object          *play;
   Evas_Object          *prev;
   Evas_Object          *rewind;
   Evas_Object          *stop;
   Evas_Object          *eject;
   Evas_Object          *volume;
   Evas_Object          *mute;
   Evas_Object          *slider;
   Evas_Object          *vslider;

   Eina_Bool             dragging : 1;
};

/**
 * @}
 */

#define ELM_PLAYER_DATA_GET(o, sd) \
  Elm_Player_Data * sd = efl_data_scope_get(o, ELM_PLAYER_CLASS)

#define ELM_PLAYER_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_PLAYER_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_PLAYER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PLAYER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       ERR("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_PLAYER_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_PLAYER_CLASS))) \
    return

#endif
