#ifndef ELM_WIDGET_THUMB_H
#define ELM_WIDGET_THUMB_H

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
 * @def ELM_THUMB_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Thumb_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_THUMB_CLASS(x) ((Elm_Thumb_Smart_Class *) x)

/**
 * @def ELM_THUMB_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Thumb_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_THUMB_DATA(x) ((Elm_Thumb_Smart_Data *) x)

/**
 * @def ELM_THUMB_SMART_CLASS_VERSION
 *
 * Current version for Elementary thumb @b base smart class, a value
 * which goes to _Elm_Thumb_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_THUMB_SMART_CLASS_VERSION 1

/**
 * @def ELM_THUMB_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Thumb_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_THUMB_SMART_CLASS_INIT_NULL
 * @see ELM_THUMB_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_THUMB_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_THUMB_SMART_CLASS_VERSION}

/**
 * @def ELM_THUMB_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Thumb_Smart_Class structure.
 *
 * @see ELM_THUMB_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_THUMB_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_THUMB_SMART_CLASS_INIT_NULL \
  ELM_THUMB_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_THUMB_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Thumb_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_THUMB_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Thumb_Smart_Class (base field)
 * to the latest #ELM_THUMB_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_THUMB_SMART_CLASS_INIT_NULL
 * @see ELM_THUMB_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_THUMB_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_THUMB_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary thumb base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a thumb.
 *
 * All of the functions listed on @ref Thumb namespace will work for
 * objects deriving from #Elm_Thumb_Smart_Class.
 */
typedef struct _Elm_Thumb_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Thumb_Smart_Class;

/**
 * Base widget smart data extended with thumb instance data.
 */
typedef struct _Elm_Thumb_Smart_Data Elm_Thumb_Smart_Data;
struct _Elm_Thumb_Smart_Data
{
   Elm_Widget_Smart_Data base;

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
#ifdef HAVE_ELEMENTARY_ETHUMB
      const char          *thumb_path;
      const char          *thumb_key;
      Ethumb_Client_Async *request;

      Ethumb_Thumb_Format  format;

      Eina_Bool            retry : 1;
#endif
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

EAPI extern const char ELM_THUMB_SMART_NAME[];
EAPI const Elm_Thumb_Smart_Class *elm_thumb_smart_class_get(void);

#define ELM_THUMB_DATA_GET(o, sd) \
  Elm_Thumb_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_THUMB_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_THUMB_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_THUMB_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_THUMB_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_THUMB_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_THUMB_SMART_NAME, __func__)) \
    return

#endif
