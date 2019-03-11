#ifndef _EFL_CANVAS_OBJECT_EO_LEGACY_H_
#define _EFL_CANVAS_OBJECT_EO_LEGACY_H_

#ifndef _EFL_CANVAS_OBJECT_EO_CLASS_TYPE
#define _EFL_CANVAS_OBJECT_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Object;

#endif

#ifndef _EFL_CANVAS_OBJECT_EO_TYPES
#define _EFL_CANVAS_OBJECT_EO_TYPES

/** Information of animation events
 *
 * @ingroup Efl_Canvas
 */
typedef struct _Efl_Canvas_Object_Animation_Event Efl_Canvas_Object_Animation_Event;

/** EFL event animator tick data structure
 *
 * @ingroup Efl
 */
typedef struct _Efl_Event_Animator_Tick
{
  Eina_Rect update_area; /**< Area of the canvas that will be pushed to screen.
                          */
} Efl_Event_Animator_Tick;


#endif



/**
 * @brief Low-level pointer behaviour.
 *
 * This function has a direct effect on event callbacks related to pointers
 * (mouse, ...).
 *
 * If the value is @ref EVAS_OBJECT_POINTER_MODE_AUTO_GRAB (default), then
 * when mouse is pressed down over this object, events will be restricted to it
 * as source, mouse moves, for example, will be emitted even when the pointer
 * goes outside this objects geometry.
 *
 * If the value is @ref EVAS_OBJECT_POINTER_MODE_NO_GRAB, then events will
 * be emitted just when inside this object area.
 *
 * The default value is @ref EVAS_OBJECT_POINTER_MODE_AUTO_GRAB. See also:
 * @ref Efl.Canvas.Object.pointer_mode_by_device.get and
 * @ref Efl.Canvas.Object.pointer_mode_by_device.set Note: This function will
 * only set/get the mode for the default pointer.
 *
 * @param[in] obj The object.
 * @param[in] pointer_mode Input pointer mode
 *
 * @return @c true if pointer behaviour was set, @c false otherwise
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_pointer_mode_set(Efl_Canvas_Object *obj, Evas_Object_Pointer_Mode pointer_mode);

/**
 * @brief Low-level pointer behaviour.
 *
 * This function has a direct effect on event callbacks related to pointers
 * (mouse, ...).
 *
 * If the value is @ref EVAS_OBJECT_POINTER_MODE_AUTO_GRAB (default), then
 * when mouse is pressed down over this object, events will be restricted to it
 * as source, mouse moves, for example, will be emitted even when the pointer
 * goes outside this objects geometry.
 *
 * If the value is @ref EVAS_OBJECT_POINTER_MODE_NO_GRAB, then events will
 * be emitted just when inside this object area.
 *
 * The default value is @ref EVAS_OBJECT_POINTER_MODE_AUTO_GRAB. See also:
 * @ref Efl.Canvas.Object.pointer_mode_by_device.get and
 * @ref Efl.Canvas.Object.pointer_mode_by_device.set Note: This function will
 * only set/get the mode for the default pointer.
 *
 * @param[in] obj The object.
 *
 * @return Input pointer mode
 *
 * @ingroup Evas_Object_Group
 */
EAPI Evas_Object_Pointer_Mode evas_object_pointer_mode_get(const Efl_Canvas_Object *obj);



/**
 * @brief Clip one object to another.
 *
 * This function will clip the object @c obj to the area occupied by the object
 * @c clip. This means the object @c obj will only be visible within the area
 * occupied by the clipping object ($clip).
 *
 * The color of the object being clipped will be multiplied by the color of the
 * clipping one, so the resulting color for the former will be "RESULT = (OBJ *
 * CLIP) / (255 * 255)", per color element (red, green, blue and alpha).
 *
 * Clipping is recursive, so clipping objects may be clipped by others, and
 * their color will in term be multiplied. You may not set up circular clipping
 * lists (i.e. object 1 clips object 2, which clips object 1): the behavior of
 * Evas is undefined in this case.
 *
 * Objects which do not clip others are visible in the canvas as normal; those
 * that clip one or more objects become invisible themselves, only affecting
 * what they clip. If an object ceases to have other objects being clipped by
 * it, it will become visible again.
 *
 * The visibility of an object affects the objects that are clipped by it, so
 * if the object clipping others is not shown (as in
 * @ref Efl.Gfx.Entity.visible), the objects clipped by it will not be shown
 *  either.
 *
 * If @c obj was being clipped by another object when this function is  called,
 * it gets implicitly removed from the old clipper's domain and is made now to
 * be clipped by its new clipper.
 *
 * If @c clip is @c null, this call will disable clipping for the object i.e.
 * its visibility and color get detached from the previous clipper. If it
 * wasn't, this has no effect.
 *
 * @note Only rectangle and image (masks) objects can be used as clippers.
 * Anything else will result in undefined behaviour.
 *
 * @param[in] obj The object.
 * @param[in] clip The object to clip @c obj by.
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_clip_set(Efl_Canvas_Object *obj, Efl_Canvas_Object *clip) EINA_ARG_NONNULL(2);

/**
 * @brief Get the object clipping @c obj (if any).
 *
 * This function returns the object clipping @c obj. If @c obj is not being
 * clipped at all, @c null is returned. The object @c obj must be a valid
 * Evas_Object.
 *
 * @param[in] obj The object.
 *
 * @return The object to clip @c obj by.
 *
 * @ingroup Evas_Object_Group
 */
