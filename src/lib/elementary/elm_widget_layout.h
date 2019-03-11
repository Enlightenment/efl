#ifndef ELM_WIDGET_LAYOUT_H
#define ELM_WIDGET_LAYOUT_H

#include "elm_widget_container.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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
 * Base widget smart data extended with layout instance data.
 */
typedef struct _Elm_Layout_Smart_Data
{
   Evas_Object          *obj; /**< The object itself */
   Eina_List            *subs; /**< List of Elm_Layout_Sub_Object_Data structs, to hold the actual sub objects such as text, content and the children of box and table. */
   Eina_List            *edje_signals; /**< The list of edje signal callbacks. */
   Eina_List            *parts_cursors; /**< The list of cursor names of layout parts. This is a list of Elm_Layout_Sub_Object_Cursor struct. */

   struct {
      Eina_Hash         *properties; /**< The list of properties connected to layout parts. */
      Eina_Hash         *signals; /**< The list of signals connected. */
      Eina_Hash         *factories; /**< The hash with parts connected to factories. */

      Eina_Bool          updating : 1;
   } connect;

   int                   frozen; /**< Layout freeze counter */

   Eina_Bool             needs_size_calc : 1; /**< This flag is set true when the layout sizing eval is already requested. This defers sizing evaluation until smart calculation to avoid unnecessary calculation. */
   Eina_Bool             restricted_calc_w : 1; /**< This is a flag to support edje restricted_calc in w axis. */
   Eina_Bool             restricted_calc_h : 1; /**< This is a flag to support edje restricted_calc in y axis. */
   Eina_Bool             can_access : 1; /**< This is true when all text(including textblock) parts can be accessible by accessibility. */
   Eina_Bool             destructed_is : 1; /**< This flag indicates if Efl.Ui.Layout destructor was called. This is needed to avoid unnecessary calculation of subobject deletion during layout object's deletion. */
   Eina_Bool             file_set : 1; /**< This flag indicates if Efl.Ui.Layout source is set from a file*/
   Eina_Bool             automatic_orientation_apply : 1;
} Efl_Ui_Layout_Data;

/**
 * @}
 */

#define EFL_UI_LAYOUT_DATA_GET(o, sd) \
  Efl_Ui_Layout_Data * sd = efl_data_scope_get(o, EFL_UI_LAYOUT_BASE_CLASS)

#define EFL_UI_LAYOUT_CHECK(obj) \
  if (EINA_UNLIKELY(!efl_isa(obj, EFL_UI_LAYOUT_BASE_CLASS))) \
    return

#endif
