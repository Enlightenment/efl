#ifndef EVAS_PRIVATE_H
#define EVAS_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef EFL_CANVAS_OBJECT_PROTECTED
# define EFL_CANVAS_OBJECT_PROTECTED
#endif

#include "Evas.h"
#include "Eet.h"

#include "../file/evas_module.h"
#include "../file/evas_path.h"

#include "evas_3d_utils.h"


#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENC  _evas_engine_context(obj->layer->evas)

#include "canvas/evas_canvas.eo.h"
#include "canvas/evas_text.eo.h"
#include "canvas/evas_textgrid.eo.h"
#include "canvas/evas_line.eo.h"
#include "canvas/evas_box.eo.h"
#include "canvas/evas_table.eo.h"
#include "canvas/evas_grid.eo.h"

#define RENDER_METHOD_INVALID            0x00000000

typedef struct _Evas_Layer                  Evas_Layer;
typedef struct _Evas_Size                   Evas_Size;
typedef struct _Evas_Aspect                 Evas_Aspect;
typedef struct _Evas_Border                 Evas_Border;
typedef struct _Evas_Bool_Pair              Evas_Bool_Pair;
typedef struct _Evas_Double_Pair            Evas_Double_Pair;
typedef struct _Evas_Size_Hints             Evas_Size_Hints;
typedef struct _Evas_Data_Node              Evas_Data_Node;
typedef struct _Evas_Func                   Evas_Func;
typedef struct _Evas_Image_Save_Func        Evas_Image_Save_Func;
typedef struct _Evas_Vg_Load_Func           Evas_Vg_Load_Func;
typedef struct _Evas_Vg_Save_Func           Evas_Vg_Save_Func;
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
typedef struct _Evas_Object_Events_Data     Evas_Object_Events_Data;
typedef struct _Evas_Proxy_Render_Data      Evas_Proxy_Render_Data;
typedef struct _Evas_Object_3D_Data         Evas_Object_3D_Data;
typedef struct _Evas_Object_Mask_Data       Evas_Object_Mask_Data;
typedef struct _Evas_Object_Pointer_Data            Evas_Object_Pointer_Data;

typedef struct _Evas_Smart_Data             Evas_Smart_Data;
typedef struct _Efl_Object_Event_Grabber_Data  Efl_Object_Event_Grabber_Data;


typedef struct _Evas_Object_Protected_State Evas_Object_Protected_State;
typedef struct _Evas_Object_Protected_Data  Evas_Object_Protected_Data;

/* gfx filters typedef only */
typedef struct _Evas_Filter_Program         Evas_Filter_Program;
typedef struct _Evas_Filter_Context         Evas_Filter_Context;
typedef struct _Evas_Object_Filter_Data     Evas_Object_Filter_Data;
typedef struct _Evas_Filter_Data_Binding    Evas_Filter_Data_Binding;
typedef struct _Evas_Pointer_Data           Evas_Pointer_Data;
typedef struct _Evas_Filter_Command         Evas_Filter_Command;
typedef enum _Evas_Filter_Support           Evas_Filter_Support;

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

typedef struct _Vg_File_Data                   Vg_File_Data;

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

   Eina_File        *f;
   const char       *key;

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

/* General types - used for script type chceking */
#define OPAQUE_TYPE(type) struct __##type { int a; }; \
   typedef struct __##type type

OPAQUE_TYPE(Evas_Font_Set); /* General type for RGBA_Font */
OPAQUE_TYPE(Evas_Font_Instance); /* General type for RGBA_Font_Int */
/* End of general types */

#define MAGIC_EVAS                 0x70777770
#define MAGIC_OBJ                  0x71737723
#define MAGIC_SMART                0x7c6977c5
#define MAGIC_OBJ_SHAPE            0x747297f7
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

// helper function for legacy EAPI implementations
#define EVAS_OBJ_GET_OR_RETURN(o, ...) ({ \
   Evas_Object_Protected_Data *_obj = efl_isa(o, EFL_CANVAS_OBJECT_CLASS) ? \
     efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS) : NULL; \
   if (!_obj) { MAGIC_CHECK_FAILED(o,0,0) return __VA_ARGS__; } \
   _obj; })

#define NEW_RECT(_r, _x, _y, _w, _h) (_r) = eina_rectangle_new(_x, _y, _w, _h);

#define MERR_NONE() _evas_alloc_error = EVAS_ALLOC_ERROR_NONE
#define MERR_FATAL() _evas_alloc_error = EVAS_ALLOC_ERROR_FATAL
#define MERR_BAD() _evas_alloc_error = EVAS_ALLOC_ERROR_RECOVERED

/* DEBUG mode: fail, but normally just ERR(). This also returns if NULL. */
#ifdef DEBUG
#define EVAS_OBJECT_LEGACY_API(_eo, ...) \
   do { Evas_Object_Protected_Data *_o = efl_data_scope_get(_eo, EFL_CANVAS_OBJECT_CLASS); \
      if (EINA_UNLIKELY(!_o)) return __VA_ARGS__; \
      if (EINA_UNLIKELY(!_o->legacy)) { \
         char buf[1024]; snprintf(buf, sizeof(buf), "Calling legacy API on EO object '%s' is not permitted!", efl_class_name_get(_o->object)); \
         EINA_SAFETY_ERROR(buf); \
         return __VA_ARGS__; \
   } } while (0)
#else
#define EVAS_OBJECT_LEGACY_API(_eo, ...) \
   do { Evas_Object_Protected_Data *_o = efl_data_scope_get(_eo, EFL_CANVAS_OBJECT_CLASS); \
      if (EINA_UNLIKELY(!_o)) return __VA_ARGS__; \
      if (EINA_UNLIKELY(!_o->legacy.ctor)) { \
         char buf[1024]; snprintf(buf, sizeof(buf), "Calling legacy API on EO object '%s' is not permitted!", efl_class_name_get(_o->object)); \
         EINA_SAFETY_ERROR(buf); \
   } } while (0)
#endif

#define EVAS_LEGACY_API(_obj, _e, ...) \
   Evas_Public_Data *_e = efl_data_scope_safe_get((_obj), EVAS_CANVAS_CLASS); \
   if (!_e) return __VA_ARGS__

#define EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(cur, prev)                  \
  if (cur->key)                                                         \
    {                                                                   \
       eina_stringshare_del(cur->key);                                  \
       if (prev->key == cur->key)                                       \
         prev->key = NULL;                                              \
       cur->key = NULL;                                                 \
    }                                                                   \
  if (prev->key)                                                        \
    {                                                                   \
       eina_stringshare_del(prev->key);                                 \
       prev->key = NULL;                                                \
    }

