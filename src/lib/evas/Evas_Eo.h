#ifndef _EVAS_H
# error You shall not include this header directly
#endif

#ifdef EFL_EO_API_SUPPORT

#ifndef _EFL_CANVAS_VG_NODE_EO_CLASS_TYPE
#define _EFL_CANVAS_VG_NODE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Vg_Node;

#endif


#ifndef _EFL_CANVAS_ANIMATION_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation;

#endif

#ifndef _EFL_CANVAS_ANIMATION_ALPHA_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_ALPHA_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Alpha;

#endif

#ifndef _EFL_CANVAS_ANIMATION_ROTATE_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_ROTATE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Rotate;

#endif

#ifndef _EFL_CANVAS_ANIMATION_SCALE_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_SCALE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Scale;

#endif

#ifndef _EFL_CANVAS_ANIMATION_TRANSLATE_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_TRANSLATE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Translate;

#endif

#ifndef _EFL_CANVAS_ANIMATION_GROUP_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_GROUP_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Group;

#endif

#ifndef _EFL_CANVAS_ANIMATION_GROUP_PARALLEL_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_GROUP_PARALLEL_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Group_Parallel;

#endif

#ifndef _EFL_CANVAS_ANIMATION_GROUP_SEQUENTIAL_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_GROUP_SEQUENTIAL_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Group_Sequential;

#endif

#ifndef _EFL_CANVAS_ANIMATION_PLAYER_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_PLAYER_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Player;

#endif

#ifndef _EFL_CANVAS_ANIMATION_PLAYER_ALPHA_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_PLAYER_ALPHA_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Player_Alpha;

#endif

#ifndef _EFL_CANVAS_ANIMATION_PLAYER_ROTATE_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_PLAYER_ROTATE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Player_Rotate;

#endif

#ifndef _EFL_CANVAS_ANIMATION_PLAYER_SCALE_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_PLAYER_SCALE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Player_Scale;

#endif

#ifndef _EFL_CANVAS_ANIMATION_PLAYER_TRANSLATE_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_PLAYER_TRANSLATE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Player_Translate;

#endif

#ifndef _EFL_CANVAS_ANIMATION_PLAYER_GROUP_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_PLAYER_GROUP_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Player_Group;

#endif

#ifndef _EFL_CANVAS_ANIMATION_GROUP_PARALLEL_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_GROUP_PARALLEL_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Group_Parallel;

#endif

#ifndef _EFL_CANVAS_ANIMATION_GROUP_SEQUENTIAL_EO_CLASS_TYPE
#define _EFL_CANVAS_ANIMATION_GROUP_SEQUENTIAL_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Animation_Group_Sequential;

#endif

#define EFL_ANIMATION_GROUP_DURATION_NONE -1
#define EFL_ANIMATION_PLAYER_GROUP_DURATION_NONE -1

#define EFL_ANIMATION_REPEAT_INFINITE -1
#define EFL_ANIMATION_PLAYER_REPEAT_INFINITE -1


struct _Efl_Canvas_Animation_Player_Event_Running
{
   double progress;
};

struct _Efl_Canvas_Object_Animation_Event
{
   const Efl_Event_Description *event_desc;
};

/**
 * @ingroup Evas_Object_Rectangle
 *
 * @{
 */
#include "canvas/efl_canvas_rectangle.eo.h"
/**
 * @}
 */

/**
 * @ingroup Efl_Canvas_Text
 *
 * @{
 */
