#ifndef ELM_WIDGET_WEB_H
#define ELM_WIDGET_WEB_H

#ifdef HAVE_ELEMENTARY_WEB
#ifdef USE_WEBKIT2
#include <EWebKit2.h>
#else
#include <EWebKit.h>
#endif
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
 * Base widget smart data extended with web instance data.
 */
typedef struct _Elm_Web_Smart_Data Elm_Web_Smart_Data;
struct _Elm_Web_Smart_Data
{
   Evas_Object *obj;
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

#define ELM_WEB_DATA_GET(o, sd) \
  Elm_Web_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_WEB_CLASS)

#define ELM_WEB_DATA_GET_OR_RETURN(o, ptr)           \
  ELM_WEB_DATA_GET(o, ptr);                          \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_WEB_DATA_GET_OR_RETURN_VAL(o, ptr, val)  \
  ELM_WEB_DATA_GET(o, ptr);                          \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define ELM_WEB_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_WEB_CLASS))) \
    return

#endif
