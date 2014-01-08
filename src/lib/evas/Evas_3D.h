#ifndef _EVAS_3D_H
#define _EVAS_3D_H

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
 * @date 2014 (created)
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
 * canvas by using evas_object_image_3d_scene_set() function.
 */

/**
 * @defgroup Evas_3D_Node Node Object
 * @ingroup Evas_3D
 *
 * A node is used for hierarchical construction of a scene graph. Evas 3D
 * provides n-ary tree structure for the scene graph construction.A node has
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
 * @ingroup Evas_3D_Types
 */
typedef float   Evas_Real;

/**
 * @typedef Evas_3D_Scene
 *
 * Scene object handle
 *
 * @ingroup Evas_3D_Scene
 */
typedef struct _Evas_3D_Scene    Evas_3D_Scene;

/**
 * @typedef Evas_3D_Node
 *
 * Node object handle
 *
 * @ingroup Evas_3D_Node
 */
typedef struct _Evas_3D_Node     Evas_3D_Node;

/**
 * @typedef Evas_3D_Camera
 *
 * Camera object handle
 *
 * @ingroup Evas_3D_Camera
 */
typedef struct _Evas_3D_Camera   Evas_3D_Camera;

/**
 * @typedef Evas_3D_Light
 *
 * Light object handle
 *
 * @ingroup Evas_3D_Light
 */
typedef struct _Evas_3D_Light    Evas_3D_Light;

/**
 * @typedef Evas_3D_Mesh
 *
 * Mesh object handle
 *
 * @ingroup Evas_3D_Mesh
 */
typedef struct _Evas_3D_Mesh     Evas_3D_Mesh;

/**
 * @typedef Evas_3D_Texture
 *
 * Texture object handle
 *
 * @ingroup Evas_3D_Texture
 */
typedef struct _Evas_3D_Texture  Evas_3D_Texture;

/**
 * @typedef Evas_3D_Material
 *
 * Material object handle
 *
 * @ingroup Evas_3D_Material
 */
typedef struct _Evas_3D_Material Evas_3D_Material;

/**
 * Transform space
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Space
{
   EVAS_3D_SPACE_LOCAL,    /**< Local coordinate space */
   EVAS_3D_SPACE_PARENT,   /**< Parent coordinate space */
   EVAS_3D_SPACE_WORLD,    /**< World coordinate space */
} Evas_3D_Space;

/**
 * Types of a node
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Node_Type
{
   EVAS_3D_NODE_TYPE_NODE,    /**< Node with no items */
   EVAS_3D_NODE_TYPE_CAMERA,  /**< Node which can contain camera object */
   EVAS_3D_NODE_TYPE_LIGHT,   /**< Node which can contain light object */
   EVAS_3D_NODE_TYPE_MESH,    /**< Node which can contain mesh objects */
} Evas_3D_Node_Type;

/**
 * Vertex attribute IDs
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Vertex_Attrib
{
   EVAS_3D_VERTEX_POSITION,   /**< Vertex position */
   EVAS_3D_VERTEX_NORMAL,     /**< Vertex normal */
   EVAS_3D_VERTEX_TANGENT,    /**< Vertex tangent (for normal mapping) */
   EVAS_3D_VERTEX_COLOR,      /**< Vertex color */
   EVAS_3D_VERTEX_TEXCOORD,   /**< Vertex texture coordinate */
} Evas_3D_Vertex_Attrib;

/**
 * Index formats
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Index_Format
{
   EVAS_3D_INDEX_FORMAT_NONE,             /**< Indexing is not used */
   EVAS_3D_INDEX_FORMAT_UNSIGNED_BYTE,    /**< Index is of type unsigned byte */
   EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,   /**< Index is of type unsigned short */
} Evas_3D_Index_Format;

/**
 * Vertex assembly modes
 * @ingroup Evas_3D_Types
 *
 * Vertex assembly represents how the vertices are organized into geometric
 * primitives.
 */
typedef enum _Evas_3D_Vertex_Assembly
{
   EVAS_3D_VERTEX_ASSEMBLY_POINTS,           /**< A vertex is rendered as a point */
   EVAS_3D_VERTEX_ASSEMBLY_LINES,            /**< Two vertices are organized as a line */
   EVAS_3D_VERTEX_ASSEMBLY_LINE_STRIP,       /**< Vertices are organized as a connected line path */
   EVAS_3D_VERTEX_ASSEMBLY_LINE_LOOP,        /**< Vertices are organized as a closed line path */
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES,        /**< Three vertices are organized as a triangle */
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP,   /**< Vertices are organized as connected triangles */
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_FAN,     /**< Vertices are organized as a triangle fan */
} Evas_3D_Vertex_Assembly;

/**
 * Color formats of pixel data
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Color_Format
{
   EVAS_3D_COLOR_FORMAT_RGBA,    /**< Color contains full components, red, green, blue and alpha */
   EVAS_3D_COLOR_FORMAT_RGB,     /**< Color contains only red, green and blue components */
   EVAS_3D_COLOR_FORMAT_ALPHA,   /**< Color contains only alpha component */
} Evas_3D_Color_Format;

/**
 * Pixel formats
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Pixel_Format
{
   EVAS_3D_PIXEL_FORMAT_8,    /**< 8-bit pixel with single component */
   EVAS_3D_PIXEL_FORMAT_565,  /**< 16-bit pixel with three components (5-6-5 bit) */
   EVAS_3D_PIXEL_FORMAT_888,  /**< 24-bit pixel with three 8-bit components */
   EVAS_3D_PIXEL_FORMAT_8888, /**< 32-bit pixel with four 8-bit components */
   EVAS_3D_PIXEL_FORMAT_4444, /**< 16-bit pixel with four 4-bit components */
   EVAS_3D_PIXEL_FORMAT_5551, /**< 16-bit pixel with four components  (5-5-5-1 bit) */
} Evas_3D_Pixel_Format;

/**
 * Wrap modes
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Wrap_Mode
{
   EVAS_3D_WRAP_MODE_CLAMP,   /**< Values will be clamped to be in range [min, max] */
   EVAS_3D_WRAP_MODE_REPEAT,  /**< Values will be repeated */
   EVAS_3D_WRAP_MODE_REFLECT, /**< Values will be repeated in a reflected manner */
} Evas_3D_Wrap_Mode;

/**
 * Texture filters
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Texture_Filter
{
   EVAS_3D_TEXTURE_FILTER_NEAREST,                 /**< Samples nearest texel */
   EVAS_3D_TEXTURE_FILTER_LINEAR,                  /**< Lineary interpolate nearby texels */
   EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,  /**< Nearest sampling mipmap */
   EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,   /**< Nearest sampling mipmap and interpolate */
   EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,   /**< Linear sampling in nearest mipmap */
   EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR,    /**< Linear sampling in mipmap and interpolate */
} Evas_3D_Texture_Filter;

/**
 * Shade modes
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Shade_Mode
{
   EVAS_3D_SHADE_MODE_VERTEX_COLOR,    /**< Shaded using vertex color attribute */
   EVAS_3D_SHADE_MODE_DIFFUSE,         /**< Shaded using material diffuse term */
   EVAS_3D_SHADE_MODE_FLAT,            /**< Per-vertex flat lighting */
   EVAS_3D_SHADE_MODE_PHONG,           /**< Per-pixel phong shading */
   EVAS_3D_SHADE_MODE_NORMAL_MAP,      /**< Per-pixel normal map shading */
} Evas_3D_Shade_Mode;

