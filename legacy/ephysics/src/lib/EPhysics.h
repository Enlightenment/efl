#ifndef EPHYSICS_H
#define EPHYSICS_H

/**
 * @mainpage EPhysics Library Documentation
 *
 * @version 0.1.0
 * @date 2012
 *
 * @section intro What is EPhysics ?
 *
 * EPhysics is a library that manages Ecore, Evas and Bullet Physics into
 * an easy to use way. It's a kind of wrapper, a glue, between these libraries.
 * It's not intended to be a physics library (we already have many out there).
 *
 * @image html diagram_ephysics.png
 *
 * For a better reference, check the following groups:
 * @li @ref EPhysics
 * @li @ref EPhysics_World
 * @li @ref EPhysics_Body
 * @li @ref EPhysics_Camera
 * @li @ref EPhysics_Constraint
 *
 * Please see the @ref authors page for contact details.
 */

/**
 *
 * @page authors Authors
 *
 * @author Bruno Dilly <bdilly@@profusion.mobi>
 * @author Leandro Dorileo <dorileo@@profusion.mobi>
 *
 * Please contact <enlightenment-devel@lists.sourceforge.net> to get in
 * contact with the developers and maintainers.
 *
 */

#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EPHYSICS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EPHYSICS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

#define EPHYSICS_VERSION_MAJOR 0
#define EPHYSICS_VERSION_MINOR 1

/**
 * @file
 * @brief These routines are used for EPhysics library interaction.
 */

/**
 * @brief How to initialize EPhysics.
 * @defgroup EPhysics Top Level API available to add physics effects.
 *
 * @{
 *
 * See @ref EPhysics and @ref EPhysics_World for details about
 * these interfaces.
 *
 */

/**
 * Initialize EPhysics
 *
 * Initializes Bullet physics engine.
 *
 * @return The init counter value.
 *
 * @see ephysics_shutdown().
 *
 * @ingroup EPhysics
 */
EAPI int ephysics_init(void);

/**
 * Shutdown EPhysics
 *
 * Shutdown Bullet physics engine. If init count reaches 0, all the existing
 * worlds will be deleted, and consequently all the bodies.
 *
 * @return Ephysics' init counter value.
 *
 * @see ephysics_init().
 *
 * @ingroup EPhysics
 */
EAPI int ephysics_shutdown(void);

/**
 * @}
 */

/**
 * @defgroup EPhysics_Camera EPhysics Camera
 * @ingroup EPhysics
 *
 * @{
 *
 * A camera defines the region of the physics world that will be rendered
 * on the canvas. It sets the point of view.
 *
 * Every world has a camera, that can be get with
 * @ref ephysics_world_camera_get().
 * Its position can be set with @ref ephysics_camera_position_set() and zoom
 * in / zoom out can be done with @ref ephysics_camera_zoom_set().
 *
 * @note WIP: IT'S NOT WORKING YET!!
 */

typedef struct _EPhysics_Camera EPhysics_Camera; /**< Camera handle, used to zoom in / out a scene, or change the frame position to be rendered. Every world have a camera that can be get with @ref ephysics_world_camera_get(). */

/**
 * @brief
 * Set camera's position.
 *
 * Camera's position referes to the position of the top-left point of the
 * camera.
 *
 * By default a camera is created to map the first quadrant of physics
 * world from the point (0, 0) to
 * (canvas width / world rate, canvas height / world rate).
 *
 * It can be modified passing another top-left point position, so another
 * region of the physics world will be rendered on the canvas.
 *
 * @note This change will be noticed on the next physics tick, so evas objects
 * will be updated taking the camera's new position in account.
 *
 * @param camera The camera to be positioned.
 * @param x The new position on x axis, in pixels.
 * @param y The new position on y axis, in pixels.
 *
 * @see ephysics_camera_position_get().
 * @see ephysics_world_camera_get().
 * @see ephysics_world_rate_get().
 *
 * @ingroup EPhysics_Camera
 */
EAPI void ephysics_camera_position_set(EPhysics_Camera *camera, double x, double y);

/**
 * @brief
 * Get camera's position.
 *
 * @param camera The world's camera.
 * @param x Position on x axis, in pixels.
 * @param y Position on y axis, in pixels.
 *
 * @see ephysics_camera_position_set() for more details.
 *
 * @ingroup EPhysics_Camera
 */
EAPI void ephysics_camera_position_get(const EPhysics_Camera *camera, double *x, double *y);

/**
 * @brief
 * Set camera's zoom.
 *
 * Zoom affects size and position of evas objects automatically updated
 * by ephysics. With zoom equal to 1, the original size of evas objects is
 * used and their position on canvas depends only on world rate and
 * camera's position.
 *
 * When zoom is smaller than 1, evas objects will be scaled down, and position
 * will take world rate, camera's position and zoom in account. More objects
 * could fit on the screen with small zoom values.
 *
 * If zoom is bigger than 1, evas objects will be scaled up, and position
 * will take world rate, camera's position and zoom in account. Less objects
 * will fit on the screen with big zoom values. It's nice for close-up
 * effects.
 *
 * By default zoom value is 1.
 *
 * @note This change will be noticed on the next physics tick, so evas objects
 * will be updated taking the camera's new zoom in account.
 *
 * @param camera The camera to be zoomed in / out.
 * @param zoom The new zoom value. Must to be bigger than 0.
 *
 * @see ephysics_camera_zoom_get().
 *
 * @ingroup EPhysics_Camera
 */
EAPI void ephysics_camera_zoom_set(EPhysics_Camera *camera, double zoom);

/**
 * @brief
 * Get camera's zoom.
 *
 * @param camera The world's camera.
 * @return The zoom value or -1 on error.
 *
 * @see ephysics_camera_zoom_set() for more details.
 *
 * @ingroup EPhysics_Camera
 */
EAPI double ephysics_camera_zoom_get(const EPhysics_Camera *camera);

/**
 * @}
 */


