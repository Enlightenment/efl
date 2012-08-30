#ifndef ELM_WIDGET_ENTRY_H
#define ELM_WIDGET_ENTRY_H

#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-entry-class The Elementary Entry Class
 *
 * Elementary, besides having the @ref Entry widget, exposes its
 * foundation -- the Elementary Entry Class -- in order to create
 * other widgets which are a entry with some more logic on top.
 */

/**
 * @def ELM_ENTRY_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Entry_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_ENTRY_CLASS(x) ((Elm_Entry_Smart_Class *)x)

/**
 * @def ELM_ENTRY_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Entry_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_ENTRY_DATA(x)  ((Elm_Entry_Smart_Data *)x)

/**
 * @def ELM_ENTRY_SMART_CLASS_VERSION
 *
 * Current version for Elementary entry @b base smart class, a value
 * which goes to _Elm_Entry_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_ENTRY_SMART_CLASS_VERSION 1

/**
 * @def ELM_ENTRY_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Entry_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_ENTRY_SMART_CLASS_INIT_NULL
 * @see ELM_ENTRY_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_ENTRY_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_ENTRY_SMART_CLASS_VERSION}

/**
 * @def ELM_ENTRY_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Entry_Smart_Class structure.
 *
 * @see ELM_ENTRY_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_ENTRY_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_ENTRY_SMART_CLASS_INIT_NULL \
  ELM_ENTRY_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_ENTRY_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Entry_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_ENTRY_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Entry_Smart_Class (base field)
 * to the latest #ELM_ENTRY_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_ENTRY_SMART_CLASS_INIT_NULL
 * @see ELM_ENTRY_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_ENTRY_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_ENTRY_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary entry base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a entry.
 *
 * All of the functions listed on @ref Entry namespace will work for
 * objects deriving from #Elm_Entry_Smart_Class.
 */
typedef struct _Elm_Entry_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Entry_Smart_Class;

typedef struct _Mod_Api                     Mod_Api;

/**
 * Base widget smart data extended with entry instance data.
 */
typedef struct _Elm_Entry_Smart_Data        Elm_Entry_Smart_Data;
struct _Elm_Entry_Smart_Data
{
   Elm_Layout_Smart_Data                 base; /* base widget smart data as
                                                * first member obligatory, as
                                                * we're inheriting from it */

   Evas_Object                          *hit_rect, *entry_edje, *scr_edje;
   const Elm_Scrollable_Smart_Interface *s_iface;

   Evas_Object                          *hoversel;
   Ecore_Job                            *deferred_recalc_job;
   Ecore_Event_Handler                  *sel_notify_handler;
   Ecore_Event_Handler                  *sel_clear_handler;
   Ecore_Timer                          *longpress_timer;
   Ecore_Timer                          *delay_write;
   /* for deferred appending */
   Ecore_Idler                          *append_text_idler;
   char                                 *append_text_left;
   int                                   append_text_position;
   int                                   append_text_len;
   /* Only for clipboard */
   const char                           *cut_sel;
   const char                           *text;
   const char                           *file;
   Elm_Text_Format                       format;
   Evas_Coord                            last_w, ent_mw, ent_mh;
   Evas_Coord                            downx, downy;
   Eina_List                            *items;
   Eina_List                            *item_providers;
   Eina_List                            *text_filters;
   Eina_List                            *markup_filters;
   Ecore_Job                            *hov_deljob;
   Mod_Api                              *api; // module api if supplied
   int                                   cursor_pos;
   Elm_Scroller_Policy                   policy_h, policy_v;
   Elm_Wrap_Type                         line_wrap;
   Elm_Input_Panel_Layout                input_panel_layout;
   Elm_Autocapital_Type                  autocapital_type;
   Elm_Input_Panel_Lang                  input_panel_lang;
   Elm_Input_Panel_Return_Key_Type       input_panel_return_key_type;
   void                                 *input_panel_imdata;
   int                                   input_panel_imdata_len;
   struct
   {
      Evas_Object *hover_parent;
      Evas_Object *pop, *hover;
      const char  *hover_style;
   } anchor_hover;

   Elm_Cnp_Mode                          cnp_mode;

   Eina_Bool                             input_panel_return_key_disabled : 1;
   Eina_Bool                             drag_selection_asked : 1;
   Eina_Bool                             input_panel_enable : 1;
   Eina_Bool                             prediction_allow : 1;
   Eina_Bool                             selection_asked : 1;
   Eina_Bool                             auto_return_key : 1;
   Eina_Bool                             have_selection : 1;
   Eina_Bool                             deferred_cur : 1;
   Eina_Bool                             context_menu : 1;
   Eina_Bool                             cur_changed : 1;
   Eina_Bool                             single_line : 1;
   Eina_Bool                             can_write : 1;
   Eina_Bool                             auto_save : 1;
   Eina_Bool                             password : 1;
   Eina_Bool                             editable : 1;
   Eina_Bool                             disabled : 1;
   Eina_Bool                             h_bounce : 1;
   Eina_Bool                             v_bounce : 1;
   Eina_Bool                             has_text : 1;
   Eina_Bool                             use_down : 1;
   Eina_Bool                             sel_mode : 1;
   Eina_Bool                             changed : 1;
   Eina_Bool                             scroll : 1;
};

typedef struct _Elm_Entry_Context_Menu_Item Elm_Entry_Context_Menu_Item;
typedef struct _Elm_Entry_Item_Provider     Elm_Entry_Item_Provider;
typedef struct _Elm_Entry_Markup_Filter     Elm_Entry_Markup_Filter;

struct _Elm_Entry_Context_Menu_Item
{
   Evas_Object  *obj;
   const char   *label;
   const char   *icon_file;
   const char   *icon_group;
   Elm_Icon_Type icon_type;
   Evas_Smart_Cb func;
   void         *data;
};

struct _Elm_Entry_Item_Provider
{
   Evas_Object *(*func)(void *data, Evas_Object * entry, const char *item);
   void        *data;
};

struct _Elm_Entry_Markup_Filter
{
   Elm_Entry_Filter_Cb func;
   void               *data;
};

typedef enum _Length_Unit
{
   LENGTH_UNIT_CHAR,
   LENGTH_UNIT_BYTE,
   LENGTH_UNIT_LAST
} Length_Unit;

/**
 * @}
 */

EAPI extern const char ELM_ENTRY_SMART_NAME[];
EAPI const Elm_Entry_Smart_Class *elm_entry_smart_class_get(void);

#define ELM_ENTRY_DATA_GET(o, sd) \
  Elm_Entry_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_ENTRY_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_ENTRY_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_ENTRY_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ENTRY_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_ENTRY_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check             \
        ((obj), ELM_ENTRY_SMART_NAME, __func__)) \
    return

#endif
