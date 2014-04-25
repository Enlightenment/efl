#ifndef _EVAS_3D_H
#define _EVAS_3D_H

typedef float   Evas_Real;

typedef struct _Evas_3D_Scene    Evas_3D_Scene;
typedef struct _Evas_3D_Node     Evas_3D_Node;
typedef struct _Evas_3D_Camera   Evas_3D_Camera;
typedef struct _Evas_3D_Light    Evas_3D_Light;
typedef struct _Evas_3D_Mesh     Evas_3D_Mesh;
typedef struct _Evas_3D_Texture  Evas_3D_Texture;
typedef struct _Evas_3D_Material Evas_3D_Material;

typedef enum _Evas_3D_Space
{
   EVAS_3D_SPACE_LOCAL,
   EVAS_3D_SPACE_PARENT,
   EVAS_3D_SPACE_WORLD,
} Evas_3D_Space;

typedef enum _Evas_3D_Node_Type
{
   EVAS_3D_NODE_TYPE_NODE,
   EVAS_3D_NODE_TYPE_CAMERA,
   EVAS_3D_NODE_TYPE_LIGHT,
   EVAS_3D_NODE_TYPE_MESH,
} Evas_3D_Node_Type;

typedef enum _Evas_3D_Vertex_Attrib
{
   EVAS_3D_VERTEX_POSITION,
   EVAS_3D_VERTEX_NORMAL,
   EVAS_3D_VERTEX_TANGENT,
   EVAS_3D_VERTEX_COLOR,
   EVAS_3D_VERTEX_TEXCOORD,
} Evas_3D_Vertex_Attrib;

typedef enum _Evas_3D_Index_Format
{
   EVAS_3D_INDEX_FORMAT_NONE,
   EVAS_3D_INDEX_FORMAT_UNSIGNED_BYTE,
   EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
} Evas_3D_Index_Format;

typedef enum _Evas_3D_Vertex_Assembly
{
   EVAS_3D_VERTEX_ASSEMBLY_POINTS,
   EVAS_3D_VERTEX_ASSEMBLY_LINES,
   EVAS_3D_VERTEX_ASSEMBLY_LINE_STRIP,
   EVAS_3D_VERTEX_ASSEMBLY_LINE_LOOP,
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES,
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP,
   EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_FAN,
} Evas_3D_Vertex_Assembly;

typedef enum _Evas_3D_Color_Format
{
   EVAS_3D_COLOR_FORMAT_RGBA,
   EVAS_3D_COLOR_FORMAT_RGB,
   EVAS_3D_COLOR_FORMAT_ALPHA,
} Evas_3D_Color_Format;

typedef enum _Evas_3D_Pixel_Format
{
   EVAS_3D_PIXEL_FORMAT_8,
   EVAS_3D_PIXEL_FORMAT_565,
   EVAS_3D_PIXEL_FORMAT_888,
   EVAS_3D_PIXEL_FORMAT_8888,
   EVAS_3D_PIXEL_FORMAT_4444,
   EVAS_3D_PIXEL_FORMAT_5551,
} Evas_3D_Pixel_Format;

typedef enum _Evas_3D_Wrap_Mode
{
   EVAS_3D_WRAP_MODE_CLAMP,
   EVAS_3D_WRAP_MODE_REPEAT,
   EVAS_3D_WRAP_MODE_REFLECT,
} Evas_3D_Wrap_Mode;

typedef enum _Evas_3D_Texture_Filter
{
   EVAS_3D_TEXTURE_FILTER_NEAREST,
   EVAS_3D_TEXTURE_FILTER_LINEAR,
   EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
   EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
   EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
   EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR,
} Evas_3D_Texture_Filter;

typedef enum _Evas_3D_Shade_Mode
{
   EVAS_3D_SHADE_MODE_VERTEX_COLOR,
   EVAS_3D_SHADE_MODE_DIFFUSE,
   EVAS_3D_SHADE_MODE_FLAT,
   EVAS_3D_SHADE_MODE_PHONG,
   EVAS_3D_SHADE_MODE_NORMAL_MAP,
} Evas_3D_Shade_Mode;

