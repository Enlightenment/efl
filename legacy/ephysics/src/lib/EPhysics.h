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
 * @li @ref EPhysics_Shape
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
 * @defgroup EPhysics_Shape EPhysics Shape
 * @ingroup EPhysics
 *
 * @{
 *
 * Shapes are used to create bodies with shapes that differ from primitive
 * ones, like box and circle.
 *
 * A shape consists in a group of points, the vertices of the body to be
 * created later with @ref ephysics_body_shape_add().
 *
 * A new shape is created with @ref ephysics_shape_new() and points are
 * set with @ref ephysics_shape_point_add(). A shape can be used to
 * create many bodies. When done, it's required to delete the shape
 * with @ref ephysics_shape_del().
 *
 * A shape can be load from a file describing it with
 * @ref ephysics_shape_load(), and can be saved to a file with
 * @ref ephysics_shape_save(). With that shapes can be done or visualized
 * on design applications.
 *
 * @note Using primitive shapes has better perfomance than generic shapes.
 * @note For now, only convex shapes are supported.
 *
 */

/**
 * @typedef EPhysics_Shape
 *
 * Shape handle, represents a shape to be used to create a body.
 *
 * Created with @ref ephysics_shape_new() and deleted with
 * @ref ephysics_shape_del().
 *
 * @ingroup EPhysics_Shape
 */
typedef struct _EPhysics_Shape EPhysics_Shape;

/**
 * @brief
 * Create a new shape.
 *
 * The returned shape initially doesn't has points set, so it's required
 * to set vertices with @ref ephysics_shape_point_add().
 *
 * After the shape is completelly defined, all the points were added,
 * it's possible to create one or more bodies with
 * @ref ephysics_body_shape_add().
 *
 * @return The created shape or @c NULL on error.
 *
 * @see ephysics_shape_del().
 * @see ephysics_shape_load().
 *
 * @ingroup EPhysics_Shape
 */
EAPI EPhysics_Shape *ephysics_shape_new(void);

/**
 * @brief
 * Delete a shape.
 *
 * After a shape is used to create the wanted bodies, it's required
 * to delete it. It won't be deleted automatically by ephysics
 * at any point, even on shutdown. The creator is responsible to
 * free it after usage is concluded.
 *
 * @param shape The shape to be deleted.
 *
 * @see ephysics_shape_new().
 *
 * @ingroup EPhysics_Shape
 */
EAPI void ephysics_shape_del(EPhysics_Shape *shape);

/**
 * @brief
 * Add a new point to the shape.
 *
 * Any point can be added to a shape, but only vertices matter.
 * A vertex is a special kind of point that describes a corner of
 * geometric shapes. The final shape will be constructed in such a way
 * it will have all the added points and will be convex.
 *
 * The order of points doesn't matter.
 *
 * For example, to create a pentagon:
 *
 * @code
 * EPhysics_Shape *shape = ephysics_shape_new();
 *
 * ephysics_shape_point_add(shape, 0/70., 24/66.);
 * ephysics_shape_point_add(shape, 35/70., 0/66.);
 * ephysics_shape_point_add(shape, 70/70., 24/66.);
 * ephysics_shape_point_add(shape, 56/70., 66/66.);
 * ephysics_shape_point_add(shape, 14/70., 66/66.);
 *
 * ephysics_body_shape_add(world, shape);
 *
 * ephysics_shape_del(shape);
 * @endcode
 *
 * @param shape The shape to be modified.
 * @param x Point position at x axis. Should be a value between 0 and 1.
 * @param y Point position at y axis. Should be a value between 0 and 1.
 * @return @c EINA_TRUE on success or EINA_FALSE on error.
 *
 * @see ephysics_shape_new().
 *
 * @ingroup EPhysics_Shape
 */
EAPI Eina_Bool ephysics_shape_point_add(EPhysics_Shape *shape, double x, double y);

/**
 * @brief
 * Load the shape from a file.
 *
 * Useful to edit shapes on design tools and load it from an exported file.
 *
 * Also it helps to avoid lots of @ref ephysics_shape_point_add() in
 * the code, and keep a better separation between code logic and
 * design stuff.
 *
 * @param filename The path to the file describing the shape.
 * @return The loaded shape or @c NULL on error.
 *
 * @note Not implemented yet.
 *
 * @see ephysics_shape_new() for more details.
 * @see ephysics_shape_save().
 *
 * @ingroup EPhysics_Shape
 */
EAPI EPhysics_Shape *ephysics_shape_load(const char *filename);

/**
 * @brief
 * Save the shape to a file.
 *
 * It can be useful to visualize it on design tools.
 *
 * @param shape The shape to be saved.
 * @param filename The path to save the shape.
 * @return @c EINA_TRUE on success or EINA_FALSE on error.
 *
 * @note Not implemented yet.
 *
 * @see ephysics_shape_new().
 * @see ephysics_shape_load().
 *
 * @ingroup EPhysics_Shape
 */
EAPI Eina_Bool ephysics_shape_save(const EPhysics_Shape *shape, const char *filename);

/**
 * @}
 */


/**
 * @typedef EPhysics_Body
 *
 * Body handle, represents an object on EPhysics world.
 *
 * Created with @ref ephysics_body_circle_add() or @ref ephysics_body_box_add()
 * and deleted with @ref ephysics_body_del().
 *
 * @ingroup EPhysics_Body
 */
typedef struct _EPhysics_Body EPhysics_Body;

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
 * Its position can be set with @ref ephysics_camera_position_set() or
 * can be set to track a body, with @ref ephysics_camera_body_track();
 *
 */

typedef struct _EPhysics_Camera EPhysics_Camera; /**< Camera handle, used to change the position of the frame to be rendered. Every world have a camera that can be get with @ref ephysics_world_camera_get(). */

/**
 * @brief
 * Set camera's position.
 *
 * Camera's position referes to the position of the top-left point of the
 * camera.
 *
 * By default a camera is created to map the first quadrant of physics
 * world from the point (0, 0) to
 * (render area width / world rate, render area height / world rate).
 *
 * When render area is set with @ref ephysics_world_render_geometry_set(),
 * the camera geometry is updated to match it. So, for most cases, camera
 * won't need to be handled by the user.
 *
 * But it can be modified passing another top-left point position, so another
 * region of the physics world will be rendered on the render area.
 * So if you have a scene larger than the render area, camera handling can
 * be very useful.
 *
 * This function will make camera stop tracking a body set with
 * @ref ephysics_camera_body_track().
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
EAPI void ephysics_camera_position_set(EPhysics_Camera *camera, Evas_Coord x, Evas_Coord y);

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
EAPI void ephysics_camera_position_get(const EPhysics_Camera *camera, Evas_Coord *x, Evas_Coord *y);

/**
 * @brief
 * Set camera to track a body.
 *
 * When a body is tracked, the camera will move automatically, following
 * this body. It will keeps the body centralized on rendered area.
 * If it will be centralized horizontally and / or vertically depends
 * if parameters @p horizontal and @p vertical are set to @c EINA_TRUE.
 *
 * Default updates (@ref ephysics_body_evas_object_update())
 * will take care of updating evas objects associated
 * to the bodies correctly. But if you need to do it yourself, you'll need
 * to take camera's position in consideration, using
 * @ref ephysics_camera_position_get().
 *
 * @note This change will be noticed on the next physics tick, so evas objects
 * will be updated taking the camera's new position in account.
 *
 * @param camera The world's camera.
 * @param body The body tracked by the @p camera, or @c NULL if camera isn't
 * tracking any body.
 * @param horizontal @c EINA_TRUE if @p body is tracked on x axis,
 * @c EINA_FALSE otherwise;
 * @param vertical @c EINA_TRUE if @p body is tracked on y axis,
 * @c EINA_FALSE otherwise;
 *
 * @see ephysics_camera_tracked_body_get().
 * @see ephysics_camera_position_set().
 * @see ephysics_world_camera_get().
 *
 * @ingroup EPhysics_Camera
 */
EAPI void ephysics_camera_body_track(EPhysics_Camera *camera, EPhysics_Body *body, Eina_Bool horizontal, Eina_Bool vertical);

/**
 * @brief
 * Get body tracked by camera.
 *
 * @param camera The world's camera.
 * @param body The body tracked by the @p camera, or @c NULL if camera isn't
 * tracking any body.
 * @param horizontal @c EINA_TRUE if @p body is tracked on x axis,
 * @c EINA_FALSE otherwise;
 * @param vertical @c EINA_TRUE if @p body is tracked on y axis,
 * @c EINA_FALSE otherwise;
 *
 * @see ephysics_camera_body_track() for more details.
 *
 * @ingroup EPhysics_Camera
 */
EAPI void ephysics_camera_tracked_body_get(EPhysics_Camera *camera, EPhysics_Body **body, Eina_Bool *horizontal, Eina_Bool *vertical);

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
   EPHYSICS_CALLBACK_WORLD_CAMERA_MOVED, /**< camera position changed */
   EPHYSICS_CALLBACK_WORLD_LAST, /**< kept as sentinel, not really an event */
} EPhysics_Callback_World_Type;

