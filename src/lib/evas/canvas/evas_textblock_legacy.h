/**
 * @defgroup Evas_Object_Textblock Textblock Object Functions
 *
 * Functions used to create and manipulate textblock objects. Unlike
 * @ref Evas_Object_Text, these handle complex text, doing multiple
 * styles and multiline text based on HTML-like tags. Of these extra
 * features will be heavier on memory and processing cost.
 *
 * @section Evas_Object_Textblock_Tutorial Textblock Object Tutorial
 *
 * This part explains about the textblock object's API and proper usage.
 * The main user of the textblock object is the edje entry object in Edje, so
 * that's a good place to learn from, but I think this document is more than
 * enough, if it's not, please contact me and I'll update it.
 *
 * @subsection textblock_intro Introduction
 * The textblock objects is, as implied, an object that can show big chunks of
 * text. Textblock supports many features including: Text formatting, automatic
 * and manual text alignment, embedding items (for example icons) and more.
 * Textblock has three important parts, the text paragraphs, the format nodes
 * and the cursors.
 *
 * You can use markup to format text, for example: "<font_size=50>Big!</font_size>".
 * You can also put more than one style directive in one tag:
 * "<font_size=50 color=#F00>Big and Red!</font_size>".
 * Please notice that we used "</font_size>" although the format also included
 * color, this is because the first format determines the matching closing tag's
 * name. You can also use anonymous tags, like: "<font_size=30>Big</>" which
 * just pop any type of format, but it's advised to use the named alternatives
 * instead.
 *
 * @subsection textblock_cursors Textblock Object Cursors
 * A textblock Cursor is data type that represents
 * a position in a textblock. Each cursor contains information about the
 * paragraph it points to, the position in that paragraph and the object itself.
 * Cursors register to textblock objects upon creation, this means that once
 * you created a cursor, it belongs to a specific obj and you can't for example
 * copy a cursor "into" a cursor of a different object. Registered cursors
 * also have the added benefit of updating automatically upon textblock changes,
 * this means that if you have a cursor pointing to a specific character, it'll
 * still point to it even after you change the whole object completely (as long
 * as the char was not deleted), this is not possible without updating, because
 * as mentioned, each cursor holds a character position. There are many
 * functions that handle cursors, just check out the evas_textblock_cursor*
 * functions. For creation and deletion of cursors check out:
 * @see evas_object_textblock_cursor_new()
 * @see evas_textblock_cursor_free()
 * @note Cursors are generally the correct way to handle text in the textblock object, and there are enough functions to do everything you need with them (no need to get big chunks of text and processing them yourself).
 *
 * @subsection textblock_paragraphs Textblock Object Paragraphs
 * The textblock object is made out of text splitted to paragraphs (delimited
 * by the paragraph separation character). Each paragraph has many (or none)
 * format nodes associated with it which are responsible for the formatting
 * of that paragraph.
 *
 * @subsection textblock_format_nodes Textblock Object Format Nodes
 * As explained in @ref textblock_paragraphs each one of the format nodes
 * is associated with a paragraph.
 * There are two types of format nodes, visible and invisible:
 * Visible: formats that a cursor can point to, i.e formats that
 * occupy space, for example: newlines, tabs, items and etc. Some visible items
 * are made of two parts, in this case, only the opening tag is visible.
 * A closing tag (i.e a \</tag\> tag) should NEVER be visible.
 * Invisible: formats that don't occupy space, for example: bold and underline.
 * Being able to access format nodes is very important for some uses. For
 * example, edje uses the "<a>" format to create links in the text (and pop
 * popups above them when clicked). For the textblock object a is just a
 * formatting instruction (how to color the text), but edje utilizes the access
 * to the format nodes to make it do more.
 * For more information, take a look at all the evas_textblock_node_format_*
 * functions.
 * The translation of "<tag>" tags to actual format is done according to the
 * tags defined in the style, see @ref evas_textblock_style_set
 *
 * @subsection textblock_special_formats Special Formats
 * Textblock supports various format directives that can be used in markup. In
 * addition to the mentioned format directives, textblock allows creating
 * additional format directives using "tags" that can be set in the style see
 * @ref evas_textblock_style_set .
 *
 * For more details see @ref evas_textblock_style_page
 *
 * Textblock supports the following formats:
 * @li font - Font description in fontconfig like format, e.g: "Sans:style=Italic:lang=hi". or "Serif:style=Bold".
 * @li font_weight - Overrides the weight defined in "font". E.g: "font_weight=Bold" is the same as "font=:style=Bold". Supported weights: "normal", "thin", "ultralight", "light", "book", "medium", "semibold", "bold", "ultrabold", "black", and "extrablack".
 * @li font_style - Overrides the style defined in "font". E.g: "font_style=Italic" is the same as "font=:style=Italic". Supported styles: "normal", "oblique", and "italic".
 * @li font_width - Overrides the width defined in "font". E.g: "font_width=Condensed" is the same as "font=:style=Condensed". Supported widths: "normal", "ultracondensed", "extracondensed", "condensed", "semicondensed", "semiexpanded", "expanded", "extraexpanded", and "ultraexpanded".
 * @li lang - Overrides the language defined in "font". E.g: "lang=he" is the same as "font=:lang=he".
 * @li font_fallbacks - A comma delimited list of fonts to try if finding the main font fails.
 * @li font_size - The font size in points.
 * @li font_source - The source of the font, e.g an eet file.
 * @li color - Text color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li underline_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li underline2_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li outline_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li shadow_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li glow_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li glow2_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li strikethrough_color - color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li align - Either "auto" (meaning according to text direction), "left", "right", "center", "middle", a value between 0.0 and 1.0, or a value between 0% to 100%.
 * @li valign - Either "top", "bottom", "middle", "center", "baseline", "base", a value between 0.0 and 1.0, or a value between 0% to 100%.
 * @li wrap - "word", "char", "mixed", or "none".
 * @li left_margin - Either "reset", or a pixel value indicating the margin.
 * @li right_margin - Either "reset", or a pixel value indicating the margin.
 * @li underline - "on", "off", "single", or "double".
 * @li strikethrough - "on" or "off"
 * @li backing_color - Background color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA".
 * @li backing - Enable/disable background color. ex) "on" or "off"
 * @li style - Either "off", "none", "plain", "shadow", "outline", "soft_outline", "outline_shadow", "outline_soft_shadow", "glow", "far_shadow", "soft_shadow", or "far_soft_shadow". Direction can be selected by adding "bottom_right", "bottom", "bottom_left", "left", "top_left", "top", "top_right", or "right". E.g: "style=shadow,bottom_right".
 * @li tabstops - Pixel value for tab width.
 * @li linesize - Force a line size in pixels.
 * @li linerelsize - Either a floating point value or a percentage indicating the wanted size of the line relative to the calculated size.
 * @li linegap - Force a line gap in pixels.
 * @li linerelgap - Either a floating point value or a percentage indicating the wanted size of the line relative to the calculated size.
 * @li item - Creates an empty space that should be filled by an upper layer. Use "size", "abssize", or "relsize". To define the items size, and an optional: vsize=full/ascent to define the item's position in the line.
 * @li linefill - Either a float value or percentage indicating how much to fill the line.
 * @li ellipsis - Value between 0.0-1.0 to indicate the type of ellipsis, or -1.0 to indicate ellipsis isn't wanted.
 * @li password - "on" or "off". This is used to specifically turn replacing chars with the replacement char (i.e password mode) on and off.
 *
 * @warning We don't guarantee any proper results if you create a Textblock
 * object
 * without setting the evas engine.
 *
 * @todo put here some usage examples
 *
 * @ingroup Evas_Object_Specific
 *
 * @{
 */
