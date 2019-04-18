#ifndef _EVAS_BOX_EO_H_
#define _EVAS_BOX_EO_H_

#ifndef _EVAS_BOX_EO_CLASS_TYPE
#define _EVAS_BOX_EO_CLASS_TYPE

typedef Eo Evas_Box;

#endif

#ifndef _EVAS_BOX_EO_TYPES
#define _EVAS_BOX_EO_TYPES


#endif
/** Evas box class
 *
 * @ingroup Evas_Box
 */
#define EVAS_BOX_CLASS evas_box_class_get()

EWAPI const Efl_Class *evas_box_class_get(void);

/**
 * @brief Set the alignment of the whole bounding box of contents, for a given
 * box object.
 *
 * This will influence how a box object is to align its bounding box of
 * contents within its own area. The values must be in the range $0.0 - $1.0,
 * or undefined behavior is expected. For horizontal alignment, $0.0 means to
 * the left, with $1.0 meaning to the right. For vertical alignment, $0.0 means
 * to the top, with $1.0 meaning to the bottom.
 *
 * @note The default values for both alignments is $0.5.
 *
 * See also @ref evas_obj_box_align_get.
 *
 * @param[in] obj The object.
 * @param[in] horizontal The horizontal alignment, in pixels.
 * @param[in] vertical The vertical alignment, in pixels.
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_align_set(Eo *obj, double horizontal, double vertical);

/**
 * @brief Get the alignment of the whole bounding box of contents, for a given
 * box object.
 *
 * See also @ref evas_obj_box_align_set for more information.
 *
 * @param[in] obj The object.
 * @param[out] horizontal The horizontal alignment, in pixels.
 * @param[out] vertical The vertical alignment, in pixels.
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_align_get(const Eo *obj, double *horizontal, double *vertical);

/**
 * @brief Set the (space) padding between cells set for a given box object.
 *
 * @note The default values for both padding components is $0.
 *
 * See also @ref evas_obj_box_padding_get.
 *
 * @param[in] obj The object.
 * @param[in] horizontal The horizontal padding, in pixels.
 * @param[in] vertical The vertical padding, in pixels.
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_padding_set(Eo *obj, int horizontal, int vertical);

/**
 * @brief Get the (space) padding between cells set for a given box object.
 *
 * See also @ref evas_obj_box_padding_set.
 *
 * @param[in] obj The object.
 * @param[out] horizontal The horizontal padding, in pixels.
 * @param[out] vertical The vertical padding, in pixels.
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_padding_get(const Eo *obj, int *horizontal, int *vertical);

/**
 * @brief Set a new layouting function to a given box object
 *
 * A box layout function affects how a box object displays child elements
 * within its area. The list of pre-defined box layouts available in Evas are
 * @ref evas_obj_box_layout_horizontal, @ref evas_obj_box_layout_vertical,
 * @ref evas_obj_box_layout_homogeneous_horizontal,
 * @ref evas_obj_box_layout_homogeneous_vertical,
 * @ref evas_obj_box_layout_homogeneous_max_size_horizontal,
 * @ref evas_obj_box_layout_homogeneous_max_size_vertical,
 * @ref evas_obj_box_layout_flow_horizontal,
 * @ref evas_obj_box_layout_flow_vertical and @ref evas_obj_box_layout_stack
 *
 * Refer to each of their documentation texts for details on them.
 *
 * @note A box layouting function will be triggered by the $'calculate' smart
 * callback of the box's smart class.
 *
 * @param[in] obj The object.
 * @param[in] cb The new layout function to set on @c o.
 * @param[in] data Data pointer to be passed to @c cb.
 * @param[in] free_data Function to free @c data, if need be.
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_set(Eo *obj, Evas_Object_Box_Layout cb, const void *data, Eina_Free_Cb free_data) EINA_ARG_NONNULL(2);

/**
 * @brief Layout function which sets the box o to a (basic) horizontal box
 *
 * In this layout, the box object's overall behavior is controlled by its
 * padding/alignment properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set family of functions. The size
 * hints of the elements in the box -- set by the
 * evas_object_size_hint_{align,padding,weight}_set functions -- also control
 * the way this function works.
 *
 * Box's properties: @c align_h controls the horizontal alignment of the child
 * objects relative to the containing box. When set to $0.0, children are
 * aligned to the left. A value of $1.0 makes them aligned to the right border.
 * Values in between align them proportionally. Note that if the size required
 * by the children, which is given by their widths and the @c padding_h
 * property of the box, is bigger than the their container's width, the
 * children will be displayed out of the box's bounds. A negative value of
 * @c align_h makes the box to justify its children. The padding between them,
 * in this case, is corrected so that the leftmost one touches the left border
 * and the rightmost one touches the right border (even if they must overlap).
 * The @c align_v and @c padding_v properties of the box don't contribute to
 * its behaviour when this layout is chosen.
 *
 * Child element's properties: @c align_x does not influence the box's
 * behavior. @c padding_l and @c padding_r sum up to the container's horizontal
 * padding between elements. The child's @c padding_t, @c padding_b and
 * @c align_y properties apply for padding/alignment relative to the overall
 * height of the box. Finally, there is the @c weight_x property, which, if set
 * to a non-zero value, tells the container that the child width is not
 * pre-defined. If the container can't accommodate all its children, it sets
 * the widths of the ones with weights to sizes as small as they can all fit
 * into it. If the size required by the children is less than the available,
 * the box increases its childrens' (which have weights) widths as to fit the
 * remaining space. The @c weight_x property, besides telling the element is
 * resizable, gives a weight for the resizing process.  The parent box will try
 * to distribute (or take off) widths accordingly to the normalized list of
 * weights: most weighted children remain/get larger in this process than the
 * least ones. @c weight_y does not influence the layout.
 *
 * If one desires that, besides having weights, child elements must be resized
 * bounded to a minimum or maximum size, those size hints must be set, by the
 * evas_object_size_hint_{min,max}_set functions.
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_horizontal(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief Layout function which sets the box o to a (basic) vertical box
 *
 * This function behaves analogously to evas_object_box_layout_horizontal. The
 * description of its behaviour can be derived from that function's
 * documentation.
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_vertical(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief Layout function which sets the box o to a maximum size, homogeneous
 * horizontal box
 *
 * In a maximum size, homogeneous horizontal box, besides having cells of equal
 * size reserved for the child objects, this size will be defined by the size
 * of the largest child in the box (in width). The box's overall behavior is
 * controlled by its properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set family of functions.  The size
 * hints of the elements in the box -- set by the
 * evas_object_size_hint_{align,padding,weight}_set functions -- also control
 * the way this function works.
 *
 * Box's properties: @c padding_h tells the box to draw empty spaces of that
 * size, in pixels, between the child objects' cells. @c align_h controls the
 * horizontal alignment of the child objects, relative to the containing box.
 * When set to $0.0, children are aligned to the left. A value of $1.0 lets
 * them aligned to the right border. Values in between align them
 * proportionally. A negative value of @c align_h makes the box to justify its
 * children cells. The padding between them, in this case, is corrected so that
 * the leftmost one touches the left border and the rightmost one touches the
 * right border (even if they must overlap). The @c align_v and @c padding_v
 * properties of the box don't contribute to its behaviour when this layout is
 * chosen.
 *
 * Child element's properties: @c padding_l and @c padding_r sum up to the
 * required width of the child element. The @c align_x property tells the
 * relative position of this overall child width in its allocated cell ($0.0 to
 * extreme left, $1.0 to extreme right). A value of $-1.0 to @c align_x makes
 * the box try to resize this child element to the exact width of its cell
 * (respecting the minimum and maximum size hints on the child's width and
 * accounting for its horizontal padding hints). The child's @c padding_t,
 * @c padding_b and @c align_y properties apply for padding/alignment relative
 * to the overall height of the box. A value of $-1.0 to @c align_y makes the
 * box try to resize this child element to the exact height of its parent
 * (respecting the max hint on the child's height).
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_homogeneous_max_size_horizontal(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] child Child object to be removed
 *
 * @return New object with child removed
 *
 * @ingroup Evas_Box
 */
