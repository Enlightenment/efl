#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT


#include <stdio.h>
#include <math.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include "Eo.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct _Scene_Data
{
   Eo      *texture_diffuse_eagle;
   Eo      *texture_diffuse_world;
   Eo      *texture_diffuse_grass;
   Eo      *root_node;
   Eo      *camera_node;
   Eo      *mesh_node_world;
   Eo      *mesh_node_grass[30];
   Eo      *light_node;
   Eo      *mediator_node;
   Eo      *mesh_node_cube;
   Eo      *mesh_node_eagle;
   Eo      *mesh_node_gun;
   Eo      *mesh_node_gun_butt;
   Eo      *mesh_node_gun_cage;
   Eo      *mesh_node_gun_bling[3];
   Eo      *mesh_node_rocket;
   Eo      *mesh_node_level[11];
   Eo      *mesh_node_snake;

   Eo      *camera;
   Eo      *light;
   Eo      *mesh_world;
   Eo      *mesh_grass[30];
   Eo      *mesh_cube;
   Eo      *mesh_eagle;
   Eo      *mesh_gun;
   Eo      *mesh_gun_cage;
   Eo      *mesh_gun_butt;
   Eo      *mesh_gun_bling[3];
   Eo      *mesh_rocket;
   Eo      *mesh_level[4];
   Eo      *mesh_snake;
   Eo      *material_world;
   Eo      *material_grass;
   Eo      *material_eagle;
   Eo      *material_cube;
   Eo      *material_level;
   Eo      *material_snake;
   Eo      *material_gun_butt;
   Eo      *material_gun_bling;
   Eo      *material_gun_cage;
   Eo      *gun;
   Eo      *rocket;
   Eo      *texture;
   Eo      *texture_snake;
} Scene_Data;

static Evas             *evas              = NULL;
static Eo               *background        = NULL;
static Eo               *image             = NULL;

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

typedef struct _Box3
{
   vec3    p0;
   vec3    p1;
} Box3;

typedef struct _vertex
{
   vec3    position;
   vec3    normal;
   vec3    tangent;
   vec4    color;
   vec3    texcoord;
} vertex;

void evas_vec3_set(vec3 *dst, Evas_Real x, Evas_Real y, Evas_Real z);

void evas_box3_empty_set(Box3 *box);

void evas_box3_set(Box3 *box, Evas_Real x0, Evas_Real y0, Evas_Real z0, Evas_Real x1, Evas_Real y1, Evas_Real z1);

void evas_box3_union(Box3 *out, const Box3 *a, const Box3 *b);

/* fill vector by indices which are on one rotation ring */
void _add_annulus(float * current_r1, int size, float r1, float r2, float z1, float z2);

/* fill vector by vertices which are arranged between two rotation rings */
void _add_annulus_vertices(unsigned short * current_r1, int size, int first);

void       _scale(Evas_3D_Node *node, Evas_Real scale);

void       _add_solid_of_revolution(float * start, int size, float * vertic, unsigned short * indic);

void       _camera_setup(Scene_Data *data);

void       _light_setup(Scene_Data *data);

Eina_Bool  _mesh_aabb(Evas_3D_Mesh **mesh, Scene_Data *scene);

void       _on_delete(Ecore_Evas *ee EINA_UNUSED);

void       _on_canvas_resize(Ecore_Evas *ee);

Eina_Bool _horizontal_circle_resolution(Evas_3D_Node *node, Evas_Real x, Evas_Real z, Evas_Real r);

Eina_Bool _horizontal_position_resolution(Evas_3D_Node *node, Evas_Real x, Evas_Real z);
