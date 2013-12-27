/**
 * @defgroup Edje_External_Part_Group Edje Use of External Parts
 *
 * @brief Functions to manipulate parts of type EXTERNAL.
 *
 * Edje supports parts of type EXTERNAL, which will call plugins defined by the user
 * to create and manipulate the object that's allocated in that part.
 *
 * Parts of type external may carry extra properties that have meanings defined
 * by the external plugin. For instance, it may be a string that defines a button
 * label and setting this property will change that label on the fly.
 *
 * @ingroup Edje_External_Group
 *
 * @{
 */

/**
 * @brief Get the object created by this external part.
 *
 * Parts of type external creates the part object using information
 * provided by external plugins. It's somehow like "swallow"
 * (edje_object_part_swallow()), but it's all set automatically.
 *
 * This function returns the part created by such external plugins and
 * being currently managed by this Edje.
 *
 * @note Almost all swallow rules apply: you should not move, resize,
 *       hide, show, set the color or clipper of such part. It's a bit
 *       more restrictive as one must @b never delete this object!
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The externally created object, or NULL if there is none or
 *         part is not an external.
 */
EAPI Evas_Object              *edje_object_part_external_object_get     (const Evas_Object *obj, const char *part);

/**
 * @brief Set the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have
 * meanings defined by the external plugin. For instance, it may be a
 * string that defines a button label and setting this property will
 * change that label on the fly.
 *
 * @note external parts have parameters set when they change
 *       states. Those parameters will never be changed by this
 *       function. The interpretation of how state_set parameters and
 *       param_set will interact is up to the external plugin.
 *
 * @note this function will not check if parameter value is valid
 *       using #Edje_External_Param_Info minimum, maximum, valid
 *       choices and others. However these should be checked by the
 *       underlying implementation provided by the external
 *       plugin. This is done for performance reasons.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param param the parameter details, including its name, type and
 *        actual value. This pointer should be valid, and the
 *        parameter must exist in
 *        #Edje_External_Type::parameters_info, with the exact type,
 *        otherwise the operation will fail and @c EINA_FALSE will be
 *        returned.
 *
 * @return @c EINA_TRUE if everything went fine, @c EINA_FALSE on errors.
 */
EAPI Eina_Bool                 edje_object_part_external_param_set      (Evas_Object *obj, const char *part, const Edje_External_Param *param);

/**
 * @brief Get the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have
 * meanings defined by the external plugin. For instance, it may be a
 * string that defines a button label. This property can be modified by
 * state parameters, by explicit calls to
 * edje_object_part_external_param_set() or getting the actual object
 * with edje_object_part_external_object_get() and calling native
 * functions.
 *
 * This function asks the external plugin what is the current value,
 * independent on how it was set.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name

 * @param param the parameter details. It is used as both input and
 *        output variable. This pointer should be valid, and the
 *        parameter must exist in
 *        #Edje_External_Type::parameters_info, with the exact type,
 *        otherwise the operation will fail and @c EINA_FALSE will be
 *        returned.
 *
 * @return @c EINA_TRUE if everything went fine and @p param members
 *         are filled with information, @c EINA_FALSE on errors and @p
 *         param member values are not set or valid.
 */
EAPI Eina_Bool                 edje_object_part_external_param_get      (const Evas_Object *obj, const char *part, Edje_External_Param *param);

/**
 * @brief Get an object contained in an part of type EXTERNAL
 *
 * The @p content string must not be NULL. Its actual value depends on the
 * code providing the EXTERNAL.
 *
 * @param obj The Edje object
 * @param part The name of the part holding the EXTERNAL
 * @param content A string identifying which content from the EXTERNAL to get
 */
EAPI Evas_Object              *edje_object_part_external_content_get    (const Evas_Object *obj, const char *part, const char *content);

/**
 * Facility to query the type of the given parameter of the given part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param param the parameter name to use.
 *
 * @return @c EDJE_EXTERNAL_PARAM_TYPE_MAX on errors, or another value
 *         from #Edje_External_Param_Type on success.
 */
