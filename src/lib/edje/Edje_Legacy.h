/**
 * @brief Instantiates a new Edje object.
 *
 * @param evas A valid Evas handle, the canvas to place the new object
 * in
 * @return A handle to the new object created, or @c NULL on errors.
 *
 * This function creates a new Edje smart object, returning its @c
 * Evas_Object handle. An Edje object is useless without a (source)
 * file set to it, so you'd most probably call edje_object_file_set()
 * afterwards, like in:
 * @code
 * Evas_Object *edje;
 *
 * edje = edje_object_add(canvas);
 * if (!edje)
 *   {
 *      fprintf(stderr, "could not create edje object!\n");
 *      return NULL;
 *   }
 *
 * if (!edje_object_file_set(edje, "theme.edj", "group_name"))
 *   {
 *      int err = edje_object_load_error_get(edje);
 *      const char *errmsg = edje_load_error_str(err);
 *      fprintf(stderr, "could not load 'group_name' from theme.edj: %s",
 *      	errmsg);
 *
 *      evas_object_del(edje);
 *      return NULL;
 *   }
 *
 * @endcode
 *
 * @note You can get a callback every time edje re-calculates the object
 * (either due to animation or some kind of signal or input). This is called
 * in-line just after the recalculation has occurred. It is a good idea not
 * to go and delete or alter the object inside this callbacks, simply make
 * a note that the recalculation has taken place and then do something about
 * it outside the callback. To register a callback use code like:
 *
 * @code
 *    evas_object_smart_callback_add(edje_obj, "recalc", my_cb, my_cb_data);
 * @endcode
 *
 * @see evas_object_smart_callback_add()
 *
 * @note Before creating the first Edje object in your code, remember
 * to initialize the library, with edje_init(), or unexpected behavior
 * might occur.
 */
EAPI Evas_Object *edje_object_add                 (Evas *evas);

/**
 * @brief Preloads the images on the Edje Object in the background.
 *
 * This function requests the preload of all data images (on the given object)
 * in the background. The work is queued before being processed (because there
 * might be other pending requests of this type). It emits a signal
 * "preload,done" when finished.
 *
 * @note Use @c true on scenarios where you don't need the image data preloaded
 * anymore.
 *
 * @param[in] obj The object.
 * @param[in] cancel @c false will add it the preloading work queue, @c true
 * will remove it (if it was issued before).
 *
 * @return @c false if obj was not a valid Edje object otherwise @c true
 */
EAPI Eina_Bool edje_object_preload(Evas_Object *obj, Eina_Bool cancel);

/**
 * @brief Adds a callback for an arriving Edje signal, emitted by a given Edje
 * object.
 *
 * Edje signals are one of the communication interfaces between code and a
 * given Edje object's theme. With signals, one can communicate two string
 * values at a time, which are: - "emission" value: the name of the signal, in
 * general - "source" value: a name for the signal's context, in general
 *
 * Though there are those common uses for the two strings, one is free to use
 * them however they like.
 *
 * Signal callback registration is powerful, in the way that  blobs may be used
 * to match multiple signals at once. All the "*?[\" set of @c fnmatch()
 * operators can be used, both for emission and source.
 *
 * Edje has  internal signals it will emit, automatically, on various actions
 * taking place on group parts. For example, the mouse cursor being moved,
 * pressed, released, etc., over a given part's area, all generate individual
 * signals.
 *
 * By using something like edje_object_signal_callback_add(obj, "mouse,down,*",
 * "button.*", signal_cb, NULL); being @ref "button.*" the pattern for the
 * names of parts implementing buttons on an interface, you'd be registering
 * for notifications on events of mouse buttons being pressed down on either of
 * those parts (those events all have the @"mouse,down," common prefix on their
 * names, with a suffix giving the button number). The actual emission and
 * source strings of an event will be passed in as the  emission and  source
 * parameters of the callback function (e.g. "mouse,down,2" and
 * @"button.close"), for each of those events.
 *
 * @note See @ref edcref "the syntax" for EDC files See also
 * @ref edje_object_signal_emit() on how to emits Edje signals from code to a
 * an object @ref edje_object_signal_callback_del_full()
 *
 * @param[in] emission The signal's "emission" string
 * @param[in] source The signal's "source" string
 * @param[in] func The callback function to be executed when the signal is
 * emitted.
 * @param[in] data A pointer to data to pass in to func.
 */
EAPI void edje_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Sends/emits an Edje signal to a given Edje object
 *
 * This function sends a signal to the object  obj. An Edje program, at obj's
 * EDC specification level, can respond to a signal by having declared matching
 * @'signal' and @'source' fields on its block (see @ref edcref "the syntax"
 * for EDC files).
 *
 * See also @ref edje_object_signal_callback_add() for more on Edje signals.
 *
 * @param[in] emission The signal's "emission" string
 * @param[in] source The signal's "source" string
 */
EAPI void edje_object_signal_emit(Evas_Object *obj, const char *emission, const char *source);

/**
 * @brief Removes a signal-triggered callback from an object.
 *
 * @param obj A valid Evas_Object handle.
 * @param emission The emission string.
 * @param source The source string.
 * @param func The callback function.
 * @return The data pointer
 *
 * This function removes a callback, previously attached to the
 * emission of a signal, from the object @a obj. The parameters @a
 * emission, @a source and @a func must match exactly those passed to
 * a previous call to edje_object_signal_callback_add(). The data
 * pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del_full().
 *
 */
EAPI void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);

/**
 * @brief Unregisters/deletes a callback set for an arriving Edje
 * signal, emitted by a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 * @param func The callback function passed on the callback's
 * registration
 * @param data The pointer given to be passed as data to @p func
 * @return @p data on success, or @c NULL on errors (or if @p data
 * had this value)
 *
 * This function removes a callback, previously attached to the
 * emission of a signal, from the object @a obj. The parameters
 * @a emission, @a source, @a func and @a data must match exactly those
 * passed to a previous call to edje_object_signal_callback_add(). The
 * data pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del().
 *
 */
