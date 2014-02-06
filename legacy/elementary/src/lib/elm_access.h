/**
 * @defgroup Access Access
 * @ingroup Elementary
 *
 * WARNING! this API is not finalized. It is unstable. - do not use it if
 * you want no breaks in future.
 *
 * TODO: description
 *
 */

#define ELM_OBJ_ACCESS_CLASS elm_obj_access_class_get()

const Eo_Class *elm_obj_access_class_get(void) EINA_CONST;

enum _Elm_Access_Info_Type
{
   ELM_ACCESS_INFO_FIRST = -1,
   ELM_ACCESS_INFO,         /* next read is info - this is
                             * normally label */
   ELM_ACCESS_TYPE,         /* when reading out widget or item
                             * this is read first */
   ELM_ACCESS_STATE,        /* if there is a state (eg checkbox)
                             * then read state out */
   ELM_ACCESS_CONTEXT_INFO, /* to give contextual information */
   ELM_ACCESS_INFO_LAST
};

/**
 * @since 1.8
 * @typedef Elm_Access_Info_Type
 */
typedef enum _Elm_Access_Info_Type Elm_Access_Info_Type;

/**
 * @enum _Elm_Access_Action_Type
 * Enum of supported access action types.
 */
enum _Elm_Access_Action_Type
{
   ELM_ACCESS_ACTION_FIRST = -1,

   ELM_ACCESS_ACTION_HIGHLIGHT, /* highlight an object */
   ELM_ACCESS_ACTION_UNHIGHLIGHT, /* unhighlight an object */
   ELM_ACCESS_ACTION_HIGHLIGHT_NEXT, /* set highlight to next object */
   ELM_ACCESS_ACTION_HIGHLIGHT_PREV, /* set highlight to previous object */
   ELM_ACCESS_ACTION_ACTIVATE, /* activate a highlight object */
   ELM_ACCESS_ACTION_SCROLL, /* scroll if one of highlight object parents
                              * is scrollable */
   ELM_ACCESS_ACTION_UP, /* change value up of highlight object */
   ELM_ACCESS_ACTION_DOWN, /* change value down of highlight object */
   ELM_ACCESS_ACTION_BACK, /* go back to a previous view
                              ex: pop naviframe item */
   ELM_ACCESS_ACTION_READ, /* highlight an object */

   ELM_ACCESS_ACTION_LAST
};

/**
 * @since 1.8
 * @typedef Elm_Access_Action_Type
 */
typedef enum _Elm_Access_Action_Type Elm_Access_Action_Type;

struct _Elm_Access_Action_Info
{
   Evas_Coord   x;
   Evas_Coord   y;
   unsigned int mouse_type; /* 0: mouse down
                               1: mouse move
                               2: mouse up   */

   Elm_Access_Action_Type action_type;
   Elm_Access_Action_Type action_by;
   Eina_Bool              highlight_cycle : 1;
};

/**
 * @since 1.8
 * @typedef Elm_Access_Action_Info
 */
typedef struct _Elm_Access_Action_Info Elm_Access_Action_Info;

enum _Elm_Highlight_Direction
{
   ELM_HIGHLIGHT_DIR_FIRST = -1,
   ELM_HIGHLIGHT_DIR_NEXT,
   ELM_HIGHLIGHT_DIR_PREVIOUS
};

/**
 * @since 1.8
 * @typedef Elm_Highlight_Direction
 */
typedef enum _Elm_Highlight_Direction Elm_Highlight_Direction;

/**
 * @since 1.8
 * @typedef Elm_Access_Action_Cb
 *
 * User callback to make access object do specific action
 *
 * @param data user data
 * @param action_info information to classify the action
 * Returns @c EINA_TRUE on success, @c EINA FALSE otherwise
 *
 */
typedef Eina_Bool (*Elm_Access_Action_Cb)(void *data, Evas_Object *obj, Elm_Access_Action_Info *action_info);

typedef char *(*Elm_Access_Info_Cb)(void *data, Evas_Object *obj);
typedef void (*Elm_Access_Activate_Cb)(void *data, Evas_Object *part_obj, Elm_Object_Item *item);


/**
 * @brief Register evas object as an accessible object.
 * @since 1.8
 *
 * @param obj The evas object to register as an accessible object.
 * @param parent The elementary object which is used for creating
 * accessible object.
 *
 * @ingroup Access
 */