struct _Evas_Coord_Touch_Point
{
   double x, y;
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
   Eina_Bool           is_active : 1;
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
   struct {
      Evas_Object_Intercept_Device_Focus_Set_Cb func;
      void *data;
      Eina_Bool intercepted;
   } device_focus_set;
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
   Eina_Hash *masks; /* we have a max of 64 modifiers per seat */
   Evas_Public_Data *e;
};

struct _Evas_Lock
{
   struct {
      int       count;
      char    **list;
   } lock;
   Eina_Hash *masks; /* we have a max of 64 locks per seat */
   Evas_Public_Data *e;
};

struct _Evas_Post_Callback
{
   Evas_Object_Event_Post_Cb  func;
   const void                *data;
   int                        event_id;
   Evas_Callback_Type         type;
   unsigned char              delete_me : 1;
};

// somehow this has bugs ... and i am not sure why...
//#define INLINE_ACTIVE_GEOM 1

typedef struct
{
#ifdef INLINE_ACTIVE_GEOM
   Eina_Rectangle        rect;
#endif
   Evas_Object_Protected_Data *obj;
} Evas_Active_Entry;

typedef struct Evas_Pointer_Seat
{
   EINA_INLIST;
   Evas_Device *seat;
   Eina_Inlist *pointers;
   struct {
      Eina_List *in;
   } object;
   Evas_Coord     x, y;
   Evas_Point     prev;
   int            mouse_grabbed;
   int            downs;
   int            nogrep;
   unsigned char  inside : 1;
} Evas_Pointer_Seat;

struct _Evas_Pointer_Data
{
   EINA_INLIST;
   Evas_Device    *pointer;
   DATA32         button;
   Evas_Pointer_Seat *seat;
};

typedef struct _Evas_Post_Render_Job
{
   EINA_INLIST;
   void (*func)(void *);
   void *data;
} Evas_Post_Render_Job;

struct _Evas_Public_Data
{
   EINA_INLIST;

   DATA32            magic;
   Evas              *evas;

   Eina_Inlist       *seats;

   struct  {
      Evas_Coord     x, y, w, h;
      unsigned char  changed : 1;
   } viewport;

   struct {
      int            w, h;
      DATA32         render_method;
      Eina_Bool      legacy : 1;
   } output;

   struct
     {
        Evas_Coord x, y, w, h;
        Eina_Bool changed : 1;
     } framespace;

   Eina_List        *damages;
   Eina_List        *obscures;

   Evas_Layer       *layers;

   Eina_Hash        *name_hash;

   // locking so we can implement async rendering threads
   Eina_Lock         lock_objects;

   int               output_validity;

   Evas_Event_Flags  default_event_flags;

   struct {
      Evas_Module *module;
      Evas_Func *func;
   } engine;

   struct {
      Eina_Spinlock lock;
   } render;

   Eina_Array     delete_objects;
   Eina_Inarray   active_objects;
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
   Eina_List     *finalize_objects;

   struct {
      Evas_Post_Render_Job *jobs;
      Eina_Spinlock lock;
   } post_render;

   Eina_Clist     calc_list;
   Eina_Clist     calc_done;
   Eina_List     *video_objects;

   Eina_List     *post_events; // free me on evas_free

   Eina_Inlist    *callbacks;
   Eina_Inlist    *deferred_callbacks;

   int            delete_grabs;
   int            walking_grabs;
   Eina_List     *grabs;

   Eina_List     *font_path;

   Eina_Inarray  *update_del_redirect_array;

   int            in_smart_calc;
   int            smart_calc_count;

   Eo            *gesture_manager;
   Eo            *pending_default_focus_obj;
   Eina_Hash     *focused_objects; //Key - seat; value - the focused object
   Eina_List     *focused_by; //Which seat has the canvas focus
   void          *attach_data;
   Evas_Modifier  modifiers;
   Evas_Lock      locks;
   unsigned int   last_timestamp;
   int            last_mouse_down_counter;
   int            last_mouse_up_counter;
   int            nochange;
   Evas_Font_Hinting_Flags hinting;
   Evas_Callback_Type current_event;
   int            running_post_events;

   Eina_List     *touch_points;
   Eina_List     *devices;
   Eina_Array    *cur_device;

   void          *backend;
   Ector_Surface *ector;
   Eina_List     *outputs;

   Evas_Device   *default_seat;
   Evas_Device   *default_mouse;
   Evas_Device   *default_keyboard;

   Eina_List     *rendering;

   unsigned char  changed : 1;
   unsigned char  delete_me : 1;
   unsigned char  invalidate : 1;
   unsigned char  cleanup : 1;
   Eina_Bool      is_frozen : 1;
   Eina_Bool      inside_post_render : 1;
   Eina_Bool      devices_modified : 1;
};

struct _Evas_Layer
{
   EINA_INLIST;

   short             layer;
   Evas_Object_Protected_Data      *objects;

   Evas_Public_Data *evas;

   void             *engine_data;
   Eina_List        *removes;
   int               usage;
   int               walking_objects;
   unsigned char     delete_me : 1;
};

struct _Evas_Size
{
   Evas_Coord w, h;
};

struct _Evas_Aspect
{
   Evas_Aspect_Control mode;
   Eina_Size2D size;
};

struct _Evas_Border
{
   Evas_Coord l, r, t, b;
};

struct _Evas_Bool_Pair
{
   Eina_Bool x, y;
};

struct _Evas_Double_Pair
{
   double x, y;
};

struct _Evas_Size_Hints
{
   Evas_Size request;
   Eina_Size2D min, user_min, max;
   Evas_Aspect aspect;
   Evas_Double_Pair align, weight;
   Evas_Border padding;
   Evas_Bool_Pair fill;
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
   Eina_Rectangle        normal_geometry; // bounding box of map geom actually
//   void                 *surface; // surface holding map if needed
//   int                   surface_w, surface_h; // current surface w & h alloc
   double                mx, my; // mouse x, y after conversion to map space
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
   // WARNING - you cannot change the below cur/prev layout, content or size
   // unless you also update evas_object_main.c _map_same() func
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

struct _Evas_Object_Events_Data
{
   /*
      The list below contain the seats (Efl.Input.Devices) which this
      object allows events to be reported (Mouse, Keybord and focus events).
    */
   Eina_List     *events_whitelist; /* Efl_Input_Device */

   Eina_List     *focused_by_seats; /* Efl_Input_Device */
   Eina_Inlist   *pointer_grabs; /* Evas_Object_Pointer_Data */

   Evas_Object   *parent; /* An Efl.Canvas.Object.Event.Grabber */
};

struct _Evas_Object_Protected_State
{
   Evas_Object_Protected_Data *clipper;

   Eina_Rectangle        geometry;
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

struct _Evas_Object_Pointer_Data {
   EINA_INLIST;