typedef Eo Evas_Textblock;

/**
 * @typedef Evas_Textblock_Style
 *
 * A textblock style object.
 * @see evas_textblock_style_new()
 * @see evas_textblock_style_get()
 * @see evas_textblock_style_set()
 */

typedef struct _Efl_Canvas_Text_Style             Evas_Textblock_Style;

/**
 * @typedef Evas_Textblock_Cursor
 *
 * A textblock cursor object, used to maipulate the cursor of an evas textblock
 * @see evas_object_textblock_cursor_new
 *
 */
typedef struct _Efl_Text_Cursor_Cursor       Evas_Textblock_Cursor;

/**
 * @typedef Evas_Object_Textblock_Node_Format
 * A format node.
 *
 * XXX: Adapter for legacy.
 */
typedef struct _Evas_Textblock_Node_Format Evas_Object_Textblock_Node_Format;

/**
 * @typedef Evas_Textblock_Node_Format
 * A format node.
 *
 * XXX: Adapter for legacy.
 */
typedef struct _Evas_Textblock_Node_Format Evas_Textblock_Node_Format;

/**
 * @typedef Evas_Textblock_Rectangle
 * General-purpose rectangle that represents some geometry in this object.
 *
 */
typedef Eina_Rectangle Evas_Textblock_Rectangle;

