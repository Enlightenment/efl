/**
 * @defgroup Genlist Genlist
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
 * - one_icon - Only 1 icon (left) @since 1.7
 * - end_icon - Only 1 icon (at end/right) @since 1.7
 * - no_icon - No icon (at end/right) @since 1.7
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
 * if larger than the item, but genlist widget with is
 * limited to the largest item. D not use ELM_LIST_LIMIT mode with homogenous
 * mode turned on. ELM_LIST_COMPRESS can be combined with a different style
 * that uses edjes' ellipsis feature (cutting text off like this: "tex...").
 *
 * Items will only call their selection func and callback when first becoming
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
 * Genlist also implements a tree structure, but it does so with callbacks to
 * the application, with the application filling in tree structures when
 * requested (allowing for efficient building of a very deep tree that could
 * even be used for file-management). See the above smart signal callbacks for
 * details.
 *
 * @section Genlist_Smart_Events Genlist smart events
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "activated" - The user has double-clicked or pressed
 *   (enter|return|spacebar) on an item. The @c event_info parameter is the
 *   item that was activated.
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
 * - @c multi,pinch,in" - This is called when the genlist is multi-touch
 *   pinched in.
 * - @c "swipe" - This is called when the genlist is swiped.
 * - @c "moved" - This is called when a genlist item is moved in reorder mode.
 * - @c "moved,after" - This is called when a genlist item is moved after
 *   another item in reorder mode. The event_info parameter is the reordered
 *   item. To get the relative previous item, use elm_genlist_item_prev_get().
 *   This signal is called along with "moved" signal.
 * - @c "moved,before" - This is called when a genlist item is moved before
 *   another item in reorder mode. The event_info parameter is the reordered
 *   item. To get the relative previous item, use elm_genlist_item_next_get().
 *   This signal is called along with "moved" signal.
 * - @c "language,changed" - This is called when the program's language is
 *   changed.
 * - @c "tree,effect,finished" - This is called when a genlist tree effect is finished.
 *
 *
 * Supported elm_object_item common APIs
 * @li @ref elm_object_item_part_content_get()
 * @li @ref elm_object_item_part_text_get()
 * @li @ref elm_object_item_disabled_set()
 * @li @ref elm_object_item_disabled_get()
 * @li @ref elm_object_item_signal_emit()
 *
 * Unsupported elm_object_item common APIs due to the genlist concept.
 * Genlist fills content/text according to the appropriate callback functions.
 * Please use elm_genlist_item_update() or elm_genlist_item_fields_update()
 * instead.
 * @li @ref elm_object_item_part_content_set()
 * @li @ref elm_object_item_part_content_unset()
 * @li @ref elm_object_item_part_text_set()
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

#define ELM_GENLIST_ITEM_CLASS_VERSION ELM_GEN_ITEM_CLASS_VERSION
#define ELM_GENLIST_ITEM_CLASS_HEADER ELM_GEN_ITEM_CLASS_HEADER

/**
 * Defines if the item is of any special type (has subitems or it's the
 * index of a group), or is just a simple item.
 *
 * @ingroup Genlist
 */
typedef enum
{
   ELM_GENLIST_ITEM_NONE = 0, /**< simple item */
   ELM_GENLIST_ITEM_TREE = (1 << 0), /**< this may be expanded and have child items. */
   ELM_GENLIST_ITEM_GROUP = (1 << 1), /**< an index item of a group of items. this item can have child items. */

   ELM_GENLIST_ITEM_MAX = (1 << 2)
} Elm_Genlist_Item_Type;

/**
 * Defines the type of the item part
 * Used while updating item's parts
 * It can be used at updating multi fields.
 *
 * @ingroup Genlist
 */
typedef enum
{
   ELM_GENLIST_ITEM_FIELD_ALL = 0,
   ELM_GENLIST_ITEM_FIELD_TEXT = (1 << 0),
   ELM_GENLIST_ITEM_FIELD_CONTENT = (1 << 1),
   ELM_GENLIST_ITEM_FIELD_STATE = (1 << 2)
} Elm_Genlist_Item_Field_Type;

/**
 * Defines where to position the item in the genlist.
 *
 * @ingroup Genlist
 */
typedef enum
{
   ELM_GENLIST_ITEM_SCROLLTO_NONE = 0,   /**< no scrollto */
   ELM_GENLIST_ITEM_SCROLLTO_IN = (1 << 0),   /**< to the nearest viewport */
   ELM_GENLIST_ITEM_SCROLLTO_TOP = (1 << 1),   /**< to the top of viewport */
   ELM_GENLIST_ITEM_SCROLLTO_MIDDLE = (1 << 2)   /**< to the middle of viewport */
} Elm_Genlist_Item_Scrollto_Type;

/**
 * @see Elm_Gen_Item_Class
 */
typedef Elm_Gen_Item_Class Elm_Genlist_Item_Class;

/**
 * @see Elm_Gen_Item_Text_Get_Cb
 */
typedef Elm_Gen_Item_Text_Get_Cb Elm_Genlist_Item_Text_Get_Cb;

/**
 * @see Elm_Gen_Item_Content_Get_Cb
 */
typedef Elm_Gen_Item_Content_Get_Cb Elm_Genlist_Item_Content_Get_Cb;

/**
 * @see Elm_Gen_Item_State_Get_Cb
 */
typedef Elm_Gen_Item_State_Get_Cb Elm_Genlist_Item_State_Get_Cb;

/**
 * @see Elm_Gen_Item_Del_Cb
 */
typedef Elm_Gen_Item_Del_Cb Elm_Genlist_Item_Del_Cb;

/**
 * Add a new genlist widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new genlist widget handle or @c NULL, on errors
 *
 * This function inserts a new genlist widget on the canvas.
 *
 * @see elm_genlist_item_append()
 * @see elm_object_item_del()
 * @see elm_genlist_clear()
 *
 * @ingroup Genlist
 */