EAPI Efl_Canvas_Object *evas_object_clip_get(const Efl_Canvas_Object *obj);

/**
 * @brief Set whether an Evas object is to repeat events.
 *
 * If @c repeat is @c true, it will make events on @c obj to also be repeated
 * for the next lower object in the objects' stack (see see
 * @ref Efl.Gfx.Stack.below).
 *
 * If @c repeat is @c false, events occurring on @c obj will be processed only
 * on it.
 *
 * @param[in] obj The object.
 * @param[in] repeat Whether @c obj is to repeat events ($true) or not
 * ($false).
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_repeat_events_set(Efl_Canvas_Object *obj, Eina_Bool repeat);

/**
 * @brief Determine whether an object is set to repeat events.
 *
 * @param[in] obj The object.
 *
 * @return Whether @c obj is to repeat events ($true) or not ($false).
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_repeat_events_get(const Efl_Canvas_Object *obj);

/**
 * @brief Indicates that this object is the keyboard event receiver on its
 * canvas.
 *
 * Changing focus only affects where (key) input events go. There can be only
 * one object focused at any time. If @c focus is @c true, @c obj will be set
 * as the currently focused object and it will receive all keyboard events that
 * are not exclusive key grabs on other objects. See also
 * @ref Efl.Canvas.Object.seat_focus_check,
 * @ref Efl.Canvas.Object.seat_focus_add,
 * @ref Efl.Canvas.Object.seat_focus_del.
 *
 * @param[in] obj The object.
 * @param[in] focus @c true when set as focused or @c false otherwise.
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_focus_set(Efl_Canvas_Object *obj, Eina_Bool focus);

/**
 * @brief Indicates that this object is the keyboard event receiver on its
 * canvas.
 *
 * Changing focus only affects where (key) input events go. There can be only
 * one object focused at any time. If @c focus is @c true, @c obj will be set
 * as the currently focused object and it will receive all keyboard events that
 * are not exclusive key grabs on other objects. See also
 * @ref Efl.Canvas.Object.seat_focus_check,
 * @ref Efl.Canvas.Object.seat_focus_add,
 * @ref Efl.Canvas.Object.seat_focus_del.
 *
 * @param[in] obj The object.
 *
 * @return @c true when set as focused or @c false otherwise.
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_focus_get(const Efl_Canvas_Object *obj);





/**
 * @brief Set whether to use precise (usually expensive) point collision
 * detection for a given Evas object.
 *
 * Use this function to make Evas treat objects' transparent areas as not
 * belonging to it with regard to mouse pointer events. By default, all of the
 * object's boundary rectangle will be taken in account for them.
 *
 * @warning By using precise point collision detection you'll be making Evas
 * more resource intensive.
 *
 * @param[in] obj The object.
 * @param[in] precise Whether to use precise point collision detection or not.
 * The default value is false.
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_precise_is_inside_set(Efl_Canvas_Object *obj, Eina_Bool precise);

/**
 * @brief Determine whether an object is set to use precise point collision
 * detection.
 *
 * @param[in] obj The object.
 *
 * @return Whether to use precise point collision detection or not. The default
 * value is false.
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_precise_is_inside_get(const Efl_Canvas_Object *obj);

/**
 * @brief Set whether events on a smart object's member should be propagated up
 * to its parent.
 *
 * This function has no effect if @c obj is not a member of a smart object.
 *
 * If @c prop is @c true, events occurring on this object will be propagated on
 * to the smart object of which @c obj is a member. If @c prop is @c false,
 * events occurring on this object will not be propagated on to the smart
 * object of which @c obj is a member. The default value is @c true.
 *
 * See also @ref evas_object_repeat_events_set,
 * @ref evas_object_pass_events_set.
 *
 * @param[in] obj The object.
 * @param[in] propagate Whether to propagate events ($true) or not ($false).
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_propagate_events_set(Efl_Canvas_Object *obj, Eina_Bool propagate);

/**
 * @brief Retrieve whether an Evas object is set to propagate events.
 *
 * See also @ref evas_object_repeat_events_get,
 * @ref evas_object_pass_events_get.
 *
 * @param[in] obj The object.
 *
 * @return Whether to propagate events ($true) or not ($false).
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_propagate_events_get(const Efl_Canvas_Object *obj);

/**
 * @brief Set whether an Evas object is to pass (ignore) events.
 *
 * If @c pass is @c true, it will make events on @c obj to be ignored. They
 * will be triggered on the next lower object (that is not set to pass events),
 * instead (see @ref Efl.Gfx.Stack.below).
 *
 * If @c pass is @c false events will be processed on that object as normal.
 *
 * See also @ref evas_object_repeat_events_set,
 * @ref evas_object_propagate_events_set
 *
 * @param[in] obj The object.
 * @param[in] pass Whether @c obj is to pass events ($true) or not ($false).
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_pass_events_set(Efl_Canvas_Object *obj, Eina_Bool pass);

/**
 * @brief Determine whether an object is set to pass (ignore) events.
 *
 * See also @ref evas_object_repeat_events_get,
 * @ref evas_object_propagate_events_get.
 *
 * @param[in] obj The object.
 *
 * @return Whether @c obj is to pass events ($true) or not ($false).
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_pass_events_get(const Efl_Canvas_Object *obj);

/**
 * @brief Sets whether or not the given Evas object is to be drawn
 * anti-aliased.
 *
 * @param[in] obj The object.
 * @param[in] anti_alias @c true if the object is to be anti_aliased, @c false
 * otherwise.
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_anti_alias_set(Efl_Canvas_Object *obj, Eina_Bool anti_alias);

/**
 * @brief Retrieves whether or not the given Evas object is to be drawn
 * anti_aliased.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the object is to be anti_aliased, @c false otherwise.
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_anti_alias_get(const Efl_Canvas_Object *obj);


/**
 * @brief Gets the parent smart object of a given Evas object, if it has one.
 *
 * This can be different from @ref Efl.Object.parent because this one is used
 * internally for rendering and the normal parent is what the user expects to
 * be the parent.
 *
 * @param[in] obj The object.
 *
 * @return The parent smart object of @c obj or @c null.
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Group
 */