/**
 * Text type for evas textblock.
 */
typedef enum _Evas_Textblock_Text_Type
{
   EVAS_TEXTBLOCK_TEXT_RAW, /**< textblock text of type raw */
   EVAS_TEXTBLOCK_TEXT_PLAIN, /**< textblock text of type plain */
   EVAS_TEXTBLOCK_TEXT_MARKUP /**< textblock text of type markup */
} Evas_Textblock_Text_Type;

/**
 * Cursor type for evas textblock.
 */
typedef enum _Evas_Textblock_Cursor_Type
{
   EVAS_TEXTBLOCK_CURSOR_UNDER, /**< cursor type is under */
   EVAS_TEXTBLOCK_CURSOR_BEFORE /**< cursor type is before */
} Evas_Textblock_Cursor_Type;

/**
 * Returns the unescaped version of escape.
 * @param escape the string to be escaped
 * @return the unescaped version of escape
 */
EAPI const char                              *evas_textblock_escape_string_get(const char *escape) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Returns the escaped version of the string.
 * @param string to escape
 * @param len_ret the len of the part of the string that was used.
 * @return the escaped string.
 */
EAPI const char                              *evas_textblock_string_escape_get(const char *string, int *len_ret) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Return the unescaped version of the string between start and end.
 *
 * @param escape_start the start of the string.
 * @param escape_end the end of the string.
 * @return the unescaped version of the range
 */
EAPI const char                              *evas_textblock_escape_string_range_get(const char *escape_start, const char *escape_end) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Creates a new textblock style.
 * @return  The new textblock style.
 */
EAPI Evas_Textblock_Style                    *evas_textblock_style_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Destroys a textblock style.
 * @param ts The textblock style to free.
 */
EAPI void                                     evas_textblock_style_free(Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);

/**
 * Sets the style ts to the style passed as text by text.
 * Expected a string consisting of many (or none) tag='format' pairs.
 *
 * @param ts  the style to set.
 * @param text the text to parse - NOT NULL.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_style_set(Evas_Textblock_Style *ts, const char *text) EINA_ARG_NONNULL(1);

/**
 * Return the text of the style ts.
 * @param ts  the style to get it's text.
 * @return the text of the style or null on error.
 */
EAPI const char                              *evas_textblock_style_get(const Evas_Textblock_Style *ts) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Returns the next format node (after n)
 *
 * @param n the current format node - not null.
 * @return Returns the next format node, may be null.
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_next_get(const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1);

/**
 * Returns the prev format node (after n)
 *
 * @param n the current format node - not null.
 * @return Returns the prev format node, may be null.
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_prev_get(const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1);

/**
 * Sets the cursor to point to the place where format points to.
 *
 * @param cur the cursor to update.
 * @param n the format node to update according.
 * @deprecated duplicate of evas_textblock_cursor_at_format_set
 */
EAPI void                                     evas_textblock_cursor_set_at_format(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);

/**
 * Return the format node at the position pointed by cur.
 *
 * @param cur the position to look at.
 * @return the format node if found, @c NULL otherwise.
 * @see evas_textblock_cursor_format_is_visible_get()
 */
EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_cursor_format_get(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Get the text format representation of the format node.
 *
 * @param fnode the format node.
 * @return the textual format of the format node.
 */
EAPI const char                              *evas_textblock_node_format_text_get(const Evas_Object_Textblock_Node_Format *fnode) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the cursor to point to the position of fmt.
 *
 * @param cur the cursor to update
 * @param fmt the format to update according to.
 */
EAPI void                                     evas_textblock_cursor_at_format_set(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *fmt) EINA_ARG_NONNULL(1, 2);

/**
 * Check if the current cursor position is a visible format. This way is more
 * efficient than evas_textblock_cursor_format_get() to check for the existence
 * of a visible format.
 *
 * @param cur the cursor to look at.
 * @return @c EINA_TRUE if the cursor points to a visible format, @c EINA_FALSE
 * otherwise.
 * @see evas_textblock_cursor_format_get()
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_is_visible_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Advances to the next format node
 *
 * @param cur the cursor to be updated.
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Advances to the previous format node.
 *
 * @param cur the cursor to update.
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Returns true if the cursor points to a format.
 *
 * @param cur the cursor to check.
 * @return  @c EINA_TRUE if a cursor points to a format @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_is_format(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

/**
 * Return the current cursor pos.
 *
 * @param cur the cursor to take the position from.
 * @return the position or -1 on error
 */
EAPI int                                      evas_textblock_cursor_pos_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Go to the start of the line passed
 *
 * @param cur cursor to update.
 * @param line number to set.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 */
EAPI Eina_Bool                                evas_textblock_cursor_line_set(Evas_Textblock_Cursor *cur, int line) EINA_ARG_NONNULL(1);

/**
 * Adds format to the current cursor position. If the format being added is a
 * visible format, add it *before* the cursor position, otherwise, add it after.
 * This behavior is because visible formats are like characters and invisible
 * should be stacked in a way that the last one is added last.
 *
 * This function works with native formats, that means that style defined
 * tags like <br> won't work here. For those kind of things use markup prepend.
 *
 * @param cur the cursor to where to add format at.
 * @param format the format to add.
 * @return Returns true if a visible format was added, false otherwise.
 * @see evas_textblock_cursor_format_prepend()
 */

/**
 * Check if the current cursor position points to the terminating null of the
 * last paragraph. (shouldn't be allowed to point to the terminating null of
 * any previous paragraph anyway.
 *
 * @param cur the cursor to look at.
 * @return @c EINA_TRUE if the cursor points to the terminating null, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);

/**
 * Adds format to the current cursor position. If the format being added is a
 * visible format, add it *before* the cursor position, otherwise, add it after.
 * This behavior is because visible formats are like characters and invisible
 * should be stacked in a way that the last one is added last.
 * If the format is visible the cursor is advanced after it.
 *
 * This function works with native formats, that means that style defined
 * tags like <br> won't work here. For those kind of things use markup prepend.
 *
 * @param cur the cursor to where to add format at.
 * @param format the format to add.
 * @return Returns true if a visible format was added, false otherwise.
 * @see evas_textblock_cursor_format_prepend()
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_prepend(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);

/**
 * Delete the range between cur1 and cur2.
 *
 * @param cur1 one side of the range.
 * @param cur2 the second side of the range
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2) EINA_ARG_NONNULL(1, 2);

/**
 * Return the text of the paragraph cur points to - returns the text in markup.
 *
 * @param cur the cursor pointing to the paragraph.
 * @return the text on success, @c NULL otherwise.
 */
EAPI const char                              *evas_textblock_cursor_paragraph_text_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Return the length of the paragraph, cheaper the eina_unicode_strlen()
 *
 * @param cur the position of the paragraph.
 * @return the length of the paragraph on success, -1 otehrwise.
 */
EAPI int                                      evas_textblock_cursor_paragraph_text_length_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Return the currently visible range.
 *
 * @param start the start of the range.
 * @param end the end of the range.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 * @since 1.1
 */
EAPI Eina_Bool                                evas_textblock_cursor_visible_range_get(Evas_Textblock_Cursor *start, Evas_Textblock_Cursor *end) EINA_ARG_NONNULL(1, 2);

/**
 * Return the format nodes in the range between cur1 and cur2.
 *
 * @param cur1 one side of the range.
 * @param cur2 the other side of the range
 * @return the foramt nodes in the range. You have to free it.
 * @since 1.1
 */
EAPI Eina_List                               *evas_textblock_cursor_range_formats_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Return the text in the range between cur1 and cur2
 *
 * @param cur1 one side of the range.
 * @param cur2 the other side of the range
 * @param format The form on which to return the text. Markup - in textblock markup. Plain - UTF8.
 * @return the text in the range
 * @see elm_entry_markup_to_utf8()
 */
EAPI char                                    *evas_textblock_cursor_range_text_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2, Evas_Textblock_Text_Type format) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Return the content of the cursor.
 *
 * Free the returned string pointer when done (if it is not NULL).
 *
 * @param cur the cursor
 * @return the text in the range, terminated by a nul byte (may be utf8).
 */
