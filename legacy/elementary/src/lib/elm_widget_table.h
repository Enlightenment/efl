#ifndef ELM_WIDGET_TABLE_H
#define ELM_WIDGET_TABLE_H

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-table-class The Elementary Table Class
 *
 * Elementary, besides having the @ref Table widget, exposes its
 * foundation -- the Elementary Table Class -- in order to create
 * other widgets which are a table with some more logic on top.
 */

/**
 * @def ELM_TABLE_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Table_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_TABLE_CLASS(x) ((Elm_Table_Smart_Class *) x)

/**
 * @def ELM_TABLE_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Table_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_TABLE_DATA(x) ((Elm_Table_Smart_Data *) x)

/**
 * @def ELM_TABLE_SMART_CLASS_VERSION
 *
 * Current version for Elementary table @b base smart class, a value
 * which goes to _Elm_Table_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_TABLE_SMART_CLASS_VERSION 1

/**
 * @def ELM_TABLE_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Table_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_TABLE_SMART_CLASS_INIT_NULL
 * @see ELM_TABLE_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_TABLE_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_TABLE_SMART_CLASS_VERSION}

/**
 * @def ELM_TABLE_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Table_Smart_Class structure.
 *
 * @see ELM_TABLE_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_TABLE_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_TABLE_SMART_CLASS_INIT_NULL \
  ELM_TABLE_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_TABLE_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Table_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_TABLE_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Table_Smart_Class (base field)
 * to the latest #ELM_TABLE_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_TABLE_SMART_CLASS_INIT_NULL
 * @see ELM_TABLE_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_TABLE_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_TABLE_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary table base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a table.
 *
 * All of the functions listed on @ref Table namespace will work for
 * objects deriving from #Elm_Table_Smart_Class.
 */
typedef struct _Elm_Table_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Table_Smart_Class;

/**
 * Base widget smart data extended with table instance data.
 */
typedef struct _Elm_Table_Smart_Data        Elm_Table_Smart_Data;
struct _Elm_Table_Smart_Data
{
   Elm_Widget_Smart_Data base; /* base widget smart data as
                                * first member obligatory, as
                                * we're inheriting from it */
};

/**
 * @}
 */

EAPI extern const char ELM_TABLE_SMART_NAME[];
EAPI const Elm_Table_Smart_Class *elm_table_smart_class_get(void);

#define ELM_TABLE_DATA_GET(o, sd) \
  Elm_Table_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_TABLE_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_TABLE_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_TABLE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_TABLE_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_TABLE_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_TABLE_SMART_NAME, __func__)) \
    return

#endif
