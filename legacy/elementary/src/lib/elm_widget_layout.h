#ifndef ELM_WIDGET_LAYOUT_H
#define ELM_WIDGET_LAYOUT_H

#include "elm_widget_container.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-layout-class The Elementary Layout Class
 *
 * Elementary, besides having the @ref Layout widget, exposes its
 * foundation -- the Elementary Layout Class -- in order to create
 * other widgets which are, basically, a certain layout with some more
 * logic on top.
 *
 * The idea is to make the creation of that widgets as easy as possible,
 * factorizing code on this common base. For example, a button is a
 * layout (that looks like push button) that happens to react on
 * clicks and keyboard events in a special manner, calling its user
 * back on those events. That's no surprise, then, that the @ref
 * Button implementation relies on #Elm_Layout_Smart_Class, if you go
 * to check it.
 *
 * The Layout class inherits from
 * #Elm_Container_Smart_Class. Container parts, here, map directly to
 * Edje parts from the layout's Edje group. Besides that, there's a whole
 * infrastructure around Edje files:
 * - interfacing by signals,
 * - setting/retrieving text part values,
 * - dealing with table and box parts directly,
 * - etc.
 *
 * Take a look at #Elm_Layout_Smart_Class's 'virtual' functions to
 * understand the whole interface. Finally, layout objects will do
 * <b>part aliasing</b> for you, if you set it up properly. For that,
 * take a look at #Elm_Layout_Part_Alias_Description, where it's
 * explained in detail.
 */

/**
 * @def ELM_LAYOUT_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Layout_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_LAYOUT_CLASS(x) ((Elm_Layout_Smart_Class *) x)

/**
 * @def ELM_LAYOUT_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Layout_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_LAYOUT_DATA(x) ((Elm_Layout_Smart_Data *) x)

/**
 * @def ELM_LAYOUT_SMART_CLASS_VERSION
 *
 * Current version for Elementary layout @b base smart class, a value
 * which goes to _Elm_Layout_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_LAYOUT_SMART_CLASS_VERSION 1

/**
 * @def ELM_LAYOUT_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Layout_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_LAYOUT_SMART_CLASS_INIT_NULL
 * @see ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_LAYOUT_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_LAYOUT_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, \
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   \
   NULL}

/**
 * @def ELM_LAYOUT_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Layout_Smart_Class structure.
 *
 * @see ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_LAYOUT_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_LAYOUT_SMART_CLASS_INIT_NULL \
  ELM_LAYOUT_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Layout_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_LAYOUT_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Layout_Smart_Class (base field)
 * to the latest #ELM_LAYOUT_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_LAYOUT_SMART_CLASS_INIT_NULL
 * @see ELM_LAYOUT_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_LAYOUT_SMART_CLASS_INIT                          \
    (ELM_CONTAINER_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * @typedef Elm_Layout_Part_Alias_Description
 *
 * A layout part aliasing (proxying) description, used to get part
 * names aliasing independently of a widgets theme.
 *
 * @ingroup Widget
 */
typedef struct _Elm_Layout_Part_Alias_Description Elm_Layout_Part_Alias_Description;

/**
 * @struct _Elm_Layout_Part_Alias_Description
 *
 * Elementary Layout-based widgets may declare part proxies, i.e., aliases
 * for real theme part names to expose to the API calls:
 * - elm_layout_text_set()
 * - elm_layout_text_get()
 * - elm_layout_content_set()
 * - elm_layout_content_get()
 * - elm_layout_content_unset()
 * and their equivalents. This list must be set on the
 * @c "_smart_set_user()" function of inheriting widgets, so that part
 * aliasing is handled automatically for them.
 *
 * @ingroup Widget
 */
struct _Elm_Layout_Part_Alias_Description
{
   const char *alias; /**< Alternate name for a given (real) part. Calls receiving this string as a part name will be translated to the string at _Elm_Layout_Part_Proxies_Description::real_part */

   const char *real_part; /**< Target part name for the alias set on @ref _Elm_Layout_Part_Proxies_Description::real_part. An example of usage would be @c "default" on that field, with @c "elm.content.swallow" on this one */
};

/**
 * Elementary layout base smart class. This inherits directly from
 * #Elm_Container_Smart_Class and is meant to build widgets relying on
 * an Edje layout as a building block of its visuals.
 *
 * For instance, the elm_layout @b widget itself is just a realization
 * of this smart class (see the code for elm_layout_add()). All of the
 * functions listed on @ref Layout namespace will work for objects
 * deriving from #Elm_Layout_Smart_Class.
 */
