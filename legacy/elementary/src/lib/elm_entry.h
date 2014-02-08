/**
 * @defgroup Entry Entry
 * @ingroup Elementary
 *
 * @image html entry_inheritance_tree.png
 * @image latex entry_inheritance_tree.eps
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
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for entry objects (@since 1.8).
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base
 * @ref Scroller widget also work for entries (@since 1.8).
 *
 * Some calls on the entry's API are marked as @b deprecated, as they
 * just wrap the scrollable widgets counterpart functions. Use the
 * ones we point you to, for each case of deprecation here, instead --
 * eventually the deprecated ones will be discarded (next major
 * release).
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
 * @li \<title\>...\</title\>: Main title.
 * @li \<subtitle\>...\</subtitle\>: Secondary level title.
 * @li \<bigger\>...\</bigger\>: A really big text, not so big as the titles.
 * @li \<big\>...\</big\>: Big text.
 * @li \<small\>...\</small\>: Small text.
 * @li \<smaller\>...\</smaller\>: Really small text, at the point of unreadability.
 *
 * Entry also support tags for code syntax highlight. Note that this does not
 * mean that the entry will automatically perform code highlight, application
 * are responsable of applying the correct tag to code blocks.
 * The default theme define the following tags:
 * @li \<code\>...\</code\>: Monospace font without shadows.
 * @li \<comment\>...\</comment\>: Code comments. 
 * @li \<string\>...\</string\>: Strings of text.
 * @li \<number\>...\</number\>: Numeric expression (ex: 1, 2, 0.34, etc)
 * @li \<brace\>...\</brace\>: Braces used for code syntax.
 * @li \<type\>...\</type\>: Variables types (ex: int, float, char, Evas_Object, etc)
 * @li \<class\>...\</class\>: Class names, when defined, not when used.
 * @li \<function\>...\</function\>: Function names, when defined, not called.
 * @li \<param\>...\</param\>: Generic parameters.
 * @li \<keyword\>...\</keyword\>: Language keywords (ex: return, NULL, while, for, etc)
 * @li \<preprocessor\>...\</preprocessor\>: Preprocessors definitions.
 * @li \<line_added\>...\</line_added\>: Diff addeded lines.
 * @li \<line_removed\>...\</line_removed\>: Diff removed lines.
 * @li \<line_changed\>...\</line_changed\>: Diff changed lines.
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
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li "aborted": The escape key was pressed on a single line entry. (since 1.7)
 * @li "activated": The enter key was pressed on a single line entry.
 * @li "anchor,clicked": An anchor has been clicked. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "anchor,down": Mouse button has been pressed on an anchor. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "anchor,hover,opened": The anchor is clicked.
 * @li "anchor,in": Mouse cursor has moved into an anchor. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "anchor,out": Mouse cursor has moved out of an anchor. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "anchor,up": Mouse button has been unpressed on an anchor. The event_info
 * parameter for the callback will be an #Elm_Entry_Anchor_Info.
 * @li "changed": The text within the entry was changed.
 * @li "changed,user": The text within the entry was changed because of user interaction.
 * @li "clicked": The entry has been clicked (mouse press and release).
 * @li "clicked,double": The entry has been double clicked.
 * @li "clicked,triple": The entry has been triple clicked.
 * @li "cursor,changed": The cursor has changed position.
 * @li "cursor,changed,manual": The cursor has changed position manually.
 * @li "focused": The entry has received focus.
 * @li "unfocused": The entry has lost focus.
 * @li "language,changed": Program language changed.
 * @li "longpressed": A mouse button has been pressed and held for a couple
 * @li "maxlength,reached": Called when a maximum length is reached.
 * @li "preedit,changed": The preedit string has changed.
 * @li "press": A mouse button has been pressed on the entry.
 * seconds.
 * @li "redo,request": Called on redo request.
 * @li "selection,changed": The current selection has changed.
 * @li "selection,cleared": The current selection has been cleared.
 * @li "selection,copy": A copy of the selected text into the clipboard was
 * requested.
 * @li "selection,cut": A cut of the selected text into the clipboard was
 * requested.
 * @li "selection,paste": A paste of the clipboard contents was requested.
 * @li "selection,start": A selection has begun and no previous selection
 * existed.
 * @li "text,set,done": Whole text has been set to the entry.
 * @li "theme,changed": Called when the theme is changed.
 * @li "undo,request": Called on undo request.
 * @li "rejected": Called when some of inputs are rejected by the filter. (since 1.9)
 *
 * Default content parts of the entry items that you can use for are:
 * @li "icon" - An icon in the entry
 * @li "end" - A content in the end of the entry
 *
 * Default text parts of the entry that you can use for are:
 * @li "default" - text of the entry
 * @li "guide" - placeholder of the entry
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

#include "elm_entry_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_entry_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_entry_legacy.h"
#endif
/**
 * @}
 */
