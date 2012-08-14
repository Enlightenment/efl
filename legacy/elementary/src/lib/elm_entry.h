/**
 * @defgroup Entry Entry
 * @ingroup Elementary
 *
 * @image html img/widget/entry/preview-00.png
 * @image latex img/widget/entry/preview-00.eps width=\textwidth
 * @image html img/widget/entry/preview-01.png
 * @image latex img/widget/entry/preview-01.eps width=\textwidth
 * @image html img/widget/entry/preview-02.png
 * @image latex img/widget/entry/preview-02.eps width=\textwidth
 * @image html img/widget/entry/preview-03.png
 * @image latex img/widget/entry/preview-03.eps width=\textwidth
 *
 * An entry is a convenience widget which shows a box that the user can
 * enter text into. Entries by default don't scroll, so they grow to
 * accommodate the entire text, resizing the parent window as needed. This
 * can be changed with the elm_entry_scrollable_set() function.
 *
 * They can also be single line or multi line (the default) and when set
 * to multi line mode they support text wrapping in any of the modes
 * indicated by #Elm_Wrap_Type.
 *
 * Other features include password mode, filtering of inserted text with
 * elm_entry_markup_filter_append() and related functions, inline "items" and
 * formatted markup text.
 *
 * @section entry-markup Formatted text
 *
 * The markup tags supported by the Entry are defined by the theme, but
 * even when writing new themes or extensions it's a good idea to stick to
 * a sane default, to maintain coherency and avoid application breakages.
 * Currently defined by the default theme are the following tags:
 * @li \<br\>: Inserts a line break.
 * @li \<ps\>: Inserts a paragraph separator. This is preferred over line
 * breaks.
 * @li \<tab\>: Inserts a tab.
 * @li \<em\>...\</em\>: Emphasis. Sets the @em oblique style for the
 * enclosed text.
 * @li \<b\>...\</b\>: Sets the @b bold style for the enclosed text.
 * @li \<link\>...\</link\>: Underlines the enclosed text.
 * @li \<hilight\>...\</hilight\>: Highlights the enclosed text.
 *
 * @section entry-special Special markups
 *
 * Besides those used to format text, entries support two special markup
 * tags used to insert click-able portions of text or items inlined within
 * the text.
 *
 * @subsection entry-anchors Anchors
 *
 * Anchors are similar to HTML anchors. Text can be surrounded by \<a\> and
 * \</a\> tags and an event will be generated when this text is clicked,
 * like this:
 *
 * @code
 * This text is outside <a href=anc-01>but this one is an anchor</a>
 * @endcode
 *
 * The @c href attribute in the opening tag gives the name that will be
 * used to identify the anchor and it can be any valid utf8 string.
 *
 * When an anchor is clicked, an @c "anchor,clicked" signal is emitted with
 * an #Elm_Entry_Anchor_Info in the @c event_info parameter for the
 * callback function. The same applies for "anchor,in" (mouse in), "anchor,out"
 * (mouse out), "anchor,down" (mouse down), and "anchor,up" (mouse up) events on
 * an anchor.
 *
 * @subsection entry-items Items
 *
 * Inlined in the text, any other @c Evas_Object can be inserted by using
 * \<item\> tags this way:
 *
 * @code
 * <item size=16x16 vsize=full href=emoticon/haha></item>
 * @endcode
 *
 * Just like with anchors, the @c href identifies each item, but these need,
 * in addition, to indicate their size, which is done using any one of
 * @c size, @c absize or @c relsize attributes. These attributes take their
 * value in the WxH format, where W is the width and H the height of the
 * item.
 *
 * @li absize: Absolute pixel size for the item. Whatever value is set will
 * be the item's size regardless of any scale value the object may have
 * been set to. The final line height will be adjusted to fit larger items.
 * @li size: Similar to @c absize, but it's adjusted to the scale value set
 * for the object.
 * @li relsize: Size is adjusted for the item to fit within the current
 * line height.
 *
 * Besides their size, items are specified a @c vsize value that affects
 * how their final size and position are calculated. The possible values
 * are:
 * @li ascent: Item will be placed within the line's baseline and its
 * ascent. That is, the height between the line where all characters are
 * positioned and the highest point in the line. For @c size and @c absize
 * items, the descent value will be added to the total line height to make
 * them fit. @c relsize items will be adjusted to fit within this space.
 * @li full: Items will be placed between the descent and ascent, or the
 * lowest point in the line and its highest.
 *
 * The next image shows different configurations of items and how
 * the previously mentioned options affect their sizes. In all cases,
 * the green line indicates the ascent, blue for the baseline and red for
 * the descent.
 *
 * @image html entry_item.png
 * @image latex entry_item.eps width=\textwidth
 *
 * And another one to show how size differs from absize. In the first one,
 * the scale value is set to 1.0, while the second one is using one of 2.0.
 *
 * @image html entry_item_scale.png
 * @image latex entry_item_scale.eps width=\textwidth
 *
 * After the size for an item is calculated, the entry will request an
 * object to place in its space. For this, the functions set with
 * elm_entry_item_provider_append() and related functions will be called
 * in order until one of them returns a @c non-NULL value. If no providers
 * are available, or all of them return @c NULL, then the entry falls back
 * to one of the internal defaults, provided the name matches with one of
 * them.
 *
 * All of the following are currently supported:
 *
 * - emoticon/angry
 * - emoticon/angry-shout
 * - emoticon/crazy-laugh
 * - emoticon/evil-laugh
 * - emoticon/evil
 * - emoticon/goggle-smile
 * - emoticon/grumpy
 * - emoticon/grumpy-smile
 * - emoticon/guilty
 * - emoticon/guilty-smile
 * - emoticon/haha
 * - emoticon/half-smile
 * - emoticon/happy-panting
 * - emoticon/happy
 * - emoticon/indifferent
 * - emoticon/kiss
 * - emoticon/knowing-grin
 * - emoticon/laugh
 * - emoticon/little-bit-sorry
 * - emoticon/love-lots
 * - emoticon/love
 * - emoticon/minimal-smile
 * - emoticon/not-happy
 * - emoticon/not-impressed
 * - emoticon/omg
 * - emoticon/opensmile
 * - emoticon/smile
 * - emoticon/sorry
 * - emoticon/squint-laugh
 * - emoticon/surprised
 * - emoticon/suspicious
 * - emoticon/tongue-dangling
 * - emoticon/tongue-poke
 * - emoticon/uh
 * - emoticon/unhappy
 * - emoticon/very-sorry
 * - emoticon/what
 * - emoticon/wink
 * - emoticon/worried
 * - emoticon/wtf
 *
 * Alternatively, an item may reference an image by its path, using
 * the URI form @c file:///path/to/an/image.png and the entry will then
 * use that image for the item.
 *
 * @section entry-style-set Setting entry's style
 *
 * There are 2 major ways to change the entry's style:
 * - Theme - set the "base" field to the desired style.
 * - User style - Pushing overrides to the theme style to the textblock object by using evas_object_textblock_style_user_push().
 *
 * You should modify the theme when you would like to change the style for
 * aesthetic reasons. While the user style should be changed when you would
 * like to change the style to something specific defined at run-time, e.g,
 * setting font or font size in a text editor.
 *
 * @section entry-files Loading and saving files
 *
 * Entries have convenience functions to load text from a file and save
 * changes back to it after a short delay. The automatic saving is enabled
 * by default, but can be disabled with elm_entry_autosave_set() and files
 * can be loaded directly as plain text or have any markup in them
 * recognized. See elm_entry_file_set() for more details.
 *
 * @section entry-signals Emitted signals
 *
 * This widget emits the following signals:
 *
 * @li "changed": The text within the entry was changed.
 * @li "changed,user": The text within the entry was changed because of user interaction.
 * @li "activated": The enter key was pressed on a single line entry.
 * @li "aborted": The escape key was pressed on a single line entry. (since 1.7)
 * @li "press": A mouse button has been pressed on the entry.
 * @li "longpressed": A mouse button has been pressed and held for a couple
 * seconds.
 * @li "clicked": The entry has been clicked (mouse press and release).
 * @li "clicked,double": The entry has been double clicked.
 * @li "clicked,triple": The entry has been triple clicked.
 * @li "focused": The entry has received focus.
 * @li "unfocused": The entry has lost focus.
 * @li "selection,paste": A paste of the clipboard contents was requested.
 * @li "selection,copy": A copy of the selected text into the clipboard was
 * requested.
 * @li "selection,cut": A cut of the selected text into the clipboard was
 * requested.
 * @li "selection,start": A selection has begun and no previous selection
 * existed.
 * @li "selection,changed": The current selection has changed.
 * @li "selection,cleared": The current selection has been cleared.
 * @li "cursor,changed": The cursor has changed position.
 * @li "anchor,clicked": An anchor has been clicked. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "anchor,in": Mouse cursor has moved into an anchor. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "anchor,out": Mouse cursor has moved out of an anchor. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "anchor,up": Mouse button has been unpressed on an anchor. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "anchor,down": Mouse button has been pressed on an anchor. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "preedit,changed": The preedit string has changed.
 * @li "language,changed": Program language changed.
 *
 * Default content parts of the entry items that you can use for are:
 * @li "icon" - An icon in the entry
 * @li "end" - A content in the end of the entry
 *
 * Default text parts of the entry that you can use for are:
 * @li "default" - text of the entry
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 *
 * @section entry-examples
 *
 * An overview of the Entry API can be seen in @ref entry_example
 *
 * @{
 */

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
   ELM_INPUT_PANEL_LAYOUT_TERMINAL,    /**< Command-line terminal layout */
   ELM_INPUT_PANEL_LAYOUT_PASSWORD     /**< Like normal, but no auto-correct, no auto-capitalization etc. */
} Elm_Input_Panel_Layout; /**< Type of input panel (virtual keyboard) to use - this is a hint and may not provide exactly what is desired. */

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
   ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEND     /**< Send */
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
 * This adds an entry to @p parent object.
 *
 * By default, entries are:
 * @li not scrolled
 * @li multi-line
 * @li word wrapped
 * @li autosave is enabled
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Entry
 */