typedef struct _Elm_Layout_Smart_Class
{
   Elm_Container_Smart_Class base; /**< Elementary container widget class struct, since we're inheriting from it */

   int                    version; /**< Version of this smart class definition */

   void                   (*sizing_eval)(Evas_Object *obj); /* 'Virtual' function on evalutating the object's final geometry, accounting for its sub-objects */
   void                   (*signal)(Evas_Object *obj,
                                    const char *emission,
                                    const char *source); /* 'Virtual' function on emitting an (Edje) signal to the object, acting on its internal layout */
   void                   (*callback_add)(Evas_Object *obj,
                                          const char *emission,
                                          const char *source,
                                          Edje_Signal_Cb func,
                                          void *data); /* 'Virtual' function on adding an (Edje) signal callback to the object, proxyed from its internal layout */
   void                 * (*callback_del)(Evas_Object * obj,
                                          const char *emission,
                                          const char *source,
                                          Edje_Signal_Cb func); /* 'Virtual' function on deleting an (Edje) signal callback on the object, proxyed from its internal layout */
   Eina_Bool              (*text_set)(Evas_Object *obj,
                                      const char *part,
                                      const char *text); /* 'Virtual' function on setting text on an (Edje) part of the object, from its internal layout */
   const char            *(*text_get)(const Evas_Object * obj,
                                      const char *part); /* 'Virtual' function on fetching text from an (Edje) part of the object, on its internal layout */

   Eina_Bool              (*box_append)(Evas_Object *obj,
                                        const char *part,
                                        Evas_Object *child); /* 'Virtual' function on appending an object to an (Edje) box part of the object, from its internal layout */
   Eina_Bool              (*box_prepend)(Evas_Object *obj,
                                         const char *part,
                                         Evas_Object *child); /* 'Virtual' function on prepending an object to an (Edje) box part of the object, from its internal layout */
   Eina_Bool              (*box_insert_before)(Evas_Object *obj,
                                               const char *part,
                                               Evas_Object *child,
                                               const Evas_Object *reference); /* 'Virtual' function on inserting an object to an (Edje) box part of the object, from its internal layout. The new child's position in the box is be prior to the one of a relative child already in the box */
   Eina_Bool              (*box_insert_at)(Evas_Object *obj,
                                           const char *part,
                                           Evas_Object *child,
                                           unsigned int pos); /* 'Virtual' function on inserting an object to an (Edje) box part of the object, from its internal layout. The new child's position number is passed explicitly */
   Evas_Object           *(*box_remove)(Evas_Object * obj,
                                        const char *part,
                                        Evas_Object * child); /* 'Virtual' function on removing an object from an (Edje) box part of the object, on its internal layout */
   Eina_Bool              (*box_remove_all)(Evas_Object *obj,
                                            const char *part,
                                            Eina_Bool clear); /* 'Virtual' function on removing @b all objects from an (Edje) box part of the object, on its internal layout */
   Eina_Bool              (*table_pack)(Evas_Object *obj,
                                        const char *part,
                                        Evas_Object *child,
                                        unsigned short col,
                                        unsigned short row,
                                        unsigned short colspan,
                                        unsigned short rowspan); /* 'Virtual' function on inserting an object to an (Edje) table part of the object, from its internal layout */
   Evas_Object           *(*table_unpack)(Evas_Object * obj,
                                          const char *part,
                                          Evas_Object * child); /* 'Virtual' function on removing an object from an (Edje) table part of the object, on its internal layout */
   Eina_Bool              (*table_clear)(Evas_Object *obj,
                                         const char *part,
                                         Eina_Bool clear); /* 'Virtual' function on removing @b all objects from an (Edje) table part of the object, on its internal layout */

   const Elm_Layout_Part_Alias_Description *content_aliases; /**< List of (@c 'SWALLOW') part aliases, <b>@c NULL terminated</b>. If @c NULL is passed as part name, it will be translated to the 1st _Elm_Layout_Part_Proxies_Description::real_part field in the list. */

   const Elm_Layout_Part_Alias_Description *text_aliases; /**< List of (@c 'TEXT' or 'TEXTBLOCK') part aliases, <b>@c NULL terminated</b>. If @c NULL is passed as part name, it will be translated to the 1st _Elm_Layout_Part_Proxies_Description::real_part field in the list. */
} Elm_Layout_Smart_Class;

/**
 * Base widget smart data extended with layout instance data.
 */
typedef struct _Elm_Layout_Smart_Data
{
   Elm_Widget_Smart_Data base; /**< Base widget smart data as first member obligatory */

   Eina_List            *subs; /**< List of Elm_Layout_Sub_Object_Data structs, to hold the actual sub objects */
   Eina_List            *edje_signals;
   Eina_List            *parts_cursors;
   const char           *klass, *group;

   Eina_Bool             needs_size_calc : 1;
} Elm_Layout_Smart_Data;

/**
 * @}
 */

EAPI extern const char ELM_LAYOUT_SMART_NAME[];
EAPI const Elm_Layout_Smart_Class *elm_layout_smart_class_get(void);

#define ELM_LAYOUT_DATA_GET(o, sd) \
  Elm_Layout_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_LAYOUT_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_LAYOUT_DATA_GET(o, ptr);                       \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_LAYOUT_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_LAYOUT_DATA_GET(o, ptr);                         \
  if (!ptr)                                            \
    {                                                  \
       CRITICAL("No widget data for object %p (%s)",   \
                o, evas_object_type_get(o));           \
       return val;                                     \
    }

#define ELM_LAYOUT_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_LAYOUT_SMART_NAME, __func__)) \
    return

#endif
