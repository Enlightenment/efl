#ifndef ELM_WIDGET_WEB_H
#define ELM_WIDGET_WEB_H

#ifdef HAVE_ELEMENTARY_WEB
#include <EWebKit.h>
#endif

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-web-class The Elementary Web Class
 *
 * Elementary, besides having the @ref Web widget, exposes its
 * foundation -- the Elementary Web Class -- in order to create other
 * widgets which are a web view with some more logic on top.
 */

/**
 * @def ELM_WEB_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Web_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_WEB_CLASS(x) ((Elm_Web_Smart_Class *) x)

/**
 * @def ELM_WEB_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Web_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_WEB_DATA(x) ((Elm_Web_Smart_Data *) x)

/**
 * @def ELM_WEB_SMART_CLASS_VERSION
 *
 * Current version for Elementary web @b base smart class, a value
 * which goes to _Elm_Web_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_WEB_SMART_CLASS_VERSION 1

/**
 * @def ELM_WEB_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Web_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_WEB_SMART_CLASS_INIT_NULL
 * @see ELM_WEB_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_WEB_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_WEB_SMART_CLASS_VERSION}

/**
 * @def ELM_WEB_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Web_Smart_Class structure.
 *
 * @see ELM_WEB_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_WEB_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_WEB_SMART_CLASS_INIT_NULL \
  ELM_WEB_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_WEB_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Web_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_WEB_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Web_Smart_Class (base field)
 * to the latest #ELM_WEB_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_WEB_SMART_CLASS_INIT_NULL
 * @see ELM_WEB_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_WEB_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_WEB_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary web base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a web.
 *
 * All of the functions listed on @ref Web namespace will work for
 * objects deriving from #Elm_Web_Smart_Class.
 */
typedef struct _Elm_Web_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Web_Smart_Class;

/**
 * Base widget smart data extended with web instance data.
 */
typedef struct _Elm_Web_Smart_Data Elm_Web_Smart_Data;
struct _Elm_Web_Smart_Data
{
   Elm_Widget_Smart_Data base;    /* base widget smart data as
                                   * first member obligatory, as
                                   * we're inheriting from it */

#ifdef HAVE_ELEMENTARY_WEB
   struct
   {
      Elm_Web_Window_Open          window_create;
      void                        *window_create_data;
      Elm_Web_Dialog_Alert         alert;
      void                        *alert_data;
      Elm_Web_Dialog_Confirm       confirm;
      void                        *confirm_data;
      Elm_Web_Dialog_Prompt        prompt;
      void                        *prompt_data;
      Elm_Web_Dialog_File_Selector file_selector;
      void                        *file_selector_data;
      Elm_Web_Console_Message      console_message;
      void                        *console_message_data;
   } hook;

   Elm_Win_Keyboard_Mode input_method;

   struct
   {
      Elm_Web_Zoom_Mode mode;
      float             current;
      float             min, max;
      Eina_Bool         no_anim;
      Ecore_Timer      *timer;
   } zoom;

   struct
   {
      struct
      {
         int x, y;
      } start, end;
      Ecore_Animator *animator;
   } bring_in;

   Eina_Bool tab_propagate : 1;
   Eina_Bool inwin_mode : 1;
#endif
};

enum Dialog_Type
{
   DIALOG_ALERT,
   DIALOG_CONFIRM,
   DIALOG_PROMPT,
   DIALOG_FILE_SELECTOR
};

typedef struct _Dialog_Data Dialog_Data;
struct _Dialog_Data
{
   enum Dialog_Type type;

   Evas_Object     *dialog;
   Evas_Object     *box;
   Evas_Object     *bt_ok, *bt_cancel;
   Evas_Object     *entry;
   Evas_Object     *file_sel;

   Eina_Bool       *response;
   const char     **entry_value;
   Eina_List      **selected_files;
};

struct _Elm_Web_Callback_Proxy_Context
{
   const char  *name;
   Evas_Object *obj;
};
typedef struct _Elm_Web_Callback_Proxy_Context Elm_Web_Callback_Proxy_Context;

#ifdef HAVE_ELEMENTARY_WEB

typedef struct _View_Smart_Data View_Smart_Data;
struct _View_Smart_Data
{
   Ewk_View_Smart_Data base;
   struct
   {
      Evas_Event_Mouse_Down event;
      Evas_Coord            x, y;
      unsigned int          move_count;
      Ecore_Timer          *longpress_timer;
      Ecore_Animator       *pan_anim;
   } mouse;
};

#endif

/**
 * @}
 */

EAPI extern const char ELM_WEB_SMART_NAME[];
EAPI const Elm_Web_Smart_Class *elm_web_smart_class_get(void);

#define ELM_WEB_DATA_GET(o, sd) \
  Elm_Web_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_WEB_DATA_GET_OR_RETURN(o, ptr)           \
  ELM_WEB_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_WEB_DATA_GET_OR_RETURN_VAL(o, ptr, val)  \
  ELM_WEB_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_WEB_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_WEB_SMART_NAME, __func__)) \
    return

#endif