EAPI Edje_External_Param_Type  edje_object_part_external_param_type_get (const Evas_Object *obj, const char *part, const char *param);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Geometry_Group
 *
 * @{
 */

/**
 * @brief Get the minimum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum width
 * @param minh Pointer to a variable where to store the minimum height
 *
 * This function retrieves the @p obj object's minimum size values,
 * <b>as declared in its EDC group definition</b>. Minimum size of
 * groups have the following syntax
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     min: 100 100;
 *   }
 * }
 * @endcode
 *
 * where one declares a minimum size of 100 pixels both for width and
 * height. Those are (hint) values which should be respected when the
 * given object/group is to be controlled by a given container object
 * (e.g. an Edje object being "swallowed" into a given @c SWALLOW
 * typed part, as in edje_object_part_swallow()). Check the complete
 * @ref edcref "syntax reference" for EDC files.
 *
 * @note If the @c min EDC property was not declared for @p obj, this
 * call will return the value 0, for each axis.
 *
 * @note On failure, this function will make all non-@c NULL size
 * pointers' pointed variables be set to zero.
 *
 * @see edje_object_size_max_get()
 */
EAPI void         edje_object_size_min_get            (const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

/**
 * @brief Edje will automatically update the size hints on itself.
 *
 * @param obj A handle to an Edje object.
 * @param update Wether or not update the size hints.
 *
 * By default edje doesn't set size hints on itself. With this function
 * call, it will do so if update is true. Be carefully, it cost a lot to
 * trigger this feature as it will recalc the object every time it make
 * sense to be sure that's its minimal size hint is always accurate.
 */
EAPI void edje_object_update_hints_set(Evas_Object *obj, Eina_Bool update);

/**
 * @brief Wether or not Edje will update size hints on itself.
 *
 * @param obj A handle to an Edje object.
 * @return @c true if does, @c false if it doesn't.
 */
EAPI Eina_Bool edje_object_update_hints_get(Evas_Object *obj);

/**
 * @brief Get the maximum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param obj A handle to an Edje object
 * @param maxw Pointer to a variable where to store the maximum width
 * @param maxh Pointer to a variable where to store the maximum height
 *
 * This function retrieves the @p obj object's maximum size values,
 * <b>as declared in its EDC group definition</b>. Maximum size of
 * groups have the following syntax
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     max: 100 100;
 *   }
 * }
 * @endcode
 *
 * where one declares a maximum size of 100 pixels both for width and
 * height. Those are (hint) values which should be respected when the
 * given object/group is to be controlled by a given container object
 * (e.g. an Edje object being "swallowed" into a given @c SWALLOW
 * typed part, as in edje_object_part_swallow()). Check the complete
 * @ref edcref "syntax reference" for EDC files.
 *
 * @note If the @c max EDC property was not declared for @p obj, this
 * call will return the maximum size a given Edje object may have, for
 * each axis.
 *
 * @note On failure, this function will make all non-@c NULL size
 * pointers' pointed variables be set to zero.
 *
 * @see edje_object_size_min_get()
 */
EAPI void         edje_object_size_max_get            (const Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh);

/**
 * @brief Force a Size/Geometry calculation.
 *
 * @param obj A valid Evas_Object handle
 *
 * Forces the object @p obj to recalculation layout regardless of
 * freeze/thaw.
 */
EAPI void         edje_object_calc_force              (Evas_Object *obj);

/**
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum
 * required width
 * @param minh Pointer to a variable where to store the minimum
 * required height
 *
 * This call works exactly as edje_object_size_min_restricted_calc(),
 * with the last two arguments set to 0. Please refer to its
 * documentation, then.
 */
EAPI void         edje_object_size_min_calc           (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

/**
 * Calculate the geometry of the region, relative to a given Edje
 * object's area, <b>occupied by all parts in the object</b>
 *
 * @param obj A handle to an Edje object
 * @param x A pointer to a variable where to store the parts region's
 * x coordinate
 * @param y A pointer to a variable where to store the parts region's
 * y coordinate
 * @param w A pointer to a variable where to store the parts region's
 * width
 * @param h A pointer to a variable where to store the parts region's
 * height
 *
 * This function gets the geometry of the rectangle equal to the area
 * required to group all parts in @p obj's group/collection. The @p x
 * and @p y coordinates are relative to the top left corner of the
 * whole @p obj object's area. Parts placed out of the group's
 * boundaries will also be taken in account, so that @p x and @p y
 * <b>may be negative</b>.
 *
 * @note Use @c NULL pointers on the geometry components you're not
 * interested in: they'll be ignored by the function.
 *
 * @note On failure, this function will make all non-@c NULL geometry
 * pointers' pointed variables be set to zero.
 */
EAPI Eina_Bool    edje_object_parts_extends_calc      (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum
 * required width
 * @param minh Pointer to a variable where to store the minimum
 * required height
 * @param restrictedw Do not allow object's calculated (minimum) width
 * to be less than this value
 * @param restrictedh Do not allow object's calculated (minimum)
 * height to be less than this value
 *
 * This call will trigger an internal recalculation of all parts of
 * the @p obj object, in order to return its minimum required
 * dimensions for width and height. The user might choose to @b impose
 * those minimum sizes, making the resulting calculation to get to values
 * equal or bigger than @p restrictedw and @p restrictedh, for width and
 * height, respectively.
 *
 * @note At the end of this call, @p obj @b won't be automatically
 * resized to new dimensions, but just return the calculated
 * sizes. The caller is the one up to change its geometry or not.
 *
 * @warning Be advised that invisible parts in @p obj @b will be taken
 * into account in this calculation.
 */
EAPI void         edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Scale
 *
 * @{
 */

/**
 * @brief Set the scaling factor for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param scale The scaling factor (the default value is @c 0.0,
 * meaning individual scaling @b not set)
 *
 * This function sets an @b individual scaling factor on the @a obj
 * Edje object. This property (or Edje's global scaling factor, when
 * applicable), will affect this object's part sizes. If @p scale is
 * not zero, than the individual scaling will @b override any global
 * scaling set, for the object @p obj's parts. Put it back to zero to
 * get the effects of the global scaling again.
 *
 * @warning Only parts which, at EDC level, had the @c "scale"
 * property set to @c 1, will be affected by this function. Check the
 * complete @ref edcref "syntax reference" for EDC files.
 *
 * @see edje_object_scale_get()
 * @see edje_scale_get() for more details
 */
EAPI Eina_Bool    edje_object_scale_set           (Evas_Object *obj, double scale);

/**
 * @brief Get a given Edje object's scaling factor.
 *
 * @param obj A handle to an Edje object
 *
 * This function returns the @c individual scaling factor set on the
 * @a obj Edje object.
 *
 * @see edje_object_scale_set() for more details
 *
 */
EAPI double       edje_object_scale_get           (const Evas_Object *obj);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Part
 *
 * @{
 */

/**
 * @brief Check if an Edje part exists in a given Edje object's group
 * definition.
 *
 * @param obj A handle to an Edje object
 * @param part The part's name to check for existence in @p obj's
 * group
 * @return @c EINA_TRUE, if the Edje part exists in @p obj's group or
 * @c EINA_FALSE, otherwise (and on errors)
 *
 * This function returns if a given part exists in the Edje group
 * bound to object @p obj (with edje_object_file_set()).
 *
 * This call is useful, for example, when one could expect or not a
 * given GUI element, depending on the @b theme applied to @p obj.
 */
EAPI Eina_Bool    edje_object_part_exists             (const Evas_Object *obj, const char *part);

/**
 * @brief Get a handle to the Evas object implementing a given Edje
 * part, in an Edje object.
 *
 * @param obj A handle to an Edje object
 * @param part The Edje part's name
 * @return A pointer to the Evas object implementing the given part,
 * or @c NULL on failure (e.g. the given part doesn't exist)
 *
 * This function gets a pointer of the Evas object corresponding to a
 * given part in the @p obj object's group.
 *
 * You should @b never modify the state of the returned object (with
 * @c evas_object_move() or @c evas_object_hide() for example),
 * because it's meant to be managed by Edje, solely. You are safe to
 * query information about its current state (with @c
 * evas_object_visible_get() or @c evas_object_color_get() for
 * example), though.
 */
EAPI const Evas_Object *edje_object_part_object_get   (const Evas_Object *obj, const char *part);

/**
 * @brief Retrieve the geometry of a given Edje part, in a given Edje
 * object's group definition, <b>relative to the object's area</b>
 *
 * @param obj A handle to an Edje object
 * @param part The Edje part's name
 * @param x A pointer to a variable where to store the part's x
 * coordinate
 * @param y A pointer to a variable where to store the part's y
 * coordinate
 * @param w A pointer to a variable where to store the part's width
 * @param h A pointer to a variable where to store the part's height
 *
 * This function gets the geometry of an Edje part within its
 * group. The @p x and @p y coordinates are relative to the top left
 * corner of the whole @p obj object's area.
 *
 * @note Use @c NULL pointers on the geometry components you're not
 * interested in: they'll be ignored by the function.
 *
 * @note On failure, this function will make all non-@c NULL geometry
 * pointers' pointed variables be set to zero.
 */
EAPI Eina_Bool    edje_object_part_geometry_get       (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Retrieve a list all accessibility part names
 *
 * @param obj A valid Evas_Object handle
 * @return A list all accessibility part names on @p obj
 * @since 1.7.0
 */
EAPI Eina_List    *edje_object_access_part_list_get   (const Evas_Object *obj);

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Text
 *
 * @{
 */

/**
 * @brief Set the object text callback.
 *
 * @param obj A valid Evas_Object handle
 * @param func The callback function to handle the text change
 * @param data The data associated to the callback function.
 *
 * This function sets the callback to be called when the text changes.
 */
EAPI void         edje_object_text_change_cb_set      (Evas_Object *obj, Edje_Text_Change_Cb func, void *data);

/**
 * @brief Sets the text for an object part
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 */
EAPI Eina_Bool    edje_object_part_text_set           (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Sets the text for an object part, but converts HTML escapes to UTF8
 *
 * This converts the given string @p text to UTF8 assuming it contains HTML
 * style escapes like "&amp;" and "&copy;" etc. IF the part is of type TEXT,
 * as opposed to TEXTBLOCK.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @since 1.2
 */
EAPI Eina_Bool    edje_object_part_text_escaped_set   (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Return the text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The text string
 *
 * This function returns the text associated to the object part.
 *
 * @see edje_object_part_text_set().
 */
EAPI const char  *edje_object_part_text_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Set the style of the
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param style The style to set (textblock conventions).
 *
 * This function sets the style associated with the textblock part.
 *
 * @since 1.2.0
 */
EAPI void edje_object_part_text_style_user_push(Evas_Object *obj, const char *part, const char *style);

/**
 * @brief Return the text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The text string
 *
 * This function returns the style associated with the textblock part.
 *
 * @since 1.2.0
 */
EAPI const char *edje_object_part_text_style_user_peek(const Evas_Object *obj, const char *part);

/**
 * @brief Delete the top style form the user style stack.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @since 1.2.0
 */
EAPI void edje_object_part_text_style_user_pop(Evas_Object *obj, const char *part);

/**
 * @brief Sets the raw (non escaped) text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text_to_escape The text string
 *
 * This funciton will not do escape for you if it is a TEXTBLOCK part, that is,
 * if text contain tags, these tags will not be interpreted/parsed by TEXTBLOCK.
 *
 * @see edje_object_part_text_unescaped_get().
 */
EAPI Eina_Bool    edje_object_part_text_unescaped_set (Evas_Object *obj, const char *part, const char *text_to_escape);

/**
 * @brief Returns the text of the object part, without escaping.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The @b allocated text string without escaping, or NULL on
 * problems.
 *
 * This function is the counterpart of
 * edje_object_part_text_unescaped_set(). Please notice that the
 * result is newly allocated memory and should be released with free()
 * when done.
 *
 * @see edje_object_part_text_unescaped_set().
 */
EAPI char        *edje_object_part_text_unescaped_get (const Evas_Object *obj, const char *part);

/**
 * @brief Insert text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * This function inserts the text for an object part just before the
 * cursor position.
 *
 */
EAPI void             edje_object_part_text_insert                  (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Insert text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * This function inserts the text for an object part at the end; It does not
 * move the cursor.
 *
 * @since 1.1
 */
EAPI void             edje_object_part_text_append(Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Return a list of char anchor names.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The list of anchors (const char *), do not modify!
 *
 * This function returns a list of char anchor names.
 *
 */
EAPI const Eina_List *edje_object_part_text_anchor_list_get         (const Evas_Object *obj, const char *part);

/**
 * @brief Return a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param anchor The anchor name
 *
 * @return The list of anchor rects (const Evas_Textblock_Rectangle
 * *), do not modify! Geometry is relative to entry part.
 *
 * This function return a list of Evas_Textblock_Rectangle anchor
 * rectangles.
 *
 */
EAPI const Eina_List *edje_object_part_text_anchor_geometry_get     (const Evas_Object *obj, const char *part, const char *anchor);

/**
 * @brief Return a list of char item names.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The list of items (const char *), do not modify!
 *
 * This function returns a list of char item names.
 *
 */
EAPI const Eina_List *edje_object_part_text_item_list_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Return item geometry.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param item The item name
 * @param cx Item x return (relative to entry part)
 * @param cy Item y return (relative to entry part)
 * @param cw Item width return
 * @param ch Item height return
 *
 * @return 1 if item exists, 0 if not
 *
 * This function return a list of Evas_Textblock_Rectangle item
 * rectangles.
 *
 */
EAPI Eina_Bool        edje_object_part_text_item_geometry_get       (const Evas_Object *obj, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);

/**
 * @brief This function inserts text as if the user has inserted it.
 *
 * This means it actually registers as a change and emits signals, triggers
 * callbacks as appropriate.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param text The text string
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_user_insert        (const Evas_Object *obj, const char *part, const char *text);

/**
 * @}
 */

/**
 * @ingroup Edje_Text_Selection
 *
 * @{
 */

/**
 * @brief Return the selection text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The text string
 *
 * This function returns selection text of the object part.
 *
 * @see edje_object_part_text_select_all()
 * @see edje_object_part_text_select_none()
 */
EAPI const char      *edje_object_part_text_selection_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Set the selection to be none.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * This function sets the selection text to be none.
 */
EAPI void             edje_object_part_text_select_none             (const Evas_Object *obj, const char *part);

/**
 * @brief Set the selection to be everything.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * This function selects all text of the object of the part.
 */
EAPI void             edje_object_part_text_select_all              (const Evas_Object *obj, const char *part);

/**
 * @brief Enables selection if the entry is an EXPLICIT selection mode
 * type.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param allow EINA_TRUE to enable, EINA_FALSE otherwise
 *
 * The default is to @b not allow selection. This function only affects user
 * selection, functions such as edje_object_part_text_select_all() and
 * edje_object_part_text_select_none() are not affected.
 */
EAPI void             edje_object_part_text_select_allow_set        (const Evas_Object *obj, const char *part, Eina_Bool allow);

/**
 * @brief Aborts any selection action on a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_abort            (const Evas_Object *obj, const char *part);

/**
 * @brief Starts selecting at current cursor position
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_begin            (const Evas_Object *obj, const char *part);

/**
 * @brief Extends the current selection to the current cursor position
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_extend           (const Evas_Object *obj, const char *part);

/**
 * @}
 */

/**
 * @ingroup Edje_Text_Cursor
 *
 * @{
 */

/**
 * @brief Advances the cursor to the next cursor position.
 * @see evas_textblock_cursor_char_next
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The edje cursor to advance
 */
EAPI Eina_Bool        edje_object_part_text_cursor_next                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the previous char
 * @see evas_textblock_cursor_char_prev
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_prev                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Move the cursor to the char above the current cursor position.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_up                   (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the char below the current cursor position.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_down                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the beginning of the text part
 * @see evas_textblock_cursor_paragraph_first
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_begin_set            (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the end of the text part.
 * @see evas_textblock_cursor_paragraph_last
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_end_set              (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Copy the cursor to another cursor.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param src the cursor to copy from
 * @param dst the cursor to copy to
 */
EAPI void             edje_object_part_text_cursor_copy                 (Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst);

/**
 * @brief Move the cursor to the beginning of the line.
 * @see evas_textblock_cursor_line_char_first
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_line_begin_set       (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Move the cursor to the end of the line.
 * @see evas_textblock_cursor_line_char_last
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_line_end_set         (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * Position the given cursor to a X,Y position.
 *
 * This is frequently used with the user cursor.
 *
 * @param obj An Edje object.
 * @param part The part containing the object.
 * @param cur The cursor to adjust.
 * @param x X Coordinate.
 * @param y Y Coordinate.
 * @return True on success, false on error.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_coord_set            (Evas_Object *obj, const char *part, Edje_Cursor cur, Evas_Coord x, Evas_Coord y);

/**
 * @brief Returns whether the cursor points to a format.
 * @see evas_textblock_cursor_is_format
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to adjust.
 * @return EINA_TRUE if it's true, EINA_FALSE otherwise.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_is_format_get        (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Return true if the cursor points to a visible format
 * For example \\t, \\n, item and etc.
 * @see  evas_textblock_cursor_format_is_visible_get
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to adjust.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Returns the content (char) at the cursor position.
 * @see evas_textblock_cursor_content_get
 *
 * You must free the return (if not NULL) after you are done with it.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to use
 * @return The character string pointed to (may be a multi-byte utf8 sequence) terminated by a nul byte.
 */
EAPI char            *edje_object_part_text_cursor_content_get          (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Sets the cursor position to the given value
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to move
 * @param pos the position of the cursor
 * @since 1.1.0
 */
EAPI void             edje_object_part_text_cursor_pos_set              (Evas_Object *obj, const char *part, Edje_Cursor cur, int pos);

/**
 * @brief Retrieves the current position of the cursor
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to get the position
 * @return The cursor position
 * @since 1.1.0
 */
EAPI int              edje_object_part_text_cursor_pos_get              (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Returns the cursor geometry of the part relative to the edje
 * object.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param x Cursor X position
 * @param y Cursor Y position
 * @param w Cursor width
 * @param h Cursor height
 *
 */
EAPI void             edje_object_part_text_cursor_geometry_get     (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @}
 */

/**
 * @ingroup Edje_Text_Entry
 *
 * @{
 */

/**
 * @brief Set the RTL orientation for this object.
 *
 * @param obj A handle to an Edje object.
 * @param rtl new value of flag EINA_TRUE/EINA_FALSE
 * @since 1.1.0
 */
EAPI void         edje_object_mirrored_set        (Evas_Object *obj, Eina_Bool rtl);

/**
 * @brief Get the RTL orientation for this object.
 *
 * You can RTL orientation explicitly with edje_object_mirrored_set.
 *
 * @param obj A handle to an Edje object.
 * @return @c EINA_TRUE if the flag is set or @c EINA_FALSE if not.
 * @since 1.1.0
 */
EAPI Eina_Bool    edje_object_mirrored_get        (const Evas_Object *obj);

/**
 * @brief Set the function that provides item objects for named items in an edje entry text
 *
 * @param obj A valid Evas Object handle
 * @param func The function to call (or NULL to disable) to get item objects
 * @param data The data pointer to pass to the @p func callback
 *
 * Item objects may be deleted any time by Edje, and will be deleted when the
 * Edje object is deleted (or file is set to a new file).
 */
EAPI void         edje_object_item_provider_set       (Evas_Object *obj, Edje_Item_Provider_Cb func, void *data);

/**
 * @brief Reset the input method context if needed.
 *
 * This can be necessary in the case where modifying the buffer would confuse on-going input method behavior
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_imf_context_reset           (const Evas_Object *obj, const char *part);

/**
 * @brief Get the input method context in entry.
 *
 * If ecore_imf was not available when edje was compiled, this function returns NULL
 * otherwise, the returned pointer is an Ecore_IMF *
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The input method context (Ecore_IMF_Context *) in entry
 * @since 1.2.0
 */
EAPI void              *edje_object_part_text_imf_context_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Set the layout of the input panel.
 *
 * The layout of the input panel or virtual keyboard can make it easier or
 * harder to enter content. This allows you to hint what kind of input you
 * are expecting to enter and thus have the input panel automatically
 * come up with the right mode.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param layout layout type
 * @since 1.1
 */
EAPI void             edje_object_part_text_input_panel_layout_set      (Evas_Object *obj, const char *part, Edje_Input_Panel_Layout layout);

/**
 * @brief Get the layout of the input panel.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return Layout type of the input panel
 *
 * @see edje_object_part_text_input_panel_layout_set
 * @since 1.1
 */
EAPI Edje_Input_Panel_Layout edje_object_part_text_input_panel_layout_get (const Evas_Object *obj, const char *part);

/**
 * @brief Set the layout variation of the input panel.
 *
 * The layout variation of the input panel or virtual keyboard can make it easier or
 * harder to enter content. This allows you to hint what kind of input you
 * are expecting to enter and thus have the input panel automatically
 * come up with the right mode.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param variation layout variation type
 * @since 1.8
 */
EAPI void             edje_object_part_text_input_panel_layout_variation_set(Evas_Object *obj, const char *part, int variation);

/**
 * @brief Get the layout variation of the input panel.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return Layout variation type of the input panel
 *
 * @see edje_object_part_text_input_panel_layout_variation_set
 * @since 1.8
 */
EAPI int              edje_object_part_text_input_panel_layout_variation_get(const Evas_Object *obj, const char *part);

/**
 * @brief Set the autocapitalization type on the immodule.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param autocapital_type The type of autocapitalization
 * @since 1.1.0
 */
EAPI void         edje_object_part_text_autocapital_type_set            (Evas_Object *obj, const char *part, Edje_Text_Autocapital_Type autocapital_type);

/**
 * @brief Retrieves the autocapitalization type
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The autocapitalization type
 * @since 1.1.0
 */
EAPI Edje_Text_Autocapital_Type edje_object_part_text_autocapital_type_get (const Evas_Object *obj, const char *part);

/**
 * @brief Set whether the prediction is allowed or not.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param prediction If true, the prediction feature is allowed.
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_prediction_allow_set        (Evas_Object *obj, const char *part, Eina_Bool prediction);

/**
 * @brief Get whether the prediction is allowed or not.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return EINA_TRUE if prediction feature is allowed.
 * @since 1.2.0
 */
EAPI Eina_Bool        edje_object_part_text_prediction_allow_get        (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the attribute to show the input panel automatically.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param enabled If true, the input panel is appeared when entry is clicked or has a focus
 * @since 1.1.0
 */
EAPI void             edje_object_part_text_input_panel_enabled_set     (Evas_Object *obj, const char *part, Eina_Bool enabled);

/**
 * @brief Retrieve the attribute to show the input panel automatically.
 * @see edje_object_part_text_input_panel_enabled_set
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return EINA_TRUE if it supports or EINA_FALSE otherwise
 * @since 1.1.0
 */
EAPI Eina_Bool        edje_object_part_text_input_panel_enabled_get (const Evas_Object *obj, const char *part);

/**
 * @brief Show the input panel (virtual keyboard) based on the input panel property such as layout, autocapital types, and so on.
 *
 * Note that input panel is shown or hidden automatically according to the focus state.
 * This API can be used in the case of manually controlling by using edje_object_part_text_input_panel_enabled_set.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_show(const Evas_Object *obj, const char *part);

/**
 * @brief Hide the input panel (virtual keyboard).
 * @see edje_object_part_text_input_panel_show
 *
 * Note that input panel is shown or hidden automatically according to the focus state.
 * This API can be used in the case of manually controlling by using edje_object_part_text_input_panel_enabled_set.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_hide(const Evas_Object *obj, const char *part);

/**
 * Set the language mode of the input panel.
 *
 * This API can be used if you want to show the Alphabet keyboard.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param lang the language to be set to the input panel.
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_language_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Lang lang);

/**
 * Get the language mode of the input panel.
 *
 * See @ref edje_object_part_text_input_panel_language_set for more details.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return input panel language type
 * @since 1.2.0
 */
EAPI Edje_Input_Panel_Lang edje_object_part_text_input_panel_language_get(const Evas_Object *obj, const char *part);

/**
 * Set the input panel-specific data to deliver to the input panel.
 *
 * This API is used by applications to deliver specific data to the input panel.
 * The data format MUST be negotiated by both application and the input panel.
 * The size and format of data are defined by the input panel.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param data The specific data to be set to the input panel.
 * @param len the length of data, in bytes, to send to the input panel
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_imdata_set(Evas_Object *obj, const char *part, const void *data, int len);

/**
 * Get the specific data of the current active input panel.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param data The specific data to be got from the input panel
 * @param len The length of data
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_imdata_get(const Evas_Object *obj, const char *part, void *data, int *len);

/**
 * Set the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * An input panel displays the string or icon associated with this type
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param return_key_type The type of "return" key on the input panel
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_return_key_type_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Return_Key_Type return_key_type);

/**
 * Get the "return" key type.
 *
 * @see edje_object_part_text_input_panel_return_key_type_set() for more details
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The type of "return" key on the input panel
 * @since 1.2.0
 */
EAPI Edje_Input_Panel_Return_Key_Type edje_object_part_text_input_panel_return_key_type_get(const Evas_Object *obj, const char *part);

/**
 * Set the return key on the input panel to be disabled.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param disabled The state
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_return_key_disabled_set(Evas_Object *obj, const char *part, Eina_Bool disabled);

/**
 * Get whether the return key on the input panel should be disabled or not.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return EINA_TRUE if it should be disabled
 * @since 1.2.0
 */
EAPI Eina_Bool        edje_object_part_text_input_panel_return_key_disabled_get(const Evas_Object *obj, const char *part);

/**
 * Set the attribute to show the input panel in case of only an user's explicit Mouse Up event.
 * It doesn't request to show the input panel even though it has focus.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param ondemand If true, the input panel will be shown in case of only Mouse up event. (Focus event will be ignored.)
 * @since 1.9.0
 */
EAPI void             edje_object_part_text_input_panel_show_on_demand_set(Evas_Object *obj, const char *part, Eina_Bool ondemand);

/**
 * Get the attribute to show the input panel in case of only an user's explicit Mouse Up event.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return @c EINA_TRUE if the input panel will be shown in case of only Mouse up event.
 * @since 1.9.0
 */
EAPI Eina_Bool        edje_object_part_text_input_panel_show_on_demand_get(const Evas_Object *obj, const char *part);

/**
 * Add a filter function for newly inserted text.
 *
 * Whenever text is inserted (not the same as set) into the given @p part,
 * the list of filter functions will be called to decide if and how the new
 * text will be accepted.
 * There are three types of filters, EDJE_TEXT_FILTER_TEXT,
 * EDJE_TEXT_FILTER_FORMAT and EDJE_TEXT_FILTER_MARKUP.
 * The text parameter in the @p func filter can be modified by the user and
 * it's up to him to free the one passed if he's to change the pointer. If
 * doing so, the newly set text should be malloc'ed, as once all the filters
 * are called Edje will free it.
 * If the text is to be rejected, freeing it and setting the pointer to NULL
 * will make Edje break out of the filter cycle and reject the inserted
 * text.
 *
 * @warning This function will be deprecated because of difficulty in use.
 *          The type(format, text, or markup) of text should be always
 *          checked in the filter function for correct filtering.
 *          Please use edje_object_text_markup_filter_callback_add() instead. There
 *          is no need to check the type of text in the filter function
 *          because the text is always markup.
 * @warning If you use this function with
 *          edje_object_text_markup_filter_callback_add() together, all
 *          Edje_Text_Filter_Cb functions and Edje_Markup_Filter_Cb functions
 *          will be executed, and then filtered text will be inserted.
 *
 * @see edje_object_text_insert_filter_callback_del
 * @see edje_object_text_insert_filter_callback_del_full
 * @see edje_object_text_markup_filter_callback_add
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The callback function that will act as filter
 * @param data User provided data to pass to the filter function
 */
EAPI void             edje_object_text_insert_filter_callback_add       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * Delete a function from the filter list.
 *
 * Delete the given @p func filter from the list in @p part. Returns
 * the user data pointer given when added.
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del_full
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 *
 * @return The user data pointer if successful, or NULL otherwise
 */
EAPI void            *edje_object_text_insert_filter_callback_del       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func);

/**
 * Delete a function and matching user data from the filter list.
 *
 * Delete the given @p func filter and @p data user data from the list
 * in @p part.
 * Returns the user data pointer given when added.
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 * @param data The data passed to the callback function
 *
 * @return The same data pointer if successful, or NULL otherwise
 */
EAPI void            *edje_object_text_insert_filter_callback_del_full  (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * Add a markup filter function for newly inserted text.
 *
 * Whenever text is inserted (not the same as set) into the given @p part,
 * the list of markup filter functions will be called to decide if and how
 * the new text will be accepted.
 * The text parameter in the @p func filter is always markup. It can be
 * modified by the user and it's up to him to free the one passed if he's to
 * change the pointer. If doing so, the newly set text should be malloc'ed,
 * as once all the filters are called Edje will free it.
 * If the text is to be rejected, freeing it and setting the pointer to NULL
 * will make Edje break out of the filter cycle and reject the inserted
 * text.
 * This function is different from edje_object_text_insert_filter_callback_add()
 * in that the text parameter in the @p fucn filter is always markup.
 *
 * @warning If you use this function with
 *          edje_object_text_insert_filter_callback_add() togehter, all
 *          Edje_Text_Filter_Cb functions and Edje_Markup_Filter_Cb functions
 *          will be executed, and then filtered text will be inserted.
 *
 * @see edje_object_text_markup_filter_callback_del
 * @see edje_object_text_markup_filter_callback_del_full
 * @see edje_object_text_insert_filter_callback_add
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The callback function that will act as markup filter
 * @param data User provided data to pass to the filter function
 * @since 1.2.0
 */
EAPI void edje_object_text_markup_filter_callback_add(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data);

/**
 * Delete a function from the markup filter list.
 *
 * Delete the given @p func filter from the list in @p part. Returns
 * the user data pointer given when added.
 *
 * @see edje_object_text_markup_filter_callback_add
 * @see edje_object_text_markup_filter_callback_del_full
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 *
 * @return The user data pointer if successful, or NULL otherwise
 * @since 1.2.0
 */
EAPI void *edje_object_text_markup_filter_callback_del(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func);

/**
 * Delete a function and matching user data from the markup filter list.
 *
 * Delete the given @p func filter and @p data user data from the list
 * in @p part.
 * Returns the user data pointer given when added.
 *
 * @see edje_object_text_markup_filter_callback_add
 * @see edje_object_text_markup_filter_callback_del
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 * @param data The data passed to the callback function
 *
 * @return The same data pointer if successful, or NULL otherwise
 * @since 1.2.0
 */
EAPI void *edje_object_text_markup_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data);

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Swallow
 *
 * @{
 */

/**
 * @brief Set the object minimum size.
 *
 * @param obj A valid Evas_Object handle
 * @param minw The minimum width
 * @param minh The minimum height
 *
 * This sets the minimum size restriction for the object.
 */
EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);

/**
 * @brief Set the object maximum size.
 *
 * @param obj A valid Evas_Object handle
 * @param maxw The maximum width
 * @param maxh The maximum height
 *
 * This sets the maximum size restriction for the object.
 */
EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);

/**
 * @brief Set the object aspect size.
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
 */
EAPI void         edje_extern_object_aspect_set   (Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);

/**
 * @brief "Swallows" an object into one of the Edje object @c SWALLOW
 * parts.
 *
 * @param obj A valid Edje object handle
 * @param part The swallow part's name
 * @param obj_swallow The object to occupy that part
 *
 * Swallowing an object into an Edje object is, for a given part of
 * type @c SWALLOW in the EDC group which gave life to @a obj, to set
 * an external object to be controlled by @a obj, being displayed
 * exactly over that part's region inside the whole Edje object's
 * viewport.
 *
 * From this point on, @a obj will have total control over @a
 * obj_swallow's geometry and visibility. For instance, if @a obj is
 * visible, as in @c evas_object_show(), the swallowed object will be
 * visible too -- if the given @c SWALLOW part it's in is also
 * visible. Other actions on @a obj will also reflect on the swallowed
 * object as well (e.g. resizing, moving, raising/lowering, etc.).
 *
 * Finally, all internal changes to @a part, specifically, will
 * reflect on the displaying of @a obj_swallow, for example state
 * changes leading to different visibility states, geometries,
 * positions, etc.
 *
 * If an object has already been swallowed into this part, then it
 * will first be unswallowed (as in edje_object_part_unswallow())
 * before the new object is swallowed.
 *
 * @note @a obj @b won't delete the swallowed object once it is
 * deleted -- @a obj_swallow will get to an unparented state again.
 *
 * For more details on EDC @c SWALLOW parts, see @ref edcref "syntax
 * reference".
 */
EAPI Eina_Bool        edje_object_part_swallow        (Evas_Object *obj, const char *part, Evas_Object *obj_swallow);

/**
 * @brief Unswallow an object.
 *
 * @param obj A valid Evas_Object handle
 * @param obj_swallow The swallowed object
 *
 * Causes the edje to regurgitate a previously swallowed object. :)
 *
 * @note @p obj_swallow will @b not be deleted or hidden.
 * @note @p obj_swallow may appear shown on the evas depending on its state when
 * it got unswallowed. Make sure you delete it or hide it if you do not want it to.
 */
EAPI void             edje_object_part_unswallow      (Evas_Object *obj, Evas_Object *obj_swallow);

/**
 * @brief Get the object currently swallowed by a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The swallowed object, or NULL if there is none.
 */
EAPI Evas_Object     *edje_object_part_swallow_get    (const Evas_Object *obj, const char *part);

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Drag
 *
 * @{
 */

/**
 * @brief Determine dragable directions.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * The dragable directions are defined in the EDC file, inside the @c dragable
 * section, by the attributes @c x and @c y. See the @ref edcref for more
 * information.
 *
 * @return #EDJE_DRAG_DIR_NONE: Not dragable\n
 * #EDJE_DRAG_DIR_X: Dragable in X direction\n
 * #EDJE_DRAG_DIR_Y: Dragable in Y direction\n
 * #EDJE_DRAG_DIR_XY: Dragable in X & Y directions
 */
EAPI Edje_Drag_Dir    edje_object_part_drag_dir_get   (const Evas_Object *obj, const char *part);

/**
 * @brief Set the dragable object location.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x value
 * @param dy The y value
 *
 * Places the dragable object at the given location.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1,
 * representing the relative position to the dragable area on that axis.
 *
 * This value means, for the vertical axis, that 0.0 will be at the top if the
 * first parameter of @c y in the dragable part theme is 1, and at bottom if it
 * is -1.
 *
 * For the horizontal axis, 0.0 means left if the first parameter of @c x in the
 * dragable part theme is 1, and right if it is -1.
 *
 * @see edje_object_part_drag_value_get()
 */
EAPI Eina_Bool        edje_object_part_drag_value_set (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Get the dragable object location.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The X value pointer
 * @param dy The Y value pointer
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1,
 * representing the relative position to the dragable area on that axis.
 *
 * @see edje_object_part_drag_value_set()
 *
 * Gets the drag location values.
 */
EAPI Eina_Bool        edje_object_part_drag_value_get (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Set the dragable object size.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width
 * @param dh The drag height
 *
 * Values for @p dw and @p dh are real numbers that range from 0 to 1,
 * representing the relative size of the dragable area on that axis.
 *
 * Sets the size of the dragable object.
 *
 * @see edje_object_part_drag_size_get()
 */
EAPI Eina_Bool        edje_object_part_drag_size_set  (Evas_Object *obj, const char *part, double dw, double dh);

/**
 * @brief Get the dragable object size.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width pointer
 * @param dh The drag height pointer
 *
 * Gets the dragable object size.
 *
 * @see edje_object_part_drag_size_set()
 */
EAPI Eina_Bool        edje_object_part_drag_size_get  (const Evas_Object *obj, const char *part, double *dw, double *dh);

/**
 * @brief Sets the drag step increment.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step amount
 * @param dy The y step amount
 *
 * Sets the x,y step increments for a dragable object.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1,
 * representing the relative size of the dragable area on that axis by which the
 * part will be moved.
 *
 * @see edje_object_part_drag_step_get()
 */
EAPI Eina_Bool        edje_object_part_drag_step_set  (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the drag step increment values.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part
 * @param dx The x step increment pointer
 * @param dy The y step increment pointer
 *
 * Gets the x and y step increments for the dragable object.
 *
 *
 * @see edje_object_part_drag_step_set()
 */
EAPI Eina_Bool        edje_object_part_drag_step_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Sets the page step increments.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x page step increment
 * @param dy The y page step increment
 *
 * Sets the x,y page step increment values.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1,
 * representing the relative size of the dragable area on that axis by which the
 * part will be moved.
 *
 * @see edje_object_part_drag_page_get()
 */
EAPI Eina_Bool        edje_object_part_drag_page_set  (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the page step increments.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The dx page increment pointer
 * @param dy The dy page increment pointer
 *
 * Gets the x,y page step increments for the dragable object.
 *
 * @see edje_object_part_drag_page_set()
 */
EAPI Eina_Bool        edje_object_part_drag_page_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Steps the dragable x,y steps.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Steps x,y where the step increment is the amount set by
 * edje_object_part_drag_step_set.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1.
 *
 * @see edje_object_part_drag_page()
 */
EAPI Eina_Bool        edje_object_part_drag_step      (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Pages x,y steps.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Pages x,y where the increment is defined by
 * edje_object_part_drag_page_set.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1.
 *
 * @warning Paging is bugged!
 *
 * @see edje_object_part_drag_step()
 */
EAPI Eina_Bool        edje_object_part_drag_page      (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_File
 *
 * @{
 */

/**
 * @brief Retrive an <b>EDC data field's value</b> from a given Edje
 * object's group.
 *
 * @param obj A handle to an Edje object
 * @param key The data field's key string
 * @return The data's value string. Must not be freed.
 *
 * This function fetches an EDC data field's value, which is declared
 * on the objects building EDC file, <b>under its group</b>. EDC data
 * blocks are most commonly used to pass arbitrary parameters from an
 * application's theme to its code.
 *
 * They look like the following:
 *
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     data {
 *	 item: "key1" "value1";
 *	 item: "key2" "value2";
 *     }
 *   }
 * }
 * @endcode
 *
 * EDC data fields always hold @b strings as values, hence the return
 * type of this function. Check the complete @ref edcref "syntax reference"
 * for EDC files.
 *
 * @warning Do not confuse this call with edje_file_data_get(), which
 * queries for a @b global EDC data field on an EDC declaration file.
 *
 * @see edje_object_file_set()
 */
EAPI const char  *edje_object_data_get            (const Evas_Object *obj, const char *key);

/**
 * @brief Sets the @b EDJ file (and group within it) to load an Edje
 * object's contents from
 *
 * @param obj A handle to an Edje object
 * @param file The path to the EDJ file to load @p from
 * @param group The name of the group, in @p file, which implements an
 * Edje object
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on errors (check
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
 */
EAPI Eina_Bool        edje_object_file_set        (Evas_Object *obj, const char *file, const char *group);

/**
 * @brief Sets the @b EDJ file (and group within it) to load an Edje
 * object's contents from
 *
 * @param obj A handle to an Edje object
 * @param file The Eina_File pointing to the EDJ file to load @p from
 * @param group The name of the group, in @p file, which implements an
 * Edje object
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on errors (check
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
 * @since 1.8
 */
EAPI Eina_Bool edje_object_mmap_set(Evas_Object *obj, const Eina_File *file, const char *group);


/**
 * @brief Get the file and group name that a given Edje object is bound to
 *
 * @param obj A handle to an Edje object
 * @param file A pointer to a variable whero to store the <b>file's
 * path</b>
 * @param group A pointer to a variable where to store the <b>group
 * name</b> in
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
 */
EAPI void             edje_object_file_get        (const Evas_Object *obj, const char **file, const char **group);

/**
 * @brief Gets the (last) file loading error for a given Edje object
 *
 * @param obj A handlet to an Edje object
 *
 * @return The Edje loading error, one of:
 * - #EDJE_LOAD_ERROR_NONE
 * - #EDJE_LOAD_ERROR_GENERIC
 * - #EDJE_LOAD_ERROR_DOES_NOT_EXIST
 * - #EDJE_LOAD_ERROR_PERMISSION_DENIED
 * - #EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED
 * - #EDJE_LOAD_ERROR_CORRUPT_FILE
 * - #EDJE_LOAD_ERROR_UNKNOWN_FORMAT
 * - #EDJE_LOAD_ERROR_INCOMPATIBLE_FILE
 * - #EDJE_LOAD_ERROR_UNKNOWN_COLLECTION
 * - #EDJE_LOAD_ERROR_RECURSIVE_REFERENCE
 *
 * This function is meant to be used after an Edje EDJ <b>file
 * loading</b>, what takes place with the edje_object_file_set()
 * function. If that function does not return @c EINA_TRUE, one should
 * check for the reason of failure with this one.
 *
 * @see edje_load_error_str()
 */
EAPI Edje_Load_Error  edje_object_load_error_get  (const Evas_Object *obj);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Animation
 *
 * @{
 */

/**
 * @brief Set the Edje object to playing or paused states.
 *
 * @param obj A handle to an Edje object.
 * @param play Object state (@c EINA_TRUE to playing,
 *                           @c EINA_FALSE to paused).
 *
 * This function sets the Edje object @a obj to playing or paused
 * states, depending on the parameter @a play. This has no effect if
 * the object was already at that state.
 *
 * @see edje_object_play_get().
 *
 */
EAPI void         edje_object_play_set            (Evas_Object *obj, Eina_Bool play);

/**
 * @brief Get the Edje object's state.
 *
 * @param obj A handle to an Edje object.
 * @return @c EINA_FALSE if the object is not connected, its @c delete_me flag
 * is set, or it is at paused state; @c EINA_TRUE if the object is at playing
 * state.
 *
 * This function tells if an Edje object is playing or not. This state
 * is set by edje_object_play_set().
 *
 * @see edje_object_play_set().
 *
 */
EAPI Eina_Bool    edje_object_play_get            (const Evas_Object *obj);

/**
 * @brief Set the object's animation state.
 *
 * @param obj A handle to an Edje object.
 * @param on The animation state. @c EINA_TRUE to starts or
 *           @c EINA_FALSE to stops.
 *
 * This function starts or stops an Edje object's animation. The
 * information if it's stopped can be retrieved by
 * edje_object_animation_get().
 *
 * @see edje_object_animation_get()
 *
 */
EAPI void         edje_object_animation_set       (Evas_Object *obj, Eina_Bool on);

/**
 * @brief Get the Edje object's animation state.
 *
 * @param obj A handle to an Edje object.
 * @return @c EINA_FALSE on error or if object is not animated;
 *         @c EINA_TRUE if animated.
 *
 * This function returns if the animation is stopped or not. The
 * animation state is set by edje_object_animation_set().
 *
 * @see edje_object_animation_set().
 *
 */
EAPI Eina_Bool    edje_object_animation_get       (const Evas_Object *obj);

/**
 * @brief Freezes the Edje object.
 *
 * @param obj A handle to an Edje object.
 * @return The frozen state or 0 on Error
 *
 * This function puts all changes on hold. Successive freezes will
 * nest, requiring an equal number of thaws.
 *
 * @see edje_object_thaw()
 */
EAPI int          edje_object_freeze                  (Evas_Object *obj);

/**
 * @brief Thaws the Edje object.
 *
 * @param obj A handle to an Edje object.
 * @return The frozen state or 0 if the object is not frozen or on error.
 *
 * This function thaws the given Edje object.
 *
 * @note: If sucessives freezes were done, an equal number of
 *        thaws will be required.
 *
 * @see edje_object_freeze()
 */
EAPI int          edje_object_thaw                    (Evas_Object *obj);

/**
 * @brief Returns the state of the Edje part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param val_ret
 *
 * @return The part state:\n
 * "default" for the default state\n
 * "" for other states
 */
EAPI const char      *edje_object_part_state_get      (const Evas_Object *obj, const char *part, double *val_ret);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Color_Class
 *
 * @{
 */

/**
 * @brief Sets the object color class.
 *
 * @param obj A valid Evas_Object handle
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * This function sets the color values for an object level color
 * class. This will cause all edje parts in the specified object that
 * have the specified color class to have their colors multiplied by
 * these values.
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * Setting color emits a signal "color_class,set" with source being
 * the given color.
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_object_color_class_set         (Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Gets the object color class.
 *
 * @param o A valid Evas_Object handle
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * @return EINA_TRUE if found or EINA_FALSE if not found and all
 *         values are zeroed.
 *
 * This function gets the color values for an object level color
 * class. If no explicit object color is set, then global values will
 * be used.
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_object_color_class_get         (const Evas_Object *o, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Delete the object color class.
 *
 * @param obj The edje object's reference.
 * @param color_class The color class to be deleted.
 *
 * This function deletes any values at the object level for the
 * specified object and color class.
 * @note Deleting the color class will revert it to the values
 *       defined by edje_color_class_set() or the color class
 *       defined in the theme file.
 *
 * Deleting the color class will emit the signal "color_class,del"
 * for the given Edje object.
 */
 EAPI void         edje_object_color_class_del         (Evas_Object *obj, const char *color_class);

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Box
 *
 * @{
 */

/**
 * @brief Appends an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to append
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Appends child to the box indicated by part.
 *
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_before()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_append             (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Prepends an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to prepend
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Prepends child to the box indicated by part.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_insert_before()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_prepend            (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Adds an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to insert
 * @param reference The object to be used as reference
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Inserts child in the box given by part, in the position marked by
 * reference.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_insert_before      (Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * @brief Inserts an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to insert
 * @param pos The position where to insert child
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Adds child to the box indicated by part, in the position given by
 * pos.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_before()
 */
EAPI Eina_Bool    edje_object_part_box_insert_at          (Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);

/**
 * @brief Removes an object from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to remove
 *
 * @return Pointer to the object removed, or @c NULL.
 *
 * Removes child from the box indicated by part.
 *
 * @see edje_object_part_box_remove_at()
 * @see edje_object_part_box_remove_all()
 */
EAPI Evas_Object *edje_object_part_box_remove             (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Removes an object from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param pos The position index of the object (starts counting from 0)
 *
 * @return Pointer to the object removed, or @c NULL.
 *
 * Removes from the box indicated by part, the object in the position
 * pos.
 *
 * @see edje_object_part_box_remove()
 * @see edje_object_part_box_remove_all()
 */
EAPI Evas_Object *edje_object_part_box_remove_at          (Evas_Object *obj, const char *part, unsigned int pos);

/**
 * @brief Removes all elements from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param clear Delete objects on removal
 *
 * @return 1: Successfully cleared.\n
 * 0: An error occurred.
 *
 * Removes all the external objects from the box indicated by part.
 * Elements created from the theme will not be removed.
 *
 * @see edje_object_part_box_remove()
 * @see edje_object_part_box_remove_at()
 */
EAPI Eina_Bool    edje_object_part_box_remove_all         (Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Table
 *
 * @{
 */

/**
 * @brief Retrieve a child from a table
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param col The column of the child to get
 * @param row The row of the child to get
 * @return The child Evas_Object
 */
EAPI Evas_Object *edje_object_part_table_child_get        (const Evas_Object *obj, const char *part, unsigned int col, unsigned int row);

/**
 * @brief Packs an object into the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child_obj The object to pack in
 * @param col The column to place it in
 * @param row The row to place it in
 * @param colspan Columns the child will take
 * @param rowspan Rows the child will take
 *
 * @return @c EINA_TRUE object was added, @c EINA_FALSE on failure
 *
 * Packs an object into the table indicated by part.
 */
EAPI Eina_Bool    edje_object_part_table_pack             (Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);

/**
 * @brief Removes an object from the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child_obj The object to pack in
 *
 * @return @c EINA_TRUE object removed, @c EINA_FALSE on failure
 *
 * Removes an object from the table indicated by part.
 */
EAPI Eina_Bool    edje_object_part_table_unpack           (Evas_Object *obj, const char *part, Evas_Object *child_obj);

/**
 * @brief Gets the number of columns and rows the table has.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cols Pointer where to store number of columns (can be NULL)
 * @param rows Pointer where to store number of rows (can be NULL)
 *
 * @return @c EINA_TRUE get some data, @c EINA_FALSE on failure
 *
 * Retrieves the size of the table in number of columns and rows.
 */
EAPI Eina_Bool    edje_object_part_table_col_row_size_get (const Evas_Object *obj, const char *part, int *cols, int *rows);

/**
 * @brief Removes all object from the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param clear If set, will delete subobjs on remove
 *
 * @return @c EINA_TRUE clear the table, @c EINA_FALSE on failure
 *
 * Removes all object from the table indicated by part, except the
 * internal ones set from the theme.
 */
EAPI Eina_Bool    edje_object_part_table_clear            (Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Text_Class
 *
 * @{
 */

/**
 * @brief Sets Edje text class.
 *
 * @param obj A valid Evas_Object handle
 * @param text_class The text class name
 * @param font Font name
 * @param size Font Size
 *
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on error
 *
 * This function sets the text class for the Edje.
 *
 */
EAPI Eina_Bool    edje_object_text_class_set          (Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size);

/**
 * @}
 */

/**
 * @ingroup Edje_Perspective
 *
 * @{
 */

/**
 * Set the given perspective object on this Edje object.
 *
 * @param obj The Edje object on the perspective will be set.
 * @param ps The perspective object that will be used.
 *
 * Make the given perspective object be the default perspective for this Edje
 * object.
 *
 * There can be only one perspective object per Edje object, and if a
 * previous one was set, it will be removed and the new perspective object
 * will be used.
 *
 * An Edje perspective will only affect a part if it doesn't point to another
 * part to be used as perspective.
 *
 * @see edje_object_perspective_new()
 * @see edje_object_perspective_get()
 * @see edje_perspective_set()
 */
EAPI void                    edje_object_perspective_set     (Evas_Object *obj, Edje_Perspective *ps);

/**
 * Get the current perspective used on this Edje object.
 *
 * @param obj the given Edje object.
 * @return The perspective object being used on this Edje object. Or @c NULL
 * if there was none, and on errors.
 *
 * @see edje_object_perspective_set()
 */
EAPI const Edje_Perspective *edje_object_perspective_get     (const Evas_Object *obj);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Group
 *
 * @{
 */

/**
 * @brief Instantiate a new Edje object
 *
 * @param evas A valid Evas handle, the canvas to place the new object
 * in
 * @return A handle to the new object created or @c NULL, on errors.
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
 * it outside the callback. to register a callback use code like:
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
 * @brief Preload the images on the Edje Object in the background.
 *
 * @param obj A handle to an Edje object
 * @param cancel @c EINA_FALSE will add it the preloading work queue,
 *               @c EINA_TRUE will remove it (if it was issued before).
 * @return @c EINA_FASLE if obj was not a valid Edje object
 *         otherwise @c EINA_TRUE
 *
 * This function requests the preload of all data images (on the given
 * object) in the background. The work is queued before being processed
 * (because there might be other pending requests of this type).
 * It emits a signal "preload,done" when finished.
 *
 * @note Use @c EINA_TRUE on scenarios where you don't need
 *       the image data preloaded anymore.
 */
EAPI Eina_Bool        edje_object_preload         (Evas_Object *obj, Eina_Bool cancel);

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Communication_Interface_Signal
 *
 * @{
 */

/**
 * @brief Add a callback for an arriving Edje signal, emitted by
 * a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @param data A pointer to data to pass in to @p func.
 *
 * Edje signals are one of the communication interfaces between
 * @b code and a given Edje object's @b theme. With signals, one can
 * communicate two string values at a time, which are:
 * - "emission" value: the name of the signal, in general
 * - "source" value: a name for the signal's context, in general
 *
 * Though there are those common uses for the two strings, one is free
 * to use them however they like.
 *
 * This function adds a callback function to a signal emitted by @a obj, to
 * be issued every time an EDC program like the following
 * @code
 * program {
 *   name: "emit_example";
 *   action: SIGNAL_EMIT "a_signal" "a_source";
 * }
 * @endcode
 * is run, if @p emission and @p source are given those same values,
 * here.
 *
 * Signal callback registration is powerful, in the way that @b blobs
 * may be used to match <b>multiple signals at once</b>. All the @c
 * "*?[\" set of @c fnmatch() operators can be used, both for @p
 * emission and @p source.
 *
 * Edje has @b internal signals it will emit, automatically, on
 * various actions taking place on group parts. For example, the mouse
 * cursor being moved, pressed, released, etc., over a given part's
 * area, all generate individual signals.
 *
 * By using something like
 * @code
 * edje_object_signal_callback_add(obj, "mouse,down,*", "button.*",
 *                                 signal_cb, NULL);
 * @endcode
 * being @c "button.*" the pattern for the names of parts implementing
 * buttons on an interface, you'd be registering for notifications on
 * events of mouse buttons being pressed down on either of those parts
 * (those events all have the @c "mouse,down," common prefix on their
 * names, with a suffix giving the button number). The actual emission
 * and source strings of an event will be passed in as the @a emission
 * and @a source parameters of the callback function (e.g. @c
 * "mouse,down,2" and @c "button.close"), for each of those events.
 *
 * @note See @ref edcref "the syntax" for EDC files
 * @see edje_object_signal_emit() on how to emits Edje signals from
 * code to a an object
 * @see edje_object_signal_callback_del_full()
 */
EAPI void         edje_object_signal_callback_add (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Remove a signal-triggered callback from an object.
 *
 * @param obj A valid Evas_Object handle.
 * @param emission The emission string.
 * @param source The source string.
 * @param func The callback function.
 * @return The data pointer
 *
 * This function removes a callback, previously attached to the
 * emittion of a signal, from the object @a obj. The parameters @a
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
 * @brief Unregister/delete a callback set for an arriving Edje
 * signal, emitted by a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 * @param func The callback function passed on the callback's
 * registration
 * @param data The pointer given to be passed as data to @p func
 * @return @p data, on success or @c NULL, on errors (or if @p data
 * had this value)
 *
 * This function removes a callback, previously attached to the
 * emittion of a signal, from the object @a obj. The parameters
 * @a emission, @a source, @a func and @a data must match exactly those
 * passed to a previous call to edje_object_signal_callback_add(). The
 * data pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del().
 *
 */
EAPI void        *edje_object_signal_callback_del_full(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Send/emit an Edje signal to a given Edje object
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 *
 * This function sends a signal to the object @a obj. An Edje program,
 * at @p obj's EDC specification level, can respond to a signal by
 * having declared matching @c 'signal' and @c 'source' fields on its
 * block (see @ref edcref "the syntax" for EDC files).
 *
 * As an example,
 * @code
 * edje_object_signal_emit(obj, "a_signal", "");
 * @endcode
 * would trigger a program which had an EDC declaration block like
 * @code
 * program {
 *  name: "a_program";
 *  signal: "a_signal";
 *  source: "";
 *  action: ...
 * }
 * @endcode
 *
 * @see edje_object_signal_callback_add() for more on Edje signals.
 */
EAPI void         edje_object_signal_emit         (Evas_Object *obj, const char *emission, const char *source);
/**
 * @}
 */

/**
 * @ingroup Edje_Object_Communication_Interface_Message
 *
 * @{
 */

/**
 * @brief Send an (Edje) message to a given Edje object
 *
 * @param obj A handle to an Edje object
 * @param type The type of message to send to @p obj
 * @param id A identification number for the message to be sent
 * @param msg The message's body, a struct depending on @p type
 *
 * This function sends an Edje message to @p obj and to all of its
 * child objects, if it has any (swallowed objects are one kind of
 * child object). @p type and @p msg @b must be matched accordingly,
 * as documented in #Edje_Message_Type.
 *
 * The @p id argument as a form of code and theme defining a common
 * interface on message communication. One should define the same IDs
 * on both code and EDC declaration (see @ref edcref "the syntax" for
 * EDC files), to individualize messages (binding them to a given
 * context).
 *
 * The function to handle messages arriving @b from @b obj is set with
 * edje_object_message_handler_set().
 */
EAPI void         edje_object_message_send                (Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

/**
 * @brief Set an Edje message handler function for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param func The function to handle messages @b coming from @p obj
 * @param data Auxiliary data to be passed to @p func
 *
 * For scriptable programs on an Edje object's defining EDC file which
 * send messages with the @c send_message() primitive, one can attach
 * <b>handler functions</b>, to be called in the code which creates
 * that object (see @ref edcref "the syntax" for EDC files).
 *
 * This function associates a message handler function and the
 * attached data pointer to the object @p obj.
 *
 * @see edje_object_message_send()
 */
EAPI void         edje_object_message_handler_set         (Evas_Object *obj, Edje_Message_Handler_Cb func, void *data);

/**
 * @brief Process an object's message queue.
 *
 * @param obj A handle to an Edje object.
 *
 * This function goes through the object message queue processing the
 * pending messages for @b this specific Edje object. Normally they'd
 * be processed only at idle time.
 *
 */
EAPI void         edje_object_message_signal_process      (Evas_Object *obj);

/**
 * @}
 */
