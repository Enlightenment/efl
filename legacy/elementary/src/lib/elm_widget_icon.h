#ifndef ELM_WIDGET_ICON_H
#define ELM_WIDGET_ICON_H

#include "elm_widget_image.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-icon-class The Elementary Icon Class
 *
 * This class defines a common interface for @b icon objects having
 * an icon as their basic graphics.
 */

 /**
  * @def ELM_ICON_CLASS
  *
  * Use this macro to cast whichever subclass of
  * #Elm_Icon_Smart_Class into it, so to access its fields.
  *
  * @ingroup Widget
  */
 #define ELM_ICON_CLASS(x) ((Elm_Icon_Smart_Class *) x)

/**
 * @def ELM_ICON_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Icon_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_ICON_DATA(x) ((Elm_Icon_Smart_Data *) x)

/**
 * @def ELM_ICON_SMART_CLASS_VERSION
 *
 * Current version for Elementary icon @b base smart class, a value
 * which goes to _Elm_Icon_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_ICON_SMART_CLASS_VERSION 1

/**
 * @def ELM_ICON_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Icon_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_ICON_SMART_CLASS_INIT_NULL
 * @see ELM_ICON_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_ICON_SMART_CLASS_INIT(smart_class_init)                        \
    {smart_class_init, ELM_ICON_SMART_CLASS_VERSION}

/**
 * @def ELM_ICON_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Icon_Smart_Class structure.
 *
 * @see ELM_ICON_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_ICON_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_ICON_SMART_CLASS_INIT_NULL \
  ELM_ICON_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_ICON_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Icon_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_ICON_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Icon_Smart_Class (base field)
 * to the latest #ELM_ICON_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_ICON_SMART_CLASS_INIT_NULL
 * @see ELM_ICON_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_ICON_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_ICON_SMART_CLASS_INIT(ELM_IMAGE_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary icon base smart class. This inherits directly from
 * #Elm_Image_Smart_Class and is meant to build widgets relying on an
 * icon as the building block of its visuals.
 */
typedef struct _Elm_Icon_Smart_Class
{
   Elm_Image_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Icon_Smart_Class;

typedef struct _Elm_Icon_Smart_Data Elm_Icon_Smart_Data;
struct _Elm_Icon_Smart_Data
{
   Elm_Image_Smart_Data  base;

   const char           *stdicon;
   Elm_Icon_Lookup_Order lookup_order;

#ifdef HAVE_ELEMENTARY_ETHUMB
   struct
   {
      struct
      {
         const char *path;
         const char *key;
      } file, thumb;

      Ecore_Event_Handler *eeh;

      Ethumb_Thumb_Format  format;

      Ethumb_Client_Async *request;

      Eina_Bool            retry : 1;
   } thumb;
#endif

#ifdef ELM_EFREET
   struct
   {
      int       requested_size;
      Eina_Bool use : 1;
   } freedesktop;
#endif

   int        in_eval;

   /* WARNING: to be deprecated */
   Eina_List *edje_signals;

   Eina_Bool  is_video : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_ICON_SMART_NAME[];
EAPI const Elm_Icon_Smart_Class *elm_icon_smart_class_get(void);

#define ELM_ICON_DATA_GET(o, sd) \
  Elm_Icon_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_ICON_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_ICON_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_ICON_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ICON_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_ICON_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_ICON_SMART_NAME, __func__)) \
    return

#endif