EAPI void        *edje_object_signal_callback_del_full(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/** Edje file loading error codes one can get - see edje_load_error_str() too. */
typedef enum
{
  EDJE_LOAD_ERROR_NONE = 0, /**< No error happened, the loading was successful */
  EDJE_LOAD_ERROR_GENERIC = 1, /**< A generic error happened during the loading */
  EDJE_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< The file pointed to did not exist */
  EDJE_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission to read the given file was denied */
  EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Resource allocation failed during the loading */
  EDJE_LOAD_ERROR_CORRUPT_FILE = 5, /**< The file pointed to was corrupt */
  EDJE_LOAD_ERROR_UNKNOWN_FORMAT = 6, /**< The file pointed to had an unknown format */
  EDJE_LOAD_ERROR_INCOMPATIBLE_FILE = 7, /**< The file pointed to is incompatible, i.e., it doesn't
                                          * match the library's current version's format */
  EDJE_LOAD_ERROR_UNKNOWN_COLLECTION = 8, /**< The group/collection set to load
                                           * from was not found in the file */
  EDJE_LOAD_ERROR_RECURSIVE_REFERENCE = 9 /**< The group/collection set to load from had
                                           * <b>recursive references</b> on its components */
} Edje_Load_Error;

/**
 * @brief Gets the (last) file loading error for a given Edje object
 *
 * This function is meant to be used after an Edje EDJ file loading, what takes
 * place with the edje_object_file_set() function. If that function does not
 * return @c true, one should check for the reason of failure with this one.
 *
 * @ref edje_load_error_str()
 *
 * @return The Edje loading error, one of: - #EDJE_LOAD_ERROR_NONE -
 * #EDJE_LOAD_ERROR_GENERIC - #EDJE_LOAD_ERROR_DOES_NOT_EXIST -
 * #EDJE_LOAD_ERROR_PERMISSION_DENIED -
 * #EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED - #EDJE_LOAD_ERROR_CORRUPT_FILE
 * - #EDJE_LOAD_ERROR_UNKNOWN_FORMAT - #EDJE_LOAD_ERROR_INCOMPATIBLE_FILE -
 * #EDJE_LOAD_ERROR_UNKNOWN_COLLECTION - #EDJE_LOAD_ERROR_RECURSIVE_REFERENCE
 */
EAPI Edje_Load_Error edje_object_load_error_get(const Evas_Object *obj);

/**
 * @brief Converts the given Edje file load error code into a string
 * describing it in English.
 *
 * @param error the error code, a value in ::Edje_Load_Error.
 * @return Always returns a valid string. If the given @p error is not
 *         supported, <code>"Unknown error"</code> is returned.
 *
 * edje_object_file_set() is a function which sets an error value,
 * afterwards, which can be fetched with
 * edje_object_load_error_get(). The function in question is meant
 * to be used in conjunction with the latter, for pretty-printing any
 * possible error cause.
 */
EAPI const char	      *edje_load_error_str	  (Edje_Load_Error error);

/**
 * @brief Retrieves the geometry of a given Edje part, in a given Edje object's
 * group definition, relative to the object's area.
 *
 * This function gets the geometry of an Edje part within its group. The x and
 * y coordinates are relative to the top left corner of the whole obj object's
 * area.
 *
 * @note Use @c null pointers on the geometry components you're not interested
 * in: they'll be ignored by the function.
 *
 * @note On failure, this function will make all non-$null geometry pointers'
 * pointed variables be set to zero.
 *
 * @param[in] part The Edje part's name
 * @param[out] x A pointer to a variable where to store the part's x coordinate
 * @param[out] y A pointer to a variable where to store the part's y coordinate
 * @param[out] w A pointer to a variable where to store the part's width
 * @param[out] h A pointer to a variable where to store the part's height
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_geometry_get(const Evas_Object *obj, const char * part, int *x, int *y, int *w, int *h);

/**
 * @brief Returns the state of the Edje part.
 *
 * @param[in] part The part name
 * @param[out] val_ret Part state value
 *
 * @return The part state: "default" for the default state "" for other states
 *
 * @ingroup Edje_Object
 */
EAPI const char *edje_object_part_state_get(const Evas_Object *obj, const char * part, double *val_ret);

/**
 * @brief Gets a handle to the Evas object implementing a given Edje part, in
 * an Edje object.
 *
 * This function gets a pointer of the Evas object corresponding to a given
 * part in the obj object's group.
 *
 * You should  never modify the state of the returned object (with @ref
 * evas_object_move() or @ref evas_object_hide() for example), because it's
 * meant to be managed by Edje, solely. You are safe to query information about
 * its current state (with evas_object_visible_get() or @ref
 * evas_object_color_get() for example), though.
 *
 * @note If the type of Edje part is GROUP, SWALLOW or EXTERNAL, returned
 * handle by this function will indicate nothing or transparent rectangle for
 * events. Use $.part_swallow_get() in that case.
 *
 * @param[in] part The Edje part's name
 *
 * @return A pointer to the Evas object implementing the given part, @c null on
 * failure (e.g. the given part doesn't exist)
 *
 * @ingroup Edje_Object
 */
EAPI const Efl_Canvas_Object *edje_object_part_object_get(const Evas_Object *obj, const char * part);

/**
 * @brief Whether this object updates its size hints automatically.
 *
 * By default edje doesn't set size hints on itself. If this property is set to
 * @c true, size hints will be updated after recalculation. Be careful, as
 * recalculation may happen often, enabling this property may have a
 * considerable performance impact as other widgets will be notified of the
 * size hints changes.
 *
 * A layout recalculation can be triggered by @ref edje_object_size_min_calc(),
 * @ref edje_object_size_min_restricted_calc(),
 * @ref edje_object_parts_extends_calc() or even any other internal event.
 *
 * Enable or disable auto-update of size hints.
 *
 * @param[in] update Whether or not update the size hints.
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_update_hints_set(Evas_Object *obj, Eina_Bool update);

/**
 * @brief Whether this object updates its size hints automatically.
 *
 * By default edje doesn't set size hints on itself. If this property is set to
 * @c true, size hints will be updated after recalculation. Be careful, as
 * recalculation may happen often, enabling this property may have a
 * considerable performance impact as other widgets will be notified of the
 * size hints changes.
 *
 * A layout recalculation can be triggered by @ref edje_object_size_min_calc(),
 * @ref edje_object_size_min_restricted_calc(),
 * @ref edje_object_parts_extends_calc() or even any other internal event.
 *
 * Whether this object updates its size hints automatically.
 *
 * @return Whether or not update the size hints.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_update_hints_get(const Evas_Object *obj);

/**
 * @brief Calculates the minimum required size for a given Edje object.
 *
 * This call works exactly as edje_object_size_min_restricted_calc(), with the
 * last two arguments set to 0. Please refer to its documentation, then.
 *
 * @param[out] minw The minimum required width (return value)
 * @param[out] minh The minimum required height (return value)
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_size_min_calc(Evas_Object *obj, int *minw, int *minh);

/**
 * @brief Calculates the minimum required size for a given Edje object.
 *
 * This call will trigger an internal recalculation of all parts of the object,
 * in order to return its minimum required dimensions for width and height. The
 * user might choose to impose those minimum sizes, making the resulting
 * calculation to get to values equal or larger than @c restrictedw and
 * @c restrictedh, for width and height, respectively.
 *
 * @note At the end of this call, the object  won't be automatically resized to
 * the new dimensions, but just return the calculated sizes. The caller is the
 * one up to change its geometry or not.
 *
 * @warning Be advised that invisible parts in the object will be taken into
 * account in this calculation.
 *
 * @param[out] minw The minimum required width (return value)
 * @param[out] minh The minimum required height (return value)
 * @param[in] restrictedw The minimum width constraint as input, @c minw can
 * not be lower than this
 * @param[in] restrictedh The minimum height constraint as input, @c minh can
 * not be lower than this
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_size_min_restricted_calc(Evas_Object *obj, int *minw, int *minh, int restrictedw, int restrictedh);

/**
 * @brief Calculates the geometry of the region, relative to a given Edje
 * object's area, occupied by all parts in the object.
 *
 * This function gets the geometry of the rectangle equal to the area required
 * to group all parts in obj's group/collection. The x and y coordinates are
 * relative to the top left corner of the whole obj object's area. Parts placed
 * out of the group's boundaries will also be taken in account, so that x and y
 * may be negative.
 *
 * @note On failure, this function will make all non-$null geometry pointers'
 * pointed variables be set to zero.
 *
 * @param[out] x The parts region's X coordinate
 * @param[out] y The parts region's Y coordinate
 * @param[out] w The parts region's width
 * @param[out] h The parts region's height
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_parts_extends_calc(Evas_Object *obj, int *x, int *y, int *w, int *h);

/**
 * @brief Forces a Size/Geometry calculation.
 *
 * Forces the object to recalculate its layout regardless of freeze/thaw.
 *
 * See also @ref edje_object_freeze and @ref edje_object_thaw.
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_calc_force(Evas_Object *obj);

/**
 * @brief Freezes the Edje object.
 *
 * This function puts all changes on hold. Successive freezes will nest,
 * requiring an equal number of thaws.
 *
 * See also @ref edje_object_thaw()
 *
 * @return The frozen state or 0 on error
 *
 * @ingroup Edje_Object
 */
EAPI int edje_object_freeze(Evas_Object *obj);

/**
 * @brief Thaws the Edje object.
 *
 * This function thaws the given Edje object.
 *
 * @note If sucessive freezes were done, an equal number of thaws will be
 * required.
 *
 * See also @ref edje_object_freeze()
 *
 * @return The frozen state or 0 if the object is not frozen or on error.
 *
 * @ingroup Edje_Object
 */
EAPI int edje_object_thaw(Evas_Object *obj);

/**
 * @typedef (*Edje_Text_Change_Cb)
 *
 * Callback prototype for Edje_Text_Change.
 * @param data User provided data to pass to the callback
 * @param obj The Evas_Object
 * @param part The edje part
 */
typedef void         (*Edje_Text_Change_Cb)     (void *data, Evas_Object *obj, const char *part);
/**
 * @}
 */

/**
 * @brief Sets the object text callback.
 *
 * This function sets the callback to be called when the text changes.
 *
 * @param[in] obj The object.
 * @param[in] func The callback function to handle the text change
 * @param[in] data The data associated to the callback function.
 */
EAPI void edje_object_text_change_cb_set(Evas_Object *obj, Edje_Text_Change_Cb func, void *data);



/**
 * @ingroup Edje_Object_Communication_Interface_Message
 *
 * @{
 */

typedef struct _Edje_Message_String           Edje_Message_String;
typedef struct _Edje_Message_Int              Edje_Message_Int;
typedef struct _Edje_Message_Float            Edje_Message_Float;
typedef struct _Edje_Message_String_Set       Edje_Message_String_Set;
typedef struct _Edje_Message_Int_Set          Edje_Message_Int_Set;
typedef struct _Edje_Message_Float_Set        Edje_Message_Float_Set;
typedef struct _Edje_Message_String_Int       Edje_Message_String_Int;
typedef struct _Edje_Message_String_Float     Edje_Message_String_Float;
typedef struct _Edje_Message_String_Int_Set   Edje_Message_String_Int_Set;
typedef struct _Edje_Message_String_Float_Set Edje_Message_String_Float_Set;

struct _Edje_Message_String
{
   char *str; /**< The message's string pointer */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING messages. The string in it is automatically freed Edje if passed to you by Edje */

struct _Edje_Message_Int
{
   int val; /**< The message's value */
}; /**< Structure passed as value on #EDJE_MESSAGE_INT messages */

struct _Edje_Message_Float
{
   double val; /**< The message's value */
}; /**< Structure passed as value on #EDJE_MESSAGE_FLOAT messages */

struct _Edje_Message_String_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   char *str[1]; /**< The message's @b array of string pointers */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_SET messages. The array in it is automatically freed if passed to you by Edje */

struct _Edje_Message_Int_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   int val[1]; /**< The message's @b array of integers */
}; /**< Structure passed as value on #EDJE_MESSAGE_INT_SET messages. The array in it is automatically freed if passed to you by Edje */

struct _Edje_Message_Float_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   double val[1]; /**< The message's @b array of floats */
}; /**< Structure passed as value on #EDJE_MESSAGE_FLOAT_SET messages. The array in it is automatically freed if passed to you by Edje */

