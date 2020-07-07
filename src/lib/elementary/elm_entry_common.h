#ifndef ELM_ENTRY_COMMON_H_
#define ELM_ENTRY_COMMON_H_

/**
 * @addtogroup Elm_Entry_Group
 *
 * @{
 */

/**
 * @typedef Elm_Input_Panel_Layout_Normal_Variation
 * @brief Enumeration for defining the types of Elm Input Panel Layout for normal variation.
 * @since 1.12
 */
typedef enum
{
   ELM_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_NORMAL,          /**< The plain normal layout @since 1.12 */
   ELM_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_FILENAME,        /**< Filename layout. Symbols such as '/' should be disabled. @since 1.12 */
   ELM_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_PERSON_NAME      /**< The name of a person. @since 1.12 */
} Elm_Input_Panel_Layout_Normal_Variation;

/**
 * @typedef Elm_Input_Panel_Layout_Numberonly_Variation
 * @brief Enumeration for defining the types of Elm Input Panel Layout for number only variation.
 * @since 1.8
 */
typedef enum
{
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_NORMAL,              /**< The plain normal number layout @since 1.8 */
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED,              /**< The number layout to allow a positive or negative sign at the start @since 1.8 */
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_DECIMAL,             /**< The number layout to allow decimal point to provide fractional value @since 1.8 */
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL   /**< The number layout to allow decimal point and negative sign @since 1.8 */
} Elm_Input_Panel_Layout_Numberonly_Variation;

/**
 * @typedef Elm_Input_Panel_Layout_Password_Variation
 * @brief Enumeration for defining the types of Elm Input Panel Layout for password variation.
 * @since 1.12
 */
typedef enum
{
   ELM_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NORMAL,    /**< The normal password layout @since 1.12 */
   ELM_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NUMBERONLY /**< The password layout to allow only number @since 1.12 */
} Elm_Input_Panel_Layout_Password_Variation;

/**
 * The info sent in the callback for the "anchor,clicked" signals emitted
 * by entries.
 */
typedef struct _Elm_Entry_Anchor_Info Elm_Entry_Anchor_Info;

/**
 * The info sent in the callback for the "anchor,clicked" signals emitted
 * by entries.
 */
struct _Elm_Entry_Anchor_Info
{
   const char *name; /**< The name of the anchor, as stated in its href */
   int         button; /**< The mouse button used to click on it */
   Evas_Coord  x, /**< Anchor geometry, relative to canvas */
               y, /**< Anchor geometry, relative to canvas */
               w, /**< Anchor geometry, relative to canvas */
               h; /**< Anchor geometry, relative to canvas */
};

/**
 * The info sent in the callback for "anchor,clicked" signals emitted by
 * the Anchor_Hover widget.
 */
typedef struct _Elm_Entry_Anchor_Hover_Info Elm_Entry_Anchor_Hover_Info;

/**
 * Type of contextual item that can be added in to long press menu.
 * @since 1.8
 */
typedef struct _Elm_Entry_Context_Menu_Item Elm_Entry_Context_Menu_Item;

/**
 * The info sent in the callback for "anchor,clicked" signals emitted by
 * the Anchor_Hover widget.
 */
struct _Elm_Entry_Anchor_Hover_Info
{
   const Elm_Entry_Anchor_Info *anchor_info; /**< The actual anchor info. */
   Evas_Object *hover; /**< The hover object to use for the popup */
   struct
   {
      Evas_Coord x, y, w, h;
   } hover_parent; /**< Geometry of the object used as parent by the
                        hover */
   Eina_Bool    hover_left : 1; /**< Hint indicating if there's space
                                     for content on the left side of
                                     the hover. Before calling the
                                     callback, the widget will make the
                                     necessary calculations to check
                                     which sides are fit to be set with
                                     content, based on the position the
                                     hover is activated and its distance
                                     to the edges of its parent object
                                 */
   Eina_Bool    hover_right : 1; /**< Hint indicating content fits on
                                      the right side of the hover.
                                      See @ref hover_left */
   Eina_Bool    hover_top : 1; /**< Hint indicating content fits on top
                                    of the hover. See @ref hover_left */
   Eina_Bool    hover_bottom : 1; /**< Hint indicating content fits
                                       below the hover. See @ref
                                       hover_left */
};

/**
 * This callback type is used to provide items.
 * If it returns an object handle other than NULL (it should create an
 * object to do this), then this object is used to replace the current item.
 * If not the next provider is called until one provides an item object, or the
 * default provider in entry does.
 * @param data The data specified as the last param when adding the provider
 * @param entry The entry object
 * @param text A pointer to the item href string in the text
 * @return The object to be placed in the entry like an icon, or other element
 * @see elm_entry_item_provider_append
 * @see elm_entry_item_provider_prepend
 * @see elm_entry_item_provider_remove
 */
typedef Evas_Object * (*Elm_Entry_Item_Provider_Cb)(void *data, Evas_Object * entry, const char *item);

/**
 * This callback type is used by entry filters to modify text.
 * @param data The data specified as the last param when adding the filter
 * @param entry The entry object
 * @param text A pointer to the location of the text being filtered. The type of text is always markup. This data can be modified, but any additional allocations must be managed by the user.
 * @see elm_entry_markup_filter_append
 * @see elm_entry_markup_filter_prepend
 * @see elm_entry_markup_filter_remove
 */
typedef void (*Elm_Entry_Filter_Cb)(void *data, Evas_Object *entry, char **text);

