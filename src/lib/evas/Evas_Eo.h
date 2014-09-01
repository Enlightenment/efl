#ifndef _EVAS_H
# error You shall not include this header directly
#endif

EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_FOCUS_IN;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_FOCUS_OUT;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_POST;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_OBJECT_FOCUS_IN;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_OBJECT_FOCUS_OUT;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_PRE;
EAPI extern const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_POST;

// Callbacks events for use with Evas canvases
#define EVAS_CANVAS_EVENT_FOCUS_IN (&(_EVAS_CANVAS_EVENT_FOCUS_IN))
#define EVAS_CANVAS_EVENT_FOCUS_OUT (&(_EVAS_CANVAS_EVENT_FOCUS_OUT))
#define EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE (&(_EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE))
#define EVAS_CANVAS_EVENT_RENDER_FLUSH_POST (&(_EVAS_CANVAS_EVENT_RENDER_FLUSH_POST))
#define EVAS_CANVAS_EVENT_OBJECT_FOCUS_IN (&(_EVAS_CANVAS_EVENT_OBJECT_FOCUS_IN))
#define EVAS_CANVAS_EVENT_OBJECT_FOCUS_OUT (&(_EVAS_CANVAS_EVENT_OBJECT_FOCUS_OUT))
#define EVAS_CANVAS_EVENT_RENDER_PRE (&(_EVAS_CANVAS_EVENT_RENDER_PRE))
#define EVAS_CANVAS_EVENT_RENDER_POST (&(_EVAS_CANVAS_EVENT_RENDER_POST))

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
 * @page evas_3d_main Evas 3D
 *
 * @since 1.10
 *
 * @section toc Table of Contents
 *
 * @li @ref evas_3d_intro
 * @li @ref evas_3d_example
 *
 * @section evas_3d_intro Introduction
 *
 * Evas 3D is an extension to support 3D scene graph rendering into 2D Evas
 * canvas supporting typicall tree-based scene graph manipulation and other 3D
 * graphics rendering techniques.
 *
 * Evas 3D provides 3D objects which are used for describing 3D scene and APIs
 * to connect the scene with an evas image object so that the scene is rendered
 * on that image object.
 *
 * Contruction of a 3D scene is process of locating desired cameras, lights and
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
 * @section evas_3d_example Introductory Example
 *
 * @include evas-3d-cube.c
 */

/**
 * @defgroup Evas_3D_Types Types & Enums
 * @ingroup Evas_3D
 *
 * Primitive type definitions and enumations.
 */

/**
 * @defgroup Evas_3D_Object Generic 3D Object Descriptions
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
 * @defgroup Evas_3D_Scene Scene Object
 * @ingroup Evas_3D
 *
 * A scene represents a captured image of a scene graph through its viewing
 * camera. A scene can be set to an image object to be displayed on the Evas
 * canvas by using evas_object_image_scene_set() function.
 */

/**
 * @defgroup Evas_3D_Node Node Object
 * @ingroup Evas_3D
 *
 * A node is used for hierarchical construction of a scene graph. Evas 3D
 * provides n-ary tree structure for the scene graph construction. A node has
 * its position, orientation and scale. Other objects, like camera, light and
 * mesh can be contained in a node to be located in a 3D space.
 */

/**
 * @defgroup Evas_3D_Camera Camera Object
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
 * and direction. Just for convinience, use evas_3d_node_position_set() to move
 * the camera to desired position and use evas_3d_node_look_at_set() to adjust
 * the viewing direction of the camera.
 */

/**
 * @defgroup Evas_3D_Light Light Object
 * @ingroup Evas_3D
 *
 * A light object represents a set of properties of a light source. Evas 3D
 * provides standard reflection model that of ambient, diffuse and specular
 * reflection model. Also, Evas 3D support 3 types of light model, directional,
 * point and spot light. Light position and direction is determined by the node
 * containing the light.
 */

/**
 * @defgroup Evas_3D_Mesh Mesh Object
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
 * @defgroup Evas_3D_Texture Texture Object
 * @ingroup Evas_3D
 *
 * A texture object is an image represents material of surfaces. A texture can
 * be set to a slot of Evas_3D_Material by using evas_3d_material_texture_set()
 * function. The data of a texture can be loaded from memory, file and other
 * Evas_Object.
 */

/**
 * @defgroup Evas_3D_Material Material Object
 * @ingroup Evas_3D
 *
 * A material object represents properties of surfaces. Evas 3D defines the
 * properties with 5 material attributes, ambient, diffuse, specular emission
 * and normal. Each attribute have its color value and texture map. Materials
 * are used to determine the color of mesh surfaces.
 */