EOAPI Efl_Canvas_Object *evas_obj_box_internal_remove(Eo *obj, Efl_Canvas_Object *child);

/**
 * @brief Layout function which sets the box o to a flow vertical box.
 *
 * This function behaves analogously to evas_object_box_layout_flow_horizontal.
 * The description of its behaviour can be derived from that function's
 * documentation.
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_flow_vertical(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] opt Box option to be freed
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_internal_option_free(Eo *obj, Evas_Object_Box_Option *opt);

/**
 * @brief Insert a new child object after another existing one, in a given box
 * object o.
 *
 * On success, the $"child,added" smart event will take place.
 *
 * @note This function will fail if @c reference is not a member of @c o.
 *
 * @note The actual placing of the item relative to @c o's area will depend on
 * the layout set to it.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api.insert_after
 * smart function.
 *
 * @param[in] obj The object.
 * @param[in] child A child Evas object to be made a member of @c o.
 * @param[in] reference The child object to place this new one after.
 *
 * @return A box option bound to the recently added box item or @c null, on
 * errors
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_insert_after(Eo *obj, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference) EINA_ARG_NONNULL(2, 3);

/**
 * @brief Remove all child objects from a box object, unparenting them again.
 *
 * This has the same effect of calling evas_object_box_remove on each of @c o's
 * child objects, in sequence. If, and only if, all those calls succeed, so
 * does this one.
 *
 * @param[in] obj The object.
 * @param[in] clear If @c true, it will delete just removed children.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Evas_Box
 */
