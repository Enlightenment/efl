#ifndef EVAS_PRIVATE_H
#define EVAS_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Evas.h"
#include "Eet.h"

#include "../file/evas_module.h"
#include "../file/evas_path.h"
#include "../common/evas_text_utils.h"
#include "../common/language/evas_bidi_utils.h"
#include "../common/language/evas_language_utils.h"

#include "evas_3d_utils.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
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

#include "canvas/evas_text.eo.h"
#include "canvas/evas_textgrid.eo.h"
#include "canvas/evas_line.eo.h"
#include "canvas/evas_box.eo.h"
#include "canvas/evas_table.eo.h"
#include "canvas/evas_grid.eo.h"
#include "canvas/evas_out.eo.h"

#define RENDER_METHOD_INVALID            0x00000000

typedef struct _Evas_Layer                  Evas_Layer;
typedef struct _Evas_Size                   Evas_Size;
typedef struct _Evas_Aspect                 Evas_Aspect;
typedef struct _Evas_Border                 Evas_Border;
typedef struct _Evas_Double_Pair            Evas_Double_Pair;
typedef struct _Evas_Size_Hints             Evas_Size_Hints;
typedef struct _Evas_Font_Dir               Evas_Font_Dir;
typedef struct _Evas_Font                   Evas_Font;
typedef struct _Evas_Font_Alias             Evas_Font_Alias;
typedef struct _Evas_Font_Description       Evas_Font_Description;
typedef struct _Evas_Data_Node              Evas_Data_Node;
typedef struct _Evas_Func                   Evas_Func;
typedef struct _Evas_Image_Save_Func        Evas_Image_Save_Func;
typedef struct _Evas_Object_Func            Evas_Object_Func;
typedef struct _Evas_Intercept_Func         Evas_Intercept_Func;
typedef struct _Evas_Key_Grab               Evas_Key_Grab;
typedef struct _Evas_Format                 Evas_Format;
typedef struct _Evas_Map_Point              Evas_Map_Point;
typedef struct _Evas_Smart_Cb_Description_Array Evas_Smart_Cb_Description_Array;
typedef struct _Evas_Smart_Interfaces_Array Evas_Smart_Interfaces_Array;
typedef struct _Evas_Post_Callback          Evas_Post_Callback;
typedef struct _Evas_Coord_Touch_Point      Evas_Coord_Touch_Point;
typedef struct _Evas_Object_Proxy_Data      Evas_Object_Proxy_Data;
typedef struct _Evas_Object_Map_Data        Evas_Object_Map_Data;
typedef struct _Evas_Proxy_Render_Data      Evas_Proxy_Render_Data;
typedef struct _Evas_Object_3D_Data         Evas_Object_3D_Data;
typedef struct _Evas_Object_Mask_Data       Evas_Object_Mask_Data;

typedef struct _Evas_Smart_Data             Evas_Smart_Data;

typedef struct _Evas_Object_Protected_State Evas_Object_Protected_State;
typedef struct _Evas_Object_Protected_Data  Evas_Object_Protected_Data;

typedef struct _Evas_Filter_Program         Evas_Filter_Program;
typedef struct _Evas_Object_Filter_Data     Evas_Object_Filter_Data;
typedef struct _Evas_Filter_Data_Binding    Evas_Filter_Data_Binding;

// 3D stuff

#define EVAS_CANVAS3D_VERTEX_ATTRIB_COUNT    5
#define EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT  5

typedef struct _Evas_Canvas3D_Object                Evas_Canvas3D_Object_Data;
typedef struct _Evas_Canvas3D_Scene_Public_Data     Evas_Canvas3D_Scene_Public_Data;
typedef struct _Evas_Canvas3D_Vertex_Buffer         Evas_Canvas3D_Vertex_Buffer;
typedef struct _Evas_Canvas3D_Mesh_Frame            Evas_Canvas3D_Mesh_Frame;
typedef struct _Evas_Canvas3D_Node_Mesh             Evas_Canvas3D_Node_Mesh;
typedef struct _Evas_Canvas3D_Object_Func           Evas_Canvas3D_Object_Func;
typedef struct _Evas_Canvas3D_Pick_Data             Evas_Canvas3D_Pick_Data;
typedef struct _Evas_Canvas3D_Interpolate_Vertex_Buffer Evas_Canvas3D_Interpolate_Vertex_Buffer;

typedef struct _Evas_Canvas3D_Scene            Evas_Canvas3D_Scene_Data;
typedef struct _Evas_Canvas3D_Node             Evas_Canvas3D_Node_Data;
typedef struct _Evas_Canvas3D_Mesh             Evas_Canvas3D_Mesh_Data;
typedef struct _Evas_Canvas3D_Camera           Evas_Canvas3D_Camera_Data;
typedef struct _Evas_Canvas3D_Light            Evas_Canvas3D_Light_Data;
typedef struct _Evas_Canvas3D_Material         Evas_Canvas3D_Material_Data;
typedef struct _Evas_Canvas3D_Texture          Evas_Canvas3D_Texture_Data;
typedef struct _Evas_Canvas3D_Primitive        Evas_Canvas3D_Primitive_Data;

/* Structs for mesh eet saver/loader */
typedef struct _Evas_Canvas3D_Vec2_Eet         Evas_Canvas3D_Vec2_Eet;
typedef struct _Evas_Canvas3D_Vec3_Eet         Evas_Canvas3D_Vec3_Eet;
typedef struct _Evas_Canvas3D_Vertex_Eet       Evas_Canvas3D_Vertex_Eet;
typedef struct _Evas_Canvas3D_Geometry_Eet     Evas_Canvas3D_Geometry_Eet;
typedef struct _Evas_Canvas3D_Color_Eet        Evas_Canvas3D_Color_Eet;
typedef struct _Evas_Canvas3D_Material_Eet     Evas_Canvas3D_Material_Eet;
typedef struct _Evas_Canvas3D_Frame_Eet        Evas_Canvas3D_Frame_Eet;
typedef struct _Evas_Canvas3D_Mesh_Eet         Evas_Canvas3D_Mesh_Eet;
typedef struct _Evas_Canvas3D_Header_Eet       Evas_Canvas3D_Header_Eet;
typedef struct _Evas_Canvas3D_File_Eet         Evas_Canvas3D_File_Eet;

struct _Evas_Canvas3D_Vec2_Eet
{
   float x;
   float y;
};

struct _Evas_Canvas3D_Vec3_Eet
{
   float x;
   float y;
   float z;
};

struct _Evas_Canvas3D_Vertex_Eet
{
   Evas_Canvas3D_Vec3_Eet position;
   Evas_Canvas3D_Vec3_Eet normal;
   Evas_Canvas3D_Vec2_Eet texcoord;
};//one point of mesh

struct _Evas_Canvas3D_Geometry_Eet
{
   unsigned int id;
   int vertices_count;
   Evas_Canvas3D_Vertex_Eet *vertices;
};//contain array of vertices and id for using in Evas_Canvas3D_Frame_Eet in future

struct _Evas_Canvas3D_Color_Eet
{
   float r;
   float g;
   float b;
   float a;
};

struct _Evas_Canvas3D_Material_Eet
{
   int id;
   float shininess;
   int colors_count;
   Evas_Canvas3D_Color_Eet *colors;//Color per attribute (ambient, diffuse, specular etc.)
};

struct _Evas_Canvas3D_Frame_Eet
{
   int id;
   int geometry_id;
   int material_id;
};//only ids to prevent of spending of memory when animation will change only geometry or only material

struct _Evas_Canvas3D_Mesh_Eet
{
   int materials_count;
   int frames_count;
   int geometries_count;
   Evas_Canvas3D_Material_Eet *materials;
   Evas_Canvas3D_Frame_Eet *frames;
   Evas_Canvas3D_Geometry_Eet *geometries;
};//contain materials, geometries and bounding between it (frames)

struct _Evas_Canvas3D_Header_Eet
{
   int version;
   int *materials;//colors_count
   int materials_count;
   int *geometries;//vertices_count
   int geometries_count;
   int frames;
};//can be use for fast allocating of memory

struct _Evas_Canvas3D_File_Eet
{
   Evas_Canvas3D_Mesh_Eet *mesh;
   Evas_Canvas3D_Header_Eet *header;
};//contain mesh data and information about mesh size

typedef Eina_Bool (*Evas_Canvas3D_Node_Func)(Evas_Canvas3D_Node *, void *data);


typedef enum _Evas_Canvas3D_Node_Traverse_Type
{
   EVAS_CANVAS3D_NODE_TRAVERSE_DOWNWARD,
   EVAS_CANVAS3D_NODE_TRAVERSE_UPWARD,
} Evas_Canvas3D_Node_Traverse_Type;

typedef enum _Evas_Canvas3D_Tree_Traverse_Type
{
   EVAS_CANVAS3D_TREE_TRAVERSE_PRE_ORDER,
   EVAS_CANVAS3D_TREE_TRAVERSE_ANY_ORDER = EVAS_CANVAS3D_TREE_TRAVERSE_PRE_ORDER,
   EVAS_CANVAS3D_TREE_TRAVERSE_POST_ORDER,
   EVAS_CANVAS3D_TREE_TRAVERSE_LEVEL_ORDER,
} Evas_Canvas3D_Tree_Traverse_Type;


struct _Evas_Canvas3D_Object
{
   Evas                *evas;
   Evas_Canvas3D_Object_Type  type;

   Eina_Bool            dirty[EVAS_CANVAS3D_STATE_MAX];
};

struct _Evas_Canvas3D_Scene
{
   Evas_Canvas3D_Node     *root_node;
   Evas_Canvas3D_Node     *camera_node;
   Evas_Color       bg_color;
   Eina_Bool        shadows_enabled :1;
   Eina_Bool        color_pick_enabled :1;

   void             *surface;
   int               w, h;
   Eina_List        *images;

   Eina_Hash        *node_mesh_colors;
   Eina_Hash        *colors_node_mesh;
   /*sets constant for shadow rendering*/
   Evas_Real depth_offset;
   Evas_Real depth_constant;
};

struct _Evas_Canvas3D_Node_Mesh
{
   Evas_Canvas3D_Node           *node;
   Evas_Canvas3D_Mesh           *mesh;
   int                     frame;
};

struct _Evas_Canvas3D_Node
{
   Eina_List        *members;
   Evas_Canvas3D_Node     *parent;
   Evas_Canvas3D_Node     *billboard_target;

   Eina_Vector3         position;
   Eina_Quaternion         orientation;
   Eina_Vector3         scale;

   Eina_Vector3         position_world;
   Eina_Quaternion   orientation_world;
   Eina_Vector3         scale_world;

   Evas_Box3         aabb;
   Evas_Box3         obb;
   Evas_Sphere       bsphere;

   Evas_Canvas3D_Node_Type type;

   /* Camera node. */
   union {
        struct {
             Evas_Canvas3D_Camera  *camera;
             Eina_Matrix4        matrix_world_to_eye;
        } camera;

        struct {
             Evas_Canvas3D_Light   *light;
             Eina_Matrix4        matrix_local_to_world;
        } light;

        struct {
             Eina_List       *meshes;
             Eina_Hash       *node_meshes;
             Eina_Matrix4        matrix_local_to_world;
        } mesh;
   } data;

   /* Scene using this node as root. */
   Eina_Hash        *scenes_root;