/**
 * @typedef Evas_Real
 *
 * Floating-point data type
 *
 * Evas 3D use its own floating-point type. Even though it's a standard IEEE
 * 754 floating-point type always use Evas_Real for the type safety. Double
 * precision and fixed-point types will be useful but it's not supported yet.
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef double   Evas_Real;


/**
 * @typedef Evas_3D_Object
 *
 * 3D Object object handle
 *
 * @since 1.10
 * @ingroup Evas_3D_Object
 */
typedef Eo    Evas_3D_Object;

#define _EVAS_3D_OBJECT_EO_CLASS_TYPE

/**
 * @typedef Evas_3D_Scene
 *
 * Scene object handle
 *
 * @since 1.10
 * @ingroup Evas_3D_Scene
 */
typedef Eo    Evas_3D_Scene;

#define _EVAS_3D_SCENE_EO_CLASS_TYPE

/**
 * @typedef Evas_3D_Node
 *
 * Node object handle
 *
 * @since 1.10
 * @ingroup Evas_3D_Node
 */
typedef Eo     Evas_3D_Node;

#define _EVAS_3D_NODE_EO_CLASS_TYPE

/**
 * @typedef Evas_3D_Camera
 *
 * Camera object handle
 *
 * @since 1.10
 * @ingroup Evas_3D_Camera
 */
typedef Eo   Evas_3D_Camera;

#define _EVAS_3D_CAMERA_EO_CLASS_TYPE

/**
 * @typedef Evas_3D_Light
 *
 * Light object handle
 *
 * @since 1.10
 * @ingroup Evas_3D_Light
 */
typedef Eo    Evas_3D_Light;

#define _EVAS_3D_LIGHT_EO_CLASS_TYPE

/**
 * @typedef Evas_3D_Mesh
 *
 * Mesh object handle
 *
 * @since 1.10
 * @ingroup Evas_3D_Mesh
 */
typedef Eo     Evas_3D_Mesh;

#define _EVAS_3D_MESH_EO_CLASS_TYPE

/**
 * @typedef Evas_3D_Texture
 *
 * Texture object handle
 *
 * @since 1.10
 * @ingroup Evas_3D_Texture
 */
typedef Eo    Evas_3D_Texture;

#define _EVAS_3D_TEXTURE_EO_CLASS_TYPE

/**
 * @typedef Evas_3D_Material
 *
 * Material object handle
 *
 * @since 1.10
 * @ingroup Evas_3D_Material
 */
typedef Eo     Evas_3D_Material;

#define _EVAS_3D_MATERIAL_EO_CLASS_TYPE

/**
 * Type of 3D Object
 *
 * @since 1.10
 * @ingroup Evas_3D_Type
 */
typedef enum _Evas_3D_Object_Type
{
   EVAS_3D_OBJECT_TYPE_INVALID = 0,
   EVAS_3D_OBJECT_TYPE_SCENE,
   EVAS_3D_OBJECT_TYPE_NODE,
   EVAS_3D_OBJECT_TYPE_CAMERA,
   EVAS_3D_OBJECT_TYPE_LIGHT,
   EVAS_3D_OBJECT_TYPE_MODEL,
   EVAS_3D_OBJECT_TYPE_MESH,
   EVAS_3D_OBJECT_TYPE_TEXTURE,
   EVAS_3D_OBJECT_TYPE_MATERIAL,
} Evas_3D_Object_Type;

/**
 * State of the Evas 3D
 *
 * @since 1.10
 * @ingroup Evas_3D_Type
 */