EAPI Evas_Object *elm_access_object_register(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Unregister accessible object.
 * @since 1.8
 *
 * @param obj The Evas object to unregister accessible object.
 *
 * @ingroup Access
 */
EAPI void elm_access_object_unregister(Evas_Object *obj);

/**
 * @brief Get an accessible object of the evas object.
 * @since 1.8
 *
 * @param obj The evas object.
 * @return Accessible object of the evas object or NULL for any error
 *
 * @ingroup Access
 */
EAPI Evas_Object *elm_access_object_get(const Evas_Object *obj);

/**
 * @brief Set text to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 * @param text The text information that will be read
 *
 * @see elm_access_info_cb_set
 * @ingroup Access
 */
EAPI void elm_access_info_set(Evas_Object *obj, int type, const char *text);

/**
 * @brief Set text to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 *
 * @see elm_access_info_cb_set
 * @ingroup Access
 */
EAPI char *elm_access_info_get(const Evas_Object *obj, int type);

/**
 * @brief Set content callback to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 * @param func The function to be called when the content is read
 * @param data The data pointer to be passed to @p func
 *
 * The type would be one of ELM_ACCESS_TYPE, ELM_ACCESS_INFO,
 * ELM_ACCESS_STATE, ELM_ACCESS_CONTEXT_INFO.
 *
 * In the case of button widget, the content of ELM_ACCESS_TYPE would be
 * "button". The label of button such as "ok", "cancel" is for ELM_ACCESS_INFO.
 * If the button is disabled, content of ELM_ACCESS_STATE would be "disabled".
 * And if there is contextual information, use ELM_ACCESS_CONTEXT_INFO.
 *
 * @ingroup Access
 */
EAPI void elm_access_info_cb_set(Evas_Object *obj, int type, Elm_Access_Info_Cb func, const void *data);

/**
 * @brief Set activate callback to activate highlight object.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param func The function to be called when the activate gesture is detected
 * @param data The data pointer to be passed to @p func
 *
 * @ingroup Access
 */
EAPI void elm_access_activate_cb_set(Evas_Object *obj, Elm_Access_Activate_Cb func, void *data);

/**
 * @brief Read out text information directly.
 * @since 1.8
 *
 * @param text The text information that will be read
 *
 * This function will not free the @p text internally.
 *
 * @ingroup Access
 */
EAPI void elm_access_say(const char *text);

/**
 * @brief Give the highlight to the object directly.
 * @since 1.8
 *
 * @param obj The object that will have the highlight and its information be read.
 *
 * The object should be an elementary object or an access object.
 *
 * @see elm_access_object_get
 * @ingroup Access
 */
EAPI void elm_access_highlight_set(Evas_Object* obj);

/**
 * @brief Do the accessibility action base on given object.
 * @since 1.8
 *
 * @param obj The object that could be an any object. it would be useful to use a container widget.
 * @param type The type of accessibility action.
 * @param action_info The action information of action @p type to give more specific information.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * The return value would be useful, when the @p type is ELM_ACCESS_ACTION_HIGHLIGHT_NEXT
 * or ELM_ACCESS_ACTION_HIGHLIGHT_PREV. If there is no way to give a highlight,
 * @c EINA_FALSE will be returned.
 *
 * @ingroup Access
 */
EAPI Eina_Bool elm_access_action(Evas_Object *obj, const Elm_Access_Action_Type type, Elm_Access_Action_Info *action_info);

/**
 * @brief Set a callback function to a given accessibility action type
 * @since 1.8
 *
 * @param obj The object to attach a callback to
 * @param type The type of accessibility action.
 * @param cb The callback function to be called when the accessibility action is triggered.
 * @param data The data pointer to be passed to @p cb
 *
 * @ingroup Access
 */
EAPI void elm_access_action_cb_set(Evas_Object *obj, const Elm_Access_Action_Type type, const Elm_Access_Action_Cb cb, const void *data);

/**
 * @brief Set the next access object for highlight.
 * @since 1.8
 *
 * @param obj  The object is previous access object of next for hilight.
 * @param dir  Access direction same as Focus direction
 * @param next The object is next access object of obj for hilight.
 *
 * Currently focus chain is used for access highlight chain. Use this API to
 * customize highlight chain. If highlight chain is already established, you can
 * change one object's highlight chain and do not break the other object's
 * highlight chain.
 *
 * @ingroup Access
 */
EAPI void
elm_access_highlight_next_set(Evas_Object *obj, Elm_Highlight_Direction dir, Evas_Object *next);
