#ifndef ELM_WIDGET_GLVIEW_H
#define ELM_WIDGET_GLVIEW_H

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
 * @def ELM_GLVIEW_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Glview_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_GLVIEW_CLASS(x) ((Elm_Glview_Smart_Class *) x)

/**
 * @def ELM_GLVIEW_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Glview_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_GLVIEW_DATA(x) ((Elm_Glview_Smart_Data *) x)

/**
 * @def ELM_GLVIEW_SMART_CLASS_VERSION
 *
 * Current version for Elementary glview @b base smart class, a value
 * which goes to _Elm_Glview_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_GLVIEW_SMART_CLASS_VERSION 1

/**
 * @def ELM_GLVIEW_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Glview_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_GLVIEW_SMART_CLASS_INIT_NULL
 * @see ELM_GLVIEW_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_GLVIEW_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_GLVIEW_SMART_CLASS_VERSION}

/**
 * @def ELM_GLVIEW_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Glview_Smart_Class structure.
 *
 * @see ELM_GLVIEW_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_GLVIEW_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_GLVIEW_SMART_CLASS_INIT_NULL \
  ELM_GLVIEW_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_GLVIEW_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Glview_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_GLVIEW_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Glview_Smart_Class (base field)
 * to the latest #ELM_GLVIEW_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_GLVIEW_SMART_CLASS_INIT_NULL
 * @see ELM_GLVIEW_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_GLVIEW_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_GLVIEW_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary glview base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a glview.
 *
 * All of the functions listed on @ref Glview namespace will work for
 * objects deriving from #Elm_Glview_Smart_Class.
 */
typedef struct _Elm_Glview_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Glview_Smart_Class;

/**
 * Base widget smart data extended with glview instance data.
 */
typedef struct _Elm_Glview_Smart_Data Elm_Glview_Smart_Data;
struct _Elm_Glview_Smart_Data
{
   Elm_Widget_Smart_Data    base; /* base widget smart data as
                                   * first member obligatory, as
                                   * we're inheriting from it */

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

EAPI extern const char ELM_GLVIEW_SMART_NAME[];
EAPI const Elm_Glview_Smart_Class *elm_glview_smart_class_get(void);

#define ELM_GLVIEW_DATA_GET(o, sd) \
  Elm_Glview_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_GLVIEW_DATA_GET_OR_RETURN(o, ptr)           \
  ELM_GLVIEW_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_GLVIEW_DATA_GET_OR_RETURN_VAL(o, ptr, val)  \
  ELM_GLVIEW_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_GLVIEW_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_GLVIEW_SMART_NAME, __func__)) \
    return

#endif