EAPI Efl_Canvas_Object *evas_object_smart_parent_get(const Efl_Canvas_Object *obj);

/**
 * @brief This handles text paragraph direction of the given object. Even if
 * the given object is not textblock or text, its smart child objects can
 * inherit the paragraph direction from the given object. The default paragraph
 * direction is @c inherit.
 *
 * @param[in] obj The object.
 * @param[in] dir Paragraph direction for the given object.
 *
 * @ingroup Evas_Object_Group
 */
EAPI void evas_object_paragraph_direction_set(Efl_Canvas_Object *obj, Efl_Text_Bidirectional_Type dir);

/**
 * @brief This handles text paragraph direction of the given object. Even if
 * the given object is not textblock or text, its smart child objects can
 * inherit the paragraph direction from the given object. The default paragraph
 * direction is @c inherit.
 *
 * @param[in] obj The object.
 *
 * @return Paragraph direction for the given object.
 *
 * @ingroup Evas_Object_Group
 */
EAPI Efl_Text_Bidirectional_Type evas_object_paragraph_direction_get(const Efl_Canvas_Object *obj);

/**
 * @brief Test if any object is clipped by @c obj.
 *
 * @param[in] obj The object.
 *
 * @return @c true if any object is clipped by @c obj, @c false otherwise
 *
 * @since 1.8
 *
 * @ingroup Evas_Object_Group
 */
EAPI Eina_Bool evas_object_clipees_has(const Efl_Canvas_Object *obj) EINA_WARN_UNUSED_RESULT;






#endif