EAPI Evas_Object       *elm_entry_add(Evas_Object *parent);

/**
 * Push the style to the top of user style stack.
 * If there is styles in the user style stack, the properties in the top style
 * of user style stack will replace the properties in current theme.
 * The input style is specified in format tag='property=value' (i.e. DEFAULT='font=Sans font_size=60'hilight=' + font_weight=Bold').
 *
 * @param obj The entry object
 * @param style The style user to push
 *
 * @since 1.7
 */
EAPI void      elm_entry_text_style_user_push(Evas_Object *obj, const char *style);

/**
 * Remove the style in the top of user style stack.
 *
 * @param  obj The entry object
 *
 * @see elm_entry_text_style_user_push()
 *
 * @since 1.7
 */
EAPI void     elm_entry_text_style_user_pop(Evas_Object *obj);

/**
 * Retrieve the style on the top of user style stack.
 *
 * @param obj The entry object
 * @return style on the top of user style stack if exist, otherwise NULL.
 *
 * @see elm_entry_text_style_user_push()
 *
 * @since 1.7
 */
EAPI const char*      elm_entry_text_style_user_peek(const Evas_Object *obj);

/**
 * Sets the entry to single line mode.
 *
 * In single line mode, entries don't ever wrap when the text reaches the
 * edge, and instead they keep growing horizontally. Pressing the @c Enter
 * key will generate an @c "activate" event instead of adding a new line.
 *
 * When @p single_line is @c EINA_FALSE, line wrapping takes effect again
 * and pressing enter will break the text into a different line
 * without generating any events.
 *
 * @param obj The entry object
 * @param single_line If true, the text in the entry
 * will be on a single line.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);

/**
 * Gets whether the entry is set to be single line.
 *
 * @param obj The entry object
 * @return single_line If true, the text in the entry is set to display
 * on a single line.
 *
 * @see elm_entry_single_line_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_single_line_get(const Evas_Object *obj);

/**
 * Sets the entry to password mode.
 *
 * In password mode, entries are implicitly single line and the display of
 * any text in them is replaced with asterisks (*).
 *
 * @param obj The entry object
 * @param password If true, password mode is enabled.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_password_set(Evas_Object *obj, Eina_Bool password);

/**
 * Gets whether the entry is set to password mode.
 *
 * @param obj The entry object
 * @return If true, the entry is set to display all characters
 * as asterisks (*).
 *
 * @see elm_entry_password_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_password_get(const Evas_Object *obj);

/**
 * This sets the text displayed within the entry to @p entry.
 *
 * @param obj The entry object
 * @param entry The text to be displayed
 *
 * @note Using this function bypasses text filters
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_entry_set(Evas_Object *obj, const char *entry);

/**
 * This returns the text currently shown in object @p entry.
 * See also elm_entry_entry_set().
 *
 * @param obj The entry object
 * @return The currently displayed text or NULL on failure
 *
 * @ingroup Entry
 */