EAPI char                                    *evas_textblock_cursor_content_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Returns the geometry of two cursors ("split cursor"), if logical cursor is
 * between LTR/RTL text, also considering paragraph direction.
 * Upper cursor is shown for the text of the same direction as paragraph,
 * lower cursor - for opposite.
 *
 * Split cursor geometry is valid only  in '|' cursor mode.
 * In this case @c EINA_TRUE is returned and cx2, cy2, cw2, ch2 are set,
 * otherwise it behaves like cursor_geometry_get.
 *
 * @param[in] cur the cursor.
 * @param[out] cx the x of the cursor (or upper cursor)
 * @param[out] cy the y of the cursor (or upper cursor)
 * @param[out] cw the width of the cursor (or upper cursor)
 * @param[out] ch the height of the cursor (or upper cursor)
 * @param[out] cx2 the x of the lower cursor
 * @param[out] cy2 the y of the lower cursor
 * @param[out] cw2 the width of the lower cursor
 * @param[out] ch2 the height of the lower cursor
 * @param[in] ctype the type of the cursor.
 * @return @c EINA_TRUE for split cursor, @c EINA_FALSE otherwise
 * @since 1.8
 */
EAPI Eina_Bool
evas_textblock_cursor_geometry_bidi_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_Coord *cx2, Evas_Coord *cy2, Evas_Coord *cw2, Evas_Coord *ch2, Evas_Textblock_Cursor_Type ctype);

/**
 * Returns the geometry of the cursor. Depends on the type of cursor requested.
 * This should be used instead of char_geometry_get because there are weird
 * special cases with BiDi text.
 * in '_' cursor mode (i.e a line below the char) it's the same as char_geometry
 * get, except for the case of the last char of a line which depends on the
 * paragraph direction.
 *
 * in '|' cursor mode (i.e a line between two chars) it is very variable.
 * For example consider the following visual string:
 * "abcCBA" (ABC are rtl chars), a cursor pointing on A should actually draw
 * a '|' between the c and the C.
 *
 * @param cur the cursor.
 * @param cx the x of the cursor
 * @param cy the y of the cursor
 * @param cw the width of the cursor
 * @param ch the height of the cursor
 * @param dir the direction of the cursor, can be NULL.
 * @param ctype the type of the cursor.
 * @return line number of the char on success, -1 on error.
 */
EAPI int                                      evas_textblock_cursor_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *dir, Evas_Textblock_Cursor_Type ctype) EINA_ARG_NONNULL(1);

/**
 * Returns the geometry of the char at cur.
 *
 * @param cur the position of the char.
 * @param cx the x of the char.
 * @param cy the y of the char.
 * @param cw the w of the char.
 * @param ch the h of the char.
 * @return line number of the char on success, -1 on error.
 */
EAPI int                                      evas_textblock_cursor_char_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Returns the geometry of the pen at cur.
 *
 * @param cur the position of the char.
 * @param cpen_x the pen_x of the char.
 * @param cy the y of the char.
 * @param cadv the adv of the char.
 * @param ch the h of the char.
 * @return line number of the char on success, -1 on error.
 */
EAPI int                                      evas_textblock_cursor_pen_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cpen_x, Evas_Coord *cy, Evas_Coord *cadv, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Returns the geometry of the line at cur.
 *
 * @param cur the position of the line.
 * @param cx the x of the line.
 * @param cy the y of the line.
 * @param cw the width of the line.
 * @param ch the height of the line.
 * @return line number of the line on success, -1 on error.
 */
EAPI int                                      evas_textblock_cursor_line_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Set the cursor position according to the y coord.
 *
 * @param cur the cur to be set.
 * @param y the coord to set by.
 * @return the line number found, -1 on error.
 */
EAPI int                                      evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y) EINA_ARG_NONNULL(1);

/**
 * Get the geometry of a range.
 *
 * @param cur1 one side of the range.
 * @param cur2 other side of the range.
 * @return a list of Rectangles representing the geometry of the range.
 */
EAPI Eina_List                               *evas_textblock_cursor_range_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Get the simple geometry of a range.
 * The simple geometry is the geomtry in which rectangles in middle
 * lines of range are merged into one big rectangle.
 * @since 1.13
 *
 * @param cur1 one side of the range.
 * @param cur2 other side of the range.
 * @return an iterator of rectangles representing the geometry of the range.
 */
EAPI Eina_Iterator                               *evas_textblock_cursor_range_simple_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Get the geometry of ?
 *
 * @param cur one side of the range.
 * @param cur2 other side of the range.
 */
EAPI Eina_Bool                                evas_textblock_cursor_format_item_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Checks if the cursor points to the end of the line.
 *
 * @param cur the cursor to check.
 * @return @c EINA_TRUE if true, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                                evas_textblock_cursor_eol_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Advances the cursor one char backwards.
 *
 * @return @c true on success, @c false otherwise.
 */