/**
 * @defgroup EPhysics_World EPhysics World
 * @ingroup EPhysics
 *
 * @{
 *
 * A world is required to simulate physics between bodies.
 * It will setup collision configuration, constraint solver, the
 * broadphase interface and a dispatcher to dispatch calculations
 * for overlapping pairs.
 *
 * A new world can be created with @ref ephysics_world_new() and deleted with
 * @ref ephysics_world_del(). It can have its gravity changed with
 * @ref ephysics_world_gravity_set() and play / paused with
 * @ref ephysics_world_running_set(). When running, the simulation will be
 * gradually stepped.
 */

typedef struct _EPhysics_World EPhysics_World; /**< World handle, most basic type of EPhysics. Created with @ref ephysics_world_new() and deleted with @ref ephysics_world_del(). */

/**
 * @enum _EPhysics_Callback_World_Type
 * @typedef EPhysics_Callback_World_Type
 *
 * Identifier of callbacks to be set for EPhysics worlds.
 *
 * @see ephysics_world_event_callback_add()
 * @see ephysics_world_event_callback_del()
 * @see ephysics_world_event_callback_del_full()
 *
 * @ingroup EPhysics_World
 */
typedef enum _EPhysics_Callback_World_Type
{
   EPHYSICS_CALLBACK_WORLD_DEL, /**< World being deleted (called before free) */
   EPHYSICS_CALLBACK_WORLD_STOPPED, /**< no objects are moving any more */
   EPHYSICS_CALLBACK_WORLD_LAST, /**< kept as sentinel, not really an event */
} EPhysics_Callback_World_Type;

/**
 * @typedef EPhysics_World_Event_Cb
 *
 * EPhysics world event callback function signature.
 *
 * Callbacks can be registered for events like world deleting.
 *
 * @param data User data that will be set when registering the callback.
 * @param world Physics world.
 * @param event_info Data specific to a kind of event. Some types of events
 * don't have @p event_info.
 *
 * @see ephysics_world_event_callback_add() for more info.
 *
 * @ingroup EPhysics_World
 */
typedef void (*EPhysics_World_Event_Cb)(void *data, EPhysics_World *world, void *event_info);

/**
 * @brief
 * Create a new physics world.
 *
 * A new world will be created with set collision configuration,
 * constraint solver, broadphase interface and dispatcher.
 *
 * It can be paused / unpaused with @ref ephysics_world_running_set() and its
 * gravity can be changed with @ref ephysics_world_gravity_set().
 *
 * By default it starts with gravity y = 294 Evas coordinates per second ^ 2
 * and playing.
 *
 * If default updates between physics bodies and evas objects will be used
 * it's mandatory to set the size of the area to be rendered with
 * @ref ephysics_world_render_geometry_set().
 *
 * @return A new world or @c NULL, on errors.
 *
 * @see ephysics_world_del().
 *
 * @ingroup EPhysics_World
 */
EAPI EPhysics_World *ephysics_world_new(void);

/**
 * @brief
 * Set dimensions of rendered area to be take on account by default updates.
 *
 * By default it starts with null x, y, width and height.
 *
 * The physics world won't be limited, but boundaries can be added with:
 * @li @ref ephysics_body_top_boundary_add(),
 * @li @ref ephysics_body_bottom_boundary_add(),
 * @li @ref ephysics_body_left_boundary_add(),
 * @li @ref ephysics_body_right_boundary_add().
 *
 * @param world the world to be configured.
 * @param x Coordinate x of the top left point of rendered area, in pixels.
 * @param y Coordinate y of the top left point of rendered area, in pixels.
 * @param w rendered area width, in pixels.
 * @param h rendered area height, in pixels.
 *
 * @note The unit used for geometry is Evas coordinates.
 *
 * @see ephysics_body_event_callback_add() for more info.
 * @see ephysics_world_rate_get().
 * @see ephysics_world_render_geometry_get().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_render_geometry_set(EPhysics_World *world, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @brief
 * Get dimensions of rendered area to be take on account by default updates.
 *
 * @param world the world to be configured.
 * @param x Coordinate x of the top left point of rendered area, in pixels.
 * @param y Coordinate y of the top left point of rendered area, in pixels.
 * @param w rendered area width, in pixels.
 * @param h rendered area height, in pixels.
 *
 * @see ephysics_world_render_geometry_set() for more information.
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_render_geometry_get(EPhysics_World *world, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief
 * Serializes the @p world to @p path.
 *
 * Save the dynamics world to a binary dump, a .bullet file.
 *
 * @note Should be used only for debugging purporses.
 *
 * @param world the world to be serialized.
 * @param path where the serialized world should be written to.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup EPhysics_World
 */
EAPI Eina_Bool ephysics_world_serialize(const EPhysics_World *world, const char *path);

/**
 * @brief
 * Deletes a physics world.
 *
 * It will also delete all bodies associated to it.
 *
 * @param world The world to be deleted.
 *
 * @see ephysics_world_new() for more details.
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_del(EPhysics_World *world);

/**
 * @brief
 * Set running status of world.
 *
 * A world can be played / paused. When running, it will simulate the
 * physics step by step. When paused, it will stop simulation. Consequently
 * all the registered callbacks won't be called since no event will ocurr
 * (no collisions, no object updates).
 *
 * When a world is created it starts running.
 *
 * @see ephysics_world_running_get()
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_running_set(EPhysics_World *world, Eina_Bool running);

/**
 * @brief
 * Get running status of world.
 *
 * By default a world starts running.
 *
 * @see ephysics_world_running_set() for more details.
 *
 * @ingroup EPhysics_World
 */
EAPI Eina_Bool ephysics_world_running_get(const EPhysics_World *world);