   /* Scene using this node as camera. */
   Eina_Hash        *scenes_camera;

   Eina_Bool         position_inherit : 1;
   Eina_Bool         orientation_inherit : 1;
   Eina_Bool         scale_inherit : 1;
   Eina_Bool         lod : 1;
};

struct _Evas_Canvas3D_Camera
{
   Eina_Matrix4      projection;
   Eina_Hash     *nodes;
};

struct _Evas_Canvas3D_Light
{
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
   Eina_Matrix4      projection;

   Eina_Hash     *nodes;
};

struct _Evas_Canvas3D_Vertex_Buffer
{
   int         element_count;
   int         stride;
   void       *data;
   int         size;
   Eina_Bool   owns_data;
   Eina_Bool   mapped;
};

struct _Evas_Canvas3D_Interpolate_Vertex_Buffer
{
   void       *data0;
   int         stride0;
   int         size0;

   void       *data1;
   int         stride1;
   int         size1;

   Evas_Real   weight;
};

struct _Evas_Canvas3D_Mesh_Frame
{
   Evas_Canvas3D_Mesh           *mesh;

   int                     frame;
   Evas_Canvas3D_Material       *material;
   Evas_Box3               aabb;

   Evas_Canvas3D_Vertex_Buffer   vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_COUNT];
};

struct _Evas_Canvas3D_Mesh
{
   Evas_Canvas3D_Shader_Mode      shader_mode;

   int                     vertex_count;
   int                     frame_count;
   Eina_List              *frames;

   Evas_Canvas3D_Index_Format    index_format;
   int                     index_count;
   void                   *indices;
   int                     index_size;
   Eina_Bool               owns_indices;
   Eina_Bool               index_mapped;

   Evas_Canvas3D_Vertex_Assembly assembly;

   Eina_Hash               *nodes;
   Eina_Bool               shadowed;

   Eina_Bool               blending :1;
   Evas_Canvas3D_Blend_Func      blend_sfactor;
   Evas_Canvas3D_Blend_Func      blend_dfactor;

   Evas_Canvas3D_Comparison      alpha_comparison;
   Evas_Real               alpha_ref_value;
   Eina_Bool               alpha_test_enabled :1;

   Evas_Color              fog_color;
   Eina_Bool               fog_enabled :1;
   Evas_Color              color_pick_key;
   Eina_Bool               color_pick_enabled :1;
   /*sets of the quality and offsets for shadow rendering*/
   int                     shadows_edges_filtering_level;
   Evas_Real               shadows_edges_size;
   Evas_Real               shadows_constant_bias;
   Evas_Real               near_lod_boundary;
   Evas_Real               far_lod_boundary;
};

struct _Evas_Canvas3D_Texture
{
   /* List of materials using this texture. */
   Eina_Hash        *materials;

   /* Proxy data. */
   Evas_Object      *source;
   Eina_Bool         proxy_rendering;
   void             *proxy_surface;
   /* Engine-side object. */
   void             *engine_data;

   /*Use atlases, @EINA_TRUE by default*/
   Eina_Bool        atlas_enable :1;
};

struct _Evas_Canvas3D_Material
{
   struct {
        Eina_Bool         enable;
        Evas_Color        color;
        Evas_Canvas3D_Texture  *texture;
   } attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT];

   Evas_Real         shininess;

   Eina_Hash        *meshes;
};

struct _Evas_Canvas3D_Primitive
{
   Evas_Canvas3D_Mesh_Primitive  form;
   Evas_Canvas3D_Primitive_Mode  mode;
   Evas_Real               ratio;
   int                     precision;
   Evas_Canvas3D_Surface_Func   *surface;

   Eina_Vector2 tex_scale;
};

struct _Evas_Canvas3D_Scene_Public_Data
{
   Evas_Color       bg_color;
   Evas_Canvas3D_Node     *camera_node;
   Eina_List        *light_nodes;
   Eina_List        *mesh_nodes;
   Eina_Bool        shadows_enabled :1;
   Eina_Bool        color_pick_enabled :1;
   Eina_Hash        *node_mesh_colors;
   Eina_Hash        *colors_node_mesh;

   /*sets constant for shadow rendering*/
   Evas_Real depth_offset;
   Evas_Real depth_constant;
   Eina_Bool render_to_texture;

   unsigned int     lod_distance;

   Eina_Bool post_processing :1;
   Evas_Canvas3D_Shader_Mode post_processing_type;
};

struct _Evas_Canvas3D_Pick_Data
{
   /* Input */
   Evas_Real         x, y;
   Eina_Matrix4         matrix_vp;
   Evas_Ray3         ray_world;

   /* Output */
   Eina_Bool         picked;
   Evas_Real         z;
   Evas_Canvas3D_Node     *node;
   Evas_Canvas3D_Mesh     *mesh;
   Evas_Real         u, v;
   Evas_Real         s, t;
};

enum _Evas_Font_Style
{
   EVAS_FONT_STYLE_SLANT,
   EVAS_FONT_STYLE_WEIGHT,
   EVAS_FONT_STYLE_WIDTH
};

enum _Evas_Font_Slant
{
   EVAS_FONT_SLANT_NORMAL,
   EVAS_FONT_SLANT_OBLIQUE,
   EVAS_FONT_SLANT_ITALIC
};

enum _Evas_Font_Weight
{
   EVAS_FONT_WEIGHT_NORMAL,
   EVAS_FONT_WEIGHT_THIN,
   EVAS_FONT_WEIGHT_ULTRALIGHT,
   EVAS_FONT_WEIGHT_EXTRALIGHT,
   EVAS_FONT_WEIGHT_LIGHT,
   EVAS_FONT_WEIGHT_BOOK,
   EVAS_FONT_WEIGHT_MEDIUM,
   EVAS_FONT_WEIGHT_SEMIBOLD,
   EVAS_FONT_WEIGHT_BOLD,
   EVAS_FONT_WEIGHT_ULTRABOLD,
   EVAS_FONT_WEIGHT_EXTRABOLD,
   EVAS_FONT_WEIGHT_BLACK,
   EVAS_FONT_WEIGHT_EXTRABLACK
};

enum _Evas_Font_Width
{
   EVAS_FONT_WIDTH_NORMAL,
   EVAS_FONT_WIDTH_ULTRACONDENSED,
   EVAS_FONT_WIDTH_EXTRACONDENSED,
   EVAS_FONT_WIDTH_CONDENSED,
   EVAS_FONT_WIDTH_SEMICONDENSED,
   EVAS_FONT_WIDTH_SEMIEXPANDED,
   EVAS_FONT_WIDTH_EXPANDED,
   EVAS_FONT_WIDTH_EXTRAEXPANDED,
   EVAS_FONT_WIDTH_ULTRAEXPANDED
};

enum _Evas_Font_Spacing
{
   EVAS_FONT_SPACING_PROPORTIONAL,
   EVAS_FONT_SPACING_DUAL,
   EVAS_FONT_SPACING_MONO,
   EVAS_FONT_SPACING_CHARCELL
};

typedef enum _Evas_Font_Style               Evas_Font_Style;
typedef enum _Evas_Font_Slant               Evas_Font_Slant;
typedef enum _Evas_Font_Weight              Evas_Font_Weight;
typedef enum _Evas_Font_Width               Evas_Font_Width;
typedef enum _Evas_Font_Spacing             Evas_Font_Spacing;

/* General types - used for script type chceking */
#define OPAQUE_TYPE(type) struct __##type { int a; }; \
   typedef struct __##type type

OPAQUE_TYPE(Evas_Font_Set); /* General type for RGBA_Font */
OPAQUE_TYPE(Evas_Font_Instance); /* General type for RGBA_Font_Int */
/* End of general types */

#define MAGIC_EVAS                 0x70777770
#define MAGIC_OBJ                  0x71737723
#define MAGIC_OBJ_RECTANGLE        0x76748772
#define MAGIC_OBJ_LINE             0x7a27f839
#define MAGIC_OBJ_POLYGON          0x7bb7577e
#define MAGIC_OBJ_IMAGE            0x747ad76c
#define MAGIC_OBJ_TEXT             0x77757721
#define MAGIC_OBJ_SMART            0x78c7c73f
#define MAGIC_OBJ_TEXTBLOCK        0x71737744
#define MAGIC_OBJ_TEXTGRID         0x7377a7ca
#define MAGIC_SMART                0x7c6977c5
#define MAGIC_OBJ_SHAPE            0x747297f7
#define MAGIC_OBJ_CONTAINER        0x71877776
#define MAGIC_OBJ_VG               0x77817EE7
#define MAGIC_OBJ_CUSTOM           0x7b7857ab
#define MAGIC_EVAS_GL              0x77976718
#define MAGIC_MAP                  0x7575177d
#define MAGIC_DEV                  0x7d773738

