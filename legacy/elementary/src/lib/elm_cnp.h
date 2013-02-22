/**
 * @defgroup CopyPaste CopyPaste
 * @ingroup Elementary
 *
 * Implements the following functionality
 *    a. select, copy/cut and paste
 *    b. clipboard
 *    c. drag and drop
 * in order to share data across application windows.
 *
 * Contains functions to select text or a portion of data,
 * send it to a buffer, and paste the data into a target.
 *
 * elm_cnp provides a generic copy and paste facility based on its windowing system.
 * It is not necessary to know the details of each windowing system,
 * but some terms and behavior are common.
 * Currently the X11 window system is widely used, and only X11 functionality is implemented.
 *
 * In X11R6 window system, CopyPaste works like a peer-to-peer communication.
 * Copying is an operation on an object in an X server.
 * X11 calls those objects 'selections' which have names.
 * Generally, two selection types are needed for copy and paste:
 * The Primary selection and the Clipboard selection.
 * Primary selection is for selecting text (that means highlighted text).
 * Clipboard selection is for explicit copying behavior
 * (such as ctrl+c, or 'copy' in a menu).
 * Thus, in applications most cases only use the clipboard selection.
 * As stated before, taking ownership of a selection doesn't move any actual data.
 * Copying and Pasting is described as follows:
 *  1. Copy text in Program A : Program A takes ownership of the selection
 *  2. Paste text in Program B : Program B notes that Program A owns the selection
 *  3. Program B asks A for the text
 *  4. Program A responds and sends the text to program B
 *  5. Program B pastes the response
 * More information is on
 *  - http://www.jwz.org/doc/x-cut-and-paste.html
 *  - X11R6 Inter-Client Communication Conventions Manual, section 2
 *
 * TODO: add for other window system.
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
   ELM_SEL_TYPE_PRIMARY, /**< Primary text selection (highlighted or selected text) */
   ELM_SEL_TYPE_SECONDARY, /**< Used when primary selection is in use */
   ELM_SEL_TYPE_XDND, /**< Drag 'n' Drop */
   ELM_SEL_TYPE_CLIPBOARD, /**< Clipboard selection (ctrl+C) */
} Elm_Sel_Type;

/**
 * Defines the types of content.
 */
typedef enum
{
   /** For matching every possible atom */
   ELM_SEL_FORMAT_TARGETS =   -1,
   /** Content is from outside of Elementary */
   ELM_SEL_FORMAT_NONE    =  0x0,
   /** Plain unformatted text: Used for things that don't want rich markup */
   ELM_SEL_FORMAT_TEXT    = 0x01,
   /** Edje textblock markup, including inline images */
   ELM_SEL_FORMAT_MARKUP  = 0x02,
   /** Images */
   ELM_SEL_FORMAT_IMAGE   = 0x04,
   /** Vcards */
   ELM_SEL_FORMAT_VCARD   = 0x08,
   /** Raw HTML-like data (eg. webkit) */
   ELM_SEL_FORMAT_HTML    = 0x10,
} Elm_Sel_Format;

/**
 * Defines the kind of action associated with the drop data if for XDND
 * @since 1.8
 */
typedef enum
{
   ELM_XDND_ACTION_UNKNOWN, /**< Action type is unknown */
   ELM_XDND_ACTION_COPY, /**< Copy the data */
   ELM_XDND_ACTION_MOVE, /**< Move the data */
   ELM_XDND_ACTION_PRIVATE, /**< Pricate action type */
   ELM_XDND_ACTION_ASK, /**< Ask the user what to do */
   ELM_XDND_ACTION_LIST, /**< List the data */
   ELM_XDND_ACTION_LINK, /**< Link the data */
   ELM_XDND_ACTION_DESCRIPTION /**< Describe the data */
} Elm_Xdnd_Action;

/**
 * Structure holding the info about selected data.
 */
struct _Elm_Selection_Data
{
   Evas_Coord       x, y;
   Elm_Sel_Format   format;
   void            *data;
   size_t           len;
   Elm_Xdnd_Action  action; /**< The action to perform with the data @since 1.8 */
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
 * Callback invoked in when the selection ownership for a given selection is lost.
 *
 * @param data Application specific data
 * @param selection The selection that is lost
 * @since 1.7
 */
typedef void (*Elm_Selection_Loss_Cb)(void *data, Elm_Sel_Type selection);

/**
 * Callback called to create a drag icon object
 * 
 * @param data Application specific data
 * @param win The window to create the objects relative to
 * @param xoff A return coordinate for the X offset at which to place the drag icon object relative to the source drag object
 * @param yoff A return coordinate for the Y offset at which to place the drag icon object relative to the source drag object
 * @return An object to fill the drag window with or NULL if not needed
 * @since 1.8
 */
typedef Evas_Object *(*Elm_Drag_Icon_Create_Cb) (void *data, Evas_Object *win, Evas_Coord *xoff, Evas_Coord *yoff);

/**
 * Callback called when a drag is finished, enters, or leaves an object
 * 
 * @param data Application specific data
 * @param obj The object where the drag started
 * @since 1.8
 */
typedef void (*Elm_Drag_State) (void *data, Evas_Object *obj);

/**
 * Callback called when a drag is responded to with an accept or deny
 * 
 * @param data Application specific data
 * @param obj The object where the drag started
 * @param doaccept A boolean as to if the target accepts the drag or not
 * @since 1.8
 */
typedef void (*Elm_Drag_Accept) (void *data, Evas_Object *obj, Eina_Bool doaccept);

/**
 * Callback called when a drag is over an object, and gives object-relative coordinates
 * 
 * @param data Application specific data
 * @param obj The object where the drag started
 * @param x The X coordinate relative to the top-left of the object
 * @param y The Y coordinate relative to the top-left of the object
 * @since 1.8
 */
typedef void (*Elm_Drag_Pos) (void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y, Elm_Xdnd_Action action);

/**
 * @brief Set copy data for a widget.
 *
 * Set copy data and take ownership of selection. Format is used for specifying the selection type,
 * and this is used during pasting.
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
EAPI Eina_Bool elm_cnp_selection_set(Evas_Object *obj, Elm_Sel_Type selection,
                                     Elm_Sel_Format format,
                                     const void *buf, size_t buflen);

/**
 * @brief Retrieve data from a widget that has a selection.
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
 */
EAPI Eina_Bool elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection,
                                     Elm_Sel_Format format,
                                     Elm_Drop_Cb datacb, void *udata);

