#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <stdio.h>
#include <math.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Getopt.h>
#include <Evas.h>
#include "Eo.h"
#include "evas-3d-shooter-macros.h"
#include "../evas-common.h"

#define LOCAL_IMAGE_FOLDER PACKAGE_EXAMPLES_DIR "" EVAS_PROJECT_IMAGE_FOLDER
#define LOCAL_MODEL_FOLDER PACKAGE_EXAMPLES_DIR "" EVAS_PROJECT_MODEL_FOLDER

typedef struct _Scene_Data
{
   Eo      *cylinder_primitive;
   Eo      *texture_diffuse_eagle;
   Eo      *texture_diffuse_world;
   Eo      *texture_diffuse_grass;
   Eo      *texture_diffuse_soldier;
   Eo      *texture_diffuse_soldier_jump;
   Eo      *texture_diffuse_head;
   Eo      *texture_diffuse_wall;
   Eo      *texture_diffuse_gazebo;
   Eo      *texture_diffuse_carpet;
   Eo      *texture_diffuse_column;
   Eo      *texture_diffuse_column_c;
   Eo      *texture_diffuse_level[2];
   Eo      *texture_diffuse_warrior;
   Eo      *texture_diffuse_snake;
   Eo      *texture_normal_grass;
   Eo      *texture_normal_wall;
   Eo      *texture_normal_gazebo;
   Eo      *texture_normal_carpet;
   Eo      *texture_normal_column;
   Eo      *texture_normal_column_c;
   Eo      *texture_normal_level[2];
   Eo      *texture_normal_snake;
   Eo      *root_node;
   Eo      *camera_node;
   Eo      *mesh_node_world;
   Eo      *mesh_node_grass;
   Eo      *light_node;
   Eo      *mediator_node;
   Eo      *carp_mediator_node;
   Eo      *mesh_node_cube;
   Eo      *mesh_node_eagle;
   Eo      *mesh_node_soldier;
   Eo      *mesh_node_head;
   Eo      *mesh_node_tommy;
   Eo      *mesh_node_wall[4];
   Eo      *mesh_node_gazebo;
   Eo      *mesh_node_carpet;
   Eo      *mesh_node_column[4];
   Eo      *mesh_node_column_c;
   Eo      *mesh_node_rocket[10];
   Eo      *mesh_node_level[10];
   Eo      *mesh_node_snake;
   Eo      *mesh_node_warrior;
   Eo      *mesh_node_ball;

   Eo      *camera;
   Eo      *light;
   Eo      *mesh_world;
   Eo      *mesh_grass;
   Eo      *mesh_cube;
   Eo      *mesh_eagle;
   Eo      *mesh_soldier;
   Eo      *mesh_soldier_jump;
   Eo      *mesh_head;
   Eo      *mesh_tommy;
   Eo      *mesh_wall[4];
   Eo      *mesh_gazebo;
   Eo      *mesh_carpet;
   Eo      *mesh_column[4];
   Eo      *mesh_column_c;
   Eo      *mesh_rocket[10];
   Eo      *mesh_level[10];
   Eo      *mesh_snake;
   Eo      *mesh_warrior;
   Eo      *mesh_ball;
   Eo      *material_world;
   Eo      *material_grass;
   Eo      *material_eagle;
   Eo      *material_soldier;
   Eo      *material_soldier_jump;
   Eo      *material_head;
   Eo      *material_tommy;
   Eo      *material_wall;
   Eo      *material_gazebo;
   Eo      *material_carpet;
   Eo      *material_column;
   Eo      *material_column_c;
   Eo      *material_cube;
   Eo      *material_level[2];
   Eo      *material_snake;
   Eo      *material_warrior;
   Eo      *material_rocket[10];
   Eo      *material_ball;

   Eina_Bool   shadow;
   Eina_Bool   colorpick;
   Eina_Bool   fog;
   Eina_Bool   blending;
   Eina_Bool   normal;
} Scene_Data;

static Evas             *evas              = NULL;
static Eo               *background        = NULL;
static Eo               *image             = NULL;

