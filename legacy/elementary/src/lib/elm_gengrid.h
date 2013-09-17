/**
 * @defgroup Gengrid Gengrid (Generic grid)
 * @ingroup Elementary
 *
 * @image html gengrid_inheritance_tree.png
 * @image latex gengrid_inheritance_tree.eps
 *
 * This widget aims to position objects in a grid layout while
 * actually creating and rendering only the visible ones, using the
 * same idea as the @ref Genlist "genlist": the user defines a @b
 * class for each item, specifying functions that will be called at
 * object creation, deletion, etc. When those items are selected by
 * the user, a callback function is issued. Users may interact with
 * a gengrid via the mouse (by clicking on items to select them and
 * clicking on the grid's viewport and swiping to pan the whole
 * view) or via the keyboard, navigating through item with the
 * arrow keys.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for gengrid objects.
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base @ref
 * Scroller widget also work for gengrids.
 *
 * Some calls on the gengrid's API are marked as @b deprecated, as
 * they just wrap the scrollable widgets counterpart functions. Use
 * the ones we point you to, for each case of deprecation here,
 * instead -- eventually the deprecated ones will be discarded (next
 * major release).
 *
 * @section Gengrid_Layouts Gengrid layouts
 *
 * Gengrid may layout its items in one of two possible layouts:
 * - horizontal or
 * - vertical.
 *
 * When in "horizontal mode", items will be placed in @b columns,
 * from top to bottom and, when the space for a column is filled,
 * another one is started on the right, thus expanding the grid
 * horizontally, making for horizontal scrolling. When in "vertical
 * mode" , though, items will be placed in @b rows, from left to
 * right and, when the space for a row is filled, another one is
 * started below, thus expanding the grid vertically (and making
 * for vertical scrolling).
 *
 * @section Gengrid_Items Gengrid items
 *
 * An item in a gengrid can have 0 or more texts (they can be
 * regular text or textblock Evas objects - that's up to the style
 * to determine), 0 or more contents (which are simply objects
 * swallowed into the gengrid item's theming Edje object) and 0 or
 * more <b>boolean states</b>, which have the behavior left to the
 * user to define. The Edje part names for each of these properties
 * will be looked up, in the theme file for the gengrid, under the
 * Edje (string) data items named @c "texts", @c "contents" and @c
 * "states", respectively. For each of those properties, if more
 * than one part is provided, they must have names listed separated
 * by spaces in the data fields. For the default gengrid item
 * theme, we have @b one text part (@c "elm.text"), @b two content
 * parts (@c "elm.swalllow.icon" and @c "elm.swallow.end") and @b
 * no state parts.
 *
 * A gengrid item may be at one of several styles. Elementary
 * provides one by default - "default", but this can be extended by
 * system or application custom themes/overlays/extensions (see
 * @ref Theme "themes" for more details).
 *
 * @section Gengrid_Item_Class Gengrid item classes
 *
 * In order to have the ability to add and delete items on the fly,
 * gengrid implements a class (callback) system where the
 * application provides a structure with information about that
 * type of item (gengrid may contain multiple different items with
 * different classes, states and styles). Gengrid will call the
 * functions in this struct (methods) when an item is "realized"
 * (i.e., created dynamically, while the user is scrolling the
 * grid). All objects will simply be deleted when no longer needed
 * with evas_object_del(). The #Elm_Gengrid_Item_Class structure
 * contains the following members:
 * - @c item_style - This is a constant string and simply defines
 * the name of the item style. It @b must be specified and the
 * default should be @c "default".
 * - @c func.text_get - This function is called when an item
 * object is actually created. The @c data parameter will point to
 * the same data passed to elm_gengrid_item_append() and related
 * item creation functions. The @c obj parameter is the gengrid
 * object itself, while the @c part one is the name string of one
 * of the existing text parts in the Edje group implementing the
 * item's theme. This function @b must return a strdup'()ed string,
 * as the caller will free() it when done.
 * See #Elm_Gengrid_Item_Text_Get_Cb.
 * - @c func.content_get - This function is called when an item object
 * is actually created. The @c data parameter will point to the
 * same data passed to elm_gengrid_item_append() and related item
 * creation functions. The @c obj parameter is the gengrid object
 * itself, while the @c part one is the name string of one of the
 * existing (content) swallow parts in the Edje group implementing the
 * item's theme. It must return @c NULL, when no content is desired,
 * or a valid object handle, otherwise. The object will be deleted
 * by the gengrid on its deletion or when the item is "unrealized".
 * See #Elm_Gengrid_Item_Content_Get_Cb.
 * - @c func.state_get - This function is called when an item
 * object is actually created. The @c data parameter will point to
 * the same data passed to elm_gengrid_item_append() and related
 * item creation functions. The @c obj parameter is the gengrid
 * object itself, while the @c part one is the name string of one
 * of the state parts in the Edje group implementing the item's
 * theme. Return @c EINA_FALSE for false/off or @c EINA_TRUE for
 * true/on. Gengrids will emit a signal to its theming Edje object
 * with @c "elm,state,xxx,active" and @c "elm" as "emission" and
 * "source" arguments, respectively, when the state is true (the
 * default is false), where @c xxx is the name of the (state) part.
 * See #Elm_Gengrid_Item_State_Get_Cb.
 * - @c func.del - This is called when elm_object_item_del() is
 * called on an item or elm_gengrid_clear() is called on the
 * gengrid. This is intended for use when gengrid items are
 * deleted, so any data attached to the item (e.g. its data
 * parameter on creation) can be deleted. See #Elm_Gengrid_Item_Del_Cb.
 *
 * @section Gengrid_Usage_Hints Usage hints
 *
 * If the user wants to have multiple items selected at the same
 * time, elm_gengrid_multi_select_set() will permit it. If the
 * gengrid is single-selection only (the default), then
 * elm_gengrid_select_item_get() will return the selected item or
 * @c NULL, if none is selected. If the gengrid is under
 * multi-selection, then elm_gengrid_selected_items_get() will
 * return a list (that is only valid as long as no items are
 * modified (added, deleted, selected or unselected) of child items
 * on a gengrid.
 *
 * If an item changes (internal (boolean) state, text or content
 * changes), then use elm_gengrid_item_update() to have gengrid
 * update the item with the new state. A gengrid will re-"realize"
 * the item, thus calling the functions in the #Elm_Gengrid_Item_Class
 * set for that item.
 *
 * To programmatically (un)select an item, use
 * elm_gengrid_item_selected_set(). To get its selected state use
 * elm_gengrid_item_selected_get(). To make an item disabled
 * (unable to be selected and appear differently) use
 * elm_object_item_disabled_set() to set this and
 * elm_object_item_disabled_get() to get the disabled state.
 *
 * Grid cells will only have their selection smart callbacks called
 * when firstly getting selected. Any further clicks will do
 * nothing, unless you enable the "always select mode", with
 * elm_gengrid_select_mode_set() as ELM_OBJECT_SELECT_MODE_ALWAYS,
 * thus making every click to issue selection callbacks.
 * elm_gengrid_select_mode_set() as ELM_OBJECT_SELECT_MODE_NONE will
 * turn off the ability to select items entirely in the widget and
 * they will neither appear selected nor call the selection smart
 * callbacks.
 *
 * Remember that you can create new styles and add your own theme
 * augmentation per application with elm_theme_extension_add(). If
 * you absolutely must have a specific style that overrides any
 * theme the user or system sets up you can use
 * elm_theme_overlay_add() to add such a file.
 *
 * @section Gengrid_Smart_Events Gengrid smart events
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "activated" - The user has double-clicked or pressed
 *   (enter|return|spacebar) on an item. The @c event_info parameter
 *   is the gengrid item that was activated.
 * - @c "pressed" - The user pressed the an item. The @c event_info
 *   parameter is the item that was pressed.
 * - @c "released" - The user released the an item. The @c event_info
 *   parameter is the item that was released.
 * - @c "clicked,double" - The user has double-clicked an item.
 *   The @c event_info parameter is the gengrid item that was double-clicked.
 * - @c "longpressed" - This is called when the item is pressed for a certain
 *   amount of time. By default it's 1 second.
 * - @c "selected" - The user has made an item selected. The
 *   @c event_info parameter is the gengrid item that was selected.
 * - @c "unselected" - The user has made an item unselected. The
 *   @c event_info parameter is the gengrid item that was unselected.
 * - @c "realized" - This is called when the item in the gengrid
 *   has its implementing Evas object instantiated, de facto. @c
 *   event_info is the gengrid item that was created.
 * - @c "unrealized" - This is called when the implementing Evas
 *   object for this item is deleted. @c event_info is the gengrid
 *   item that was deleted.
 * - @c "changed" - Called when an item is added, removed, resized
 *   or moved and when the gengrid is resized or gets "horizontal"
 *   property changes.
 * - @c "scroll,anim,start" - This is called when scrolling animation has
 *   started.
 * - @c "scroll,anim,stop" - This is called when scrolling animation has
 *   stopped.
 * - @c "drag,start,up" - Called when the item in the gengrid has
 *   been dragged (not scrolled) up.
 * - @c "drag,start,down" - Called when the item in the gengrid has
 *   been dragged (not scrolled) down.
 * - @c "drag,start,left" - Called when the item in the gengrid has
 *   been dragged (not scrolled) left.
 * - @c "drag,start,right" - Called when the item in the gengrid has
 *   been dragged (not scrolled) right.
 * - @c "drag,stop" - Called when the item in the gengrid has
 *   stopped being dragged.
 * - @c "drag" - Called when the item in the gengrid is being
 *   dragged.
 * - @c "scroll" - called when the content has been scrolled
 *   (moved).
 * - @c "scroll,drag,start" - called when dragging the content has
 *   started.
 * - @c "scroll,drag,stop" - called when dragging the content has
 *   stopped.
 * - @c "scroll,page,changed" - called when the visible page has
 *   changed.
 * - @c "edge,top" - This is called when the gengrid is scrolled until
 *   the top edge.
 * - @c "edge,bottom" - This is called when the gengrid is scrolled
 *   until the bottom edge.
 * - @c "edge,left" - This is called when the gengrid is scrolled
 *   until the left edge.
 * - @c "edge,right" - This is called when the gengrid is scrolled
 *   until the right edge.
 * - @c "moved" - This is called when a gengrid item is moved by a user
 *   interaction in a reorder mode. The %c event_info parameter is the item that
 *   was moved.
 * - @c "index,update" - This is called when a gengrid item index is changed.
 *   Note that this callback is called while each item is being realized.
 * - @c "highlighted" - an item in the list is highlighted. This is called when
 *   the user presses an item or keyboard selection is done so the item is
 *   physically highlighted. The %c event_info parameter is the item that was
 *   highlighted.
 * - @c "unhighlighted" - an item in the list is unhighlighted. This is called
 *   when the user releases an item or keyboard selection is moved so the item
 *   is physically unhighlighted. The %c event_info parameter is the item that
 *   was unhighlighted.
 * - @c "language,changed" - This is called when the program's language is
 *   changed. Call the elm_gengrid_realized_items_update() if items text should
 *   be translated.
 * - @c "focused" - When the gengrid has received focus. (since 1.8)
 * - @c "unfocused" - When the gengrid has lost focus. (since 1.8)
 *
 * Supported elm_object common APIs
 * @li elm_object_signal_emit()
 *
 * Supported elm_object_item common APIs
 * @li elm_object_item_part_content_get
 * @li elm_object_item_part_text_get
 * @li elm_object_item_disabled_set
 * @li elm_object_item_disabled_get
 * @li elm_object_item_del
 * @li elm_object_item_signal_emit
 *
 * Unsupported elm_object_item common APIs due to the gengrid concept.
 * Gengrid fills content/text according to the appropriate callback functions.
 * Please use elm_gengrid_item_update() instead.
 * @li elm_object_item_part_content_set()
 * @li elm_object_item_part_content_unset()
 * @li elm_object_item_part_text_set()
 *
 * List of gengrid examples:
 * @li @ref gengrid_example
 */

/**
 * @addtogroup Gengrid
 * @{
 */

#include <elm_gengrid_common.h>
#ifdef EFL_EO_API_SUPPORT
#include <elm_gengrid_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_gengrid_legacy.h>
#endif

/**
 * @}
 */
