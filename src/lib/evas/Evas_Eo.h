#ifndef _EVAS_H
# error You shall not include this header directly
#endif

#include "canvas/evas_signal_interface.eo.h"
#include "canvas/evas_draggable_interface.eo.h"
#include "canvas/evas_clickable_interface.eo.h"
#include "canvas/evas_scrollable_interface.eo.h"
#include "canvas/evas_selectable_interface.eo.h"
#include "canvas/evas_zoomable_interface.eo.h"

// Interface classes links
#define EVAS_SMART_SIGNAL_INTERFACE     EVAS_SIGNAL_INTERFACE_INTERFACE
#define EVAS_SMART_CLICKABLE_INTERFACE  EVAS_CLICKABLE_INTERFACE_INTERFACE
#define EVAS_SMART_SCROLLABLE_INTERFACE EVAS_SCROLLABLE_INTERFACE_INTERFACE
#define EVAS_SMART_DRAGGABLE_INTERFACE  EVAS_DRAGGABLE_INTERFACE_INTERFACE
#define EVAS_SMART_SELECTABLE_INTERFACE EVAS_SELECTABLE_INTERFACE_INTERFACE
#define EVAS_SMART_ZOOMABLE_INTERFACE   EVAS_ZOOMABLE_INTERFACE_INTERFACE

#include "canvas/evas_canvas.eo.h"

/**
 * @ingroup Evas_Object_Rectangle
 *
 * @{
 */