/**
 * @brief
 * Set world gravity in 2 axises (x, y).
 *
 * Gravity will act over bodies with mass over all the time.
 *
 * By default values are 0, 294 Evas Coordinates per second ^ 2
 * (9.8 m/s^2, since we've a default rate of 30 pixels).
 *
 * If you change the rate but wants to keep 9.8 m/s^2, you well need
 * to set world gravity with: 9.8 * new_rate.
 *
 * @param world The world object.
 * @param gx Gravity on x axis.
 * @param gy Gravity on y axis.
 *
 * @note The unit used for acceleration is Evas coordinates per second ^ 2.
 *
 * @see ephysics_world_gravity_get().
 * @see ephysics_world_rate_set().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_gravity_set(EPhysics_World *world, double gx, double gy);

/**
 * @brief
 * Set the number of iterations the constraint solver will have for contact and
 * joint constraints.
 *
 * The default value is set to 10. The greater number of iterations more
 * quality and precise the result but with performance penalty.
 *
 * By default, the Projected Gauss Seidel constraint solver is used for contact
 * and joint constraints. The algorithm is an iterative LCP solver, informally
 * known as 'sequential impulse'.
 *
 * A reasonable range of iterations is from 4 (low quality, good performance)
 * to 20 (good quality, less but still reasonable performance).
 *
 * @param world The world to be set.
 * @param iterations The number of iterations to be set.
 *
 * @see ephysics_world_constraint_solver_iterations_get().
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_constraint_solver_iterations_set(EPhysics_World *world, int iterations);

/**
 * @brief
 * Get the number of iterations the constraint solver will have for contact and
 * joint constraints.
 *
 * @param world The world to get number of iterations from.
 * @return the number of iterations set to @p world, or 0 on failure.
 *
 * @see ephysics_world_constraint_solver_iterations_set() for its meaning.
 * @ingroup EPhysics_World
 */
EAPI int ephysics_world_constraint_solver_iterations_get(EPhysics_World *world);

/**
 * @brief
 * Get world gravity values for axis x and y.
 *
 * @param world The world object.
 * @param gx Gravity on x axis.
 * @param gy Gravity on y axis.
 *
 * @see ephysics_world_gravity_set().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_gravity_get(const EPhysics_World *world, double *gx, double *gy);

/**
 * @brief
 * Set rate between pixels on evas canvas and meters on ephysics world.
 *
 * It will be used by automatic updates of evas objects associated to
 * physics bodies.
 *
 * By default its value is 20 Evas coordinates (pixels) per meter.
 *
 * If you change the rate but wants to keep gravity as (0, 9.8 m/s^2),
 * you well need to set world gravity with: 9.8 * new_rate.
 * For this, use @ref ephysics_world_gravity_set();
 *
 * @param world The world object.
 * @param rate Rate between pixels and meters. Value must be > 0.
 *
 * @see ephysics_body_event_callback_add() for more info.
 * @see ephysics_world_rate_get().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_rate_set(EPhysics_World *world, double rate);

/**
 * @brief
 * Get rate between pixels on evas canvas and meters on ephysics world.
 *
 * @param world The world object.
 * @return The rate between pixels and meters.
 *
 * @see ephysics_world_rate_set() for details.
 *
 * @ingroup EPhysics_World
 */
EAPI double ephysics_world_rate_get(const EPhysics_World *world);

/**
 * @brief
 * Gets the world's bodies list.
 *
 * @param world The world object.
 * @return The list of bodies that belongs to this @p world.
 *
 * @see ephysics_body_circle_add().
 * @see ephysics_body_box_add().
 * @see ephysics_body_del().
 *
 * @ingroup EPhysics_World
 */
EAPI const Eina_List *ephysics_world_bodies_get(const EPhysics_World *world);

/**
 * @brief
 * Get the camera used by an ephysics world.
 *
 * @param world The world object.
 * @return The camera.
 *
 * @see ephysics_camera_position_set().
 * @see ephysics_camera_zoom_set().
 *
 * @ingroup EPhysics_World
 */
EAPI EPhysics_Camera *ephysics_world_camera_get(const EPhysics_World *world);

/**
 * @brief
 * Register a callback to a type of physics world event.
 *
 * The registered callback will receives the world and extra user data that
 * can be passed.
 *
 * Regarding EPHYSICS_CALLBACK_WORLD_DEL:
 *
 * Registered callbacks will be called before world is freed.
 * No event_info is passed.
 *
 * @param world The physics world.
 * @param type Type of callback to be listened by @p func.
 * @param func Callback function that will be called when event occurs.
 * @param data User data that will be passed to callback function. It won't
 * be used by ephysics in any way.
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_event_callback_add(EPhysics_World *world, EPhysics_Callback_World_Type type, EPhysics_World_Event_Cb func, const void *data);

/**
 * @brief
 * Unregister an ephysics world event callback.
 *
 * A previously added callback that match @p world, @p type and @p func
 * will be deleted.
 *
 * @param world The physics world.
 * @param type The type of callback to be unregistered.
 * @param func The callback function to be unregistered.
 * @return The user data passed when the callback was registered, or @c NULL
 * on error.
 *
 * @see ephysics_world_event_callback_add() for details.
 * @see ephysics_world_event_callback_del_full() if you need to match data
 * pointer.
 *
 * @ingroup EPhysics_World
 */
EAPI void *ephysics_world_event_callback_del(EPhysics_World *world, EPhysics_Callback_World_Type type, EPhysics_World_Event_Cb func);

/**
 * @brief
 * Unregister an ephysics world event callback matching data pointer.
 *
 * A previously added callback that match @p world, @p type, @p func
 * and @p data will be deleted.
 *
 * @param world The physics world.
 * @param type The type of callback to be unregistered.
 * @param func The callback function to be unregistered.
 * @param data The data pointer that was passed to the callback.
 * @return The user data passed when the callback was registered, or @c NULL
 * on error.
 *
 * @see ephysics_world_event_callback_add() for details.
 * @see ephysics_world_event_callback_del() if you don't need to match data
 * pointer.
 *
 * @ingroup EPhysics_World
 */
EAPI void *ephysics_world_event_callback_del_full(EPhysics_World *world, EPhysics_Callback_World_Type type, EPhysics_World_Event_Cb func, void *data);

/**
 * @brief
 * Set linear slop to be used by world.
 *
 * Constraint solver can be configured using some advanced settings, like
 * the solver slop factor.
 *
 * The default value is set to 0 with a small value results in a smoother
 * stabilization for stacking bodies.
 *
 * Linear slop on sequencial impulse constraint solver is used as a factor
 * for penetration. The penetration will the manifold distance + linear slop.
 *
 * @param world The physics world.
 * @param linear_slop New linear slop value to be used by constraint solver
 * of physics engine.
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_linear_slop_set(EPhysics_World *world, double linear_slop);

/**
 * @brief
 * Get linear slop used by world.
 *
 * @param world The physics world.
 * @return Linear slop value used by constraint solver of physics engine or 0
 * on failure.
 *
 * @see ephysics_world_linear_slop_set() for details.
 *
 * @ingroup EPhysics_World
 */