/**
 * Material attributes
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Material_Attrib
{
   EVAS_3D_MATERIAL_AMBIENT,     /**< Ambient term */
   EVAS_3D_MATERIAL_DIFFUSE,     /**< Diffuse term */
   EVAS_3D_MATERIAL_SPECULAR,    /**< Specular term */
   EVAS_3D_MATERIAL_EMISSION,    /**< Emission term */
   EVAS_3D_MATERIAL_NORMAL,      /**< Normal map term */
} Evas_3D_Material_Attrib;

/**
 * Mesh file type
 * @ingroup Evas_3D_Types
 */
typedef enum _Evas_3D_Mesh_File_Type
{
   EVAS_3D_MESH_FILE_TYPE_MD2,   /**< Quake's MD2 mesh file format */
} Evas_3D_Mesh_File_Type;

typedef enum _Evas_3D_Pick_Type
{
   EVAS_3D_PICK_NODE,
   EVAS_3D_PICK_MESH,
} Evas_3D_Pick_Type;

/**
 * Set the scene on an image object.
 *
 * @param obj     Image object.
 * @param scene   Scene object used as a content of the given image object.
 *
 * An image object can get its content from various sources like memory buffers,
 * image files and other evas object. A scene also can be a source for an image
 * object to display the rendered result onto evas canvas.
 *
 * Any existing content (data, file or proxy source) will be removed after this
 * call. Setting @p src to @c NULL detach the 3D scene from the image object.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI void               evas_object_image_t3d_scene_set(Evas_Object *obj, Evas_3D_Scene *scene) EINA_ARG_NONNULL(1);

/**
 * Get the current scene of an image object.
 *
 * @param obj     Image object.
 * @return        Scene object handle (if any), or @c NULL if there's no scene attached.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI Evas_3D_Scene     *evas_object_image_t3d_scene_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Create a new scene on the given Evas @p e canvas.
 *
 * @param e    The given canvas.
 * @return     The created scene handle.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI Evas_3D_Scene     *evas_3d_scene_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Delete a scene from its belonging Evas canvas.
 *
 * @param scene   The given scene to be deleted.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI void               evas_3d_scene_del(Evas_3D_Scene *scene) EINA_ARG_NONNULL(1);

/**
 * Get the Evas canvas where the given scene belongs to.
 *
 * @param scene   The given scene.
 * @return        The Evas canvas.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI Evas              *evas_3d_scene_evas_get(const Evas_3D_Scene *scene) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the root node of a scene.
 *
 * @param scene   The given scene.
 * @param node    A node which will be used as a root node for the scene.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI void               evas_3d_scene_root_node_set(Evas_3D_Scene *scene, Evas_3D_Node *node) EINA_ARG_NONNULL(1);

/**
 * Get the root node of a scene.
 *
 * @param scene   The given scene.
 * @return        The root node of the given scene.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI Evas_3D_Node      *evas_3d_scene_root_node_get(const Evas_3D_Scene *scene) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the camera node of a scene.
 *
 * @param scene   The given scene.
 * @param node    A node which will be used as a camera node for the scene.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI void               evas_3d_scene_camera_node_set(Evas_3D_Scene *scene, Evas_3D_Node *node) EINA_ARG_NONNULL(1);

/**
 * Get the camera node of a scene.
 *
 * @param scene   The given scene.
 * @return        The camera node of the given scene.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI Evas_3D_Node      *evas_3d_scene_camera_node_get(const Evas_3D_Scene *scene) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the resolution of a scene.
 *
 * @param scene   The given scene.
 * @param w       Width of the resolution.
 * @param h       Height of the resolution.
 *
 * A scene should be rendered to be displayed through an image objects. The
 * resolution defines size of the internal surface holding the rendered result.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI void               evas_3d_scene_size_set(Evas_3D_Scene *scene, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Get the internal resolution of a scene.
 *
 * @param scene   The given scene.
 * @param w       Pointer to receive width of the resolution.
 * @param h       Pointer to receive height of the resolution.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI void               evas_3d_scene_size_get(const Evas_3D_Scene *scene, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Set the background color of a scene.
 *
 * @param scene   The given scene.
 * @param r       Red component of the background color.
 * @param g       Green component of the background color.
 * @param b       Blue component of the background color.
 * @param a       Alpha component of the background color.
 *
 * Background color defines initial color of pixels before a scene is rendered.
 * If you want to display a scene with background evas objects are still
 * remaining as if it was the background, set the alpha term to 0.0.
 *
 * Default background color is (0.0, 0.0, 0.0, 0.0).
 *
 * @ingroup Evas_3D_Scene
 */
EAPI void               evas_3d_scene_background_color_set(Evas_3D_Scene *scene, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);

/**
 * Get the background color of a scene.
 *
 * @param scene   The given scene.
 * @param r       Pointer to receive red component of the background color.
 * @param g       Pointer to receive green component of the background color.
 * @param b       Pointer to receive blue component of the background color.
 * @param a       Pointer to receive alpha component of the background color.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI void               evas_3d_scene_background_color_get(const Evas_3D_Scene *scene, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);

/**
 * Get information on the most front visible mesh for the given position.
 *
 * @param scene   The given scene.
 * @param x       X coordinate of the picking position.
 * @param y       Y coordinate of the picking position.
 * @param node    Pointer to receive the node contains the picked mesh.
 * @param mesh    Pointer to receive the picked mesh.
 * @param s       Pointer to receive the texture "s" coordinate.
 * @param t       Pointer to receive the texture "t" coordinate.
 *
 * (x, y) is the screen coordinate of the given scene. That is, left-top is
 * (0, 0) and right-bottom is (w, h) where (w, h) is the size of the scene.
 * The texture coordinate is useful when using proxy texture source.
 *
 * @ingroup Evas_3D_Scene
 */
EAPI Eina_Bool          evas_3d_scene_pick(const Evas_3D_Scene *scene, Evas_Real x, Evas_Real y, Evas_3D_Node **node, Evas_3D_Mesh **mesh, Evas_Real *s, Evas_Real *t) EINA_ARG_NONNULL(1);

/**
 * Create a new node on the given Evas @p canvas.
 *
 * @param e    The given canvas.
 * @param type The type of the node.
 * @return     The created node handle.
 *
 * @ingroup Evas_3D_Node
 */