/**
 * @enum _EPhysics_World_Constraint_Solver_Mode
 * typedef EPhysics_World_Constraint_Solver_Mode
 *
 * Identifies the worlds contact and joint constraint solver mode. By default
 * EPHYSICS_WORLD_SOLVER_USE_WARMSTARTING is the only enabled solver mode.
 *
 * @see ephysics_world_constraint_solver_mode_enable_set()
 * @see ephysics_world_constraint_solver_mode_enable_get()
 *
 * @ingroup EPhysics_World
 */
typedef enum _EPhysics_World_Constraint_Solver_Mode
{
   EPHYSICS_WORLD_SOLVER_RANDMIZE_ORDER = 1, /**< Randomize the order of solving the constraint rows*/
   EPHYSICS_WORLD_SOLVER_USE_WARMSTARTING = 4, /**< The PGS is an iterative algorithm where each iteration is based on the solution of previous iteration, if no warmstarting is used, the initial solution for PGS is set to zero each frame (by default this mode is enabled, disabling this mode the user can face a better performance depending on the amount of objects in the simulation)*/
   EPHYSICS_WORLD_SOLVER_USE_2_FRICTION_DIRECTIONS = 16, /**< While calculating a friction impulse consider this should be applied on both bodies (this mode cause a better stacking stabilization)*/
   EPHYSICS_WORLD_SOLVER_SIMD = 256, /**< Use a SSE optimized innerloop, using assembly intrinsics, this is implemented and can be enabled/disabled for Windows and Mac OSX versions, single-precision floating point, 32bit(disabled by default)*/
} EPhysics_World_Solver_Mode;

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
EAPI void ephysics_world_render_geometry_get(const EPhysics_World *world, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

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
 * Set the max sleeping time value.
 *
 * This value determines how long(in seconds) a rigid body under the linear and
 * angular threshold is supposed to be marked as sleeping. Default value is set
 * to 2.0.
 *
 * @param world The world to set the max sleeping time.
 * @param sleeping_time The max sleeping time to set to @p world.
 *
 * @see ephysics_world_max_sleeping_time_get()
 * @see ephysics_body_sleeping_threshold_set() for sleeping thresholds details.
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_max_sleeping_time_set(EPhysics_World *world, double sleeping_time);

/**
 * @brief
 * Get the max sleeping time value for @p world.
 *
 * @param world The world to get the max sleeping time from.
 * @return The max sleeping time from @p world.
 *
 * @see ephysics_world_max_sleeping_time_set()
 * @ingroup EPhysics_World
 */
EAPI double ephysics_world_max_sleeping_time_get(const EPhysics_World *world);

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
EAPI int ephysics_world_constraint_solver_iterations_get(const EPhysics_World *world);

/**
 * @brief
 * Enable or disable a constraint solver mode to @p world. A world can operate
 * on several constraint solver modes.
 *
 * @param world The world to be set.
 * @param solver_mode The solver mode to set.
 * @param enable If @c EINA_TRUE enable the mode, if EINA_FALSE, disable it.
 *
 * @see EPhysics_World_Solver_Mode for supported solver modes.
 * @see ephysics_world_constraint_solver_mode_enable_get()
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_constraint_solver_mode_enable_set(EPhysics_World *world, EPhysics_World_Solver_Mode solver_mode, Eina_Bool enable);

/**
 * @brief
 * Get the @p solver_mode status on @p world.
 *
 * @param world The world to be queried.
 * @param solver_mode The solver mode of interest.
 * @return EINA_TRUE if @p solver_mode is enabled, EINA_FALSE otherwise.
 *
 * @see ephysics_world_constraint_solver_mode_enable_set()
 * @ingroup EPhysics_World
 */
EAPI Eina_Bool ephysics_world_constraint_solver_mode_enable_get(const EPhysics_World *world, EPhysics_World_Solver_Mode solver_mode);

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
 * @note The list should be freed after usage.
 *
 * @see ephysics_body_circle_add().
 * @see ephysics_body_box_add().
 * @see ephysics_body_del().
 *
 * @ingroup EPhysics_World
 */
EAPI Eina_List *ephysics_world_bodies_get(const EPhysics_World *world);

/**
 * @brief
 * Get the camera used by an ephysics world.
 *
 * @param world The world object.
 * @return The camera.
 *
 * @see ephysics_camera_position_set().
 * @see ephysics_camera_body_track().
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
 * What follows is a list of details about each callback type:
 *
 *  - #EPHYSICS_CALLBACK_WORLD_DEL: Called just before a world is freed.
 * No event_info is passed to the callback.
 *
 *  - #EPHYSICS_CALLBACK_WORLD_STOPPED: Called when all the bodies
 * are stopped. No event_info is passed to the callback.
 *
 *  - #EPHYSICS_CALLBACK_WORLD_CAMERA_MOVED: Called if camera position changed
 * on physics simulation tick. Camera is passed as event_info to the callback.
 * Useful to move backgrounds, since objects would be already updated
 * considering camera's position.
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
EAPI double ephysics_world_linear_slop_get(const EPhysics_World *world);

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
EAPI Eina_Bool ephysics_world_bodies_outside_top_autodel_get(const EPhysics_World *world);

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
EAPI Eina_Bool ephysics_world_bodies_outside_bottom_autodel_get(const EPhysics_World *world);

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
EAPI Eina_Bool ephysics_world_bodies_outside_right_autodel_get(const EPhysics_World *world);

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
EAPI Eina_Bool ephysics_world_bodies_outside_left_autodel_get(const EPhysics_World *world);

/**
 * @brief
 * Set world simulation's fixed time step and max number of sub steps
 * configuration.
 *
 * It's important that time step is always less than
 * @p max_sub_steps * @p fixed_time_step, otherwise you are losing time.
 * Mathematically:
 *
 * time step < @p max_sub_steps * @p fixed_time_step;
 *
 * If you're a using a very large time step
 * [say, five times the size of the fixed internal time step],
 * then you must increase the number of max sub steps to compensate for this,
 * otherwise your simulation is “losing” time.
 *
 * The time step may vary. Simulation ticks are called by an animator,
 * so, by default, time step is @c 1/30 secs. If you're using elementary,
 * default FPS configuration is 60 fps, i.e. time step is @c 1/60 secs.
 * You can change that setting a
 * different time with ecore_animator_frametime_set().
 *
 * Also, keep in mind
 * that if you're using CPU intense calculations maybe this framerate won't
 * be achieved, so the time step will be bigger. You need to define
 * what range of frames per secons you need to support and configure
 * @p max_sub_steps and @p fixed_time_step according to this.
 *
 * By decreasing the size of @p fixed_time_step, you are increasing the
 * “resolution” of the simulation.
 *
 * If you are finding that your objects are moving very fast and escaping
 * from your walls instead of colliding with them, then one way to help fix
 * this problem is by decreasing @p fixed_time_step. If you do this,
 * then you will need to increase @p max_sub_steps to ensure the equation
 * listed above is still satisfied.
 *
 * The issue with this is that each internal “tick” takes an amount of
 * computation. More of them means your CPU will be spending more time on
 * physics and therefore less time on other stuff. Say you want twice the
 * resolution, you'll need twice the @p max_sub_steps, which could chew up
 * twice as much CPU for the same amount of simulation time.
 *
 * When you pass @p max_sub_steps > 1, it will interpolate movement for you.
 * This means that if your @p fixed_time_step is 3 units, and you pass
 * a timeStep of 4, then it will do exactly one tick, and estimate the
 * remaining movement by 1/3. This saves you having to do interpolation
 * yourself, but keep in mind that maxSubSteps needs to be greater than 1.
 *
 * By default @p fixed_time_step is 1/60 seconds and @p max_sub_steps is 3.
 *
 * @param world The physics world.
 * @param fixed_time_step size of the internal simulation step, in seconds.
 * @param max_sub_steps maximum number of steps that simulation is allowed
 * to take at each simulation tick.
 *
 * @note The unit used for time is seconds.
 *
 * @see ephysics_world_simulation_get().
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_simulation_set(EPhysics_World *world, double fixed_time_step, int max_sub_steps);

/**
 * @brief
 * Get world simulation's fixed time step and max number of sub steps
 * configuration.
 *
 * @param world The physics world.
 * @param fixed_time_step size of the internal simulation step, in seconds.
 * @param max_sub_steps maximum number of steps that simulation is allowed
 * to take at each simulation tick.
 *
 * @see ephysics_world_simulation_set() for details.
 *
 * @ingroup EPhysics_World
 */
EAPI void ephysics_world_simulation_get(const EPhysics_World *world, double *fixed_time_step, int *max_sub_steps);

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
 * @li @ref ephysics_body_box_add();
 * @li or @ref ephysics_body_shape_add().
 *
 * They can collide and have customizable properties, like:
 * @li mass, set with @ref ephysics_body_mass_set();
 * @li coefficient of restitution, set with
 * @ref ephysics_body_restitution_set();
 * @li and friction, set with @ref ephysics_body_friction_set().
 *
 * Bodies can have its position and size directly set by:
 * @ref ephysics_body_move();
 * @ref ephysics_body_resize();
 * @ref ephysics_body_geometry_set().
 *
 * Also, they can have an associated evas object, done with
 * @ref ephysics_body_evas_object_set() function, being responsible for
 * updating its position and rotation, or letting a user callback be set
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

/**
 * @typedef EPhysics_Body_Collision
 *
 * Body collision wraps collision informations.
 *
 * EPhysics_Body_Collision is used on EPHYSICS_CALLBACK_BODY_COLLISION callback
 * and is mostly interested to hold informations like:
 * @li contact_body - the body which the collision occurred against;
 * @li position - points the position where the collision happened;
 *
 * @see ephysics_body_collision_position_get()
 * @see ephysics_body_collision_contact_body_get()
 * @see EPHYSICS_CALLBACK_BODY_COLLISION and @ref
 * ephysics_body_event_callback_add() for collision callback.
 * @ingroup EPhysics_Body
 */
typedef struct _EPhysics_Body_Collision EPhysics_Body_Collision;

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
 * EPhysics body event callback function signature.
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
 * Create a new physics body using a custom shape.
 *
 * Its collision shape will be a convex shape that has all the points
 * added to this @p shape. A shape can be created with
 * @ref ephysics_shape_new().
 *
 * To change it's size @ref ephysics_body_geometry_set() should be used,
 * so it can be deformed on x and y axises.
 *
 * @param world The world this body will belongs to.
 * @param shape The custom shape to be used.
 * @return a new body or @c NULL, on errors.
 *
 * @see ephysics_body_del().
 * @see ephysics_body_evas_object_set().
 *
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_Body *ephysics_body_shape_add(EPhysics_World *world, EPhysics_Shape *shape);

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
 * Set physics body size.
 *
 * All the physics bodies are and with canonical dimensions.
 * Circles have diameter 1, boxes have dimensions 1
 * on all the axises.
 *
 * There are three direct ways of modifying it's size:
 * @li With @ref ephysics_body_resize();
 * @li With @ref ephysics_body_geometry_set();
 * @li When associating an evas object with
 * @ref ephysics_body_evas_object_set().
 *
 * @note The unit used for size is Evas coordinates.
 *
 * @param body The body to be resized.
 * @param w The body width, in pixels.
 * @param h The body height, in pixels.
 *
 * @see ephysics_body_geometry_get().
 * @see ephysics_body_geometry_set().
 * @see ephysics_body_move().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_resize(EPhysics_Body *body, Evas_Coord w, Evas_Coord h);

/**
 * @brief
 * Set physics body position.
 *
 * All the physics bodies are created centered on origin (0, 0).
 *
 * There are three direct ways of modifying this position:
 * @li With @ref ephysics_body_move();
 * @li With @ref ephysics_body_geometry_set();
 * @li When associating an evas object with
 * @ref ephysics_body_evas_object_set().
 *
 * When the world is simulated forces will be applied on objects
 * with mass and position will be modified too.
 *
 * @note The unit used for position is Evas coordinates.
 *
 * @param body The body to be positioned.
 * @param x The position on axis x, in pixels.
 * @param y The position on axis y, in pixels.
 *
 * @see ephysics_body_geometry_get().
 * @see ephysics_body_geometry_set().
 * @see ephysics_body_resize().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_move(EPhysics_Body *body, Evas_Coord x, Evas_Coord y);

/**
 * @brief
 * Set physics body geometry.
 *
 * All the physics bodies are created centered on origin (0, 0) and with
 * canonical dimensions. Circles have diameter 1, boxes have dimensions 1
 * on all the axises.
 *
 * There are four direct ways of modifying this geometry:
 * @li With @ref ephysics_body_geometry_set();
 * @li With @ref ephysics_body_move();
 * @li With @ref ephysics_body_resize();
 * @li When associating an evas object with
 * @ref ephysics_body_evas_object_set().
 *
 * If you just need to move the body, no changes are required to it's size,
 * use @ref ephysics_body_move(). If you just need to modify the size of
 * @p body use @ref ephysics_body_resize().
 *
 * When the world is simulated forces will be applied on objects
 * with mass and position will be modified too.
 *
 * @note The unit used for geometry is Evas coordinates.
 *
 * @param body The body to be modified.
 * @param x The position on axis x, in pixels.
 * @param y The position on axis y, in pixels.
 * @param w The body width, in pixels.
 * @param h The body height, in pixels.
 *
 * @see ephysics_body_geometry_get().
 * @see ephysics_body_move().
 * @see ephysics_body_resize().
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
 * @see ephysics_body_move().
 * @see ephysics_body_resize().
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
 * @see ephysics_body_angular_velocity_get().
 * @see ephysics_body_linear_velocity_set().
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
 * @see ephysics_body_angular_velocity_set().
 * @see ephysics_body_linear_velocity_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI double ephysics_body_angular_velocity_get(const EPhysics_Body *body);

/**
 * @brief
 * Set the linear and angular sleeping threshold.
 *
 * These factors are used to determine whenever a rigid body is supposed to
 * increment the sleeping time.
 *
 * After every tick the sleeping time is incremented, if the body's linear and
 * angular speed is less than the respective thresholds the sleeping time is
 * incremented by the current time step (delta time).
 *
 * After reaching the max sleeping time the body is marked to sleep, that means
 * the rigid body is to be deactivated.
 *
 * @note The expected value to be informed as @p linear_threshold is
 * the body's speed. It is measured in Evas coordinates per second.
 *
 * @note The expected angular velocity to be informed as @p angular_threshold
 * is measured in degrees per second.
 *
 * @param body The body to be set.
 * @param linear_threshold The linear sleeping threshold factor.
 * @param angular_threshold The angular sleeping threshold factor.
 *
 * @see ephysics_body_sleeping_threshold_get().
 * @see ephysics_world_max_sleeping_time_set() for sleeping time details.
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_sleeping_threshold_set(EPhysics_Body *body, double linear_threshold, double angular_threshold);

/**
 * @brief
 * Get the linear sleeping threshold.
 *
 * @note The linear sleeping threshold is measured in Evas coordinates per
 * second and the angular sleeping threshold is measured in degrees per second.
 *
 * @param body The body to get the linear sleeping threshold from.
 * @param linear_threshold The linear sleeping threshold factor.
 * @param angular_threshold The angular sleeping threshold factor.
 *
 * @see ephysics_body_sleeping_threshold_set() for more details.
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_sleeping_threshold_get(const EPhysics_Body *body, double *linear_threshold, double *angular_threshold);

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
 * Set the angular and linear damping values.
 *
 * Damping(linear and angular) values are applied to body's linear and angular
 * velocity.
 *
 * By applying a bodies damping factor the user will face a velocity reduction,
 * with a force applied to it - like air resistance. The force is applied to
 * slow it down. Different factors can be applied to angular and linear
 * velocity to fine tune translation and rotation.
 *
 * The damping is a force synchronous with the velocity of the object but in
 * opposite direction.
 *
 * Damping is specified as a value between 0 and 1, which is the proportion of
 * velocity lost per second. If specified damping value - either for angular or
 * linear - is greater or equal to 1, velocities are nulled(that means, will
 * become 0 on every axis), resulting on a frozen body.
 *
 * Default linear damping is set to 0. The same is true for angular damping
 * factor.
 *
 * @param body The physics body.
 * @param linear_damping The linear damping factor to apply on @p body.
 * @param angular_damping The angular damping factor to apply on @p body.
 *
 * @see ephysics_body_damping_get().
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_damping_set(EPhysics_Body *body, double linear_damping, double angular_damping);

/**
 * @brief
 * Get the angular and linear damping values.
 *
 * Damping(linear and angular) values are applied to body's linear and angular
 * velocity.
 *
 * @param body The physics body.
 * @param linear_damping The linear damping factor applied over @p body.
 * @param angular_damping The angular damping factor applied over @p body.
 *
 * @see ephysics_body_damping_set() for details.
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_damping_get(const EPhysics_Body *body, double *linear_damping, double *angular_damping);

/**
 * @brief
 * Add a @p body to a given collision group.
 *
 * After calling this function the body is said to be added to collision @p
 * group.
 *
 * If not added to any group the body will collide against any other body.
 * Otherwise this body will collide only against those in the same groups.
 *
 * If @p body was already part of @p group, nothing will happen.
 *
 * @param body The body to be added to @p group.
 * @param group The group the @p body will belong to.
 * @return @c EINA_TRUE if body is added to group, or @c EINA_FALSE on error.
 *
 * @see ephysics_body_collision_group_del()
 * @see ephysics_body_collision_group_list_get()
 * @ingroup EPhysics_Body
 */
EAPI Eina_Bool ephysics_body_collision_group_add(EPhysics_Body *body, const char *group);

/**
 * @brief
 * Removes @p body from collision @p group.
 *
 * This @p body will not belong to @p group any more and the collisions filter
 * must take that on account.
 *
 * If @p body wasn't part of @p group before, nothing will happen.
 *
 * @param body The body to be removed from @p group.
 * @param group The group @p body must be removed from.
 * @return @c EINA_TRUE if body is removed from group, or @c EINA_FALSE on
 * error.
 *
 * @see ephysics_body_collision_group_add()
 * @ingroup EPhysics_Body
 */
EAPI Eina_Bool ephysics_body_collision_group_del(EPhysics_Body *body, const char *group);

/**
 * @brief
 * Get the collision group list of @p body.
 *
 * @param body The body of interest.
 * @return The collision group list of @p body, NULL on failure or case no
 * group has been added to @p body.
 *
 * @warning The collision group list is an EPhysics internal data structure and
 * should @b never be modified by its callers.
 *
 * @see ephysics_body_collision_group_add()
 * @ingroup EPhysics_Body
 */
EAPI const Eina_List *ephysics_body_collision_group_list_get(const EPhysics_Body *body);

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
 * What follows is a list of details about each callback type:
 *
 * - #EPHYSICS_CALLBACK_BODY_UPDATE: Called after every physics iteration. @p
 *   body points to the EPhysics_Body itself and @p event_info points to the
 *   evas object associated to the body.
 *
 * - #EPHYSICS_CALLBACK_BODY_COLLISION: Called just after the collision has
 *   been actually processed by the physics engine. The body involved in the
 *   collision is passed as @p body argument. @p event_info is a pointer to
 *   @ref EPhysics_Body_Collision - note, this structure(@p event_info) is
 *   discarded/freed right after callback returns.
 *
 * - #EPHYSICS_CALLBACK_BODY_DEL: Called when a body deletion has been issued
 *   and just before the deletion actually happens. @p body points to the body
 *   being deleted and no @p event_info is provided.
 *
 * - #EPHYSICS_CALLBACK_BODY_STOPPED: Called when a body is found to be
 *   stopped. @p body points to the body of interest and @p event_info is a
 *   pointer to the evas object associated to it.
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
 * Get the position(x, y) of a body's collision.
 *
 * Given a body collision data, fills @p x and @p y pointers with the position
 * where the collision occurred.
 *
 * @param collision The body collision data of interest.
 * @param x The x pointer to set the x coordinate to.
 * @param y The y pointer to set the y coordinate to.
 *
 * @see EPHYSICS_CALLBACK_BODY_COLLISION and @ref
 * ephysics_body_event_callback_add() for collision callback.
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_collision_position_get(const EPhysics_Body_Collision *collision, Evas_Coord *x, Evas_Coord *y);

/**
 * @brief
 * Get the body's collision contact body.
 *
 * Given a body collision data returns the contact body which a collision
 * occurred against.
 *
 * @param collision The body collision of interest.
 * @return The contact body of @p collision.
 *
 * @see EPHYSICS_CALLBACK_BODY_COLLISION and @ref
 * ephysics_body_event_callback_add() for collision callback.
 * @ingroup EPhysics_Body
 */
EAPI EPhysics_Body *ephysics_body_collision_contact_body_get(const EPhysics_Body_Collision *collision);

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
 *
 * Impulse is the product of the force over the time this force is applied.
 * In ephysics case, it would be the time of a tick, so it behaves just
 * summing current linear velocity to impulse per mass.
 *
 * Example:
 * A body of 2kg of mass has an initial velocity of 30 p/s.
 * After a impulse of 30 kg * p / s in the same direction is applied,
 * the velocity will be * 45 p/s.
 *
 *    (0, 30) + (0, 300) / 2 = (0, 30) + (0, 15) = (0, 45)
 *
 * When a impulse is applied over a body, it will has its velocity changed.
 * This impulse will be applied on body's center, so it won't implies in
 * rotating the body. For that is possible to apply a torque impulse with
 * @ref ephysics_body_torque_impulse_apply().
 *
 * @note Impulse is measured in kg * p / s.
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
EAPI Eina_Bool ephysics_body_rotation_on_z_axis_enable_get(const EPhysics_Body *body);

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
EAPI void ephysics_body_linear_movement_enable_get(const EPhysics_Body *body, Eina_Bool *enable_x, Eina_Bool *enable_y);

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
EAPI double ephysics_body_rotation_get(const EPhysics_Body *body);

/**
 * @brief
 * Set data to @p body.
 *
 * If a previous data was set, it's reference will be lost and body
 * will point to the new data.
 *
 * It can be useful when you need to store a structure per body. For example,
 * some values that must to be updated when a collision occurs between two
 * bodies.
 *
 * @note EPhysics won't handle this data, it won't be used in any way
 * by the library. If it need to be freed when the body is deleted, a
 * callback for @ref EPHYSICS_CALLBACK_BODY_DEL can be added and
 * data should be explicity freed.
 *
 * @param body The physics body.
 * @param data The data to be set.
 *
 * @see ephysics_body_data_get()
 *
 * @ingroup EPhysics_Body
 */
EAPI void ephysics_body_data_set(EPhysics_Body *body, void *data);

/**
 * @brief
 * Return data previously set to body.
 *
 * @param body The physics body.
 * @return The data set or @c NULL on error.
 *
 * @see ephysics_body_data_get() for more details
 *
 * @ingroup EPhysics_Body
 */
EAPI void *ephysics_body_data_get(const EPhysics_Body *body);

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
 * Constraints can be created with @ref ephysics_constraint_p2p_add()
 * or @ref ephysics_constraint_slider_add() and removed
 * with @ref ephysics_constraint_del().
 * Can be applied between two bodies or between a body and the world.
 */

typedef struct _EPhysics_Constraint EPhysics_Constraint; /**< Constraint handle, used to limit bodies movements. Created with @ref ephysics_constraint_p2p_add() or @ref ephysics_constraint_slider_add() and deleted with @ref ephysics_constraint_del(). */

/**
 * @brief
 * Create a new constraint between 2 bodies(Point to Point constraint).
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
 * @return A new p2p(point to point) constraint or @c NULL, on errors.
 *
 * @see ephysics_constraint_del().
 *
 * @ingroup EPhysics_Constraint
 */
EAPI EPhysics_Constraint *ephysics_constraint_p2p_add(EPhysics_Body *body1, EPhysics_Body *body2, Evas_Coord anchor_b1_x, Evas_Coord anchor_b1_y, Evas_Coord anchor_b2_x, Evas_Coord anchor_b2_y);

/**
 * @brief
 * Create a new slider constraint.
 *
 * The constraint will limit the linear and angular moving of a body.
 *
 * @param body The body to apply the constraint.
 *
 * @see ephysics_constraint_slider_linear_limit_set() for linear moving limit
 * configuration.
 * @see ephysics_constraint_slider_angular_limit_set() for angular moving limit
 * configuration.
 * @return A new slider constraint or @c NULL on erros.
 *
 * @see ephysics_constraint_del().
 *
 * @ingroup EPhysics_Constraint
 */
EAPI EPhysics_Constraint *ephysics_constraint_slider_add(EPhysics_Body *body);

/**
 * @brief
 * Define the linear moving limits of a slider @p constraint.
 *
 * The linear limits are defined from the body's position on. The user will
 * want to limit the movements to the left, right, under and above the rigid
 * body. The unit for every limits are defined on Evas coordinates.
 *
 * @param constraint The constraint to be set.
 * @param left_x The moving limit to the left on X axis - from the body's
 * position on.
 * @param under_y The moving limit down on Y axis - from the body's position
 * on.
 * @param right_x The moving limit to the right on X axis - from the body's
 * position on.
 * @param above_y The moving limit up on Y axis - from the body's position on.
 *
 * @see ephysics_constraint_slider_linear_limit_get()
 * @ingroup EPhysics_Constraint
 */
EAPI void ephysics_constraint_slider_linear_limit_set(EPhysics_Constraint *constraint, Evas_Coord left_x, Evas_Coord under_y, Evas_Coord right_x, Evas_Coord above_y);

/**
 * @brief
 * Get the linear moving limits of a slider constraint.
 *
 * @param constraint The constraint to get linear limits from.
 * @param left_x Pointer to set with the limit to the left on X axis.
 * @param under_y Pointer to set with the limit down on Y axis.
 * @param right_x Pointer to set with the limit to the right on X axis.
 * @param above_y Pointer to set with the limit up on Y axis.
 *
 * @see ephysics_constraint_slider_linear_limit_set()
 * @ingroup EPhysics_Constraint
 */
EAPI void ephysics_constraint_slider_linear_limit_get(const EPhysics_Constraint *constraint, Evas_Coord *left_x, Evas_Coord *under_y, Evas_Coord *right_x, Evas_Coord *above_y);

/**
 * @brief
 * Set the angular moving limits of a slider @p constraint.
 *
 * The angular moving limits is defined in degrees and will limit the moving on
 * Z axis - counter clock and clock wise directions.
 *
 * @param constraint The constraint to be set.
 * @param counter_clock_z Amount of degrees from 0.0 to 360.0 to limit
 * counter clock rotation.
 * @param clock_wise_z Amount of degrees from 0.0 to 360.0 to limit clock wise
 * rotation.
 *
 * @see ephysics_constraint_slider_angular_limit_get()
 * @ingroup EPhysics_Constraint
 */
EAPI void ephysics_constraint_slider_angular_limit_set(EPhysics_Constraint *constraint, double counter_clock_z, double clock_wise_z);

/**
 * @brief
 * Get the angular moving limits of a slider @p constraint.
 *
 * @param constraint The constraint to get the angular limits from.
 * @param counter_clock_z Pointer to set with the counter clock limit degrees.
 * @param clock_wise_z Pointer to set with the clock wise limit degrees.
 *
 * @see ephysics_constraint_slider_angular_limit_set()
 * @ingroup EPhysics_Constraint
 */
EAPI void ephysics_constraint_slider_angular_limit_get(const EPhysics_Constraint *constraint, double *counter_clock_z, double *clock_wise_z);

/**
 * @brief
 * Deletes a physics constraint.
 *
 * @param constraint The constraint to be deleted.
 *
 * @see ephysics_constraint_p2p_add() for more details.
 * @see ephysics_constraint_slider_add() for more details.
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