EAPI double ephysics_world_linear_slop_get(EPhysics_World *world);

/**
 * @brief
 * Set world autodeleting bodies mode when they're outside of render area
 * by the top.
 *
 * It's useful when you don't care about bodies leaving the render
 * area set with @ref ephysics_world_render_geometry_set(), and don't think
 * they could / should return. So you can safely delete them and save resources.
 *
 * Also, it's useful if you have only a bottom border set with
 * @ref ephysics_body_top_boundary_add() and gravity set,
 * and want to listen for @ref EPHYSICS_CALLBACK_WORLD_STOPPED event.
 * If a body goes out of the render area, they will be acting by gravity
 * and won't collide to anything, so they could be moving forever and
 * world would never stop. For this case, enabling autodel for left and right
 * borders seems to be a good idea.
 *
 * @param world The physics world.
 * @param autodel If @c EINA_TRUE delete bodies when they are outside render
 * area, otherwise, don't delete.
 *
 * @see ephysics_world_bodies_outside_top_autodel_get().
 * @see ephysics_world_bodies_outside_bottom_autodel_set().
 * @see ephysics_world_bodies_outside_left_autodel_set().
 * @see ephysics_world_bodies_outside_right_autodel_set().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_bodies_outside_top_autodel_set(EPhysics_World *world, Eina_Bool autodel);

/**
 * @brief
 * Get world autodeleting bodies mode when they're outside of render area by
 * the top.
 *
 * @param world The physics world.
 * @return @c EINA_TRUE if bodies will be deleted or @c EINA_FALSE if they
 * won't, or on error.
 *
 * @see ephysics_world_bodies_outside_top_autodel_set() for details.
 *
 * @ingroup EPhysics_World
 */
EAPI Eina_Bool ephysics_world_bodies_outside_top_autodel_get(EPhysics_World *world);

/**
 * @brief
 * Set world autodeleting bodies mode when they're outside of render area
 * by the bottom.
 *
 * @param world The physics world.
 * @param autodel If @c EINA_TRUE delete bodies when they are outside render
 * area, otherwise, don't delete.
 *
 * @see ephysics_world_bodies_outside_top_autodel_set() for more details.
 * @see ephysics_world_bodies_outside_bottom_autodel_get().
 * @see ephysics_world_bodies_outside_left_autodel_set().
 * @see ephysics_world_bodies_outside_right_autodel_set().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_bodies_outside_bottom_autodel_set(EPhysics_World *world, Eina_Bool autodel);

/**
 * @brief
 * Get world autodeleting bodies mode when they're outside of render area by
 * the bottom.
 *
 * @param world The physics world.
 * @return @c EINA_TRUE if bodies will be deleted or @c EINA_FALSE if they
 * won't, or on error.
 *
 * @see ephysics_world_bodies_outside_bottom_autodel_set() for details.
 *
 * @ingroup EPhysics_World
 */
EAPI Eina_Bool ephysics_world_bodies_outside_bottom_autodel_get(EPhysics_World *world);

/**
 * @brief
 * Set world autodeleting bodies mode when they're outside of render area
 * by the right.
 *
 * @param world The physics world.
 * @param autodel If @c EINA_TRUE delete bodies when they are outside render
 * area, otherwise, don't delete.
 *
 * @see ephysics_world_bodies_outside_top_autodel_set() for more details.
 * @see ephysics_world_bodies_outside_right_autodel_get().
 * @see ephysics_world_bodies_outside_bottom_autodel_set().
 * @see ephysics_world_bodies_outside_left_autodel_set().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_bodies_outside_right_autodel_set(EPhysics_World *world, Eina_Bool autodel);

/**
 * @brief
 * Get world autodeleting bodies mode when they're outside of render area by
 * the right.
 *
 * @param world The physics world.
 * @return @c EINA_TRUE if bodies will be deleted or @c EINA_FALSE if they
 * won't, or on error.
 *
 * @see ephysics_world_bodies_outside_right_autodel_set() for details.
 *
 * @ingroup EPhysics_World
 */
EAPI Eina_Bool ephysics_world_bodies_outside_right_autodel_get(EPhysics_World *world);

/**
 * @brief
 * Set world autodeleting bodies mode when they're outside of render area
 * by the left.
 *
 * @param world The physics world.
 * @param autodel If @c EINA_TRUE delete bodies when they are outside render
 * area, otherwise, don't delete.
 *
 * @see ephysics_world_bodies_outside_top_autodel_set() for more details.
 * @see ephysics_world_bodies_outside_left_autodel_get().
 * @see ephysics_world_bodies_outside_bottom_autodel_set().
 * @see ephysics_world_bodies_outside_right_autodel_set().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_bodies_outside_left_autodel_set(EPhysics_World *world, Eina_Bool autodel);

/**
 * @brief
 * Get world autodeleting bodies mode when they're outside of render area by
 * the left.
 *
 * @param world The physics world.
 * @return @c EINA_TRUE if bodies will be deleted or @c EINA_FALSE if they
 * won't, or on error.
 *
 * @see ephysics_world_bodies_outside_left_autodel_set() for details.
 *
 * @ingroup EPhysics_World
 */
EAPI Eina_Bool ephysics_world_bodies_outside_left_autodel_get(EPhysics_World *world);

/**
 * @}
 */

/**
 * @defgroup EPhysics_Body EPhysics Body
 * @ingroup EPhysics
 *
 * @{
 *
 * A body is a representation of an object inside a physics world.
 *
 * Bodies can have different shapes that can be created with:
 * @li @ref ephysics_body_circle_add();
 * @li or @ref ephysics_body_box_add().
 *
 * They can collide and have customizable properties, like:
 * @li mass, set with @ref ephysics_body_mass_set();
 * @li coefficient of restitution, set with
 * @ref ephysics_body_restitution_set();
 * @li and friction, set with @ref ephysics_body_friction_set().
 *
 * Bodies can have its position and size directly set by
 * @ref ephysics_body_geometry_set().
 *
 * Also, they can have an associated evas object, done with
 * @ref ephysics_body_evas_object_set() function, being responsible for updating
 * its position and rotation, or letting a user callback be set
 * for this task with @ref ephysics_body_event_callback_add().
 *
 * Also, bodies can have impulses applied over them, and will be affected
 * by gravity. Impulses can be applied with:
 * @li @ref ephysics_body_central_impulse_apply();
 * @li @ref ephysics_body_torque_impulse_apply().
 *
 * Bodies can be removed from the world being directly deleted with
 * @ref ephysics_body_del() or when the world is deleted, case when all the
 * bodies belonging to it will be deleted as well. Evas objects associated
 * to these bodies won't be affected in any way, but they will stop being
 * moved or rotated.
 */

