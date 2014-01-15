#ifndef ELM_WIDGET_THUMB_H
#define ELM_WIDGET_THUMB_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-thumb-class The Elementary Thumb Class
 *
 * Elementary, besides having the @ref Thumb widget, exposes its
 * foundation -- the Elementary Thumb Class -- in order to create
 * other widgets which are a thumb with some more logic on top.
 */

/**
 * Base widget smart data extended with thumb instance data.
 */
typedef struct _Elm_Thumb_Smart_Data Elm_Thumb_Smart_Data;
struct _Elm_Thumb_Smart_Data
{
   Evas_Object          *obj; // the object itself
   Evas_Object          *view;  /* actual thumbnail, to be swallowed
                                 * at the thumb frame */

   /* original object's file/key pair */
   const char           *file;
   const char           *key;

   struct
   {
      /* object's thumbnail file/key pair */
      const char          *file;
      const char          *key;
      const char          *thumb_path;
      const char          *thumb_key;
      Ethumb_Client_Async *request;

      double                cropx;
      double                cropy;
      int                  compress;
      int                  quality;
      int                  tw;
      int                  th;

      Ethumb_Thumb_Aspect  aspect;
      Ethumb_Thumb_FDO_Size size;
      Ethumb_Thumb_Format  format;
      Ethumb_Thumb_Orientation orient;

      Eina_Bool            retry : 1;
   } thumb;

   Ecore_Event_Handler        *eeh;
   Elm_Thumb_Animation_Setting anim_setting;

   Eina_Bool                   edit : 1;
   Eina_Bool                   on_hold : 1;
   Eina_Bool                   is_video : 1;
   Eina_Bool                   was_video : 1;
};

/**
 * @}
 */

#define ELM_THUMB_DATA_GET(o, sd) \
  Elm_Thumb_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_THUMB_CLASS)

#define ELM_THUMB_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_THUMB_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_THUMB_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_THUMB_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_THUMB_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_THUMB_CLASS))) \
    return

#endif