EAPI Eina_Bool evas_textblock_cursor_char_prev(Evas_Textblock_Cursor *obj);

/**
 * @brief Advances the cursor one char forward.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Evas_Textblock_Cursor
 */
EAPI Eina_Bool evas_textblock_cursor_char_next(Evas_Textblock_Cursor *obj);

/**
 * @brief Advances the cursor one grapheme cluster backwards.
 *
 * @return @c true on success, @c false otherwise.
 */
EAPI Eina_Bool evas_textblock_cursor_cluster_prev(Evas_Textblock_Cursor *obj);

/**
 * @brief Advances the cursor one grapheme cluster forward.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Evas_Textblock_Cursor
 */
EAPI Eina_Bool evas_textblock_cursor_cluster_next(Evas_Textblock_Cursor *obj);

/**
 * @brief Advances to the start of the next text node
 *
 * @return @c true if managed to advance, @c false otherwise
 *
 * @ingroup Evas_Textblock_Cursor
 */
EAPI Eina_Bool evas_textblock_cursor_paragraph_next(Evas_Textblock_Cursor *obj);

/**
 * @brief Advances to the end of the previous text node
 *
 * @return @c true if managed to advance, @c false otherwise
 *
 * @ingroup Evas_Textblock_Cursor
 */
EAPI Eina_Bool evas_textblock_cursor_paragraph_prev(Evas_Textblock_Cursor *obj);

/**
 * @brief Moves the cursor to the start of the word under the cursor
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Evas_Textblock_Cursor
 */
EAPI Eina_Bool evas_textblock_cursor_word_start(Evas_Textblock_Cursor *obj);

/**
 * @brief Moves the cursor to the end of the word under the cursor
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Evas_Textblock_Cursor
 */