typedef enum _Evas_3D_State
{
   EVAS_3D_STATE_MAX = 16,

   EVAS_3D_STATE_ANY = 0,

   EVAS_3D_STATE_SCENE_ROOT_NODE = 1,
   EVAS_3D_STATE_SCENE_CAMERA_NODE,
   EVAS_3D_STATE_SCENE_BACKGROUND_COLOR,
   EVAS_3D_STATE_SCENE_SIZE,

   EVAS_3D_STATE_TEXTURE_DATA = 1,
   EVAS_3D_STATE_TEXTURE_WRAP,
   EVAS_3D_STATE_TEXTURE_FILTER,

   EVAS_3D_STATE_MATERIAL_ID = 1,
   EVAS_3D_STATE_MATERIAL_COLOR,
   EVAS_3D_STATE_MATERIAL_TEXTURE,

   EVAS_3D_STATE_MESH_VERTEX_COUNT = 1,
   EVAS_3D_STATE_MESH_FRAME,
   EVAS_3D_STATE_MESH_MATERIAL,
   EVAS_3D_STATE_MESH_TRANSFORM,
   EVAS_3D_STATE_MESH_VERTEX_DATA,
   EVAS_3D_STATE_MESH_INDEX_DATA,
   EVAS_3D_STATE_MESH_VERTEX_ASSEMBLY,
   EVAS_3D_STATE_MESH_SHADE_MODE,
   EVAS_3D_STATE_MESH_FOG,

   EVAS_3D_STATE_CAMERA_PROJECTION = 1,

   EVAS_3D_STATE_LIGHT_AMBIENT = 1,
   EVAS_3D_STATE_LIGHT_DIFFUSE,
   EVAS_3D_STATE_LIGHT_SPECULAR,
   EVAS_3D_STATE_LIGHT_SPOT_DIR,
   EVAS_3D_STATE_LIGHT_SPOT_EXP,
   EVAS_3D_STATE_LIGHT_SPOT_CUTOFF,
   EVAS_3D_STATE_LIGHT_ATTENUATION,

   EVAS_3D_STATE_NODE_TRANSFORM = 1,
   EVAS_3D_STATE_NODE_MESH_GEOMETRY,
   EVAS_3D_STATE_NODE_MESH_MATERIAL,
   EVAS_3D_STATE_NODE_MESH_FRAME,
   EVAS_3D_STATE_NODE_MESH_SHADE_MODE,
   EVAS_3D_STATE_NODE_MESH_MATERIAL_ID,
   EVAS_3D_STATE_NODE_LIGHT,
   EVAS_3D_STATE_NODE_CAMERA,
   EVAS_3D_STATE_NODE_PARENT,
   EVAS_3D_STATE_NODE_MEMBER,
} Evas_3D_State;

/**
 * Transform space
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Space
{
   EVAS_3D_SPACE_LOCAL = 0, /**< Local coordinate space */
   EVAS_3D_SPACE_PARENT,    /**< Parent coordinate space */
   EVAS_3D_SPACE_WORLD,     /**< World coordinate space */
} Evas_3D_Space;

/**
 * Types of a node
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Node_Type
{
   EVAS_3D_NODE_TYPE_NODE = 0, /**< Node with no items */
   EVAS_3D_NODE_TYPE_CAMERA,   /**< Node which can contain camera object */
   EVAS_3D_NODE_TYPE_LIGHT,    /**< Node which can contain light object */
   EVAS_3D_NODE_TYPE_MESH,     /**< Node which can contain mesh objects */
} Evas_3D_Node_Type;

/**
 * Vertex attribute IDs
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Vertex_Attrib
{
   EVAS_3D_VERTEX_POSITION = 0,   /**< Vertex position */
   EVAS_3D_VERTEX_NORMAL,         /**< Vertex normal */
   EVAS_3D_VERTEX_TANGENT,        /**< Vertex tangent (for normal mapping) */
   EVAS_3D_VERTEX_COLOR,          /**< Vertex color */
   EVAS_3D_VERTEX_TEXCOORD,       /**< Vertex texture coordinate */
} Evas_3D_Vertex_Attrib;

/**
 * Index formats
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Index_Format
{
   EVAS_3D_INDEX_FORMAT_NONE = 0,       /**< Indexing is not used */
   EVAS_3D_INDEX_FORMAT_UNSIGNED_BYTE,  /**< Index is of type unsigned byte */
   EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT  /**< Index is of type unsigned short */
} Evas_3D_Index_Format;

/**
 * Vertex assembly modes
 *
 * Vertex assembly represents how the vertices are organized into geometric
 * primitives.
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Vertex_Assembly
{
   /**< A vertex is rendered as a point */
   EVAS_3D_VERTEX_ASSEMBLY_POINTS = 0,
   /**< Two vertices are organized as a line */
   EVAS_3D_VERTEX_ASSEMBLY_LINES,
   /**< Vertices are organized as a connected line path */
   EVAS_3D_VERTEX_ASSEMBLY_LINE_STRIP,
   /**< Vertices are organized as a closed line path */
   EVAS_3D_VERTEX_ASSEMBLY_LINE_LOOP,
   /**< Three vertices are organized as a triangle */
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES,
   /**< Vertices are organized as connected triangles */
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP,
   /**< Vertices are organized as a triangle fan */
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_FAN,
} Evas_3D_Vertex_Assembly;

