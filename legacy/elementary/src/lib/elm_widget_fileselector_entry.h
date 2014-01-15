#ifndef ELM_WIDGET_FILESELECTOR_ENTRY_H
#define ELM_WIDGET_FILESELECTOR_ENTRY_H

#include "elm_widget_layout.h"

#include <Eio.h>

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-fileselector-entry-class The Elementary Fileselector Entry Class
 *
 * Elementary, besides having the @ref Fileselector_Entry widget,
 * exposes its foundation -- the Elementary Fileselector Entry Class
 * -- in order to create other widgets which are a fileselector_entry
 * with some more logic on top.
 */

/**
 * Base entry smart data extended with fileselector_entry instance data.
 */
typedef struct _Elm_Fileselector_Entry_Smart_Data \
Elm_Fileselector_Entry_Smart_Data;
struct _Elm_Fileselector_Entry_Smart_Data
{
   Evas_Object *button;
   Evas_Object *entry;
   char *path;
};

/**
 * @}
 */


#define ELM_FILESELECTOR_ENTRY_DATA_GET(o, sd) \
  Elm_Fileselector_Entry_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_FILESELECTOR_ENTRY_CLASS)

#define ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN(o, ptr) \
  ELM_FILESELECTOR_ENTRY_DATA_GET(o, ptr);                \
  if (EINA_UNLIKELY(!ptr))                                \
    {                                                     \
       CRI("No widget data for object %p (%s)",           \
           o, evas_object_type_get(o));                   \
       return;                                            \
    }

#define ELM_FILESELECTOR_ENTRY_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_FILESELECTOR_ENTRY_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                         \
    {                                                              \
       CRI("No widget data for object %p (%s)",                    \
           o, evas_object_type_get(o));                            \
       return val;                                                 \
    }

#define ELM_FILESELECTOR_ENTRY_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_FILESELECTOR_ENTRY_CLASS))) \
    return

#endif
