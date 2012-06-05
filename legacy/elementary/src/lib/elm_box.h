/**
 * @defgroup Box Box
 * @ingroup Elementary
 *
 * @image html box_inheritance_tree.png
 * @image latex box_inheritance_tree.eps
 *
 * @image html img/widget/box/preview-00.png
 * @image latex img/widget/box/preview-00.eps width=\textwidth
 *
 * @image html img/box.png
 * @image latex img/box.eps width=\textwidth
 *
 * A box arranges objects in a linear fashion, governed by a layout function
 * that defines the details of this arrangement.
 *
 * By default, the box will use an internal function to set the layout to
 * a single row, either vertical or horizontal. This layout is affected
 * by a number of parameters, such as the homogeneous flag set by
 * elm_box_homogeneous_set(), the values given by elm_box_padding_set() and
 * elm_box_align_set() and the hints set to each object in the box.
 *
 * For this default layout, it's possible to change the orientation with
 * elm_box_horizontal_set(). The box will start in the vertical orientation,
 * placing its elements ordered from top to bottom. When horizontal is set,
 * the order will go from left to right. If the box is set to be
 * homogeneous, every object in it will be assigned the same space, that
 * of the largest object. Padding can be used to set some spacing between
 * the cell given to each object. The alignment of the box, set with
 * elm_box_align_set(), determines how the bounding box of all the elements
 * will be placed within the space given to the box widget itself.
 *
 * The size hints of each object also affect how they are placed and sized
 * within the box. evas_object_size_hint_min_set() will give the minimum
 * size the object can have, and the box will use it as the basis for all
 * latter calculations. Elementary widgets set their own minimum size as
 * needed, so there's rarely any need to use it manually.
 *
 * evas_object_size_hint_weight_set(), when not in homogeneous mode, is
 * used to tell whether the object will be allocated the minimum size it
 * needs or if the space given to it should be expanded. It's important
 * to realize that expanding the size given to the object is not the same
 * thing as resizing the object. It could very well end being a small
 * widget floating in a much larger empty space. If not set, the weight
 * for objects will normally be 0.0 for both axis, meaning the widget will
 * not be expanded. To take as much space possible, set the weight to
 * EVAS_HINT_EXPAND (defined to 1.0) for the desired axis to expand.
 *
 * Besides how much space each object is allocated, it's possible to control
 * how the widget will be placed within that space using
 * evas_object_size_hint_align_set(). By default, this value will be 0.5
 * for both axis, meaning the object will be centered, but any value from
 * 0.0 (left or top, for the @c x and @c y axis, respectively) to 1.0
 * (right or bottom) can be used. The special value EVAS_HINT_FILL, which
 * is -1.0, means the object will be resized to fill the entire space it
 * was allocated.
 *
 * In addition, customized functions to define the layout can be set, which
 * allow the application developer to organize the objects within the box
 * in any number of ways.
 *
 * The special elm_box_layout_transition() function can be used
 * to switch from one layout to another, animating the motion of the
 * children of the box.
 *
 * @note Objects should not be added to box objects using _add() calls.
 *
 * Some examples on how to use boxes follow:
 * @li @ref box_example_01
 * @li @ref box_example_02
 *
 * @{
 */
/**
 * @typedef Elm_Box_Transition
 *
 * Opaque handler containing the parameters to perform an animated
 * transition of the layout the box uses.
 *
 * @see elm_box_transition_new()
 * @see elm_box_layout_set()
 * @see elm_box_layout_transition()
 */
typedef struct _Elm_Box_Transition Elm_Box_Transition;

/**
 * Add a new box to the parent
 *
 * By default, the box will be in vertical mode and non-homogeneous.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Box
 */
EAPI Evas_Object        *elm_box_add(Evas_Object *parent);

/**
 * Set the horizontal orientation
 *
 * By default, box object arranges their contents vertically from top to
 * bottom.
 * By calling this function with @p horizontal as EINA_TRUE, the box will
 * become horizontal, arranging contents from left to right.
 *
 * @note This flag is ignored if a custom layout function is set.
 *
 * @param obj The box object
 * @param horizontal The horizontal flag (EINA_TRUE = horizontal,
 * EINA_FALSE = vertical)
 *
 * @ingroup Box
 */
