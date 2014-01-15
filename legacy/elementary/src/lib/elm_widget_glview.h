#ifndef ELM_WIDGET_GLVIEW_H
#define ELM_WIDGET_GLVIEW_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-glview-class The Elementary Glview Class
 *
 * Elementary, besides having the @ref Glview widget, exposes its
 * foundation -- the Elementary Glview Class -- in order to create other
 * widgets which are a glview with some more logic on top.
 */

/**
 * Base widget smart data extended with glview instance data.
 */
typedef struct _Elm_Glview_Smart_Data Elm_Glview_Smart_Data;
struct _Elm_Glview_Smart_Data
{
   Elm_GLView_Mode          mode;
   Elm_GLView_Resize_Policy scale_policy;
   Elm_GLView_Render_Policy render_policy;

   Evas_GL                 *evasgl;
   Evas_GL_Config          *config;
   Evas_GL_Surface         *surface;
   Evas_GL_Context         *context;

   Evas_Coord               w, h;

   Elm_GLView_Func_Cb       init_func;
   Elm_GLView_Func_Cb       del_func;
   Elm_GLView_Func_Cb       resize_func;
   Elm_GLView_Func_Cb       render_func;

   Ecore_Idle_Enterer      *render_idle_enterer;

   Eina_Bool                initialized : 1;
   Eina_Bool                resized : 1;
};

/**
 * @}
 */

#define ELM_GLVIEW_DATA_GET(o, sd) \
  Elm_Glview_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_GLVIEW_CLASS)

#define ELM_GLVIEW_DATA_GET_OR_RETURN(o, ptr)   \
  ELM_GLVIEW_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                      \
    {                                           \
       CRI("No widget data for object %p (%s)", \
           o, evas_object_type_get(o));         \
       return;                                  \
    }

#define ELM_GLVIEW_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_GLVIEW_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       CRI("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_GLVIEW_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_GLVIEW_CLASS))) \
    return

#endif
