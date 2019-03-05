#ifndef _EFL_CANVAS_LAYOUT_EO_LEGACY_H_
#define _EFL_CANVAS_LAYOUT_EO_LEGACY_H_

#ifndef _EFL_CANVAS_LAYOUT_EO_CLASS_TYPE
#define _EFL_CANVAS_LAYOUT_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Layout;

#endif

#ifndef _EFL_CANVAS_LAYOUT_EO_TYPES
#define _EFL_CANVAS_LAYOUT_EO_TYPES


#endif

/**
 * @brief Whether this object is animating or not.
 *
 * This property indicates whether animations are stopped or not. Animations
 * here refer to transitions between states.
 *
 * If animations are disabled, transitions between states (as defined in EDC)
 * are then instantaneous. This is conceptually similar to setting the
 * @ref Efl.Player.play_speed to an infinitely high value.
 *
 * Start or stop animating this object.
 *
 * @param[in] obj The object.
 * @param[in] on The animation state, @c true by default.
 *
 * @ingroup Edje_Object_Group
 */
EAPI void edje_object_animation_set(Efl_Canvas_Layout *obj, Eina_Bool on);

/**
 * @brief Whether this object is animating or not.
 *
 * This property indicates whether animations are stopped or not. Animations
 * here refer to transitions between states.
 *
 * If animations are disabled, transitions between states (as defined in EDC)
 * are then instantaneous. This is conceptually similar to setting the
 * @ref Efl.Player.play_speed to an infinitely high value.
 *
 * Get the current state of animation, @c true by default.
 *
 * @param[in] obj The object.
 *
 * @return The animation state, @c true by default.
 *
 * @ingroup Edje_Object_Group
 */
EAPI Eina_Bool edje_object_animation_get(const Efl_Canvas_Layout *obj);

/**
 * @brief Returns the seat device given its Edje's name.
 *
 * Edje references seats by a name that differs from Evas. Edje naming follows
 * a incrementional convention: first registered name is "seat1", second is
 * "seat2", differently from Evas.
 *
 * @param[in] obj The object.
 * @param[in] name The name's character string.
 *
 * @return The seat device or @c null if not found.
 *
 * @since 1.19
 *
 * @ingroup Edje_Object_Group
 */
EAPI Efl_Input_Device *edje_object_seat_get(const Efl_Canvas_Layout *obj, Eina_Stringshare *name);

/**
 * @brief Gets the name given to a set by Edje.
 *
 * Edje references seats by a name that differs from Evas. Edje naming follows
 * a incrementional convention: first registered name is "seat1", second is
 * "seat2", differently from Evas.
 *
 * @param[in] obj The object.
 * @param[in] device The seat device
 *
 * @return The name's character string or @c null if not found.
 *
 * @since 1.19
 *
 * @ingroup Edje_Object_Group
 */
EAPI Eina_Stringshare *edje_object_seat_name_get(const Efl_Canvas_Layout *obj, Efl_Input_Device *device);

/**
 * @brief Gets the (last) file loading error for a given object.
 *
 * @param[in] obj The object.
 *
 * @return The load error code.
 *
 * @ingroup Edje_Object_Group
 */
EAPI Eina_Error edje_object_layout_load_error_get(const Efl_Canvas_Layout *obj);

/**
 * @brief Unswallow an object from this Edje.
 *
 * @param[in] obj The object.
 * @param[in] content To be removed content.
 *
 * @return @c false if @c content was not a child or can not be removed.
 *
 * @ingroup Edje_Object_Group
 */
EAPI Eina_Bool edje_object_content_remove(Efl_Canvas_Layout *obj, Efl_Gfx_Entity *content);

#endif