typedef struct _EPhysics_Body EPhysics_Body; /**< Body handle, represents an object on EPhysics world. Created with @ref ephysics_body_circle_add() or @ref ephysics_body_box_add() and deleted with @ref ephysics_body_del(). */

/**
 * @enum _EPhysics_Callback_Body_Type
 * @typedef EPhysics_Callback_Body_Type
 *
 * Identifier of callbacks to be set for EPhysics bodies.
 *
 * @see ephysics_body_event_callback_add()
 * @see ephysics_body_event_callback_del()
 * @see ephysics_body_event_callback_del_full()
 *
 * @ingroup EPhysics_Body
 */
typedef enum _EPhysics_Callback_Body_Type
{
   EPHYSICS_CALLBACK_BODY_UPDATE, /**< Body being updated */
   EPHYSICS_CALLBACK_BODY_COLLISION, /**< Body collided with other body */
   EPHYSICS_CALLBACK_BODY_DEL, /**< Body being deleted (called before free) */
   EPHYSICS_CALLBACK_BODY_STOPPED, /**< Body is not moving any more */
   EPHYSICS_CALLBACK_BODY_LAST, /**< kept as sentinel, not really an event */
} EPhysics_Callback_Body_Type; /**< The types of events triggering a callback */


/**
 * @typedef EPhysics_Body_Event_Cb
 *
 * EPhysics bode event callback function signature.
 *
 * Callbacks can be registered for events like body updating or deleting.
 *
 * @param data User data that will be set when registering the callback.
 * @param body Physics body.
 * @param event_info Data specific to a kind of event. Some types of events
 * don't have @p event_info.
 *
 * @see ephysics_body_event_callback_add() for more info.
 *
 * @ingroup EPhysics_Body
 */
typedef void (*EPhysics_Body_Event_Cb)(void *data, EPhysics_Body *body, void *event_info);

/**
 * @brief
 * Create a new circle physics body.
 *
 * Its collision shape will be a circle of diameter 1. To change it's size
 * @ref ephysics_body_geometry_set() should be used, so it can be deformed
 * on x and y axises.
 * Any evas object can be associated to it with
 * @ref ephysics_body_evas_object_set(),
 * and it will collide as a circle (even if you have an evas rectangle).
 *
 * Actually, since we're using a 3D backend, it will be a cylinder on
 * z axis.
 *
 * @param world The world this body will belongs to.
 * @return a new body or @c NULL, on errors.
 *
 * @see ephysics_body_del().
 *
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_Body *ephysics_body_circle_add(EPhysics_World *world);

/**
 * @brief
 * Create a new box physics body.
 *
 * Its collision shape will be a box of dimensions 1 on all the axises.
 * To change it's size @ref ephysics_body_geometry_set() should be used,
 * so it can be deformed on x and y axises.
 *
 * @param world The world this body will belongs to.
 * @return a new body or @c NULL, on errors.
 *
 * @see ephysics_body_del().
 * @see ephysics_body_evas_object_set().
 *
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_Body *ephysics_body_box_add(EPhysics_World *world);

/**
 * @brief
 * Create a physic top boundary.
 *
 * A physic top boundary will limit the bodies area and placed on top edge of
 * worlds render geometry - defined with
 * @ref ephysics_world_render_geometry_set().
 *
 * @param world The world this body will belong to.
 * @return a new body or @c NULL, on erros.
 * @see ephysics_world_render_geometry_set()
 *
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_Body *ephysics_body_top_boundary_add(EPhysics_World *world);

/**
 * @brief
 * Create a physic bottom boundary.
 *
 * A physic bottom boundary will limit the bodies area and placed on bottom
 * edge of worlds render geometry - defined with
 * @ref ephysics_world_render_geometry_set().
 *
 * @param world The world this body will belong to.
 * @return a new body or @c NULL, on erros.
 * @see ephysics_world_render_geometry_set()
 *
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_Body *ephysics_body_bottom_boundary_add(EPhysics_World *world);

/**
 * @brief
 * Create a physic left boundary.
 *
 * A physic left boundary will limit the bodies area and placed right o the
 * left edge of worlds render geometry - defined with
 * @ref ephysics_world_render_geometry_set().
 *
 * @param world The world this body will belong to.
 * @return a new body or @c NULL, on erros.
 * @see ephysics_world_render_geometry_set()
 *
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_Body *ephysics_body_left_boundary_add(EPhysics_World *world);

/**
 * @brief
 * Create a physic right boundary.
 *
 * A physic right boundary will limit the bodies area and placed right o the
 * right edge of worlds render geometry - defined with
 * @ref ephysics_world_render_geometry_set().
 *
 * @param world The world this body will belong to.
 * @return a new body or @c NULL, on erros.
 * @see ephysics_world_render_geometry_set()
 *
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_Body *ephysics_body_right_boundary_add(EPhysics_World *world);

/**
 * @brief
 * Delete a physics body.
 *
 * This function will remove this body from its world and will
 * free all the memory used. It won't delete or modify an associated evas
 * object, what can be done with @ref ephysics_body_evas_object_set(). So after
 * it is removed the evas object will stop being updated, but will continue
 * to be rendered on canvas.
 *
 * @param body The body to be deleted.
 *
 * @see ephysics_body_box_add().
 * @see ephysics_body_circle_add().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_del(EPhysics_Body *body);

/**
 * @brief
 * Get the world a body belongs to.
 *
 * It will return the world where the body was added to.
 *
 * @param body The physics body.
 * @return The world, or @c NULL on error.
 *
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_World *ephysics_body_world_get(const EPhysics_Body *body);

/**
 * @brief
 * Set an evas object to a physics body.
 *
 * It will create a direct association between a physics body and an
 * evas object. With that EPhysics will be able to update evas object
 * position and rotation automatically.
 *
 * This association should be 1:1. You can have physics bodies without evas
 * objects, but you can't have more than an evas object directly associated
 * to this body. If you want more, you can use
 * @ref ephysics_body_event_callback_add() to register a callback that
 * will update the other evas objects. This function can be used to disable
 * updates of associated evas objects, or complement updates, like changing
 * evas objects properties under certain conditions of position or rotation.
 *
 * @param body The body to associate to an evas object.
 * @param evas_obj The evas object that will be associated to this @p body.
 * @param use_obj_pos If @c EINA_TRUE it will set the physics body position
 * to match evas object position taking world rate on consideration.
 *
 * @see ephysics_body_box_add().
 * @see ephysics_body_circle_add().
 * @see ephysics_body_evas_object_unset().
 * @see ephysics_world_rate_set().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_evas_object_set(EPhysics_Body *body, Evas_Object *evas_obj, Eina_Bool use_obj_pos);

/**
 * @brief
 * Unset the evas object associated to a physics body.
 *
 * @param body The body to unset an evas object from.
 * @return The associated evas object, or @c NULL if no object is associated
 * or on error.
 *
 * @see ephysics_body_evas_object_set() for more details.
 *
 * @ingroup EPhysics_Body
 */