EAPI Evas_Object                  *elm_genlist_add(Evas_Object *parent);

/**
 * Remove all items from a given genlist widget.
 *
 * @param obj The genlist object
 *
 * This removes (and deletes) all items in @p obj, leaving it empty.
 *
 * @see elm_object_item_del(), to remove just one item.
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_clear(Evas_Object *obj);

/**
 * Enable or disable multi-selection in the genlist
 *
 * @param obj The genlist object
 * @param multi Multi-select enable/disable. Default is disabled.
 *
 * This enables (@c EINA_TRUE) or disables (@c EINA_FALSE) multi-selection in
 * the list. This allows more than 1 item to be selected. To retrieve the list
 * of selected items, use elm_genlist_selected_items_get().
 *
 * @see elm_genlist_selected_items_get()
 * @see elm_genlist_multi_select_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_multi_select_set(Evas_Object *obj, Eina_Bool multi);

/**
 * Gets if multi-selection in genlist is enabled or disabled.
 *
 * @param obj The genlist object
 * @return Multi-select enabled/disabled
 * (@c EINA_TRUE = enabled/@c EINA_FALSE = disabled). Default is @c EINA_FALSE.
 *
 * @see elm_genlist_multi_select_set()
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_multi_select_get(const Evas_Object *obj);

/**
 * This sets the horizontal stretching mode.
 *
 * @param obj The genlist object
 * @param mode The mode to use (one of #ELM_LIST_SCROLL or #ELM_LIST_LIMIT).
 *
 * This sets the mode used for sizing items horizontally. Valid modes
 * are #ELM_LIST_LIMIT, #ELM_LIST_SCROLL, and #ELM_LIST_COMPRESS. The default is
 * ELM_LIST_SCROLL. This mode means that if items are too wide to fit,
 * the scroller will scroll horizontally. Otherwise items are expanded
 * to fill the width of the viewport of the scroller. If it is
 * ELM_LIST_LIMIT, items will be expanded to the viewport width and
 * limited to that size. If it is ELM_LIST_COMPRESS, the item width will be
 * fixed (restricted to a minimum of) to the list width when calculating its
 * size in order to allow the height to be calculated based on it. This allows,
 * for instance, text block to wrap lines if the Edje part is configured with
 * "text.min: 0 1".
 * @note ELM_LIST_COMPRESS will make list resize slower as it will have to
 *       recalculate every item height again whenever the list width
 *       changes!
 * @note Homogeneous mode is for that all items in the genlist same
 *       width/height. With ELM_LIST_COMPRESS, it makes genlist items to fast
 *       initializing. However there's no sub-objects in genlist which can be
 *       on the flying resizable (such as TEXTBLOCK). If then, some dynamic
 *       resizable objects in genlist would not diplayed properly.
 *
 * @see elm_genlist_mode_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_mode_set(Evas_Object *obj, Elm_List_Mode mode);

/**
 * Gets the horizontal stretching mode.
 *
 * @param obj The genlist object
 * @return The mode to use
 * (#ELM_LIST_LIMIT, #ELM_LIST_SCROLL)
 *
 * @see elm_genlist_mode_set()
 *
 * @ingroup Genlist
 */
EAPI Elm_List_Mode                 elm_genlist_mode_get(const Evas_Object *obj);

/**
 * Enable/disable horizontal and vertical bouncing effect.
 *
 * @param obj The genlist object
 * @param h_bounce Allow bounce horizontally (@c EINA_TRUE = on, @c
 * EINA_FALSE = off). Default is @c EINA_FALSE.
 * @param v_bounce Allow bounce vertically (@c EINA_TRUE = on, @c
 * EINA_FALSE = off). Default is @c EINA_TRUE.
 *
 * This will enable or disable the scroller bouncing effect for the
 * genlist. See elm_scroller_bounce_set() for details.
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @see elm_scroller_bounce_set()
 * @see elm_genlist_bounce_get()
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void          elm_genlist_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get whether the horizontal and vertical bouncing effect is enabled.
 *
 * @param obj The genlist object
 * @param h_bounce Pointer to a bool to receive if the bounce horizontally
 * option is set.
 * @param v_bounce Pointer to a bool to receive if the bounce vertically
 * option is set.
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @see elm_scroller_bounce_get()
 * @see elm_genlist_bounce_set()
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void          elm_genlist_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Append a new item in a given genlist widget.
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param parent The parent item, or NULL if none
 * @param type Item type
 * @param func Convenience function called when the item is selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or @c NULL if not possible
 *
 * This adds the given item to the end of the list or the end of
 * the children list if the @p parent is given.
 *
 * @see elm_genlist_item_prepend()
 * @see elm_genlist_item_insert_before()
 * @see elm_genlist_item_insert_after()
 * @see elm_object_item_del()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_append(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Object_Item *parent, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

/**
 * Prepend a new item in a given genlist widget.
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param parent The parent item, or NULL if none
 * @param type Item type
 * @param func Convenience function called when the item is selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible
 *
 * This adds an item to the beginning of the list or beginning of the
 * children of the parent if given.
 *
 * @see elm_genlist_item_append()
 * @see elm_genlist_item_insert_before()
 * @see elm_genlist_item_insert_after()
 * @see elm_object_item_del()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_prepend(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Object_Item *parent, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

/**
 * Insert an item before another in a genlist widget
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param parent The parent item, or NULL if none
 * @param before The item to place this new one before.
 * @param type Item type
 * @param func Convenience function called when the item is selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or @c NULL if not possible
 *
 * This inserts an item before another in the list. It will be in the
 * same tree level or group as the item it is inserted before.
 *
 * @see elm_genlist_item_append()
 * @see elm_genlist_item_prepend()
 * @see elm_genlist_item_insert_after()
 * @see elm_object_item_del()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_insert_before(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Object_Item *parent, Elm_Object_Item *before, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

/**
 * Insert an item after another in a genlist widget
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param parent The parent item, or NULL if none
 * @param after The item to place this new one after.
 * @param type Item type
 * @param func Convenience function called when the item is selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or @c NULL if not possible
 *
 * This inserts an item after another in the list. It will be in the
 * same tree level or group as the item it is inserted after.
 *
 * @see elm_genlist_item_append()
 * @see elm_genlist_item_prepend()
 * @see elm_genlist_item_insert_before()
 * @see elm_object_item_del()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_insert_after(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Object_Item *parent, Elm_Object_Item *after, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

/**
 * Insert a new item into the sorted genlist object
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param parent The parent item, or NULL if none
 * @param type Item type
 * @param comp The function called for the sort
 * @param func Convenience function called when item selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible
 *
 * This inserts an item in the genlist based on user defined comparison
 * function. The two arguments passed to the function @p func are genlist item
 * handles to compare.
 *
 * @see elm_genlist_item_append()
 * @see elm_genlist_item_prepend()
 * @see elm_genlist_item_insert_after()
 * @see elm_object_item_del()

 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_sorted_insert(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Object_Item *parent, Elm_Genlist_Item_Type type, Eina_Compare_Cb comp, Evas_Smart_Cb func, const void *func_data);

/* operations to retrieve existing items */
/**
 * Get the selected item in the genlist.
 *
 * @param obj The genlist object
 * @return The selected item, or NULL if none is selected.
 *
 * This gets the selected item in the list (if multi-selection is enabled, only
 * the item that was first selected in the list is returned - which is not very
 * useful, so see elm_genlist_selected_items_get() for when multi-selection is
 * used).
 *
 * If no item is selected, NULL is returned.
 *
 * @see elm_genlist_selected_items_get()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_selected_item_get(const Evas_Object *obj);

/**
 * Get a list of selected items in the genlist.
 *
 * @param obj The genlist object
 * @return The list of selected items, or NULL if none are selected.
 *
 * It returns a list of the selected items. This list pointer is only valid so
 * long as the selection doesn't change (no items are selected or unselected, or
 * unselected implicitly by deletion). The list contains genlist items
 * pointers. The order of the items in this list is the order which they were
 * selected, i.e. the first item in this list is the first item that was
 * selected, and so on.
 *
 * @note If not in multi-select mode, consider using function
 * elm_genlist_selected_item_get() instead.
 *
 * @see elm_genlist_multi_select_set()
 * @see elm_genlist_selected_item_get()
 *
 * @ingroup Genlist
 */