EAPI Evas_3D_Node      *evas_3d_node_add(Evas *e, Evas_3D_Node_Type type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Delete a node from its belonging Evas canvas.
 *
 * @param node The given node.
 *
 * @see evas_3d_node_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI void               evas_3d_node_del(Evas_3D_Node *node) EINA_ARG_NONNULL(1);

/**
 * Get the type of the given node.
 *
 * @param node The given node.
 * @return     The type of the given node.
 *
 * @see evas_3d_node_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI Evas_3D_Node_Type  evas_3d_node_type_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the Evas canvas where the given node belongs to.
 *
 * @param node The given node.
 * @return     The Evas canvas.
 *
 * @see evas_3d_node_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI Evas              *evas_3d_node_evas_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Add a member node to the given node.
 *
 * @param node    The given node which will be the parent.
 * @param member  Node object to be added.
 *
 * Nodes can be constructed into N-ary tree structure like other ordinary scene
 * graph. Basically a node inherit transforms from its parent.
 *
 * @see evas_3d_node_parent_get()
 *
 * @ingroup Evas_3D_Node
 */
EAPI void               evas_3d_node_member_add(Evas_3D_Node *node, Evas_3D_Node *member) EINA_ARG_NONNULL(1, 2);

/**
 * Delete a member node from the given node.
 *
 * @param node    The given node.
 * @param member  Member node to be deleted from the given node.
 *
 * @see evas_3d_node_member_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI void               evas_3d_node_member_del(Evas_3D_Node *node, Evas_3D_Node *member) EINA_ARG_NONNULL(1, 2);

/**
 * Get the parent node of the given node.
 *
 * @param node    The given node.
 * @return        The parent node of the given node.
 *
 * @see evas_3d_node_member_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI Evas_3D_Node      *evas_3d_node_parent_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the list of member nodes of the given node.
 *
 * @param node    The given node.
 * @return        The list of member nodes if any or @c NULL if there are none.
 *
 * @see evas_3d_node_member_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI const Eina_List   *evas_3d_node_member_list_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_3D_Node_Transform
 * @ingroup Evas_3D_Node
 *
 * Functions that manipulate node transforms.
 *
 * Evas_3D_Node does not provide standard 4x4 matrix to transform something.
 * Instead, one can set position, orientation and scale of a node separately.
 * A node will be first scaled and rotated and then translated according to its
 * position, orientation and scale. Each transform attributes can be set to
 * inherit from its parent or not.
 */

/**
 * Set the position of the given node.
 *
 * @param node    The given node.
 * @param x       X coordinate of the position.
 * @param y       Y coordinate of the position.
 * @param z       Z coordinate of the position.
 *
 * According to the inheritance flag, (x, y, z) can be a world space position or
 * parent space position.
 *
 * Default position is (0.0, 0.0, 0.0).
 *
 * @see evas_3d_node_position_inherit_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_position_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z) EINA_ARG_NONNULL(1);

/**
 * Set the orientation of the given node using quaternion.
 *
 * @param node    The given node.
 * @param x       X term of the orientation quaternion (w, x, y, z)
 * @param y       Y term of the orientation quaternion (w, x, y, z)
 * @param z       Z term of the orientation quaternion (w, x, y, z)
 * @param w       W term of the orientation quaternion (w, x, y, z)
 *
 * According the the inheritance flag, (w, x, y, z) can be a world space
 * orientation or parent space orientation.
 *
 * Default orientation is (1.0, 0.0, 0.0, 0.0) (identity quaternion).
 *
 * @see evas_3d_node_orientation_inherit_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_orientation_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z, Evas_Real w) EINA_ARG_NONNULL(1);

/**
 * Set the orientation of the given node using euler angle.
 *
 * @param node    The given node.
 * @param x       Rotation angle about X-axis.
 * @param y       Rotation angle about Y-axis.
 * @param z       Rotation angle about Z-axis.
 *
 * @see evas_3d_node_orientation_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_orientation_euler_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z) EINA_ARG_NONNULL(1);

/**
 * Set the orientation of the given node using axis-angle.
 *
 * @param node    The given node.
 * @param angle   Rotation angle.
 * @param x       X term of the rotation axis.
 * @param y       Y term of the rotation axis.
 * @param z       Z term of the rotation axis.
 *
 * @see evas_3d_node_orientation_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_orientation_angle_axis_set(Evas_3D_Node *node, Evas_Real angle, Evas_Real x, Evas_Real y, Evas_Real z) EINA_ARG_NONNULL(1);

/**
 * Set the scale of the given node.
 *
 * @param node    The given node.
 * @param x       Scale factor along X-axis.
 * @param y       Scale factor along Y-axis.
 * @param z       Scale factor along Z-axis.
 *
 * According to the inheritance flag, (x, y, z) can be a world space scale or
 * parent space scale. Be careful when using non-uniform scale factor with
 * inheritance, each transform attributes are not affected by other attributes.
 *
 * Default scale is (1.0, 1.0, 1.0).
 *
 * @see evas_3d_node_scale_inherit_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_scale_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z) EINA_ARG_NONNULL(1);

/**
 * Get the position of the given node.
 *
 * @param node    The given node.
 * @param x       Pointer to receive X coordinate of the position.
 * @param y       Pointer to receive Y coordinate of the position.
 * @param z       Pointer to receive Z coordinate of the position.
 *
 * @see evas_3d_node_position_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_position_get(const Evas_3D_Node *node, Evas_3D_Space space, Evas_Real *x, Evas_Real *y, Evas_Real *z) EINA_ARG_NONNULL(1);

/**
 * Get the orientation of the given node as quaternion.
 *
 * @param node    The given node.
 * @param x       Pointer to receive X term of the orientation quaternion.
 * @param y       Pointer to receive Y term of the orientation quaternion.
 * @param z       Pointer to receive Z term of the orientation quaternion.
 * @param w       Pointer to receive W term of the orientation quaternion.
 *
 * @see evas_3d_node_orientation_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_orientation_get(const Evas_3D_Node *node, Evas_3D_Space space, Evas_Real *x, Evas_Real *y, Evas_Real *z, Evas_Real *w) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_scale_get(const Evas_3D_Node *node, Evas_3D_Space space, Evas_Real *x, Evas_Real *y, Evas_Real *z) EINA_ARG_NONNULL(1);

/**
 * Set the position inheritance flag of the given node.
 *
 * @param node    The given node.
 * @param inherit Whether to inherit parent position @c EINA_TRUE or not @c EINA_FALSE.
 *
 * When inheritance is enabled, a node's world space position is determined by
 * adding the parent node's world position and the node's position, otherwise,
 * the node's position will be the world space position.
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_position_inherit_set(Evas_3D_Node *node, Eina_Bool inherit) EINA_ARG_NONNULL(1);

/**
 * Set the orientation inheritance flag of the given node.
 *
 * @param node    The given node.
 * @param inherit Whether to inherit parent orientation @c EINA_TRUE or not @c EINA_FALSE.
 *
 * When inheritance is enabled, a node's world space orientation is determined
 * by multiplying the parent node's world orientation and the node's
 * orientation, otherwise, the node's orientation will be the world space
 * orientation.
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_orientation_inherit_set(Evas_3D_Node *node, Eina_Bool inherit) EINA_ARG_NONNULL(1);

/**
 * Set the scale inheritance flag of the given node.
 *
 * @param node    The given node.
 * @param inherit Whether to inherit parent scale @c EINA_TRUE or not @c EINA_FALSE.
 *
 * When inheritance is enabled, a node's world space scale is determined by
 * multiplying the parent node's world scale and the node's scale, otherwise,
 * the node's scale will be the world space scale.
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_scale_inherit_set(Evas_3D_Node *node, Eina_Bool inherit) EINA_ARG_NONNULL(1);

/**
 * Get the position inheritance flag of the given node.
 *
 * @param node    The given node.
 * @return        @c EINA_TRUE if inheritance is enabled, or @c EINA_FALSE if not.
 *
 * @see evas_3d_node_position_inherit_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI Eina_Bool          evas_3d_node_position_inherit_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the orientation inheritance flag of the given node.
 *
 * @param node    The given node.
 * @return        @c EINA_TRUE if inheritance is enabled, or @c EINA_FALSE if not.
 *
 * @see evas_3d_node_orientation_inherit_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI Eina_Bool          evas_3d_node_orientation_inherit_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the scale inheritance flag of the given node.
 *
 * @param node    The given node.
 * @return        @c EINA_TRUE if inheritance is enabled, or @c EINA_FALSE if not.
 *
 * @see evas_3d_node_scale_inherit_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI Eina_Bool          evas_3d_node_scale_inherit_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Rotate the given node to look at desired position.
 *
 * @param node          The given node.
 * @param target_space  Space where the target position belongs to.
 * @param x             X coordinate of the target position.
 * @param y             Y coordinate of the target position.
 * @param z             Z coordinate of the target position.
 * @param up_space      Space where the up vector belongs to.
 * @param ux            X term of the up vector.
 * @param uy            Y term of the up vector.
 * @param uz            Z term of the up vector.
 *
 * This function rotate the given node so that its forward vector (negative
 * Z-axis) points to the desired position and the up vector coincide with the
 * given up vector.
 *
 * @see evas_3d_node_orientation_set()
 *
 * @ingroup Evas_3D_Node_Transform
 */
EAPI void               evas_3d_node_look_at_set(Evas_3D_Node *node, Evas_3D_Space target_space, Evas_Real x, Evas_Real y, Evas_Real z, Evas_3D_Space up_space, Evas_Real ux, Evas_Real uy, Evas_Real uz) EINA_ARG_NONNULL(1);

/**
 * Set a camera to the given node.
 *
 * @param node          The given node.
 * @param camera        The camera to be set.
 *
 * If the node is not of type EVAS_3D_NODE_TYPE_CAMERA, error message will be
 * generated and nothing happens.
 *
 * @see evas_3d_node_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI void               evas_3d_node_camera_set(Evas_3D_Node *node, Evas_3D_Camera *camera) EINA_ARG_NONNULL(1);

/**
 * Get the camera of the given node.
 *
 * @param node          The given node.
 * @return              The camera of the given node if any, or @c NULL if there're none.
 *
 * @see evas_3d_node_camera_set()
 *
 * @ingroup Evas_3D_Node
 */
EAPI Evas_3D_Camera    *evas_3d_node_camera_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the light of the given node.
 *
 * @param node          The given node.
 * @param light         The light to be set.
 *
 * If the node is not of type EVAS_3D_NODE_TYPE_LIGHT, error message will be
 * generated and nothing happens.
 *
 * @see evas_3d_node_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI void               evas_3d_node_light_set(Evas_3D_Node *node, Evas_3D_Light *light) EINA_ARG_NONNULL(1);

/**
 * Get the light of the given node.
 *
 * @param node          The given node.
 * @return              The light of the given node if any, or @c NULL if there're none.
 *
 * @see evas_3d_node_light_set()
 *
 * @ingroup Evas_3D_Node
 */
EAPI Evas_3D_Light     *evas_3d_node_light_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Add a mesh to the given node.
 *
 * @param node          The given node.
 * @param mesh          The mesh to be added.
 *
 * If the node is not of type EVAS_3D_NODE_TYPE_MESH, error message will be
 * generated and nothing happens.
 *
 * @see evas_3d_node_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI void               evas_3d_node_mesh_add(Evas_3D_Node *node, Evas_3D_Mesh *mesh) EINA_ARG_NONNULL(1);

/**
 * Delete a mesh from the given node.
 *
 * @param node          The given node.
 * @param mesh          The mesh to be deleted.
 *
 * If the node is not of type EVAS_3D_NODE_TYPE_MESH or the given mesh does not
 * belong to the given node, error message will be gnerated and nothing happens.
 *
 * @see evas_3d_node_mesh_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI void               evas_3d_node_mesh_del(Evas_3D_Node *node, Evas_3D_Mesh *mesh) EINA_ARG_NONNULL(1);

/**
 * Get the list of meshes of the given node.
 *
 * @param node          The given node.
 * @return              The list of meshes if any, or @c NULL if there're none.
 *
 * If the node is not of type EVAS_3D_NODE_TYPE_MESH, error message will be
 * generated and @c NULL will be returned. If there're no meshes in the given
 * node, @c NULL will be returned.
 *
 * @see evas_3d_node_mesh_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI const Eina_List   *evas_3d_node_mesh_list_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the animation frame number of the given node for the given mesh.
 *
 * @param node          The given node.
 * @param mesh          The given mesh.
 * @param frame         The animation frame number.
 *
 * If the node is not of type EVAS_3D_NODE_TYPE_MESH or the given mesh does not
 * belong to the given mesh error mesh will be generated and nothing happens.
 *
 * Default mesh frame is 0.
 *
 * @see evas_3d_node_mesh_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI void               evas_3d_node_mesh_frame_set(Evas_3D_Node *node, Evas_3D_Mesh *mesh, int frame) EINA_ARG_NONNULL(1);

/**
 * Set the animation frame number of the given node for the given mesh.
 *
 * @param node          The given node.
 * @param mesh          The given mesh.
 * @param frame         The animation frame number.
 *
 * If the node is not of type EVAS_3D_NODE_TYPE_MESH or the given mesh does not
 * belong to the given mesh error mesh will be generated and nothing happens.
 *
 * @see evas_3d_node_mesh_add()
 *
 * @ingroup Evas_3D_Node
 */
EAPI int                evas_3d_node_mesh_frame_get(const Evas_3D_Node *node, Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Create a new camera on the given Evas @p canvas.
 *
 * @param e          The given canvas.
 * @return           The created camera handle.
 *
 * @ingroup Evas_3D_Camera
 */
EAPI Evas_3D_Camera    *evas_3d_camera_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Delete a node from its belonging Evas canvas.
 *
 * @param camera     The given camera.
 *
 * @see evas_3d_camera_add()
 *
 * @ingroup Evas_3D_Camera
 */
EAPI void               evas_3d_camera_del(Evas_3D_Camera *camera) EINA_ARG_NONNULL(1);

/**
 * Get the Evas canvas where the given node belongs to.
 *
 * @param camera     The given camera.
 * @return           The Evas canvas.
 *
 * @see evas_3d_node_add()
 *
 * @ingroup Evas_3D_Camera
 */
EAPI Evas              *evas_3d_camera_evas_get(const Evas_3D_Camera *camera) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the projection matrix of the given camera.
 *
 * @param camera     The given camera.
 * @param matrix     Pointer to the array of 16 Evas_Real values in column major order.
 *
 * Default projection matrix is identity matrix.
 *
 * @see evas_3d_camera_projection_perspective_set()
 * @see evas_3d_camera_projection_ortho_set()
 * @see evas_3d_camera_projection_frustum_set()
 *
 * @ingroup Evas_3D_Camera
 */
EAPI void               evas_3d_camera_projection_matrix_set(Evas_3D_Camera *camera, const Evas_Real *matrix) EINA_ARG_NONNULL(1);

/**
 * Get the projection matrix of the given camera.
 *
 * @param camera     The given camera.
 * @param matrix     Pointer to receive the 16 Evas_Real values in column major order.
 *
 * @see evas_3d_camera_projection_matrix_set()
 *
 * @ingroup Evas_3D_Camera
 */
EAPI void               evas_3d_camera_projection_matrix_get(const Evas_3D_Camera *camera, Evas_Real *matrix) EINA_ARG_NONNULL(1, 2);

/**
 * Set the projection matrix of the given camera with perspective projection.
 *
 * @param camera     The given camera.
 * @param fovy       Field of view angle in Y direction.
 * @param aspect     Aspect ratio.
 * @param near       Distance to near clipping plane.
 * @param far        Distance to far clipping plane.
 *
 * @see evas_3d_camera_projection_matrix_set()
 *
 * @ingroup Evas_3D_Camera
 */
EAPI void               evas_3d_camera_projection_perspective_set(Evas_3D_Camera *camera, Evas_Real fovy, Evas_Real aspect, Evas_Real near, Evas_Real far) EINA_ARG_NONNULL(1);

/**
 * Set the projection matrix of the given camera with frustum projection.
 *
 * @param camera     The given camera.
 * @param left       Left X coordinate of the near clipping plane.
 * @param right      Right X coordinate of the near clipping plane.
 * @param top        Top Y coordinate of the near clipping plane.
 * @param bottom     Bottom Y coordinate of the near clipping plane.
 * @param near       Distance to near clipping plane.
 * @param far        Distance to far clipping plane.
 *
 * @see evas_3d_camera_projection_matrix_set()
 *
 * @ingroup Evas_3D_Camera
 */
EAPI void               evas_3d_camera_projection_frustum_set(Evas_3D_Camera *camera, Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top, Evas_Real near, Evas_Real far) EINA_ARG_NONNULL(1);

/**
 * Set the projection matrix of the given camera with orthogonal projection.
 *
 * @param camera     The given camera.
 * @param left       Left X coordinate of the near clipping plane.
 * @param right      Right X coordinate of the near clipping plane.
 * @param top        Top Y coordinate of the near clipping plane.
 * @param bottom     Bottom Y coordinate of the near clipping plane.
 * @param near       Distance to near clipping plane.
 * @param far        Distance to far clipping plane.
 *
 * @see evas_3d_camera_projection_matrix_set()
 *
 * @ingroup Evas_3D_Camera
 */
EAPI void               evas_3d_camera_projection_ortho_set(Evas_3D_Camera *camera, Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top, Evas_Real near, Evas_Real far) EINA_ARG_NONNULL(1);

/**
 * Create a new light on the given Evas @p canvas.
 *
 * @param e          The given canvas.
 * @return           The created light handle.
 *
 * @ingroup Evas_3D_Light
 */
EAPI Evas_3D_Light     *evas_3d_light_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Delete a node from its belonging Evas canvas.
 *
 * @param light      The given light.
 *
 * @see evas_3d_light_add()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_del(Evas_3D_Light *light) EINA_ARG_NONNULL(1);

/**
 * Get the Evas canvas where the given node belongs to.
 *
 * @param light      The given light.
 * @return           The Evas canvas.
 *
 * @see evas_3d_node_add()
 *
 * @ingroup Evas_3D_Light
 */
EAPI Evas              *evas_3d_light_evas_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the directional flag of the given light.
 *
 * @param light         The given light.
 * @param directional   Whether the light is directional (@c EINA_TRUE), or not (@c EINA_FALSE).
 *
 * Directional light is a type of light which is infinitely far away with no
 * attenuation. The light direction is determined by the containing node's
 * forward vector (negative Z-axis).
 *
 * By default, directional is not enabled.
 *
 * @see evas_3d_node_look_at_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_directional_set(Evas_3D_Light *light, Eina_Bool directional) EINA_ARG_NONNULL(1);

/**
 * Get the directional flag of the given light.
 *
 * @param light      The given light.
 * @return           @c EINA_TRUE if the light is directional or @c EINA_FALSE if not.
 *
 * @see evas_3d_light_directional_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI Eina_Bool          evas_3d_light_directional_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the ambient color of the given light.
 *
 * @param light      The given light.
 * @param r          Red component of the ambient color between [0.0, 1.0].
 * @param g          Green component of the ambient color between [0.0, 1.0].
 * @param b          Blue component of the ambient color between [0.0, 1.0].
 * @param a          Alpha component of the ambient color between [0.0, 1.0].
 *
 * Default ambient color is (0.0, 0.0, 0.0, 1.0).
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_ambient_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);

/**
 * Get the ambient color of the given light.
 *
 * @param light      The given light.
 * @param r          Pointer to receive the red component of the ambient color.
 * @param g          Pointer to receive the green component of the ambient color.
 * @param b          Pointer to receive the blue component of the ambient color.
 * @param a          Pointer to receive the alpha component of the ambient color.
 *
 * @see evas_3d_light_ambient_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_ambient_get(const Evas_3D_Light *light, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);

/**
 * Set the diffuse color of the given light.
 *
 * @param light      The given light.
 * @param r          Red component of the diffuse color between [0.0, 1.0].
 * @param g          Green component of the diffuse color between [0.0, 1.0].
 * @param b          Blue component of the diffuse color between [0.0, 1.0].
 * @param a          Alpha component of the diffuse color between [0.0, 1.0].
 *
 * Default diffuse color is (1.0, 1.0, 1.0, 1.0).
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_diffuse_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);

/**
 * Get the diffuse color of the given light.
 *
 * @param light      The given light.
 * @param r          Pointer to receive the red component of the diffuse color.
 * @param g          Pointer to receive the green component of the diffuse color.
 * @param b          Pointer to receive the blue component of the diffuse color.
 * @param a          Pointer to receive the alpha component of the diffuse color.
 *
 * @see evas_3d_light_diffuse_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_diffuse_get(const Evas_3D_Light *light, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);

/**
 * Get the specular color of the given light.
 *
 * @param light      The given light.
 * @param r          Pointer to receive the red component of the specular color.
 * @param g          Pointer to receive the green component of the specular color.
 * @param b          Pointer to receive the blue component of the specular color.
 * @param a          Pointer to receive the alpha component of the specular color.
 *
 * Default specular color is (1.0, 1.0, 1.0, 1.0).
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_specular_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);

/**
 * Get the specular color of the given light.
 *
 * @param light      The given light.
 * @param r          Pointer to receive the red component of the specular color.
 * @param g          Pointer to receive the green component of the specular color.
 * @param b          Pointer to receive the blue component of the specular color.
 * @param a          Pointer to receive the alpha component of the specular color.
 *
 * @see evas_3d_light_specular_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_specular_get(const Evas_3D_Light *light, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);

/**
 * Set the spot exponent of the given light.
 *
 * @param light      The given light.
 * @param exponent   Spot exponent value.
 *
 * Higher spot exponent means intensity at the center of the cone is relatively
 * stronger. Zero exponent means the light intensity is evenly distibuted. The
 * spot exponent has no effect when the light is not spot light (spot cutoff
 * angle is less than 180 degree).
 *
 * Default spot exponent is 0.
 *
 * @see evas_3d_light_spot_cutoff_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_spot_exponent_set(Evas_3D_Light *light, Evas_Real exponent) EINA_ARG_NONNULL(1);

/**
 * Get the spot exponent of the given light.
 *
 * @param light      The given light.
 * @return           The spot exponent value.
 *
 * @see evas_3d_light_spot_exponent_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI Evas_Real          evas_3d_light_spot_exponent_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the spot cutoff angle of the given light.
 *
 * @param light      The given light.
 * @param cutoff     Cutoff angle in degree.
 *
 * Only angle less than 180 degree will make it spot light, so that other spot
 * light attribute will take effect.
 *
 * Default spot cutoff angle is 180.
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_spot_cutoff_set(Evas_3D_Light *light, Evas_Real cutoff) EINA_ARG_NONNULL(1);

/**
 * Get the spot cutoff angle of the given light.
 *
 * @param light      The given light.
 * @return           Cutoff angle in degree.
 *
 * @see evas_3d_light_spot_cutoff_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI Evas_Real          evas_3d_light_spot_cutoff_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the attenuation of the given light.
 *
 * @param light      The given light.
 * @param constant   Constant attenuation term.
 * @param linear     Linear attenuation term.
 * @param quadratic  Quadratic attenuation term.
 *
 * Light attenuation has no effect with directional light. And the attenuation
 * should be enabled first to take effect. The attenuation factor is calculated
 * as follows.
 *
 * atten = 1.0 / constant + linear * distance + quadratic * distance * distance
 *
 * Default attenuation is constant = 1.0, linear = 0.0, quadratic = 0.0.
 *
 * @see evas_3d_light_attenuation_enable_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_attenuation_set(Evas_3D_Light *light, Evas_Real constant, Evas_Real linear, Evas_Real quadratic) EINA_ARG_NONNULL(1);

/**
 * Get the attenuation of the given light.
 *
 * @param light      The given light.
 * @param constant   Pointer to receive constant attenuation term.
 * @param linear     Pointer to receive linear attenuation term.
 * @param quadratic  Pointer to receive quadratic attenuation term.
 *
 * @see evas_3d_light_attenuation_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_attenuation_get(const Evas_3D_Light *light, Evas_Real *constant, Evas_Real *linear, Evas_Real *quadratic) EINA_ARG_NONNULL(1);

/**
 * Set the attenuation enable flag of the given light.
 *
 * @param light      The given light.
 * @param enable     Whether to enable attenuation (@c EINA_TRUE), or not (@c EINA_FALSE).
 *
 * By default, light attenuation is not enabled.
 *
 * @see evas_3d_light_attenuation_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI void               evas_3d_light_attenuation_enable_set(Evas_3D_Light *light, Eina_Bool enable) EINA_ARG_NONNULL(1);

/**
 * Get the attenuation enable flag of the given light.
 *
 * @param light      The given light.
 * @return           @c EINA_TRUE if enabled, or @c EINA_FALSE if not.
 *
 * @see evas_3d_light_attenuation_enable_set()
 *
 * @ingroup Evas_3D_Light
 */
EAPI Eina_Bool          evas_3d_light_attenuation_enable_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Create a new mesh on the given Evas @p canvas.
 *
 * @param e       The given canvas.
 * @return        The created mesh handle.
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI Evas_3D_Mesh      *evas_3d_mesh_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Delete a mesh from its belonging Evas canvas.
 *
 * @param mesh    The given mesh.
 *
 * @see evas_3d_mesh_add()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_del(Evas_3D_Mesh *mesh) EINA_ARG_NONNULL(1);

/**
 * Get the Evas canvas where the given node belongs to.
 *
 * @param mesh    The given mesh.
 * @return        The Evas canvas.
 *
 * @see evas_3d_mesh_add()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI Evas              *evas_3d_mesh_evas_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the shade mode of the given mesh.
 *
 * @param mesh    The given mesh.
 * @param mode    The shade mode.
 *
 * Default shade mode is EVAS_3D_SHADE_MODE_VERTEX_COLOR.
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_shade_mode_set(Evas_3D_Mesh *mesh, Evas_3D_Shade_Mode mode) EINA_ARG_NONNULL(1);

/**
 * Get the shade mode of the given mesh.
 *
 * @param mesh    The given mesh.
 * @return        The shade mode.
 *
 * @see eavs_3d_mesh_shade_mode_set()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI Evas_3D_Shade_Mode evas_3d_mesh_shade_mode_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Load mesh data from file.
 *
 * @param mesh    The given mesh.
 * @param type    The type of the mesh file.
 * @param file    Path to the mesh file.
 * @param key     Key in the mesh file.
 *
 * Loading a mesh from existing file is supported. Currently, only MD2 file
 * format is supported.
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_file_set(Evas_3D_Mesh *mesh, Evas_3D_Mesh_File_Type type, const char *file, const char *key) EINA_ARG_NONNULL(1);

/**
 * Set the vertex count of the given mesh.
 *
 * @param mesh    The given mesh.
 * @param count   Vertex count.
 *
 * Each key frame should have same vertex count to be properly interpolated.
 * Key frames have their own vertex data and the data should have more vertices
 * than the mesh's vertex count.
 *
 * Default vertex count is 0.
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_vertex_count_set(Evas_3D_Mesh *mesh, unsigned int count) EINA_ARG_NONNULL(1);

/**
 * Get the vertex count of the given mesh.
 *
 * @param mesh    The given mesh.
 * @return        Vertex count.
 *
 * @see evas_3d_mesh_vertex_count_set()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI int                evas_3d_mesh_vertex_count_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Add a key frame to the given mesh.
 *
 * @param mesh    The given mesh.
 * @param frame   The number of the key frame to be added.
 *
 * If specified frame is already exist, error message will be generated.
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_frame_add(Evas_3D_Mesh *mesh, int frame) EINA_ARG_NONNULL(1);

/**
 * Delete a key frame from the given mesh.
 *
 * @param mesh    The given mesh.
 * @param frame   The number of the key frame to be deleted.
 *
 * @see evas_3d_mesh_frame_add()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_frame_del(Evas_3D_Mesh *mesh, int frame) EINA_ARG_NONNULL(1);

/**
 * Set the material of the key frame of the given mesh.
 *
 * @param mesh       The given mesh.
 * @param frame      The number of the key frame.
 * @param material   The material to be set to the key frame.
 *
 * Setting different materials for each key frame is useful for doing animations
 * like GIF images or color changing animationas.
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_frame_material_set(Evas_3D_Mesh *mesh, int frame, Evas_3D_Material *material) EINA_ARG_NONNULL(1);

/**
 * Get the material of the key frame of the given mesh.
 *
 * @param mesh       The given mesh.
 * @param frame      The number of the key frame.
 * @return           The material of the key frame.
 *
 * @see evas_3d_mesh_frame_material_set()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI Evas_3D_Material  *evas_3d_mesh_frame_material_get(const Evas_3D_Mesh *mesh, int frame) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the vertex data of the key frame of the given mesh.
 *
 * @param mesh       The given mesh.
 * @param frame      The number of the key frame.
 * @param attrib     Vertex attribute ID.
 * @param stride     Stride to go to the next vertex (in bytes).
 * @param data       Pointer to the vertex data buffer.
 *
 * This function make evas read from the given buffer whenever it requires.
 * If you want to release the buffer after calling this functions, use
 * evas_3d_mesh_frame_vertex_data_copy_set() instead.
 *
 * After setting the vertex data, further modifications should be protected
 * by map/unmap pair.
 *
 * @see evas_3d_mesh_frame_vertex_data_copy_set()
 * @see evas_3d_mesh_frame_vertex_data_map()
 * @see evas_3d_mesh_frame_vertex_data_unmap()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_frame_vertex_data_set(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib, int stride, const void *data) EINA_ARG_NONNULL(1);

/**
 * Set the vertex data of the key frame of the given mesh by copying from a buffer.
 *
 * @param mesh       The given mesh.
 * @param frame      The number of the key frame.
 * @param attrib     Vertex attribute ID.
 * @param stride     Stride to go to the next vertex (in bytes).
 * @param data       Pointer to the vertex data buffer.
 *
 * This function allocates internal vertex buffer and copy from the given
 * buffer. So you can release the buffer. If you want to modify the vertex data
 * use evas_3d_mesh_frame_vertex_data_map(). After finishing the modifications,
 * you should call evas_3d_mesh_frame_vertex_data_unmap().
 *
 * @see evas_3d_mesh_frame_vertex_data_set()
 * @see evas_3d_mesh_frame_vertex_data_map()
 * @see evas_3d_mesh_frame_vertex_data_unmap()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_frame_vertex_data_copy_set(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib, int stride, const void *data) EINA_ARG_NONNULL(1);

/**
 * Map the vertex buffer of the key frame of the given mesh.
 *
 * @param mesh       The given mesh.
 * @param frame      The number of the key frame.
 * @param attrib     Vertex attribute ID.
 * @return           Starting address of the mapped vertex buffer.
 *
 * After manipulating the mapped buffer, evas_3d_mesh_frame_vertex_data_unmap()
 * should be called to properly download the data to the engine. If the data
 * was set using evas_3d_mesh_frame_vertex_data_set(), pointer to the original
 * buffer will be returned. Otherwise, the returned pointer can differ every
 * time calling this function.
 *
 * @see evas_3d_mesh_frame_vertex_data_unmap()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void              *evas_3d_mesh_frame_vertex_data_map(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Unmap the vertex buffer of the key frame of the given mesh.
 *
 * @param mesh       The given mesh.
 * @param frame      The number of the key frame.
 * @param attrib     Vertex attribute ID.
 *
 * @see evas_3d_mesh_frame_vertex_data_map()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_frame_vertex_data_unmap(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib) EINA_ARG_NONNULL(1);

/**
 * Get the vertex buffer stride of the key frame of the given mesh.
 *
 * @param mesh       The given mesh.
 * @param frame      The number of the key frame.
 * @param attrib     Vertex attribute ID.
 * @return           Stride to go to the next vertex (in bytes).
 *
 * This function returns valid stride only when the vertex buffer is mapped.
 * If the data was set with evas_3d_mesh_frame_vertex_data_set(), the original
 * stride will be returned unchanged.
 *
 * @see evas_3d_mesh_frame_vertex_data_map()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI int                evas_3d_mesh_frame_vertex_stride_get(const Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the vertex index data of the given mesh.
 *
 * @param mesh       The given mesh.
 * @param format     Vertex index data format.
 * @param count      Vertex index count.
 * @param indices    Pointer to the index data.
 *
 * When the index data is set, Evas 3D assembles vertices using the index data.
 * If you want to free the data buffer, use evas_3d_mesh_index_data_copy_set().
 * Further modifications should be made within map/unmap pair.
 *
 * @see evas_3d_mesh_index_data_copy_set()
 * @see evas_3d_mesh_index_data_map()
 * @see evas_3d_mesh_index_data_unmap()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_index_data_set(Evas_3D_Mesh *mesh, Evas_3D_Index_Format format, int count, const void *indices) EINA_ARG_NONNULL(1);

/**
 * Set the vertex index data of the given mesh by copying from a buffer.
 *
 * @param mesh       The given mesh.
 * @param format     Vertex index data format.
 * @param count      Vertex index count.
 * @param indices    Pointer to the vertex data.
 *
 * This function allocates internal index buffer any copy data from the given
 * buffer. Futher modifications can be made within map/unmap pair.
 *
 * @see evas_3d_mesh_index_data_set()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_index_data_copy_set(Evas_3D_Mesh *mesh, Evas_3D_Index_Format format, int count, const void *indices) EINA_ARG_NONNULL(1);

/**
 * Get the format of the index data of the given mesh.
 *
 * @param mesh       The given mesh.
 * @return           Format of the index data.
 *
 * Returns valid format only when the index buffer is mapped. First map the
 * index buffer and then query the properties of the mapped buffer. If the index
 * data was set by evas_3d_mesh_index_data_set(), the original format will be
 * returned. Otherwise the format can differ every time you call the
 * evas_3d_mesh_index_data_map() function.
 *
 * @see evas_3d_mesh_index_data_map()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI Evas_3D_Index_Format evas_3d_mesh_index_format_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the count of the index data of the given mesh.
 *
 * @param mesh       The given mesh.
 * @return           Index data count.
 *
 * This function returns the index count of the last called data_set function.
 *
 * @see evas_3d_mesh_index_data_set()
 * @see evas_3d_mesh_index_data_copy_set()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI int                evas_3d_mesh_index_count_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Map the index buffer of the given mesh.
 *
 * @param mesh       The given mesh.
 * @return           Pointer to the mapped buffer.
 *
 * evas_3d_mesh_index_data_unmap() should be called after modifications. If the
 * data was set using evas_3d_mesh_index_data_set(), the original pointer will
 * be returned, otherwise, the returned pointer may differ every time you call
 * this function.
 *
 * @see evas_3d_mesh_index_data_unmap()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void              *evas_3d_mesh_index_data_map(Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Unmap the index buffer of the given mesh.
 *
 * @param mesh       The given mesh.
 *
 * @see evas_3d_mesh_index_data_map()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_index_data_unmap(Evas_3D_Mesh *mesh) EINA_ARG_NONNULL(1);

/**
 * Set the vertex assembly of the given mesh.
 *
 * @param mesh       The given mesh.
 * @param assembly   Vertex assembly.
 *
 * Vertex assembly defines how the engine organizes vertices into geometric
 * primitives.
 *
 * Default vertex assembly is EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES.
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI void               evas_3d_mesh_vertex_assembly_set(Evas_3D_Mesh *mesh, Evas_3D_Vertex_Assembly assembly);

/**
 * Get the vertex assembly of the given mesh.
 *
 * @param mesh       The given mesh.
 * @return           The vertex assembly.
 *
 * @see evas_3d_mesh_vertex_assembly_set()
 *
 * @ingroup Evas_3D_Mesh
 */
EAPI Evas_3D_Vertex_Assembly evas_3d_mesh_vertex_assembly_get(const Evas_3D_Mesh *mesh);

/**
 * Create a new texture on the given Evas @p canvas.
 *
 * @param e    The given canvas.
 * @return     The created texture handle.
 *
 * @ingroup Evas_3D_Texture
 */
EAPI Evas_3D_Texture   *evas_3d_texture_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Delete a texture from its belonging Evas canvas.
 *
 * @param texture The given texture.
 *
 * @see evas_3d_texture_add()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_del(Evas_3D_Texture *texture) EINA_ARG_NONNULL(1);

/**
 * Get the Evas canvas where the given texture belongs to.
 *
 * @param texture The given texture.
 * @return        The Evas canvas.
 *
 * @see evas_3d_texture_add()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI Evas              *evas_3d_texture_evas_get(const Evas_3D_Texture *texture) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the data of the given texture.
 *
 * @param texure        The given texture
 * @param color_format  Color format of the texture.
 * @param pixel_format  Pixel format of the data.
 * @param w             Width of the data.
 * @param h             Height of the data.
 * @param data          Pointer to the data.
 *
 * @see evas_3d_texture_file_set()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_data_set(Evas_3D_Texture *texture, Evas_3D_Color_Format color_format, Evas_3D_Pixel_Format pixel_format, int w, int h, const void *data);

/**
 * Set the data of the given texture from file.
 *
 * @param texture       The given texture.
 * @param file          Path to the image file.
 * @param key           Key in the image file.
 *
 * Only PNG format is supported.
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_file_set(Evas_3D_Texture *texture, const char *file, const char *key) EINA_ARG_NONNULL(1);

/**
 * Set the data of the given texture from an evas object.
 *
 * @param texture       The given texture.
 * @param source        Source evas object to be used as the texture data.
 *
 * Evas 3D support using existing evas object as a texture source. This feature
 * make it possible using any exisiting evas object inside 3D scene.
 *
 * @see evas_3d_texture_source_visible_set
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_source_set(Evas_3D_Texture *texture, Evas_Object *source) EINA_ARG_NONNULL(1);

/**
 * Set the visibility flag of the source evas object of the given texture.
 *
 * @param texture       The given texture.
 * @param visible       @c EINA_TRUE for visible, @c EINA_FALSE for invisible.
 *
 * Recommend to call evas_object_show() on the source object and controll the
 * visibility using this function.
 *
 * By default, source object is visible.
 *
 * @see evas_3d_texture_source_set()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_source_visible_set(Evas_3D_Texture *texture, Eina_Bool visible) EINA_ARG_NONNULL(1);

/**
 * Get the visibility flag of the source evas object of the given texture.
 *
 * @param texture       The given texture.
 * @return              @c EINA_TRUE if visible, @c EINA_FALSE if invisible.
 *
 * @see evas_3d_texture_source_visible_set()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI Eina_Bool          evas_3d_texture_source_visible_get(const Evas_3D_Texture *texture) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the color format of the given texture.
 *
 * @param texture       The given texture.
 *
 * EVAS_3D_COLOR_FORMAT_RGBA will be returned if the texture has source object.
 * Otherwise, the color format of the data will be returned.
 *
 * @see evas_3d_texture_data_set()
 * @see evas_3d_texture_file_set()
 * @see evas_3d_texture_source_set()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI Evas_3D_Color_Format evas_3d_texture_color_format_get(const Evas_3D_Texture *texture) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the size of the given texture.
 *
 * @param texture       The given texture.
 * @param w             Pointer to receive the width of the texture size.
 * @param h             Pointer to receive the height of the texture size.
 *
 * If the texture has source object, the size of the source object will be
 * returned. Otherwise, the size of the data (or image file) will be returned.
 *
 * @see evas_3d_texture_data_set()
 * @see evas_3d_texture_file_set()
 * @see evas_3d_texture_source_set()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_size_get(const Evas_3D_Texture *texture, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Set the wrap mode of the given texture.
 *
 * @param texture       The given texture.
 * @param s             Wrap mode for S-axis.
 * @param t             Wrap mode for T-axis.
 *
 * If the texture coordinate exceed range [0.0, 1.0] the values are modified
 * according to the wrap mode.
 *
 * Default wrap modes are both EVAS_3D_WRAP_MODE_CLAMP for s and t.
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_wrap_set(Evas_3D_Texture *texture, Evas_3D_Wrap_Mode s, Evas_3D_Wrap_Mode t) EINA_ARG_NONNULL(1);

/**
 * Get the wrap mode of the given texture.
 *
 * @param texture       The given texture.
 * @param s             Pointer to receive S-axis wrap mode.
 * @param t             Pointer to receive T-axis wrap mode.
 *
 * @see evas_3d_texture_wrap_set()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_wrap_get(const Evas_3D_Texture *texture, Evas_3D_Wrap_Mode *s, Evas_3D_Wrap_Mode *t) EINA_ARG_NONNULL(1);

/**
 * Set the filter of the given texture.
 *
 * @param texture       The given texture.
 * @param min           Minification filter used when down-scaling.
 * @param mag           Magnification filter used when up-scaling.
 *
 * Default filters are both EVAS_3D_TEXTURE_FILTER_NEAREST for s and t.
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_filter_set(Evas_3D_Texture *texture, Evas_3D_Texture_Filter min, Evas_3D_Texture_Filter mag) EINA_ARG_NONNULL(1);

/**
 * Get the filter of the given texture.
 *
 * @param texture       The given texture.
 * @param min           Pointer to receive the minification filter.
 * @param mag           Pointer to receive the magnification filter.
 *
 * @see evas_3d_texture_filter_set()
 *
 * @ingroup Evas_3D_Texture
 */
EAPI void               evas_3d_texture_filter_get(const Evas_3D_Texture *texture, Evas_3D_Texture_Filter *min, Evas_3D_Texture_Filter *mag) EINA_ARG_NONNULL(1);

/**
 * Create a new material on the given Evas @p canvas.
 *
 * @param e    The given canvas.
 * @param type The type of the material.
 * @return     The created material handle.
 *
 * @ingroup Evas_3D_Material
 */
EAPI Evas_3D_Material  *evas_3d_material_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Delete a material from its belonging Evas canvas.
 *
 * @param material The given material.
 *
 * @see evas_3d_material_add()
 *
 * @ingroup Evas_3D_Material
 */
EAPI void               evas_3d_material_del(Evas_3D_Material *material) EINA_ARG_NONNULL(1);

/**
 * Get the Evas canvas where the given material belongs to.
 *
 * @param material The given material.
 * @return     The Evas canvas.
 *
 * @see evas_3d_material_add()
 *
 * @ingroup Evas_3D_Material
 */
EAPI Evas              *evas_3d_material_evas_get(const Evas_3D_Material *material) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the material attribute enable flag of the given material.
 *
 * @param material      The given material.
 * @param attrib        Material attribute ID.
 * @param enable        Whether to enable the attribute (@c EINA_TRUE), or not (@c EINA_FALSE).
 *
 * You might want to disable some material reflection contribution. For
 * example,Emission attribute is rarely used. Disabling unused attributes
 * might help the shading less complex so that can get speed up.
 *
 * By default, diffuse and specular is enabled.
 *
 * @ingroup Evas_3D_Material
 */
EAPI void               evas_3d_material_enable_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib, Eina_Bool enable) EINA_ARG_NONNULL(1);

/**
 * Get the material attribute enable flag of the given material.
 *
 * @param material      The given material.
 * @param attrib        Material attribute ID.
 * @return              @c EINA_TRUE if enabled, or @c EINA_FALSE if not.
 *
 * @see evas_3d_material_enable_set()
 *
 * @ingroup Evas_3D_Material
 */
EAPI Eina_Bool          evas_3d_material_enable_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the material attribute color of the given material.
 *
 * @param material      The given material.
 * @param attrib        Material attribute ID.
 * @param r             Red component of the color.
 * @param g             Green component of the color.
 * @param b             Blue component of the color.
 * @param a             Alpha component of the color.
 *
 * Material color is used also when texture map is enabled. The colors will be
 * modulated (multiplied). To controll the color contribution of a material
 * attribute, use gray color. Setting color value for normal attribute has no
 * effect.
 *
 * Default color is as follows.
 *
 * Ambient  : (0.2, 0.2, 0.2, 1.0)
 * Diffuse  : (0.8, 0.8, 0.8, 1.0)
 * Specular : (1.0, 1.0, 1.0, 1.0)
 * Emission : (0.0, 0.0, 0.0, 1.0)
 * Normal   : Not used
 *
 * @ingroup Evas_3D_Material
 */
EAPI void               evas_3d_material_color_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);

