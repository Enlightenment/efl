#ifndef ELM_WIDGET_PREFS_H
#define ELM_WIDGET_PREFS_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-prefs-class The Elementary Prefs Class
 *
 * Elementary, besides having the @ref Prefs widget, exposes its
 * foundation -- the Elementary Prefs Class -- in order to create
 * other widgets which are a prefs with some more logic on top.
 */

typedef struct _Elm_Prefs_Page_Node
{
   unsigned int                version;

   /* not to be serialized */
   Evas_Object                *prefs;
   Evas_Object                *parent;
   Evas_Object                *w_obj;
   const Elm_Prefs_Page_Iface *w_impl;

   const char                 *name;
   const char                 *title;
   const char                 *sub_title;
   const char                 *widget;
   const char                 *style;
   const char                 *icon;

   Eina_List                  *items;

   Eina_Bool                   autosave;
} Elm_Prefs_Page_Node;

typedef struct _Elm_Prefs_Item_Node
{
   Elm_Prefs_Item_Type         type;

   /* not to be serialized */
   Evas_Object                *prefs;
   Elm_Prefs_Page_Node        *page;
   Elm_Prefs_Page_Node        *subpage; /* page item type only */
   Evas_Object                *w_obj;
   const Elm_Prefs_Item_Iface *w_impl;
   Eina_Bool                   available;

   const char                 *name;
   const char                 *label;
   const char                 *icon;
   const char                 *style;
   const char                 *widget;

   Elm_Prefs_Item_Spec         spec;

   Eina_Bool                   persistent;
   Eina_Bool                   editable;
   Eina_Bool                   visible;
} Elm_Prefs_Item_Node;

/**
 * Base widget smart data extended with prefs instance data.
 */
typedef struct _Elm_Prefs_Smart_Data Elm_Prefs_Smart_Data;
struct _Elm_Prefs_Smart_Data
{
   Elm_Prefs_Page_Node  *root;

   Elm_Prefs_Data       *prefs_data;
   const char           *file;
   const char           *page;

   Ecore_Poller         *saving_poller;

   Eina_Bool             changing_from_ui : 1;
   Eina_Bool             values_fetching : 1;
   Eina_Bool             delete_me : 1;
   Eina_Bool             autosave : 1;
   Eina_Bool             dirty : 1;
};

/**
 * @}
 */

extern const Eina_Hash *elm_prefs_item_widgets_map;
extern const Eina_Hash *elm_prefs_item_type_widgets_map;
extern const Elm_Prefs_Item_Iface *elm_prefs_item_default_widget;

#define ELM_PREFS_DATA_GET(o, sd) \
  Elm_Prefs_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_PREFS_CLASS)

#define ELM_PREFS_ENTRY(o, sd)                                              \
  if (EINA_UNLIKELY(!sd->data_file || !sd->page))                           \
    {                                                                       \
       CRI("You must issue elm_prefs_file_set() on this widget before"      \
           " you make this call");                                          \
    }                                                                       \
  return

#define ELM_PREFS_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_PREFS_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_PREFS_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PREFS_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_PREFS_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_PREFS_CLASS))) \
    return

#endif