EAPI const Eina_List              *elm_genlist_selected_items_get(const Evas_Object *obj);

/**
 * Get a list of realized items in genlist
 *
 * @param obj The genlist object
 * @return The list of realized items, nor NULL if none are realized.
 *
 * This returns a list of the realized items in the genlist. The list
 * contains genlist item pointers. The list must be freed by the
 * caller when done with eina_list_free(). The item pointers in the
 * list are only valid so long as those items are not deleted or the
 * genlist is not deleted.
 *
 * @see elm_genlist_realized_items_update()
 *
 * @ingroup Genlist
 */
EAPI Eina_List                    *elm_genlist_realized_items_get(const Evas_Object *obj);

/**
 * Get the first item in the genlist
 *
 * This returns the first item in the list.
 *
 * @param obj The genlist object
 * @return The first item, or NULL if none
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_first_item_get(const Evas_Object *obj);

/**
 * Get the last item in the genlist
 *
 * This returns the last item in the list.
 *
 * @return The last item, or NULL if none
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_last_item_get(const Evas_Object *obj);

/**
 * Set the scrollbar policy
 *
 * @param obj The genlist object
 * @param policy_h Horizontal scrollbar policy.
 * @param policy_v Vertical scrollbar policy.
 *
 * This sets the scrollbar visibility policy for the given genlist
 * scroller. #ELM_SCROLLER_POLICY_AUTO means the scrollbar is
 * made visible if it is needed, and otherwise kept hidden. #ELM_SCROLLER_POLICY_ON
 * turns it on all the time, and #ELM_SCROLLER_POLICY_OFF always keeps it off.
 * This applies respectively for the horizontal and vertical scrollbars.
 * Default is #ELM_SCROLLER_POLICY_AUTO
 *
 * @deprecated Use elm_scroller_policy_set() instead.
 *
 * @see elm_scroller_policy_set()
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void          elm_genlist_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * Get the scrollbar policy
 *
 * @param obj The genlist object
 * @param policy_h Pointer to store the horizontal scrollbar policy.
 * @param policy_v Pointer to store the vertical scrollbar policy.
 *
 * @deprecated Use elm_scroller_policy_get() instead.
 *
 * @see elm_scroller_policy_get()
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void          elm_genlist_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

/**
 * Get the @b next item in a genlist widget's internal list of items,
 * given a handle to one of those items.
 *
 * @param it The genlist item to fetch next from
 * @return The item after @p item, or @c NULL if there's none (and
 * on errors)
 *
 * This returns the item placed after the @p item, on the container
 * genlist.
 *
 * @see elm_genlist_item_prev_get()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_next_get(const Elm_Object_Item *it);

/**
 * Get the @b previous item in a genlist widget's internal list of items,
 * given a handle to one of those items.
 *
 * @param it The genlist item to fetch previous from
 * @return The item before @p item, or @c NULL if there's none (and
 * on errors)
 *
 * This returns the item placed before the @p item, on the container
 * genlist.
 *
 * @see elm_genlist_item_next_get()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_prev_get(const Elm_Object_Item *it);

/**
 * Set whether a given genlist item is selected or not
 *
 * @param it The item
 * @param selected Use @c EINA_TRUE, to make it selected, @c
 * EINA_FALSE to make it unselected
 *
 * This sets the selected state of an item. If multi selection is
 * not enabled on the containing genlist and @p selected is @c
 * EINA_TRUE, any other previously selected items will get
 * unselected in favor of this new one.
 *
 * @see elm_genlist_item_selected_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * Get whether a given genlist item is selected or not
 *
 * @param it The item
 * @return @c EINA_TRUE, if it's selected, @c EINA_FALSE otherwise
 *
 * @see elm_genlist_item_selected_set() for more details
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_item_selected_get(const Elm_Object_Item *it);

/**
 * Show the portion of a genlist's internal list containing a given
 * item, immediately.
 *
 * @param it The item to display
 * @param type The position to bring in, the given item to.
 *             @ref Elm_Genlist_Item_Scrollto_Type
 *
 * This causes genlist to jump to the given item @p it and show it (by
 * jumping to that position), if it is not fully visible.
 *
 * @see elm_genlist_item_bring_in()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_show(Elm_Object_Item *it, Elm_Genlist_Item_Scrollto_Type type);

/**
 * Animatedly bring in, to the visible are of a genlist, a given
 * item on it.
 *
 * @param it The item to display
 * @param type The position to bring in, the given item to.
 *             @ref Elm_Genlist_Item_Scrollto_Type
 *
 * This causes genlist to jump to the given item @p it and show it (by
 * animatedly scrolling), if it is not fully visible.
 * This may use animation and take a some time to do so.
 *
 * @see elm_genlist_item_show()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_bring_in(Elm_Object_Item *it, Elm_Genlist_Item_Scrollto_Type type);

/**
 * Update all the contents of an item
 *
 * @param it The item
 *
 * This updates an item by calling all the item class functions again
 * to get the contents, texts and states. Use this when the original
 * item data has changed and the changes are desired to be reflected.
 *
 * Use elm_genlist_realized_items_update() to update all already realized
 * items.
 *
 * @note This also updates internal genlist item object(edje_object as of now).
 * So when this is called between mouse down and mouse up, mouse up event will
 * be ignored because edje_object is deleted and created again by this API. If
 * you want to avoid this, please use @ref elm_genlist_item_fields_update.
 *
 * @see elm_genlist_realized_items_update()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_update(Elm_Object_Item *it);

/**
 * Update the item class of an item
 *
 * @param it The item
 * @param itc The item class for the item
 *
 * This sets another class of the item, changing the way that it is
 * displayed. After changing the item class, elm_genlist_item_update() is
 * called on the item @p it.
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_item_class_update(Elm_Object_Item *it, const Elm_Genlist_Item_Class *itc);

/**
 * Get the Genlist Item class for the given Genlist Item.
 *
 * @param it The genlist item
 *
 * This returns the Genlist_Item_Class for the given item. It can be used to
 * examine the function pointers and item_style.
 *
 * @ingroup Genlist
 */
