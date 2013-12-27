#ifndef EVAS_PRIVATE_H
# error You shall not include this header directly
#endif

#include "Evas_3D.h"
#include "evas_3d_utils.h"

#define EVAS_3D_VERTEX_ATTRIB_COUNT    5
#define EVAS_3D_MATERIAL_ATTRIB_COUNT  5

typedef struct _Evas_3D_Object         Evas_3D_Object;
typedef struct _Evas_3D_Scene_Data     Evas_3D_Scene_Data;
typedef struct _Evas_3D_Vertex_Buffer  Evas_3D_Vertex_Buffer;
typedef struct _Evas_3D_Mesh_Frame     Evas_3D_Mesh_Frame;
typedef struct _Evas_3D_Node_Mesh      Evas_3D_Node_Mesh;
typedef struct _Evas_3D_Object_Func    Evas_3D_Object_Func;
typedef struct _Evas_3D_Pick_Data      Evas_3D_Pick_Data;
typedef struct _Evas_3D_Interpolate_Vertex_Buffer Evas_3D_Interpolate_Vertex_Buffer;

typedef Eina_Bool (*Evas_3D_Node_Func)(Evas_3D_Node *, void *data);

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

typedef enum _Evas_3D_Node_Traverse_Type
{
   EVAS_3D_NODE_TRAVERSE_DOWNWARD,
   EVAS_3D_NODE_TRAVERSE_UPWARD,
} Evas_3D_Node_Traverse_Type;

typedef enum _Evas_3D_Tree_Traverse_Type
{
   EVAS_3D_TREE_TRAVERSE_PRE_ORDER,
   EVAS_3D_TREE_TRAVERSE_ANY_ORDER = EVAS_3D_TREE_TRAVERSE_PRE_ORDER,
   EVAS_3D_TREE_TRAVERSE_POST_ORDER,
   EVAS_3D_TREE_TRAVERSE_LEVEL_ORDER,
} Evas_3D_Tree_Traverse_Type;

struct _Evas_3D_Object_Func
{
   void  (*free)(Evas_3D_Object *obj);
   void  (*change)(Evas_3D_Object *obj, Evas_3D_State state, Evas_3D_Object *ref);
   void  (*update)(Evas_3D_Object *obj);
};

struct _Evas_3D_Object
{
   Evas                *evas;

   Evas_3D_Object_Type  type;
   int                  ref_count;
   Evas_3D_Object_Func  func;

   Eina_Bool            dirty[EVAS_3D_STATE_MAX];
};

struct _Evas_3D_Scene
{
   Evas_3D_Object    base;

   Evas_3D_Node     *root_node;
   Evas_3D_Node     *camera_node;
   Evas_Color        bg_color;

   void             *surface;
   int               w, h;
   Eina_List        *images;
};

struct _Evas_3D_Node_Mesh
{
   Evas_3D_Node           *node;
   Evas_3D_Mesh           *mesh;
   int                     frame;
};

struct _Evas_3D_Node
{
   Evas_3D_Object    base;

   Eina_List        *members;
   Evas_3D_Node     *parent;

   Evas_Vec3         position;
   Evas_Vec4         orientation;
   Evas_Vec3         scale;

   Evas_Vec3         position_world;
   Evas_Vec4         orientation_world;
   Evas_Vec3         scale_world;

   Eina_Bool         position_inherit;
   Eina_Bool         orientation_inherit;
   Eina_Bool         scale_inherit;

   Evas_Box3         aabb;

   Evas_3D_Node_Type type;

   /* Camera node. */
   union {
        struct {
             Evas_3D_Camera  *camera;
             Evas_Mat4        matrix_world_to_eye;
        } camera;

        struct {
             Evas_3D_Light   *light;
             Evas_Mat4        matrix_local_to_world;
        } light;

        struct {
             Eina_List       *meshes;
             Eina_Hash       *node_meshes;
             Evas_Mat4        matrix_local_to_world;
        } mesh;
   } data;

   /* Scene using this node as root. */
   Eina_Hash        *scenes_root;

   /* Scene using this node as camera. */
   Eina_Hash        *scenes_camera;
};

struct _Evas_3D_Camera
{
   Evas_3D_Object base;

   Evas_Mat4      projection;
   Eina_Hash     *nodes;
};

struct _Evas_3D_Light
{
   Evas_3D_Object base;

   Evas_Color     ambient;
   Evas_Color     diffuse;
   Evas_Color     specular;

   Eina_Bool      directional;
   Evas_Real      spot_exp;
   Evas_Real      spot_cutoff;
   Evas_Real      spot_cutoff_cos;

   Eina_Bool      enable_attenuation;
   Evas_Real      atten_const;
   Evas_Real      atten_linear;
   Evas_Real      atten_quad;

   Eina_Hash     *nodes;
};

struct _Evas_3D_Vertex_Buffer
{
   int         element_count;
   int         stride;
   void       *data;
   int         size;
   Eina_Bool   owns_data;
   Eina_Bool   mapped;
};

struct _Evas_3D_Interpolate_Vertex_Buffer
{
   void       *data0;
   int         stride0;
   int         size0;

   void       *data1;
   int         stride1;
   int         size1;

   Evas_Real   weight;
};

struct _Evas_3D_Mesh_Frame
{
   Evas_3D_Mesh           *mesh;

   int                     frame;
   Evas_3D_Material       *material;
   Evas_Box3               aabb;

   Evas_3D_Vertex_Buffer   vertices[EVAS_3D_VERTEX_ATTRIB_COUNT];
};

struct _Evas_3D_Mesh
{
   Evas_3D_Object          base;