/**
 * Color formats of pixel data
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Color_Format
{
   /**< Color contains full components, red, green, blue and alpha */
   EVAS_3D_COLOR_FORMAT_RGBA = 0,
   /**< Color contains only red, green and blue components */
   EVAS_3D_COLOR_FORMAT_RGB,
   /**< Color contains only alpha component */
   EVAS_3D_COLOR_FORMAT_ALPHA
} Evas_3D_Color_Format;

/**
 * Pixel formats
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Pixel_Format
{
   /**< 8-bit pixel with single component */
   EVAS_3D_PIXEL_FORMAT_8 = 0,
   /**< 16-bit pixel with three components (5-6-5 bit) */
   EVAS_3D_PIXEL_FORMAT_565,
   /**< 24-bit pixel with three 8-bit components */
   EVAS_3D_PIXEL_FORMAT_888,
   /**< 32-bit pixel with four 8-bit components */
   EVAS_3D_PIXEL_FORMAT_8888,
   /**< 16-bit pixel with four 4-bit components */
   EVAS_3D_PIXEL_FORMAT_4444,
   /**< 16-bit pixel with four components (5-5-5-1 bit) */
   EVAS_3D_PIXEL_FORMAT_5551
} Evas_3D_Pixel_Format;

/**
 * Wrap modes
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Wrap_Mode
{
   /**< Values will be clamped to be in range [min, max] */
   EVAS_3D_WRAP_MODE_CLAMP = 0,
   /**< Values will be repeated */
   EVAS_3D_WRAP_MODE_REPEAT,
   /**< Values will be repeated in a reflected manner */
   EVAS_3D_WRAP_MODE_REFLECT
} Evas_3D_Wrap_Mode;

/**
 * Texture filters
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Texture_Filter
{
   /**< Samples nearest texel */
   EVAS_3D_TEXTURE_FILTER_NEAREST = 0,
   /**< Lineary interpolate nearby texels */
   EVAS_3D_TEXTURE_FILTER_LINEAR,
   /**< Nearest sampling mipmap */
   EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
   /**< Nearest sampling mipmap and interpolate */
   EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
   /**< Linear sampling in nearest mipmap */
   EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
   /**< Linear sampling in mipmap and interpolate */
   EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR
} Evas_3D_Texture_Filter;

/**
 * Shade modes
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Shade_Mode
{
   /**< Shaded using vertex color attribute */
   EVAS_3D_SHADE_MODE_VERTEX_COLOR = 0,
   /**< Shaded using material diffuse term */
   EVAS_3D_SHADE_MODE_DIFFUSE,
   /**< Per-vertex flat lighting */
   EVAS_3D_SHADE_MODE_FLAT,
   /**< Per-pixel phong shading */
   EVAS_3D_SHADE_MODE_PHONG,
   /**< Per-pixel normal map shading */
   EVAS_3D_SHADE_MODE_NORMAL_MAP
} Evas_3D_Shade_Mode;

/**
 * Material attributes
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Material_Attrib
{
   EVAS_3D_MATERIAL_AMBIENT = 0,     /**< Ambient term */
   EVAS_3D_MATERIAL_DIFFUSE,         /**< Diffuse term */
   EVAS_3D_MATERIAL_SPECULAR,        /**< Specular term */
   EVAS_3D_MATERIAL_EMISSION,        /**< Emission term */
   EVAS_3D_MATERIAL_NORMAL,          /**< Normal map term */
} Evas_3D_Material_Attrib;

/**
 * Mesh file type
 *
 * @since 1.10
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Mesh_File_Type
{
   EVAS_3D_MESH_FILE_TYPE_MD2 = 0,   /**< Quake's MD2 mesh file format */
   EVAS_3D_MESH_FILE_TYPE_OBJ,
   EVAS_3D_MESH_FILE_TYPE_EET,
} Evas_3D_Mesh_File_Type;

#include "canvas/evas_image.eo.h"

#include "canvas/evas_3d_camera.eo.h"

#include "canvas/evas_3d_texture.eo.h"

#include "canvas/evas_3d_material.eo.h"

#include "canvas/evas_3d_light.eo.h"

#include "canvas/evas_3d_mesh.eo.h"

#include "canvas/evas_3d_node.eo.h"

#include "canvas/evas_3d_scene.eo.h"

#include "canvas/evas_3d_object.eo.h"
