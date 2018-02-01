#ifndef EFL_UI_WIDGET_FLIP_H
#define EFL_UI_WIDGET_FLIP_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-flip-class The Elementary Flip Class
 *
 * Elementary, besides having the @ref Flip widget, exposes its
 * foundation -- the Elementary Flip Class -- in order to create other
 * widgets which are a flip with some more logic on top.
 */

typedef struct _Slice               Slice;

/**
 * Base widget smart data extended with flip instance data.
 */
typedef struct _Efl_Ui_Flip_Data Efl_Ui_Flip_Data;
struct _Efl_Ui_Flip_Data
{
   Evas_Object          *obj;
   Evas_Object          *clip;
   Evas_Object          *event[4];
   struct
   {
      Evas_Object *content, *clip;
   } front, back;

   Eina_List            *content_list;

   Ecore_Animator       *animator;
   double                start, len;
   Ecore_Job            *job;
   Evas_Coord            down_x, down_y, x, y, ox, oy, w, h;
   Efl_Ui_Flip_Interaction  intmode;
   Elm_Flip_Mode         mode;
   int                   dir;
   double                dir_hitsize[4];
   Eina_Bool             dir_enabled[4];
   int                   slices_w, slices_h;
   Slice               **slices, **slices2;

   Eina_Bool             state : 1;
   Eina_Bool             next_state : 1;
   Eina_Bool             mouse_down : 1;
   Eina_Bool             finish : 1;
   Eina_Bool             started : 1;
   Eina_Bool             backflip : 1;
   Eina_Bool             pageflip : 1;
   Eina_Bool             manual : 1;
};

typedef struct _Vertex2             Vertex2;
typedef struct _Vertex3             Vertex3;

struct _Slice
{
   Evas_Object *obj;
   double       u[4], v[4], x[4], y[4], z[4];
};

struct _Vertex2
{
   double x, y;
};

struct _Vertex3
{
   double x, y, z;
};

/**
 * @}
 */

#define EFL_UI_FLIP_DATA_GET(o, sd) \
  Efl_Ui_Flip_Data * sd = efl_data_scope_get(o, EFL_UI_FLIP_CLASS)

#define EFL_UI_FLIP_DATA_GET_OR_RETURN(o, ptr)          \
  EFL_UI_FLIP_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_FLIP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_FLIP_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define EFL_UI_FLIP_CHECK(obj)                           \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_FLIP_CLASS))) \
    return

#endif