EAPI const char        *elm_entry_entry_get(const Evas_Object *obj);

/**
 * Appends @p entry to the text of the entry.
 *
 * Adds the text in @p entry to the end of any text already present in the
 * widget.
 *
 * The appended text is subject to any filters set for the widget.
 *
 * @param obj The entry object
 * @param entry The text to be displayed
 *
 * @see elm_entry_markup_filter_append()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_entry_append(Evas_Object *obj, const char *entry);

/**
 * Gets whether the entry is empty.
 *
 * Empty means no text at all. If there are any markup tags, like an item
 * tag for which no provider finds anything, and no text is displayed, this
 * function still returns EINA_FALSE.
 *
 * @param obj The entry object
 * @return EINA_TRUE if the entry is empty, EINA_FALSE otherwise.
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_is_empty(const Evas_Object *obj);

/**
 * Gets any selected text within the entry.
 *
 * If there's any selected text in the entry, this function returns it as
 * a string in markup format. NULL is returned if no selection exists or
 * if an error occurred.
 *
 * The returned value points to an internal string and should not be freed
 * or modified in any way. If the @p entry object is deleted or its
 * contents are changed, the returned pointer should be considered invalid.
 *
 * @param obj The entry object
 * @return The selected text within the entry or NULL on failure
 *
 * @ingroup Entry
 */
