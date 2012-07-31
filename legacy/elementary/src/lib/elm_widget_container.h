#ifndef ELM_WIDGET_CONTAINER_H
#define ELM_WIDGET_CONTAINER_H

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-container-class The Elementary Container Class
 *
 * @image html container_inheritance_tree.png
 * @image latex container_inheritance_tree.eps
 *
 * This class defines a common interface for objects acting like
 * containers, i.e. objects parenting others and displaying their
 * childs "inside" of them somehow.
 *
 * The container must define "parts" (or spots) into which child
 * objects will be placed, inside of it. This is a way of handling
 * more the one content object, by naming content locations
 * properly. This is the role of the @c name argument of the virtual
 * functions in the class.
 *
 * The following object functions are meant to be used with all
 * container objects and derived ones:
 *
 * - elm_object_part_content_set()
 * - elm_object_part_content_get()
 * - elm_object_part_content_unset()
 */

 /**
  * @def ELM_CONTAINER_CLASS
  *
  * Use this macro to cast whichever subclass of
  * #Elm_Container_Smart_Class into it, so to access its fields.
  *
  * @ingroup Widget
  */
 #define ELM_CONTAINER_CLASS(x) ((Elm_Container_Smart_Class *) x)

/**
 * @def ELM_CONTAINER_SMART_CLASS_VERSION
 *
 * Current version for Elementary container @b base smart class, a value
 * which goes to _Elm_Container_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_CONTAINER_SMART_CLASS_VERSION 1

/**
 * @def ELM_CONTAINER_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Container_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_CONTAINER_SMART_CLASS_INIT_NULL
 * @see ELM_CONTAINER_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_CONTAINER_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_CONTAINER_SMART_CLASS_VERSION, NULL, NULL, NULL}

/**
 * @def ELM_CONTAINER_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Container_Smart_Class structure.
 *
 * @see ELM_CONTAINER_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_CONTAINER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_CONTAINER_SMART_CLASS_INIT_NULL \
  ELM_CONTAINER_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_CONTAINER_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Container_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_CONTAINER_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Container_Smart_Class (base field)
 * to the latest #ELM_CONTAINER_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_CONTAINER_SMART_CLASS_INIT_NULL
 * @see ELM_CONTAINER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_CONTAINER_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_CONTAINER_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary container base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets exposing
 * "parts" to hold child elements at.
 */
typedef struct _Elm_Container_Smart_Class
{
   Elm_Widget_Smart_Class base; /**< Base Elementary widget class struct, since we're inheriting from it */

   int                    version; /**< Version of this smart class definition */
   Eina_Bool            (*content_set)(Evas_Object *obj,
                                       const char *part,
                                       Evas_Object *content); /* 'Virtual' function on setting content on the object, at the given @a part part */
   Evas_Object         *(*content_get)(const Evas_Object * obj,
                                       const char *part); /* 'Virtual' function on retrieving content from the object, at the given @a part part */
   Evas_Object         *(*content_unset)(Evas_Object * obj,
                                         const char *part); /* 'Virtual' function on unsetting content from the object, at the given @a part part. Meant to return the content's pointer. */
} Elm_Container_Smart_Class;

typedef struct _Elm_Container_Smart_Data Elm_Container_Smart_Data;
struct _Elm_Container_Smart_Data
{
   Elm_Widget_Smart_Data base;
};
/**
 * @}
 */

EAPI const Elm_Container_Smart_Class *elm_container_smart_class_get(void);

#endif
