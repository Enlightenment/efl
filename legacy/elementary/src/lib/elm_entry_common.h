/**
 * @typedef Elm_Text_Format
 *
 * Text Format types.
 *
 * @see elm_entry_file_set()
 */
typedef enum
{
   ELM_TEXT_FORMAT_PLAIN_UTF8,  /**< Plain UTF8 type */
   ELM_TEXT_FORMAT_MARKUP_UTF8  /**< Markup UTF8 type */
} Elm_Text_Format;

/**
 * @typedef Elm_Wrap_Type
 *
 * Line wrapping types.
 *
 * @see elm_entry_line_wrap_set()
 */
typedef enum
{
   ELM_WRAP_NONE = 0, /**< No wrap - value is zero */
   ELM_WRAP_CHAR,     /**< Char wrap - wrap between characters */
   ELM_WRAP_WORD,     /**< Word wrap - wrap in allowed wrapping points (as defined in the unicode standard) */
   ELM_WRAP_MIXED,    /**< Mixed wrap - Word wrap, and if that fails, char wrap. */
   ELM_WRAP_LAST
} Elm_Wrap_Type; /**< Type of word or character wrapping to use */

/**
 * @typedef Elm_Input_Panel_Layout
 *
 * Input panel (virtual keyboard) layout types.
 *
 * @see elm_entry_input_panel_layout_set()
 */
typedef enum
{
   ELM_INPUT_PANEL_LAYOUT_NORMAL,      /**< Default layout */
   ELM_INPUT_PANEL_LAYOUT_NUMBER,      /**< Number layout */
   ELM_INPUT_PANEL_LAYOUT_EMAIL,       /**< Email layout */
   ELM_INPUT_PANEL_LAYOUT_URL,         /**< URL layout */
   ELM_INPUT_PANEL_LAYOUT_PHONENUMBER, /**< Phone Number layout */
   ELM_INPUT_PANEL_LAYOUT_IP,          /**< IP layout */
   ELM_INPUT_PANEL_LAYOUT_MONTH,       /**< Month layout */
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY,  /**< Number Only layout */
   ELM_INPUT_PANEL_LAYOUT_INVALID,     /**< Never use this */
   ELM_INPUT_PANEL_LAYOUT_HEX,         /**< Hexadecimal layout */
   ELM_INPUT_PANEL_LAYOUT_TERMINAL,    /**< Command-line terminal layout including esc, alt, ctrl key, so on (no auto-correct, no auto-capitalization) */
   ELM_INPUT_PANEL_LAYOUT_PASSWORD,    /**< Like normal, but no auto-correct, no auto-capitalization etc. */
   ELM_INPUT_PANEL_LAYOUT_DATETIME     /**< Date and time layout @since 1.8 */
} Elm_Input_Panel_Layout; /**< Type of input panel (virtual keyboard) to use - this is a hint and may not provide exactly what is desired. */

enum
{
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_NORMAL,
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED,
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_DECIMAL,
   ELM_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL
};

/**
 * @typedef Elm_Input_Panel_Lang
 *
 * Input panel (virtual keyboard) language modes.
 *
 * @see elm_entry_input_panel_language_set()
 */
typedef enum
{
   ELM_INPUT_PANEL_LANG_AUTOMATIC,    /**< Automatic */
   ELM_INPUT_PANEL_LANG_ALPHABET      /**< Alphabet */
} Elm_Input_Panel_Lang;

/**
 * @typedef Elm_Autocapital_Type
 *
 * Autocapitalization Types.
 *
 * @see elm_entry_autocapital_type_set()
 */
typedef enum
{
   ELM_AUTOCAPITAL_TYPE_NONE,         /**< No auto-capitalization when typing */
   ELM_AUTOCAPITAL_TYPE_WORD,         /**< Autocapitalize each word typed */
   ELM_AUTOCAPITAL_TYPE_SENTENCE,     /**< Autocapitalize the start of each sentence */
   ELM_AUTOCAPITAL_TYPE_ALLCHARACTER, /**< Autocapitalize all letters */
} Elm_Autocapital_Type; /**< Choose method of auto-capitalization */

/**
 * @typedef Elm_Input_Panel_Return_Key_Type
 *
 * "Return" Key types on the input panel (virtual keyboard).
 *
 * @see elm_entry_input_panel_return_key_type_set()
 */