EAPI const char        *elm_entry_selection_get(const Evas_Object *obj);

/**
 * Returns the actual textblock object of the entry.
 *
 * This function exposes the internal textblock object that actually
 * contains and draws the text. This should be used for low-level
 * manipulations that are otherwise not possible.
 *
 * Changing the textblock directly from here will not notify edje/elm to
 * recalculate the textblock size automatically, so any modifications
 * done to the textblock returned by this function should be followed by
 * a call to elm_entry_calc_force().
 *
 * The return value is marked as const as an additional warning.
 * One should not use the returned object with any of the generic evas
 * functions (geometry_get/resize/move and etc), but only with the textblock
 * functions; The former will either not work at all, or break the correct
 * functionality.
 *
 * IMPORTANT: Many functions may change (i.e delete and create a new one)
 * the internal textblock object. Do NOT cache the returned object, and try
 * not to mix calls on this object with regular elm_entry calls (which may
 * change the internal textblock object). This applies to all cursors
 * returned from textblock calls, and all the other derivative values.
 *
 * @param obj The entry object
 * @return The textblock object.
 *
 * @ingroup Entry
 */
EAPI Evas_Object *      elm_entry_textblock_get(Evas_Object *obj);

/**
 * Forces calculation of the entry size and text layouting.
 *
 * This should be used after modifying the textblock object directly. See
 * elm_entry_textblock_get() for more information.
 *
 * @param obj The entry object
 *
 * @see elm_entry_textblock_get()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_calc_force(Evas_Object *obj);

/**
 * Inserts the given text into the entry at the current cursor position.
 *
 * This inserts text at the cursor position as if it was typed
 * by the user (note that this also allows markup which a user
 * can't just "type" as it would be converted to escaped text, so this
 * call can be used to insert things like emoticon items or bold push/pop
 * tags, other font and color change tags etc.)
 *
 * If any selection exists, it will be replaced by the inserted text.
 *
 * The inserted text is subject to any filters set for the widget.
 *
 * @param obj The entry object
 * @param entry The text to insert
 *
 * @see elm_entry_markup_filter_append()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_entry_insert(Evas_Object *obj, const char *entry);

/**
 * Set the line wrap type to use on multi-line entries.
 *
 * Sets the wrap type used by the entry to any of the specified in
 * Elm_Wrap_Type. This tells how the text will be implicitly cut into a new
 * line (without inserting a line break or paragraph separator) when it
 * reaches the far edge of the widget.
 *
 * Note that this only makes sense for multi-line entries. A widget set
 * to be single line will never wrap.
 *
 * @param obj The entry object
 * @param wrap The wrap mode to use. See Elm_Wrap_Type for details on them
 */
EAPI void               elm_entry_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap);

/**
 * Gets the wrap mode the entry was set to use.
 *
 * @param obj The entry object
 * @return Wrap type
 *
 * @see also elm_entry_line_wrap_set()
 *
 * @ingroup Entry
 */
EAPI Elm_Wrap_Type      elm_entry_line_wrap_get(const Evas_Object *obj);

