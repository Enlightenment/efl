#ifndef ELM_WIDGET_MAPBUF_H
#define ELM_WIDGET_MAPBUF_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-mapbuf-class The Elementary Mapbuf Class
 *
 * Elementary, besides having the @ref Mapbuf widget, exposes its
 * foundation -- the Elementary Mapbuf Class -- in order to create other
 * widgets which are a mapbuf with some more logic on top.
 */

/**
 * Base widget smart data extended with mapbuf instance data.
 */
typedef struct _Elm_Mapbuf_Smart_Data Elm_Mapbuf_Smart_Data;
struct _Elm_Mapbuf_Smart_Data
{
   Evas_Object          *self;
   Evas_Object          *content;
   Ecore_Idler          *idler;
   Evas_Map             *map;
   struct
     {
        int r, g, b, a;
     } colors[4];

   Eina_Bool             enabled : 1;
   Eina_Bool             smooth_saved : 1;
   Eina_Bool             smooth : 1;
   Eina_Bool             alpha : 1;
   Eina_Bool             automode : 1;
};

/**
 * @}
 */

#define ELM_MAPBUF_DATA_GET(o, sd) \
  Elm_Mapbuf_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_MAPBUF_CLASS)

#define ELM_MAPBUF_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_MAPBUF_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_MAPBUF_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_MAPBUF_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       CRI("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_MAPBUF_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_MAPBUF_CLASS))) \
    return

#endif
