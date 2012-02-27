/**
 * @defgroup CopyPaste CopyPaste
 *
 * Implements the following functionality
 *    a. select, copy/cut and paste 
 *    b. clipboard
 *    c. drag and drop 
 * in order to share data across application windows.
 *
 * Contains functions to select a portion of text, send it to a buffer,
 * and paste the selection into a target.
 *
 * @{
 */

/**
 * Defines the types of selection property names.
 * @see http://www.x.org/docs/X11/xlib.pdf
 * for more details.
 */
typedef enum
{
   ELM_SEL_TYPE_PRIMARY, /**< Primary text selection (middle mouse) */
   ELM_SEL_TYPE_SECONDARY, /**< Used when primary selection is in use */
   ELM_SEL_TYPE_XDND, /**< Drag 'n' Drop */
   ELM_SEL_TYPE_CLIPBOARD, /**< Clipboard selection (ctrl+C)
} Elm_Sel_Type;

/**
 * Defines the types of content.
 */
typedef enum
{
   /** For matching every possible atom */
   ELM_SEL_FORMAT_TARGETS = -1,
   /** Content is from outside of Elementary */
   ELM_SEL_FORMAT_NONE = 0x0,
   /** Plain unformatted text: Used for things that don't want rich markup */
   ELM_SEL_FORMAT_TEXT = 0x01,
   /** Edje textblock markup, including inline images */
   ELM_SEL_FORMAT_MARKUP = 0x02,
   /** Images */
   ELM_SEL_FORMAT_IMAGE = 0x04,
   /** Vcards */
   ELM_SEL_FORMAT_VCARD = 0x08,
   /** Raw HTML-like data (eg. webkit) */
   ELM_SEL_FORMAT_HTML = 0x10,
} Elm_Sel_Format;

/**
 * Structure holding the info about selected data.
 */
struct _Elm_Selection_Data
{
   Evas_Coord     x, y;
   Elm_Sel_Format format;
   void          *data;
   size_t         len;
};
typedef struct _Elm_Selection_Data Elm_Selection_Data;

/**
 * Callback invoked in when the selected data is 'dropped' at its destination.
 *
 * @param data Application specific data
 * @param obj The evas object where selected data is 'dropped'.
 * @param ev struct holding information about selected data
 * FIXME: this should probably be a smart callback
 */
typedef Eina_Bool (*Elm_Drop_Cb)(void *data, Evas_Object *obj, Elm_Selection_Data *ev);


/**
 * @brief Set copy and paste data for a widget.
 *
 * XXX: need to be rewritten.
 * Append the given callback to the list.
 *
 * @param selection Selection type for copying and pasting
 * @param obj The source widget pointer
 * @param format Selection format
 * @param buf The data selected
 * @param buflen The size of @p buf
 * @return If EINA_TRUE, setting data was successful.
 *
 * @ingroup CopyPaste
 *
 */
// XXX: EAPI void elm_object_cnp_selection_set(Evas_Object *obj, Elm_Sel_Type selection,
//                                             Elm_Sel_Format format, const void *buf,
//                                             size_t buflen);
EAPI Eina_Bool elm_cnp_selection_set(Elm_Sel_Type selection, Evas_Object *obj,
                                     Elm_Sel_Format format, const void *buf,
                                     size_t buflen);

/**
 * @brief Retrieve selection data from a widget.
 *
 * Gets the current selection data from a widget.
 * The widget input here will usually be elm_entry,
 * in which case @p datacb and @p udata can be NULL.
 * If a different widget is passed, @p datacb and @p udata are used for retrieving data.
 *
 * @see also elm_cnp_selection_set()
 *
 * @param selection Selection type for copying and pasting
 * @param format Selection format
 * @param obj The source widget
 * @param datacb The user data callback if the target widget isn't elm_entry
 * @param udata The user data pointer for @p datacb
 * @return If EINA_TRUE, getting selection data was successful.
 *
 * @ingroup CopyPaste
 *
 */
// XXX: This api needs to be refined by cnp experts.
//      I suggest:
//         1. return copy and paste data.
//         2. call cnp callback regardless of widget type.
//         3. apps insert text data into entry manually.
// XXX: EAPI void *elm_object_cnp_selection_get(Evas_Object *obj,
//                                              Elm_Sel_Type selection,
//                                              Elm_Sel_Format format,
//                                              Elm_Cnp_Cb datacb);
EAPI Eina_Bool elm_cnp_selection_get(Elm_Sel_Type selection,
                                     Elm_Sel_Format format, Evas_Object *obj,
                                     Elm_Drop_Cb datacb, void *udata);

/**
 * @brief Clear the selection data of a widget.
 *
 * @see also elm_cnp_selection_set()
 *
 * @param obj The source widget
 * @param selection Selection type for copying and pasting
 * @return If EINA_TRUE, clearing data was successful.
 *
 * @ingroup CopyPaste
 *
 */
EAPI Eina_Bool elm_object_cnp_selection_clear(Evas_Object *obj, 
                                               Elm_Sel_Type selection);

/**
 * @}
 */