static const vec2 tex_scale = {1, 1};
static const char *gray_brick_n_path = LOCAL_IMAGE_FOLDER "/brick-stone_n.jpg";
static const char *gray_brick_path = LOCAL_IMAGE_FOLDER "/brick-stone.jpg";
static const char *red_brick_n_path = LOCAL_IMAGE_FOLDER "/bricks_n.jpg";
static const char *red_brick_path = LOCAL_IMAGE_FOLDER "/bricks.jpg";
static const char *eagle_tex_path = LOCAL_IMAGE_FOLDER "/eagle.png";
static const char *snake_tex_path = LOCAL_IMAGE_FOLDER "/snake.png";
static const char *snake_tex_n_path = LOCAL_IMAGE_FOLDER "/snake_n.png";
static const char *soldier_tex_path = LOCAL_IMAGE_FOLDER "/soldier.png";
static const char *gazebo_b_path = LOCAL_IMAGE_FOLDER "/gazebo.png";
static const char *gazebo_t_path = LOCAL_IMAGE_FOLDER "/gazebo_t.png";
static const char *gazebo_t_trans_path = LOCAL_IMAGE_FOLDER "/gazebo_t_t.png";
static const char *gazebo_t_n_path = LOCAL_IMAGE_FOLDER "/gazebo_t_n.png";
static const char *gazebo_b_n_path = LOCAL_IMAGE_FOLDER "/gazebo_b_n.png";
static const char *warrior_tex_path = LOCAL_IMAGE_FOLDER "/warrior.png";

static const char *warrior_path = LOCAL_MODEL_FOLDER "/warrior.md2";
static const char *gazebo_bot_path = LOCAL_MODEL_FOLDER "/gazebo_b.md2";
static const char *gazebo_top_path = LOCAL_MODEL_FOLDER "/gazebo_t.md2";
static const char *eagle_path = LOCAL_MODEL_FOLDER "/eagle.md2";
static const char *snake_path = LOCAL_MODEL_FOLDER "/snake.md2";
static const char *soldier_jump_path = LOCAL_MODEL_FOLDER "/soldier_jump.md2";
static const char *soldier_path = LOCAL_MODEL_FOLDER "/soldier.md2";
static const char *column_path = LOCAL_MODEL_FOLDER "/column.ply";
static const char *gun_path = LOCAL_MODEL_FOLDER "/tommy.ply";

static const Ecore_Getopt optdesc = {
   "ecore_thread_example",
   NULL,
   "0.0",
   "(C) 2011 Enlightenment",
   "Public domain?",
   "Example program for Ecore_Thread",
   0,
   {
      ECORE_GETOPT_STORE_BOOL('s', "shadow", "If shadow is enable"),
      ECORE_GETOPT_STORE_BOOL('c', "colopick", "If colorpick is enable"),
      ECORE_GETOPT_STORE_BOOL('f', "fog", "If fog is enable"),
      ECORE_GETOPT_STORE_BOOL('b', "blending", "If blending is enable"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

typedef struct _vec4
{
   float   x;
   float   y;
   float   z;
   float   w;
} vec4;

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

void evas_box3_empty_set(Box3 *box);

void evas_box3_set(Box3 *box, Evas_Real x0, Evas_Real y0, Evas_Real z0, Evas_Real x1, Evas_Real y1, Evas_Real z1);

void       _scale(Evas_Canvas3D_Node *node, Evas_Real scale);

float      _distance(float x1, float z1, float x2, float z2);

void       _camera_setup(Scene_Data *data);

void       _light_setup(Scene_Data *data);

Eina_Bool  _mesh_aabb(Evas_Canvas3D_Mesh **mesh, Scene_Data *scene, int index);

void       _on_delete(Ecore_Evas *ee EINA_UNUSED);

void       _on_canvas_resize(Ecore_Evas *ee);

Eina_Bool _horizontal_circle_resolution(Evas_Canvas3D_Node *node, Evas_Real x, Evas_Real z, Evas_Real r);

Eina_Bool _horizontal_circle_position_resolution(Evas_Canvas3D_Node *node, Evas_Real x, Evas_Real z, Evas_Real r);

Eina_Bool _horizontal_circle_object_resolution(Evas_Canvas3D_Node *node, Evas_Canvas3D_Node *node2, Evas_Real r);

Eina_Bool _horizontal_position_resolution(Evas_Canvas3D_Node *node, Evas_Real x, Evas_Real z);

Eina_Bool _horizontal_object_resolution(Evas_Canvas3D_Node *node, Evas_Real x1, Evas_Real z1, Evas_Real x2,
                                        Evas_Real z2, Evas_Real x_move, Evas_Real z_move);

Eina_Bool _horizontal_frame_resolution(Evas_Canvas3D_Node *node, Evas_Real x1, Evas_Real z1, Evas_Real x2,
                                        Evas_Real z2, Evas_Real *x_move, Evas_Real *z_move);

void _set_ball(Eo *mesh, int p);