EOAPI Eina_Bool evas_obj_box_remove_all(Eo *obj, Eina_Bool clear);

/**
 * @brief Get an iterator to walk the list of children of a given box object.
 *
 * @note Do not remove or delete objects while walking the list.
 *
 * @param[in] obj The object.
 *
 * @return An iterator on @c o's child objects, on success, or @c null, on
 * errors.
 *
 * @ingroup Evas_Box
 */
EOAPI Eina_Iterator *evas_obj_box_iterator_new(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Add a new box as a child of a given smart object.
 *
 * This is a helper function that has the same effect of putting a new box
 * object into @c parent by use of evas_object_smart_member_add.
 *
 * @param[in] obj The object.
 *
 * @return @c null on error, a pointer to a new box object on success.
 *
 * @ingroup Evas_Box
 */
EOAPI Efl_Canvas_Object *evas_obj_box_add_to(Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Append a new child object to the given box object o.
 *
 * On success, the $"child,added" smart event will take place.
 *
 * @note The actual placing of the item relative to @c o's area will depend on
 * the layout set to it. For example, on horizontal layouts an item in the end
 * of the box's list of children will appear on its right.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api.append smart
 * function.
 *
 * @param[in] obj The object.
 * @param[in] child A child Evas object to be made a member of @c o.
 *
 * @return A box option bound to the recently added box item or @c null, on
 * errors.
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_append(Eo *obj, Efl_Canvas_Object *child) EINA_ARG_NONNULL(2);

/**
 * @brief Get the numerical identifier of the property of the child elements of
 * the box o which have name as name string
 *
 * @note This call won't do anything for a canonical Evas box. Only users which
 * have subclassed it, setting custom box items options (see
 * #Evas_Object_Box_Option) on it, would benefit from this function. They'd
 * have to implement it and set it to be the
 * _Evas_Object_Box_Api.property_id_get smart class function of the box, which
 * is originally set to @c null.
 *
 * @param[in] obj The object.
 * @param[in] name The name string of the option being searched, for its ID.
 *
 * @return The numerical ID of the given property or $-1, on errors.
 *
 * @ingroup Evas_Box
 */
EOAPI int evas_obj_box_option_property_id_get(const Eo *obj, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(2);

/**
 * @brief Prepend a new child object to the given box object o.
 *
 * On success, the $"child,added" smart event will take place.
 *
 * @note The actual placing of the item relative to @c o's area will depend on
 * the layout set to it. For example, on horizontal layouts an item in the
 * beginning of the box's list of children will appear on its left.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api.prepend smart
 * function.
 *
 * @param[in] obj The object.
 * @param[in] child A child Evas object to be made a member of @c o.
 *
 * @return A box option bound to the recently added box item or @c null, on
 * errors.
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_prepend(Eo *obj, Efl_Canvas_Object *child) EINA_ARG_NONNULL(2);

/**
 * @brief Get an accessor (a structure providing random items access) to the
 * list of children of a given box object.
 *
 * @note Do not remove or delete objects while walking the list.
 *
 * @param[in] obj The object.
 *
 * @return An accessor on @c o's child objects, on success, or @c null, on
 * errors.
 *
 * @ingroup Evas_Box
 */
EOAPI Eina_Accessor *evas_obj_box_accessor_new(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] child Child object to be appended
 *
 * @return Box option
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_internal_append(Eo *obj, Efl_Canvas_Object *child);

/**
 * @brief Set a property value (by its given numerical identifier), on a given
 * box child element -- by a variable argument list
 *
 * This is a variable argument list variant of the
 * evas_object_box_option_property_set. See its documentation for more details.
 *
 * @param[in] obj The object.
 * @param[in] opt The box option structure bound to the child box element to
 * set a property on.
 * @param[in] property The numerical ID of the given property.
 * @param[in] args The variable argument list implementing the value to be set
 * for this property. It must be of the same type the user has defined for it.
 *
 * @return @c true on success, @c false on failure.
 *
 * @ingroup Evas_Box
 */
EOAPI Eina_Bool evas_obj_box_option_property_vset(Eo *obj, Evas_Object_Box_Option *opt, int property, va_list *args) EINA_ARG_NONNULL(2);

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] pos Position of object to be removed
 *
 * @return Canvas object
 *
 * @ingroup Evas_Box
 */
EOAPI Efl_Canvas_Object *evas_obj_box_internal_remove_at(Eo *obj, unsigned int pos);

/**
 * @brief Remove an object, bound to a given position in a box object,
 * unparenting it again.
 *
 * On removal, you'll get an unparented object again, just as it was before you
 * inserted it in the box. The @c option_free box smart callback will be called
 * automatically for you and, also, the $"child,removed" smart event will take
 * place.
 *
 * @note This function will fail if the given position is invalid, given @c o's
 * internal list of elements.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api.remove_at smart
 * function.
 *
 * @param[in] obj The object.
 * @param[in] pos The numeric position (starting from $0) of the child object
 * to be removed.
 *
 * @return @c true on success, @c false on failure.
 *
 * @ingroup Evas_Box
 */
EOAPI Eina_Bool evas_obj_box_remove_at(Eo *obj, unsigned int pos);

/**
 * @brief Get a property's value (by its given numerical identifier), on a
 * given box child element -- by a variable argument list
 *
 * This is a variable argument list variant of the
 * evas_object_box_option_property_get. See its documentation for more details.
 *
 * @param[in] obj The object.
 * @param[in] opt The box option structure bound to the child box element to
 * get a property from.
 * @param[in] property The numerical ID of the given property.
 * @param[in] args The variable argument list with pointers to where to store
 * the values of this property. They must point to variables of the same type
 * the user has defined for them.
 *
 * @return @c true on success, @c false on failure.
 *
 * @ingroup Evas_Box
 */
EOAPI Eina_Bool evas_obj_box_option_property_vget(const Eo *obj, Evas_Object_Box_Option *opt, int property, va_list *args) EINA_ARG_NONNULL(2);

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] child Child object to be inserted
 * @param[in] pos Position where the object will be inserted
 *
 * @return Box option
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_internal_insert_at(Eo *obj, Efl_Canvas_Object *child, unsigned int pos);

/**
 * @brief Insert a new child object before another existing one, in a given box
 * object o.
 *
 * On success, the $"child,added" smart event will take place.
 *
 * @note This function will fail if @c reference is not a member of @c o.
 *
 * @note The actual placing of the item relative to @c o's area will depend on
 * the layout set to it.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api.insert_before
 * smart function.
 *
 * @param[in] obj The object.
 * @param[in] child A child Evas object to be made a member of @c o.
 * @param[in] reference The child object to place this new one before.
 *
 * @return A box option bound to the recently added box item or @c null, on
 * errors.
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_insert_before(Eo *obj, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference) EINA_ARG_NONNULL(2, 3);

/**
 * @brief Get the name of the property of the child elements of the box o which
 * have id as identifier
 *
 * @note This call won't do anything for a canonical Evas box. Only users which
 * have subclassed it, setting custom box items options (see
 * #Evas_Object_Box_Option) on it, would benefit from this function. They'd
 * have to implement it and set it to be the
 * _Evas_Object_Box_Api.property_name_get smart class function of the box,
 * which is originally set to @c null.
 *
 * @param[in] obj The object.
 * @param[in] property The numerical identifier of the option being searched,
 * for its name.
 *
 * @return The name of the given property or @c null, on errors.
 *
 * @ingroup Evas_Box
 */
EOAPI const char *evas_obj_box_option_property_name_get(const Eo *obj, int property) EINA_WARN_UNUSED_RESULT;

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] child Object to be inserted
 * @param[in] reference Reference where the object will be inserted
 *
 * @return Box option
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_internal_insert_before(Eo *obj, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

/**
 * @brief Layout function which sets the box o to a homogeneous horizontal box
 *
 * In a homogeneous horizontal box, its width is divided equally between the
 * contained objects. The box's overall behavior is controlled by its
 * padding/alignment properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set family of functions.  The size
 * hints the elements in the box -- set by the
 * evas_object_size_hint_{align,padding,weight}_set functions -- also control
 * the way this function works.
 *
 * Box's properties: @c align_h has no influence on the box for this layout.
 * @c padding_h tells the box to draw empty spaces of that size, in pixels,
 * between the (equal) child objects' cells. The @c align_v and @c padding_v
 * properties of the box don't contribute to its behaviour when this layout is
 * chosen.
 *
 * Child element's properties: @c padding_l and @c padding_r sum up to the
 * required width of the child element. The @c align_x property tells the
 * relative position of this overall child width in its allocated cell ($0.0 to
 * extreme left, $1.0 to extreme right). A value of $-1.0 to @c align_x makes
 * the box try to resize this child element to the exact width of its cell
 * (respecting the minimum and maximum size hints on the child's width and
 * accounting for its horizontal padding hints). The child's @c padding_t,
 * @c padding_b and @c align_y properties apply for padding/alignment relative
 * to the overall height of the box. A value of $-1.0 to @c align_y makes the
 * box try to resize this child element to the exact height of its parent
 * (respecting the maximum size hint on the child's height).
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_homogeneous_horizontal(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] child New box object
 *
 * @return Box option
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_internal_option_new(Eo *obj, Efl_Canvas_Object *child);

/**
 * @brief Layout function which sets the box o to a maximum size, homogeneous
 * vertical box
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_max_size_horizontal. The description of
 * its behaviour can be derived from that function's documentation.
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_homogeneous_max_size_vertical(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] child Object to be inserted
 * @param[in] reference Reference where the object will be inserted
 *
 * @return Box option
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_internal_insert_after(Eo *obj, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

/**
 * @brief Insert a new child object at a given position, in a given box object
 * @c o.
 *
 * On success, the $"child,added" smart event will take place.
 *
 * @note This function will fail if the given position is invalid, given @c o's
 * internal list of elements.
 *
 * @note The actual placing of the item relative to @c o's area will depend on
 * the layout set to it.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api.insert_at smart
 * function.
 *
 * @param[in] obj The object.
 * @param[in] child A child Evas object to be made a member of @c o.
 * @param[in] pos The numeric position (starting from $0) to place the new
 * child object at.
 *
 * @return A box option bound to the recently added box item or @c null, on
 * errors.
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_insert_at(Eo *obj, Efl_Canvas_Object *child, unsigned int pos) EINA_ARG_NONNULL(2);

/**
 * @brief No description supplied by the EAPI.
 *
 * @param[in] obj The object.
 * @param[in] child Object to be prepended
 *
 * @return Box option
 *
 * @ingroup Evas_Box
 */
EOAPI Evas_Object_Box_Option *evas_obj_box_internal_prepend(Eo *obj, Efl_Canvas_Object *child);

/**
 * @brief Remove a given object from a box object, unparenting it again.
 *
 * On removal, you'll get an unparented object again, just as it was before you
 * inserted it in the box. The _Evas_Object_Box_Api.option_free box smart
 * callback will be called automatically for you and, also, the
 * $"child,removed" smart event will take place.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api.remove smart
 * function.
 *
 * @param[in] obj The object.
 * @param[in] child The handle to the child object to be removed.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Evas_Box
 */
EOAPI Eina_Bool evas_obj_box_remove(Eo *obj, Efl_Canvas_Object *child) EINA_ARG_NONNULL(2);

/**
 * @brief Layout function which sets the box o to a stacking box
 *
 * In a stacking box, all children will be given the same size -- the box's own
 * size -- and they will be stacked one above the other, so that the first
 * object in @c o's internal list of child elements will be the bottommost in
 * the stack.
 *
 * Box's properties: No box properties are used.
 *
 * Child element's properties: @c padding_l and @c padding_r sum up to the
 * required width of the child element. The @c align_x property tells the
 * relative position of this overall child width in its allocated cell ($0.0 to
 * extreme left, $1.0 to extreme right). A value of $-1.0 to @c align_x makes
 * the box try to resize this child element to the exact width of its cell
 * (respecting the min and max hints on the child's width and accounting for
 * its horizontal padding properties). The same applies to the vertical axis.
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_stack(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief Layout function which sets the box o to a homogeneous vertical box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_horizontal.  The description of its
 * behaviour can be derived from that function's documentation.
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_homogeneous_vertical(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief Layout function which sets the box o to a flow horizontal box.
 *
 * In a flow horizontal box, the box's child elements are placed in rows (think
 * of text as an analogy). A row has as many elements as can fit into the box's
 * width. The box's overall behavior is controlled by its properties, which are
 * set by the evas_object_box_{h,v}_{align,padding}_set family of functions.
 *  The size hints of the elements in the box -- set by the
 * evas_object_size_hint_{align,padding,weight}_set functions -- also control
 * the way this function works.
 *
 * Box's properties: @c padding_h tells the box to draw empty spaces of that
 * size, in pixels, between the child objects' cells. @c align_h dictates the
 * horizontal alignment of the rows ($0.0 to left align them, $1.0 to right
 * align). A value of $-1.0 to @c align_h lets the rows justified horizontally.
 * @c align_v controls the vertical alignment of the entire set of rows ($0.0
 * to top, $1.0 to bottom). A value of $-1.0 to @c align_v makes the box to
 * justify the rows vertically. The padding between them in this case is
 * corrected so that the first row touches the top border and the last one
 * touches the bottom border (even if they must overlap). @c padding_v has no
 * influence on the layout.
 *
 * Child element's properties: @c padding_l and @c padding_r sum up to the
 * required width of the child element. The @c align_x property has no
 * influence on the layout. The child's @c padding_t and @c padding_b sum up to
 * the required height of the child element and is the only means (besides row
 * justifying) of setting space between rows. Note, however, that @c align_y
 * dictates positioning relative to the largest height required by a child
 * object in the actual row.
 *
 * @param[in] obj The object.
 * @param[in] priv Private data pointer
 * @param[in] data Data pointer
 *
 * @ingroup Evas_Box
 */
EOAPI void evas_obj_box_layout_flow_horizontal(Eo *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * @brief Returns the number of items in the box.
 *
 * @param[in] obj The object.
 *
 * @return Number of items in the box
 *
 * @ingroup Evas_Box
 */
EOAPI int evas_obj_box_count(Eo *obj);

EWAPI extern const Efl_Event_Description _EVAS_BOX_EVENT_CHILD_ADDED;

/** Called when a child object was added to the box
 * @return Evas_Object_Box_Option *
 *
 * @ingroup Evas_Box
 */
#define EVAS_BOX_EVENT_CHILD_ADDED (&(_EVAS_BOX_EVENT_CHILD_ADDED))

EWAPI extern const Efl_Event_Description _EVAS_BOX_EVENT_CHILD_REMOVED;

/** Called when a child object was removed from the box
 * @return Efl_Canvas_Object *
 *
 * @ingroup Evas_Box
 */
#define EVAS_BOX_EVENT_CHILD_REMOVED (&(_EVAS_BOX_EVENT_CHILD_REMOVED))

#endif