EAPI void                elm_box_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Get the horizontal orientation
 *
 * @param obj The box object
 * @return EINA_TRUE if the box is set to horizontal mode, EINA_FALSE otherwise
 */
EAPI Eina_Bool           elm_box_horizontal_get(const Evas_Object *obj);

/**
 * Set the box to arrange its children homogeneously
 *
 * If enabled, homogeneous layout makes all items the same size, according
 * to the size of the largest of its children.
 *
 * @note This flag is ignored if a custom layout function is set.
 *
 * @param obj The box object
 * @param homogeneous The homogeneous flag
 *
 * @ingroup Box
 */
EAPI void                elm_box_homogeneous_set(Evas_Object *obj, Eina_Bool homogeneous);

/**
 * Get whether the box is using homogeneous mode or not
 *
 * @param obj The box object
 * @return EINA_TRUE if it's homogeneous, EINA_FALSE otherwise
 *
 * @ingroup Box
 */
EAPI Eina_Bool           elm_box_homogeneous_get(const Evas_Object *obj);

/**
 * Add an object to the beginning of the pack list
 *
 * Pack @p subobj into the box @p obj, placing it first in the list of
 * children objects. The actual position the object will get on screen
 * depends on the layout used. If no custom layout is set, it will be at
 * the top or left, depending if the box is vertical or horizontal,
 * respectively.
 *
 * @param obj The box object
 * @param subobj The object to add to the box
 *
 * @see elm_box_pack_end()
 * @see elm_box_pack_before()
 * @see elm_box_pack_after()
 * @see elm_box_unpack()
 * @see elm_box_unpack_all()
 * @see elm_box_clear()
 *
 * @ingroup Box
 */
EAPI void                elm_box_pack_start(Evas_Object *obj, Evas_Object *subobj);

/**
 * Add an object at the end of the pack list
 *
 * Pack @p subobj into the box @p obj, placing it last in the list of
 * children objects. The actual position the object will get on screen
 * depends on the layout used. If no custom layout is set, it will be at
 * the bottom or right, depending if the box is vertical or horizontal,
 * respectively.
 *
 * @param obj The box object
 * @param subobj The object to add to the box
 *
 * @see elm_box_pack_start()
 * @see elm_box_pack_before()
 * @see elm_box_pack_after()
 * @see elm_box_unpack()
 * @see elm_box_unpack_all()
 * @see elm_box_clear()
 *
 * @ingroup Box
 */
EAPI void                elm_box_pack_end(Evas_Object *obj, Evas_Object *subobj);

/**
 * Adds an object to the box before the indicated object
 *
 * This will add the @p subobj to the box indicated before the object
 * indicated with @p before. If @p before is not already in the box, results
 * are undefined. Before means either to the left of the indicated object or
 * above it depending on orientation.
 *
 * @param obj The box object
 * @param subobj The object to add to the box
 * @param before The object before which to add it
 *
 * @see elm_box_pack_start()
 * @see elm_box_pack_end()
 * @see elm_box_pack_after()
 * @see elm_box_unpack()
 * @see elm_box_unpack_all()
 * @see elm_box_clear()
 *
 * @ingroup Box
 */
EAPI void                elm_box_pack_before(Evas_Object *obj, Evas_Object *subobj, Evas_Object *before);

/**
 * Adds an object to the box after the indicated object
 *
 * This will add the @p subobj to the box indicated after the object
 * indicated with @p after. If @p after is not already in the box, results
 * are undefined. After means either to the right of the indicated object or
 * below it depending on orientation.
 *
 * @param obj The box object
 * @param subobj The object to add to the box
 * @param after The object after which to add it
 *
 * @see elm_box_pack_start()
 * @see elm_box_pack_end()
 * @see elm_box_pack_before()
 * @see elm_box_unpack()
 * @see elm_box_unpack_all()
 * @see elm_box_clear()
 *
 * @ingroup Box
 */
EAPI void                elm_box_pack_after(Evas_Object *obj, Evas_Object *subobj, Evas_Object *after);

/**
 * Clear the box of all children
 *
 * Remove all the elements contained by the box, deleting the respective
 * objects.
 *
 * @param obj The box object
 *
 * @see elm_box_unpack()
 * @see elm_box_unpack_all()
 *
 * @ingroup Box
 */
EAPI void                elm_box_clear(Evas_Object *obj);

