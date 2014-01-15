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
 * Base widget smart data extended with layout instance data.
 */
typedef struct _Elm_Layout_Smart_Data
{
   Evas_Object          *obj; /* The object itself */
   Eina_List            *subs; /**< List of Elm_Layout_Sub_Object_Data structs, to hold the actual sub objects */
   Eina_List            *edje_signals;
   Eina_List            *parts_cursors;
   const char           *klass, *group;
   int                   frozen; /**< Layout freeze counter */

   Eina_Bool             needs_size_calc : 1;
   Eina_Bool             restricted_calc_w : 1;
   Eina_Bool             restricted_calc_h : 1;
   Eina_Bool             can_access : 1; /**< This is true when all text(including textblock) parts can be accessible by accessibility. */
} Elm_Layout_Smart_Data;

/**
 * @}
 */

#define ELM_LAYOUT_DATA_GET(o, sd) \
  Elm_Layout_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_LAYOUT_CLASS)

#define ELM_LAYOUT_CHECK(obj)                            \
  if (EINA_UNLIKELY(!eo_isa(obj, ELM_OBJ_LAYOUT_CLASS))) \
    return

#endif
