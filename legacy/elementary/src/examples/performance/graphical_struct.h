#ifndef GRAPHICAL_STRUCT_H
#define GRAPHICAL_STRUCT_H

typedef struct _vec4
{
    float   x;
    float   y;
    float   z;
    float   w;
} vec4;

typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

typedef struct _vec2
{
    float   x;
    float   y;
} vec2;
typedef struct _vertex
{
    vec3    position;
    vec3    normal;
    vec3    tangent;
    vec4    color;
    vec3    texcoord;
} vertex;

typedef struct _place
{
   vec3    position;
} place;

typedef struct _Flags_Change
{
   Eina_Bool fog_enable;
   Eina_Bool shadow_enable;
   Eina_Bool colorpick_enable;
   Eina_Bool blend_enable;
   Eina_Bool fps_enable;
} Changes;

typedef struct _Graphical
{
   Evas *evas;
   Evas_Object *img;
   Eo *scene;
   Eo *root_node;
   Eo *light_node;
   Eo *light;
   Eo *camera_node;
   Eo *camera;
   Eo *mesh_node;
   Eo *mesh;
   Eo *material;
   Eo *material_box;
   Eo *texture;
   Eo *mesh_box;
   Eo *mesh_nodebox;
   Eo *texture_box;

   Eina_List *list_nodes;
   Eina_List *list_meshes;

   const char *model_path;

   void (*init_scene)();
   void (*change_scene_setup)();

   vertex *vertices;
   place *places;
   unsigned short *indices;
   int index_count;
   int vertex_count;

   int count;
   int speed;
   int precision;
   int angle;

   Changes flags;

} Graphical;

extern Graphical globalGraphical;
#endif