EAPI const Elm_Genlist_Item_Class *elm_genlist_item_item_class_get(const Elm_Object_Item *it);

/**
 * Get the index of the item. It is only valid once displayed.
 *
 * @param it a genlist item
 * @return the position inside the list of item.
 *
 * @ingroup Genlist
 */
EAPI int                           elm_genlist_item_index_get(const Elm_Object_Item *it);

/**
 * Update the contents of all realized items.
 *
 * @param obj The genlist object.
 *
 * This updates all realized items by calling all the item class functions again
 * to get the contents, texts and states. Use this when the original
 * item data has changed and the changes are desired to be reflected.
 *
 * To update just one item, use elm_genlist_item_update().
 *
 * @see elm_genlist_realized_items_get()
 * @see elm_genlist_item_update()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_realized_items_update(Evas_Object *obj);

/**
 * Return how many items are currently in a list
 *
 * @param obj The list
 * @return The total number of list items in the list
 *
 * This behavior is O(1) and includes items which may or may not be realized.
 *
 * @ingroup Genlist
 */
EAPI unsigned int elm_genlist_items_count(const Evas_Object *obj);

/**
 * Create a new genlist item class in a given genlist widget.
 *
 * @return New allocated a genlist item class.
 *
 * This adds genlist item class for the genlist widget. When adding an item,
 * genlist_item_{append, prepend, insert} function needs item class of the item.
 * Given callback parameters are used at retrieving {text, content} of
 * added item. Set as NULL if it's not used.
 * If there's no available memory, return can be NULL.
 *
 * @see elm_genlist_item_class_free()
 * @see elm_genlist_item_append()
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item_Class *elm_genlist_item_class_new(void);

/**
 * Remove an item class in a given genlist widget.
 *
 * @param itc The itc to be removed.
 *
 * This removes item class from the genlist widget.
 * Whenever it has no more references to it, item class is going to be freed.
 * Otherwise it just decreases its reference count.
 *
 * @see elm_genlist_item_class_new()
 * @see elm_genlist_item_class_ref()
 * @see elm_genlist_item_class_unref()
 *
 * @ingroup Genlist
 */
EAPI void elm_genlist_item_class_free(Elm_Genlist_Item_Class *itc);

/**
 * Increments object reference count for the item class.
 *
 * @param itc The given item class object to reference
 *
 * This API just increases its reference count for item class management.
 *
 * @see elm_genlist_item_class_unref()
 *
 * @ingroup Genlist
 */
EAPI void elm_genlist_item_class_ref(Elm_Genlist_Item_Class *itc);

/**
 * Decrements object reference count for the item class.
 *
 * @param itc The given item class object to reference
 *
 * This API just decreases its reference count for item class management.
 * Reference count can't be less than 0.
 *
 * @see elm_genlist_item_class_ref()
 * @see elm_genlist_item_class_free()
 *
 * @ingroup Genlist
 */
EAPI void elm_genlist_item_class_unref(Elm_Genlist_Item_Class *itc);