//#include "canvas/efl_canvas_text_cursor.eo.h"
#include "canvas/efl_canvas_text.eo.h"
#include "canvas/efl_canvas_text_factory.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Polygon
 *
 * @{
 */
#include "canvas/efl_canvas_polygon.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */
#include "canvas/efl_canvas_group.eo.h"
/**
 * @}
 */

/**
 * @}
 */

#include "canvas/efl_canvas_animation_types.eot.h"

#include "canvas/efl_canvas_object.eo.h"

#include "canvas/efl_canvas_animation.eo.h"
#include "canvas/efl_canvas_animation_alpha.eo.h"
#include "canvas/efl_canvas_animation_rotate.eo.h"
#include "canvas/efl_canvas_animation_scale.eo.h"
#include "canvas/efl_canvas_animation_translate.eo.h"
#include "canvas/efl_canvas_animation_group.eo.h"
#include "canvas/efl_canvas_animation_group_parallel.eo.h"
#include "canvas/efl_canvas_animation_group_sequential.eo.h"
#include "canvas/efl_canvas_animation_player.eo.h"

#endif /* EFL_EO_API_SUPPORT */

#if defined(EFL_BETA_API_SUPPORT) && defined(EFL_EO_API_SUPPORT)

#include "canvas/efl_canvas_event_grabber.eo.h"

 /**
 * @defgroup Evas_3D Evas 3D Extensions
 *
 * Evas extension to support 3D rendering.
 *
 * @ingroup Evas
 */

#include "canvas/evas_canvas3d_types.eot.h"

/**
 * @page evas_canvas3d_main Evas 3D
 *
 * @since 1.10
 *
 * @section toc Table of Contents
 *
 * @li @ref evas_canvas3d_intro
 * @li @ref evas_canvas3d_example
 *
 * @section evas_canvas3d_intro Introduction
 *
 * Evas 3D is an extension to support 3D scene graph rendering into 2D Evas
 * canvases, with tree-based scene graph manipulation and other typical 3D
 * rendering techniques.
 *
 * 3D objects are used to describe three dimensional scenes and to provide
 * interfaces to connect the scene to an Evas image object for rendering.
 *
 * Scenes are constructed by locating cameras and lights and selecting desired
 * meshes, and organizing Node objects into hierarchical n-ary tree data structures.
 * The scene is stacked on the canvas with existing Evas objects, which permits
 * intermingling 3D rendered content in existing 2D application layouts.
 *
 * Rendering techniques supported by Evas 3D include flat and phong
 * shading, normal and texture mapping, and triangle meshes.  Existing
 * Evas objects may also be used as textures inside the 3D scene,
 * including EFL widgets and even application windows.  This latter
 * capability makes it possible to create a 3D version of an arbitrary
 * EFL application with minimal code changes.
 *
 * @section evas_canvas3d_example Introductory Example
 *
 * @include evas-3d-cube.c
 */

/**
 * @defgroup Evas_Canvas3D_Types Types & Enums
 * @ingroup Evas_3D
 *
 * Primitive type definitions and enumerations.
 */

/**
 * @defgroup Evas_Canvas3D_Object Generic 3D Object Descriptions
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Object structure is an abstract base for other Evas
 * 3D objects (scenes, nodes, lights, meshes, textures, and materials)
 * with reference counting and propagation of modifications via
 * reference tracking.  This permits, for example, when a scene object
 * is modified (marked dirty), a number of image objects rendering that
 * object to be notified to update themselves, without needing to call
 * update functions manually.
 */

/**
 * @defgroup Evas_Canvas3D_Scene Scene Object
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Scene structure represents a captured image of a
 * scene graph through its viewing camera. A scene can be associated
 * with an image object for canvas display via the
 * evas_object_image_scene_set() function.
 */

/**
 * @defgroup Evas_Canvas3D_Node Node Object
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Node structure defines the position, orientation,
 * and scale of canvas objects (cameras, lights, meshes, etc.) in a 3D
 * space.  These nodes can be organized into a hierarchical n-ary tree
 * structure to construct a scene graph.
 */

/**
 * @defgroup Evas_Canvas3D_Camera Camera Object
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Camera structure defines properties used to
 * generate 2D pictures of a given scene graph, similar in concept to
 * focus length and film size for a real world camera.
 *
 * A given camera definition can be used to take multiple pictures of
 * the scene by establishing multiple nodes for the
 * Evas_Canvas3D_Camera, each of which are located at different
 * positions and with different orientations.  Convenience routines
 * evas_canvas3d_node_position_set() and
 * evas_canvas3d_node_look_at_set() are provided to adjust the position
 * and viewing direction for these nodes.
 */

/**
 * @defgroup Evas_Canvas3D_Light Light Object
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Light structure defines various light source
 * properties.  Reflection models include: Ambient, diffuse, and
 * specular.  Light models include directional, point, and spot.  The
 * position and direction for the light is tracked by the node that
 * contains the light.
 */

/**
 * @defgroup Evas_Canvas3D_Mesh Mesh Object
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Mesh structure manages key-frame based mesh
 * animations for geometrical objects like character models, terrain,
 * buildings, and other such visible objects.  Each mesh frame can have
 * its own material and geometric data.  Blending functions, fog
 * effects, level of detail boundaries, and shadow properties are also
 * supported.
 *
 * Like other data objects, a mesh definition is located and oriented in
 * the canvas with one or more nodes, with the mesh transformed from its
 * modeling coordinate space to the node's coordinate space.  The frame
 * number is also tracked by the node, permitting creation of multiple
 * nodes in the canvas each set to a different animation frame, for
 * example.
 *
 * Unlike camera and light objects, multiple meshes can be contained in
 * a single node.
 */

/**
 * @defgroup Evas_Canvas3D_Primitive Primitive Object
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Primitive structure defines the data for
 * generating meshes for various types of primitive shapes such as
 * cubes, cylinders, spheres, surfaces, terrain, etc.  Use the
 * evas_canvas3d_mesh_from_primitive_set() function to generate a
 * mesh's frame of this primitive.
 */

/**
 * @defgroup Evas_Canvas3D_Texture Texture Object
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Texture structure associates a 2D bitmap image to a
 * material for a surface.  Image data for the texture can be loaded
 * from memory, a file, or an Evas_Object.  Use the
 * evas_canvas3d_material_texture_set() function to add the texture to
 * an Evas_Canvas3DMaterial's slot.
 */

/**
 * @defgroup Evas_Canvas3D_Material Material Object
 * @ingroup Evas_3D
 *
 * The Evas_Canvas3D_Material structure defines a set of material
 * attributes used for determining the color of mesh surfaces.  Each
 * attribute is defined by a color value and texture map.  The five
 * attributes are: ambient, diffuse, specular, emission, and normal.
 */

/**
 * @typedef Evas_Canvas3D_Surface_Func
 *
 * The Evas_Canvas3D_Surface_Func type of functions are used to
 * create parametric surfaces as primitives.  These compute the
 * vertex x,y,z values for given v,u values.
 *
 * @param out_x The x component of the calculated value.
 * @param out_y The y component of the calculated value.
 * @param out_z The z component of the calculated value.
 * @param a is the v value.
 * @param b is the u value.

 * @see Evas_Canvas3D_Primitive
 *
 * @since 1.15
 * @ingroup Evas_Canvas3D_Primitive
 */
typedef void (Evas_Canvas3D_Surface_Func)(Evas_Real *out_x,
                                    Evas_Real *out_y,
                                    Evas_Real *out_z,
                                    Evas_Real a,
                                    Evas_Real b);

#include "canvas/evas_canvas3d_object.eo.h"

#include "canvas/evas_canvas3d_texture.eo.h"

#include "canvas/evas_canvas3d_material.eo.h"

#include "canvas/evas_canvas3d_light.eo.h"

#include "canvas/evas_canvas3d_primitive.eo.h"

#include "canvas/evas_canvas3d_mesh.eo.h"

#include "canvas/evas_canvas3d_node.eo.h"

#include "canvas/evas_canvas3d_camera.eo.h"

#include "canvas/evas_canvas3d_scene.eo.h"

#include "canvas/efl_canvas_scene3d.eo.h"
#endif /* EFL_BETA_API_SUPPORT */

#ifdef EFL_EO_API_SUPPORT
#include "canvas/efl_canvas_image_internal.eo.h"
#include "canvas/efl_canvas_image.eo.h"
#include "canvas/efl_canvas_snapshot.eo.h"
#include "canvas/efl_canvas_proxy.eo.h"
#include "canvas/efl_gfx_mapping.eo.h"
#endif /* EFL_EO_API_SUPPORT */

#if defined(EFL_BETA_API_SUPPORT) && defined(EFL_EO_API_SUPPORT)
/**
 * @ingroup Evas_Object_VG
 *
 * @{
 */
#include "canvas/efl_canvas_vg_object.eo.h"
/**
 * @}
 */

#include "canvas/efl_canvas_vg_node.eo.h"
#include "canvas/efl_canvas_vg_container.eo.h"
#include "canvas/efl_canvas_vg_shape.eo.h"
#include "canvas/efl_canvas_vg_gradient.eo.h"
#include "canvas/efl_canvas_vg_gradient_linear.eo.h"
#include "canvas/efl_canvas_vg_gradient_radial.eo.h"
#endif /* EFL_BETA_API_SUPPORT */

#ifdef EFL_EO_API_SUPPORT
#include "canvas/efl_input_state.eo.h"
#include "canvas/efl_input_event.eo.h"
#include "canvas/efl_input_pointer.eo.h"
#include "canvas/efl_input_key.eo.h"
#include "canvas/efl_input_hold.eo.h"
#include "canvas/efl_input_interface.eo.h"
#include "canvas/efl_input_focus.eo.h"
#endif /* EFL_EO_API_SUPPORT */

#ifdef EFL_EO_API_SUPPORT
# include "gesture/efl_canvas_gesture_types.eot.h"
# include "gesture/efl_canvas_gesture_touch.eo.h"
# include "gesture/efl_canvas_gesture.eo.h"
# include "gesture/efl_canvas_gesture_tap.eo.h"
# include "gesture/efl_canvas_gesture_long_tap.eo.h"
# include "gesture/efl_canvas_gesture_recognizer.eo.h"
# include "gesture/efl_canvas_gesture_manager.eo.h"
#endif /* EFL_EO_API_SUPPORT */