/**
 * Sets if the entry is to be editable or not.
 *
 * By default, entries are editable and when focused, any text input by the
 * user will be inserted at the current cursor position. But calling this
 * function with @p editable as EINA_FALSE will prevent the user from
 * inputting text into the entry.
 *
 * The only way to change the text of a non-editable entry is to use
 * elm_object_text_set(), elm_entry_entry_insert() and other related
 * functions.
 *
 * @param obj The entry object
 * @param editable If EINA_TRUE, user input will be inserted in the entry,
 * if not, the entry is read-only and no user input is allowed.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_editable_set(Evas_Object *obj, Eina_Bool editable);

/**
 * Gets whether the entry is editable or not.
 *
 * @param obj The entry object
 * @return If true, the entry is editable by the user.
 * If false, it is not editable by the user
 *
 * @see elm_entry_editable_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_editable_get(const Evas_Object *obj);

/**
 * This drops any existing text selection within the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_select_none(Evas_Object *obj);

/**
 * This selects all text within the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_select_all(Evas_Object *obj);

/**
 * This moves the cursor one place to the right within the entry.
 *
 * @param obj The entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_next(Evas_Object *obj);

/**
 * This moves the cursor one place to the left within the entry.
 *
 * @param obj The entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_prev(Evas_Object *obj);

/**
 * This moves the cursor one line up within the entry.
 *
 * @param obj The entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_up(Evas_Object *obj);

/**
 * This moves the cursor one line down within the entry.
 *
 * @param obj The entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_down(Evas_Object *obj);

/**
 * This moves the cursor to the beginning of the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_begin_set(Evas_Object *obj);

/**
 * This moves the cursor to the end of the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_end_set(Evas_Object *obj);

/**
 * This moves the cursor to the beginning of the current line.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_line_begin_set(Evas_Object *obj);

/**
 * This moves the cursor to the end of the current line.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_line_end_set(Evas_Object *obj);

/**
 * This begins a selection within the entry as though
 * the user were holding down the mouse button to make a selection.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_selection_begin(Evas_Object *obj);

/**
 * This ends a selection within the entry as though
 * the user had just released the mouse button while making a selection.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_selection_end(Evas_Object *obj);

/**
 * Gets whether a format node exists at the current cursor position.
 *
 * A format node is anything that defines how the text is rendered. It can
 * be a visible format node, such as a line break or a paragraph separator,
 * or an invisible one, such as bold begin or end tag.
 * This function returns whether any format node exists at the current
 * cursor position.
 *
 * @param obj The entry object
 * @return EINA_TRUE if the current cursor position contains a format node,
 * EINA_FALSE otherwise.
 *
 * @see elm_entry_cursor_is_visible_format_get()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_is_format_get(const Evas_Object *obj);

/**
 * Gets if the current cursor position holds a visible format node.
 *
 * @param obj The entry object
 * @return EINA_TRUE if the current cursor is a visible format, EINA_FALSE
 * if it's an invisible one or no format exists.
 *
 * @see elm_entry_cursor_is_format_get()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_is_visible_format_get(const Evas_Object *obj);

/**
 * Gets the character pointed by the cursor at its current position.
 *
 * This function returns a string with the utf8 character stored at the
 * current cursor position.
 * Only the text is returned, any format that may exist will not be part
 * of the return value. You must free the string when done with free().
 *
 * @param obj The entry object
 * @return The text pointed by the cursors.
 *
 * @ingroup Entry
 */
EAPI char              *elm_entry_cursor_content_get(const Evas_Object *obj);

/**
 * This function returns the geometry of the cursor.
 *
 * It's useful if you want to draw something on the cursor (or where it is),
 * or for example in the case of scrolled entry where you want to show the
 * cursor.
 *
 * @param obj The entry object
 * @param x returned geometry
 * @param y returned geometry
 * @param w returned geometry
 * @param h returned geometry
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * Sets the cursor position in the entry to the given value
 *
 * The value in @p pos is the index of the character position within the
 * contents of the string as returned by elm_entry_cursor_pos_get().
 *
 * @param obj The entry object
 * @param pos The position of the cursor
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_pos_set(Evas_Object *obj, int pos);

/**
 * Retrieves the current position of the cursor in the entry
 *
 * @param obj The entry object
 * @return The cursor position
 *
 * @ingroup Entry
 */
EAPI int                elm_entry_cursor_pos_get(const Evas_Object *obj);