/**
 * Set the text to be shown in a given genlist item's tooltips.
 *
 * @param it The genlist item
 * @param text The text to set in the content
 *
 * This call will setup the text to be used as tooltip to that item
 * (analogous to elm_object_tooltip_text_set(), but being item
 * tooltips with higher precedence than object tooltips). It can
 * have only one tooltip at a time, so any previous tooltip data
 * will get removed.
 *
 * In order to set a content or something else as a tooltip, look at
 * elm_genlist_item_tooltip_content_cb_set().
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * Set the content to be shown in a given genlist item's tooltips
 *
 * @param it The genlist item.
 * @param func The function returning the tooltip contents.
 * @param data What to provide to @a func as callback data/context.
 * @param del_cb Called when data is not needed anymore, either when
 *        another callback replaces @p func, the tooltip is unset with
 *        elm_genlist_item_tooltip_unset() or the owner @p item
 *        dies. This callback receives as its first parameter the
 *        given @p data, being @c event_info the item handle.
 *
 * This call will setup the tooltip's contents to @p item
 * (analogous to elm_object_tooltip_content_cb_set(), but being
 * item tooltips with higher precedence than object tooltips). It
 * can have only one tooltip at a time, so any previous tooltip
 * content will get removed. @p func (with @p data) will be called
 * every time Elementary needs to show the tooltip and it should
 * return a valid Evas object, which will be fully managed by the
 * tooltip system, getting deleted when the tooltip is gone.
 *
 * In order to set just a text as a tooltip, look at
 * elm_genlist_item_tooltip_text_set().
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset a tooltip from a given genlist item
 *
 * @param it genlist item to remove a previously set tooltip from.
 *
 * This call removes any tooltip set on @p item. The callback
 * provided as @c del_cb to
 * elm_genlist_item_tooltip_content_cb_set() will be called to
 * notify it is not used anymore (and have resources cleaned, if
 * need be).
 *
 * @see elm_genlist_item_tooltip_content_cb_set()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given genlist item's tooltip.
 *
 * @param it genlist item with tooltip set
 * @param style the <b>theme style</b> to use on tooltips (e.g. @c
 * "default", @c "transparent", etc)
 *
 * Tooltips can have <b>alternate styles</b> to be displayed on,
 * which are defined by the theme set on Elementary. This function
 * works analogously as elm_object_tooltip_style_set(), but here
 * applied only to genlist item objects. The default style for
 * tooltips is @c "default".
 *
 * @note before you set a style you should define a tooltip with
 *       elm_genlist_item_tooltip_content_cb_set() or
 *       elm_genlist_item_tooltip_text_set()
 *
 * @see elm_genlist_item_tooltip_style_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style set a given genlist item's tooltip.
 *
 * @param it genlist item with tooltip already set on.
 * @return style the theme style in use, which defaults to
 *         "default". If the object does not have a tooltip set,
 *         then @c NULL is returned.
 *
 * @see elm_genlist_item_tooltip_style_set() for more details
 *
 * @ingroup Genlist
 */
EAPI const char                   *elm_genlist_item_tooltip_style_get(const Elm_Object_Item *it);

/**
 * @brief Disable size restrictions on an object's tooltip
 * @param it The tooltip's anchor object
 * @param disable If EINA_TRUE, size restrictions are disabled
 * @return EINA_FALSE on failure, EINA_TRUE on success
 *
 * This function allows a tooltip to expand beyond its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                     elm_genlist_item_tooltip_window_mode_set(Elm_Object_Item *it, Eina_Bool disable);

/**
 * @brief Retrieve size restriction state of an object's tooltip
 * @param it The tooltip's anchor object
 * @return If EINA_TRUE, size restrictions are disabled
 *
 * This function returns whether a tooltip is allowed to expand beyond
 * its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                     elm_genlist_item_tooltip_window_mode_get(const Elm_Object_Item *it);

/**
 * Set the type of mouse pointer/cursor decoration to be shown,
 * when the mouse pointer is over the given genlist widget item
 *
 * @param it genlist item to customize cursor on
 * @param cursor the cursor type's name
 *
 * This function works analogously as elm_object_cursor_set(), but
 * here the cursor's changing area is restricted to the item's
 * area, and not the whole widget's. Note that that item cursors
 * have precedence over widget cursors, so that a mouse over @p
 * item will always show cursor @p type.
 *
 * If this function is called twice for an object, a previously set
 * cursor will be unset on the second call.
 *
 * @see elm_object_cursor_set()
 * @see elm_genlist_item_cursor_get()
 * @see elm_genlist_item_cursor_unset()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_cursor_set(Elm_Object_Item *it, const char *cursor);

/**
 * Get the type of mouse pointer/cursor decoration set to be shown,
 * when the mouse pointer is over the given genlist widget item
 *
 * @param it genlist item with custom cursor set
 * @return the cursor type's name or @c NULL, if no custom cursors
 * were set to @p item (and on errors)
 *
 * @see elm_object_cursor_get()
 * @see elm_genlist_item_cursor_set() for more details
 * @see elm_genlist_item_cursor_unset()
 *
 * @ingroup Genlist
 */
EAPI const char                   *elm_genlist_item_cursor_get(const Elm_Object_Item *it);

/**
 * Unset any custom mouse pointer/cursor decoration set to be
 * shown, when the mouse pointer is over the given genlist widget
 * item, thus making it show the @b default cursor again.
 *
 * @param it a genlist item
 *
 * Use this call to undo any custom settings on this item's cursor
 * decoration, bringing it back to defaults (no custom style set).
 *
 * @see elm_object_cursor_unset()
 * @see elm_genlist_item_cursor_set() for more details
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_cursor_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given custom cursor set for a
 * genlist item.
 *
 * @param it genlist item with custom cursor set
 * @param style the <b>theme style</b> to use (e.g. @c "default",
 * @c "transparent", etc)
 *
 * This function only makes sense when one is using custom mouse
 * cursor decorations <b>defined in a theme file</b> , which can
 * have, given a cursor name/type, <b>alternate styles</b> on
 * it. It works analogously as elm_object_cursor_style_set(), but
 * here applied only to genlist item objects.
 *
 * @warning Before you set a cursor style you should have defined a
 *       custom cursor previously on the item, with
 *       elm_genlist_item_cursor_set()
 *
 * @see elm_genlist_item_cursor_engine_only_set()
 * @see elm_genlist_item_cursor_style_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_cursor_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the current @b style set for a given genlist item's custom
 * cursor
 *
 * @param it genlist item with custom cursor set.
 * @return style the cursor style in use. If the object does not
 *         have a cursor set, then @c NULL is returned.
 *
 * @see elm_genlist_item_cursor_style_set() for more details
 *
 * @ingroup Genlist
 */