   Evas_Pointer_Data *evas_pdata;
   Evas_Object_Pointer_Mode pointer_mode;
   int mouse_grabbed;
   Eina_Bool mouse_in;
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
   const Evas_Object_Events_Data *events;

   // Pointer to the Evas_Object itself
   Evas_Object                *object;

   Evas_Object                *anim_player;

   Evas_Size_Hints            *size_hints;

   int                         last_mouse_down_counter;
   int                         last_mouse_up_counter;
   int                         last_event_id;
   Evas_Callback_Type          last_event_type;

   struct {
      int                      in_move, in_resize;
   } doing;

   unsigned int                ref;

   unsigned int                animator_ref;
   uint64_t                    callback_mask;

   unsigned char               no_change_render;
   unsigned char               delete_me;

   Eina_Bool                   render_pre : 1;
   Eina_Bool                   rect_del : 1;

   Eina_Bool                   is_active : 1;
   Eina_Bool                   changed : 1;
   Eina_Bool                   in_pending_objects : 1;
   Eina_Bool                   restack : 1;
   Eina_Bool                   is_smart : 1;
   Eina_Bool                   is_event_parent : 1;
   Eina_Bool                   pass_events : 1;
   Eina_Bool                   store : 1;

   Eina_Bool                   changed_move : 1;
   Eina_Bool                   changed_color : 1;
   Eina_Bool                   changed_map : 1;
   Eina_Bool                   changed_pchange : 1;
   Eina_Bool                   changed_src_visible : 1;
   Eina_Bool                   freeze_events : 1;
   Eina_Bool                   repeat_events : 1;
   Eina_Bool                   no_propagate : 1;

   Eina_Bool                   del_ref : 1;
   Eina_Bool                   need_surface_clear : 1;
   Eina_Bool                   pre_render_done : 1;
   Eina_Bool                   precise_is_inside : 1;
   Eina_Bool                   is_static_clip : 1;

   Eina_Bool                   in_layer : 1;
   Eina_Bool                   is_frame : 1;
   Eina_Bool                   is_frame_top : 1; // this is the frame edje
   Eina_Bool                   child_has_map : 1;
   Eina_Bool                   efl_del_called : 1;
   Eina_Bool                   no_render : 1; // since 1.15

   Eina_Bool                   snapshot_needs_redraw : 1;
   Eina_Bool                   snapshot_no_obscure : 1;
   Eina_Bool                   is_image_object : 1;
   Eina_Bool                   gfx_mapping_has : 1;
   Eina_Bool                   gfx_mapping_update : 1;

   struct {
      Eina_Bool                ctor : 1; // used legacy constructor
      Eina_Bool                visible_set : 1; // visibility manually set
      Eina_Bool                weight_set : 1; // weight manually set
      Eina_Bool                finalized : 1; // object fully constructed
   } legacy;

   struct  {
      Eina_Bool                pass_events : 1;
      Eina_Bool                pass_events_valid : 1;
      Eina_Bool                freeze_events : 1;
      Eina_Bool                freeze_events_valid : 1;
      Eina_Bool                src_invisible : 1;
      Eina_Bool                src_invisible_valid : 1;
   } parent_cache;

   Eina_Bool                   events_filter_enabled : 1;
   Eina_Bool                   is_pointer_inside_legacy : 1;
};

struct _Evas_Data_Node
{
   char *key;
   void *data;
};

struct _Efl_Canvas_Output
{
   Eo *canvas;

   Evas_Engine_Info *info;
   void *output;

   Eina_List *planes;
   Eina_List *updates;

   Eina_Rectangle geometry;

   int info_magic;

   unsigned short lock;

   Eina_Bool changed : 1;
};

struct _Evas_Object_Func
{
   void (*free) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   void (*render) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data,
                   void *engine, void *output, void *context, void *surface,
                   int x, int y, Eina_Bool do_async);
   void (*render_pre) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   void (*render_post) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);

   void *(*engine_data_get) (Evas_Object *obj);

   int  (*is_visible) (Evas_Object *obj);
   int  (*was_visible) (Evas_Object *obj);

   int  (*is_opaque) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   int  (*was_opaque) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);

   int  (*is_inside) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data,
                      Evas_Coord x, Evas_Coord y);
   int  (*was_inside) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data,
                       Evas_Coord x, Evas_Coord y);

   void (*coords_recalc) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);

   int (*has_opaque_rect) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   int (*get_opaque_rect) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data,
                           Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

   int (*can_map) (Evas_Object *obj);
// new - add to prepare list during render if object needs some pre-render
// preparation - may include rendering content to buffer or loading data
// from disk or uploading to texture etc.
   void (*render_prepare) (Evas_Object *obj, Evas_Object_Protected_Data *pd, Eina_Bool do_async);
   int (*is_on_plane) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
   int (*plane_changed) (Evas_Object *obj, Evas_Object_Protected_Data *pd, void *type_private_data);
};

struct _Evas_Func
{
   void *(*engine_new)                     (void);
   void (*engine_free)                     (void *engine);

   void (*output_info_setup)               (void *info);
   void *(*output_setup)                   (void *engine, void *info, unsigned int w, unsigned int h);
   int  (*output_update)                   (void *engine, void *data, void *info, unsigned int w, unsigned int h);
   void (*output_free)                     (void *engine, void *data);
   void (*output_resize)                   (void *engine, void *data, int w, int h);

   /* The redraws are automatically propagated on all output */
   void (*output_redraws_rect_add)         (void *engine, int x, int y, int w, int h);
   void (*output_redraws_rect_del)         (void *engine, int x, int y, int w, int h);