/**
 * Unpack a box item
 *
 * Remove the object given by @p subobj from the box @p obj without
 * deleting it.
 *
 * @param obj The box object
 * @param subobj The object to unpack
 *
 * @see elm_box_unpack_all()
 * @see elm_box_clear()
 *
 * @ingroup Box
 */
EAPI void                elm_box_unpack(Evas_Object *obj, Evas_Object *subobj);

/**
 * Remove all items from the box, without deleting them
 *
 * Clear the box from all children, but don't delete the respective objects.
 * If no other references of the box children exist, the objects will never
 * be deleted, and thus the application will leak the memory. Make sure
 * when using this function that you hold a reference to all the objects
 * in the box @p obj.
 *
 * @param obj The box object
 *
 * @see elm_box_clear()
 * @see elm_box_unpack()
 *
 * @ingroup Box
 */
EAPI void                elm_box_unpack_all(Evas_Object *obj);

/**
 * Retrieve a list of the objects packed into the box
 *
 * Returns a new @c Eina_List with a pointer to @c Evas_Object in its nodes.
 * The order of the list corresponds to the packing order the box uses.
 *
 * You must free this list with eina_list_free() once you are done with it.
 *
 * @param obj The box object
 *
 * @ingroup Box
 */
EAPI Eina_List    *elm_box_children_get(const Evas_Object *obj);

/**
 * Set the space (padding) between the box's elements.
 *
 * Extra space in pixels that will be added between a box child and its
 * neighbors after its containing cell has been calculated. This padding
 * is set for all elements in the box, besides any possible padding that
 * individual elements may have through their size hints.
 *
 * @param obj The box object
 * @param horizontal The horizontal space between elements
 * @param vertical The vertical space between elements
 *
 * @ingroup Box
 */
EAPI void                elm_box_padding_set(Evas_Object *obj, Evas_Coord horizontal, Evas_Coord vertical);

/**
 * Get the space (padding) between the box's elements.
 *
 * @param obj The box object
 * @param horizontal The horizontal space between elements
 * @param vertical The vertical space between elements
 *
 * @see elm_box_padding_set()
 *
 * @ingroup Box
 */
EAPI void                elm_box_padding_get(const Evas_Object *obj, Evas_Coord *horizontal, Evas_Coord *vertical);

/**
 * Set the alignment of the whole bounding box of contents.
 *
 * Sets how the bounding box containing all the elements of the box, after
 * their sizes and position has been calculated, will be aligned within
 * the space given for the whole box widget.
 *
 * @param obj The box object
 * @param horizontal The horizontal alignment of elements
 * @param vertical The vertical alignment of elements
 *
 * @ingroup Box
 */
EAPI void                elm_box_align_set(Evas_Object *obj, double horizontal, double vertical);

/**
 * Get the alignment of the whole bounding box of contents.
 *
 * @param obj The box object
 * @param horizontal The horizontal alignment of elements
 * @param vertical The vertical alignment of elements
 *
 * @see elm_box_align_set()
 *
 * @ingroup Box
 */
EAPI void                elm_box_align_get(const Evas_Object *obj, double *horizontal, double *vertical);

/**
 * Force the box to recalculate its children packing.
 *
 * If any children was added or removed, box will not calculate the
 * values immediately rather leaving it to the next main loop
 * iteration. While this is great as it would save lots of
 * recalculation, whenever you need to get the position of a just
 * added item you must force recalculate before doing so.
 *
 * @param obj The box object.
 *
 * @ingroup Box
 */
EAPI void                elm_box_recalculate(Evas_Object *obj);

/**
 * Set the layout defining function to be used by the box
 *
 * Whenever anything changes that requires the box in @p obj to recalculate
 * the size and position of its elements, the function @p cb will be called
 * to determine what the layout of the children will be.
 *
 * Once a custom function is set, everything about the children layout
 * is defined by it. The flags set by elm_box_horizontal_set() and
 * elm_box_homogeneous_set() no longer have any meaning, and the values
 * given by elm_box_padding_set() and elm_box_align_set() are up to this
 * layout function to decide if they are used and how. These last two
 * will be found in the @c priv parameter, of type @c Evas_Object_Box_Data,
 * passed to @p cb. The @c Evas_Object the function receives is not the
 * Elementary widget, but the internal Evas Box it uses, so none of the
 * functions described here can be used on it.
 *
 * Any of the layout functions in @c Evas can be used here, as well as the
 * special elm_box_layout_transition().
 *
 * The final @p data argument received by @p cb is the same @p data passed
 * here, and the @p free_data function will be called to free it
 * whenever the box is destroyed or another layout function is set.
 *
 * Setting @p cb to NULL will revert back to the default layout function.
 *
 * @param obj The box object
 * @param cb The callback function used for layout
 * @param data Data that will be passed to layout function
 * @param free_data Function called to free @p data
 *
 * @see elm_box_layout_transition()
 *
 * @ingroup Box
 */