/**
 * Get the material attribute color of the given material.
 *
 * @param material      The given material.
 * @param attrib        Material attribute ID.
 * @param r             Pointer to receive red component of the color.
 * @param g             Pointer to receive green component of the color.
 * @param b             Pointer to receive blue component of the color.
 * @param a             Pointer to receive alpha component of the color.
 *
 * @see evas_3d_material_color_set()
 *
 * @ingroup Evas_3D_Material
 */
EAPI void               evas_3d_material_color_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);

/**
 * Set the shininess of the given material.
 *
 * @param material      The given material.
 * @param shininess     Shininess value.
 *
 * Shininess is only used when specular attribute is enabled. Higher shininess
 * value will make the object more shiny.
 *
 * Default shininess value is 150.0.
 *
 * @see evas_3d_material_enable_set()
 *
 * @ingroup Evas_3D_Material
 */
EAPI void               evas_3d_material_shininess_set(Evas_3D_Material *material, Evas_Real shininess) EINA_ARG_NONNULL(1);

/**
 * Get the shininess of the given material.
 *
 * @param material      The given material.
 * @return              The shininess value.
 *
 * @see evas_3d_material_shininess_set()
 *
 * @ingroup Evas_3D_Material
 */
EAPI Evas_Real          evas_3d_material_shininess_get(const Evas_3D_Material *material) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the texture of the given material.
 *
 * @param material      The given material.
 * @param attrib        Material attribute ID.
 * @param texture       Texture to be set.
 *
 * You have to enable the desired attribute first.
 *
 * @see evas_3d_material_enable_set()
 *
 * @ingroup Evas_3D_Material
 */
EAPI void               evas_3d_material_texture_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib, Evas_3D_Texture *texture) EINA_ARG_NONNULL(1);

/**
 * Get the texture of the given material.
 *
 * @param material      The given material.
 * @param attrib        Material attribute ID.
 * @return              The texture that is set to the given material attribute.
 *
 * @see evas_3d_material_texture_set()
 *
 * @ingroup Evas_3D_Material
 */
EAPI Evas_3D_Texture   *evas_3d_material_texture_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

#endif /* _EVAS_3D_H */
