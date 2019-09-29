#ifndef _EFL2_TEXT_RAW_EDITABLE_EO_H_
#define _EFL2_TEXT_RAW_EDITABLE_EO_H_

#ifndef _EFL2_TEXT_RAW_EDITABLE_EO_CLASS_TYPE
#define _EFL2_TEXT_RAW_EDITABLE_EO_CLASS_TYPE

typedef Eo Efl2_Text_Raw_Editable;

#endif

#ifndef _EFL2_TEXT_RAW_EDITABLE_EO_TYPES
#define _EFL2_TEXT_RAW_EDITABLE_EO_TYPES

#ifdef EFL_BETA_API_SUPPORT
/**
 * @brief This structure includes all the information about content changes.
 *
 * It's meant to be used to implement undo/redo.
 *
 * @ingroup Efl2_Text
 */
typedef struct _Efl2_Text_Change_Info
{
  const char *content; /**< The content added/removed */
  size_t position; /**< The position where it was added/removed */
  size_t length; /**< The length of content in characters (not bytes, actual
                  * unicode characters) */
  Eina_Bool insert; /**< @c true if the content was inserted, @c false if
                     * removed */
  Eina_Bool merge; /**< @c true if can be merged with the previous one. Used for
                    * example with insertion when something is already selected
                    */
} Efl2_Text_Change_Info;
#endif /* EFL_BETA_API_SUPPORT */


#endif
#ifdef EFL_BETA_API_SUPPORT
#define EFL2_TEXT_RAW_EDITABLE_CLASS efl2_text_raw_editable_class_get()

EWAPI const Efl_Class *efl2_text_raw_editable_class_get(void);

/**
 * @brief Whether the entry is editable.
 *
 * By default text interactives are editable. However setting this property to
 * @c false will make it so that key input will be disregarded.
 *
 * @param[in] obj The object.
 * @param[in] editable If @c true, user input will be inserted in the entry, if
 * not, the entry is read-only and no user input is allowed.
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI void efl2_text_raw_editable_set(Eo *obj, Eina_Bool editable);

/**
 * @brief Whether the entry is editable.
 *
 * By default text interactives are editable. However setting this property to
 * @c false will make it so that key input will be disregarded.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, user input will be inserted in the entry, if not, the
 * entry is read-only and no user input is allowed.
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI Eina_Bool efl2_text_raw_editable_get(const Eo *obj);

/**
 * @brief Whether text is a password
 *
 * @param[in] obj The object.
 * @param[in] enabled @c true if the text is a password, @c false otherwise
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI void efl2_text_raw_editable_password_mode_set(Eo *obj, Eina_Bool enabled);

/**
 * @brief Whether text is a password
 *
 * @param[in] obj The object.
 *
 * @return @c true if the text is a password, @c false otherwise
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI Eina_Bool efl2_text_raw_editable_password_mode_get(const Eo *obj);

/**
 * @brief The character used to replace characters that can't be displayed
 *
 * Currently only used to replace characters if
 * @ref efl2_text_raw_editable_password_mode_get is enabled.
 *
 * @param[in] obj The object.
 * @param[in] repch Replacement character
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI void efl2_text_raw_editable_replacement_char_set(Eo *obj, const char *repch);

/**
 * @brief The character used to replace characters that can't be displayed
 *
 * Currently only used to replace characters if
 * @ref efl2_text_raw_editable_password_mode_get is enabled.
 *
 * @param[in] obj The object.
 *
 * @return Replacement character
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI const char *efl2_text_raw_editable_replacement_char_get(const Eo *obj);

/**
 * @brief The user visible cursor
 *
 * @param[in] obj The object.
 *
 * @return The user visible cursor
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI Efl2_Text_Cursor *efl2_text_raw_editable_main_cursor_get(const Eo *obj);

/**
 * @brief Whether or not selection is allowed on this object
 *
 * @param[in] obj The object.
 * @param[in] allowed @c true if enabled, @c false otherwise
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI void efl2_text_raw_editable_selection_allowed_set(Eo *obj, Eina_Bool allowed);

/**
 * @brief Whether or not selection is allowed on this object
 *
 * @param[in] obj The object.
 *
 * @return @c true if enabled, @c false otherwise
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI Eina_Bool efl2_text_raw_editable_selection_allowed_get(const Eo *obj);

/**
 * @brief The cursors used for selection handling.
 *
 * If the cursors are equal there's no selection.
 *
 * You are allowed to retain and modify them. Modifying them modifies the
 * selection of the object.
 *
 * @param[in] obj The object.
 * @param[out] start The start of the selection
 * @param[out] end The end of the selection
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI void efl2_text_raw_editable_selection_cursors_get(const Eo *obj, Efl2_Text_Cursor **start, Efl2_Text_Cursor **end);

/** This starts a selection within the entry as though the user had just
 * started selecting text.
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI void efl2_text_raw_editable_selection_start(Eo *obj);

/** This ends a selection within the entry as though the user had just released
 * the mouse button while making a selection.
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI void efl2_text_raw_editable_selection_end(Eo *obj);

/** Clears the selection.
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
EOAPI void efl2_text_raw_editable_select_none(Eo *obj);

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER_PRE;

/** The text object has changed due to user interaction - before the changes
 * are committed to the object
 * @return Efl2_Text_Change_Info
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER_PRE (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER_PRE))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER;

/** The text object has changed due to user interaction
 * @return Efl2_Text_Change_Info
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_REDO_REQUEST;

/** Called when redo is requested
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_REDO_REQUEST (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_REDO_REQUEST))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_UNDO_REQUEST;

/** Called when undo is requested
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_UNDO_REQUEST (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_UNDO_REQUEST))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_CURSOR_CHANGED_MANUAL;

/** Called on manual cursor change
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_CURSOR_CHANGED_MANUAL (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_CURSOR_CHANGED_MANUAL))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_CONTEXT_OPEN;

/** Called when context menu was opened
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_CONTEXT_OPEN (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_CONTEXT_OPEN))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_PREEDIT_CHANGED;

/** Called when entry preedit changed
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_PREEDIT_CHANGED (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_PREEDIT_CHANGED))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_START;

/** Called at selection start
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_START (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_START))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CHANGED;

/** Called when selection is changed
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CHANGED (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CHANGED))

EWAPI extern const Efl_Event_Description _EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CLEARED;

/** Called when selection is cleared
 *
 * @ingroup Efl2_Text_Raw_Editable
 */
#define EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CLEARED (&(_EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CLEARED))
#endif /* EFL_BETA_API_SUPPORT */

#endif