EAPI void                elm_box_layout_set(Evas_Object *obj, Evas_Object_Box_Layout cb, const void *data, Ecore_Cb free_data);

/**
 * Special layout function that animates the transition from one layout to another
 *
 * Normally, when switching the layout function for a box, this will be
 * reflected immediately on screen on the next render, but it's also
 * possible to do this through an animated transition.
 *
 * This is done by creating an ::Elm_Box_Transition and setting the box
 * layout to this function.
 *
 * For example:
 * @code
 * Elm_Box_Transition *t = elm_box_transition_new(1.0,
 *                            evas_object_box_layout_vertical, // start
 *                            NULL, // data for initial layout
 *                            NULL, // free function for initial data
 *                            evas_object_box_layout_horizontal, // end
 *                            NULL, // data for final layout
 *                            NULL, // free function for final data
 *                            anim_end, // will be called when animation ends
 *                            NULL); // data for anim_end function\
 * elm_box_layout_set(box, elm_box_layout_transition, t,
 *                    elm_box_transition_free);
 * @endcode
 *
 * @note This function can only be used with elm_box_layout_set(). Calling
 * it directly will not have the expected results.
 *
 * @see elm_box_transition_new
 * @see elm_box_transition_free
 * @see elm_box_layout_set
 *
 * @ingroup Box
 */
EAPI void                elm_box_layout_transition(Evas_Object *obj, Evas_Object_Box_Data *priv, void *data);

/**
 * Create a new ::Elm_Box_Transition to animate the switch of layouts
 *
 * If you want to animate the change from one layout to another, you need
 * to set the layout function of the box to elm_box_layout_transition(),
 * passing as user data to it an instance of ::Elm_Box_Transition with the
 * necessary information to perform this animation. The free function to
 * set for the layout is elm_box_transition_free().
 *
 * The parameters to create an ::Elm_Box_Transition sum up to how long
 * will it be, in seconds, a layout function to describe the initial point,
 * another for the final position of the children and one function to be
 * called when the whole animation ends. This last function is useful to
 * set the definitive layout for the box, usually the same as the end
 * layout for the animation, but could be used to start another transition.
 *
 * @param duration The duration of the transition in seconds
 * @param start_layout The layout function that will be used to start the animation
 * @param start_layout_data The data to be passed the @p start_layout function
 * @param start_layout_free_data Function to free @p start_layout_data
 * @param end_layout The layout function that will be used to end the animation
 * @param end_layout_data Data param passed to @p end_layout
 * @param end_layout_free_data The data to be passed the @p end_layout function
 * @param end_layout_free_data Function to free @p end_layout_data
 * @param transition_end_cb Callback function called when animation ends
 * @param transition_end_data Data to be passed to @p transition_end_cb
 * @return An instance of ::Elm_Box_Transition
 *
 * @see elm_box_transition_new
 * @see elm_box_layout_transition
 *
 * @ingroup Box
 */
EAPI Elm_Box_Transition *elm_box_transition_new(const double duration, Evas_Object_Box_Layout start_layout, void *start_layout_data, Ecore_Cb start_layout_free_data, Evas_Object_Box_Layout end_layout, void *end_layout_data, Ecore_Cb end_layout_free_data, Ecore_Cb transition_end_cb, void *transition_end_data);

/**
 * Free a Elm_Box_Transition instance created with elm_box_transition_new().
 *
 * This function is mostly useful as the @c free_data parameter in
 * elm_box_layout_set() when elm_box_layout_transition().
 *
 * @param data The Elm_Box_Transition instance to be freed.
 *
 * @see elm_box_transition_new
 * @see elm_box_layout_transition
 *
 * @ingroup Box
 */
EAPI void                elm_box_transition_free(void *data);

/**
 * @}
 */