EAPI Evas_Object *ephysics_body_evas_object_unset(EPhysics_Body *body);

/**
 * @brief
 * Get the evas object associated to a physics body.
 *
 * @param body The body to get an evas object from.
 * @return The associated evas object, or @c NULL if no object is associated
 * or on error.
 *
 * @see ephysics_body_evas_object_set() for more details.
 *
 * @ingroup EPhysics_Body
 */
EAPI Evas_Object *ephysics_body_evas_object_get(const EPhysics_Body *body);

/**
 * @brief
 * Set physics body geometry.
 *
 * All the physics bodies are created centered on origin (0, 0) and with
 * canonical dimensions. Circles have diameter 1, boxes have dimensions 1
 * on all the axises.
 *
 * There are two direct ways of modifying this geometry:
 * @li With @ref ephysics_body_geometry_set();
 * @li When associating an evas object with
 * @ref ephysics_body_evas_object_set().
 *
 * When the world is simulated forces will be applied on objects
 * with mass and position will be modified too.
 *
 * @note The unit used for geometry is Evas coordinates.
 *
 * @param body The body to be positioned.
 * @param x The position on axis x, in pixels.
 * @param y The position on axis y, in pixels.
 * @param w The body width, in pixels.
 * @param h The body height, in pixels.
 *
 * @see ephysics_body_geometry_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_geometry_set(EPhysics_Body *body, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @brief
 * Get physics body position.
 *
 * @param body The physics body.
 * @param x The position on axis x, in pixels.
 * @param y The position on axis y, in pixels.
 * @param w The body width, in pixels.
 * @param h The body height, in pixels.
 *
 * @see ephysics_body_geometry_set() for more details.
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_geometry_get(const EPhysics_Body *body, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief
 * Set body's mass.
 *
 * It will set inertial mass of the body. It is a quantitative measure of
 * an object's resistance to the change of its speed. It's required to apply
 * more force on objects with more mass to increase its speed.
 *
 * @note The unit used for mass is kilograms.
 *
 * @param body The body to has its mass set.
 * @param mass The @p body's mass, in kilograms.
 *
 * @see ephysics_body_mass_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_mass_set(EPhysics_Body *body, double mass);

/**
 * @brief
 * Get body's mass.
 *
 * It will get inertial mass of the body.
 *
 * @param body The physics body.
 * @return the @p body mass, in kilograms.
 *
 * @see ephysics_body_mass_set() for details.
 *
 * @ingroup EPhysics_Body
 */
EAPI double ephysics_body_mass_get(const EPhysics_Body *body);