#include "canvas/evas_rectangle.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Text
 *
 * @{
 */
#include "canvas/evas_text.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Textblock
 *
 * @{
 */
#include "canvas/evas_textblock.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Textgrid
 *
 * @{
 */
#include "canvas/evas_textgrid.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Line
 *
 * @{
 */
#include "canvas/evas_line.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Polygon
 *
 * @{
 */
#include "canvas/evas_polygon.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */
#include "canvas/evas_object_smart.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Clipped
 *
 * @{
 */
#include "canvas/evas_smart_clipped.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Box
 *
 * @{
 */
#include "canvas/evas_box.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Table
 *
 * @{
 */
#include "canvas/evas_table.eo.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Grid
 *
 * @{
 */
#include "canvas/evas_grid.eo.h"
/**
 * @}
 */

#include "canvas/evas_common_interface.eo.h"

#include "canvas/evas_object.eo.h"

#include "canvas/evas_out.eo.h"


 /**
 * @defgroup Evas_3D Evas 3D Extensions
 *
 * Evas extension to support 3D rendering.
 *
 * @ingroup Evas
 */

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
 * canvas supporting typicall tree-based scene graph manipulation and other 3D
 * graphics rendering techniques.
 *
 * Evas 3D provides 3D objects which are used for describing 3D scene and APIs
 * to connect the scene with an evas image object so that the scene is rendered
 * on that image object.
 *
 * Construction of a 3D scene is process of locating desired cameras, lights and
 * meshes in the scene. Typically the scene is structured with some hierarchical
 * data structure. Evas 3D support n-ary tree structure for describing the
 * scene. Node is used to build the tree representation of the scene. Other
 * objects, like camera, light and mesh can be located in the scene by being
 * contained in a node.
 *
 * Like other 3D graphics engine, Evas 3D support standard 3D rendering method
 * like flat shading, phong shading and normal map and other features like
 * texture mapping, triangle meshes.
 *
 * Besides all the traditional 3D rendering things, one of the key feature of
 * the Evas 3D is that it is able to use existing evas objects as textures
 * inside of the 3D scene. "Existing evas objects" means all the EFL widgets
 * and applications. By supporting this, it is easy to make 3D version of an
 * application without modifying the original source that much.
 *
 * Also, 3D scene can be located on the canvas naturally stacked with existing
 * evas objects. This can make it possible putting 3D things into existing 2D
 * application layouts.
 *
 * @section evas_canvas3d_example Introductory Example
 *
 * @include evas-3d-cube.c
 */

/**
 * @defgroup Evas_Canvas3D_Types Types & Enums
 * @ingroup Evas_3D
 *
 * Primitive type definitions and enumations.
 */

/**
 * @defgroup Evas_Canvas3D_Object Generic 3D Object Descriptions
 * @ingroup Evas_3D
 *
 * Evas 3D object is a generic type of all evas 3D objects like scene, node,
 * camera, light, mesh, texture and material. Evas 3D object is basically
 * reference counted. Any successful function call on an object which make a
 * reference to an another object will increase the reference count. When the
 * reference count gets to 0, the object will be actually deleted.
 *
 * Any modifications are automatically propagated to other objects referencing
 * the modified objects. As a result, if the scene object is set to modified
 * state, all image objects having the scene as a rendering source are marked
 * as dirty, so that rendering will be updated at next frame. But all these
 * things are done internally, so feel free to forget about calling some kind
 * of update functions.
 */

/**
 * @defgroup Evas_Canvas3D_Scene Scene Object
 * @ingroup Evas_3D
 *
 * A scene represents a captured image of a scene graph through its viewing
 * camera. A scene can be set to an image object to be displayed on the Evas
 * canvas by using evas_object_image_scene_set() function.
 */

/**
 * @defgroup Evas_Canvas3D_Node Node Object
 * @ingroup Evas_3D
 *
 * A node is used for hierarchical construction of a scene graph. Evas 3D
 * provides n-ary tree structure for the scene graph construction. A node has
 * its position, orientation and scale. Other objects, like camera, light and
 * mesh can be contained in a node to be located in a 3D space.
 */

/**
 * @defgroup Evas_Canvas3D_Camera Camera Object
 * @ingroup Evas_3D
 *
 * A camera object is used for taking a picture of a scene graph. A camera
 * object itself is just a set of properties on how the camera should take the
 * picture (like focus length and film size of the real world cameras). To be
 * able to take a shot of the scene, a camera should be located in the scene, so
 * that it has its viewing position and direction. It is done by containing the
 * camera on a node. If one wants to locate several cameras having same
 * properties, instead of creating multiple cameras, just create one camera and
 * multiple nodes containing the camera and locate them at each desired position
 * and direction. Just for convenience, use evas_canvas3d_node_position_set() to move
 * the camera to desired position and use evas_canvas3d_node_look_at_set() to adjust
 * the viewing direction of the camera.
 */

/**
 * @defgroup Evas_Canvas3D_Light Light Object
 * @ingroup Evas_3D
 *
 * A light object represents a set of properties of a light source. Evas 3D
 * provides standard reflection model that of ambient, diffuse and specular
 * reflection model. Also, Evas 3D support 3 types of light model, directional,
 * point and spot light. Light position and direction is determined by the node
 * containing the light.
 */

/**
 * @defgroup Evas_Canvas3D_Mesh Mesh Object
 * @ingroup Evas_3D
 *
 * A mesh object is a set of information on a visible geometrical object like
 * character model, terrain or other structures and entities. Evas 3D support
 * key-frame-based mesh animation, so a mesh can have multiple frames and each
 * frame has its own material and geometric data. Like other data objects, a
 * mesh can be located on a scene by being contained in a node. The mesh is
 * transformed from its modeling coordinate space into the node's coordinate
 * space. Also, the frame number is saved in the containing node. So, one can
 * locate multiple nodes having same mesh object with different animation frame
 * and transform. Unlike camera and light object, multiple meshes can be
 * contained in a single node.
 */

/**
 * @defgroup Evas_Canvas3D_Primitive Primitive Object
 * @ingroup Evas_3D
 *
 * A primitive object is an object with necessary data for creating meshes with ptimitive shapes.
 * Data from a primitive can be set to Evas_Canvas3D_Mesh by using evas_canvas3d_mesh_from_primitive_set()
 * function.
 */

/**
 * @defgroup Evas_Canvas3D_Texture Texture Object
 * @ingroup Evas_3D
 *
 * A texture object is an image represents material of surfaces. A texture can
 * be set to a slot of Evas_Canvas3D_Material by using evas_canvas3d_material_texture_set()
 * function. The data of a texture can be loaded from memory, file and other
 * Evas_Object.
 */

/**
 * @defgroup Evas_Canvas3D_Material Material Object
 * @ingroup Evas_3D
 *
 * A material object represents properties of surfaces. Evas 3D defines the
 * properties with 5 material attributes, ambient, diffuse, specular emission
 * and normal. Each attribute have its color value and texture map. Materials
 * are used to determine the color of mesh surfaces.
 */

/**
 * @typedef Evas_Canvas3D_Surface_Func
 *
 * User-defined parametric surface function.
 * Used for easy creation of custom surfaces as a primitive.
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

#include "canvas/evas_image.eo.h"

/**
 * @ingroup Evas_Object_VG
 *
 * @{
 */
#include "canvas/evas_vg.eo.h"
/**
 * @}
 */

#include "canvas/efl_vg_base.eo.h"
#include "canvas/efl_vg_container.eo.h"
#include "canvas/efl_vg_shape.eo.h"
#include "canvas/efl_vg_gradient.eo.h"
#include "canvas/efl_vg_gradient_linear.eo.h"
#include "canvas/efl_vg_gradient_radial.eo.h"