/**
 * This corresponds to Edje_Entry_Change_Info. Includes information about
 * a change in the entry.
 */
typedef Edje_Entry_Change_Info Elm_Entry_Change_Info;

/**
 * This converts a markup (HTML-like) string into UTF-8.
 *
 * The returned string is a malloc'ed buffer and it should be freed when
 * not needed anymore.
 *
 * @param s The string (in markup) to be converted
 * @return The converted string (in UTF-8). It should be freed.
 */
EAPI char              *elm_entry_markup_to_utf8(const char *s);

/**
 * This converts a UTF-8 string into markup (HTML-like).
 *
 * The returned string is a malloc'ed buffer and it should be freed when
 * not needed anymore.
 *
 * @param s The string (in UTF-8) to be converted
 * @return The converted string (in markup). It should be freed.
 *
 * For example, passing "<align=center>hello</align>&gt;" will return
 * "&lt;align=center&gt;hello&lt;/align&gt; &amp;gt;". This is useful when you
 * want to display "&" in label, entry, and some widgets which use textblock
 * internally.
 */
EAPI char              *elm_entry_utf8_to_markup(const char *s);



/* pre-made filters for entries */

/**
 * Data for the elm_entry_filter_limit_size() entry filter.
 */
typedef struct _Elm_Entry_Filter_Limit_Size Elm_Entry_Filter_Limit_Size;

/**
 * Data for the elm_entry_filter_limit_size() entry filter.
 */
struct _Elm_Entry_Filter_Limit_Size
{
   int max_char_count;      /**< The maximum number of characters allowed. */
   int max_byte_count;      /**< The maximum number of bytes allowed*/
};

/**
 * Filter inserted text based on user defined character and byte limits
 *
 * Add this filter to an entry to limit the characters that it will accept
 * based the contents of the provided #Elm_Entry_Filter_Limit_Size.
 * The function works on the UTF-8 representation of the string, converting
 * it from the set markup, thus not accounting for any format in it.
 *
 * @param[in] data The data passed to this function.
 * @param[in] entry The object.
 * @param[in,out] text The entry's text to limit size
 *
 * The user must create an #Elm_Entry_Filter_Limit_Size structure and pass
 * it as data when setting the filter. In it, it's possible to set limits
 * by character count or bytes (any of them is disabled if 0), and both can
 * be set at the same time. In that case, it first checks for characters,
 * then bytes. The #Elm_Entry_Filter_Limit_Size structure must be alive and
 * valid for as long as the entry is alive AND the elm_entry_filter_limit_size
 * filter is set.
 *
 * The function will cut the inserted text in order to allow only the first
 * number of characters that are still allowed. The cut is made in
 * characters, even when limiting by bytes, in order to always contain
 * valid ones and avoid half unicode characters making it in.
 *
 * This filter, like any others, does not apply when setting the entry text
 * directly with elm_object_text_set().
 */
EAPI void elm_entry_filter_limit_size(void *data, Evas_Object *entry, char **text);

/**
 * Data for the elm_entry_filter_accept_set() entry filter.
 */
typedef struct _Elm_Entry_Filter_Accept_Set Elm_Entry_Filter_Accept_Set;

/**
 * Data for the elm_entry_filter_accept_set() entry filter.
 */
struct _Elm_Entry_Filter_Accept_Set
{
   const char *accepted;      /**< Set of characters accepted in the entry. */
   const char *rejected;      /**< Set of characters rejected from the entry. */
};

/**
 * Filter inserted text based on accepted or rejected sets of characters
 *
 * Add this filter to an entry to restrict the set of accepted characters
 * based on the sets in the provided #Elm_Entry_Filter_Accept_Set.
 * This structure contains both accepted and rejected sets, but they are
 * mutually exclusive. This structure must be available for as long as
 * the entry is alive AND the elm_entry_filter_accept_set is being used.
 *
 * @param[in] data The data passed to this function.
 * @param[in] entry The object.
 * @param[in,out] text The entry's text to accept or reject filter
 *
 * The @c accepted set takes preference, so if it is set, the filter will
 * only work based on the accepted characters, ignoring anything in the
 * @c rejected value. If @c accepted is @c NULL, then @c rejected is used.
 *
 * In both cases, the function filters by matching utf8 characters to the
 * raw markup text, so it can be used to remove formatting tags.
 *
 * This filter, like any others, does not apply when setting the entry text
 * directly with elm_object_text_set()
 */
EAPI void                   elm_entry_filter_accept_set(void *data, Evas_Object *entry, char **text);

/**
 * Get the text of the contextual menu item of entry.
 *
 * @param item The item to get the label
 * @return The text of contextual menu item
 *
 * @see elm_entry_context_menu_item_add()
 * @since 1.8
 */
EAPI const char                  *elm_entry_context_menu_item_label_get(const Elm_Entry_Context_Menu_Item *item);

/**
 * Get the icon object packed in the contextual menu item of entry.
 *
 * @param item The item to get the icon from
 * @param icon_file The image file path on disk used for the icon or standard
 * icon name
 * @param icon_group The edje group used if @p icon_file is an edje file. NULL
 * if the icon is not an edje file
 * @param icon_type The icon type
 *
 * @see elm_entry_context_menu_item_add()
 * @since 1.8
 */
EAPI void                         elm_entry_context_menu_item_icon_get(const Elm_Entry_Context_Menu_Item *item, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type);


/**
 * @}
 */
#endif //ELM_ENTRY_COMMON_H_