/**
 * @brief
 * Set body's linear velocity on x and y axis.
 *
 * @param body The physics body.
 * @param x The linear velocity on axis x.
 * @param y The linear velocity on axis y.
 *
 * @note EPhysics unit for linear velocity is Evas coordinates per second.
 *
 * @see ephysics_body_linear_velocity_get().
 * @see ephysics_body_angular_velocity_set().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_linear_velocity_set(EPhysics_Body *body, double x, double y);

/**
 * @brief
 * Get body's linear velocity on x and y axis.
 *
 * @param body The physics body.
 * @param x The linear velocity on axis x.
 * @param y The linear velocity on axis y.
 *
 * @note EPhysics unit for linear velocity is Evas coordinates per second.
 *
 * @see ephysics_body_linear_velocity_set().
 * @see ephysics_body_angular_velocity_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_linear_velocity_get(const EPhysics_Body *body, double *x, double *y);

/**
 * @brief
 * Set body's angular velocity on z axis.
 *
 * @param body The physics body.
 * @param z The angular velocity on axis z.
 *
 * @note EPhysics unit for angular velocity is degrees per second.
 *
 * @see ephysics_body_angular_velocity_set().
 * @see ephysics_body_linear_velocity_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_angular_velocity_set(EPhysics_Body *body, double z);

/**
 * @brief
 * Get body's angular velocity on z axis.
 *
 * @param body The physics body.
 * @return The angular velocity on axis z, or 0 on error.
 *
 * @note EPhysics unit for angular velocity is degrees per second.
 *
 * @see ephysics_body_linear_velocity_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI double ephysics_body_angular_velocity_get(const EPhysics_Body *body);

/**
 * @brief
 * Stop angular and linear body movement.
 *
 * It's equivalent to set linear velocity to 0 on both axis and
 * angular velocity to 0 as well.
 *
 * It's a momentary situation. If it receives impulse, directly or
 * by collision, if gravity acts over this body,
 * it will stop but it will accelerate again.
 *
 * @param body The physics body.
 *
 * @see ephysics_body_angular_velocity_set().
 * @see ephysics_body_linear_velocity_set().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_stop(EPhysics_Body *body);

/**
 * @brief
 * Update the evas object associated to the body.
 *
 * This function should be called to update position and rotation of
 * the evas object associated to the body with
 * @ref ephysics_body_evas_object_set().
 * It will take rate between pixels and meters set with
 * @ref ephysics_world_rate_set() in account.
 *
 * If an update callback wasn't set with
 * @ref ephysics_body_event_callback_add(), this function will be executed
 * after each physics simulation tick. If a callback was set, it won't be
 * called automatically. So inside this callback it could be called, or
 * a customized update could be implemented.
 *
 * @see ephysics_body_event_callback_add() for more details.
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_evas_object_update(EPhysics_Body *body);

/**
 * @brief
 * Register a callback to a type of physics body event.
 *
 * The registered callback will receives the body and extra user data that
 * can be passed. From body it's possible to get the world it belongs to
 * with @ref ephysics_body_world_get(), the rate between pixels and meters
 * with @ref ephysics_world_rate_get() and the associated evas object with
 * @ref ephysics_body_evas_object_get().
 *
 * So it's enough to do customized updates or fix pointers in your program.
 *
 * Regarding EPHYSICS_CALLBACK_BODY_DEL:
 *
 * Registered callbacks will be called before body is freed.
 * No event_info is passed.
 *
 * Regarding EPHYSICS_CALLBACK_BODY_UPDATE:
 *
 * This update event happens after each physics world tick. Its main use
 * could be updating the evas object associated to a physics body.
 *
 * If no callback is registered, the evas object associated to physics body
 * will be automatically moved and rotated, taking rate between meters and
 * pixels on account. This rate is set by @ref ephysics_world_rate_set().
 *
 * If callbacks are registered, these function will be called and will
 * be responsible for updating the evas object. If the default update
 * is wanted, function @ref ephysics_body_evas_object_update() can be called
 * inside the callback. So you could make changes before and after
 * the evas object is updated.
 *
 * A callback could be something like this:
 * @code
 *   static void
 *   _update_cb(void *data, EPhysics_Body *body, void *event_info)
 *   {
 *     // Something you want to do before updating the evas object
 *     ephysics_body_evas_object_update(body);
 *     // Something to be done after the update, like checking the new position
 *     // of the evas object to change a property.
 *   }
 *
 *  ephysics_body_event_callback_add(body, EPHYSICS_CALLBACK_BODY_UPDATE,
 *                                   _update_cb, NULL);
 * @endcode
 *
 * Update callbacks receives evas object set to body as event_info argument.
 *
 * Regarding EPHYSICS_CALLBACK_BODY_COLLISION:
 *
 * Callbacks are called just after the collision has been actually processed
 * by the physics engine.
 *
 * The other body involved in the collision is passed as event_info argument.
 *
 * @param body The physics body.
 * @param type Type of callback to be listened by @p func.
 * @param func Callback function that will be called when event occurs.
 * @param data User data that will be passed to callback function. It won't
 * be used by ephysics in any way.
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_event_callback_add(EPhysics_Body *body, EPhysics_Callback_Body_Type type, EPhysics_Body_Event_Cb func, const void *data);

/**
 * @brief
 * Unregister an ephysics body event callback.
 *
 * A previously added callback that match @p body, @p type and @p func
 * will be deleted.
 *
 * @param body The physics body.
 * @param type The type of callback to be unregistered.
 * @param func The callback function to be unregistered.
 * @return The user data passed when the callback was registered, or @c NULL
 * on error.
 *
 * @see ephysics_body_event_callback_add() for details.
 * @see ephysics_body_event_callback_del_full() if you need to match data
 * pointer.
 *
 * @ingroup EPhysics_Body
 */
EAPI void *ephysics_body_event_callback_del(EPhysics_Body *body, EPhysics_Callback_Body_Type type, EPhysics_Body_Event_Cb func);

/**
 * @brief
 * Unregister an ephysics body event callback matching data pointer.
 *
 * A previously added callback that match @p body, @p type, @p func
 * and @p data will be deleted.
 *
 * @param body The physics body.
 * @param type The type of callback to be unregistered.
 * @param func The callback function to be unregistered.
 * @param data The data pointer that was passed to the callback.
 * @return The user data passed when the callback was registered, or @c NULL
 * on error.
 *
 * @see ephysics_body_event_callback_add() for details.
 * @see ephysics_body_event_callback_del() if you don't need to match data
 * pointer.
 *
 * @ingroup EPhysics_Body
 */
EAPI void *ephysics_body_event_callback_del_full(EPhysics_Body *body, EPhysics_Callback_Body_Type type, EPhysics_Body_Event_Cb func, void *data);

/**
 * @brief
 * Set body's coefficient of restitution.
 *
 * The coefficient of restitution is proporcion between speed after and
 * before a collision:
 * COR = relative speed after collision / relative speed before collision
 *
 * The body COR is the coefficient of restitution with respect to a perfectly
 * rigid and elastic object. Bodies will collide between them with different
 * behaviors depending on COR:
 * @li they will elastically collide for COR == 1;
 * @li they will inelastically collide for 0 < COR < 1;
 * @li they will completelly stop (no bouncing at all) for COR == 0.
 *
 * By default restitution coefficient of each body is 0.
 *
 * @param body The body to has its restitution coefficient set.
 * @param restitution The new @p body's restitution coefficient.
 *
 * @see ephysics_body_restitution_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_restitution_set(EPhysics_Body *body, double restitution);

/**
 * @brief
 * Get body's restitution.
 *
 * @param body The physics body.
 * @return the @p body's restitution value.
 *
 * @see ephysics_body_restitution_set() for details.
 *
 * @ingroup EPhysics_Body
 */
EAPI double ephysics_body_restitution_get(const EPhysics_Body *body);

/**
 * @brief
 * Set body's friction.
 *
 * Friction is used to make objects slide along each other realistically.
 *
 * The friction parameter is usually set between 0 and 1, but can be any
 * non-negative value. A friction value of 0 turns off friction and a value
 * of 1 makes the friction strong.
 *
 * By default friction value is 0.5 and simulation results will be better
 * when friction in non-zero.
 *
 * @param body The body to has its friction set.
 * @param friction The new @p body's friction value.
 *
 * @see ephysics_body_friction_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_friction_set(EPhysics_Body *body, double friction);

/**
 * @brief
 * Get body's friction.
 *
 * @param body The physics body.
 * @return the @p body's friction value.
 *
 * @see ephysics_body_friction_set() for details.
 *
 * @ingroup EPhysics_Body
 */