/**
 * This executes a "cut" action on the selected text in the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_selection_cut(Evas_Object *obj);

/**
 * This executes a "copy" action on the selected text in the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_selection_copy(Evas_Object *obj);

/**
 * This executes a "paste" action in the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_selection_paste(Evas_Object *obj);

/**
 * This clears and frees the items in a entry's contextual (longpress)
 * menu.
 *
 * @param obj The entry object
 *
 * @see elm_entry_context_menu_item_add()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_context_menu_clear(Evas_Object *obj);

/**
 * This adds an item to the entry's contextual menu.
 *
 * A longpress on an entry will make the contextual menu show up, if this
 * hasn't been disabled with elm_entry_context_menu_disabled_set().
 * By default, this menu provides a few options like enabling selection mode,
 * which is useful on embedded devices that need to be explicit about it,
 * and when a selection exists it also shows the copy and cut actions.
 *
 * With this function, developers can add other options to this menu to
 * perform any action they deem necessary.
 *
 * @param obj The entry object
 * @param label The item's text label
 * @param icon_file The item's icon file
 * @param icon_type The item's icon type
 * @param func The callback to execute when the item is clicked
 * @param data The data to associate with the item for related functions
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

/**
 * This disables the entry's contextual (longpress) menu.
 *
 * @param obj The entry object
 * @param disabled If true, the menu is disabled
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * This returns whether the entry's contextual (longpress) menu is
 * disabled.
 *
 * @param obj The entry object
 * @return If true, the menu is disabled
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_context_menu_disabled_get(const Evas_Object *obj);

/**
 * This appends a custom item provider to the list for that entry
 *
 * This appends the given callback. The list is walked from beginning to end
 * with each function called given the item href string in the text. If the
 * function returns an object handle other than NULL (it should create an
 * object to do this), then this object is used to replace that item. If
 * not the next provider is called until one provides an item object, or the
 * default provider in entry does.
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @see @ref entry-items
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_append(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * This prepends a custom item provider to the list for that entry
 *
 * This prepends the given callback. See elm_entry_item_provider_append() for
 * more information
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_prepend(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * This removes a custom item provider to the list for that entry
 *
 * This removes the given callback. See elm_entry_item_provider_append() for
 * more information
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_remove(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * Append a markup filter function for text inserted in the entry
 *
 * Append the given callback to the list. This functions will be called
 * whenever any text is inserted into the entry, with the text to be inserted
 * as a parameter. The type of given text is always markup.
 * The callback function is free to alter the text in any way it wants, but
 * it must remember to free the given pointer and update it.
 * If the new text is to be discarded, the function can free it and set its
 * text parameter to NULL. This will also prevent any following filters from
 * being called.
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_append(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * Prepend a markup filter function for text inserted in the entry
 *
 * Prepend the given callback to the list. See elm_entry_markup_filter_append()
 * for more information
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_prepend(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * Remove a markup filter from the list
 *
 * Removes the given callback from the filter list. See
 * elm_entry_markup_filter_append() for more information.
 *
 * @param obj The entry object
 * @param func The filter function to remove
 * @param data The user data passed when adding the function
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_remove(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

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
 * @ingroup Entry
 */
EAPI char              *elm_entry_utf8_to_markup(const char *s);

/**
 * This sets the file (and implicitly loads it) for the text to display and
 * then edit. All changes are written back to the file after a short delay if
 * the entry object is set to autosave (which is the default).
 *
 * If the entry had any other file set previously, any changes made to it
 * will be saved if the autosave feature is enabled, otherwise, the file
 * will be silently discarded and any non-saved changes will be lost.
 *
 * @param obj The entry object
 * @param file The path to the file to load and save
 * @param format The file format
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format);

/**
 * Gets the file being edited by the entry.
 *
 * This function can be used to retrieve any file set on the entry for
 * edition, along with the format used to load and save it.
 *
 * @param obj The entry object
 * @param file The path to the file to load and save
 * @param format The file format
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format);

/**
 * This function writes any changes made to the file set with
 * elm_entry_file_set()
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_file_save(Evas_Object *obj);

/**
 * This sets the entry object to 'autosave' the loaded text file or not.
 *
 * @param obj The entry object
 * @param autosave Autosave the loaded file or not
 *
 * @see elm_entry_file_set()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave);

/**
 * This gets the entry object's 'autosave' status.
 *
 * @param obj The entry object
 * @return Autosave the loaded file or not
 *
 * @see elm_entry_file_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_autosave_get(const Evas_Object *obj);

/**
 * Enable or disable scrolling in entry
 *
 * Normally the entry is not scrollable unless you enable it with this call.
 *
 * @param obj The entry object
 * @param scroll EINA_TRUE if it is to be scrollable, EINA_FALSE otherwise
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_scrollable_set(Evas_Object *obj, Eina_Bool scroll);

/**
 * Get the scrollable state of the entry
 *
 * Normally the entry is not scrollable. This gets the scrollable state
 * of the entry. See elm_entry_scrollable_set() for more information.
 *
 * @param obj The entry object
 * @return The scrollable state
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_scrollable_get(const Evas_Object *obj);

/**
 * Sets the visibility of the left-side widget of the entry,
 * set by elm_object_part_content_set().
 *
 * @param obj The entry object
 * @param setting EINA_TRUE if the object should be displayed,
 * EINA_FALSE if not.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting);

/**
 * Sets the visibility of the end widget of the entry, set by
 * elm_object_part_content_set(ent, "end", content).
 *
 * @param obj The entry object
 * @param setting EINA_TRUE if the object should be displayed,
 * EINA_FALSE if not.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting);

/**
 * This sets the entry's scrollbar policy (i.e. enabling/disabling
 * them).
 *
 * Setting an entry to single-line mode with elm_entry_single_line_set()
 * will automatically disable the display of scrollbars when the entry
 * moves inside its scroller.
 *
 * @param obj The entry object
 * @param h The horizontal scrollbar policy to apply
 * @param v The vertical scrollbar policy to apply
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v);

/**
 * This enables/disables bouncing within the entry.
 *
 * This function sets whether the entry will bounce when scrolling reaches
 * the end of the contained entry.
 *
 * @param obj The entry object
 * @param h_bounce The horizontal bounce state
 * @param v_bounce The vertical bounce state
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get the bounce mode
 *
 * @param obj The Entry object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Set the input panel layout of the entry
 *
 * @param obj The entry object
 * @param layout layout type
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_layout_set(Evas_Object *obj, Elm_Input_Panel_Layout layout);

/**
 * Get the input panel layout of the entry
 *
 * @param obj The entry object
 * @return layout type
 *
 * @see elm_entry_input_panel_layout_set
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Layout elm_entry_input_panel_layout_get(const Evas_Object *obj);

/**
 * Set the autocapitalization type on the immodule.
 *
 * @param obj The entry object
 * @param autocapital_type The type of autocapitalization
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_autocapital_type_set(Evas_Object *obj, Elm_Autocapital_Type autocapital_type);

/**
 * Retrieve the autocapitalization type on the immodule.
 *
 * @param obj The entry object
 * @return autocapitalization type
 *
 * @ingroup Entry
 */
