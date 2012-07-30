#ifndef ELM_WIDGET_BOX_H
#define ELM_WIDGET_BOX_H

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
 * @def ELM_BOX_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Box_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_BOX_CLASS(x) ((Elm_Box_Smart_Class *) x)

/**
 * @def ELM_BOX_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Box_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_BOX_DATA(x) ((Elm_Box_Smart_Data *) x)

/**
 * @def ELM_BOX_SMART_CLASS_VERSION
 *
 * Current version for Elementary box @b base smart class, a value
 * which goes to _Elm_Box_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_BOX_SMART_CLASS_VERSION 1

/**
 * @def ELM_BOX_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Box_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_BOX_SMART_CLASS_INIT_NULL
 * @see ELM_BOX_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_BOX_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_BOX_SMART_CLASS_VERSION}

/**
 * @def ELM_BOX_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Box_Smart_Class structure.
 *
 * @see ELM_BOX_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_BOX_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_BOX_SMART_CLASS_INIT_NULL \
  ELM_BOX_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_BOX_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Box_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_BOX_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Box_Smart_Class (base field)
 * to the latest #ELM_BOX_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_BOX_SMART_CLASS_INIT_NULL
 * @see ELM_BOX_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_BOX_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_BOX_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary box base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a box.
 *
 * All of the functions listed on @ref Box namespace will work for
 * objects deriving from #Elm_Box_Smart_Class.
 */
typedef struct _Elm_Box_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Box_Smart_Class;

/**
 * Base widget smart data extended with box instance data.
 */
typedef struct _Elm_Box_Smart_Data        Elm_Box_Smart_Data;
struct _Elm_Box_Smart_Data
{
   Elm_Widget_Smart_Data base; /* base widget smart data as
                                * first member obligatory, as
                                * we're inheriting from it */

   Eina_Bool             horizontal : 1;
   Eina_Bool             homogeneous : 1;
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

EAPI extern const char ELM_BOX_SMART_NAME[];
EAPI const Elm_Box_Smart_Class *elm_box_smart_class_get(void);

#define ELM_BOX_DATA_GET(o, sd) \
  Elm_Box_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_BOX_DATA_GET_OR_RETURN(o, ptr)           \
  ELM_BOX_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_BOX_DATA_GET_OR_RETURN_VAL(o, ptr, val)  \
  ELM_BOX_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_BOX_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_BOX_SMART_NAME, __func__)) \
    return

#endif