typedef enum
{
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT, /**< Default */
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE,    /**< Done */
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_GO,      /**< Go */
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_JOIN,    /**< Join */
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN,   /**< Login */
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_NEXT,    /**< Next */
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH,  /**< Search string or magnifier icon */
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEND,    /**< Send */
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN   /**< Sign-in @since 1.8 */
} Elm_Input_Panel_Return_Key_Type;

/**
 * @typedef Elm_Entry_Anchor_Info
 *
 * The info sent in the callback for the "anchor,clicked" signals emitted
 * by entries.
 */
typedef struct _Elm_Entry_Anchor_Info Elm_Entry_Anchor_Info;

/**
 * @struct _Elm_Entry_Anchor_Info
 *
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
 * @typedef Elm_Entry_Anchor_Hover_Info
 *
 * The info sent in the callback for "anchor,clicked" signals emitted by
 * the Anchor_Hover widget.
 */
typedef struct _Elm_Entry_Anchor_Hover_Info Elm_Entry_Anchor_Hover_Info;

/**
 * @typedef Elm_Entry_Context_Menu_Item
 *
 * Type of contextual item that can be added in to long press menu.
 * @since 1.8
 */
typedef struct _Elm_Entry_Context_Menu_Item Elm_Entry_Context_Menu_Item;

/**
 * @struct _Elm_Entry_Anchor_Hover_Info
 *
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
 * @typedef Elm_Entry_Item_Provider_Cb
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
 * @typedef Elm_Entry_Filter_Cb
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
 * @typedef Elm_Entry_Change_Info
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
 *
 * @ingroup Entry
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
 *
 * @ingroup Entry
 */
EAPI char              *elm_entry_utf8_to_markup(const char *s);



/* pre-made filters for entries */

/**
 * @typedef Elm_Entry_Filter_Limit_Size
 *
 * Data for the elm_entry_filter_limit_size() entry filter.
 */
typedef struct _Elm_Entry_Filter_Limit_Size Elm_Entry_Filter_Limit_Size;

/**
 * @struct _Elm_Entry_Filter_Limit_Size
 *
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
 *
 * @ingroup Entry
 */
EAPI void elm_entry_filter_limit_size(void *data, Evas_Object *entry, char **text);

/**
 * @typedef Elm_Entry_Filter_Accept_Set
 *
 * Data for the elm_entry_filter_accept_set() entry filter.
 */
typedef struct _Elm_Entry_Filter_Accept_Set Elm_Entry_Filter_Accept_Set;

/**
 * @struct _Elm_Entry_Filter_Accept_Set
 *
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
 * The @c accepted set takes preference, so if it is set, the filter will
 * only work based on the accepted characters, ignoring anything in the
 * @c rejected value. If @c accepted is @c NULL, then @c rejected is used.
 *
 * In both cases, the function filters by matching utf8 characters to the
 * raw markup text, so it can be used to remove formatting tags.
 *
 * This filter, like any others, does not apply when setting the entry text
 * directly with elm_object_text_set()
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_filter_accept_set(void *data, Evas_Object *entry, char **text);

/**
 * @typedef Elm_Cnp_Mode
 * Enum of entry's copy & paste policy.
 *
 * @see elm_entry_cnp_mode_set()
 * @see elm_entry_cnp_mode_get()
 */
typedef enum {
   ELM_CNP_MODE_MARKUP,   /**< copy & paste text with markup tag */
   ELM_CNP_MODE_NO_IMAGE, /**< copy & paste text without item(image) tag */
   ELM_CNP_MODE_PLAINTEXT /**< copy & paste text without markup tag */
} Elm_Cnp_Mode;

/**
 * Get the text of the contextual menu item.
 *
 * Get the text of the contextual menu item of entry.
 *
 * @param item The item to get the label
 * @return The text of contextual menu item
 *
 * @see elm_entry_context_menu_item_add()
 * @ingroup Entry
 * @since 1.8
 */
EAPI const char                  *elm_entry_context_menu_item_label_get(const Elm_Entry_Context_Menu_Item *item);

/**
 * Get the icon object of the contextual menu item.
 *
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
 * @ingroup Entry
 * @since 1.8
 */
EAPI void                         elm_entry_context_menu_item_icon_get(const Elm_Entry_Context_Menu_Item *item, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type);