   void (*output_redraws_clear)            (void *engine, void *data);
   void *(*output_redraws_next_update_get) (void *engine, void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
   void (*output_redraws_next_update_push) (void *engine, void *data, void *surface, int x, int y, int w, int h, Evas_Render_Mode render_mode);
   void (*output_flush)                    (void *engine, void *data, Evas_Render_Mode render_mode);
   void (*output_idle_flush)               (void *engine, void *data);
   void (*output_dump)                     (void *engine, void *data);

   void *(*context_new)                    (void *engine);
   void *(*context_dup)                    (void *engine, void *context);
   Eina_Bool (*canvas_alpha_get)           (void *engine);
   void (*context_free)                    (void *engine, void *context);
   void (*context_clip_set)                (void *engine, void *context, int x, int y, int w, int h);
   void (*context_clip_image_set)          (void *engine, void *context, void *surface, int x, int y, Evas_Public_Data *evas, Eina_Bool do_async);
   void (*context_clip_image_unset)        (void *engine, void *context);
   void (*context_clip_image_get)          (void *engine, void *context, void **surface, int *x, int *y); /* incref surface if not NULL */
   void (*context_clip_clip)               (void *engine, void *context, int x, int y, int w, int h);
   void (*context_clip_unset)              (void *engine, void *context);
   int  (*context_clip_get)                (void *engine, void *context, int *x, int *y, int *w, int *h);
   void (*context_color_set)               (void *engine, void *context, int r, int g, int b, int a);
   int  (*context_color_get)               (void *engine, void *context, int *r, int *g, int *b, int *a);
   void (*context_multiplier_set)          (void *engine, void *context, int r, int g, int b, int a);
   void (*context_multiplier_unset)        (void *engine, void *context);
   int  (*context_multiplier_get)          (void *engine, void *context, int *r, int *g, int *b, int *a);
   void (*context_cutout_add)              (void *engine, void *context, int x, int y, int w, int h);
   void (*context_cutout_clear)            (void *engine, void *context);
   void (*context_cutout_target)           (void *engine, void *context, int x, int y, int w, int h);
   void (*context_anti_alias_set)          (void *engine, void *context, unsigned char aa);
   unsigned char (*context_anti_alias_get) (void *engine, void *context);
   void (*context_color_interpolation_set) (void *engine, void *context, int color_space);
   int  (*context_color_interpolation_get) (void *engine, void *context);
   void (*context_render_op_set)           (void *engine, void *context, int render_op);
   int  (*context_render_op_get)           (void *engine, void *context);

   void (*rectangle_draw)                  (void *engine, void *data, void *context, void *surface, int x, int y, int w, int h, Eina_Bool do_async);

   void (*line_draw)                       (void *engine, void *data, void *context, void *surface, int x1, int y1, int x2, int y2, Eina_Bool do_async);

   void *(*polygon_point_add)              (void *engine, void *polygon, int x, int y);
   void *(*polygon_points_clear)           (void *engine, void *polygon);
   void (*polygon_draw)                    (void *engine, void *data, void *context, void *surface, void *polygon, int x, int y, Eina_Bool do_async);

   void *(*image_mmap)                     (void *engine, Eina_File *f, const char *key, int *error, Evas_Image_Load_Opts *lo);
   void *(*image_new_from_data)            (void *engine, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
   void *(*image_new_from_copied_data)     (void *engine, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
   void (*image_free)                      (void *engine, void *image);
   void *(*image_ref)                      (void *engine, void *image);
   void (*image_size_get)                  (void *engine, void *image, int *w, int *h);
   void *(*image_size_set)                 (void *engine, void *image, int w, int h);
   void (*image_stride_get)                (void *engine, void *image, int *stride);
   void *(*image_dirty_region)             (void *engine, void *image, int x, int y, int w, int h);
   void *(*image_data_get)                 (void *engine, void *image, int to_write, DATA32 **image_data, int *err, Eina_Bool *tofree);
   void *(*image_data_put)                 (void *engine, void *image, DATA32 *image_data);
   Eina_Bool (*image_data_direct_get)      (void *engine, void *image, int plane, Eina_Slice *slice, Evas_Colorspace *cspace, Eina_Bool load);
   void  (*image_data_preload_request)     (void *engine, void *image, const Eo *target);
   void  (*image_data_preload_cancel)      (void *engine, void *image, const Eo *target, Eina_Bool force);
   void *(*image_alpha_set)                (void *engine, void *image, int has_alpha);
   int  (*image_alpha_get)                 (void *engine, void *image);
   void *(*image_orient_set)               (void *engine, void *image, Evas_Image_Orient orient);
   Evas_Image_Orient (*image_orient_get)   (void *engine, void *image);
   Eina_Bool (*image_draw)                 (void *engine, void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth, Eina_Bool do_async);
   void (*image_colorspace_set)            (void *engine, void *image, Evas_Colorspace cspace);
   Evas_Colorspace (*image_colorspace_get) (void *engine, void *image);
   Evas_Colorspace (*image_file_colorspace_get)(void *engine, void *image);
   Eina_Bool (*image_can_region_get)       (void *engine, void *image);

   /* image data map/unmap: direct or indirect access to pixels data */
   Eina_Bool (*image_data_map)             (void *engine, void **image, Eina_Rw_Slice *slice, int *stride, int x, int y, int w, int h, Evas_Colorspace cspace, Efl_Gfx_Buffer_Access_Mode mode, int plane);
   Eina_Bool (*image_data_unmap)           (void *engine, void *image, const Eina_Rw_Slice *slice);
   int (*image_data_maps_get)              (void *engine, const void *image, const Eina_Rw_Slice **slices);

   /* new api for direct data set (not put) */
   void *(*image_data_slice_add)           (void *engine, void *image, const Eina_Slice *slice, Eina_Bool copy, int w, int h, int stride, Evas_Colorspace space, int plane, Eina_Bool alpha);

   void (*image_prepare)                   (void *engine, void *image);

   void *(*image_surface_noscale_new)      (void *engine, int w, int h, int alpha);

   int (*image_native_init)                (void *engine, Evas_Native_Surface_Type type);
   void (*image_native_shutdown)           (void *engine, Evas_Native_Surface_Type type);
   void *(*image_native_set)               (void *engine, void *image, void *native);
   void *(*image_native_get)               (void *engine, void *image);

   void (*image_cache_flush)               (void *engine);
   void (*image_cache_set)                 (void *engine, int bytes);
   int  (*image_cache_get)                 (void *engine);

   void *(*image_plane_assign)             (void *data, void *image, int x, int y);
   void (*image_plane_release)             (void *data, void *image, void *plane);

   Evas_Font_Set *(*font_load)             (void *engine, const char *name, int size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable);
   Evas_Font_Set *(*font_memory_load)      (void *engine, const char *source, const char *name, int size, const void *fdata, int fdata_size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable);
   Evas_Font_Set *(*font_add)              (void *engine, Evas_Font_Set *font, const char *name, int size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable);
   Evas_Font_Set *(*font_memory_add)       (void *engine, Evas_Font_Set *font, const char *source, const char *name, int size, const void *fdata, int fdata_size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable);
   void (*font_free)                       (void *engine, Evas_Font_Set *font);
   int  (*font_ascent_get)                 (void *engine, Evas_Font_Set *font);
   int  (*font_descent_get)                (void *engine, Evas_Font_Set *font);
   int  (*font_max_ascent_get)             (void *engine, Evas_Font_Set *font);
   int  (*font_max_descent_get)            (void *engine, Evas_Font_Set *font);
   void (*font_string_size_get)            (void *engine, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int *w, int *h);
   int  (*font_inset_get)                  (void *engine, Evas_Font_Set *font, const Evas_Text_Props *text_props);
   int  (*font_h_advance_get)              (void *engine, Evas_Font_Set *font, const Evas_Text_Props *intl_props);
   int  (*font_v_advance_get)              (void *engine, Evas_Font_Set *font, const Evas_Text_Props *intl_props);
   int  (*font_char_coords_get)            (void *engine, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int pos, int *cx, int *cy, int *cw, int *ch);
   int  (*font_char_at_coords_get)         (void *engine, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int x, int y, int *cx, int *cy, int *cw, int *ch);
   Eina_Bool (*font_draw)                  (void *engine, void *data, void *context, void *surface, Evas_Font_Set *font, int x, int y, int w, int h, int ow, int oh, Evas_Text_Props *intl_props, Eina_Bool do_async);
   void (*font_cache_flush)                (void *engine);
   void (*font_cache_set)                  (void *engine, int bytes);
   int  (*font_cache_get)                  (void *engine);

   /* Engine functions will over time expand from here */

   void (*font_hinting_set)                (void *engine, Evas_Font_Set *font, int hinting);
   int  (*font_hinting_can_hint)           (void *engine, int hinting);

   void (*image_scale_hint_set)            (void *engine, void *image, int hint);
   int  (*image_scale_hint_get)            (void *engine, void *image);
   int  (*font_last_up_to_pos)             (void *engine, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int x, int y, int width_offset);

   Eina_Bool (*image_map_draw)             (void *engine, void *data, void *context, void *surface, void *image, RGBA_Map *m, int smooth, int level, Eina_Bool do_async);
   void *(*image_map_surface_new)          (void *engine, int w, int h, int alpha);
   void (*image_map_clean)                 (void *engine, RGBA_Map *m);
   void *(*image_scaled_update)            (void *engine, void *scaled, void *image, int dst_w, int dst_h, Eina_Bool smooth, Evas_Colorspace cspace);

   void (*image_content_hint_set)          (void *engine, void *surface, int hint);
   int  (*font_pen_coords_get)             (void *engine, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int pos, int *cpen_x, int *cy, int *cadv, int *ch);
   Eina_Bool (*font_text_props_info_create) (void *engine, Evas_Font_Instance *fi, const Eina_Unicode *text, Evas_Text_Props *intl_props, const Evas_BiDi_Paragraph_Props *par_props, size_t pos, size_t len, Evas_Text_Props_Mode mode, const char *lang);
   int  (*font_right_inset_get)            (void *engine, Evas_Font_Set *font, const Evas_Text_Props *text_props);

   /* EFL-GL Glue Layer */
   Eina_Bool (*gl_supports_evas_gl)      (void *engine);
   void *(*gl_output_set)                (void *engine, void *output);
   void *(*gl_surface_create)            (void *engine, void *config, int w, int h);
   void *(*gl_pbuffer_surface_create)    (void *engine, void *config, int w, int h, int const *attrib_list);
   int  (*gl_surface_destroy)            (void *engine, void *surface);
   void *(*gl_context_create)            (void *engine, void *share_context, int version, void *(*native_context_get)(void *ctx), void *(*engine_data_get)(void *evasgl));
   int  (*gl_context_destroy)            (void *engine, void *context);
   int  (*gl_make_current)               (void *engine, void *surface, void *context);
   const char *(*gl_string_query)        (void *engine, int name);
   void *(*gl_proc_address_get)          (void *engine, const char *name);
   int  (*gl_native_surface_get)         (void *engine, void *surface, void *native_surface);
   void *(*gl_api_get)                   (void *engine, int version);
   void (*gl_direct_override_get)        (void *engine, Eina_Bool *override, Eina_Bool *force_off);
   void (*gl_get_pixels_set)             (void *engine, void *get_pixels, void *get_pixels_data, void *obj);
   Eina_Bool (*gl_surface_lock)          (void *engine, void *surface);
   Eina_Bool (*gl_surface_read_pixels)   (void *engine, void *surface, int x, int y, int w, int h, Evas_Colorspace cspace, void *pixels);
   Eina_Bool (*gl_surface_unlock)        (void *engine, void *surface);
   int  (*gl_error_get)                  (void *engine);
   void *(*gl_current_context_get)       (void *engine);
   void *(*gl_current_surface_get)       (void *engine);
   int  (*gl_rotation_angle_get)         (void *engine);
   Eina_Bool (*gl_surface_query)         (void *engine, void *surface, int attr, void *value);
   Eina_Bool (*gl_surface_direct_renderable_get) (void *engine, void *output, Evas_Native_Surface *ns, Eina_Bool *override, void *surface);
   void (*gl_image_direct_set)           (void *engine, void *image, Eina_Bool direct);
   int  (*gl_image_direct_get)           (void *engine, void *image);
   void (*gl_get_pixels_pre)             (void *engine, void *output);
   void (*gl_get_pixels_post)            (void *engine, void *output);

   int  (*image_load_error_get)          (void *engine, void *image);
   int  (*font_run_end_get)              (void *engine, Evas_Font_Set *font, Evas_Font_Instance **script_fi, Evas_Font_Instance **cur_fi, Evas_Script_Type script, const Eina_Unicode *text, int run_len);

   /* animated feature */
   Eina_Bool (*image_animated_get)       (void *engine, void *image);
   int (*image_animated_frame_count_get) (void *engine, void *image);
   Evas_Image_Animated_Loop_Hint  (*image_animated_loop_type_get) (void *engine, void *image);
   int (*image_animated_loop_count_get)  (void *engine, void *image);
   double (*image_animated_frame_duration_get) (void *engine, void *image, int start_frame, int frame_num);
   Eina_Bool (*image_animated_frame_set) (void *engine, void *image, int frame_index);

   /* max size query */
   void (*image_max_size_get)            (void *engine, int *maxw, int *maxh);

   /* multiple font draws */
   Eina_Bool (*multi_font_draw)          (void *engine, void *data, void *context, void *surface, Evas_Font_Set *font, int x, int y, int w, int h, int ow, int oh, Evas_Font_Array *texts, Eina_Bool do_async);

   Eina_Bool (*pixel_alpha_get)          (void *image, int x, int y, DATA8 *alpha, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

   void (*context_flush)                 (void *engine);

   /* 3D features */
   void *(*drawable_new)                 (void *engine, int w, int h, int alpha);
   void  (*drawable_free)                (void *engine, void *drawable);
   void  (*drawable_size_get)            (void *engine, void *drawable, int *w, int *h);
   void *(*image_drawable_set)           (void *engine, void *image, void *drawable);
   void (*drawable_texture_rendered_pixels_get) (unsigned int tex, int x, int y, int w, int h, void *drawable EINA_UNUSED, void *data);
   void  (*drawable_scene_render)        (void *engine, void *data, void *drawable, void *scene_data);
   Eina_Bool (*drawable_scene_render_to_texture) (void *engine, void *drawable, void *scene_data);

   int (*drawable_texture_color_pick_id_get) (void *drawable);
   int (*drawable_texture_target_id_get) (void *drawable);
   void (*drawable_texture_pixel_color_get) (unsigned int tex EINA_UNUSED, int x, int y, Evas_Color *color, void *drawable);

   void *(*texture_new)                  (void *engine, Eina_Bool use_atlas);
   void  (*texture_free)                 (void *engine, void *texture);
   void  (*texture_size_get)             (void *engine, void *texture, int *w, int *h);
   void  (*texture_wrap_set)             (void *engine, void *texture, Evas_Canvas3D_Wrap_Mode s, Evas_Canvas3D_Wrap_Mode t);
   void  (*texture_wrap_get)             (void *engine, void *texture, Evas_Canvas3D_Wrap_Mode *s, Evas_Canvas3D_Wrap_Mode *t);
   void  (*texture_filter_set)           (void *engine, void *texture, Evas_Canvas3D_Texture_Filter min, Evas_Canvas3D_Texture_Filter mag);
   void  (*texture_filter_get)           (void *engine, void *texture, Evas_Canvas3D_Texture_Filter *min, Evas_Canvas3D_Texture_Filter *mag);
   void  (*texture_image_set)            (void *engine, void *texture, void *image);
   void *(*texture_image_get)            (void *engine, void *texture);

   Ector_Surface *(*ector_create)        (void *engine);
   void  (*ector_destroy)                (void *engine, Ector_Surface *surface);
   Ector_Buffer *(*ector_buffer_wrap)    (void *engine, Evas *e, void *engine_image);
   Ector_Buffer *(*ector_buffer_new)     (void *engine, Evas *e, int width, int height, Efl_Gfx_Colorspace cspace, Ector_Buffer_Flag flags);
   void  (*ector_begin)                  (void *engine, void *output, void *context, Ector_Surface *ector, int x, int y, Eina_Bool clear, Eina_Bool do_async);
   void  (*ector_renderer_draw)          (void *engine, void *output, void *context, Ector_Renderer *r, Eina_Array *clips, Eina_Bool do_async);
   void  (*ector_end)                    (void *engine, void *output, void *context, Ector_Surface *ector, Eina_Bool do_async);

   void *(*ector_surface_create)         (void *engine, int w, int h, int *error);
   void  (*ector_surface_destroy)        (void *engine, void *surface);
   void  (*ector_surface_cache_set)      (void *engine, void *key, void *surface);
   void *(*ector_surface_cache_get)      (void *engine, void *key);
   void  (*ector_surface_cache_drop)     (void *engine, void *key);

   Evas_Filter_Support (*gfx_filter_supports) (void *engine, Evas_Filter_Command *cmd);
   Eina_Bool (*gfx_filter_process)       (void *engine, Evas_Filter_Command *cmd);

   unsigned int info_size;
};

struct _Evas_Image_Save_Func
{
  int (*image_save) (RGBA_Image *im, const char *file, const char *key, int quality, int compress, const char *encoding);
};

struct _Vg_File_Data
{
   Efl_VG            *root;
   Evas_Vg_Load_Func *loader;
   Eina_Rectangle  view_box;
   int ref;

   Eina_Bool       static_viewbox: 1;
   Eina_Bool       preserve_aspect : 1;    //Used in SVG
};

struct _Evas_Vg_Load_Func
{
   Vg_File_Data *(*file_open) (const char *file, const char *key, int *error);
   Eina_Bool (*file_close) (Vg_File_Data *vfd);
   Eina_Bool (*file_data) (Vg_File_Data *vfd);
};

struct _Evas_Vg_Save_Func
{
   int (*file_save) (Vg_File_Data *vfd, const char *file, const char *key, int compress);
};

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *evas_object_new(Evas *e);
void evas_object_change_reset(Evas_Object_Protected_Data *obj);
void evas_object_clip_recalc_do(Evas_Object_Protected_Data *obj, Evas_Object_Protected_Data *clipper);
void evas_object_cur_prev(Evas_Object_Protected_Data *obj);
void evas_object_free(Evas_Object_Protected_Data *obj, Eina_Bool clean_layer);
void evas_object_update_bounding_box(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Smart_Data *s);
void evas_object_inject(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas *e);
void evas_object_release(Evas_Object *obj, Evas_Object_Protected_Data *pd, int clean_layer);
void evas_object_change(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_content_change(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_clip_changes_clean(Evas_Object_Protected_Data *obj);
void evas_object_render_pre_visible_change(Eina_Array *rects, Evas_Object *obj, int is_v, int was_v);
void evas_object_render_pre_clipper_change(Eina_Array *rects, Evas_Object *obj);
void evas_object_render_pre_prev_cur_add(Eina_Array *rects, Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_render_pre_effect_updates(Eina_Array *rects, Evas_Object *obj, int is_v, int was_v);
void evas_rects_return_difference_rects(Eina_Array *rects, int x, int y, int w, int h, int xx, int yy, int ww, int hh);
Evas_Object_Pointer_Data *_evas_object_pointer_data_get(Evas_Pointer_Data *evas_pdata, Evas_Object_Protected_Data *obj);

void evas_object_clip_dirty_do(Evas_Object_Protected_Data *pd);
void evas_object_recalc_clippees(Evas_Object_Protected_Data *pd);
Evas_Layer *evas_layer_new(Evas *e);
void _evas_layer_flush_removes(Evas_Layer *lay);
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
void evas_object_callback_init(Efl_Canvas_Object *eo_obj, Evas_Object_Protected_Data *obj);
void evas_object_callback_shutdown(Efl_Canvas_Object *eo_obj, Evas_Object_Protected_Data *obj);
void evas_object_event_callback_call(Evas_Object *obj, Evas_Object_Protected_Data *pd, Evas_Callback_Type type, void *event_info, int event_id, const Efl_Event_Description *efl_event_desc);
Eina_List *evas_event_objects_event_list(Evas *e, Evas_Object *stop, int x, int y);
void evas_debug_error(void);
void evas_debug_input_null(void);
void evas_debug_generic(const char *str);
const char *evas_debug_magic_string_get(DATA32 magic);
void evas_render_update_del(Evas_Public_Data *e, int x, int y, int w, int h);
void evas_render_object_render_cache_free(Evas_Object *eo_obj, void *data);

void evas_object_smart_use(Evas_Smart *s);
void evas_object_smart_unuse(Evas_Smart *s);
void evas_smart_cb_descriptions_fix(Evas_Smart_Cb_Description_Array *a) EINA_ARG_NONNULL(1);
Eina_Bool evas_smart_cb_descriptions_resize(Evas_Smart_Cb_Description_Array *a, unsigned int size) EINA_ARG_NONNULL(1);
const Evas_Smart_Cb_Description *evas_smart_cb_description_find(const Evas_Smart_Cb_Description_Array *a, const char *name) EINA_ARG_NONNULL(1, 2) EINA_PURE;

Evas_Object *_evas_object_image_source_get(Evas_Object *obj);
Eina_Bool _evas_object_image_preloading_get(const Evas_Object *obj);
Evas_Object *_evas_object_image_video_parent_get(Evas_Object *obj);
void _evas_object_image_video_overlay_show(Evas_Object *obj);
void _evas_object_image_video_overlay_hide(Evas_Object *obj);
void _evas_object_image_video_overlay_do(Evas_Object *obj);
Eina_Bool _evas_object_image_can_use_plane(Evas_Object_Protected_Data *obj, Efl_Canvas_Output *output);
void _evas_object_image_plane_release(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Efl_Canvas_Output *output);
void _evas_object_image_free(Evas_Object *obj);
void evas_object_smart_bounding_box_get(Evas_Object_Protected_Data *obj, Eina_Rectangle *cur_bounding_box, Eina_Rectangle *prev_bounding_box);
void evas_object_smart_del(Evas_Object *obj);
void evas_object_smart_cleanup(Evas_Object *obj);
void evas_object_smart_member_raise(Evas_Object *member);
void evas_object_smart_member_lower(Evas_Object *member);
void evas_object_smart_member_stack_above(Evas_Object *member, Evas_Object *other);
void evas_object_smart_member_stack_below(Evas_Object *member, Evas_Object *other);
void evas_object_smart_render_cache_clear(Evas_Object *eo_obj);
void *evas_object_smart_render_cache_get(const Evas_Object *eo_obj);
void evas_object_smart_render_cache_set(Evas_Object *eo_obj, void *data);

const Eina_List *evas_object_event_grabber_members_list(const Eo *eo_obj);

const Eina_Inlist *evas_object_smart_members_get_direct(const Evas_Object *obj);
void _efl_canvas_group_group_members_all_del(Evas_Object *eo_obj);
void _evas_object_smart_clipped_init(Evas_Object *eo_obj);
void evas_object_smart_clipped_smart_move(Evas_Object *eo_obj, Evas_Coord x, Evas_Coord y);
void _evas_object_smart_clipped_smart_move_internal(Evas_Object *eo_obj, Evas_Coord x, Evas_Coord y);
void evas_call_smarts_calculate(Evas *e);
void evas_object_smart_bounding_box_update(Evas_Object_Protected_Data *obj);
void evas_object_smart_need_bounding_box_update(Evas_Smart_Data *o, Evas_Object_Protected_Data *obj);
Eina_Bool evas_object_smart_changed_get(Evas_Object_Protected_Data *obj);
void evas_object_smart_attach(Evas_Object *eo_obj, Evas_Smart *s);
void _evas_post_event_callback_call_real(Evas *e, Evas_Public_Data* e_pd, int min_event_id);
#define _evas_post_event_callback_call(e, pd, id) do { \
   if (!pd->delete_me && pd->post_events) _evas_post_event_callback_call_real(e, pd, id); } while (0)
void _evas_post_event_callback_free(Evas *e);
void evas_event_callback_list_post_free(Eina_Inlist **list);
void evas_object_event_callback_all_del(Evas_Object *obj);
void evas_object_event_callback_cleanup(Evas_Object *obj);
void evas_event_callback_all_del(Evas *e);
void evas_event_callback_cleanup(Evas *e);
void evas_object_inform_call_show(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_inform_call_hide(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_inform_call_move(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_inform_call_resize(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_inform_call_restack(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_inform_call_changed_size_hints(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_object_inform_call_image_preloaded(Evas_Object *obj);
void evas_object_inform_call_image_unloaded(Evas_Object *obj);
void evas_object_inform_call_image_resize(Evas_Object *obj);
void evas_object_intercept_cleanup(Evas_Object *obj);
void evas_object_grabs_cleanup(Evas_Object *obj, Evas_Object_Protected_Data *pd);
void evas_key_grab_free(Evas_Object *obj, Evas_Object_Protected_Data *pd, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);
void evas_object_smart_member_cache_invalidate(Evas_Object *obj, Eina_Bool pass_events, Eina_Bool freeze_events, Eina_Bool source_invisible);
void evas_text_style_pad_get(Evas_Text_Style_Type style, int *l, int *r, int *t, int *b);
void _evas_object_text_rehint(Evas_Object *obj);
void _evas_object_textblock_rehint(Evas_Object *obj);

Eina_Bool _evas_object_intercept_call_evas(Evas_Object_Protected_Data *obj, Evas_Object_Intercept_Cb_Type cb_type, int internal, ...);

void evas_unref_queue_image_put(Evas_Public_Data *pd, void *image);
void evas_unref_queue_glyph_put(Evas_Public_Data *pd, void *glyph);
void evas_unref_queue_texts_put(Evas_Public_Data *pd, void *glyph);
void evas_post_render_job_add(Evas_Public_Data *pd, void (*func)(void *), void *data);

void evas_draw_image_map_async_check(Evas_Object_Protected_Data *obj,
                                     void *engine, void *data, void *context, void *surface,
                                     void *image, RGBA_Map *m, int smooth,
                                     int level, Eina_Bool do_async);
void evas_font_draw_async_check(Evas_Object_Protected_Data *obj,
                                void *engine, void *data, void *context, void *surface,
                                Evas_Font_Set *font,
                                int x, int y, int w, int h, int ow, int oh,
                                Evas_Text_Props *intl_props, Eina_Bool do_async);

void _efl_canvas_object_clip_prev_reset(Evas_Object_Protected_Data *obj, Eina_Bool cur_prev);

Eina_Bool _efl_canvas_object_clip_set_block(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object *eo_clip, Evas_Object_Protected_Data *clip);
Eina_Bool _efl_canvas_object_clip_unset_block(Eo *eo_obj, Evas_Object_Protected_Data *obj);
Eina_Bool _efl_canvas_object_efl_gfx_entity_size_set_block(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord w, Evas_Coord h, Eina_Bool internal);

void _evas_focus_device_invalidate_cb(void *data, const Efl_Event *ev);

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

/* Efl.Gfx.Mapping */
void _efl_gfx_mapping_init(void);
void _efl_gfx_mapping_shutdown(void);
void _efl_gfx_mapping_update(Eo *eo_obj);

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
Eina_Bool evas_filter_object_render(Eo *eo_obj, Evas_Object_Protected_Data *obj, void *engine, void *output, void *context, void *surface, int x, int y, Eina_Bool do_async, Eina_Bool alpha);

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
void _evas_canvas_event_pointer_in_rect_mouse_move_feed(Evas_Public_Data *edata,
                                                        Evas_Object *obj,
                                                        Evas_Object_Protected_Data *obj_data,
                                                        int w, int h,
                                                        Eina_Bool in_objects_list,
                                                        void *data);
void _evas_canvas_event_pointer_in_list_mouse_move_feed(Evas_Public_Data *edata,
                                                        Eina_List *was,
                                                        Evas_Object *obj,
                                                        Evas_Object_Protected_Data *obj_data,
                                                        int w, int h,
                                                        Eina_Bool xor_rule,
                                                        void *data);
void _evas_canvas_event_pointer_move_event_dispatch(Evas_Public_Data *edata,
                                                    Evas_Pointer_Data *pdata,
                                                    void *data);
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
Eina_Bool evas_preload_thread_cancelled_is(Evas_Preload_Pthread *thread);
Eina_Bool evas_preload_pthread_wait(Evas_Preload_Pthread *work, double wait);

void _evas_walk(Evas_Public_Data *e_pd);
void _evas_unwalk(Evas_Public_Data *e_pd);

EAPI void evas_module_task_register(Eina_Bool (*cancelled)(void *data), void *data);
EAPI void evas_module_task_unregister(void);

// expose for use in engines
EAPI int _evas_module_engine_inherit(Evas_Func *funcs, char *name, size_t info);
EAPI const char *_evas_module_libdir_get(void);
const char *_evas_module_datadir_get(void);
EAPI Eina_List *_evas_canvas_image_data_unset(Evas *eo_e);
EAPI void _evas_canvas_image_data_regenerate(Eina_List *list);

Eina_Bool evas_render_mapped(Evas_Public_Data *e, Evas_Object *obj,
                             Evas_Object_Protected_Data *source_pd,
                             void *context, void *output, void *surface,
                             int off_x, int off_y,
                             int mapped, int ecx, int ecy, int ecw, int ech,
                             Evas_Proxy_Render_Data *proxy_render_data,
                             int level, Eina_Bool do_async);
void evas_render_invalidate(Evas *e);
void evas_render_object_recalc(Evas_Object_Protected_Data *obj);
void evas_render_proxy_subrender(Evas *eo_e, void *output, Evas_Object *eo_source, Evas_Object *eo_proxy, Evas_Object_Protected_Data *proxy_obj, Eina_Bool source_clip, Eina_Bool do_async);

Eina_Bool evas_map_inside_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y);
Eina_Bool evas_map_coords_get(const Evas_Map *m, double x, double y, double *mx, double *my, int grab);
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
Efl_Input_Event *efl_input_event_instance_get(const Eo *klass, Eo *owner);
void efl_input_event_instance_clean(Eo *klass);

void *efl_input_pointer_legacy_info_fill(Evas *eo_evas, Efl_Input_Key *eo_ev, Evas_Callback_Type type, Evas_Event_Flags **pflags);
void *efl_input_key_legacy_info_fill(Efl_Input_Key *evt, Evas_Event_Flags **pflags);
void *efl_input_hold_legacy_info_fill(Efl_Input_Hold *evt, Evas_Event_Flags **pflags);

Eina_Bool evas_vg_loader_svg(Evas_Object *vg, const Eina_File *f, const char *key EINA_UNUSED);

void *_evas_object_image_surface_get(Evas_Object_Protected_Data *obj, Eina_Bool create);
void _evas_filter_radius_get(Evas_Object_Protected_Data *obj, int *l, int *r, int *t, int *b);
Eina_Bool _evas_filter_obscured_regions_set(Evas_Object_Protected_Data *obj, const Eina_Tiler *tiler);
Eina_Bool _evas_image_proxy_source_clip_get(const Eo *eo_obj);

void _evas_focus_dispatch_event(Evas_Object_Protected_Data *obj,
                                Efl_Input_Device *seat, Eina_Bool in);
Evas_Pointer_Data *_evas_pointer_data_by_device_get(Evas_Public_Data *edata, Efl_Input_Device *pointer);
Evas_Pointer_Data *_evas_pointer_data_add(Evas_Public_Data *edata, Efl_Input_Device *pointer);
void _evas_pointer_data_remove(Evas_Public_Data *edata, Efl_Input_Device *pointer);
Eina_List *_evas_pointer_list_in_rect_get(Evas_Public_Data *edata,
                                         Evas_Object *obj,
                                         Evas_Object_Protected_Data *obj_data,
                                         int w, int h);

void efl_canvas_output_info_get(Evas_Public_Data *e, Efl_Canvas_Output *output);

void evas_object_pixels_get_force(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);

// Gesture Manager
void *_efl_canvas_gesture_manager_private_data_get(Eo *obj);
void _efl_canvas_gesture_manager_filter_event(Eo *gesture_manager, Eo *target, void *event);
void _efl_canvas_gesture_manager_callback_del_hook(void *data, Eo *target, const Efl_Event_Description *type);
void _efl_canvas_gesture_manager_callback_add_hook(void *data, Eo *target, const Efl_Event_Description *type);
Eina_Bool _efl_canvas_gesture_manager_watches(const Efl_Event_Description *ev);

//evas focus functions
void evas_focus_init(void);
void evas_focus_shutdown(void);

void _deferred_callbacks_process(Evas *eo_e, Evas_Public_Data *e);

extern Eina_Cow *evas_object_proxy_cow;
extern Eina_Cow *evas_object_map_cow;
extern Eina_Cow *evas_object_state_cow;

extern Eina_Cow *evas_object_3d_cow;

extern Eina_Cow *evas_object_image_pixels_cow;
extern Eina_Cow *evas_object_image_load_opts_cow;
extern Eina_Cow *evas_object_image_state_cow;
extern Eina_Cow *evas_object_mask_cow;
extern Eina_Cow *evas_object_events_cow;

# define EINA_COW_STATE_WRITE_BEGIN(Obj, Write, State)          \
  EINA_COW_WRITE_BEGIN(evas_object_state_cow, Obj->State, \
                       Evas_Object_Protected_State, Write)

# define EINA_COW_STATE_WRITE_END(Obj, Write, State)                    \
    eina_cow_done(evas_object_state_cow, ((const Eina_Cow_Data**)&(Obj->State)), \
		  Write, EINA_FALSE);					\
   }									\
  while (0);

/* BEGIN: events to maintain compatibility with legacy */
EWAPI extern const Efl_Event_Description _EFL_GFX_ENTITY_EVENT_SHOW;
#define EFL_GFX_ENTITY_EVENT_SHOW (&(_EFL_GFX_ENTITY_EVENT_SHOW))
EWAPI extern const Efl_Event_Description _EFL_GFX_ENTITY_EVENT_HIDE;
#define EFL_GFX_ENTITY_EVENT_HIDE (&(_EFL_GFX_ENTITY_EVENT_HIDE))
/* END: events to maintain compatibility with legacy */

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

#undef EAPI
#define EAPI

#endif

