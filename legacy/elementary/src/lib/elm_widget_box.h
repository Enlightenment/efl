#ifndef ELM_WIDGET_BOX_H
#define ELM_WIDGET_BOX_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-box-class The Elementary Box Class
 *
 * Elementary, besides having the @ref Box widget, exposes its
 * foundation -- the Elementary Box Class -- in order to create
 * other widgets which are a box with some more logic on top.
 */

/**
 * Base widget smart data extended with box instance data.
 */
typedef struct _Elm_Box_Smart_Data        Elm_Box_Smart_Data;
struct _Elm_Box_Smart_Data
{
   Eina_Bool             homogeneous : 1;
   Eina_Bool             delete_me : 1;
   Eina_Bool             horizontal : 1;
   Eina_Bool             recalc : 1;
};

struct _Elm_Box_Transition
{
   double          initial_time;
   double          duration;
   Ecore_Animator *animator;

   struct
   {
      Evas_Object_Box_Layout layout;
      void                  *data;
      void                   (*free_data)(void *data);
   } start, end;

   void            (*transition_end_cb)(void *data);
   void           *transition_end_data;
   void            (*transition_end_free_data)(void *data);
   Eina_List      *objs;
   Evas_Object    *box;

   Eina_Bool       animation_ended : 1;
   Eina_Bool       recalculate : 1;
};

typedef struct _Transition_Animation_Data Transition_Animation_Data;
struct _Transition_Animation_Data
{
   Evas_Object *obj;
   struct
   {
      Evas_Coord x, y, w, h;
   } start, end;
};

/**
 * @}
 */

#define ELM_BOX_DATA_GET(o, sd) \
  Elm_Box_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_BOX_CLASS)

#define ELM_BOX_DATA_GET_OR_RETURN(o, ptr)           \
  ELM_BOX_DATA_GET(o, ptr);                          \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_BOX_DATA_GET_OR_RETURN_VAL(o, ptr, val)  \
  ELM_BOX_DATA_GET(o, ptr);                          \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define ELM_BOX_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_BOX_CLASS))) \
    return

#endif