EAPI Elm_Autocapital_Type   elm_entry_autocapital_type_get(const Evas_Object *obj);

/**
 * Sets the attribute to show the input panel automatically.
 *
 * @param obj The entry object
 * @param enabled If true, the input panel is appeared when entry is clicked or has a focus
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Retrieve the attribute to show the input panel automatically.
 *
 * @param obj The entry object
 * @return EINA_TRUE if input panel will be appeared when the entry is clicked or has a focus, EINA_FALSE otherwise
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_input_panel_enabled_get(const Evas_Object *obj);

/**
 * Show the input panel (virtual keyboard) based on the input panel property of entry such as layout, autocapital types, and so on.
 *
 * Note that input panel is shown or hidden automatically according to the focus state of entry widget.
 * This API can be used in the case of manually controlling by using elm_entry_input_panel_enabled_set(en, EINA_FALSE).
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_show(Evas_Object *obj);

/**
 * Hide the input panel (virtual keyboard).
 *
 * Note that input panel is shown or hidden automatically according to the focus state of entry widget.
 * This API can be used in the case of manually controlling by using elm_entry_input_panel_enabled_set(en, EINA_FALSE)
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_hide(Evas_Object *obj);

/**
 * Set the language mode of the input panel.
 *
 * This API can be used if you want to show the alphabet keyboard mode.
 *
 * @param obj The entry object
 * @param lang language to be set to the input panel.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_language_set(Evas_Object *obj, Elm_Input_Panel_Lang lang);

/**
 * Get the language mode of the input panel.
 *
 * See @ref elm_entry_input_panel_language_set for more details.
 *
 * @param obj The entry object
 * @return input panel language type
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Lang   elm_entry_input_panel_language_get(const Evas_Object *obj);

/**
 * Set the input panel-specific data to deliver to the input panel.
 *
 * This API is used by applications to deliver specific data to the input panel.
 * The data format MUST be negotiated by both application and the input panel.
 * The size and format of data are defined by the input panel.
 *
 * @param obj The entry object
 * @param data The specific data to be set to the input panel.
 * @param len the length of data, in bytes, to send to the input panel
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_imdata_set(Evas_Object *obj, const void *data, int len);

/**
 * Get the specific data of the current input panel.
 *
 * See @ref elm_entry_input_panel_imdata_set for more details.
 *
 * @param obj The entry object
 * @param data The specific data to be got from the input panel
 * @param len The length of data
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_imdata_get(const Evas_Object *obj, void *data, int *len);

/**
 * Set the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * An input panel displays the string or icon associated with this type
 *
 * @param obj The entry object
 * @param return_key_type The type of "return" key on the input panel
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_type_set(Evas_Object *obj, Elm_Input_Panel_Return_Key_Type return_key_type);

/**
 * Get the "return" key type.
 *
 * @see elm_entry_input_panel_return_key_type_set() for more details
 *
 * @param obj The entry object
 * @return The type of "return" key on the input panel
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Return_Key_Type elm_entry_input_panel_return_key_type_get(const Evas_Object *obj);

/**
 * Set the return key on the input panel to be disabled.
 *
 * @param obj The entry object
 * @param disabled The state to put in in: @c EINA_TRUE for
 *        disabled, @c EINA_FALSE for enabled
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Get whether the return key on the input panel should be disabled or not.
 *
 * @param obj The entry object
 * @return EINA_TRUE if it should be disabled
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_input_panel_return_key_disabled_get(const Evas_Object *obj);

/**
 * Set whether the return key on the input panel is disabled automatically when entry has no text.
 *
 * If @p enabled is EINA_TRUE, The return key on input panel is disabled when the entry has no text.
 * The return key on the input panel is automatically enabled when the entry has text.
 * The default value is EINA_FALSE.
 *
 * @param obj The entry object
 * @param enabled If @p enabled is EINA_TRUE, the return key is automatically disabled when the entry has no text.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_autoenabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Reset the input method context of the entry if needed.
 *
 * This can be necessary in the case where modifying the buffer would confuse on-going input method behavior.
 * This will typically cause the Input Method Context to clear the preedit state.
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_imf_context_reset(Evas_Object *obj);

/**
 * Set whether the entry should allow to use the text prediction.
 *
 * @param obj The entry object
 * @param prediction Whether the entry should allow to use the text prediction.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_prediction_allow_set(Evas_Object *obj, Eina_Bool prediction);

/**
 * Get whether the entry should allow to use the text prediction.
 *
 * @param obj The entry object
 * @return EINA_TRUE if it allows to use the text prediction, otherwise EINA_FALSE.
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_prediction_allow_get(const Evas_Object *obj);

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
 * Returns the input method context of the entry.
 *
 * This function exposes the internal input method context.
 *
 * IMPORTANT: Many functions may change (i.e delete and create a new one)
 * the internal input method context. Do NOT cache the returned object.
 *
 * @param obj The entry object
 * @return The input method context (Ecore_IMF_Context *) in entry.
 *
 * @ingroup Entry
 */