struct _Edje_Message_String_Int
{
   char *str; /**< The message's string value */
   int val; /**< The message's integer value */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_INT messages. The string in it is automatically freed if passed to you by Edje */

struct _Edje_Message_String_Float
{
   char *str; /**< The message's string value */
   double val; /**< The message's float value */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_FLOAT messages. The string in it is automatically freed if passed to you by Edje */

struct _Edje_Message_String_Int_Set
{
   char *str; /**< The message's string value */
   int count; /**< The size of the message's array (may be greater than 1) */
   int val[1]; /**< The message's @b array of integers */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_INT_SET messages. The array and string in it are automatically freed if passed to you by Edje */

struct _Edje_Message_String_Float_Set
{
   char *str; /**< The message's string value */
   int count; /**< The size of the message's array (may be greater than 1) */
   double val[1]; /**< The message's @b array of floats */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_FLOAT_SET messages. The array and string in it are automatically freed if passed to you by Edje */

/** Identifiers of Edje message types, which can be sent back and forth code
 * and a given Edje object's theme file/group.
 *
 * @ref edje_object_message_send
 * @ref edje_object_message_handler_set
 */
typedef enum
{
  EDJE_MESSAGE_NONE = 0, /**< No message type */
  EDJE_MESSAGE_SIGNAL = 1, /**< DO NOT USE THIS */
  EDJE_MESSAGE_STRING = 2, /**< A message with a string as value. Use
                            * #Edje_Message_String structs as message body, for
                            * this type. */
  EDJE_MESSAGE_INT = 3, /**< A message with an integer number as value. Use
                         * #Edje_Message_Int structs as message body, for this
                         * type. */
  EDJE_MESSAGE_FLOAT = 4, /**< A message with a floating pointer number as
                           * value. Use #Edje_Message_Float structs as message
                           * body, for this type. */
  EDJE_MESSAGE_STRING_SET = 5, /**< A message with a list of strings as value.
                                * Use #Edje_Message_String_Set structs as
                                * message body, for this type. */
  EDJE_MESSAGE_INT_SET = 6, /**< A message with a list of integer numbers as
                             * value. Use #Edje_Message_Int_Set structs as
                             * message body, for this type. */
  EDJE_MESSAGE_FLOAT_SET = 7, /**< A message with a list of floating point
                               * numbers as value. Use #Edje_Message_Float_Set
                               * structs as message body, for this type. */
  EDJE_MESSAGE_STRING_INT = 8, /**< A message with a struct containing a string
                                * and an integer number as value. Use
                                * #Edje_Message_String_Int structs as message
                                * body, for this type. */
  EDJE_MESSAGE_STRING_FLOAT = 9, /**< A message with a struct containing a
                                  * string and a floating point number as
                                  * value. Use #Edje_Message_String_Float
                                  * structs as message body, for this type. */
  EDJE_MESSAGE_STRING_INT_SET = 10, /**< A message with a struct containing a
                                     * string and list of integer numbers as
                                     * value. Use #Edje_Message_String_Int_Set
                                     * structs as message body, for this type.
                                     */
  EDJE_MESSAGE_STRING_FLOAT_SET = 11 /**< A message with a struct containing a
                                      * string and list of floating point
                                      * numbers as value. Use
                                      * #Edje_Message_String_Float_Set structs
                                      * as message body, for this type. */
} Edje_Message_Type;

typedef void         (*Edje_Message_Handler_Cb) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg); /**< Edje message handler callback functions's prototype definition. @c data will have the auxiliary data pointer set at the time the callback registration. @c obj will be a pointer the Edje object where the message comes from. @c type will identify the type of the given message and @c msg will be a pointer the message's contents, de facto, which depend on @c type. */

/**
 * @brief Sets an Edje message handler function for a given Edje object.
 *
 * For scriptable programs on an Edje object's defining EDC file which send
 * messages with the send_message() primitive, one can attach handler
 * functions, to be called in the code which creates that object (see @ref
 * edcref "the syntax" for EDC files).
 *
 * This function associates a message handler function and the attached data
 * pointer to the object obj.
 *
 * See also @ref edje_object_message_send()
 *
 * @param[in] func The function to handle messages  coming from obj
 * @param[in] data Auxiliary data to be passed to func
 */
EAPI void edje_object_message_handler_set(Evas_Object *obj, Edje_Message_Handler_Cb func, void *data);

/**
 * @brief Sends an (Edje) message to a given Edje object
 *
 * This function sends an Edje message to obj and to all of its child objects,
 * if it has any (swallowed objects are one kind of child object). type and msg
 *  must be matched accordingly, as documented in #Edje_Message_Type.
 *
 * The id argument as a form of code and theme defining a common interface on
 * message communication. One should define the same IDs on both code and EDC
 * declaration (see @ref edcref "the syntax" for EDC files), to individualize
 * messages (binding them to a given context).
 *
 * The function to handle messages arriving  from obj is set with
 * edje_object_message_handler_set().
 *
 * @param[in] type The type of message to send to obj
 * @param[in] id A identification number for the message to be sent
 * @param[in] msg The message's body, a struct depending on type
 */
EAPI void edje_object_message_send(Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

/**
 * @brief Processes an object's message queue.
 *
 * This function goes through the object message queue processing the pending
 * messages for  this specific Edje object. Normally they'd be processed only
 * at idle time. Child objects will not be affected.
 *
 * @see edje_object_message_signal_recursive_process
 */
EAPI void edje_object_message_signal_process(Evas_Object *obj);

/**
 * @brief Processes an object's message queue recursively.
 *
 * This function goes through the object message queue processing the pending
 * messages for this specific Edje object. Normally they'd be processed only
 * at idle time. This will also propagate the processing to all child objects.
 *
 * @see edje_object_message_signal_process
 *
 * @since 1.20
 */
EAPI void edje_object_message_signal_recursive_process(Evas_Object *obj);

/**
 * @}
 */

/**
 * @ingroup Edje_Object
 *
 * @{
 */

/**
 * @brief Facility to query the type of the given parameter of the given part.
 *
 * @param[in] part The part name
 * @param[in] param The parameter name to use
 *
 * @return @ref EDJE_EXTERNAL_PARAM_TYPE_MAX on errors, or another value from
 * @ref Edje_External_Param_Type on success.
 *
 * @ingroup Edje_Object
 */
EAPI Edje_External_Param_Type edje_object_part_external_param_type_get(const Evas_Object *obj, const char *part, const char * param);

/**
 * @brief Sets the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have meanings defined
 * by the external plugin. For instance, it may be a string that defines a
 * button label and setting this property will change that label on the fly.
 *
 * @note external parts have parameters set when they change states. Those
 * parameters will never be changed by this function. The interpretation of how
 * state_set parameters and param_set will interact is up to the external
 * plugin.
 *
 * @note this function will not check if parameter value is valid using
 * #Edje_External_Param_Info minimum, maximum, valid choices and others.
 * However these should be checked by the underlying implementation provided by
 * the external plugin. This is done for performance reasons.
 *
 * @param[in] part The part name
 * @param[in] param The parameter details, including its name, type and actual
 * value. This pointer should be valid, and the parameter must exist in
 * #Edje_External_Type.parameters_info, with the exact type, otherwise the
 * operation will fail and @c false will be returned.
 *
 * @return @c true if everything went fine, @c false on errors.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_external_param_set(Evas_Object *obj, const char *part, const Edje_External_Param *param);

/**
 * @brief Gets the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have meanings defined
 * by the external plugin. For instance, it may be a string that defines a
 * button label. This property can be modified by state parameters, by explicit
 * calls to edje_object_part_external_param_set() or getting the actual object
 * with edje_object_part_external_object_get() and calling native functions.
 *
 * This function asks the external plugin what is the current value,
 * independent on how it was set.
 *
 * @param[in] part The part name
 * @param[out] param The parameter details. It is used as both input and output
 * variable. This pointer should be valid, and the parameter must exist in
 * #Edje_External_Type.parameters_info, with the exact type, otherwise the
 * operation will fail and @c false will be returned.
 *
 * @return @c true if everything went fine and param members are filled with
 * information, @c false on errors and param member values are not set or
 * valid.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_external_param_get(const Evas_Object *obj, const char *part, Edje_External_Param *param);

/**
 * @brief Gets the object created by this external part.
 *
 * Parts of type external creates the part object using information provided by
 * external plugins. It's somehow like "swallow" (edje_object_part_swallow()),
 * but it's all set automatically.
 *
 * This function returns the part created by such external plugins and being
 * currently managed by this Edje.
 *
 * @note Almost all swallow rules apply: you should not move, resize, hide,
 * show, set the color or clipper of such part. It's a bit more restrictive as
 * one must  never delete this object!
 *
 * @param[in] part The part name
 *
 * @return The externally created object, or @c null if there is none or part
 * is not an external.
 *
 * @ingroup Edje_Object
 */
EAPI Evas_Object *edje_object_part_external_object_get(const Evas_Object *obj, const char * part);

/**
 * @brief Gets an object contained in an part of type EXTERNAL
 *
 * The content string must not be @c null. Its actual value depends on the code
 * providing the EXTERNAL.
 *
 * @param[in] part The name of the part holding the EXTERNAL
 * @param[in] content A string identifying which content from the EXTERNAL to
 * get
 *
 * @return Canvas object
 *
 * @ingroup Edje_Object
 */
EAPI Evas_Object *edje_object_part_external_content_get(const Evas_Object *obj, const char *part, const char *content);

/**
 * @}
 */

 /**
 * @deprecated use evas_object_size_hint_min_set() instead.
 * @brief Sets the object minimum size.
 *
 * @param obj A valid Evas_Object handle
 * @param minw The minimum width
 * @param minh The minimum height
 *
 * This sets the minimum size restriction for the object.
 *
 */
EINA_DEPRECATED EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);

/**
 * @deprecated use evas_object_size_hint_max_set() instead.
 * @brief Sets the object maximum size.
 *
 * @param obj A valid Evas_Object handle
 * @param maxw The maximum width
 * @param maxh The maximum height
 *
 * This sets the maximum size restriction for the object.
 *
 */
EINA_DEPRECATED EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);

/**
 * @deprecated use evas_object_size_hint_aspect_set() instead.
 * @brief Sets the object aspect size.
 *
 * @param obj A valid Evas_Object handle
 * @param aspect The aspect control axes
 * @param aw The aspect radio width
 * @param ah The aspect ratio height
 *
 * This sets the desired aspect ratio to keep an object that will be
 * swallowed by Edje. The width and height define a preferred size
 * ASPECT and the object may be scaled to be larger or smaller, but
 * retaining the relative scale of both aspect width and height.
 *
 */
EINA_DEPRECATED EAPI void         edje_extern_object_aspect_set   (Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);

/**
 * @brief Sets the @b EDJ file (and group within it) to load an Edje
 * object's contents from
 *
 * @return @c EINA_TRUE on success, or @c EINA_FALSE on errors (check
 * edje_object_load_error_get() after this call to get errors causes)
 *
 * Edje expects EDJ files, which are theming objects' descriptions and
 * resources packed together in an EET file, to read Edje object
 * definitions from. They usually are created with the @c .edj
 * extension. EDJ files, in turn, are assembled from @b textual object
 * description files, where one describes Edje objects declaratively
 * -- the EDC files (see @ref edcref "the syntax" for those files).
 *
 * Those description files were designed so that many Edje object
 * definitions -- also called @b groups (or collections) -- could be
 * packed together <b>in the same EDJ file</b>, so that a whole
 * application's theme could be packed in one file only. This is the
 * reason for the @p group argument.
 *
 * Use this function after you instantiate a new Edje object, so that
 * you can "give him life", telling where to get its contents from.
 *
 * @see edje_object_add()
 * @see edje_object_file_get()
 * @see edje_object_mmap_set()
 *
 * @param[in] file The path to the EDJ file to load @p from
 * @param[in] group The name of the group, in @p file, which implements an
Edje object
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * @brief Gets the file and group name that a given Edje object is bound to.
 *
 * This gets the EDJ file's path, with the respective group set for
 * the given Edje object. If @a obj is either not an Edje file, or has
 * not had its file/group set previously, by edje_object_file_set(),
 * then both @p file and @p group will be set to @c NULL, indicating
 * an error.
 *
 * @see edje_object_file_set()
 *
 * @note Use @c NULL pointers on the file/group components you're not
 * interested in: they'll be ignored by the function.
 *
 * @param[out] file The path to the EDJ file to load @p from
 * @param[out] group The name of the group, in @p file, which implements an
Edje object
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_file_get(const Evas_Object *obj, const char **file, const char **group);


/**
 * @brief Sets the @b EDJ file (and group within it) to load an Edje
 * object's contents from.
 *
 * @return @c EINA_TRUE on success, or @c EINA_FALSE on errors (check
 * edje_object_load_error_get() after this call to get errors causes)
 *
 * Edje expects EDJ files, which are theming objects' descriptions and
 * resources packed together in an EET file, to read Edje object
 * definitions from. They usually are created with the @c .edj
 * extension. EDJ files, in turn, are assembled from @b textual object
 * description files, where one describes Edje objects declaratively
 * -- the EDC files (see @ref edcref "the syntax" for those files).
 *
 * Those description files were designed so that many Edje object
 * definitions -- also called @b groups (or collections) -- could be
 * packed together <b>in the same EDJ file</b>, so that a whole
 * application's theme could be packed in one file only. This is the
 * reason for the @p group argument.
 *
 * Use this function after you instantiate a new Edje object, so that
 * you can "give him life", telling where to get its contents from.
 *
 * @see edje_object_add()
 * @see edje_object_file_get()
 * @since 1.8
 *
 * @param[in] file The Eina.File pointing to the EDJ file to load @p from
 * @param[in] group The name of the group, in @p file, which implements an
Edje object
 *
 * @ingroup Edje_Object
 *
 */
EAPI Eina_Bool edje_object_mmap_set(Evas_Object *obj, const Eina_File *file, const char *group);

/**
 * @brief "Swallows" an object into one of the Edje object @c SWALLOW parts.
 *
 * Swallowing an object into an Edje object is, for a given part of type
 * @c SWALLOW in the EDC group which gave life to  obj, to set an external
 * object to be controlled by  obj, being displayed exactly over that part's
 * region inside the whole Edje object's viewport.
 *
 * From this point on,  obj will have total control over obj_swallow's geometry
 * and visibility. For instance, if  obj is visible, as in @ref
 * evas_object_show(), the swallowed object will be visible too -- if the given
 * @c SWALLOW part it's in is also visible. Other actions on  obj will also
 * reflect on the swallowed object as well (e.g. resizing, moving,
 * raising/lowering, etc.).
 *
 * Finally, all internal changes to  part, specifically, will reflect on the
 * displaying of  obj_swallow, for example state changes leading to different
 * visibility states, geometries, positions, etc.
 *
 * If an object has already been swallowed into this part, then it will first
 * be unswallowed (as in edje_object_part_unswallow()) before the new object is
 * swallowed.
 *
 * @note  obj  won't delete the swallowed object once it is deleted --
 *  obj_swallow will get to an unparented state again.
 *
 * For more details on EDC @c SWALLOW parts, see @ref edcref "syntax
 * reference".
 *
 * @param[in] obj_swallow The object to occupy that part
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_swallow(Evas_Object *obj, const char *part, Evas_Object *obj_swallow);

/**
 * @brief Gets the object currently swallowed by a part.
 *
 * @param[in] part The part name
 *
 * @return The swallowed object, or @c null if there is none.
 *
 * @ingroup Edje_Object
 */
EAPI Evas_Object *edje_object_part_swallow_get(const Evas_Object *obj, const char *part);

/**
 * @brief Unswallows an object.
 *
 * Causes the edje to regurgitate a previously swallowed object. :)
 *
 * @note obj_swallow will  not be deleted or hidden. Note: obj_swallow may
 * appear shown on the evas depending on its state when it got unswallowed.
 * Make sure you delete it or hide it if you do not want it to.
 *
 * @param[in] obj_swallow The swallowed object
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_unswallow(Evas_Object *obj, Evas_Object *obj_swallow);

/**
 * @brief Retrieves a list all accessibility part names
 *
 * @return A list all accessibility part names on obj
 *
 * @since 1.7.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_List *edje_object_access_part_list_get(const Evas_Object *obj);

/**
 * @brief Appends an object to the box.
 *
 * Appends child to the box indicated by part.
 *
 * See also @ref edje_object_part_box_prepend(),
 * @ref edje_object_part_box_insert_before(),
 * @ref edje_object_part_box_insert_after() and
 * @ref edje_object_part_box_insert_at()
 *
 * @param[in] child The object to append
 *
 * @return @c true: Successfully added. @c false: An error occurred.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_box_append(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Prepends an object to the box.
 *
 * Prepends child to the box indicated by part.
 *
 * See also @ref edje_object_part_box_append(),
 * @ref edje_object_part_box_insert_before(),
 * @ref edje_object_part_box_insert_after and
 * @ref edje_object_part_box_insert_at()
 *
 * @param[in] child The object to prepend
 *
 * @return @c true: Successfully added. @c false: An error occurred.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_box_prepend(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Adds an object to the box.
 *
 * Inserts child in the box given by part, in the position marked by reference.
 *
 * See also @ref edje_object_part_box_append(),
 * @ref edje_object_part_box_prepend(),
 * @ref edje_object_part_box_insert_after() and
 * @ref edje_object_part_box_insert_at()
 *
 * @param[in] child The object to insert
 * @param[in] reference The object to be used as reference
 *
 * @return @c true: Successfully added. @c false: An error occurred.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_box_insert_before(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * @brief Adds an object to the box.
 *
 * Inserts child in the box given by part, in the position marked by reference.
 *
 * See also @ref edje_object_part_box_append(),
 * @ref edje_object_part_box_prepend(),
 * @ref edje_object_part_box_insert_before() and
 * @ref edje_object_part_box_insert_at()
 *
 * @param[in] child The object to insert
 * @param[in] reference The object to be used as reference
 *
 * @return @c true: Successfully added. @c false: An error occurred.
 *
 * @since 1.18
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_box_insert_after(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * @brief Inserts an object to the box.
 *
 * Adds child to the box indicated by part, in the position given by pos.
 *
 * See also @ref edje_object_part_box_append(),
 * @ref edje_object_part_box_prepend(),
 * @ref edje_object_part_box_insert_before() and
 * @ref edje_object_part_box_insert_after()
 *
 * @param[in] child The object to insert
 * @param[in] pos The position where to insert child
 *
 * @return @c true: Successfully added. @c false: An error occurred.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_box_insert_at(Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);

/**
 * @brief Removes an object from the box.
 *
 * Removes from the box indicated by part, the object in the position pos.
 *
 * See also @ref edje_object_part_box_remove() and
 * @ref edje_object_part_box_remove_all()
 *
 * @param[in] pos The position index of the object (starts counting from 0)
 *
 * @return Pointer to the object removed, or @c null.
 *
 * @ingroup Edje_Object
 */
EAPI Evas_Object *edje_object_part_box_remove_at(Evas_Object *obj, const char *part, unsigned int pos);

/**
 * @brief Removes an object from the box.
 *
 * Removes child from the box indicated by part.
 *
 * See also @ref edje_object_part_box_remove_at() and
 * @ref edje_object_part_box_remove_all()
 *
 * @param[in] child The object to remove
 *
 * @return Pointer to the object removed, or @c null.
 *
 * @ingroup Edje_Object
 */
EAPI Evas_Object *edje_object_part_box_remove(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Removes all elements from the box.
 *
 * Removes all the external objects from the box indicated by part. Elements
 * created from the theme will not be removed.
 *
 * See also @ref edje_object_part_box_remove() and
 * @ref edje_object_part_box_remove_at()
 *
 * @param[in] clear Delete objects on removal
 *
 * @return 1: Successfully cleared. 0: An error occurred.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_box_remove_all(Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @brief Packs an object into the table.
 *
 * Packs an object into the table indicated by part.
 *
 * @param[in] child_obj The object to pack in
 * @param[in] col The column to place it in
 * @param[in] row The row to place it in
 * @param[in] colspan Columns the child will take
 * @param[in] rowspan Rows the child will take
 *
 * @return @c true object was added, @c false on failure
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_table_pack(Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);

/**
 * @brief Removes an object from the table.
 *
 * Removes an object from the table indicated by part.
 *
 * @param[in] child_obj The object to pack in
 *
 * @return @c true object removed, @c false on failure
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_table_unpack(Evas_Object *obj, const char *part, Evas_Object *child_obj);

/**
 * @brief Gets the number of columns and rows the table has.
 *
 * Retrieves the size of the table in number of columns and rows.
 *
 * @param[out] cols Pointer where to store number of columns (can be @c null)
 * @param[out] rows Pointer where to store number of rows (can be @c null)
 *
 * @return @c true get some data, @c false on failure
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_table_col_row_size_get(const Evas_Object *obj, const char *part, int *cols, int *rows);

/**
 * @brief Retrieves a child from a table
 *
 * @param[in] col The column of the child to get
 * @param[in] row The row of the child to get
 *
 * @return The child Efl.Canvas.Object
 *
 * @ingroup Edje_Object
 */
EAPI Evas_Object *edje_object_part_table_child_get(const Evas_Object *obj, const char *part, unsigned int col, unsigned int row);

/**
 * @brief Removes all object from the table.
 *
 * Removes all object from the table indicated by part, except the internal
 * ones set from the theme.
 *
 * @param[in] clear If set, will delete subobjs on remove
 *
 * @return @c true clear the table, @c false on failure
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_table_clear(Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @brief Sets the object color class.
 *
 * This function sets the color values for an object level color class. This
 * will cause all edje parts in the specified object that have the specified
 * color class to have their colors multiplied by these values.
 *
 * The first color is the object, the second is the text outline, and the third
 * is the text shadow. (Note that the second two only apply to text parts).
 *
 * Setting color emits a signal "color_class,set" with source being the given
 * color.
 *
 * @note unlike Evas, Edje colors are not pre-multiplied. That is,
 * half-transparent white is 255 255 255 128.
 *
 * @param[in] color_class The name of color class
 * @param[in] r Object Red value
 * @param[in] g Object Green value
 * @param[in] b Object Blue value
 * @param[in] a Object Alpha value
 * @param[in] r2 Outline Red value
 * @param[in] g2 Outline Green value
 * @param[in] b2 Outline Blue value
 * @param[in] a2 Outline Alpha value
 * @param[in] r3 Shadow Red value
 * @param[in] g3 Shadow Green value
 * @param[in] b3 Shadow Blue value
 * @param[in] a3 Shadow Alpha value
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_color_class_set(Evas_Object *obj, const char * color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Gets the object color class.
 *
 * This function gets the color values for an object level color class. If no
 * explicit object color is set, then global values will be used.
 *
 * The first color is the object, the second is the text outline, and the third
 * is the text shadow. (Note that the second two only apply to text parts).
 *
 * @note unlike Evas, Edje colors are not pre-multiplied. That is,
 * half-transparent white is 255 255 255 128.
 *
 * @param[in] color_class The name of color class
 * @param[out] r Object Red value
 * @param[out] g Object Green value
 * @param[out] b Object Blue value
 * @param[out] a Object Alpha value
 * @param[out] r2 Outline Red value
 * @param[out] g2 Outline Green value
 * @param[out] b2 Outline Blue value
 * @param[out] a2 Outline Alpha value
 * @param[out] r3 Shadow Red value
 * @param[out] g3 Shadow Green value
 * @param[out] b3 Shadow Blue value
 * @param[out] a3 Shadow Alpha value
 *
 * @return true if found or false if not found and all values are zeroed.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_color_class_get(const Evas_Object *obj, const char * color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Delete the object color class.
 *
 * This function deletes any values at the object level for the specified
 * object and color class.
 *
 * Deleting the color class will revert it to the values defined by
 * edje_color_class_set() or the color class defined in the theme file.
 *
 * Deleting the color class will emit the signal "color_class,del" for the
 * given Edje object.
 *
 * @param[in] color_class The color class to be deleted.
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_color_class_del(Evas_Object *obj, const char *color_class);

/**
 * @brief Delete all color classes defined in object level.
 *
 * This function deletes any color classes defined in object level.
 * Clearing color classes will revert the color of all edje parts to
 * the values defined in global level or theme file.
 *
 * @return @c true, on success or @c false, on error
 *
 * @since 1.17.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_color_class_clear(const Evas_Object *obj);

/**
 * @brief Sets Edje text class.
 *
 * This function sets the text class for the Edje.
 *
 * @param[in] text_class The text class name
 * @param[in] font Font name
 * @param[in] size Font Size
 *
 * @return @c true, on success or @c false, on error
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_text_class_set(Evas_Object *obj, const char * text_class, const char *font, Evas_Font_Size size);

/**
 * @brief Gets font and font size from edje text class.
 *
 * This function gets the font and the font size from the object text class.
 * The font string will only be valid until the text class is changed or the
 * edje object is deleted.
 *
 * @param[in] text_class The text class name
 * @param[out] font Font name
 * @param[out] size Font Size
 *
 * @return @c true, on success or @c false, on error
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_text_class_get(const Evas_Object *obj, const char * text_class, const char **font, Evas_Font_Size *size);

/**
 * @brief Delete the object text class.
 *
 * This function deletes any values at the object level for the specified
 * object and text class.
 *
 * Deleting the text class will revert it to the values defined by
 * edje_text_class_set() or the text class defined in the theme file.
 *
 * @param[in] text_class The color class to be deleted.
 *
 * @since 1.17
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_text_class_del(Evas_Object *obj, const char *text_class);

/**
 * @brief Sets the object size class.
 *
 * This function sets the min and max values for an object level size class.
 * This will make all edje parts in the specified object that have the
 * specified size class update their min and max size with given values.
 *
 * @param[in] size_class The size class name
 * @param[in] minw The min width
 * @param[in] minh The min height
 * @param[in] maxw The max width
 * @param[in] maxh The max height
 *
 * @return @c true, on success or @c false, on error
 *
 * @since 1.17
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_size_class_set(Evas_Object *obj, const char * size_class, int minw, int minh, int maxw, int maxh);

/**
 * @brief Gets the object size class.
 *
 * This function gets the min and max values for an object level size class.
 * These values will only be valid until the size class is changed or the edje
 * object is deleted.
 *
 * @param[in] size_class The size class name
 * @param[out] minw The min width
 * @param[out] minh The min height
 * @param[out] maxw The max width
 * @param[out] maxh The max height
 *
 * @return @c true, on success or @c false, on error
 *
 * @since 1.17
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_size_class_get(const Evas_Object *obj, const char * size_class, int *minw, int *minh, int *maxw, int *maxh);

/**
 * @brief Delete the object size class.
 *
 * This function deletes any values at the object level for the specified
 * object and size class.
 *
 * Deleting the size class will revert it to the values defined by
 * edje_size_class_set() or the color class defined in the theme file.
 *
 * @param[in] size_class Size class name
 *
 * @since 1.17
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_size_class_del(Evas_Object *obj, const char *size_class);

/**
 * @brief Enables selection if the entry is an EXPLICIT selection mode type.
 *
 * The default is to  not allow selection. This function only affects user
 * selection, functions such as edje_object_part_text_select_all() and
 * edje_object_part_text_select_none() are not affected.
 *
 * @param[in] part The part name
 * @param[in] allow true to enable, false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_select_allow_set(const Evas_Object *obj, const char *part, Eina_Bool allow);

/**
 * @brief Sets the RTL orientation for this object.
 *
 * @param[in] rtl New value of flag @c true/$false
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_mirrored_set(Evas_Object *obj, Eina_Bool rtl);

/**
 * @brief Gets the RTL orientation for this object.
 *
 * You can RTL orientation explicitly with edje_object_mirrored_set.
 *
 * @return New value of flag @c true/$false
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_mirrored_get(const Evas_Object *obj);

/**
 * @brief Sets the language for this object.
 *
 * @param[in] language The language value
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_language_set(Evas_Object *obj, const char *language);

/**
 * @brief Gets the language for this object.
 *
 * @return The language value
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI const char *edje_object_language_get(const Evas_Object *obj);

/**
 * @brief Sets the scaling factor for a given Edje object.
 *
 * This function sets an  individual scaling factor on the  obj Edje object.
 * This property (or Edje's global scaling factor, when applicable), will
 * affect this object's part sizes. If scale is not zero, than the individual
 * scaling will  override any global scaling set, for the object obj's parts.
 * Put it back to zero to get the effects of the global scaling again.
 *
 * @warning Only parts which, at EDC level, had the @"scale" property set to
 * @1, will be affected by this function. Check the complete @ref edcref
 * "syntax reference" for EDC files.
 *
 * See also @ref edje_object_scale_get() @ref edje_scale_get() for more details
 *
 * @param[in] scale The scaling factor (the default value is @0.0, meaning
 * individual scaling  not set)
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_scale_set(Evas_Object *obj, double scale);

/**
 * @brief Gets a given Edje object's scaling factor.
 *
 * This function returns the individual scaling factor set on the obj Edje
 * object.
 *
 * See also @ref edje_object_scale_set() for more details
 *
 * @return The scaling factor (the default value is @0.0, meaning individual
 * scaling  not set)
 *
 * @ingroup Edje_Object
 */
EAPI double edje_object_scale_get(const Evas_Object *obj);

/**
 * @brief Gets a given Edje object's base_scale factor.
 *
 * This function returns the base_scale factor set on the obj Edje object. The
 * base_scale can be set in the collection of edc. If it isn't set, the default
 * value is 1.0
 *
 * @return The base_scale factor (the default value is @ 1.0, that means the
 * edc file is made based on scale 1.0.
 *
 * @ingroup Edje_Object
 */
EAPI double edje_object_base_scale_get(const Evas_Object *obj);

/**
 * @defgroup Edje_Part_Drag Edje Drag
 * @ingroup Edje_Object_Part
 *
 * @brief Functions that deal with dragable parts.
 *
 * To create a movable part it must be declared as dragable
 * in EDC file. To do so, one must define a "dragable" block inside
 * the "part" block.
 *
 * These functions are used to set dragging properties to a
 * part or get dragging information about it.
 *
 * @see @ref tutorial_edje_drag *
 *
 * @{
 */

/** Dragable properties values */
typedef Efl_Ui_Drag_Dir Edje_Drag_Dir;
/** Not dragable */
#define EDJE_DRAG_DIR_NONE EFL_UI_DRAG_DIR_NONE
/** Dragable horizontally */
#define EDJE_DRAG_DIR_X     EFL_UI_DRAG_DIR_X
/** Dragable verically */
#define EDJE_DRAG_DIR_Y    EFL_UI_DRAG_DIR_Y
/** Dragable in both directions */
#define EDJE_DRAG_DIR_XY   EFL_UI_DRAG_DIR_XY


/**
 * @brief Sets the dragable object location.
 *
 * Places the dragable object at the given location.
 *
 * Values for dx and dy are real numbers that range from 0 to 1, representing
 * the relative position to the dragable area on that axis.
 *
 * This value means, for the vertical axis, that 0.0 will be at the top if the
 * first parameter of @c y in the dragable part theme is 1, and at bottom if it
 * is -1.
 *
 * For the horizontal axis, 0.0 means left if the first parameter of @c x in
 * the dragable part theme is 1, and right if it is -1.
 *
 * See also @ref edje_object_part_drag_value_get()
 *
 * @param[in] part The part name
 * @param[in] dx The x value
 * @param[in] dy The y value
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_value_set(Evas_Object *obj, const char * part, double dx, double dy);

/**
 * @brief Gets the dragable object location.
 *
 * Values for dx and dy are real numbers that range from 0 to 1, representing
 * the relative position to the dragable area on that axis.
 *
 * See also @ref edje_object_part_drag_value_set()
 *
 * Gets the drag location values.
 *
 * @param[in] part The part name
 * @param[out] dx The x value
 * @param[out] dy The y value
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_value_get(const Evas_Object *obj, const char * part, double *dx, double *dy);

/**
 * @brief Sets the dragable object size.
 *
 * Values for dw and dh are real numbers that range from 0 to 1, representing
 * the relative size of the dragable area on that axis.
 *
 * Sets the size of the dragable object.
 *
 * See also @ref edje_object_part_drag_size_get()
 *
 * @param[in] part The part name
 * @param[in] dw The drag width
 * @param[in] dh The drag height
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_size_set(Evas_Object *obj, const char * part, double dw, double dh);

/**
 * @brief Gets the dragable object size.
 *
 * Gets the dragable object size.
 *
 * See also @ref edje_object_part_drag_size_set()
 *
 * @param[in] part The part name
 * @param[out] dw The drag width
 * @param[out] dh The drag height
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_size_get(const Evas_Object *obj, const char * part, double *dw, double *dh);

/**
 * @brief Determines dragable directions.
 *
 * The dragable directions are defined in the EDC file, inside the @ref
 * dragable section, by the attributes @c x and @c y. See the @ref edcref for
 * more information.
 *
 * @param[in] part The part name
 *
 * @return #EDJE_DRAG_DIR_NONE: Not dragable #EDJE_DRAG_DIR_X: Dragable in X
 * direction #EDJE_DRAG_DIR_Y: Dragable in Y direction #EDJE_DRAG_DIR_XY:
 * Dragable in X & Y directions
 *
 * @ingroup Edje_Object
 */
EAPI Edje_Drag_Dir edje_object_part_drag_dir_get(const Evas_Object *obj, const char * part);

/**
 * @brief Sets the drag step increment.
 *
 * Sets the x,y step increments for a dragable object.
 *
 * Values for dx and dy are real numbers that range from 0 to 1, representing
 * the relative size of the dragable area on that axis by which the part will
 * be moved.
 *
 * See also @ref edje_object_part_drag_step_get()
 *
 * @param[in] part The part name
 * @param[in] dx The x step amount
 * @param[in] dy The y step amount
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_step_set(Evas_Object *obj, const char * part, double dx, double dy);

/**
 * @brief Gets the drag step increment values.
 *
 * Gets the x and y step increments for the dragable object.
 *
 * See also @ref edje_object_part_drag_step_set()
 *
 * @param[in] part The part name
 * @param[out] dx The x step amount
 * @param[out] dy The y step amount
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_step_get(const Evas_Object *obj, const char * part, double *dx, double *dy);

/**
 * @brief Steps the dragable x,y steps.
 *
 * Steps x,y where the step increment is the amount set by
 * @ref edje_object_part_drag_step_set().
 *
 * Values for dx and dy are real numbers that range from 0 to 1.
 *
 * See also @ref edje_object_part_drag_page()
 *
 * @param[in] part The part name
 * @param[in] dx The x step
 * @param[in] dy The y step
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_step(Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Sets the page step increments.
 *
 * Sets the x,y page step increment values.
 *
 * Values for dx and dy are real numbers that range from 0 to 1, representing
 * the relative size of the dragable area on that axis by which the part will
 * be moved.
 *
 * See also @ref edje_object_part_drag_page_get()
 *
 * @param[in] part The part name
 * @param[in] dx The x page step increment
 * @param[in] dy The y page step increment
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_page_set(Evas_Object *obj, const char * part, double dx, double dy);

/**
 * @brief Gets the page step increments.
 *
 * Gets the x,y page step increments for the dragable object.
 *
 * See also @ref edje_object_part_drag_page_set()
 *
 * @param[in] part The part name
 * @param[out] dx The x page step increment
 * @param[out] dy The y page step increment
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_page_get(const Evas_Object *obj, const char * part, double *dx, double *dy);

/**
 * @brief Pages x,y steps.
 *
 * Pages x,y where the increment is defined by
 * @ref edje_object_part_drag_page_set().
 *
 * Values for dx and dy are real numbers that range from 0 to 1.
 *
 * @warning Paging is bugged!
 *
 * See also @ref edje_object_part_drag_step()
 *
 * @param[in] part The part name
 * @param[in] dx The x step
 * @param[in] dy The y step
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_drag_page(Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Sets a given text to an Edje object @c TEXT or TEXTBLOCK
 * parts.
 *
 * @param[in] part The part name
 * @param[in] text The text to set on that part
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_set(const Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Gets the text currntly set to the given part
 *
 * @param[in] part The part name
 *
 * @return The text set on the part, @c null otherwise.
 *
 * @ingroup Edje_Object
 */
EAPI const char * edje_object_part_text_get(const Evas_Object *obj, const char *part);

/**
 * @brief Moves the cursor to the beginning of the text part @ref
 * evas_textblock_cursor_paragraph_first
 *
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_cursor_begin_set(Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the end of the text part. @ref
 * evas_textblock_cursor_paragraph_last
 *
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_cursor_end_set(Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Sets the cursor position to the given value
 *
 * @param[in] part The part name
 * @param[in] cur The cursor to move
 * @param[in] pos The position of the cursor
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_cursor_pos_set(Evas_Object *obj, const char * part, Edje_Cursor cur, int pos);

/**
 * @brief Retrieves the current position of the cursor
 *
 * @param[in] part The part name
 * @param[in] cur The cursor to move
 *
 * @return The position of the cursor
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI int edje_object_part_text_cursor_pos_get(const Evas_Object *obj, const char * part, Edje_Cursor cur);

/**
 * @brief Position the given cursor to a X,Y position.
 *
 * This is frequently used with the user cursor.
 *
 * @param[in] part The part containing the object.
 * @param[in] cur The cursor to adjust.
 * @param[in] x X Coordinate.
 * @param[in] y Y Coordinate.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_cursor_coord_set(Evas_Object *obj, const char *part, Edje_Cursor cur, int x, int y);

/**
 * @brief Moves the cursor to the beginning of the line. @ref
 * evas_textblock_cursor_line_char_first
 *
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_cursor_line_begin_set(Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the end of the line. @ref
 * evas_textblock_cursor_line_char_last
 *
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_cursor_line_end_set(Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the previous char @ref
 * evas_textblock_cursor_char_prev
 *
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_cursor_prev(Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Advances the cursor to the next cursor position. @ref
 * evas_textblock_cursor_char_next
 *
 * @param[in] part The part name
 * @param[in] cur The edje cursor to advance
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_cursor_next(Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the char above the current cursor position.
 *
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_cursor_up(Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the char below the current cursor position.
 *
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_cursor_down(Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Copies the cursor to another cursor.
 *
 * @param[in] part The part name
 * @param[in] src The cursor to copy from
 * @param[in] dst The cursor to copy to
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_cursor_copy(Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst);

/**
 * @brief Returns the content (char) at the cursor position. @ref
 * evas_textblock_cursor_content_get
 *
 * You must free the return (if not @c null) after you are done with it.
 *
 * @param[in] part The part name
 * @param[in] cur The cursor to use
 *
 * @return The character string pointed to (may be a multi-byte utf8 sequence)
 * terminated by a null byte.
 *
 * @ingroup Edje_Object
 */
EAPI char *edje_object_part_text_cursor_content_get(const Evas_Object *obj, const char * part, Edje_Cursor cur);

/**
 * @brief Returns the cursor geometry of the part relative to the edje object.
 *
 * @param[in] part The part name
 * @param[out] x Cursor X position
 * @param[out] y Cursor Y position
 * @param[out] w Cursor width
 * @param[out] h Cursor height
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_cursor_geometry_get(const Evas_Object *obj, const char * part, int *x, int *y, int *w, int *h);

/**
 * @brief Hides visible last character for password mode.
 *
 * @param[in] part The part name
 *
 * @return @c true if the visible character is hidden. @c false if there is no
 * visible character or the object is not set for password mode.
 *
 * @since 1.18.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_hide_visible_password(Evas_Object *obj, const char *part);

/**
 * @brief Returns whether the cursor points to a format. @ref
 * evas_textblock_cursor_is_format
 *
 * @param[in] part The part name
 * @param[in] cur The cursor to adjust.
 *
 * @return @c true if the cursor points to a format, @c false otherwise.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_cursor_is_format_get(const Evas_Object *obj, const char * part, Edje_Cursor cur);

/**
 * @brief Returns @c true if the cursor points to a visible format For example
 * \\t, \\n, item and etc. @ref evas_textblock_cursor_format_is_visible_get
 *
 * @param[in] part The part name
 * @param[in] cur The cursor to adjust.
 *
 * @return @c true if the cursor points to a visible format, @c false
 * otherwise.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char * part, Edje_Cursor cur);

/**
 * @brief Returns a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * This function return a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * @param[in] part The part name
 * @param[in] anchor The anchor name
 *
 * @return The list of anchor rects (const Evas_Textblock_Rectangle *), do not
 * modify! Geometry is relative to entry part.
 *
 * @ingroup Edje_Object
 */
EAPI const Eina_List *edje_object_part_text_anchor_geometry_get(const Evas_Object *obj, const char * part, const char * anchor);

/**
 * @brief Returns a list of char anchor names.
 *
 * This function returns a list of char anchor names.
 *
 * @param[in] part The part name
 *
 * @return The list of anchors (const char *), do not modify!
 *
 * @ingroup Edje_Object
 */
EAPI const Eina_List *edje_object_part_text_anchor_list_get(const Evas_Object *obj, const char * part);

/**
 * @brief Returns the text of the object part.
 *
 * This function returns the style associated with the textblock part.
 *
 * @param[in] part The part name
 *
 * @return The text string
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI const char *edje_object_part_text_style_user_peek(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the style of the
 *
 * This function sets the style associated with the textblock part.
 *
 * @param[in] part The part name
 * @param[in] style The style to set (textblock conventions).
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_style_user_push(Evas_Object *obj, const char *part, const char *style);

/**
 * @brief Deletes the top style form the user style stack.
 *
 * @param[in] part The part name
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_style_user_pop(Evas_Object *obj, const char *part);

/**
 * @brief Returns item geometry.
 *
 * This function return a list of Evas_Textblock_Rectangle item rectangles.
 *
 * @param[in] part The part name
 * @param[in] item The item name
 * @param[out] cx Item x return (relative to entry part)
 * @param[out] cy Item y return (relative to entry part)
 * @param[out] cw Item width return
 * @param[out] ch Item height return
 *
 * @return $1 if item exists, $0 if not
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_item_geometry_get(const Evas_Object *obj, const char * part, const char * item, int *cx, int *cy, int *cw, int *ch);

/**
 * @brief Returns a list of char item names.
 *
 * This function returns a list of char item names.
 *
 * @param[in] part The part name
 *
 * @return The list of items (const char *), do not modify!
 *
 * @ingroup Edje_Object
 */
EAPI const Eina_List *edje_object_part_text_item_list_get(const Evas_Object *obj, const char * part);

/**
 * @brief Adds a filter function for newly inserted text.
 *
 * Whenever text is inserted (not the same as set) into the given part, the
 * list of filter functions will be called to decide if and how the new text
 * will be accepted. There are three types of filters, EDJE_TEXT_FILTER_TEXT,
 * EDJE_TEXT_FILTER_FORMAT and EDJE_TEXT_FILTER_MARKUP. The text parameter in
 * the func filter can be modified by the user and it's up to him to free the
 * one passed if he's to change the pointer. If doing so, the newly set text
 * should be malloc'ed, as once all the filters are called Edje will free it.
 * If the text is to be rejected, freeing it and setting the pointer to @c null
 * will make Edje break out of the filter cycle and reject the inserted text.
 *
 * @warning This function will be deprecated because of difficulty in use. The
 * type(format, text, or markup) of text should be always checked in the filter
 * function for correct filtering. Please use
 * edje_object_text_markup_filter_callback_add() instead. There is no need to
 * check the type of text in the filter function because the text is always
 * markup. Warning: If you use this function with
 * edje_object_text_markup_filter_callback_add() together, all
 * Edje_Text_Filter_Cb functions and Edje_Markup_Filter_Cb functions will be
 * executed, and then filtered text will be inserted.
 *
 * See also @ref edje_object_text_insert_filter_callback_del,
 * @ref edje_object_text_insert_filter_callback_del_full and
 * @ref edje_object_text_markup_filter_callback_add
 *
 * @param[in] part The part name
 * @param[in] func The callback function that will act as filter
 * @param[in] data User provided data to pass to the filter function
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_text_insert_filter_callback_add(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * @brief Deletes a function from the filter list.
 *
 * Delete the given func filter from the list in part. Returns the user data
 * pointer given when added.
 *
 * See also @ref edje_object_text_insert_filter_callback_add and
 * @ref edje_object_text_insert_filter_callback_del_full
 *
 * @param[in] part The part name
 * @param[in] func The function callback to remove
 *
 * @return The user data pointer if successful, or @c null otherwise
 *
 * @ingroup Edje_Object
 */
EAPI void *edje_object_text_insert_filter_callback_del(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func);

/**
 * @brief Deletes a function and matching user data from the filter list.
 *
 * Delete the given func filter and data user data from the list in part.
 * Returns the user data pointer given when added.
 *
 * See also @ref edje_object_text_insert_filter_callback_add and
 * @ref edje_object_text_insert_filter_callback_del
 *
 * @param[in] part The part name
 * @param[in] func The function callback to remove
 * @param[in] data The data passed to the callback function
 *
 * @return The same data pointer if successful, or @c null otherwise
 *
 * @ingroup Edje_Object
 */
EAPI void *edje_object_text_insert_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * @brief Adds a markup filter function for newly inserted text.
 *
 * Whenever text is inserted (not the same as set) into the given part, the
 * list of markup filter functions will be called to decide if and how the new
 * text will be accepted. The text parameter in the func filter is always
 * markup. It can be modified by the user and it's up to him to free the one
 * passed if he's to change the pointer. If doing so, the newly set text should
 * be malloc'ed, as once all the filters are called Edje will free it. If the
 * text is to be rejected, freeing it and setting the pointer to @c null will
 * make Edje break out of the filter cycle and reject the inserted text. This
 * function is different from edje_object_text_insert_filter_callback_add() in
 * that the text parameter in the func filter is always markup.
 *
 * @warning If you use this function with
 * edje_object_text_insert_filter_callback_add() together, all
 * Edje_Text_Filter_Cb functions and Edje_Markup_Filter_Cb functions will be
 * executed, and then filtered text will be inserted.
 *
 * See also @ref edje_object_text_markup_filter_callback_del,
 * @ref edje_object_text_markup_filter_callback_del_full and
 * @ref edje_object_text_insert_filter_callback_add
 *
 * @param[in] part The part name
 * @param[in] func The callback function that will act as markup filter
 * @param[in] data User provided data to pass to the filter function
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_text_markup_filter_callback_add(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data);

/**
 * @brief Deletes a function from the markup filter list.
 *
 * Delete the given func filter from the list in part. Returns the user data
 * pointer given when added.
 *
 * See also @ref edje_object_text_markup_filter_callback_add and
 * @ref edje_object_text_markup_filter_callback_del_full
 *
 * @param[in] part The part name
 * @param[in] func The function callback to remove
 *
 * @return The user data pointer if successful, or @c null otherwise
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void *edje_object_text_markup_filter_callback_del(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func);

/**
 * @brief Deletes a function and matching user data from the markup filter
 * list.
 *
 * Delete the given func filter and data user data from the list in part.
 * Returns the user data pointer given when added.
 *
 * See also @ref edje_object_text_markup_filter_callback_add and
 * @ref edje_object_text_markup_filter_callback_del
 *
 * @param[in] part The part name
 * @param[in] func The function callback to remove
 * @param[in] data The data passed to the callback function
 *
 * @return The same data pointer if successful, or @c null otherwise
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void *edje_object_text_markup_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data);

/**
 * @brief This function inserts text as if the user has inserted it.
 *
 * This means it actually registers as a change and emits signals, triggers
 * callbacks as appropriate.
 *
 * @param[in] part The part name
 * @param[in] text The text string
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_user_insert(const Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Inserts text for an object part.
 *
 * This function inserts the text for an object part at the end; It does not
 * move the cursor.
 *
 * @param[in] part The part name
 * @param[in] text The text string
 *
 * @since 1.1
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_append(Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Sets the text for an object part, but converts HTML escapes to UTF8
 *
 * This converts the given string text to UTF8 assuming it contains HTML style
 * escapes like "&amp;" and "&copy;" etc. IF the part is of type TEXT, as
 * opposed to TEXTBLOCK.
 *
 * @param[in] part The part name
 * @param[in] text The text string
 *
 * @return @c true on success, @c false otherwise
 *
 * @since 1.2
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_escaped_set(Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Sets the raw (non escaped) text for an object part.
 *
 * This function will not do escape for you if it is a TEXTBLOCK part, that is,
 * if text contain tags, these tags will not be interpreted/parsed by
 * TEXTBLOCK.
 *
 * See also @ref edje_object_part_text_unescaped_get().
 *
 * @param[in] part The part name
 * @param[in] text_to_escape The text string
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_unescaped_set(Evas_Object *obj, const char * part, const char *text_to_escape);

/**
 * @brief Returns the text of the object part, without escaping.
 *
 * This function is the counterpart of
 * @ref edje_object_part_text_unescaped_set(). Please notice that the result is
 * newly allocated memory and should be released with free() when done.
 *
 * See also @ref edje_object_part_text_unescaped_set().
 *
 * @param[in] part The part name
 *
 * @return The text string
 *
 * @ingroup Edje_Object
 */
EAPI char *edje_object_part_text_unescaped_get(const Evas_Object *obj, const char * part);

/**
 * @brief Inserts text for an object part.
 *
 * This function inserts the text for an object part just before the cursor
 * position.
 *
 * @param[in] part The part name
 * @param[in] text The text string
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_insert(Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Sets the autocapitalization type on the immodule.
 *
 * @param[in] part The part name
 * @param[in] autocapital_type The type of autocapitalization
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_autocapital_type_set(Evas_Object *obj, const char *part, Edje_Text_Autocapital_Type autocapital_type);

/**
 * @brief Retrieves the autocapitalization type
 *
 * @param[in] part The part name
 *
 * @return The type of autocapitalization
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI Edje_Text_Autocapital_Type edje_object_part_text_autocapital_type_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets whether the prediction is allowed or not.
 *
 * @param[in] part The part name
 * @param[in] prediction If @c true, the prediction feature is allowed.
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_prediction_allow_set(Evas_Object *obj, const char *part, Eina_Bool prediction);

/**
 * @brief Gets whether the prediction is allowed or not.
 *
 * @param[in] part The part name
 *
 * @return If @c true, the prediction feature is allowed.
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_prediction_allow_get(const Evas_Object *obj, const char *part);

/**
 * @brief Gets the input method context in entry.
 *
 * If ecore_imf was not available when edje was compiled, this function returns
 * @c null otherwise, the returned pointer is an Ecore_IMF
 *
 * @param[in] part The part name
 *
 * @return The input method context (Ecore_IMF_Context *) in entry
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void *edje_object_part_text_imf_context_get(const Evas_Object *obj, const char *part);

/**
 * @brief Resets the input method context if needed.
 *
 * This can be necessary in the case where modifying the buffer would confuse
 * on-going input method behavior
 *
 * @param[in] part The part name
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_imf_context_reset(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the input hint which allows input methods to fine-tune their
 * behavior.
 *
 * @param[in] part The part name
 * @param[in] input_hints Input hints
 *
 * @since 1.12.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_hint_set(Evas_Object *obj, const char *part, Edje_Input_Hints input_hints);

/**
 * @brief Gets the value of input hint
 *
 * @param[in] part The part name
 *
 * @return Input hints
 *
 * @since 1.12.0
 *
 * @ingroup Edje_Object
 */
EAPI Edje_Input_Hints edje_object_part_text_input_hint_get(const Evas_Object *obj, const char *part);

/**
 * @brief Shows the input panel (virtual keyboard) based on the input panel
 * property such as layout, autocapital types, and so on.
 *
 * Note that input panel is shown or hidden automatically according to the
 * focus state. This API can be used in the case of manually controlling by
 * using edje_object_part_text_input_panel_enabled_set.
 *
 * @param[in] part The part name
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_show(const Evas_Object *obj, const char *part);

/**
 * @brief Hides the input panel (virtual keyboard). See also
 * @ref edje_object_part_text_input_panel_show
 *
 * Note that input panel is shown or hidden automatically according to the
 * focus state. This API can be used in the case of manually controlling by
 * using edje_object_part_text_input_panel_enabled_set.
 *
 * @param[in] part The part name
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_hide(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the input panel-specific data to deliver to the input panel.
 *
 * This API is used by applications to deliver specific data to the input
 * panel. The data format MUST be negotiated by both application and the input
 * panel. The size and format of data are defined by the input panel.
 *
 * @param[in] part The part name
 * @param[in] data The specific data to be set to the input panel.
 * @param[in] len The length of data, in bytes, to send to the input panel
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_imdata_set(Evas_Object *obj, const char *part, const void *data, int len);

/**
 * @brief Gets the specific data of the current active input panel.
 *
 * @param[in] part The part name
 * @param[in] data The specific data to be set to the input panel.
 * @param[out] len The length of data, in bytes, to send to the input panel
 *
 * @return FIXME: void needed here?
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_imdata_get(const Evas_Object *obj, const char *part, void *data, int *len);

/**
 * @brief Sets the layout of the input panel.
 *
 * The layout of the input panel or virtual keyboard can make it easier or
 * harder to enter content. This allows you to hint what kind of input you are
 * expecting to enter and thus have the input panel automatically come up with
 * the right mode.
 *
 * @param[in] part The part name
 * @param[in] layout Layout type of the input panel
 *
 * @since 1.1
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_layout_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Layout layout);

/**
 * @brief Gets the layout of the input panel.
 *
 * See also @ref edje_object_part_text_input_panel_layout_set
 *
 * @param[in] part The part name
 *
 * @return Layout type of the input panel
 *
 * @since 1.1
 *
 * @ingroup Edje_Object
 */
EAPI Edje_Input_Panel_Layout edje_object_part_text_input_panel_layout_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the language mode of the input panel.
 *
 * This API can be used if you want to show the Alphabet keyboard.
 *
 * @param[in] part The part name
 * @param[in] lang The language to be set to the input panel.
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_language_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Lang lang);

/**
 * @brief Gets the language mode of the input panel.
 *
 * See also @ref edje_object_part_text_input_panel_language_set for more
 * details.
 *
 * @param[in] part The part name
 *
 * @return The language to be set to the input panel.
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI Edje_Input_Panel_Lang edje_object_part_text_input_panel_language_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the layout variation of the input panel.
 *
 * The layout variation of the input panel or virtual keyboard can make it
 * easier or harder to enter content. This allows you to hint what kind of
 * input you are expecting to enter and thus have the input panel automatically
 * come up with the right mode.
 *
 * @param[in] part The part name
 * @param[in] variation Layout variation type
 *
 * @since 1.8
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_layout_variation_set(Evas_Object *obj, const char *part, int variation);

/**
 * @brief Gets the layout variation of the input panel.
 *
 * See also @ref edje_object_part_text_input_panel_layout_variation_set
 *
 * @param[in] part The part name
 *
 * @return Layout variation type
 *
 * @since 1.8
 *
 * @ingroup Edje_Object
 */
EAPI int edje_object_part_text_input_panel_layout_variation_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the attribute to show the input panel automatically.
 *
 * @param[in] part The part name
 * @param[in] enabled If @c true, the input panel is appeared when entry is
 * clicked or has a focus
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_enabled_set(Evas_Object *obj, const char *part, Eina_Bool enabled);

/**
 * @brief Retrieves the attribute to show the input panel automatically. See
 * also @ref edje_object_part_text_input_panel_enabled_set
 *
 * @param[in] part The part name
 *
 * @return If @c true, the input panel is appeared when entry is clicked or has
 * a focus
 *
 * @since 1.1.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_input_panel_enabled_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the return key on the input panel to be disabled.
 *
 * @param[in] part The part name
 * @param[in] disabled The state
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_return_key_disabled_set(Evas_Object *obj, const char *part, Eina_Bool disabled);

/**
 * @brief Gets whether the return key on the input panel should be disabled or
 * not.
 *
 * @param[in] part The part name
 *
 * @return The state
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_input_panel_return_key_disabled_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the "return" key type. This type is used to set string or icon
 * on the "return" key of the input panel.
 *
 * An input panel displays the string or icon associated with this type
 *
 * @param[in] part The part name
 * @param[in] return_key_type The type of "return" key on the input panel
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_return_key_type_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Return_Key_Type return_key_type);

/**
 * @brief Gets the "return" key type.
 *
 * See also @ref edje_object_part_text_input_panel_return_key_type_set() for
 * more details
 *
 * @param[in] part The part name
 *
 * @return The type of "return" key on the input panel
 *
 * @since 1.2.0
 *
 * @ingroup Edje_Object
 */
EAPI Edje_Input_Panel_Return_Key_Type edje_object_part_text_input_panel_return_key_type_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the attribute to show the input panel in case of only an user's
 * explicit Mouse Up event. It doesn't request to show the input panel even
 * though it has focus.
 *
 * @param[in] part The part name
 * @param[in] ondemand If @c true, the input panel will be shown in case of
 * only Mouse up event. (Focus event will be ignored.)
 *
 * @since 1.9.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_input_panel_show_on_demand_set(Evas_Object *obj, const char *part, Eina_Bool ondemand);

/**
 * @brief Gets the attribute to show the input panel in case of only an user's
 * explicit Mouse Up event.
 *
 * @param[in] part The part name
 *
 * @return If @c true, the input panel will be shown in case of only Mouse up
 * event. (Focus event will be ignored.)
 *
 * @since 1.9.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_input_panel_show_on_demand_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the prediction hint to use an intelligent reply suggestion
 * service.
 *
 * @param[in] part The part name
 * @param[in] prediction_hint Prediction hint
 *
 * @since 1.20.0
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_prediction_hint_set(Evas_Object *obj, const char *part, const char *prediction_hint);

/**
 * @brief Sets the prediction hint data at the specified key.
 *
 * @param[in] part The part name
 * @param[in] key The key of the prediction hint
 * @param[in] value The data to replace
 *
 * @return @c true on success, @c false otherwise
 *
 * @since 1.21.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_prediction_hint_hash_set(Evas_Object *obj, const char *part, const char *key, const char *value);

/**
 * @brief Removes the prediction hint data identified by a key
 *
 * @param[in] part The part name
 * @param[in] key The key of the prediction hint
 *
 * @return @c true on success, @c false otherwise
 *
 * @since 1.21.0
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_text_prediction_hint_hash_del(Evas_Object *obj, const char *part, const char *key);

/**
 * @brief Starts selecting at current cursor position
 *
 * @param[in] part The part name
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_select_begin(const Evas_Object *obj, const char *part);

/**
 * @brief Aborts any selection action on a part.
 *
 * @param[in] part The part name
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_select_abort(const Evas_Object *obj, const char *part);

/**
 * @brief Extends the current selection to the current cursor position
 *
 * @param[in] part The part name
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_select_extend(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the selection to be everything.
 *
 * This function selects all text of the object of the part.
 *
 * @param[in] part The part name
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_select_all(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the selection to be none.
 *
 * This function sets the selection text to be none.
 *
 * @param[in] part The part name
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_text_select_none(const Evas_Object *obj, const char *part);

/**
 * @brief Returns the selection text of the object part.
 *
 * This function returns selection text of the object part.
 *
 * See also @ref edje_object_part_text_select_all() and
 * @ref edje_object_part_text_select_none()
 *
 * @param[in] part The part name
 *
 * @return The text string
 *
 * @ingroup Edje_Object
 */
EAPI const char *edje_object_part_text_selection_get(const Evas_Object *obj, const char *part);

/**
 * @brief Whether this object is playing or not.
 *
 * This property indicates whether the object is running or not. If stopped (or
 * paused), all transitions are disabled and programs stop running, until
 * resumed.
 *
 * If play is disabled, the object will remain the same, and its parts will not
 * change state. Note that play can be disabled during a transition between
 * states, effectively freezing the object in flight. When paused, no events
 * will be processed or sent.
 *
 * Setting to @c true resumes playing from the current state.
 *
 * Start or stop playing programs in this object.
 *
 * @param[in] play The play state, @c true by default.
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_play_set(Evas_Object *obj, Eina_Bool play);

/**
 * @brief Whether this object is playing or not.
 *
 * This property indicates whether the object is running or not. If stopped (or
 * paused), all transitions are disabled and programs stop running, until
 * resumed.
 *
 * If play is disabled, the object will remain the same, and its parts will not
 * change state. Note that play can be disabled during a transition between
 * states, effectively freezing the object in flight. When paused, no events
 * will be processed or sent.
 *
 * Setting to @c true resumes playing from the current state.
 *
 * Get the current state of play, @c true by default.
 *
 * @return The play state, @c true by default.
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_play_get(const Evas_Object *obj);

/**
 * @brief Transition duration factor.
 *
 * This defines a multiplier for the duration of transitions as they are
 * defined in EDC. By default this factor is 1.0, which means animations play
 * at the same speed as described in EDC.
 *
 * Sets transition duration factor.
 *
 * @param[in] scale The transition duration factor.
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_transition_duration_factor_set(Evas_Object *obj, double scale);

/**
 * @brief Transition duration factor.
 *
 * This defines a multiplier for the duration of transitions as they are
 * defined in EDC. By default this factor is 1.0, which means animations play
 * at the same speed as described in EDC.
 *
 * Gets transition duration factor.
 *
 * @return The transition duration factor.
 *
 * @ingroup Edje_Object
 */
EAPI double edje_object_transition_duration_factor_get(const Evas_Object *obj);

/**
 * @brief Gets the minimum size specified -- as an EDC property -- for a given
 * Edje object
 *
 * This function retrieves the obj object's minimum size values, as declared in
 * its EDC group definition. For instance, for an Edje object of minimum size
 * 100x100 pixels: collections { group { name: "a_group"; min: 100 100; } }
 *
 * @note If the @c min EDC property was not declared for this object, this call
 * will return 0x0.
 *
 * @note On failure, this function also return 0x0.
 *
 * See also @ref edje_object_size_max_get.
 *
 * @param[out] minw Pointer to a variable where to store the minimum width
 * @param[out] minh Pointer to a variable where to store the minimum height
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_size_min_get(const Evas_Object *obj, int *minw, int *minh);

/**
 * @brief Gets the maximum size specified -- as an EDC property -- for a given
 * Edje object
 *
 * This function retrieves the object's maximum size values, as declared in its
 * EDC group definition. For instance, for an Edje object of maximum size
 * 100x100 pixels: collections { group { name: "a_group"; max: 100 100; } }
 *
 * @note If the @c max EDC property was not declared for the object, this call
 * will return the maximum size a given Edje object may have, for each axis.
 *
 * @note On failure, this function will return 0x0.
 *
 * See also @ref edje_object_size_min_get.
 *
 * @param[out] maxw The maximum width
 * @param[out] maxh The maximum height
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_size_max_get(const Evas_Object *obj, int *maxw, int *maxh);

/**
 * @brief Checks if a part exists in a given Edje object's group definition.
 *
 * This function returns if a given part exists in the Edje group bound to this
 * object (with @ref edje_object_file_set()).
 *
 * This call is useful, for example, when one could expect or not a given GUI
 * element, depending on the theme applied to the object.
 *
 * @param[in] part The part's name to check for existence in obj's group
 *
 * @return @c true if the Edje part exists in obj's group, or @c false
 * otherwise (and on errors)
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_exists(const Evas_Object *obj, const char *part);


/**
 * @defgroup Edje_Perspective Edje Perspective
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with 3D projection of an 2D object.
 *
 * Perspective is a graphical tool that makes objects represented in 2D
 * look like they have a 3D appearance.
 *
 * Edje allows us to use perspective on any edje object. This group of
 * functions deal with the use of perspective, by creating and configuring
 * a perspective object that must set to a edje object or a canvas,
 * affecting all the objects inside that have no particular perspective
 * set already.
 *
 * @{
 */

/**
 * @brief Creates a new perspective in the given canvas.
 *
 * @param e The given canvas (Evas).
 * @return An @ref Edje_Perspective object for this canvas, or @c NULL on errors.
 *
 * This function creates a perspective object that can be set on an Edje
 * object, or globally to all Edje objects on this canvas.
 *
 * @see edje_perspective_set()
 * @see edje_perspective_free()
 */
EAPI Edje_Perspective       *edje_perspective_new            (Evas *e);
/**
 * @brief Deletes the given perspective object.
 *
 * @param ps A valid perspective object, or @c NULL.
 *
 * This function will delete the perspective object. If the perspective
 * effect was being applied to any Edje object or part, this effect won't be
 * applied anymore.
 *
 * @see edje_perspective_new()
 */
EAPI void                    edje_perspective_free           (Edje_Perspective *ps);
/**
 * @brief Sets up the transform for this perspective object.
 *
 * This sets the parameters of the perspective transformation. X, Y and Z
 * values are used. The px and py points specify the "infinite distance" point
 * in the 3D conversion (where all lines converge to like when artists draw
 * 3D by hand). The @p z0 value specifies the z value at which there is a 1:1
 * mapping between spatial coordinates and screen coordinates. Any points
 * on this z value will not have their X and Y values modified in the transform.
 * Those further away (Z value higher) will shrink into the distance, and
 * those less than this value will expand and become bigger. The @p foc value
 * determines the "focal length" of the camera. This is in reality the distance
 * between the camera lens plane itself (at or closer than this rendering
 * results are undefined) and the "z0" z value. This allows for some "depth"
 * control and @p foc must be greater than 0.
 *
 * @param ps The perspective object
 * @param px The perspective distance X coordinate
 * @param py The perspective distance Y coordinate
 * @param z0 The "0" z plane value
 * @param foc The focal distance
 */
EAPI void                    edje_perspective_set            (Edje_Perspective *ps, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);
/**
 * @brief Makes this perspective object be global for its canvas.
 *
 * @param ps The given perspective object
 * @param global @c EINA_TRUE if the perspective should be global, @c
 * EINA_FALSE otherwise.
 *
 * The canvas which this perspective object is being set as global is the one
 * given as argument upon the object creation (the @p evas parameter on the
 * function @c edje_perspective_new(evas) ).
 *
 * There can be only one global perspective object set per canvas, and if
 * a perspective object is set to global when there was already another
 * global perspective set, the old one will be set as non-global.
 *
 * A global perspective just affects a part if its Edje object doesn't have a
 * perspective object set to it, and if the part doesn't point to another
 * part to be used as perspective.
 *
 * @see edje_object_perspective_set()
 * @see edje_perspective_global_get()
 * @see edje_perspective_new()
 */
EAPI void                    edje_perspective_global_set     (Edje_Perspective *ps, Eina_Bool global);
/**
 * @brief Gets whether the given perspective object is global or not.
 *
 * @param ps The given perspective object.
 * @return @c EINA_TRUE if this perspective object is global, @c EINA_FALSE
 * otherwise.
 *
 * @see edje_perspective_global_set()
 */
EAPI Eina_Bool               edje_perspective_global_get     (const Edje_Perspective *ps);
/**
 * @brief Gets the global perspective object set for this canvas.
 *
 * @param e The given canvas (Evas).
 * @return The perspective object set as global for this canvas. Or @c NULL
 * if there is no global perspective set and on errors.
 *
 * This function will return the perspective object that was set as global
 * with edje_perspective_global_set().
 *
 * @see edje_perspective_global_set()
 * @see edje_perspective_global_get()
 */
EAPI const Edje_Perspective *edje_evas_global_perspective_get(const Evas *e);

/**
 * @brief Sets the given perspective object on this Edje object.
 *
 * Make the given perspective object be the default perspective for this Edje
 * object.
 *
 * There can be only one perspective object per Edje object, and if a previous
 * one was set, it will be removed and the new perspective object will be used.
 *
 * An Edje perspective will only affect a part if it doesn't point to another
 * part to be used as perspective.
 *
 * @ref edje_object_perspective_new() See also
 * @ref edje_object_perspective_get() @ref edje_perspective_set()
 *
 * @param[in] obj The object.
 * @param[in] ps The perspective object that will be used.
 */
EAPI void edje_object_perspective_set(Evas_Object *obj, Edje_Perspective *ps);

/**
 * @brief Gets the current perspective used on this Edje object.
 *
 * See also @ref edje_object_perspective_set()
 *
 * @param[in] obj The object.
 *
 * @return The perspective object that will be used.
 */
EAPI const Edje_Perspective *edje_object_perspective_get(const Evas_Object *obj);

/**
 * @}
 */


/**
 * @brief Sets the function that provides item objects for named items in an
 * edje entry text
 *
 * Item objects may be deleted any time by Edje, and will be deleted when the
 * Edje object is deleted (or file is set to a new file).
 *
 * @param[in] obj The object.
 * @param[in] func The function to call (or @c null to disable) to get item
 * objects
 * @param[in] data The data pointer to pass to the func callback
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_item_provider_set(Edje_Object *obj, Edje_Item_Provider_Cb func, void *data);


/**
 * @brief Gets the description of an object color class.
 *
 * This function gets the description of a color class in use by an object.
 *
 * @param[in] color_class Color class description
 *
 * @return The description of the target color class or @c null if not found
 *
 * @ingroup Edje_Object
 */
EAPI const char *edje_object_color_class_description_get(const Edje_Object *obj, const char * color_class);

typedef Efl_Canvas_Layout_Part_Type Edje_Part_Type;
#define EDJE_PART_TYPE_NONE        EFL_CANVAS_LAYOUT_PART_TYPE_NONE
#define EDJE_PART_TYPE_RECTANGLE   EFL_CANVAS_LAYOUT_PART_TYPE_RECTANGLE
#define EDJE_PART_TYPE_TEXT        EFL_CANVAS_LAYOUT_PART_TYPE_TEXT
#define EDJE_PART_TYPE_IMAGE       EFL_CANVAS_LAYOUT_PART_TYPE_IMAGE
#define EDJE_PART_TYPE_SWALLOW     EFL_CANVAS_LAYOUT_PART_TYPE_SWALLOW
#define EDJE_PART_TYPE_TEXTBLOCK   EFL_CANVAS_LAYOUT_PART_TYPE_TEXTBLOCK
#define EDJE_PART_TYPE_GRADIENT    EFL_CANVAS_LAYOUT_PART_TYPE_GRADIENT
#define EDJE_PART_TYPE_GROUP       EFL_CANVAS_LAYOUT_PART_TYPE_GROUP
#define EDJE_PART_TYPE_BOX         EFL_CANVAS_LAYOUT_PART_TYPE_BOX
#define EDJE_PART_TYPE_TABLE       EFL_CANVAS_LAYOUT_PART_TYPE_TABLE
#define EDJE_PART_TYPE_EXTERNAL    EFL_CANVAS_LAYOUT_PART_TYPE_EXTERNAL
#define EDJE_PART_TYPE_PROXY       EFL_CANVAS_LAYOUT_PART_TYPE_PROXY
#define EDJE_PART_TYPE_SPACER      EFL_CANVAS_LAYOUT_PART_TYPE_SPACER
#define EDJE_PART_TYPE_MESH_NODE   EFL_CANVAS_LAYOUT_PART_TYPE_MESH_NODE
#define EDJE_PART_TYPE_LIGHT       EFL_CANVAS_LAYOUT_PART_TYPE_LIGHT
#define EDJE_PART_TYPE_CAMERA      EFL_CANVAS_LAYOUT_PART_TYPE_CAMERA
#define EDJE_PART_TYPE_SNAPSHOT    EFL_CANVAS_LAYOUT_PART_TYPE_SNAPSHOT
#define EDJE_PART_TYPE_VECTOR      EFL_CANVAS_LAYOUT_PART_TYPE_VECTOR
#define EDJE_PART_TYPE_LAST        EFL_CANVAS_LAYOUT_PART_TYPE_LAST

/**
 * @}
 */
#include "efl_canvas_layout_eo.legacy.h"
#include "edje_edit.eo.legacy.h"
#include "efl_layout_group_eo.legacy.h"