EAPI Eina_Bool evas_textblock_cursor_word_end(Evas_Textblock_Cursor *obj);

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Textblock
 *
 * @{
 */

/**
 * Adds a textblock to the given evas.
 * @param   e The given evas.
 * @return  The new textblock object.
 */
EAPI Evas_Object                             *evas_object_textblock_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Return the plain version of the markup.
 *
 * Works as if you set the markup to a textblock and then retrieve the plain
 * version of the text. i.e: <br> and <\n> will be replaced with \n, &...; with
 * the actual char and etc.
 *
 * @param obj The textblock object to work with. (if @c NULL, tries the
 * default).
 * @param text The markup text (if @c NULL, return @c NULL).
 * @return An allocated plain text version of the markup.
 * @since 1.2
 */
EAPI char                                    *evas_textblock_text_markup_to_utf8(const Evas_Object *obj, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Return the markup version of the plain text.
 *
 * Replaces \\n -\> \<br/\> \\t -\> \<tab/\> and etc. Generally needed before you pass
 * plain text to be set in a textblock.
 *
 * @param obj the textblock object to work with (if @c NULL, it just does the
 * default behaviour, i.e with no extra object information).
 * @param text The plain text (if @c NULL, return @c NULL).
 * @return An allocated markup version of the plain text.
 * @since 1.2
 */
EAPI char                                    *evas_textblock_text_utf8_to_markup(const Evas_Object *obj, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Clear the textblock object.
 * @note Does *NOT* free the Evas object itself.
 *
 * @param obj the object to clear.
 * @return nothing.
 */
EAPI void                                     evas_object_textblock_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Sets given text as markup for the textblock object.
 *
 * @note assumes text does not include the unicode object replacement char (0xFFFC)
 *
 * @param obj  the textblock object.
 * @param text the markup text to set.
 * @return Returns no value.
 */
EAPI void evas_object_textblock_text_markup_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);

/**
 * Gets the current markup text of the textblock object.
 *
 * @param obj  the textblock object.
 * @return Returns the current markup text.
 */
EAPI const char *evas_object_textblock_text_markup_get(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Prepends markup to the cursor cur.
 *
 * @note assumes text does not include the unicode object replacement char (0xFFFC)
 *
 * @param cur  the cursor to prepend to.
 * @param text the markup text to prepend.
 * @return Return no value.
 */
EAPI void                                     evas_object_textblock_text_markup_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * Sets object's style to given style.
 *
 * @param obj the object we set the style on
 * @param ts  the style to set.
 * @return Returns no value.
 * @see evas_textblock_style_user_push()
 */
EAPI void                                     evas_object_textblock_style_set(Evas_Object *obj, const Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1, 2);

/**
 * Gets the object's current style.
 *
 * @param obj the object to get the style from.
 * @return Returns the current active style.
 * @see evas_textblock_style_user_peek()
 */
EAPI Evas_Textblock_Style                     *evas_object_textblock_style_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);


/**
 * Pushes ts to the top of the user style stack.
 *
 * FIXME: API is solid but currently only supports 1 style in the
 * stack.
 *
 * The user style overrides the corresponding elements of the
 * regular style. This is the proper way to do theme overrides
 * in code.
 *
 * @param obj the object to push the style.
 * @see also evas_textblock_style_set().
 *
 * @since 1.2
 */
EAPI void                                      evas_object_textblock_style_user_push(Evas_Object *obj, Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1,2);

/**
 * Peeks at the user-styles stack
 *
 * @param obj the object to get the style from.
 * @return Returns the user style at the top of the user style stack.
 * @see evas_textblock_style_user_pop()
 */
EAPI const Evas_Textblock_Style               *evas_object_textblock_style_user_peek(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Removes the style at the top of the user-style stack
 *
 * @param obj the object to remove the style from.
 * @return Returns no value.
 * @see evas_textblock_style_user_peek()
 */
EAPI void                                     evas_object_textblock_style_user_pop(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Gets the first format node
 *
 * @param obj The textblock object.
 * @return Returns the first format node.
 */
EAPI const Evas_Object_Textblock_Node_Format* evas_textblock_node_format_first_get(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Gets the last format node
 *
 * @param obj The textblock object.
 * @return Returns the first format node.
 */
EAPI const Evas_Object_Textblock_Node_Format* evas_textblock_node_format_last_get(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Gets a list of format nodes that match given format.
 *
 * @param obj The textblock object to query.
 * @param anchor The format to find in the textblock.
 * @return Returns a list of format nodes that match the given format.
 */
EAPI const Eina_List *evas_textblock_node_format_list_get(const Evas_Object *obj, const char *anchor) EINA_ARG_NONNULL(1);

/**
 * Removes format node and its matching format node.
 *
 * @param obj The textblock object.
 * @param n The format node to remove.
 * @return Returns no value.
 */
EAPI void evas_textblock_node_format_remove_pair(Evas_Object *obj, Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);

/**
 * Gets the geometry of the line specified by its number.
 *
 * @param obj The textblock object to query.
 * @param line the line number of the queried line.
 * @return Returns no value.
 */
EAPI Eina_Bool evas_object_textblock_line_number_geometry_get(const Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

/**
 * Make cur_dest point to the same place as cur. Does not work if they don't
 * point to the same object.
 *
 * @param cur the source cursor.
 * @param cur_dest destination cursor.
 * @return Returns no value.
 */
EAPI void                                     evas_textblock_cursor_copy(const Evas_Textblock_Cursor *cur, Evas_Textblock_Cursor *cur_dest);

/**
 *  Create a new cursor, associate it to the obj and init it to point
 *  to the start of the textblock.
 *
 *  Association to the object means the cursor will be updated when
 *  the object will change.
 *
 *  Note: if you need speed and you know what you are doing, it's
 *  slightly faster to just allocate the cursor yourself and not
 *  associate it. (only people developing the actual object, and
 *  not users of the object).
 *
 *  @param obj The textblock to which the new cursor will associate.
 *  @return Returns a new cursor associated with the given textblock object.
 */
EAPI Evas_Textblock_Cursor *evas_object_textblock_cursor_new(const Evas_Object *obj);

/**
 * @brief Sets the position of the cursor according to the X and Y coordinates.
 *
 * @param[in] y y coord to set by.
 *
 * @return @c true on success, @c false otherwise.
 */
EAPI Eina_Bool evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *obj, Evas_Coord x, Evas_Coord y);

/**
 * @brief Sets the position of the cursor according to the X and Y coordinates and
 * grapheme clusters of text.
 *
 * @param[in] y y coord to set by.
 *
 * @return @c true on success, @c false otherwise.
 */
EAPI Eina_Bool evas_textblock_cursor_cluster_coord_set(Evas_Textblock_Cursor *obj, Evas_Coord x, Evas_Coord y);

/**
 * Free the cursor and unassociate it from the object.
 * @note do not use it to free unassociated cursors.
 *
 * @param cur the cursor to free.
 * @return Returns no value.
 */
EAPI void evas_textblock_cursor_free(Evas_Textblock_Cursor *cur);

/**
 * Adds text to the current cursor position and set the cursor to *before*
 * the start of the text just added.
 *
 * @param cur the cursor to where to add text at.
 * @param text the text to add.
 * @return Returns the len of the text added.
 * @see evas_textblock_cursor_text_prepend()
 */
EAPI int evas_textblock_cursor_text_append(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * Adds text to the current cursor position and set the cursor to *after*
 * the start of the text just added.
 *
 * @param cur the cursor to where to add text at.
 * @param text the text to add.
 * @return Returns the len of the text added.
 * @see evas_textblock_cursor_text_append()
 */
EAPI int evas_textblock_cursor_text_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

/**
 * @brief The "replacement character" to use for the given textblock object.
 *
 * @param[in] ch The charset name.
 *
 * @ingroup Efl_Canvas_Text
 */
EAPI void evas_object_textblock_replace_char_set(Evas_Object *obj, const char *ch);

/**
 * @brief The "replacement character" to use for the given textblock object.
 *
 * @return The charset name.
 *
 * @ingroup Efl_Canvas_Text
 */
EAPI const char *evas_object_textblock_replace_char_get(const Evas_Object *obj);

/**
 * @brief The vertical alignment of text within the textblock object as a
 * whole.
 *
 * Normally alignment is 0.0 (top of object). Values given should be between
 * 0.0 and 1.0 (1.0 bottom of object, 0.5 being vertically centered etc.).
 *
 * @param[in] align The alignment set for the object.
 *
 * @since 1.1
 *
 * @ingroup Efl_Canvas_Text
 */
EAPI void evas_object_textblock_valign_set(Evas_Object *obj, double align);

/**
 * @brief The vertical alignment of text within the textblock object as a
 * whole.
 *
 * Normally alignment is 0.0 (top of object). Values given should be between
 * 0.0 and 1.0 (1.0 bottom of object, 0.5 being vertically centered etc.).
 *
 * @return The alignment set for the object.
 *
 * @since 1.1
 *
 * @ingroup Efl_Canvas_Text
 */
EAPI double evas_object_textblock_valign_get(const Evas_Object *obj);

/**
 * Sets the cursor to the start of the first text node.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void evas_textblock_cursor_paragraph_first(Evas_Textblock_Cursor *cur);

/**
 * sets the cursor to the end of the last text node.
 *
 * @param cur the cursor to set.
 * @return Returns no value.
 */
EAPI void evas_textblock_cursor_paragraph_last(Evas_Textblock_Cursor *cur);

/**
 * Compare two cursors.
 *
 * @param cur1 the first cursor.
 * @param cur2 the second cursor.
 * @return -1 if cur1 < cur2, 0 if cur1 == cur2 and 1 otherwise.
 */
EAPI int evas_textblock_cursor_compare(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2);

/**
 * @brief Checks if two cursors are equal
 *
 * This is faster than @ref evas_textblock_cursor_compare so it should be used
 * if all we care about is equality.
 *
 * @param[in] cur The second cursor.
 *
 * @return @c true if equal
 *
 * @since 1.18
 */
EAPI Eina_Bool evas_textblock_cursor_equal(const Evas_Textblock_Cursor *obj, const Evas_Textblock_Cursor *cur);

/**
 * Go to the start of the current line
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void evas_textblock_cursor_line_char_first(Evas_Textblock_Cursor *cur);

/**
 * Go to the end of the current line.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void evas_textblock_cursor_line_char_last(Evas_Textblock_Cursor *cur);

/**
 * Set the cursor pos.
 *
 * @param cur the cursor to be set.
 * @param pos the pos to set.
 */
EAPI void evas_textblock_cursor_pos_set(Evas_Textblock_Cursor *cur, int _pos);

/**
 * Go to the first char in the node the cursor is pointing on.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void evas_textblock_cursor_paragraph_char_first(Evas_Textblock_Cursor *cur);

/**
 * Go to the last char in a text node.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void evas_textblock_cursor_paragraph_char_last(Evas_Textblock_Cursor *cur);

/**
 * Deletes a single character from position pointed by given cursor.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void evas_textblock_cursor_char_delete(Evas_Textblock_Cursor *cur);

/** Get the object's main cursor.
 *
 * @ingroup Evas_Textblock
 */
EAPI Evas_Textblock_Cursor *evas_object_textblock_cursor_get(const Evas_Object *obj);
#include "canvas/efl_canvas_text.eo.legacy.h"
/**
 * @}
 */