   Evas_3D_Shade_Mode      shade_mode;

   int                     vertex_count;
   int                     frame_count;
   Eina_List              *frames;

   Evas_3D_Index_Format    index_format;
   int                     index_count;
   void                   *indices;
   int                     index_size;
   Eina_Bool               owns_indices;
   Eina_Bool               index_mapped;

   Evas_3D_Vertex_Assembly assembly;

   Eina_Hash              *nodes;
};

struct _Evas_3D_Texture
{
   Evas_3D_Object    base;

   /* List of materials using this texture. */
   Eina_Hash        *materials;

   /* Proxy data. */
   Evas_Object      *source;
   Eina_Bool         proxy_rendering;
   void             *proxy_surface;

   /* Engine-side object. */
   void             *engine_data;
};

struct _Evas_3D_Material
{
   Evas_3D_Object    base;

   struct {
        Eina_Bool          enable;
        Evas_Color         color;
        Evas_3D_Texture   *texture;
   } attribs[EVAS_3D_MATERIAL_ATTRIB_COUNT];

   Evas_Real         shininess;

   Eina_Hash        *meshes;
};

struct _Evas_3D_Scene_Data
{
   Evas_Color        bg_color;
   Evas_3D_Node     *camera_node;
   Eina_List        *light_nodes;
   Eina_List        *mesh_nodes;
};

struct _Evas_3D_Pick_Data
{
   /* Input */
   Evas_Real         x, y;
   Evas_Mat4         matrix_vp;
   Evas_Ray3         ray_world;

   /* Output */
   Eina_Bool         picked;
   Evas_Real         z;
   Evas_3D_Node     *node;
   Evas_3D_Mesh     *mesh;
   Evas_Real         u, v;
   Evas_Real         s, t;
};

/* Object generic functions. */
void                 evas_3d_object_init(Evas_3D_Object *obj, Evas *e, Evas_3D_Object_Type type, const Evas_3D_Object_Func *func);
Evas                *evas_3d_object_evas_get(const Evas_3D_Object *obj);
Evas_3D_Object_Type  evas_3d_object_type_get(const Evas_3D_Object *obj);

void                 evas_3d_object_reference(Evas_3D_Object *obj);
void                 evas_3d_object_unreference(Evas_3D_Object *obj);
int                  evas_3d_object_reference_count_get(const Evas_3D_Object *obj);

void                 evas_3d_object_change(Evas_3D_Object *obj, Evas_3D_State state, Evas_3D_Object *ref);
Eina_Bool            evas_3d_object_dirty_get(const Evas_3D_Object *obj, Evas_3D_State state);
void                 evas_3d_object_update(Evas_3D_Object *obj);
void                 evas_3d_object_update_done(Evas_3D_Object *obj);

/* Node functions. */
void                 evas_3d_node_traverse(Evas_3D_Node *from, Evas_3D_Node *to, Evas_3D_Node_Traverse_Type type, Eina_Bool skip, Evas_3D_Node_Func func, void *data);
void                 evas_3d_node_tree_traverse(Evas_3D_Node *root, Evas_3D_Tree_Traverse_Type type, Eina_Bool skip, Evas_3D_Node_Func func, void *data);
Eina_Bool            evas_3d_node_mesh_collect(Evas_3D_Node *node, void *data);
Eina_Bool            evas_3d_node_light_collect(Evas_3D_Node *node, void *data);

void                 evas_3d_node_scene_root_add(Evas_3D_Node *node, Evas_3D_Scene *scene);
void                 evas_3d_node_scene_root_del(Evas_3D_Node *node, Evas_3D_Scene *scene);
void                 evas_3d_node_scene_camera_add(Evas_3D_Node *node, Evas_3D_Scene *scene);
void                 evas_3d_node_scene_camera_del(Evas_3D_Node *node, Evas_3D_Scene *scene);

/* Camera functions. */
void                 evas_3d_camera_node_add(Evas_3D_Camera *camera, Evas_3D_Node *node);
void                 evas_3d_camera_node_del(Evas_3D_Camera *camera, Evas_3D_Node *node);

/* Light functions. */
void                 evas_3d_light_node_add(Evas_3D_Light *light, Evas_3D_Node *node);
void                 evas_3d_light_node_del(Evas_3D_Light *light, Evas_3D_Node *node);

/* Mesh functions. */
void                 evas_3d_mesh_node_add(Evas_3D_Mesh *mesh, Evas_3D_Node *node);
void                 evas_3d_mesh_node_del(Evas_3D_Mesh *mesh, Evas_3D_Node *node);

void                 evas_3d_mesh_interpolate_vertex_buffer_get(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib, Evas_3D_Vertex_Buffer *buffer0, Evas_3D_Vertex_Buffer *buffer1, Evas_Real *weight);

void                 evas_3d_mesh_file_md2_set(Evas_3D_Mesh *mesh, const char *file);

/* Texture functions. */
void                 evas_3d_texture_material_add(Evas_3D_Texture *texture, Evas_3D_Material *material);
void                 evas_3d_texture_material_del(Evas_3D_Texture *texture, Evas_3D_Material *material);

/* Material functions. */
void                 evas_3d_material_mesh_add(Evas_3D_Material *material, Evas_3D_Mesh *mesh);
void                 evas_3d_material_mesh_del(Evas_3D_Material *material, Evas_3D_Mesh *mesh);

/* Scene functions. */
void                 evas_3d_scene_data_init(Evas_3D_Scene_Data *data);
void                 evas_3d_scene_data_fini(Evas_3D_Scene_Data *data);