/**
 * @brief Clear the selection data of a widget.
 *
 * Clear all data from the selection which is owned by a widget.
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
 * @brief Set a function to be called when a selection is lost
 *
 * The function @p func is set of be called when selection @p selection is lost
 * to another process or when elm_cnp_selection_set() is called. If @p func
 * is NULL then it is not called. @p data is passed as the data parameter to
 * the callback functions and selection is passed in as the selection that
 * has been lost.
 * 
 * elm_cnp_selection_set() and elm_object_cnp_selection_clear() automatically
 * set this los callback to NULL when called. If you wish to take the selection
 * and then be notified of loss please do this (for example):
 * 
 * @code
 * elm_cnp_selection_set(obj, ELM_SEL_TYPE_PRIMARY, ELM_SEL_FORMAT_TEXT, "hello", strlen(hello));
 * elm_cnp_selection_loss_callback_set(obj, ELM_SEL_TYPE_PRIMARY, loss_cb, NULL);
 * @endcode
 *
 * @see also elm_cnp_selection_set()
 *
 * @param obj The object to indicate the window target/display system.
 * @param selection Selection to be notified of for loss
 * @param func The function to call
 * @param data The data pointer passed to the function.
 *
 * @ingroup CopyPaste
 *
 * @since 1.7
 */
EAPI void elm_cnp_selection_loss_callback_set(Evas_Object *obj, Elm_Sel_Type selection, Elm_Selection_Loss_Cb func, const void *data);

/**
 * @brief Set the given object as a target for drops for drag-and-drop
 *
 * @param obj The target object
 * @param format The formats supported for dropping
 * @param entercb The function to call when the object is entered with a drag
 * @param enterdata The application data to pass to enterdata
 * @param leavecb The function to call when the object is left with a drag
 * @param leavedata The application data to pass to leavedata
 * @param poscb The function to call when the object has a drag over it
 * @param posdata The application data to pass to posdata
 * @param dropcb The function to call when a drop has occurred
 * @param cbdata The application data to pass to dropcb
 * @return Returns EINA_TRUE, if successful, or EINA_FALSE if not.
 *
 * @ingroup CopyPaste
 *
 * @since 1.8
 */
EAPI Eina_Bool elm_drop_target_add(Evas_Object *obj, Elm_Sel_Format format, 
                                   Elm_Drag_State entercb, void *enterdata,
                                   Elm_Drag_State leavecb, void *leavedata,
                                   Elm_Drag_Pos poscb, void *posdata,
                                   Elm_Drop_Cb dropcb, void *cbdata);

/**
 * @brief Deletes the drop target status of an object
 *
 * @param obj The target object
 * @return Returns EINA_TRUE, if successful, or EINA_FALSE if not.
 *
 * @ingroup CopyPaste
 *
 * @since 1.8
 */
EAPI Eina_Bool elm_drop_target_del(Evas_Object *obj);

/**
 * @brief Begins a drag given a source object
 *
 * @param obj The source object
 * @param format The drag formats supported by the data
 * @param data The drag data itself (a string)
 * @param action The drag action to be done
 * @param createicon Function to call to create a drag object, or NULL if not wanted
 * @param createdata Application data passed to @p createicon
 * @param dragpos Function called with each position of the drag, x, y being screen coordinates if possible, and action being the current action. Do not change action inside theis callback (defer it to another).
 * @param dragdata Application data passed to @p dragpos
 * @param acceptcb Function called indicating if drop target accepts (or does not) the drop data while dragging
 * 
 * @param acceptdata Application data passed to @p acceptcb
 * @param dragdone Function to call when drag is done
 * @param donecbdata Application data to pass to @p dragdone
 * @return Returns EINA_TRUE, if successful, or EINA_FALSE if not.
 *
 * @ingroup CopyPaste
 *
 * @since 1.8
 */
EAPI Eina_Bool elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, 
                              const char *data, Elm_Xdnd_Action action, 
                              Elm_Drag_Icon_Create_Cb createicon, void *createdata, 
                              Elm_Drag_Pos dragpos, void *dragdata,
                              Elm_Drag_Accept acceptcb, void *acceptdata,
                              Elm_Drag_State dragdone, void *donecbdata);
/**
 * @brief Changes the current drag action
 *
 * @param obj The source of a drag if a drag is underway 
 * @param action The drag action to be done
 * @return Returns EINA_TRUE, if successful, or EINA_FALSE if not.
 *
 * @ingroup CopyPaste
 *
 * @since 1.8
 */
EAPI Eina_Bool elm_drag_action_set(Evas_Object *obj, Elm_Xdnd_Action action);

/**
 * @}
 */