EAPI const char                   *elm_genlist_item_cursor_style_get(const Elm_Object_Item *it);

/**
 * Set if the (custom) cursor for a given genlist item should be
 * searched in its theme, also, or should only rely on the
 * rendering engine.
 *
 * @param it item with custom (custom) cursor already set on
 * @param engine_only Use @c EINA_TRUE to have cursors looked for
 * only on those provided by the rendering engine, @c EINA_FALSE to
 * have them searched on the widget's theme, as well.
 *
 * @note This call is of use only if you've set a custom cursor
 * for genlist items, with elm_genlist_item_cursor_set().
 *
 * @note By default, cursors will only be looked for between those
 * provided by the rendering engine.
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_cursor_engine_only_set(Elm_Object_Item *it, Eina_Bool engine_only);

/**
 * Get if the (custom) cursor for a given genlist item is being
 * searched in its theme, also, or is only relying on the rendering
 * engine.
 *
 * @param it a genlist item
 * @return @c EINA_TRUE, if cursors are being looked for only on
 * those provided by the rendering engine, @c EINA_FALSE if they
 * are being searched on the widget's theme, as well.
 *
 * @see elm_genlist_item_cursor_engine_only_set(), for more details
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_item_cursor_engine_only_get(const Elm_Object_Item *it);

/**
 * Enable/disable homogeneous mode.
 *
 * @param obj The genlist object
 * @param homogeneous Assume the items within the genlist are of the
 * same height and width (EINA_TRUE = on, EINA_FALSE = off). Default is @c
 * EINA_FALSE.
 *
 * This will enable the homogeneous mode where items are of the same
 * height and width so that genlist may do the lazy-loading at its
 * maximum (which increases the performance for scrolling the list).
 * In the normal mode, genlist will pre-calculate all the items' sizes even
 * though they are not in use. So items' callbacks are called many times than
 * expected. But homogeneous mode will skip the item size pre-calculation
 * process so items' callbacks are called only when the item is needed.
 *
 * @see elm_genlist_mode_set()
 * @see elm_genlist_homogeneous_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_homogeneous_set(Evas_Object *obj, Eina_Bool homogeneous);

/**
 * Get whether the homogeneous mode is enabled.
 *
 * @param obj The genlist object
 * @return Assume the items within the genlist are of the same height
 * and width (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @see elm_genlist_homogeneous_set()
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_homogeneous_get(const Evas_Object *obj);

/**
 * Set the maximum number of items within an item block
 *
 * @param obj The genlist object
 * @param count Maximum number of items within an item block. Default is 32.
 *
 * This will configure the block count to tune to the target with particular
 * performance matrix.
 *
 * A block of objects will be used to reduce the number of operations due to
 * many objects in the screen. It can determine the visibility, or if the
 * object has changed, it theme needs to be updated, etc. doing this kind of
 * calculation to the entire block, instead of per object.
 *
 * The default value for the block count is enough for most lists, so unless
 * you know you will have a lot of objects visible in the screen at the same
 * time, don't try to change this.
 *
 * @see elm_genlist_block_count_get()
 * @see @ref Genlist_Implementation
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_block_count_set(Evas_Object *obj, int count);

/**
 * Get the maximum number of items within an item block
 *
 * @param obj The genlist object
 * @return Maximum number of items within an item block
 *
 * @see elm_genlist_block_count_set()
 *
 * @ingroup Genlist
 */
EAPI int                           elm_genlist_block_count_get(const Evas_Object *obj);

/**
 * Set the timeout in seconds for the longpress event.
 *
 * @param obj The genlist object
 * @param timeout timeout in seconds. Default is elm config value(1.0)
 *
 * This option will change how long it takes to send an event "longpressed"
 * after the mouse down signal is sent to the list. If this event occurs, no
 * "clicked" event will be sent.
 *
 * @warning If you set the longpress timeout value with this API, your genlist
 * will not be affected by the longpress value of elementary config value
 * later.
 *
 * @see elm_genlist_longpress_timeout_set()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_longpress_timeout_set(Evas_Object *obj, double timeout);

/**
 * Get the timeout in seconds for the longpress event.
 *
 * @param obj The genlist object
 * @return timeout in seconds
 *
 * @see elm_genlist_longpress_timeout_get()
 *
 * @ingroup Genlist
 */
EAPI double                        elm_genlist_longpress_timeout_get(const Evas_Object *obj);