typedef enum _Evas_3D_Material_Attrib
{
   EVAS_3D_MATERIAL_AMBIENT,
   EVAS_3D_MATERIAL_DIFFUSE,
   EVAS_3D_MATERIAL_SPECULAR,
   EVAS_3D_MATERIAL_EMISSION,
   EVAS_3D_MATERIAL_NORMAL,
} Evas_3D_Material_Attrib;

typedef enum _Evas_3D_Mesh_File_Type
{
   EVAS_3D_MESH_FILE_TYPE_MD2,
} Evas_3D_Mesh_File_Type;

typedef enum _Evas_3D_Pick_Type
{
   EVAS_3D_PICK_NODE,
   EVAS_3D_PICK_MESH,
} Evas_3D_Pick_Type;

/* Image object render target */
EAPI void               evas_object_image_3d_scene_set(Evas_Object *obj, Evas_3D_Scene *scene) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Scene     *evas_object_image_3d_scene_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/* Scene */
EAPI Evas_3D_Scene     *evas_3d_scene_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_scene_del(Evas_3D_Scene *scene) EINA_ARG_NONNULL(1);
EAPI Evas              *evas_3d_scene_evas_get(const Evas_3D_Scene *scene) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_scene_root_node_set(Evas_3D_Scene *scene, Evas_3D_Node *node) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Node      *evas_3d_scene_root_node_get(const Evas_3D_Scene *scene) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_scene_camera_node_set(Evas_3D_Scene *scene, Evas_3D_Node *node) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Node      *evas_3d_scene_camera_node_get(const Evas_3D_Scene *scene) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_scene_size_set(Evas_3D_Scene *scene, int w, int h) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_scene_size_get(const Evas_3D_Scene *scene, int *w, int *h) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_scene_background_color_set(Evas_3D_Scene *scene, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_scene_background_color_get(const Evas_3D_Scene *scene, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);

EAPI Eina_Bool          evas_3d_scene_pick(const Evas_3D_Scene *scene, Evas_Real x, Evas_Real y, Evas_3D_Node **node, Evas_3D_Mesh **mesh, Evas_Real *s, Evas_Real *t) EINA_ARG_NONNULL(1);

/* Node */
EAPI Evas_3D_Node      *evas_3d_node_add(Evas *e, Evas_3D_Node_Type type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_del(Evas_3D_Node *node) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Node_Type  evas_3d_node_type_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Evas              *evas_3d_node_evas_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_node_member_add(Evas_3D_Node *node, Evas_3D_Node *member) EINA_ARG_NONNULL(1, 2);
EAPI void               evas_3d_node_member_del(Evas_3D_Node *node, Evas_3D_Node *member) EINA_ARG_NONNULL(1, 2);
EAPI Evas_3D_Node      *evas_3d_node_parent_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI const Eina_List   *evas_3d_node_member_list_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_node_position_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_orientation_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z, Evas_Real w) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_orientation_euler_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_orientation_angle_axis_set(Evas_3D_Node *node, Evas_Real angle, Evas_Real x, Evas_Real y, Evas_Real z) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_scale_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_node_position_get(const Evas_3D_Node *node, Evas_3D_Space space, Evas_Real *x, Evas_Real *y, Evas_Real *z) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_orientation_get(const Evas_3D_Node *node, Evas_3D_Space space, Evas_Real *x, Evas_Real *y, Evas_Real *z, Evas_Real *w) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_scale_get(const Evas_3D_Node *node, Evas_3D_Space space, Evas_Real *x, Evas_Real *y, Evas_Real *z) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_node_position_inherit_set(Evas_3D_Node *node, Eina_Bool inherit) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_orientation_inherit_set(Evas_3D_Node *node, Eina_Bool inherit) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_scale_inherit_set(Evas_3D_Node *node, Eina_Bool inherit) EINA_ARG_NONNULL(1);

EAPI Eina_Bool          evas_3d_node_position_inherit_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Eina_Bool          evas_3d_node_orientation_inherit_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Eina_Bool          evas_3d_node_scale_inherit_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_node_look_at_set(Evas_3D_Node *node, Evas_3D_Space target_space, Evas_Real x, Evas_Real y, Evas_Real z, Evas_3D_Space up_space, Evas_Real ux, Evas_Real uy, Evas_Real uz) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_node_camera_set(Evas_3D_Node *node, Evas_3D_Camera *camera) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Camera    *evas_3d_node_camera_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_node_light_set(Evas_3D_Node *node, Evas_3D_Light *light) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Light     *evas_3d_node_light_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_node_mesh_add(Evas_3D_Node *node, Evas_3D_Mesh *mesh) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_mesh_del(Evas_3D_Node *node, Evas_3D_Mesh *mesh) EINA_ARG_NONNULL(1);
EAPI const Eina_List   *evas_3d_node_mesh_list_get(const Evas_3D_Node *node) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_node_mesh_frame_set(Evas_3D_Node *node, Evas_3D_Mesh *mesh, int frame) EINA_ARG_NONNULL(1);
EAPI int                evas_3d_node_mesh_frame_get(const Evas_3D_Node *node, Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/* Camera */
EAPI Evas_3D_Camera    *evas_3d_camera_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_camera_del(Evas_3D_Camera *camera) EINA_ARG_NONNULL(1);
EAPI Evas              *evas_3d_camera_evas_get(const Evas_3D_Camera *camera) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_camera_projection_matrix_set(Evas_3D_Camera *camera, const Evas_Real *matrix) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_camera_projection_matrix_get(const Evas_3D_Camera *camera, Evas_Real *matrix) EINA_ARG_NONNULL(1, 2);
EAPI void               evas_3d_camera_projection_perspective_set(Evas_3D_Camera *camera, Evas_Real fovy, Evas_Real aspect, Evas_Real near, Evas_Real far) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_camera_projection_frustum_set(Evas_3D_Camera *camera, Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top, Evas_Real near, Evas_Real far) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_camera_projection_ortho_set(Evas_3D_Camera *camera, Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top, Evas_Real near, Evas_Real far) EINA_ARG_NONNULL(1);

/* Light */
EAPI Evas_3D_Light     *evas_3d_light_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_del(Evas_3D_Light *light) EINA_ARG_NONNULL(1);
EAPI Evas              *evas_3d_light_evas_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_light_directional_set(Evas_3D_Light *light, Eina_Bool directional) EINA_ARG_NONNULL(1);
EAPI Eina_Bool          evas_3d_light_directional_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_light_ambient_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_ambient_get(const Evas_3D_Light *light, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_diffuse_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_diffuse_get(const Evas_3D_Light *light, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_specular_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_specular_get(const Evas_3D_Light *light, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_spot_exponent_set(Evas_3D_Light *light, Evas_Real exponent) EINA_ARG_NONNULL(1);
EAPI Evas_Real          evas_3d_light_spot_exponent_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_spot_cutoff_set(Evas_3D_Light *light, Evas_Real cutoff) EINA_ARG_NONNULL(1);
EAPI Evas_Real          evas_3d_light_spot_cutoff_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_attenuation_set(Evas_3D_Light *light, Evas_Real constant, Evas_Real linear, Evas_Real quadratic) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_light_attenuation_get(const Evas_3D_Light *light, Evas_Real *constant, Evas_Real *linear, Evas_Real *quadratic) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_light_attenuation_enable_set(Evas_3D_Light *light, Eina_Bool enable) EINA_ARG_NONNULL(1);
EAPI Eina_Bool          evas_3d_light_attenuation_enable_get(const Evas_3D_Light *light) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/* Mesh */
EAPI Evas_3D_Mesh      *evas_3d_mesh_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_mesh_del(Evas_3D_Mesh *mesh) EINA_ARG_NONNULL(1);
EAPI Evas              *evas_3d_mesh_evas_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_mesh_shade_mode_set(Evas_3D_Mesh *mesh, Evas_3D_Shade_Mode mode) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Shade_Mode evas_3d_mesh_shade_mode_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_mesh_file_set(Evas_3D_Mesh *mesh, Evas_3D_Mesh_File_Type type, const char *file, const char *key) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_mesh_vertex_count_set(Evas_3D_Mesh *mesh, unsigned int count) EINA_ARG_NONNULL(1);
EAPI int                evas_3d_mesh_vertex_count_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_mesh_frame_add(Evas_3D_Mesh *mesh, int frame) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_mesh_frame_del(Evas_3D_Mesh *mesh, int frame) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_mesh_frame_material_set(Evas_3D_Mesh *mesh, int frame, Evas_3D_Material *material) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Material  *evas_3d_mesh_frame_material_get(const Evas_3D_Mesh *mesh, int frame) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_mesh_frame_vertex_data_set(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib, int stride, const void *data) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_mesh_frame_vertex_data_copy_set(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib, int stride, const void *data) EINA_ARG_NONNULL(1);
EAPI void              *evas_3d_mesh_frame_vertex_data_map(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_mesh_frame_vertex_data_unmap(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib) EINA_ARG_NONNULL(1);
EAPI int                evas_3d_mesh_frame_vertex_stride_get(const Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_mesh_index_data_set(Evas_3D_Mesh *mesh, Evas_3D_Index_Format format, int count, const void *indices) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_mesh_index_data_copy_set(Evas_3D_Mesh *mesh, Evas_3D_Index_Format format, int count, const void *indices) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Index_Format evas_3d_mesh_index_format_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI int                evas_3d_mesh_index_count_get(const Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void              *evas_3d_mesh_index_data_map(Evas_3D_Mesh *mesh) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_mesh_index_data_unmap(Evas_3D_Mesh *mesh) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_mesh_vertex_assembly_set(Evas_3D_Mesh *mesh, Evas_3D_Vertex_Assembly assembly);
EAPI Evas_3D_Vertex_Assembly evas_3d_mesh_vertex_assembly_get(const Evas_3D_Mesh *mesh);

/* Texture */
EAPI Evas_3D_Texture   *evas_3d_texture_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_texture_del(Evas_3D_Texture *texture) EINA_ARG_NONNULL(1);
EAPI Evas              *evas_3d_texture_evas_get(const Evas_3D_Texture *texture) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_texture_data_set(Evas_3D_Texture *texture, Evas_3D_Color_Format color_format, Evas_3D_Pixel_Format pixel_format, int w, int h, const void *data);
EAPI void               evas_3d_texture_file_set(Evas_3D_Texture *texture, const char *file, const char *key) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_texture_source_set(Evas_3D_Texture *texture, Evas_Object *source) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Color_Format evas_3d_texture_color_format_get(const Evas_3D_Texture *texture) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_texture_size_get(const Evas_3D_Texture *texture, int *w, int *h) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_texture_wrap_set(Evas_3D_Texture *texture, Evas_3D_Wrap_Mode s, Evas_3D_Wrap_Mode t) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_texture_wrap_get(const Evas_3D_Texture *texture, Evas_3D_Wrap_Mode *s, Evas_3D_Wrap_Mode *t) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_texture_filter_set(Evas_3D_Texture *texture, Evas_3D_Texture_Filter min, Evas_3D_Texture_Filter mag) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_texture_filter_get(const Evas_3D_Texture *texture, Evas_3D_Texture_Filter *min, Evas_3D_Texture_Filter *mag) EINA_ARG_NONNULL(1);

/* Material */
EAPI Evas_3D_Material  *evas_3d_material_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI void               evas_3d_material_del(Evas_3D_Material *material) EINA_ARG_NONNULL(1);
EAPI Evas              *evas_3d_material_evas_get(const Evas_3D_Material *material) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_material_enable_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib, Eina_Bool enable) EINA_ARG_NONNULL(1);
EAPI Eina_Bool          evas_3d_material_enable_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_material_color_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a) EINA_ARG_NONNULL(1);
EAPI void               evas_3d_material_color_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a) EINA_ARG_NONNULL(1);

EAPI void               evas_3d_material_shininess_set(Evas_3D_Material *material, Evas_Real shininess) EINA_ARG_NONNULL(1);
EAPI Evas_Real          evas_3d_material_shininess_get(const Evas_3D_Material *material) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void               evas_3d_material_texture_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib, Evas_3D_Texture *texture) EINA_ARG_NONNULL(1);
EAPI Evas_3D_Texture   *evas_3d_material_texture_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

#endif /* _EVAS_3D_H */
