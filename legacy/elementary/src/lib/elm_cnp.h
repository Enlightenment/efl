/**
 * @defgroup CopyPaste
 *
 * Implement the copy and paste + clipboard functionality, in order to 
 * share data across application windows.
 *
 * Contains functions to select a portion of text, stick it to a clipboard
 * and to paste the selection to an appropriate place.
 *
 *
 *
 * @{
 */
typedef struct _Elm_Selection_Data Elm_Selection_Data;
typedef Eina_Bool                (*Elm_Drop_Cb)(void *d, Evas_Object *o, Elm_Selection_Data *data);

/**
 * Defines the types of selection property names.
 * Kindly refer to http://www.x.org/docs/X11/xlib.pdf
 * for more details.
 */
typedef enum
{
   ELM_SEL_TYPE_PRIMARY, //primary text selection
   ELM_SEL_TYPE_SECONDARY, // used when primary selection is in use.
   ELM_SEL_TYPE_XDND, //drag and drop
   ELM_SEL_TYPE_CLIPBOARD, // highlighted text 
} Elm_Sel_Type;

typedef enum
{
   /** Targets: for matching every atom requesting */
   ELM_SEL_FORMAT_TARGETS = -1,
   /** they come from outside of elm */
   ELM_SEL_FORMAT_NONE = 0x0,
   /** Plain unformatted text: Used for things that don't want rich markup */
   ELM_SEL_FORMAT_TEXT = 0x01,
   /** Edje textblock markup, including inline images */
   ELM_SEL_FORMAT_MARKUP = 0x02,
   /** Images */
   ELM_SEL_FORMAT_IMAGE = 0x04,
   /** Vcards */
   ELM_SEL_FORMAT_VCARD = 0x08,
   /** Raw HTML-like things for widgets that want that stuff (hello webkit!) */
   ELM_SEL_FORMAT_HTML = 0x10,
} Elm_Sel_Format;

struct _Elm_Selection_Data
{
   int            x, y;
   Elm_Sel_Format format;
   void          *data;
   size_t         len;
};

/**
 * @brief Set copy and paste data to a widget.
 *
 * XXX: need to be rewritten.
 * Append the given callback to the list. This functions will be called.
 *
 * @param selection Selection type for copying and pasting
 * @param obj The source widget pointer
 * @param format Type of selection format
 * @param buf The pointer of data source
 * @return If EINA_TRUE, setting data is success.
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
 * @brief Retrieve the copy and paste data from the widget.
 *
 * Gets the data from the widget which is set for copying and pasting.
 * Mainly the widget is elm_entry. If then @p datacb and @p udata can be NULL.
 * If not, @p datacb and @p udata are used for retrieving data.
 *
 * @see also elm_cnp_selection_set()
 *
 * @param selection Selection type for copying and pasting
 * @param format Type of selection format
 * @param obj The source widget pointer
 * @param datacb The user data callback if the target widget isn't elm_entry
 * @param udata The user data pointer for @p datacb
 * @return If EINA_TRUE, getting data is success.
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
 * @brief Clear the copy and paste data in the widget.
 *
 * Clear the data in the widget. Normally this function isn't need to call.
 *
 * @see also elm_cnp_selection_set()
 *
 * @param selection Selection type for copying and pasting
 * @param obj The source widget pointer
 * @return If EINA_TRUE, clearing data is success.
 *
 * @ingroup CopyPaste
 *
 */
// XXX: EAPI void elm_object_cnp_selection_clear(Evas_Object *obj,
//                                               Elm_Sel_Type selection);
EAPI Eina_Bool elm_cnp_selection_clear(Elm_Sel_Type selection,
                                       Evas_Object *obj);

/**
 * @}
 */