#ifdef EINA_MAGIC_DEBUG
# define MAGIC_CHECK_FAILED(o, t, m) \
{evas_debug_error(); \
 if (!o) evas_debug_input_null(); \
}
# define MAGIC_CHECK(o, t, m) \
{if (EINA_UNLIKELY(!o)) { \
MAGIC_CHECK_FAILED(o, t, m)
# define MAGIC_CHECK_END() }}
#else
# define MAGIC_CHECK_FAILED(o, t, m)
# define MAGIC_CHECK(o, t, m)  { if (!o) {
# define MAGIC_CHECK_END() }}
#endif

#define NEW_RECT(_r, _x, _y, _w, _h) (_r) = eina_rectangle_new(_x, _y, _w, _h);

#define MERR_NONE() _evas_alloc_error = EVAS_ALLOC_ERROR_NONE
#define MERR_FATAL() _evas_alloc_error = EVAS_ALLOC_ERROR_FATAL
#define MERR_BAD() _evas_alloc_error = EVAS_ALLOC_ERROR_RECOVERED

/* DEBUG mode: fail, but normally just ERR(). This also returns if NULL. */
#ifdef DEBUG
#define EVAS_OBJECT_LEGACY_API(_eo, ...) \
   do { Evas_Object_Protected_Data *_o = eo_data_scope_get(_eo, EVAS_OBJECT_CLASS); \
      if (EINA_UNLIKELY(!_o)) return __VA_ARGS__; \
      if (EINA_UNLIKELY(!_o->legacy)) { \
         char buf[1024]; snprintf(buf, sizeof(buf), "Calling legacy API on EO object '%s' is not permitted!", eo_class_name_get(_o->object)); \
         EINA_SAFETY_ERROR(buf); \
         return __VA_ARGS__; \
   } } while (0)
#else
#define EVAS_OBJECT_LEGACY_API(_eo, ...) \
   do { Evas_Object_Protected_Data *_o = eo_data_scope_get(_eo, EVAS_OBJECT_CLASS); \
      if (EINA_UNLIKELY(!_o)) return __VA_ARGS__; \
      if (EINA_UNLIKELY(!_o->legacy)) { \
         char buf[1024]; snprintf(buf, sizeof(buf), "Calling legacy API on EO object '%s' is not permitted!", eo_class_name_get(_o->object)); \
         EINA_SAFETY_ERROR(buf); \
   } } while (0)
#endif

#define EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(cur, prev)                  \
  if (cur->u.file && !cur->mmaped_source)				\
    {                                                                   \
       eina_stringshare_del(cur->u.file);				\
       if (prev->u.file == cur->u.file)					\
         prev->u.file = NULL;						\
       cur->u.file = NULL;						\
    }                                                                   \
  if (cur->key)                                                         \
    {                                                                   \
       eina_stringshare_del(cur->key);                                  \
       if (prev->key == cur->key)                                       \
         prev->key = NULL;                                              \
       cur->key = NULL;                                                 \
    }                                                                   \
  if (prev->u.file && !prev->mmaped_source)				\
    {                                                                   \
       eina_stringshare_del(prev->u.file);				\
       prev->u.file = NULL;						\
    }                                                                   \
  if (prev->key)                                                        \
    {                                                                   \
       eina_stringshare_del(prev->key);                                 \
       prev->key = NULL;                                                \
    }

struct _Evas_Coord_Touch_Point
{
   Evas_Coord x, y; // point's x, y position
   int id; // id in order to distinguish each point
   Evas_Touch_Point_State state;
};

struct _Evas_Key_Grab
{
   char               *keyname;
   Evas_Modifier_Mask  modifiers;
   Evas_Modifier_Mask  not_modifiers;
   Evas_Object        *object;
   Eina_Bool           exclusive : 1;
   Eina_Bool           just_added : 1;
   Eina_Bool           delete_me : 1;
};

struct _Evas_Intercept_Func
{
   struct {
      Evas_Object_Intercept_Show_Cb func;
      void *data;
      Eina_Bool intercepted;
   } show;
   struct {
      Evas_Object_Intercept_Hide_Cb func;
      void *data;
      Eina_Bool intercepted;
   } hide;
   struct {
      Evas_Object_Intercept_Move_Cb func;
      void *data;
      Eina_Bool intercepted;
   } move;
   struct {
      Evas_Object_Intercept_Resize_Cb func;
      void *data;
      Eina_Bool intercepted;
   } resize;
   struct {
      Evas_Object_Intercept_Raise_Cb func;
      void *data;
      Eina_Bool intercepted;
   } raise;
   struct {
      Evas_Object_Intercept_Lower_Cb func;
      void *data;
      Eina_Bool intercepted;
   } lower;
   struct {
      Evas_Object_Intercept_Stack_Above_Cb func;
      void *data;
      Eina_Bool intercepted;
   } stack_above;
   struct {
      Evas_Object_Intercept_Stack_Below_Cb func;
      void *data;
      Eina_Bool intercepted;
   } stack_below;
   struct {
      Evas_Object_Intercept_Layer_Set_Cb func;
      void *data;
      Eina_Bool intercepted;
   } layer_set;
   struct {
      Evas_Object_Intercept_Color_Set_Cb func;
      void *data;
      Eina_Bool intercepted;
   } color_set;
   struct {
      Evas_Object_Intercept_Clip_Set_Cb func;
      void *data;
      Eina_Bool intercepted;
   } clip_set;
   struct {
      Evas_Object_Intercept_Clip_Unset_Cb func;
      void *data;
      Eina_Bool intercepted;
   } clip_unset;
   struct {
      Evas_Object_Intercept_Focus_Set_Cb func;
      void *data;
      Eina_Bool intercepted;
   } focus_set;
};

struct _Evas_Smart_Cb_Description_Array
{
   unsigned int                      size;
   const Evas_Smart_Cb_Description **array;
};

struct _Evas_Smart_Interfaces_Array
{
   unsigned int                 size;
   const Evas_Smart_Interface **array;
};

struct _Evas_Smart
{
   DATA32            magic;

   int               usage;

   const Evas_Smart_Class *smart_class;

   Evas_Smart_Cb_Description_Array callbacks;
   Evas_Smart_Interfaces_Array interfaces;

   unsigned char     delete_me : 1;
   unsigned char     class_allocated : 1;

};

struct _Evas_Modifier
{
   struct {
      int       count;
      char    **list;
   } mod;
   Evas_Modifier_Mask mask; /* ok we have a max of 64 modifiers */
};

struct _Evas_Lock
{
   struct {
      int       count;
      char    **list;
   } lock;
   Evas_Modifier_Mask mask; /* we have a max of 64 locks */
};

struct _Evas_Post_Callback
{
   Evas_Object               *obj;
   Evas_Object_Event_Post_Cb  func;
   const void                *data;
   unsigned char              delete_me : 1;
};

struct _Evas_Public_Data
{
   EINA_INLIST;

   DATA32            magic;
   Evas              *evas;

   struct {
      unsigned char  inside : 1;
      int            mouse_grabbed;
      int            downs;
      DATA32         button;
      Evas_Coord     x, y;
      int            nogrep;
      struct {
          Eina_List *in;
      } object;
   } pointer;

   struct  {
      Evas_Coord     x, y, w, h;
      unsigned char  changed : 1;
   } viewport;

   struct {
      int            w, h;
      DATA32         render_method;
      unsigned char  changed : 1;
   } output;

   struct
     {
        Evas_Coord x, y, w, h;
        Eina_Bool changed : 1;
        Evas_Object *clip;
     } framespace;

   Eina_List        *damages;
   Eina_List        *obscures;

   Evas_Layer       *layers;

   Eina_Hash        *name_hash;

   // locking so we can implement async rendering threads
   Eina_Lock         lock_objects;

   int               output_validity;

   int               walking_list;
   Evas_Event_Flags  default_event_flags;

   struct {
      Evas_Module *module;
      Evas_Func *func;
      Ector_Surface *ector;
      struct {
         void *output;

         void *context;
      } data;

      void *info;
      int   info_magic;
   } engine;

   struct {
      Eina_List *updates;
      Eina_Spinlock lock;
   } render;

   Eina_Array     delete_objects;
   Eina_Array     active_objects;
   Eina_Array     restack_objects;
   Eina_Array     render_objects;
   Eina_Array     pending_objects;
   Eina_Array     obscuring_objects;
   Eina_Array     temporary_objects;
   Eina_Array     snapshot_objects;
   Eina_Array     clip_changes;
   Eina_Array     scie_unref_queue;
   Eina_Array     image_unref_queue;
   Eina_Array     glyph_unref_queue;
   Eina_Array     texts_unref_queue;

   Eina_Clist     calc_list;
   Eina_Clist     calc_done;
   Eina_List     *video_objects;

   Eina_List     *post_events; // free me on evas_free

   Eina_Inlist    *callbacks;

   int            delete_grabs;
   int            walking_grabs;
   Eina_List     *grabs;

   Eina_List     *font_path;

   int            in_smart_calc;
   int            smart_calc_count;

   Evas_Object   *focused;
   void          *attach_data;
   Evas_Modifier  modifiers;
   Evas_Lock      locks;
   unsigned int   last_timestamp;
   int            last_mouse_down_counter;
   int            last_mouse_up_counter;
   int            nochange;
   Evas_Font_Hinting_Flags hinting;

   Eina_List     *touch_points;
   Eina_List     *devices;
   Eina_Array    *cur_device;

   Eina_List     *outputs;

   unsigned char  changed : 1;
   unsigned char  delete_me : 1;
   unsigned char  invalidate : 1;
   unsigned char  cleanup : 1;
   unsigned char  focus : 1;
   Eina_Bool      is_frozen : 1;
   Eina_Bool      rendering : 1;
   Eina_Bool      render2 : 1;
};

struct _Evas_Layer
{
   EINA_INLIST;

   short             layer;
   Evas_Object_Protected_Data      *objects;

   Evas_Public_Data *evas;

   void             *engine_data;
   int               usage;
   unsigned char     delete_me : 1;
};

struct _Evas_Size
{
   Evas_Coord w, h;
};

struct _Evas_Aspect
{
   Evas_Aspect_Control mode;
   Evas_Size size;
};

struct _Evas_Border
{
   Evas_Coord l, r, t, b;
};

struct _Evas_Double_Pair
{
   double x, y;
};

struct _Evas_Size_Hints
{
   Evas_Size min, max, request;
   Evas_Aspect aspect;
   Evas_Double_Pair align, weight;
   Evas_Border padding;
   Evas_Display_Mode dispmode;
};

struct _Evas_Map_Point
{
   double x, y, z, px, py;
   double u, v;
   unsigned char r, g, b, a;
};

struct _Evas_Map
{
   DATA32                magic;
   int                   count; // num of points
   Evas_Coord_Rectangle  normal_geometry; // bounding box of map geom actually
//   void                 *surface; // surface holding map if needed
//   int                   surface_w, surface_h; // current surface w & h alloc
   Evas_Coord            mx, my; // mouse x, y after conversion to map space
   struct {
      Evas_Coord         px, py, z0, foc;
   } persp;
   Eina_Bool             alpha : 1;
   Eina_Bool             smooth : 1;
   struct {
      Eina_Bool          enabled : 1;
      Evas_Coord         diff_x, diff_y;
   } move_sync;
   Evas_Map_Point        points[]; // actual points
};

struct _Evas_Object_Proxy_Data
{
   Eina_List               *proxies;
   Eina_List               *proxy_textures;
   void                    *surface;
   int                      w,h;
   Eina_List               *src_event_in;
   Eina_Bool                redraw : 1;
   Eina_Bool                is_proxy : 1;
   Eina_Bool                src_invisible : 1;
   Eina_Bool                src_events: 1;
};

struct _Evas_Object_Map_Data
{
   struct {
      Evas_Map             *map;
      Evas_Object          *map_parent;

      Eina_Bool             usemap : 1;
      Eina_Bool             valid_map : 1;
   } cur, prev;
   void                 *surface; // surface holding map if needed
   int                   surface_w, surface_h; // current surface w & h alloc

   Evas_Map             *cache_map;
   RGBA_Map             *spans;
};

struct _Evas_Object_3D_Data
{
   void          *surface;
   int            w, h;
};

// Mask clipper information
struct _Evas_Object_Mask_Data
{
   void          *surface;
   int            w, h;
   Eina_Bool      is_mask : 1;
   Eina_Bool      redraw : 1;
   Eina_Bool      is_alpha : 1;
   Eina_Bool      is_scaled : 1;
};

struct _Evas_Object_Protected_State
{
   Evas_Object_Protected_Data *clipper;

   Evas_Coord_Rectangle  geometry;
   struct {
      struct {
         Evas_Coord      x, y, w, h;
         unsigned char   r, g, b, a;
         Eina_Bool       visible : 1;
         Eina_Bool       dirty : 1;
      } clip;
   } cache;
   struct {
      unsigned char      r, g, b, a;
   } color;

   double                scale;

   short                 layer;

   Evas_Render_Op        render_op : 4;

   Eina_Bool             visible : 1;
   Eina_Bool             have_clipees : 1;
   Eina_Bool             anti_alias : 1;
   Eina_Bool             valid_bounding_box : 1;
   Eina_Bool             snapshot : 1;
};

struct _Evas_Object_Protected_Data
{
   EINA_INLIST;

   const char              *type;
   Evas_Layer              *layer;

   const Evas_Object_Protected_State *cur;
   const Evas_Object_Protected_State *prev;

   char                       *name;

   Evas_Intercept_Func        *interceptors;
   Eina_List                  *grabs;

   Eina_Inlist                *callbacks;

   struct {
      Eina_List               *clipees;
      Eina_List               *cache_clipees_answer;
      Eina_List               *changes;
      Evas_Object_Protected_Data *mask, *prev_mask;
   } clip;

   const Evas_Object_Func     *func;
   void                       *private_data;

   struct {
      Evas_Smart              *smart;
      Evas_Object             *parent;
      Evas_Smart_Data         *parent_data;
      Evas_Object_Protected_Data *parent_object_data;
   } smart;

   // Eina_Cow pointer be careful when writing to it
   const Evas_Object_Proxy_Data *proxy;
   const Evas_Object_Map_Data *map;
   const Evas_Object_3D_Data  *data_3d;
   const Evas_Object_Mask_Data *mask;

   // Pointer to the Evas_Object itself
   Evas_Object                *object;

   Evas_Size_Hints            *size_hints;

   int                         last_mouse_down_counter;
   int                         last_mouse_up_counter;
   int                         mouse_grabbed;

   // Daniel: Added because the destructor can't take parameters, at least for the moment
   int                         clean_layer;
   int                         last_event;
   Evas_Callback_Type          last_event_type;

   struct {
        int                      in_move, in_resize;
   } doing;

   unsigned int                ref;

   unsigned int                animator_ref;
   unsigned int                move_ref;

   unsigned char               delete_me;

   struct  {
      Eina_Bool                pass_events : 1;
      Eina_Bool                pass_events_valid : 1;
      Eina_Bool                freeze_events : 1;
      Eina_Bool                freeze_events_valid : 1;
      Eina_Bool                src_invisible : 1;
      Eina_Bool                src_invisible_valid : 1;
   } parent_cache;

   Evas_Object_Pointer_Mode    pointer_mode : 2;
   Eina_Bool                   store : 1;
   Eina_Bool                   pass_events : 1;
   Eina_Bool                   freeze_events : 1;
   Eina_Bool                   repeat_events : 1;
   Eina_Bool                   restack : 1;
   Eina_Bool                   is_active : 1;

   Eina_Bool                   precise_is_inside : 1;
   Eina_Bool                   is_static_clip : 1;
   Eina_Bool                   render_pre : 1;
   Eina_Bool                   rect_del : 1;
   Eina_Bool                   mouse_in : 1;
   Eina_Bool                   pre_render_done : 1;
   Eina_Bool                   focused : 1;
   Eina_Bool                   in_layer : 1;

   Eina_Bool                   no_propagate : 1;
   Eina_Bool                   changed : 1;
   Eina_Bool                   changed_move : 1;
   Eina_Bool                   changed_color : 1;
   Eina_Bool                   changed_map : 1;
   Eina_Bool                   changed_pchange : 1;
   Eina_Bool                   changed_src_visible : 1;
   Eina_Bool                   need_surface_clear : 1;
   Eina_Bool                   del_ref : 1;

   Eina_Bool                   is_frame : 1;
   Eina_Bool                   child_has_map : 1;
   Eina_Bool                   eo_del_called : 1;
   Eina_Bool                   is_smart : 1;
   Eina_Bool                   no_render : 1; // since 1.15
   Eina_Bool                   legacy : 1; // used legacy constructor
};

struct _Evas_Data_Node
{
   char *key;
   void *data;
};

struct _Evas_Font_Dir
{
   Eina_Hash *lookup;
   Eina_List *fonts;
   Eina_List *aliases;
   DATA64     dir_mod_time;
   DATA64     fonts_dir_mod_time;
   DATA64     fonts_alias_mod_time;
};

struct _Evas_Font
{
   struct {
      const char *prop[14];
   } x;
   struct {
      const char *name;
   } simple;
   const char *path;
   char     type;
};

struct _Evas_Font_Alias
{
   const char *alias;
   Evas_Font  *fn;
};

struct _Evas_Font_Description
{
   int ref;
   /* We assume everywhere this is stringshared */
   const char *name;
   const char *fallbacks;
   const char *lang;

   Evas_Font_Slant slant;
   Evas_Font_Weight weight;
   Evas_Font_Width width;
   Evas_Font_Spacing spacing;

   Eina_Bool is_new : 1;
};

struct _Evas_Object_Filter_Data
{
   Eina_Stringshare    *name;
   Eina_Stringshare    *code;
   Evas_Filter_Program *chain;
   Eina_Hash           *sources; // Evas_Filter_Proxy_Binding
   Eina_Inlist         *data; // Evas_Filter_Data_Binding
   void                *output;
   struct {
      struct {
         Eina_Stringshare *name;
         double            value;
      } cur;
      struct {
         Eina_Stringshare *name;
         double            value;
      } next;
      double               pos;
   } state;
   Eina_Bool            changed : 1;
   Eina_Bool            invalid : 1; // Code parse failed
   Eina_Bool            async : 1;
};

struct _Evas_Object_Func
{
   void (*free) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   void (*render) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data,
                   void *output, void *context, void *surface, int x, int y, Eina_Bool do_async);
   void (*render_pre) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   void (*render_post) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);

   unsigned int  (*type_id_get) (Evas_Object *obj);
   unsigned int  (*visual_id_get) (Evas_Object *obj);
   void *(*engine_data_get) (Evas_Object *obj);

   void (*store) (Evas_Object *obj);
   void (*unstore) (Evas_Object *obj);

   int  (*is_visible) (Evas_Object *obj);
   int  (*was_visible) (Evas_Object *obj);

   int  (*is_opaque) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   int  (*was_opaque) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);

   int  (*is_inside) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data,
                      Evas_Coord x, Evas_Coord y);
   int  (*was_inside) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data,
                       Evas_Coord x, Evas_Coord y);

   void (*coords_recalc) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);

   void (*scale_update) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);

   int (*has_opaque_rect) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   int (*get_opaque_rect) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data,
                           Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

   int (*can_map) (Evas_Object *obj);