/**
 * Get the item that is at the x, y canvas coords.
 *
 * @param obj The genlist object.
 * @param x The input x coordinate
 * @param y The input y coordinate
 * @param posret The position relative to the item returned here
 * @return The item at the coordinates or NULL if none
 *
 * This returns the item at the given coordinates (which are canvas
 * relative, not object-relative). If an item is at that coordinate,
 * that item handle is returned, and if @p posret is not NULL, the
 * integer pointed to is set to a value of -1, 0 or 1, depending if
 * the coordinate is on the upper portion of that item (-1), on the
 * middle section (0) or on the lower part (1). If NULL is returned as
 * an item (no item found there), then posret may indicate -1 or 1
 * based if the coordinate is above or below all items respectively in
 * the genlist.
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_at_xy_item_get(const Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *posret);

/**
 * Get the parent item of the given item
 *
 * @param it The item
 * @return The parent of the item or @c NULL if it has no parent.
 *
 * This returns the item that was specified as parent of the item @p it on
 * elm_genlist_item_append() and insertion related functions.
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_parent_get(const Elm_Object_Item *it);

/**
 * Remove all sub-items (children) of the given item
 *
 * @param it The item
 *
 * This removes all items that are children (and their descendants) of the
 * given item @p it.
 *
 * @see elm_genlist_clear()
 * @see elm_object_item_del()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_subitems_clear(Elm_Object_Item *it);

/**
 * Sets the expanded state of an item.
 *
 * @param it The item
 * @param expanded The expanded state (@c EINA_TRUE expanded, @c EINA_FALSE not expanded).
 *
 * This function flags the item of type #ELM_GENLIST_ITEM_TREE as
 * expanded or not.
 *
 * The theme will respond to this change visually, and a signal "expanded" or
 * "contracted" will be sent from the genlist with a pointer to the item that
 * has been expanded/contracted.
 *
 * Calling this function won't show or hide any child of this item (if it is
 * a parent). You must manually delete and create them on the callbacks of
 * the "expanded" or "contracted" signals.
 *
 * @see elm_genlist_item_expanded_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_expanded_set(Elm_Object_Item *it, Eina_Bool expanded);

/**
 * Get the expanded state of an item
 *
 * @param it The item
 * @return The expanded state
 *
 * This gets the expanded state of an item.
 *
 * @see elm_genlist_item_expanded_set()
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_item_expanded_get(const Elm_Object_Item *it);

/**
 * Get the depth of expanded item
 *
 * @param it The genlist item object
 * @return The depth of expanded item
 *
 * @ingroup Genlist
 */
EAPI int                           elm_genlist_item_expanded_depth_get(const Elm_Object_Item *it);

/**
 * Unset all contents fetched by the item class
 *
 * @param it The item
 * @param l The contents list to return
 *
 * This instructs genlist to release references to contents in the item,
 * meaning that they will no longer be managed by genlist and are
 * floating "orphans" that can be re-used elsewhere if the user wants
 * to.
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_all_contents_unset(Elm_Object_Item *it, Eina_List **l);

/**
 * Promote an item to the top of the list
 *
 * @param it The item
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_promote(Elm_Object_Item *it);

/**
 * Demote an item to the end of the list
 *
 * @param it The item
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_demote(Elm_Object_Item *it);

/**
 * Update the part of an item
 *
 * @param it The item
 * @param parts The name of item's part
 * @param itf The type of item's part type
 *
 * This updates an item's part by calling item's fetching functions again
 * to get the contents, texts and states. Use this when the original
 * item data has changed and the changes are desired to be reflected.
 * Second parts argument is used for globbing to match '*', '?', and '.'
 * It can be used at updating multi fields.
 *
 * Use elm_genlist_realized_items_update() to update an item's all
 * property.
 *
 * @see elm_genlist_item_update()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_fields_update(Elm_Object_Item *it, const char *parts, Elm_Genlist_Item_Field_Type itf);

/**
 * Activate a genlist mode on an item
 *
 * @param it The genlist item
 * @param decorate_it_type Mode name
 * @param decorate_it_set Boolean to define set or unset mode.

 *
 * A genlist mode is a different way of selecting an item. Once a mode is
 * activated on an item, any other selected item is immediately unselected.
 * This feature provides an easy way of implementing a new kind of animation
 * for selecting an item, without having to entirely rewrite the item style
 * theme. However, the elm_genlist_selected_* API can't be used to get what
 * item is activate for a mode.
 *
 * The current item style will still be used, but applying a genlist mode to
 * an item will select it using a different kind of animation.
 *
 * The current active item for a mode can be found by
 * elm_genlist_decorated_item_get().
 *
 * The characteristics of genlist mode are:
 * - Only one mode can be active at any time, and for only one item.
 * - Genlist handles deactivating other items when one item is activated.
 * - A mode is defined in the genlist theme (edc), and more modes can easily
 *   be added.
 * - A mode style and the genlist item style are different things. They
 *   can be combined to provide a default style to the item, with some kind
 *   of animation for that item when the mode is activated.
 *
 * When a mode is activated on an item, a new view for that item is created.
 * The theme of this mode defines the animation that will be used to transit
 * the item from the old view to the new view. This second (new) view will be
 * active for that item while the mode is active on the item, and will be
 * destroyed after the mode is totally deactivated from that item.
 *
 * @see elm_genlist_mode_get()
 * @see elm_genlist_decorated_item_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_decorate_mode_set(Elm_Object_Item *it, const char *decorate_it_type, Eina_Bool decorate_it_set);

/**
 * Get the item's decorate mode.
 *
 * @param it The genlist item
 *
 * This function just returns the name of the item's decorate mode.
 *
 * @see elm_genlist_item_decorate_mode_set()
 * @see elm_genlist_decorated_item_get()
 *
 * @ingroup Genlist
 */
EAPI const char                   *elm_genlist_item_decorate_mode_get(const Elm_Object_Item *it);

/**
 * Get active genlist mode item
 *
 * @param obj The genlist object
 * @return The active item for that current mode. Or @c NULL if no item is
 * activated with any mode.
 *
 * This function returns the item that was activated with a mode, by the
 * function elm_genlist_item_decorate_mode_set().
 *
 * @see elm_genlist_item_decorate_mode_set()
 * @see elm_genlist_mode_get()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item              *elm_genlist_decorated_item_get(const Evas_Object *obj);

/**
 * Set reorder mode
 *
 * @param obj The genlist object
 * @param reorder_mode The reorder mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * After turning on the reorder mode, longpress on normal item will trigger
 * reordering of the item. You can move the item up and down. However, reorder
 * does not work with group item.
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_reorder_mode_set(Evas_Object *obj, Eina_Bool reorder_mode);

/**
 * Get the reorder mode
 *
 * @param obj The genlist object
 * @return The reorder mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_reorder_mode_get(const Evas_Object *obj);

/**
 * Get the Item's Type
 *
 * @param it The genlist item
 * @return The item type.
 *
 * This function returns the item's type. Normally the item's type.
 * If it failed, return value is ELM_GENLIST_ITEM_MAX
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item_Type        elm_genlist_item_type_get(const Elm_Object_Item *it);

/**
 * Set Genlist decorate mode
 *
 * This sets Genlist decorate mode to all items.
 *
 * @param obj The Genlist object
 * @param decorated The decorate mode status
 * (EINA_TRUE = decorate mode, EINA_FALSE = normal mode
 *
 * @ingroup Genlist
 */
