#ifndef ELM_WIDGET_FLIP_H
#define ELM_WIDGET_FLIP_H

#include "elm_widget_container.h"

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

/**
 * @def ELM_FLIP_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Flip_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_FLIP_CLASS(x) ((Elm_Flip_Smart_Class *)x)

/**
 * @def ELM_FLIP_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Flip_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_FLIP_DATA(x)  ((Elm_Flip_Smart_Data *)x)

/**
 * @def ELM_FLIP_SMART_CLASS_VERSION
 *
 * Current version for Elementary flip @b base smart class, a value
 * which goes to _Elm_Flip_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_FLIP_SMART_CLASS_VERSION 1

/**
 * @def ELM_FLIP_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Flip_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_FLIP_SMART_CLASS_INIT_NULL
 * @see ELM_FLIP_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_FLIP_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_FLIP_SMART_CLASS_VERSION}

/**
 * @def ELM_FLIP_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Flip_Smart_Class structure.
 *
 * @see ELM_FLIP_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_FLIP_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_FLIP_SMART_CLASS_INIT_NULL \
  ELM_FLIP_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_FLIP_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Flip_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_FLIP_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Flip_Smart_Class (base field)
 * to the latest #ELM_FLIP_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_FLIP_SMART_CLASS_INIT_NULL
 * @see ELM_FLIP_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_FLIP_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_FLIP_SMART_CLASS_INIT(ELM_CONTAINER_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary flip base smart class. This inherits directly from
 * #Elm_Container_Smart_Class and is meant to build widgets extending the
 * behavior of a flip.
 *
 * All of the functions listed on @ref Flip namespace will work for
 * objects deriving from #Elm_Flip_Smart_Class.
 */
typedef struct _Elm_Flip_Smart_Class
{
   Elm_Container_Smart_Class base;

   int                       version; /**< Version of this smart class definition */
} Elm_Flip_Smart_Class;

typedef struct _Slice               Slice;

/**
 * Base widget smart data extended with flip instance data.
 */
typedef struct _Elm_Flip_Smart_Data Elm_Flip_Smart_Data;
struct _Elm_Flip_Smart_Data
{
   Elm_Widget_Smart_Data base;

   Evas_Object          *clip;
   Evas_Object          *event[4];
   struct
   {
      Evas_Object *content, *clip;
   } front, back;

   Ecore_Animator       *animator;
   double                start, len;
   Ecore_Job            *job;
   Evas_Coord            down_x, down_y, x, y, ox, oy, w, h;
   Elm_Flip_Interaction  intmode;
   Elm_Flip_Mode         mode;
   int                   dir;
   double                dir_hitsize[4];
   Eina_Bool             dir_enabled[4];
   int                   slices_w, slices_h;
   Slice               **slices, **slices2;

   Eina_Bool             state : 1;
   Eina_Bool             next_state : 1;
   Eina_Bool             down : 1;
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

EAPI extern const char ELM_FLIP_SMART_NAME[];
EAPI const Elm_Flip_Smart_Class *elm_flip_smart_class_get(void);

#define ELM_FLIP_DATA_GET(o, sd) \
  Elm_Flip_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_FLIP_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_FLIP_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_FLIP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_FLIP_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_FLIP_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_FLIP_SMART_NAME, __func__)) \
    return

#endif
