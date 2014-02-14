/**
 * @defgroup Genlist Genlist (Generic list)
 * @ingroup Elementary
 *
 * @image html genlist_inheritance_tree.png
 * @image latex genlist_inheritance_tree.eps
 *
 * @image html img/widget/genlist/preview-00.png
 * @image latex img/widget/genlist/preview-00.eps
 * @image html img/genlist.png
 * @image latex img/genlist.eps
 *
 * This widget aims to have more expansive list than the simple list in
 * Elementary that could have more flexible items and allow many more entries
 * while still being fast and low on memory usage. At the same time it was
 * also made to be able to do tree structures. But the price to pay is more
 * complexity when it comes to usage. If all you want is a simple list with
 * icons and a single text, use the normal @ref List object.
 *
 * Genlist has a fairly large API, mostly because it's relatively complex,
 * trying to be both expansive, powerful and efficient. First we will begin
 * an overview on the theory behind genlist.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for genlist objects.
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base @ref
 * Scroller widget also work for genlists.
 *
 * Some calls on the genlist's API are marked as @b deprecated, as
 * they just wrap the scrollable widgets counterpart functions. Use
 * the ones we point you to, for each case of deprecation here,
 * instead -- eventually the deprecated ones will be discarded (next
 * major release).
 *
 * @section Genlist_Item_Class Genlist item classes - creating items
 *
 * In order to have the ability to add and delete items on the fly, genlist
 * implements a class (callback) system where the application provides a
 * structure with information about that type of item (genlist may contain
 * multiple different items with different classes, states and styles).
 * Genlist will call the functions in this struct (methods) when an item is
 * "realized" (i.e., created dynamically, while the user is scrolling the
 * list). All objects will simply be deleted when no longer needed with
 * evas_object_del(). The #Elm_Genlist_Item_Class structure contains the
 * following members:
 * - @c item_style - This is a constant string and simply defines the name
 *   of the item style. It @b must be specified and the default should be @c
 *   "default".
 * - @c decorate_item_style - This is a constant string and simply defines the name
 *   of the decorate mode item style. It is used to specify decorate mode item style. It can be
 *   used when you call elm_genlist_item_decorate_mode_set().
 * - @c decorate_all_item_style - This is a constant string and simply defines the name
 *   of the decorate all item style. It is used to specify decorate all item style. It can be
 *   used to set selection, checking and deletion mode. This is used when you
 *   call elm_genlist_decorate_mode_set().
 * - @c func - A struct with pointers to functions that will be called when
 *   an item is going to be actually created. All of them receive a @c data
 *   parameter that will point to the same data passed to
 *   elm_genlist_item_append() and related item creation functions, and an @c
 *   obj parameter that points to the genlist object itself.
 *
 * The function pointers inside @c func are @c text_get, @c content_get, @c
 * state_get and @c del. The 3 first functions also receive a @c part
 * parameter described below. A brief description of these functions follows:
 *
 * - @c text_get - The @c part parameter is the name string of one of the
 *   existing text parts in the Edje group implementing the item's theme.
 *   This function @b must return a strdup'()ed string, as the caller will
 *   free() it when done. See #Elm_Genlist_Item_Text_Get_Cb.
 * - @c content_get - The @c part parameter is the name string of one of the
 *   existing (content) swallow parts in the Edje group implementing the item's
 *   theme. It must return @c NULL, when no content is desired, or a valid
 *   object handle, otherwise.  The object will be deleted by the genlist on
 *   its deletion or when the item is "unrealized".
 *   See #Elm_Genlist_Item_Content_Get_Cb.
 * - @c func.state_get - The @c part parameter is the name string of one of
 *   the state parts in the Edje group implementing the item's theme. Return
 *   @c EINA_FALSE for false/off or @c EINA_TRUE for true/on. Genlists will
 *   emit a signal to its theming Edje object with @c "elm,state,xxx,active"
 *   and @c "elm" as "emission" and "source" arguments, respectively, when
 *   the state is true (the default is false), where @c xxx is the name of
 *   the (state) part.  See #Elm_Genlist_Item_State_Get_Cb.
 * - @c func.del - This is intended for use when genlist items are deleted,
 *   so any data attached to the item (e.g. its data parameter on creation)
 *   can be deleted. See #Elm_Genlist_Item_Del_Cb.
 *
 * available item styles:
 * - default
 * - default_style - The text part is a textblock
 *
 * @image html img/widget/genlist/preview-04.png
 * @image latex img/widget/genlist/preview-04.eps
 *
 * - double_label
 *
 * @image html img/widget/genlist/preview-01.png
 * @image latex img/widget/genlist/preview-01.eps
 *
 * - icon_top_text_bottom
 *
 * @image html img/widget/genlist/preview-02.png
 * @image latex img/widget/genlist/preview-02.eps
 *
 * - group_index
 *
 * @image html img/widget/genlist/preview-03.png
 * @image latex img/widget/genlist/preview-03.eps
 *
 * - one_icon - Only 1 icon (left) (since 1.7)
 * - end_icon - Only 1 icon (at end/right) (since 1.7)
 * - no_icon - No icon (at end/right) (since 1.7)
 * - full - Only 1 icon, elm.swallow.content,  which consumes whole area of
 * genlist itemj (since 1.7)
 *
 * If one wants to use more icons and texts than are offered in theme, there
 * are two solutions. One is to use 'full' style that has one big swallow part.
 * You can swallow anything there. The other solution is to customize genlist
 * item style in application side by using elm_theme_extension_add() and its
 * own edc. Please refer @ref theme_example_01 for that.
 *
 * @section Genlist_Items Structure of items
 *
 * An item in a genlist can have 0 or more texts (they can be regular
 * text or textblock Evas objects - that's up to the style to determine), 0
 * or more contents (which are simply objects swallowed into the genlist item's
 * theming Edje object) and 0 or more <b>boolean states</b>, which have the
 * behavior left to the user to define. The Edje part names for each of
 * these properties will be looked up, in the theme file for the genlist,
 * under the Edje (string) data items named @c "labels", @c "contents" and @c
 * "states", respectively. For each of those properties, if more than one
 * part is provided, they must have names listed separated by spaces in the
 * data fields. For the default genlist item theme, we have @b one text
 * part (@c "elm.text"), @b two content parts (@c "elm.swalllow.icon" and @c
 * "elm.swallow.end") and @b no state parts.
 *
 * A genlist item may be at one of several styles. Elementary provides one
 * by default - "default", but this can be extended by system or application
 * custom themes/overlays/extensions (see @ref Theme "themes" for more
 * details).
 *
 * @section Genlist_Manipulation Editing and Navigating
 *
 * Items can be added by several calls. All of them return a @ref
 * Elm_Object_Item handle that is an internal member inside the genlist.
 * They all take a data parameter that is meant to be used for a handle to
 * the applications internal data (eg. the struct with the original item
 * data). The parent parameter is the parent genlist item this belongs to if
 * it is a tree or an indexed group, and NULL if there is no parent. The
 * flags can be a bitmask of #ELM_GENLIST_ITEM_NONE, #ELM_GENLIST_ITEM_TREE
 * and #ELM_GENLIST_ITEM_GROUP. If #ELM_GENLIST_ITEM_TREE is set then this
 * item is displayed as an item that is able to expand and have child items.
 * If #ELM_GENLIST_ITEM_GROUP is set then this item is group index item that
 * is displayed at the top until the next group comes. The func parameter is
 * a convenience callback that is called when the item is selected and the
 * data parameter will be the func_data parameter, @c obj be the genlist
 * object and event_info will be the genlist item.
 *
 * elm_genlist_item_append() adds an item to the end of the list, or if
 * there is a parent, to the end of all the child items of the parent.
 * elm_genlist_item_prepend() is the same but adds to the beginning of
 * the list or children list. elm_genlist_item_insert_before() inserts at
 * item before another item and elm_genlist_item_insert_after() inserts after
 * the indicated item.
 *
 * The application can clear the list with elm_genlist_clear() which deletes
 * all the items in the list and elm_object_item_del() will delete a specific
 * item. elm_genlist_item_subitems_clear() will clear all items that are
 * children of the indicated parent item.
 *
 * To help inspect list items you can jump to the item at the top of the list
 * with elm_genlist_first_item_get() which will return the item pointer, and
 * similarly elm_genlist_last_item_get() gets the item at the end of the list.
 * elm_genlist_item_next_get() and elm_genlist_item_prev_get() get the next
 * and previous items respectively relative to the indicated item. Using
 * these calls you can walk the entire item list/tree. Note that as a tree
 * the items are flattened in the list, so elm_genlist_item_parent_get() will
 * let you know which item is the parent (and thus know how to skip them if
 * wanted).
 *
 * @section Genlist_Multi_Selection Multi-selection
 *
 * If the application wants multiple items to be able to be selected,
 * elm_genlist_multi_select_set() can enable this. If the list is
 * single-selection only (the default), then elm_genlist_selected_item_get()
 * will return the selected item, if any, or NULL if none is selected. If the
 * list is multi-select then elm_genlist_selected_items_get() will return a
 * list (that is only valid as long as no items are modified (added, deleted,
 * selected or unselected)).
 *
 * @section Genlist_Usage_Hints Usage hints
 *
 * There are also convenience functions. elm_object_item_widget_get() will
 * return the genlist object the item belongs to. elm_genlist_item_show()
 * will make the scroller scroll to show that specific item so its visible.
 * elm_object_item_data_get() returns the data pointer set by the item
 * creation functions.
 *
 * If an item changes (state of boolean changes, text or contents change),
 * then use elm_genlist_item_update() to have genlist update the item with
 * the new state. Genlist will re-realize the item and thus call the functions
 * in the _Elm_Genlist_Item_Class for that item.
 *
 * To programmatically (un)select an item use elm_genlist_item_selected_set().
 * To get its selected state use elm_genlist_item_selected_get(). Similarly
 * to expand/contract an item and get its expanded state, use
 * elm_genlist_item_expanded_set() and elm_genlist_item_expanded_get(). And
 * again to make an item disabled (unable to be selected and appear
 * differently) use elm_object_item_disabled_set() to set this and
 * elm_object_item_disabled_get() to get the disabled state.
 *
 * In general to indicate how the genlist should expand items horizontally to
 * fill the list area, use elm_genlist_mode_set(). Valid modes are
 * ELM_LIST_LIMIT, ELM_LIST_COMPRESS and ELM_LIST_SCROLL. The default is
 * ELM_LIST_SCROLL. This mode means that if items are too wide to fit, the
 * scroller will scroll horizontally. Otherwise items are expanded to
 * fill the width of the viewport of the scroller. If it is
 * ELM_LIST_LIMIT, items will be expanded to the viewport width
 * if larger than the item, but genlist widget witdh is
 * limited to the largest item. Do not use ELM_LIST_LIMIT mode with homogenous
 * mode turned on. ELM_LIST_COMPRESS can be combined with a different style
 * that uses edjes' ellipsis feature (cutting text off like this: "tex...").
 *
 * Items will call their selection func and callback only once when first becoming
 * selected. Any further clicks will do nothing, unless you enable always
 * select with elm_genlist_select_mode_set() as ELM_OBJECT_SELECT_MODE_ALWAYS.
 * This means even if selected, every click will make the selected callbacks
 * be called. elm_genlist_select_mode_set() as ELM_OBJECT_SELECT_MODE_NONE will
 * turn off the ability to select items entirely and they will neither
 * appear selected nor call selected callback functions.
 *
 * Remember that you can create new styles and add your own theme augmentation
 * per application with elm_theme_extension_add(). If you absolutely must
 * have a specific style that overrides any theme the user or system sets up
 * you can use elm_theme_overlay_add() to add such a file.
 *
 * @section Genlist_Implementation Implementation
 *
 * Evas tracks every object you create. Every time it processes an event
 * (mouse move, down, up etc.) it needs to walk through objects and find out
 * what event that affects. Even worse every time it renders display updates,
 * in order to just calculate what to re-draw, it needs to walk through many
 * many many objects. Thus, the more objects you keep active, the more
 * overhead Evas has in just doing its work. It is advisable to keep your
 * active objects to the minimum working set you need. Also remember that
 * object creation and deletion carries an overhead, so there is a
 * middle-ground, which is not easily determined. But don't keep massive lists
 * of objects you can't see or use. Genlist does this with list objects. It
 * creates and destroys them dynamically as you scroll around. It groups them
 * into blocks so it can determine the visibility etc. of a whole block at
 * once as opposed to having to walk the whole list. This 2-level list allows
 * for very large numbers of items to be in the list (tests have used up to
 * 2,000,000 items). Also genlist employs a queue for adding items. As items
 * may be different sizes, every item added needs to be calculated as to its
 * size and thus this presents a lot of overhead on populating the list, this
 * genlist employs a queue. Any item added is queued and spooled off over
 * time, actually appearing some time later, so if your list has many members
 * you may find it takes a while for them to all appear, with your process
 * consuming a lot of CPU while it is busy spooling.
 *
 * Genlist also implements a tree structure for items, but it does so with
 * callbacks to  the application, with the application filling in tree
 * structures when requested (allowing for efficient building of a very
 * deep tree that could even be used for file-management).
 * See the above smart signal callbacks for details.
 *
 * @section Genlist_Smart_Events Genlist smart events
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "activated" - The user has double-clicked or pressed
 *   (enter|return|spacebar) on an item. The @c event_info parameter is the
 *   item that was activated.
 * - @c "pressed" - The user pressed the an item. The @c event_info
 *   parameter is the item that was pressed.
 * - @c "released" - The user released the an item. The @c event_info
 *   parameter is the item that was released.
 * - @c "clicked,double" - The user has double-clicked an item.  The @c
 *   event_info parameter is the item that was double-clicked.
 * - @c "selected" - This is called when a user has made an item selected.
 *   The event_info parameter is the genlist item that was selected.
 * - @c "unselected" - This is called when a user has made an item
 *   unselected. The event_info parameter is the genlist item that was
 *   unselected.
 * - @c "expanded" - This is called when elm_genlist_item_expanded_set() is
 *   called and the item is now meant to be expanded. The event_info
 *   parameter is the genlist item that was indicated to expand.  It is the
 *   job of this callback to then fill in the child items.
 * - @c "contracted" - This is called when elm_genlist_item_expanded_set() is
 *   called and the item is now meant to be contracted. The event_info
 *   parameter is the genlist item that was indicated to contract. It is the
 *   job of this callback to then delete the child items.
 * - @c "expand,request" - This is called when a user has indicated they want
 *   to expand a tree branch item. The callback should decide if the item can
 *   expand (has any children) and then call elm_genlist_item_expanded_set()
 *   appropriately to set the state. The event_info parameter is the genlist
 *   item that was indicated to expand.
 * - @c "contract,request" - This is called when a user has indicated they
 *   want to contract a tree branch item. The callback should decide if the
 *   item can contract (has any children) and then call
 *   elm_genlist_item_expanded_set() appropriately to set the state. The
 *   event_info parameter is the genlist item that was indicated to contract.
 * - @c "realized" - This is called when the item in the list is created as a
 *   real evas object. event_info parameter is the genlist item that was
 *   created.
 * - @c "unrealized" - This is called just before an item is unrealized.
 *   After this call content objects provided will be deleted and the item
 *   object itself delete or be put into a floating cache.
 * - @c "drag,start,up" - This is called when the item in the list has been
 *   dragged (not scrolled) up.
 * - @c "drag,start,down" - This is called when the item in the list has been
 *   dragged (not scrolled) down.
 * - @c "drag,start,left" - This is called when the item in the list has been
 *   dragged (not scrolled) left.
 * - @c "drag,start,right" - This is called when the item in the list has
 *   been dragged (not scrolled) right.
 * - @c "drag,stop" - This is called when the item in the list has stopped
 *   being dragged.
 * - @c "drag" - This is called when the item in the list is being dragged.
 * - @c "longpressed" - This is called when the item is pressed for a certain
 *   amount of time. By default it's 1 second. The event_info parameter is the
 *   longpressed genlist item.
 * - @c "scroll,anim,start" - This is called when scrolling animation has
 *   started.
 * - @c "scroll,anim,stop" - This is called when scrolling animation has
 *   stopped.
 * - @c "scroll,drag,start" - This is called when dragging the content has
 *   started.
 * - @c "scroll,drag,stop" - This is called when dragging the content has
 *   stopped.
 * - @c "edge,top" - This is called when the genlist is scrolled until
 *   the top edge.
 * - @c "edge,bottom" - This is called when the genlist is scrolled
 *   until the bottom edge.
 * - @c "edge,left" - This is called when the genlist is scrolled
 *   until the left edge.
 * - @c "edge,right" - This is called when the genlist is scrolled
 *   until the right edge.
 * - @c "multi,swipe,left" - This is called when the genlist is multi-touch
 *   swiped left.
 * - @c "multi,swipe,right" - This is called when the genlist is multi-touch
 *   swiped right.
 * - @c "multi,swipe,up" - This is called when the genlist is multi-touch
 *   swiped up.
 * - @c "multi,swipe,down" - This is called when the genlist is multi-touch
 *   swiped down.
 * - @c "multi,pinch,out" - This is called when the genlist is multi-touch
 *   pinched out.
 * - @c "multi,pinch,in" - This is called when the genlist is multi-touch
 *   pinched in.
 * - @c "swipe" - This is called when the genlist is swiped.
 * - @c "moved" - This is called when a genlist item is moved by a user
 *   interaction in a reorder mode. The %c event_info parameter is the item that
 *   was moved.
 * - @c "moved,after" - This is called when a genlist item is moved after
 *   another item in reorder mode. The event_info parameter is the reordered
 *   item. To get the relative previous item, use elm_genlist_item_prev_get().
 *   This signal is called along with "moved" signal.
 * - @c "moved,before" - This is called when a genlist item is moved before
 *   another item in reorder mode. The event_info parameter is the reordered
 *   item. To get the relative previous item, use elm_genlist_item_next_get().
 *   This signal is called along with "moved" signal.
 * - @c "index,update" - This is called when a genlist item index is changed.
 *   Note that this callback is called while each item is being realized.
 * - @c "language,changed" - This is called when the program's language is
 *   changed. Call the elm_genlist_realized_items_update() if items text should
 *   be translated.
 * - @c "tree,effect,finished" - This is called when a genlist tree effect is finished.
 * - @c "highlighted" - an item in the list is highlighted. This is called when
 *   the user presses an item or keyboard selection is done so the item is
 *   physically highlighted. The %c event_info parameter is the item that was
 *   highlighted.
 * - @c "unhighlighted" - an item in the list is unhighlighted. This is called
 *   when the user releases an item or keyboard selection is moved so the item
 *   is physically unhighlighted. The %c event_info parameter is the item that
 *   was unhighlighted.
 * - @c "focused" - When the genlist has received focus. (since 1.8)
 * - @c "unfocused" - When the genlist has lost focus. (since 1.8)
 *
 *
 * Supported elm_object_item common APIs
 * @li @ref elm_object_item_part_content_get
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_disabled_set
 * @li @ref elm_object_item_disabled_get
 * @li @ref elm_object_item_signal_emit
 *
 * Unsupported elm_object_item common APIs due to the genlist concept.
 * Genlist fills content/text according to the appropriate callback functions.
 * Please use elm_genlist_item_update() or elm_genlist_item_fields_update()
 * instead.
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_unset
 * @li @ref elm_object_item_part_text_set
 *
 * @section Genlist_Examples Examples
 *
 * Here is a list of examples that use the genlist, trying to show some of
 * its capabilities:
 * - @ref genlist_example_01
 * - @ref genlist_example_02
 * - @ref genlist_example_03
 * - @ref genlist_example_04
 * - @ref genlist_example_05
 */

/**
 * @addtogroup Genlist
 * @{
 */

#include <elm_genlist_common.h>
#ifdef EFL_EO_API_SUPPORT
#include <elm_genlist_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_genlist_legacy.h>
#endif

/**
 * @}
 */