EAPI void               elm_genlist_decorate_mode_set(Evas_Object *obj, Eina_Bool decorated);

/**
 * Get Genlist decorate mode
 *
 * @param obj The genlist object
 * @return The decorate mode status
 * (EINA_TRUE = decorate mode, EINA_FALSE = normal mode
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool          elm_genlist_decorate_mode_get(const Evas_Object *obj);

/**
 * Set the flip state of a given genlist item.
 *
 * @param it The genlist item object
 * @param flip The flip mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * This function sets the flip state of a given genlist item.
 * Flip mode overrides current item object.
 * It can be used for on-the-fly item replace.
 * Flip mode can be used with/without decorate mode.
 *
 * @see elm_genlist_item_flip_get()
 *
 * @ingroup Genlist
 */

EAPI void elm_genlist_item_flip_set(Elm_Object_Item *it, Eina_Bool flip);

/**
 * Get the flip state of a given genlist item.
 *
 * @param it The genlist item object
 *
 * This function returns the flip state of a given genlist item.
 * If the parameter is invalid, it returns EINA_FALSE.
 *
 * @see elm_genlist_item_flip_set()
 *
 * @ingroup Genlist
 */

EAPI Eina_Bool elm_genlist_item_flip_get(const Elm_Object_Item *it);

/**
 * Set Genlist tree effect
 *
 * @param obj The genlist object
 * @param enabled The tree effect status
 * (EINA_TRUE = enabled, EINA_FALSE = disabled
 *
 * @ingroup Genlist
 */
EAPI void               elm_genlist_tree_effect_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Get Genlist tree effect
 *
 * @param obj The genlist object
 * @return The tree effect status
 * (EINA_TRUE = enabled, EINA_FALSE = disabled
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool          elm_genlist_tree_effect_enabled_get(const Evas_Object *obj);

/**
 * Set the genlist select mode.
 *
 * @param obj The genlist object
 * @param mode The select mode
 *
 * elm_genlist_select_mode_set() changes item select mode in the genlist widget.
 * - ELM_OBJECT_SELECT_MODE_DEFAULT : Items will only call their selection func and
 *      callback when first becoming selected. Any further clicks will
 *      do nothing, unless you set always select mode.
 * - ELM_OBJECT_SELECT_MODE_ALWAYS :  This means that, even if selected,
 *      every click will make the selected callbacks be called.
 * - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the ability to select items
 *      entirely and they will neither appear selected nor call selected
 *      callback functions.
 *
 * @see elm_genlist_select_mode_get()
 *
 * @ingroup Genlist
 */
EAPI void elm_genlist_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode mode);

/**
 * Get the genlist select mode.
 *
 * @param obj The genlist object
 * @return The select mode
 * (If getting mode is failed, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * @see elm_genlist_select_mode_set()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Select_Mode elm_genlist_select_mode_get(const Evas_Object *obj);

/**
 * Set whether the genlist items' should be highlighted when item selected.
 *
 * @param obj The genlist object.
 * @param highlight @c EINA_TRUE to enable highlighting or @c EINA_FALSE to
 * disable it.
 *
 * This will turn on/off the highlight effect when item selection and
 * they will, or will not highlighted. The selected and clicked
 * callback functions will still be called.
 *
 * Highlight is enabled by default.
 *
 * @see elm_genlist_highlight_mode_get().
 *
 * @ingroup Genlist
 */
EAPI void               elm_genlist_highlight_mode_set(Evas_Object *obj, Eina_Bool highlight);

/**
 * Get whether the genlist items' should be highlighted when item selected.
 *
 * @param obj The genlist object.
 * @return @c EINA_TRUE means items can be highlighted. @c EINA_FALSE indicates
 * they can't. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * @see elm_genlist_highlight_mode_set() for details.
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool          elm_genlist_highlight_mode_get(const Evas_Object *obj);

/**
 * Set the genlist item's select mode.
 *
 * @param it The genlist item object
 * @param mode The select mode
 *
 * elm_genlist_select_mode_set() changes item's select mode.
 * - ELM_OBJECT_SELECT_MODE_DEFAULT : The item will only call their selection func and
 *      callback when first becoming selected. Any further clicks will
 *      do nothing, unless you set always select mode.
 * - ELM_OBJECT_SELECT_MODE_ALWAYS : This means that, even if selected,
 *      every click will make the selected callbacks be called.
 * - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the ability to select the item
 *      entirely and they will neither appear selected nor call selected
 *      callback functions.
 * - ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY : This will apply no-finger-size rule
 *      with ELM_OBJECT_SELECT_MODE_NONE. No-finger-size rule makes an item can be
 *      smaller than lower limit. Clickable objects should be bigger than
 *      human touch point device (your finger) for some touch or
 *      small screen devices. So it is enabled, the item can be shrink than
 *      predefined finger-size value. And the item will be updated.
 *
 * @see elm_genlist_item_select_mode_get()
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_select_mode_set(Elm_Object_Item *it,
                                 Elm_Object_Select_Mode mode);

/**
 * Get the genlist item's select mode.
 *
 * @param it The genlist item object
 * @return The select mode
 * (If getting mode is failed, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * @see elm_genlist_item_select_mode_set()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Select_Mode
elm_genlist_item_select_mode_get(const Elm_Object_Item *it);

/**
 * @}
 */