// new render2 functions

   void (*render2_walk) (Evas_Object *obj, Evas_Object_Protected_Data *pd,
                         void *type_private_data, void *updates,
                         int offx, int offy);
//   void (*render2) (Evas_Object *obj, Evas_Object_Protected_Data *pd,
//                    void *type_private_data, void *output, void *context,
//                    void *surface, int x, int y);
};

struct _Evas_Func
{
   void *(*info)                           (Evas *e);
   void (*info_free)                       (Evas *e, void *info);
   int  (*setup)                           (Evas *e, void *info);

   void (*output_free)                     (void *data);
   void (*output_resize)                   (void *data, int w, int h);
   void (*output_tile_size_set)            (void *data, int w, int h);
   void (*output_redraws_rect_add)         (void *data, int x, int y, int w, int h);
   void (*output_redraws_rect_del)         (void *data, int x, int y, int w, int h);
   void (*output_redraws_clear)            (void *data);
   void *(*output_redraws_next_update_get) (void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
   void (*output_redraws_next_update_push) (void *data, void *surface, int x, int y, int w, int h, Evas_Render_Mode render_mode);
   void (*output_flush)                    (void *data, Evas_Render_Mode render_mode);
   void (*output_idle_flush)               (void *data);
   void (*output_dump)                     (void *data);

   void *(*context_new)                    (void *data);
   void *(*context_dup)                    (void *data, void *context);
   Eina_Bool (*canvas_alpha_get)           (void *data, void *context);
   void (*context_free)                    (void *data, void *context);
   void (*context_clip_set)                (void *data, void *context, int x, int y, int w, int h);
   void (*context_clip_image_set)          (void *data, void *context, void *surface, int x, int y, Evas_Public_Data *evas, Eina_Bool do_async);
   void (*context_clip_image_unset)        (void *data, void *context);
   void (*context_clip_image_get)          (void *data, void *context, void **surface, int *x, int *y); /* incref surface if not NULL */
   void (*context_clip_clip)               (void *data, void *context, int x, int y, int w, int h);
   void (*context_clip_unset)              (void *data, void *context);
   int  (*context_clip_get)                (void *data, void *context, int *x, int *y, int *w, int *h);
   void (*context_color_set)               (void *data, void *context, int r, int g, int b, int a);
   int  (*context_color_get)               (void *data, void *context, int *r, int *g, int *b, int *a);
   void (*context_multiplier_set)          (void *data, void *context, int r, int g, int b, int a);
   void (*context_multiplier_unset)        (void *data, void *context);
   int  (*context_multiplier_get)          (void *data, void *context, int *r, int *g, int *b, int *a);
   void (*context_cutout_add)              (void *data, void *context, int x, int y, int w, int h);
   void (*context_cutout_clear)            (void *data, void *context);
   void (*context_anti_alias_set)          (void *data, void *context, unsigned char aa);
   unsigned char (*context_anti_alias_get) (void *data, void *context);
   void (*context_color_interpolation_set) (void *data, void *context, int color_space);
   int  (*context_color_interpolation_get) (void *data, void *context);
   void (*context_render_op_set)           (void *data, void *context, int render_op);
   int  (*context_render_op_get)           (void *data, void *context);

   void (*rectangle_draw)                  (void *data, void *context, void *surface, int x, int y, int w, int h, Eina_Bool do_async);

   void (*line_draw)                       (void *data, void *context, void *surface, int x1, int y1, int x2, int y2, Eina_Bool do_async);

   void *(*polygon_point_add)              (void *data, void *context, void *polygon, int x, int y);
   void *(*polygon_points_clear)           (void *data, void *context, void *polygon);
   void (*polygon_draw)                    (void *data, void *context, void *surface, void *polygon, int x, int y, Eina_Bool do_async);

   void *(*image_load)                     (void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo);
   void *(*image_mmap)                     (void *data, Eina_File *f, const char *key, int *error, Evas_Image_Load_Opts *lo);
   void *(*image_new_from_data)            (void *data, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
   void *(*image_new_from_copied_data)     (void *data, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
   void (*image_free)                      (void *data, void *image);
   void *(*image_ref)                      (void *data, void *image);
   void (*image_size_get)                  (void *data, void *image, int *w, int *h);
   void *(*image_size_set)                 (void *data, void *image, int w, int h);
   void (*image_stride_get)                (void *data, void *image, int *stride);
   void *(*image_dirty_region)             (void *data, void *image, int x, int y, int w, int h);
   void *(*image_data_get)                 (void *data, void *image, int to_write, DATA32 **image_data, int *err, Eina_Bool *tofree);
   void *(*image_data_put)                 (void *data, void *image, DATA32 *image_data);
   void *(*image_data_direct)              (void *data, void *image, Evas_Colorspace *cspace);
   void  (*image_data_preload_request)     (void *data, void *image, const Eo *target);
   void  (*image_data_preload_cancel)      (void *data, void *image, const Eo *target);
   void *(*image_alpha_set)                (void *data, void *image, int has_alpha);
   int  (*image_alpha_get)                 (void *data, void *image);
   void *(*image_orient_set)               (void *data, void *image, Evas_Image_Orient orient);
   Evas_Image_Orient (*image_orient_get)   (void *data, void *image);
   Eina_Bool (*image_draw)                 (void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth, Eina_Bool do_async);
   void (*image_colorspace_set)            (void *data, void *image, Evas_Colorspace cspace);
   Evas_Colorspace (*image_colorspace_get) (void *data, void *image);
   Evas_Colorspace (*image_file_colorspace_get)(void *data, void *image);
   Eina_Bool (*image_can_region_get)       (void *data, void *image);

   /* image data map/unmap: direct or indirect access to pixels data */
   void *(*image_data_map)                 (void *data, void **image, int *length, int *stride, int x, int y, int w, int h, Evas_Colorspace cspace, Efl_Gfx_Buffer_Access_Mode mode);
   Eina_Bool (*image_data_unmap)           (void *data, void *image, void *map, int length);
   int (*image_data_maps_get)              (void *data, const void *image, void **maps, int *lengths);

   int (*image_native_init)                (void *data, Evas_Native_Surface_Type type);
   void (*image_native_shutdown)           (void *data, Evas_Native_Surface_Type type);
   void *(*image_native_set)               (void *data, void *image, void *native);
   void *(*image_native_get)               (void *data, void *image);

   void (*image_cache_flush)               (void *data);
   void (*image_cache_set)                 (void *data, int bytes);
   int  (*image_cache_get)                 (void *data);

   Evas_Font_Set *(*font_load)             (void *data, const char *name, int size, Font_Rend_Flags wanted_rend);
   Evas_Font_Set *(*font_memory_load)      (void *data, const char *source, const char *name, int size, const void *fdata, int fdata_size, Font_Rend_Flags wanted_rend);
   Evas_Font_Set *(*font_add)              (void *data, Evas_Font_Set *font, const char *name, int size, Font_Rend_Flags wanted_rend);
   Evas_Font_Set *(*font_memory_add)       (void *data, Evas_Font_Set *font, const char *source, const char *name, int size, const void *fdata, int fdata_size, Font_Rend_Flags wanted_rend);
   void (*font_free)                       (void *data, Evas_Font_Set *font);
   int  (*font_ascent_get)                 (void *data, Evas_Font_Set *font);
   int  (*font_descent_get)                (void *data, Evas_Font_Set *font);
   int  (*font_max_ascent_get)             (void *data, Evas_Font_Set *font);
   int  (*font_max_descent_get)            (void *data, Evas_Font_Set *font);
   void (*font_string_size_get)            (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int *w, int *h);
   int  (*font_inset_get)                  (void *data, Evas_Font_Set *font, const Evas_Text_Props *text_props);
   int  (*font_h_advance_get)              (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props);
   int  (*font_v_advance_get)              (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props);
   int  (*font_char_coords_get)            (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int pos, int *cx, int *cy, int *cw, int *ch);
   int  (*font_char_at_coords_get)         (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int x, int y, int *cx, int *cy, int *cw, int *ch);
   Eina_Bool (*font_draw)                  (void *data, void *context, void *surface, Evas_Font_Set *font, int x, int y, int w, int h, int ow, int oh, Evas_Text_Props *intl_props, Eina_Bool do_async);
   void (*font_cache_flush)                (void *data);
   void (*font_cache_set)                  (void *data, int bytes);
   int  (*font_cache_get)                  (void *data);

   /* Engine functions will over time expand from here */

   void (*font_hinting_set)                (void *data, Evas_Font_Set *font, int hinting);
   int  (*font_hinting_can_hint)           (void *data, int hinting);

/*    void (*image_rotation_set)              (void *data, void *image); */

   void (*image_scale_hint_set)            (void *data, void *image, int hint);
   int  (*image_scale_hint_get)            (void *data, void *image);
   int  (*font_last_up_to_pos)             (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int x, int y);

   Eina_Bool (*image_map_draw)                  (void *data, void *context, void *surface, void *image, RGBA_Map *m, int smooth, int level, Eina_Bool do_async);
   void *(*image_map_surface_new)          (void *data, int w, int h, int alpha);
   void (*image_map_clean)                 (void *data, RGBA_Map *m);
   void *(*image_scaled_update)            (void *data, void *scaled, void *image, int dst_w, int dst_h, Eina_Bool smooth, Eina_Bool alpha, Evas_Colorspace cspace);

   void (*image_content_hint_set)          (void *data, void *surface, int hint);
   int  (*image_content_hint_get)          (void *data, void *surface);
   int  (*font_pen_coords_get)             (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int pos, int *cpen_x, int *cy, int *cadv, int *ch);
   Eina_Bool (*font_text_props_info_create) (void *data, Evas_Font_Instance *fi, const Eina_Unicode *text, Evas_Text_Props *intl_props, const Evas_BiDi_Paragraph_Props *par_props, size_t pos, size_t len, Evas_Text_Props_Mode mode, const char *lang);
   int  (*font_right_inset_get)            (void *data, Evas_Font_Set *font, const Evas_Text_Props *text_props);

   /* EFL-GL Glue Layer */
   void *(*gl_surface_create)            (void *data, void *config, int w, int h);
   void *(*gl_pbuffer_surface_create)    (void *data, void *config, int w, int h, int const *attrib_list);
   int  (*gl_surface_destroy)            (void *data, void *surface);
   void *(*gl_context_create)            (void *data, void *share_context, int version, void *(*native_context_get)(void *ctx), void *(*engine_data_get)(void *evasgl));
   int  (*gl_context_destroy)            (void *data, void *context);
   int  (*gl_make_current)               (void *data, void *surface, void *context);
   const char *(*gl_string_query)        (void *data, int name);
   void *(*gl_proc_address_get)          (void *data, const char *name);
   int  (*gl_native_surface_get)         (void *data, void *surface, void *native_surface);
   void *(*gl_api_get)                   (void *data, int version);
   void (*gl_direct_override_get)        (void *data, Eina_Bool *override, Eina_Bool *force_off);
   void (*gl_get_pixels_set)             (void *data, void *get_pixels, void *get_pixels_data, void *obj);
   Eina_Bool (*gl_surface_lock)          (void *data, void *surface);
   Eina_Bool (*gl_surface_read_pixels)   (void *data, void *surface, int x, int y, int w, int h, Evas_Colorspace cspace, void *pixels);
   Eina_Bool (*gl_surface_unlock)        (void *data, void *surface);
   int  (*gl_error_get)                  (void *data);
   void *(*gl_current_context_get)       (void *data);
   void *(*gl_current_surface_get)       (void *data);
   int  (*gl_rotation_angle_get)         (void *data);
   Eina_Bool (*gl_surface_query)         (void *data, void *surface, int attr, void *value);
   Eina_Bool (*gl_surface_direct_renderable_get) (void *data, Evas_Native_Surface *ns, Eina_Bool *override, void *surface);
   void (*gl_image_direct_set)           (void *data, void *image, Eina_Bool direct);
   int  (*gl_image_direct_get)           (void *data, void *image);
   void (*gl_get_pixels_pre)             (void *data);
   void (*gl_get_pixels_post)            (void *data);

   int  (*image_load_error_get)          (void *data, void *image);
   int  (*font_run_end_get)              (void *data, Evas_Font_Set *font, Evas_Font_Instance **script_fi, Evas_Font_Instance **cur_fi, Evas_Script_Type script, const Eina_Unicode *text, int run_len);

   /* animated feature */
   Eina_Bool (*image_animated_get)       (void *data, void *image);
   int (*image_animated_frame_count_get) (void *data, void *image);
   Evas_Image_Animated_Loop_Hint  (*image_animated_loop_type_get) (void *data, void *image);
   int (*image_animated_loop_count_get)  (void *data, void *image);
   double (*image_animated_frame_duration_get) (void *data, void *image, int start_frame, int frame_num);
   Eina_Bool (*image_animated_frame_set) (void *data, void *image, int frame_index);

   /* max size query */
   void (*image_max_size_get)            (void *data, int *maxw, int *maxh);

   /* multiple font draws */
   Eina_Bool (*multi_font_draw)          (void *data, void *context, void *surface, Evas_Font_Set *font, int x, int y, int w, int h, int ow, int oh, Evas_Font_Array *texts, Eina_Bool do_async);

   Eina_Bool (*pixel_alpha_get)          (void *image, int x, int y, DATA8 *alpha, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

   void (*context_flush)                 (void *data);

   /* 3D features */
   void *(*drawable_new)                 (void *data, int w, int h, int alpha);
   void  (*drawable_free)                (void *data, void *drawable);
   void  (*drawable_size_get)            (void *data, void *drawable, int *w, int *h);
   void *(*image_drawable_set)           (void *data, void *image, void *drawable);

   void  (*drawable_scene_render)        (void *data, void *drawable, void *scene_data);
   Eina_Bool (*drawable_scene_render_to_texture) (void *data, void *drawable, void *scene_data);

   int (*drawable_texture_color_pick_id_get) (void *drawable);
   void (*drawable_texture_pixel_color_get) (unsigned int tex EINA_UNUSED, int x, int y, Evas_Color *color, void *drawable);

   void *(*texture_new)                  (void *data, Eina_Bool use_atlas);
   void  (*texture_free)                 (void *data, void *texture);
   void  (*texture_size_get)             (void *data, void *texture, int *w, int *h);
   void  (*texture_wrap_set)             (void *data, void *texture, Evas_Canvas3D_Wrap_Mode s, Evas_Canvas3D_Wrap_Mode t);
   void  (*texture_wrap_get)             (void *data, void *texture, Evas_Canvas3D_Wrap_Mode *s, Evas_Canvas3D_Wrap_Mode *t);
   void  (*texture_filter_set)           (void *data, void *texture, Evas_Canvas3D_Texture_Filter min, Evas_Canvas3D_Texture_Filter mag);
   void  (*texture_filter_get)           (void *data, void *texture, Evas_Canvas3D_Texture_Filter *min, Evas_Canvas3D_Texture_Filter *mag);
   void  (*texture_image_set)            (void *data, void *texture, void *image);
   void *(*texture_image_get)            (void *data, void *texture);

   Ector_Surface *(*ector_create)        (void *data);
   void  (*ector_destroy)                (void *data, Ector_Surface *surface);
   Ector_Buffer *(*ector_buffer_wrap)    (void *data, Evas *e, void *engine_image, Eina_Bool is_rgba_image);
   Ector_Buffer *(*ector_buffer_new)     (void *data, Evas *e, void *pixels, int width, int height, int stride, Efl_Gfx_Colorspace cspace, Eina_Bool writeable, int l, int r, int t, int b, Ector_Buffer_Flag flags);
   void  (*ector_begin)                  (void *data, void *context, Ector_Surface *ector, void *surface, void *engine_data, int x, int y, Eina_Bool do_async);
   void  (*ector_renderer_draw)          (void *data, void *context, void *surface, void *engine_data, Ector_Renderer *r, Eina_Array *clips, Eina_Bool do_async);
   void  (*ector_end)                    (void *data, void *context, Ector_Surface *ector, void *surface, void *engine_data, Eina_Bool do_async);
   void* (*ector_new)                    (void *data, void *context, Ector_Surface *ector, void *surface);
   void  (*ector_free)                   (void *engine_data);
};

struct _Evas_Image_Save_Func
{
  int (*image_save) (RGBA_Image *im, const char *file, const char *key, int quality, int compress, const char *encoding);
};

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *evas_object_new(Evas *e);
void evas_object_change_reset(Evas_Object *obj);
void evas_object_cur_prev(Evas_Object *obj);
void evas_object_free(Evas_Object *obj, int clean_layer);
void evas_object_update_bounding_box(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Smart_Data *s);
void evas_object_inject(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas *e);
void evas_object_release(Evas_Object *obj, Evas_Object_Protected_Data *pd, int clean_layer);
void evas_object_change(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_content_change(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_clip_changes_clean(Evas_Object *obj);
void evas_object_render_pre_visible_change(Eina_Array *rects, Evas_Object *obj, int is_v, int was_v);
void evas_object_render_pre_clipper_change(Eina_Array *rects, Evas_Object *obj);
void evas_object_render_pre_prev_cur_add(Eina_Array *rects, Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_render_pre_effect_updates(Eina_Array *rects, Evas_Object *obj, int is_v, int was_v);
void evas_rects_return_difference_rects(Eina_Array *rects, int x, int y, int w, int h, int xx, int yy, int ww, int hh);

void evas_object_clip_dirty(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_recalc_clippees(Evas_Object_Protected_Data *pd);
Evas_Layer *evas_layer_new(Evas *e);
void evas_layer_pre_free(Evas_Layer *lay);
void evas_layer_free_objects(Evas_Layer *lay);
void evas_layer_clean(Evas *e);
Evas_Layer *evas_layer_find(Evas *e, short layer_num);
void evas_layer_add(Evas_Layer *lay);
void evas_layer_del(Evas_Layer *lay);

int evas_object_was_in_output_rect(Evas_Object *obj, Evas_Object_Protected_Data *pd, int x, int y, int w, int h);

int evas_object_was_opaque(Evas_Object *obj, Evas_Object_Protected_Data *pd);
int evas_object_is_inside(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Coord x, Evas_Coord y);
int evas_object_was_inside(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Coord x, Evas_Coord y);
void evas_object_clip_across_check(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_clip_across_clippees_check(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_mapped_clip_across_mark(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_event_callback_call(Evas *e, Evas_Callback_Type type, void *event_info);
void evas_object_event_callback_call(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Callback_Type type, void *event_info, int event_id);
Eina_List *evas_event_objects_event_list(Evas *e, Evas_Object *stop, int x, int y);
int evas_mem_free(int mem_required);
int evas_mem_degrade(int mem_required);
void evas_debug_error(void);
void evas_debug_input_null(void);
void evas_debug_magic_null(void);
void evas_debug_magic_wrong(DATA32 expected, DATA32 supplied);
void evas_debug_generic(const char *str);
const char *evas_debug_magic_string_get(DATA32 magic);
void evas_object_smart_use(Evas_Smart *s);
void evas_object_smart_unuse(Evas_Smart *s);
void evas_smart_cb_descriptions_fix(Evas_Smart_Cb_Description_Array *a) EINA_ARG_NONNULL(1);
Eina_Bool evas_smart_cb_descriptions_resize(Evas_Smart_Cb_Description_Array *a, unsigned int size) EINA_ARG_NONNULL(1);
const Evas_Smart_Cb_Description *evas_smart_cb_description_find(const Evas_Smart_Cb_Description_Array *a, const char *name) EINA_ARG_NONNULL(1, 2) EINA_PURE;

Evas_Object *_evas_object_image_source_get(Evas_Object *obj);
Eina_Bool _evas_object_image_preloading_get(const Evas_Object *obj);
void _evas_object_image_preloading_set(Evas_Object *obj, Eina_Bool preloading);
void _evas_object_image_preloading_check(Evas_Object *obj);
Evas_Object *_evas_object_image_video_parent_get(Evas_Object *obj);
void _evas_object_image_video_overlay_show(Evas_Object *obj);
void _evas_object_image_video_overlay_hide(Evas_Object *obj);
void _evas_object_image_video_overlay_do(Evas_Object *obj);
void _evas_object_image_free(Evas_Object *obj);
void evas_object_smart_bounding_box_get(Evas_Object *eo_obj,
                                        Evas_Coord_Rectangle *cur_bounding_box,
                                        Evas_Coord_Rectangle *prev_bounding_box);
void evas_object_smart_del(Evas_Object *obj);
void evas_object_smart_cleanup(Evas_Object *obj);
void evas_object_smart_member_raise(Evas_Object *member);
void evas_object_smart_member_lower(Evas_Object *member);
void evas_object_smart_member_stack_above(Evas_Object *member, Evas_Object *other);
void evas_object_smart_member_stack_below(Evas_Object *member, Evas_Object *other);
const Eina_Inlist *evas_object_smart_members_get_direct(const Evas_Object *obj);
void _evas_object_smart_members_all_del(Evas_Object *obj);
void evas_call_smarts_calculate(Evas *e);
void evas_object_smart_bounding_box_update(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
void evas_object_smart_need_bounding_box_update(Evas_Object *eo_obj, Evas_Smart_Data *o, Evas_Object_Protected_Data *obj);
Eina_Bool evas_object_smart_changed_get(Evas_Object *eo_obj);
void *evas_mem_calloc(int size);
void _evas_post_event_callback_call(Evas *e, Evas_Public_Data* e_pd);
void _evas_post_event_callback_free(Evas *e);
void evas_event_callback_list_post_free(Eina_Inlist **list);
void evas_object_event_callback_all_del(Evas_Object *obj);
void evas_object_event_callback_cleanup(Evas_Object *obj);
void evas_event_callback_all_del(Evas *e);
void evas_event_callback_cleanup(Evas *e);
void evas_object_inform_call_show(Evas_Object *obj);
void evas_object_inform_call_hide(Evas_Object *obj);
void evas_object_inform_call_move(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_inform_call_resize(Evas_Object *obj);
void evas_object_inform_call_restack(Evas_Object *obj);
void evas_object_inform_call_changed_size_hints(Evas_Object *obj);
void evas_object_inform_call_image_preloaded(Evas_Object *obj);
void evas_object_inform_call_image_unloaded(Evas_Object *obj);
void evas_object_inform_call_image_resize(Evas_Object *obj);
void evas_object_intercept_cleanup(Evas_Object *obj);
int evas_object_intercept_call_show(Evas_Object *obj, Evas_Object_Protected_Data *pd);
int evas_object_intercept_call_hide(Evas_Object *obj, Evas_Object_Protected_Data *pd);
int evas_object_intercept_call_move(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Coord x, Evas_Coord y);
int evas_object_intercept_call_resize(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Coord w, Evas_Coord h);
int evas_object_intercept_call_raise(Evas_Object *obj, Evas_Object_Protected_Data *pd);
int evas_object_intercept_call_lower(Evas_Object *obj, Evas_Object_Protected_Data *pd);
int evas_object_intercept_call_stack_above(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Object *above);
int evas_object_intercept_call_stack_below(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Object *below);
int evas_object_intercept_call_layer_set(Evas_Object *obj, Evas_Object_Protected_Data *pd, int l);
int evas_object_intercept_call_color_set(Evas_Object *obj, Evas_Object_Protected_Data *pd, int r, int g, int b, int a);
int evas_object_intercept_call_clip_set(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Object *clip);
int evas_object_intercept_call_clip_unset(Evas_Object *obj, Evas_Object_Protected_Data *pd);
int evas_object_intercept_call_focus_set(Evas_Object *obj, Evas_Object_Protected_Data *pd, Eina_Bool focus);
void evas_object_grabs_cleanup(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_key_grab_free(Evas_Object *obj, Evas_Object_Protected_Data *pd, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);
void evas_font_dir_cache_free(void);
const char *evas_font_dir_cache_find(char *dir, char *font);
Eina_List *evas_font_dir_available_list(const Evas* evas);
void evas_font_dir_available_list_free(Eina_List *available);
void evas_font_free(Evas *evas, void *font);
void evas_fonts_zero_free(Evas *evas);
void evas_fonts_zero_pressure(Evas *evas);
void evas_font_name_parse(Evas_Font_Description *fdesc, const char *name);
int evas_font_style_find(const char *start, const char *end, Evas_Font_Style style);
Evas_Font_Description *evas_font_desc_new(void);
Evas_Font_Description *evas_font_desc_dup(const Evas_Font_Description *fdesc);
void evas_font_desc_unref(Evas_Font_Description *fdesc);
int evas_font_desc_cmp(const Evas_Font_Description *a, const Evas_Font_Description *b);
Evas_Font_Description *evas_font_desc_ref(Evas_Font_Description *fdesc);
const char *evas_font_lang_normalize(const char *lang);
void * evas_font_load(Evas *evas, Evas_Font_Description *fdesc, const char *source, Evas_Font_Size size);
void evas_font_load_hinting_set(Evas *evas, void *font, int hinting);
void evas_object_smart_member_cache_invalidate(Evas_Object *obj, Eina_Bool pass_events, Eina_Bool freeze_events, Eina_Bool sourve_invisible);
void evas_text_style_pad_get(Evas_Text_Style_Type style, int *l, int *r, int *t, int *b);
void _evas_object_text_rehint(Evas_Object *obj);
void _evas_object_textblock_rehint(Evas_Object *obj);

void evas_unref_queue_image_put(Evas_Public_Data *pd, void *image);
void evas_unref_queue_glyph_put(Evas_Public_Data *pd, void *glyph);
void evas_unref_queue_texts_put(Evas_Public_Data *pd, void *glyph);

void evas_draw_image_map_async_check(Evas_Object_Protected_Data *obj,
                                     void *data, void *context, void *surface,
                                     void *image, RGBA_Map *m, int smooth,
                                     int level, Eina_Bool do_async);
void evas_font_draw_async_check(Evas_Object_Protected_Data *obj,
                                void *data, void *context, void *surface,
                                Evas_Font_Set *font,
                                int x, int y, int w, int h, int ow, int oh,
                                Evas_Text_Props *intl_props, Eina_Bool do_async);

void _freeze_events_set(Eo *obj, void *_pd, va_list *list);
void _freeze_events_get(Eo *obj, void *_pd, va_list *list);
void _pass_events_set(Eo *obj, void *_pd, va_list *list);
void _pass_events_get(Eo *obj, void *_pd, va_list *list);
void _repeat_events_set(Eo *obj, void *_pd, va_list *list);
void _repeat_events_get(Eo *obj, void *_pd, va_list *list);
void _propagate_events_set(Eo *obj, void *_pd, va_list *list);
void _propagate_events_get(Eo *obj, void *_pd, va_list *list);
void _pointer_mode_set(Eo *obj, void *_pd, va_list *list);
void _pointer_mode_get(Eo *obj, void *_pd, va_list *list);
void _key_grab(Eo *obj, void *_pd, va_list *list);
void _key_ungrab(Eo *obj, void *_pd, va_list *list);
void _focus_set(Eo *obj, void *_pd, va_list *list);
void _focus_get(Eo *obj, void *_pd, va_list *list);
void _name_set(Eo *obj, void *_pd, va_list *list);
void _name_get(Eo *obj, void *_pd, va_list *list);
void _name_child_find(Eo *obj, void *_pd, va_list *list);
void _layer_set(Eo *obj, void *_pd, va_list *list);
void _layer_get(Eo *obj, void *_pd, va_list *list);
void _clip_set(Eo *obj, void *_pd, va_list *list);
void _clip_get(Eo *obj, void *_pd, va_list *list);
void _clip_unset(Eo *obj, void *_pd, va_list *list);
void _clipees_get(Eo *obj, void *_pd, va_list *list);
void _clipees_has(Eo *obj, void *_pd, va_list *list);
void _map_enable_set(Eo *obj, void *_pd, va_list *list);
void _map_enable_get(Eo *obj, void *_pd, va_list *list);
void _map_source_set(Eo *obj, void *_pd, va_list *list);
void _map_source_get(Eo *obj, void *_pd, va_list *list);
void _map_set(Eo *obj, void *_pd, va_list *list);
void _map_get(Eo *obj, void *_pd, va_list *list);
void _raise(Eo *obj, void *_pd, va_list *list);
void _lower(Eo *obj, void *_pd, va_list *list);
void _stack_above(Eo *obj, void *_pd, va_list *list);
void _stack_below(Eo *obj, void *_pd, va_list *list);
void _above_get(Eo *obj, void *_pd, va_list *list);
void _below_get(Eo *obj, void *_pd, va_list *list);
void _smart_move_children_relative(Eo *obj, void *_pd, va_list *list);
void _smart_clipped_clipper_get(Eo *obj, void *_pd, va_list *list);
void _evas_object_clip_prev_reset(Evas_Object_Protected_Data *obj, Eina_Bool cur_prev);

void _canvas_event_default_flags_set(Eo *e, void *_pd, va_list *list);
void _canvas_event_default_flags_get(Eo *e, void *_pd, va_list *list);
void _canvas_event_freeze(Eo *e, void *_pd, va_list *list);
void _canvas_event_thaw(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_mouse_down(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_mouse_up(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_mouse_cancel(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_mouse_wheel(Eo *e, void *_pd, va_list *list);
void _canvas_event_input_mouse_move(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_mouse_move(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_mouse_in(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_mouse_out(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_multi_down(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_multi_up(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_multi_move(Eo *e, void *_pd, va_list *list);
void _canvas_event_input_multi_down(Eo *e, void *_pd, va_list *list);
void _canvas_event_input_multi_up(Eo *e, void *_pd, va_list *list);
void _canvas_event_input_multi_move(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_key_down(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_key_up(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_key_down_with_keycode(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_key_up_with_keycode(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_hold(Eo *e, void *_pd, va_list *list);
void _canvas_event_feed_axis_update(Eo *e, void *_pd, va_list *list);
void _canvas_event_refeed_event(Eo *e, void *_pd, va_list *list);
void _canvas_event_down_count_get(Eo *e, void *_pd, va_list *list);
void _canvas_tree_objects_at_xy_get(Eo *e, void *_pd, va_list *list);
void _canvas_focus_get(Eo *e, void *_pd, va_list *list);
void _canvas_font_path_clear(Eo *e, void *_pd, va_list *list);
void _canvas_font_path_append(Eo *e, void *_pd, va_list *list);
void _canvas_font_path_prepend(Eo *e, void *_pd, va_list *list);
void _canvas_font_path_list(Eo *e, void *_pd, va_list *list);
void _canvas_font_hinting_set(Eo *e, void *_pd, va_list *list);
void _canvas_font_hinting_get(Eo *e, void *_pd, va_list *list);
void _canvas_font_hinting_can_hint(Eo *e, void *_pd, va_list *list);
void _canvas_font_cache_flush(Eo *e, void *_pd, va_list *list);
void _canvas_font_cache_set(Eo *e, void *_pd, va_list *list);
void _canvas_font_cache_get(Eo *e, void *_pd, va_list *list);
void _canvas_font_available_list(Eo *e, void *_pd, va_list *list);

void _canvas_key_modifier_get(Eo *e, void *_pd, va_list *list);
void _canvas_key_lock_get(Eo *e, void *_pd, va_list *list);
void _canvas_key_modifier_add(Eo *e, void *_pd, va_list *list);
void _canvas_key_modifier_del(Eo *e, void *_pd, va_list *list);
void _canvas_key_lock_add(Eo *e, void *_pd, va_list *list);
void _canvas_key_lock_del(Eo *e, void *_pd, va_list *list);
void _canvas_key_modifier_on(Eo *e, void *_pd, va_list *list);
void _canvas_key_modifier_off(Eo *e, void *_pd, va_list *list);
void _canvas_key_lock_on(Eo *e, void *_pd, va_list *list);
void _canvas_key_lock_off(Eo *e, void *_pd, va_list *list);
void _canvas_key_modifier_mask_get(Eo *e, void *_pd, va_list *list);

void _canvas_damage_rectangle_add(Eo *obj, void *_pd, va_list *list);
void _canvas_obscured_rectangle_add(Eo *obj, void *_pd, va_list *list);
void _canvas_obscured_clear(Eo *obj, void *_pd, va_list *list);
void _canvas_render_async(Eo *obj, void *_pd, va_list *list);
void _canvas_render_updates(Eo *obj, void *_pd, va_list *list);
void _canvas_render(Eo *e, void *_pd, va_list *list);
void _canvas_norender(Eo *e, void *_pd, va_list *list);
void _canvas_render_idle_flush(Eo *e, void *_pd, va_list *list);
void _canvas_sync(Eo *obj, void *_pd, va_list *list);
void _canvas_render_dump(Eo *obj, void *_pd, va_list *list);

void _canvas_object_bottom_get(Eo *e, void *_pd, va_list *list);
void _canvas_object_top_get(Eo *e, void *_pd, va_list *list);

void _canvas_touch_point_list_count(Eo *obj, void *_pd, va_list *list);
void _canvas_touch_point_list_nth_xy_get(Eo *obj, void *_pd, va_list *list);
void _canvas_touch_point_list_nth_id_get(Eo *obj, void *_pd, va_list *list);
void _canvas_touch_point_list_nth_state_get(Eo *obj, void *_pd, va_list *list);

void _canvas_image_cache_flush(Eo *e, void *_pd, va_list *list);
void _canvas_image_cache_reload(Eo *e, void *_pd, va_list *list);
void _canvas_image_cache_set(Eo *e, void *_pd, va_list *list);
void _canvas_image_cache_get(Eo *e, void *_pd, va_list *list);
void _canvas_image_max_size_get(Eo *e, void *_pd, va_list *list);

void _canvas_object_name_find(Eo *e, void *_pd, va_list *list);

void _canvas_object_top_at_xy_get(Eo *e, void *_pd, va_list *list);
void _canvas_object_top_in_rectangle_get(Eo *e, void *_pd, va_list *list);
void _canvas_objects_at_xy_get(Eo *e, void *_pd, va_list *list);
void _canvas_objects_in_rectangle_get(Eo *obj, void *_pd, va_list *list);

void _canvas_smart_objects_calculate(Eo *e, void *_pd, va_list *list);
void _canvas_smart_objects_calculate_count_get(Eo *e, void *_pd, va_list *list);

/* Node functions. */
void evas_canvas3d_node_traverse(Evas_Canvas3D_Node *from, Evas_Canvas3D_Node *to, Evas_Canvas3D_Node_Traverse_Type type, Eina_Bool skip, Evas_Canvas3D_Node_Func func, void *data);
void evas_canvas3d_node_tree_traverse(Evas_Canvas3D_Node *root, Evas_Canvas3D_Tree_Traverse_Type type, Eina_Bool skip, Evas_Canvas3D_Node_Func func, void *data);
Eina_Bool evas_canvas3d_node_mesh_collect(Evas_Canvas3D_Node *node, void *data);
Eina_Bool evas_canvas3d_node_color_node_mesh_collect(Evas_Canvas3D_Node *node, void *data);
Eina_Bool evas_canvas3d_node_light_collect(Evas_Canvas3D_Node *node, void *data);
void evas_canvas3d_node_scene_root_add(Evas_Canvas3D_Node *node, Evas_Canvas3D_Scene *scene);
void evas_canvas3d_node_scene_root_del(Evas_Canvas3D_Node *node, Evas_Canvas3D_Scene *scene);
void evas_canvas3d_node_scene_camera_add(Evas_Canvas3D_Node *node, Evas_Canvas3D_Scene *scene);
void evas_canvas3d_node_scene_camera_del(Evas_Canvas3D_Node *node, Evas_Canvas3D_Scene *scene);
Eina_Bool node_aabb_update(Evas_Canvas3D_Node *node, void *data  EINA_UNUSED);

/* Camera functions. */
void evas_canvas3d_camera_node_add(Evas_Canvas3D_Camera *camera, Evas_Canvas3D_Node *node);
void evas_canvas3d_camera_node_del(Evas_Canvas3D_Camera *camera, Evas_Canvas3D_Node *node);

/* Light functions. */
void evas_canvas3d_light_node_add(Evas_Canvas3D_Light *light, Evas_Canvas3D_Node *node);
void evas_canvas3d_light_node_del(Evas_Canvas3D_Light *light, Evas_Canvas3D_Node *node);

/* Mesh functions. */
void evas_canvas3d_mesh_node_add(Evas_Canvas3D_Mesh *mesh, Evas_Canvas3D_Node *node);
void evas_canvas3d_mesh_node_del(Evas_Canvas3D_Mesh *mesh, Evas_Canvas3D_Node *node);
void evas_canvas3d_mesh_interpolate_position_get(Eina_Vector3 *out, const Evas_Canvas3D_Vertex_Buffer *pos0, const Evas_Canvas3D_Vertex_Buffer *pos1, Evas_Real weight, int index);
void evas_canvas3d_mesh_interpolate_vertex_buffer_get(Evas_Canvas3D_Mesh *mesh, int frame, Evas_Canvas3D_Vertex_Attrib attrib, Evas_Canvas3D_Vertex_Buffer *buffer0, Evas_Canvas3D_Vertex_Buffer *buffer1, Evas_Real *weight);
void evas_canvas3d_mesh_file_md2_set(Evas_Canvas3D_Mesh *mesh, const char *file);
void evas_canvas3d_mesh_save_obj(Evas_Canvas3D_Mesh *mesh, const char *file, Evas_Canvas3D_Mesh_Frame *f);
void evas_canvas3d_mesh_file_obj_set(Evas_Canvas3D_Mesh *mesh, const char *file);
Eina_Bool evas_canvas3d_mesh_aabb_add_to_frame(Evas_Canvas3D_Mesh_Data *pd, int frame, int stride);
void evas_canvas3d_mesh_file_eet_set(Evas_Canvas3D_Mesh *mesh, const char *file);
void evas_canvas3d_mesh_save_eet(Evas_Canvas3D_Mesh *mesh, const char *file, Evas_Canvas3D_Mesh_Frame *f);
void evas_canvas3d_mesh_file_ply_set(Evas_Canvas3D_Mesh *mesh, const char *file);
void evas_canvas3d_mesh_save_ply(Evas_Canvas3D_Mesh *mesh, const char *file, Evas_Canvas3D_Mesh_Frame *f);

/* Texture functions. */
void evas_canvas3d_texture_material_add(Evas_Canvas3D_Texture *texture, Evas_Canvas3D_Material *material);
void evas_canvas3d_texture_material_del(Evas_Canvas3D_Texture *texture, Evas_Canvas3D_Material *material);


/* Material functions. */
void evas_canvas3d_material_mesh_add(Evas_Canvas3D_Material *material, Evas_Canvas3D_Mesh *mesh);
void evas_canvas3d_material_mesh_del(Evas_Canvas3D_Material *material, Evas_Canvas3D_Mesh *mesh);

/* Scene functions. */
void evas_canvas3d_scene_data_init(Evas_Canvas3D_Scene_Public_Data *data);
void evas_canvas3d_scene_data_fini(Evas_Canvas3D_Scene_Public_Data *data);

/* Eet saver/loader functions */
Evas_Canvas3D_File_Eet *_evas_canvas3d_eet_file_new(void);
Eet_Data_Descriptor* _evas_canvas3d_eet_file_get();
void _evas_canvas3d_eet_file_init();
void _evas_canvas3d_eet_descriptor_shutdown();
void _evas_canvas3d_eet_file_free(Evas_Canvas3D_File_Eet* eet_file);


/* Filters */
void evas_filter_init(void);
void evas_filter_shutdown(void);

/* Ector */
Ector_Surface *evas_ector_get(Evas_Public_Data *evas);

/* Temporary save/load functions */
void evas_common_load_model_from_file(Evas_Canvas3D_Mesh *model, const char *file);
void evas_common_load_model_from_eina_file(Evas_Canvas3D_Mesh *model, const Eina_File *file);
void evas_common_save_model_to_file(const Evas_Canvas3D_Mesh *model, const char *file, Evas_Canvas3D_Mesh_Frame *f);
void evas_model_load_file_eet(Evas_Canvas3D_Mesh *mesh, Eina_File *file);
void evas_model_load_file_md2(Evas_Canvas3D_Mesh *mesh, Eina_File *file);
void evas_model_load_file_obj(Evas_Canvas3D_Mesh *mesh, Eina_File *file);
void evas_model_load_file_ply(Evas_Canvas3D_Mesh *mesh, Eina_File *file);
void evas_model_save_file_eet(const Evas_Canvas3D_Mesh *mesh, const char *file, Evas_Canvas3D_Mesh_Frame *f);
void evas_model_save_file_obj(const Evas_Canvas3D_Mesh *mesh, const char *file, Evas_Canvas3D_Mesh_Frame *f);
void evas_model_save_file_ply(const Evas_Canvas3D_Mesh *mesh, const char *file, Evas_Canvas3D_Mesh_Frame *f);

/* Primitives functions */
void evas_common_set_model_from_primitive(Evas_Canvas3D_Mesh *model, int frame, Evas_Canvas3D_Primitive_Data *primitive);
void evas_model_set_from_square_primitive(Evas_Canvas3D_Mesh *mesh, int frame);
void evas_model_set_from_cube_primitive(Evas_Canvas3D_Mesh *mesh, int frame);
void evas_model_set_from_cylinder_primitive(Evas_Canvas3D_Mesh *mesh, int frame, Evas_Canvas3D_Primitive_Mode mode, int precision, Eina_Vector2 tex_scale);
void evas_model_set_from_cone_primitive(Evas_Canvas3D_Mesh *mesh, int frame, Evas_Canvas3D_Primitive_Mode mode, int precision, Eina_Vector2 tex_scale);
void evas_model_set_from_sphere_primitive(Evas_Canvas3D_Mesh *mesh, int frame, Evas_Canvas3D_Primitive_Mode mode, int precision, Eina_Vector2 tex_scale);
void evas_model_set_from_torus_primitive(Evas_Canvas3D_Mesh *mesh, int frame, Evas_Real ratio, int precision, Eina_Vector2 tex_scale);
void evas_model_set_from_surface_primitive(Evas_Canvas3D_Mesh *mesh, int frame, Evas_Canvas3D_Surface_Func func, int precision, Eina_Vector2 tex_scale);
void evas_model_set_from_terrain_primitive(Evas_Canvas3D_Mesh *mesh, int frame, int precision, Eina_Vector2 tex_scale);

/* Filter functions */
Eina_Bool evas_filter_object_render(Eo *eo_obj, Evas_Object_Protected_Data *obj, void *output, void *context, void *surface, int x, int y, Eina_Bool do_async, Eina_Bool alpha);

extern int _evas_alloc_error;
extern int _evas_event_counter;

struct _Evas_Imaging_Image
{
   RGBA_Image *image;
};

struct _Evas_Imaging_Font
{
   RGBA_Font *font;
};

struct _Evas_Proxy_Render_Data
{
   Evas_Object_Protected_Data *proxy_obj;
   Evas_Object_Protected_Data *src_obj;
   Evas_Object *eo_proxy;
   Evas_Object *eo_src;
   Eina_Bool source_clip : 1;
};

void _evas_canvas_event_init(Evas *eo_e, Evas_Public_Data *e);
void _evas_canvas_event_shutdown(Evas *eo_e, Evas_Public_Data *e);

int evas_async_events_init(void);
int evas_async_events_shutdown(void);
int evas_async_target_del(const void *target);

EAPI int evas_thread_main_loop_begin(void);
EAPI int evas_thread_main_loop_end(void);

void _evas_preload_thread_init(void);
void _evas_preload_thread_shutdown(void);
Evas_Preload_Pthread *evas_preload_thread_run(void (*func_heavy)(void *data),
                                              void (*func_end)(void *data),
                                              void (*func_cancel)(void *data),
                                              const void *data);
Eina_Bool evas_preload_thread_cancel(Evas_Preload_Pthread *thread);

void _evas_walk(Evas_Public_Data *e_pd);
void _evas_unwalk(Evas_Public_Data *e_pd);

// expose for use in engines
EAPI int _evas_module_engine_inherit(Evas_Func *funcs, char *name);
EAPI const char *_evas_module_libdir_get(void);
const char *_evas_module_datadir_get(void);
EAPI Eina_List *_evas_canvas_image_data_unset(Evas *eo_e);
EAPI void _evas_canvas_image_data_regenerate(Eina_List *list);

Eina_Bool evas_render_mapped(Evas_Public_Data *e, Evas_Object *obj,
                             Evas_Object_Protected_Data *source_pd,
                             void *context, void *surface, int off_x, int off_y,
                             int mapped, int ecx, int ecy, int ecw, int ech,
                             Evas_Proxy_Render_Data *proxy_render_data,
                             int level, Eina_Bool use_mapped_ctx, Eina_Bool do_async);
void evas_render_invalidate(Evas *e);
void evas_render_object_recalc(Evas_Object *obj);
void evas_render_proxy_subrender(Evas *eo_e, Evas_Object *eo_source, Evas_Object *eo_proxy,
                                 Evas_Object_Protected_Data *proxy_obj, Eina_Bool do_async);
void evas_render_mask_subrender(Evas_Public_Data *e, Evas_Object_Protected_Data *mask, Evas_Object_Protected_Data *prev_mask, int level);

Eina_Bool evas_map_inside_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y);
Eina_Bool evas_map_coords_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y, Evas_Coord *mx, Evas_Coord *my, int grab);
Eina_Bool evas_object_map_update(Evas_Object *obj, int x, int y, int imagew, int imageh, int uvw, int uvh);
void evas_map_object_move_diff_set(Evas_Map *m, Evas_Coord diff_x, Evas_Coord diff_y);

Eina_List *evas_module_engine_list(void);

/* for updating touch point list */
void _evas_touch_point_append(Evas *e, int id, Evas_Coord x, Evas_Coord y);
void _evas_touch_point_update(Evas *e, int id, Evas_Coord x, Evas_Coord y, Evas_Touch_Point_State state);
void _evas_touch_point_remove(Evas *e, int id);

void _evas_device_cleanup(Evas *e);
Evas_Device *_evas_device_top_get(const Evas *e);

/* legacy/eo events */
Eina_Bool efl_pointer_event_legacy_info_set(Efl_Pointer_Event *evt, const void *event_info, Evas_Callback_Type type);
const void *efl_pointer_event_legacy_info_get(const Efl_Pointer_Event *evt, Evas_Callback_Type *ptype, Eina_Bool multi);

Eina_Bool evas_vg_loader_svg(Evas_Object *vg, const Eina_File *f, const char *key EINA_UNUSED);

void *_evas_object_image_surface_get(Evas_Object *eo, Evas_Object_Protected_Data *obj);
Eina_Bool _evas_image_proxy_source_clip_get(const Eo *eo_obj);

extern Eina_Cow *evas_object_proxy_cow;
extern Eina_Cow *evas_object_map_cow;
extern Eina_Cow *evas_object_state_cow;

extern Eina_Cow *evas_object_3d_cow;

extern Eina_Cow *evas_object_image_pixels_cow;
extern Eina_Cow *evas_object_image_load_opts_cow;
extern Eina_Cow *evas_object_image_state_cow;

extern Eina_Cow *evas_object_filter_cow;
// This should be replaced by something like "eina_cow_default_get()" maybe
extern const void * const evas_object_filter_cow_default;
extern Eina_Cow *evas_object_mask_cow;

# define EINA_COW_STATE_WRITE_BEGIN(Obj, Write, State)          \
  EINA_COW_WRITE_BEGIN(evas_object_state_cow, Obj->State, \
                       Evas_Object_Protected_State, Write)

# define EINA_COW_STATE_WRITE_END(Obj, Write, State)                    \
    eina_cow_done(evas_object_state_cow, ((const Eina_Cow_Data**)&(Obj->State)), \
		  Write, EINA_FALSE);					\
   }									\
  while (0);

/****************************************************************************/
/*****************************************/
/********************/
#define MPOOL 1

#ifdef MPOOL
typedef struct _Evas_Mempool Evas_Mempool;

struct _Evas_Mempool
{
  int           count;
  int           num_allocs;
  int           num_frees;
  Eina_Mempool *mp;
};
# define EVAS_MEMPOOL(x) \
   static Evas_Mempool x = {0, 0, 0, NULL}
# define EVAS_MEMPOOL_INIT(x, nam, siz, cnt, ret) \
   do { \
     if (!x.mp) { \
       const char *tmp, *choice = "chained_mempool"; \
       tmp = getenv("EINA_MEMPOOL"); \
       if (tmp && tmp[0]) choice = tmp; \
       x.mp = eina_mempool_add(choice, nam, NULL, sizeof(siz), cnt); \
       if (!x.mp) { \
         return ret; \
       } \
     } \
   } while (0)
# define EVAS_MEMPOOL_ALLOC(x, siz) \
   eina_mempool_malloc(x.mp, sizeof(siz))
# define EVAS_MEMPOOL_PREP(x, p, siz) \
   do { \
     x.count++; \
     x.num_allocs++; \
     memset(p, 0, sizeof(siz)); \
   } while (0)
# define EVAS_MEMPOOL_FREE(x, p) \
   do { \
     eina_mempool_free(x.mp, p); \
     x.count--; \
     x.num_frees++; \
     if (x.count <= 0) { \
       eina_mempool_del(x.mp); \
       x.mp = NULL; \
       x.count = 0; \
     } \
   } while (0)
#else
# define EVAS_MEMPOOL(x)
# define EVAS_MEMPOOL_INIT(x, nam, siz, cnt, ret)
# define EVAS_MEMPOOL_PREP(x, p, siz)
# define EVAS_MEMPOOL_ALLOC(x, siz) \
   calloc(1, sizeof(siz))
# define EVAS_MEMPOOL_FREE(x, p) \
   free(p)
#endif
/********************/
/*****************************************/
/****************************************************************************/

#define EVAS_API_OVERRIDE(func, api, prefix) \
     (api)->func = prefix##func
#define EVAS_API_RESET(func, api) \
     (api)->func = NULL

static inline Efl_Gfx_Render_Op
_evas_to_gfx_render_op(Evas_Render_Op rop)
{
   if (rop == EVAS_RENDER_COPY)
     return EFL_GFX_RENDER_OP_COPY;
   return EFL_GFX_RENDER_OP_BLEND;
}

static inline Evas_Render_Op
_gfx_to_evas_render_op(Efl_Gfx_Render_Op rop)
{
   if (rop == EFL_GFX_RENDER_OP_COPY)
     return EVAS_RENDER_COPY;
   return EVAS_RENDER_BLEND;
}

#include "evas_inline.x"

#ifdef __cplusplus
}
#endif
#endif

