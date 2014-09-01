typedef struct _Evas_3D_Vec3_Eet
{
   float x;
   float y;
   float z;
} Evas_3D_Vec3_Eet;

typedef struct _Evas_3D_Vertex_Eet
{
   Evas_3D_Vec3_Eet position;
   Evas_3D_Vec3_Eet normal;
   Evas_3D_Vec3_Eet texcoord;
} Evas_3D_Vertex_Eet;

typedef struct _Evas_3D_Geometry_Eet
{
   unsigned int id;
   const char *name;
   int vertices_count;
   Evas_3D_Vertex_Eet *vertices;
} Evas_3D_Geometry_Eet;

typedef struct _Evas_3D_Color_Eet
{
   float r;
   float g;
   float b;
   float a;
} Evas_3D_Color_Eet;

typedef struct _Evas_3D_Material_Eet
{
   int id;
   float shininess;
   int colors_count;
   Evas_3D_Color_Eet *colors;//Color per attribute
} Evas_3D_Material_Eet;

typedef struct _Evas_3D_Frame_Eet
{
   int id;
   int geometry_id;
   int material_id;
} Evas_3D_Frame_Eet;//only ids to prevent of spending of memory

typedef struct _Evas_3D_Mesh_Eet
{
   int materials_count;
   int frames_count;
   int geometries_count;
   Evas_3D_Material_Eet *materials;
   Evas_3D_Frame_Eet *frames;
   Evas_3D_Geometry_Eet *geometries;
} Evas_3D_Mesh_Eet;

static Evas_3D_Mesh_Eet* eet_mesh;
static const char EVAS_3D_MESH_CACHE_FILE_ENTRY[] = "evas_3d mesh";
static Eet_Data_Descriptor *_vec3_descriptor;
static Eet_Data_Descriptor *_vertex_descriptor;
static Eet_Data_Descriptor *_geometry_descriptor;
static Eet_Data_Descriptor *_color_descriptor;
static Eet_Data_Descriptor *_material_descriptor;
static Eet_Data_Descriptor *_frame_descriptor;
static Eet_Data_Descriptor *_mesh_descriptor;

static inline struct Evas_3D_Geometry_Eet *
_mesh_new(void)
{
   const struct Evas_3D_Mesh_Eet *creating_mesh = calloc(1, sizeof(Evas_3D_Geometry_Eet));

   if (!creating_mesh)
     {
        ERR("Could not calloc Evas_3D_Geometry_Eet");
        return NULL;
     }

   return creating_mesh;
}

static inline void
_mesh_init(void)
{
   eina_init();
   eet_init();

   Eet_Data_Descriptor_Class eddc;
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Vec3_Eet);
   _vec3_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Vertex_Eet);
   _vertex_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Geometry_Eet);
   _geometry_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Color_Eet);
   _color_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Frame_Eet);
   _frame_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Material_Eet);
   _material_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Mesh_Eet);
   _mesh_descriptor = eet_data_descriptor_file_new(&eddc);

/* Vec_3 */
#define ADD_BASIC(member, eet_type) EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_vec3_descriptor, Evas_3D_Vec3_Eet, # member, member, eet_type);
   ADD_BASIC(x, EET_T_FLOAT);
   ADD_BASIC(y, EET_T_FLOAT);
   ADD_BASIC(z, EET_T_FLOAT);
#undef ADD_BASIC

/* Vertex */
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_vertex_descriptor, Evas_3D_Vertex_Eet, "position", position, _vec3_descriptor);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_vertex_descriptor, Evas_3D_Vertex_Eet, "normal", normal, _vec3_descriptor);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_vertex_descriptor, Evas_3D_Vertex_Eet, "texcoord", texcoord, _vec3_descriptor);

/* Geometry */
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY (_geometry_descriptor, Evas_3D_Geometry_Eet, "vertices", vertices, _vertex_descriptor);
#define ADD_BASIC(member, eet_type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_geometry_descriptor, Evas_3D_Geometry_Eet, # member, member, eet_type);
   ADD_BASIC(vertices_count, EET_T_INT);
   ADD_BASIC(id, EET_T_UINT);
   ADD_BASIC(name, EET_T_STRING);
#undef ADD_BASIC

/* Color */
#define ADD_BASIC(member, eet_type) EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_color_descriptor, Evas_3D_Color_Eet, # member, member, eet_type);
   ADD_BASIC(r, EET_T_FLOAT);
   ADD_BASIC(g, EET_T_FLOAT);
   ADD_BASIC(b, EET_T_FLOAT);
   ADD_BASIC(a, EET_T_FLOAT);
#undef ADD_BASIC

/* Material */
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY (_material_descriptor, Evas_3D_Material_Eet, "colors", colors, _color_descriptor);
#define ADD_BASIC(member, eet_type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_material_descriptor, Evas_3D_Material_Eet, # member, member, eet_type);
   ADD_BASIC(id, EET_T_UINT);
   ADD_BASIC(colors_count, EET_T_UINT);
   ADD_BASIC(shininess, EET_T_FLOAT);
#undef ADD_BASIC

/* Frame */
#define ADD_BASIC(member, eet_type) EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_frame_descriptor, Evas_3D_Frame_Eet, # member, member, eet_type);
   ADD_BASIC(id, EET_T_UINT);
   ADD_BASIC(geometry_id, EET_T_UINT);
   ADD_BASIC(material_id, EET_T_UINT);
#undef ADD_BASIC

/* Mesh */
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY (_mesh_descriptor, Evas_3D_Mesh_Eet, "frames", frames, _frame_descriptor);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY (_mesh_descriptor, Evas_3D_Mesh_Eet, "geometries", geometries, _geometry_descriptor);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY (_mesh_descriptor, Evas_3D_Mesh_Eet, "materials", materials, _material_descriptor);
#define ADD_BASIC(member, eet_type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_mesh_descriptor, Evas_3D_Mesh_Eet, # member, member, eet_type);
   ADD_BASIC(materials_count, EET_T_UINT);
   ADD_BASIC(frames_count, EET_T_UINT);
   ADD_BASIC(geometries_count, EET_T_UINT);
#undef ADD_BASIC
}

static inline void
_descriptor_shutdown(void)
{
   eet_data_descriptor_free (_geometry_descriptor);
   eet_data_descriptor_free (_vertex_descriptor);
   eet_data_descriptor_free (_vec3_descriptor);
   eet_data_descriptor_free (_color_descriptor);
   eet_data_descriptor_free (_material_descriptor);
   eet_data_descriptor_free (_frame_descriptor);
   eet_data_descriptor_free (_mesh_descriptor);
}

static inline void
_mesh_free(void)
{
   free(eet_mesh->geometries[0].vertices);
   free(eet_mesh->geometries);
   free(eet_mesh->frames);
   free(eet_mesh->materials[0].colors);
   free(eet_mesh->materials);
   free(eet_mesh);
   _descriptor_shutdown();
   eet_shutdown();
   eina_shutdown();
}