EAPI double ephysics_body_friction_get(const EPhysics_Body *body);

/**
 * @brief
 * Apply an impulse on the center of a body.
 *
 * The impulse is equal to the change of momentum of the body.
 * It's the product of the force over the time this force is applied.
 *
 * When a impulse is applied over a body it will has its velocity changed.
 * This impulse will be applied on body's center, so it won't implies in
 * rotating the body. For that is possible to apply a torque impulse with
 * @ref ephysics_body_torque_impulse_apply().
 *
 * @note Impulse is measured in Ns (Newton seconds).
 *
 * @param body The physics body that will receive the impulse.
 * @param x The axis x component of impulse.
 * @param y The axis y component of impulse.
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_central_impulse_apply(EPhysics_Body *body, double x, double y);

/**
 * @brief
 * Apply a torque impulse over a body.
 *
 * An impulse will be applied over the body to make it rotate around Z axis.
 *
 * @param body The physics body that will receive the impulse.
 * @param roll Impulse to rotate body around Z axis (rotate on x - y plane).
 * Negative values will impulse body on anti clock rotation.
 *
 * @see ephysics_body_central_impulse_apply().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_torque_impulse_apply(EPhysics_Body *body, double roll);

/**
 * @brief
 * Enable or disable body's rotation on z axis.
 *
 * Enabled by default.
 *
 * If disabled, body won't rotate on x-y plane.
 *
 * @param body The physics body.
 * @param enable If @c EINA_TRUE enable rotation on z axis, if @c EINA_FALSE
 * disable it.
 *
 * @see ephysics_body_rotation_on_z_axis_enable_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_rotation_on_z_axis_enable_set(EPhysics_Body *body, Eina_Bool enable);

/**
 * @brief
 * Return body's rotation on z axis state.
 *
 * @param body The physics body.
 * @return @c EINA_TRUE if rotation on z axis is enabled, or @c EINA_FALSE
 * if disabled (or on error).
 *
 * @see ephysics_body_rotation_on_z_axis_enable_set() for more details.
 *
 * @ingroup EPhysics_Body
 */
EAPI Eina_Bool ephysics_body_rotation_on_z_axis_enable_get(EPhysics_Body *body);

/**
 * @brief
 * Enable or disable body's movement on x and y axises.
 *
 * Enabled by default on both axises.
 *
 * @param body The physics body.
 * @param enable_x If @c EINA_TRUE allow movement on x axis, if @c EINA_FALSE
 * disallow it.
 * @param enable_y If @c EINA_TRUE allow movement on y axis, if @c EINA_FALSE
 * disallow it.
 *
 * @see ephysics_body_linear_movement_enable_set().
 * @see ephysics_body_rotation_on_z_axis_enable_set().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_linear_movement_enable_set(EPhysics_Body *body, Eina_Bool enable_x, Eina_Bool enable_y);

/**
 * @brief
 * Get body's movement on x and y axises behavior.
 *
 * @param body The physics body.
 * @param enable_x @c EINA_TRUE if movement on x axis is allowed, or
 * @c EINA_FALSE if it's not.
 * @param enable_y @c EINA_TRUE if movement on y axis is allowed, or
 * @c EINA_FALSE if it's not.
 *
 * @see ephysics_body_linear_movement_enable_get().
 * @see ephysics_body_rotation_on_z_axis_enable_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_linear_movement_enable_get(EPhysics_Body *body, Eina_Bool *enable_x, Eina_Bool *enable_y);

/**
 * @brief
 * Return body's rotation on z axis.
 *
 * By default rotation is 0 degrees.
 *
 * @note The unit used for rotation is degrees.
 *
 * @param body The physics body.
 * @return The amount of degrees @p body is rotated, from 0.0 to 360.0.
 *
 * @ingroup EPhysics_Body
 */
EAPI double ephysics_body_rotation_get(EPhysics_Body *body);

/**
 * @}
 */

/**
 * @defgroup EPhysics_Constraint EPhysics Constraint
 * @ingroup EPhysics
 *
 * @{
 *
 * Constraints can be used to limit bodies movements, between bodies or
 * between bodies and the world. Constraints can limit movement angle,
 * translation, or work like a motor.
 *
 * Constraints can be created with @ref ephysics_constraint_add() and removed
 * with @ref ephysics_constraint_del().
 * Can be applied between two bodies or between a body and the world.
 */

typedef struct _EPhysics_Constraint EPhysics_Constraint; /**< Constraint handle, used to limit bodies movements. Created with @ref ephysics_constraint_add() and deleted with @ref ephysics_constraint_del(). */

/**
 * @brief
 * Create a new constraint between 2 bodies.
 *
 * The constraint will join two bodies(@p body1 and @p body2) limiting their
 * movements based on specified anchors.
 *
 * @param body1 The first body to apply the constraint.
 * @param body2 The second body to apply the constraint.
 * @param anchor_b1_x The first body X anchor.
 * @param anchor_b1_y The fist body Y anchor.
 * @param anchor_b2_x The second body X anchor.
 * @param anchor_b2_y The second body Y anchor.
 * @return A new constraint or @c NULL, on errors.
 *
 * @see ephysics_constraint_del().
 *
 * @ingroup EPhysics_Constraint
 */
EAPI EPhysics_Constraint *ephysics_constraint_add(EPhysics_Body *body1, EPhysics_Body *body2, Evas_Coord anchor_b1_x, Evas_Coord anchor_b1_y, Evas_Coord anchor_b2_x, Evas_Coord anchor_b2_y);

/**
 * @brief
 * Deletes a physics constraint.
 *
 * @param constraint The constraint to be deleted.
 *
 * @see ephysics_constraint_add() for more details.
 *
 * @ingroup EPhysics_Constraint
 */
EAPI void ephysics_constraint_del(EPhysics_Constraint *constraint);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif
