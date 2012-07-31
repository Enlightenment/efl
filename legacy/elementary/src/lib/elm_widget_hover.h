#ifndef ELM_WIDGET_HOVER_H
#define ELM_WIDGET_HOVER_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-hover-class The Elementary Hover Class
 *
 * Elementary, besides having the @ref Hover widget, exposes its
 * foundation -- the Elementary Hover Class -- in order to create other
 * widgets which are a hover with some more logic on top.
 */

/**
 * @def ELM_HOVER_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Hover_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_HOVER_CLASS(x) ((Elm_Hover_Smart_Class *)x)

/**
 * @def ELM_HOVER_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Hover_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_HOVER_DATA(x)  ((Elm_Hover_Smart_Data *)x)

/**
 * @def ELM_HOVER_SMART_CLASS_VERSION
 *
 * Current version for Elementary hover @b base smart class, a value
 * which goes to _Elm_Hover_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_HOVER_SMART_CLASS_VERSION 1

/**
 * @def ELM_HOVER_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Hover_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_HOVER_SMART_CLASS_INIT_NULL
 * @see ELM_HOVER_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_HOVER_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_HOVER_SMART_CLASS_VERSION}

/**
 * @def ELM_HOVER_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Hover_Smart_Class structure.
 *
 * @see ELM_HOVER_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_HOVER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_HOVER_SMART_CLASS_INIT_NULL \
  ELM_HOVER_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_HOVER_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Hover_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_HOVER_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Hover_Smart_Class (base field)
 * to the latest #ELM_HOVER_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_HOVER_SMART_CLASS_INIT_NULL
 * @see ELM_HOVER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_HOVER_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_HOVER_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary hover base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a hover.
 *
 * All of the functions listed on @ref Hover namespace will work for
 * objects deriving from #Elm_Hover_Smart_Class.
 */
typedef struct _Elm_Hover_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Hover_Smart_Class;

/**
 * Base widget smart data extended with hover instance data.
 */
typedef struct _Elm_Hover_Smart_Data Elm_Hover_Smart_Data;
typedef struct _Content_Info         Content_Info;

struct _Content_Info
{
   const char  *swallow;
   Evas_Object *obj;
};

/* WARNING: sync size with actual declaration, always */
extern const Elm_Layout_Part_Alias_Description _content_aliases[10];

struct _Elm_Hover_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Evas_Object          *offset, *size;
   Evas_Object          *parent, *target;

   Content_Info         *smt_sub;  /* 'smart placement' sub object */
   Content_Info          subs[sizeof(_content_aliases) /
                              sizeof(_content_aliases[0]) - 1];

   Eina_Bool             on_del : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_HOVER_SMART_NAME[];
EAPI const Elm_Hover_Smart_Class *elm_hover_smart_class_get(void);

#define ELM_HOVER_DATA_GET(o, sd) \
  Elm_Hover_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_HOVER_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_HOVER_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_HOVER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_HOVER_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_HOVER_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_HOVER_SMART_NAME, __func__)) \
    return

#endif