EAPI void                  *elm_entry_imf_context_get(Evas_Object *obj);

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
 * Control pasting of text and images for the widget.
 *
 * Normally the entry allows both text and images to be pasted.
 * By setting cnp_mode to be #ELM_CNP_MODE_NO_IMAGE, this prevents images from being copy or past.
 * By setting cnp_mode to be #ELM_CNP_MODE_PLAINTEXT, this remove all tags in text .
 *
 * @note this only changes the behaviour of text.
 *
 * @param obj The entry object
 * @param cnp_mode One of #Elm_Cnp_Mode: #ELM_CNP_MODE_MARKUP, #ELM_CNP_MODE_NO_IMAGE, #ELM_CNP_MODE_PLAINTEXT.
 *
 * @ingroup Entry
 */
EAPI void         elm_entry_cnp_mode_set(Evas_Object *obj, Elm_Cnp_Mode cnp_mode);

/**
 * Getting elm_entry text paste/drop mode.
 *
 * Normally the entry allows both text and images to be pasted.
 * This gets the copy & paste mode of the entry.
 *
 * @param obj The entry object
 * @return mode One of #Elm_Cnp_Mode: #ELM_CNP_MODE_MARKUP, #ELM_CNP_MODE_NO_IMAGE, #ELM_CNP_MODE_PLAINTEXT.
 *
 * @ingroup Entry
 */
EAPI Elm_Cnp_Mode elm_entry_cnp_mode_get(const Evas_Object *obj);

/**
 * Set the parent of the hover popup
 *
 * Sets the parent object to use by the hover created by the entry
 * when an anchor is clicked. See @ref Hover for more details on this.
 *
 * @param obj The entry object
 * @param parent The object to use as parent for the hover
 *
 * @ingroup Entry
 */
EAPI void                        elm_entry_anchor_hover_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * Get the parent of the hover popup
 *
 * Get the object used as parent for the hover created by the entry
 * widget. See @ref Hover for more details on this.
 * If no parent is set, the same entry object will be used.
 *
 * @param obj The entry object
 * @return The object used as parent for the hover, NULL if none is set.
 *
 * @ingroup Entry
 */
EAPI Evas_Object                *elm_entry_anchor_hover_parent_get(const Evas_Object *obj);

/**
 * Set the style that the hover should use
 *
 * When creating the popup hover, entry will request that it's
 * themed according to @p style.
 *
 * Setting style no @c NULL means disabling automatic hover.
 *
 * @param obj The entry object
 * @param style The style to use for the underlying hover
 *
 * @see elm_object_style_set()
 *
 * @ingroup Entry
 */
EAPI void                        elm_entry_anchor_hover_style_set(Evas_Object *obj, const char *style);

/**
 * Get the style that the hover should use
 *
 * Get the style, the hover created by entry will use.
 *
 * @param obj The entry object
 * @return The style to use by the hover. @c NULL means the default is used.
 *
 * @see elm_object_style_set()
 *
 * @ingroup Entry
 */
EAPI const char                 *elm_entry_anchor_hover_style_get(const Evas_Object *obj);

/**
 * Ends the hover popup in the entry
 *
 * When an anchor is clicked, the entry widget will create a hover
 * object to use as a popup with user provided content. This function
 * terminates this popup, returning the entry to its normal state.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                        elm_entry_anchor_hover_end(Evas_Object *obj);

/**
 * @}
 */
