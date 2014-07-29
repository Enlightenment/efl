/*
* This is example of 3d-shooter done with help of evas-3d
*
* Basic control: 'w' - to  go forward, 's' - to go back, 'a' - to the left, 'd' - to the right,
* right mouse down - to activate camera rotation, mouse wheel down - to deactivate camera rotation,
* left mouse down - to fire, 'space' - to jump. The warrior is atackable, the snake isn't,
* this possibility is given by presence of the bounding box, which fixes the rocket entry.
* The warrior isn't passable for camera, neither is wall. There is a possibility to go upstairs
* and break down.
*
* Compile with "gcc -g evas-3d-shooter.c evas-3d-shooter-header.c -o evas-3d-shooter `pkg-config --libs --cflags evas ecore ecore-evas eo` -lm"
*/

#include "evas-3d-shooter-header.h"
#define  WIDTH            1920
#define  HEIGHT           1080
#define  SOLDIER_RADIUS_1 5.2                 /* camera couldn't get сloser to soldier than SOLDIER_RADIUS_1 */
#define  SOLDIER_RADIUS_2 5.3                 /* soldier couldn't get сloser to camera than SOLDIER_RADIUS_2 */
#define  WALL_RADIUS      2.2                 /* camera couldn't get сloser to wall than WALL_RADIUS */
#define  JUMP_HIGHT       4
#define  GUN_DIR_X        2
#define  GUN_DIR_Y        -2
#define  GUN_DIR_Z        105
#define  GUN_SPEED        0.001

Evas_3D_Scene    *scene             = NULL;
Ecore_Evas       *ecore_evas        = NULL;
static float     angle_camera       = 0.0f;
static float     angle_eagle        = 180.0f;
static float     d_angle_camera     = 0.0;
float            d_angle_eagle      = 0.3;
float            world_position     = 0.0;
float            camera_position    = 25.0;
float            camera_move        = 0.0;
float            space_move         = 0.0;
float            old_space_position = 0.0;
float            fall_speed         = 0.0;
int              d_i                = 1;
int              rocket_indicator   = 0;
int              gun_indicator      = 0;
int              mouse_diff_x       = 0;
int              mouse_diff_y       = 0;
int              mouse_old_x        = 0;
int              mouse_old_y        = 0;
int              camera_indicator   = 0;
int              lateral_motion_indicator   = 0;
int              step_indicator     = 0;
int              fall_indicator     = 0;

typedef struct _Ecore_Timers
{
   Ecore_Timer *t;
   void *data;
} Timers;

static float cage_vertices[] =
{
   /* Front */
   -1.0,  0.2,  0.4,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  0.2,  0.4,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -0.2,  0.4,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -0.2,  0.4,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,

   /* Back */
    1.0,  0.2, -0.4,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,
   -1.0,  0.2, -0.4,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -0.2, -0.4,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -0.2, -0.4,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Left */
   -1.0,  0.2, -0.4,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,
   -1.0,  0.2,  0.4,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -0.2, -0.4,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,
   -1.0, -0.2,  0.4,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Right */
    1.0,  0.2,  0.4,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  0.2, -0.4,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,
    1.0, -0.2,  0.4,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -0.2, -0.4,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Top */
   -1.0,  0.2, -0.4,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
    1.0,  0.2, -0.4,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
   -1.0,  0.2,  0.4,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    1.0,  0.2,  0.4,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Bottom */
    1.0, -0.2, -0.4,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,
   -1.0, -0.2, -0.4,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -0.2,  0.4,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -0.2,  0.4,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,
};

static float grass_vertices[] =
{
   /* Top */
   -6.5,  0.0, -4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
    6.5,  0.0, -4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
   -6.5,  0.0,  4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    6.5,  0.0,  4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

};

static float wall_vertices[] =
{
   /* Front */
   -1.0,  1.0,  4.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0,  4.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0,  4.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0,  4.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,


   /* Back */
    1.0,  1.0, -4.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,
   -1.0,  1.0, -4.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0, -4.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0, -4.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Left */
   -1.0,  1.0, -4.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,
   -1.0,  1.0,  4.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0, -4.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  4.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Right */
    1.0,  1.0,  4.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -4.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  4.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0, -4.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Top */
   -1.0,  1.0, -4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
   -1.0,  1.0,  4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    1.0,  1.0,  4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Bottom */
    1.0, -1.0, -4.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,
   -1.0, -1.0, -4.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  4.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  4.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,
};

static float bricks_vertices[] =
{
   /* Front */
   -10.0,  10.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,
    10.0,  10.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,
   -10.0, -10.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,
    10.0, -10.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,


   /* Back */
    10.0,  10.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,
   -10.0,  10.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,
    10.0, -10.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,
   -10.0, -10.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Left */
   -10.0,  10.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,
   -10.0,  10.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,
   -10.0, -10.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,
   -10.0, -10.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Right */
    10.0,  10.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,
    10.0,  10.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,
    10.0, -10.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,
    10.0, -10.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Top */
   -10.0,  10.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
    10.0,  10.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
   -10.0,  10.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    10.0,  10.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Bottom */
    10.0, -10.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,
   -10.0, -10.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,
    10.0, -10.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,
   -10.0, -10.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,
};


static const unsigned short cube_indices[] =
{
   /* Front */
   0,   1,  2,  2,  1,  3,

   /* Back */
   4,   5,  6,  6,  5,  7,

   /* Left */
   8,   9, 10, 10,  9, 11,

   /* Right */
   12, 13, 14, 14, 13, 15,

   /* Top */
   16, 17, 18, 18, 17, 19,

   /* Bottom */
   20, 21, 22, 22, 21, 23
};

static const unsigned short grass_indices[] =
{
   /* Top */
   0,   1,  2,  2,  1,  3
};

static float wave_vertices_butt[20000];

static unsigned short wave_indices_butt[20000];

static float wave_vertices[20000];

static unsigned short wave_indices[20000];

static float wave_vertices2[20000];

static unsigned short wave_indices2[20000];

static int              vertex_count = 0;
static vertex           *vertices = NULL;

static int              index_count = 0;
static unsigned short   *indices = NULL;

static void
_key_down(void *data,
          Evas *e EINA_UNUSED,
          Evas_Object *eo EINA_UNUSED,
          void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Real x, y, z;
   Scene_Data *scene = (Scene_Data *)data;
   if (!strcmp(ev->key, "space"))
     {
        if (space_move == 0.0)
          {
             space_move = 0.25;
             eo_do(scene->mediator_node,
                   evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
             old_space_position = y;
          }
     }
   if (!strcmp(ev->key, "s"))
     {
        lateral_motion_indicator = 0;
        camera_move = 0.3;
     }
   else if (!strcmp(ev->key, "w"))
     {
        lateral_motion_indicator = 0;
        camera_move = -0.3;
     }
   else if (!strcmp(ev->key, "d"))
     {
        lateral_motion_indicator = 1;
        camera_move = 0.3;
     }
   else if (!strcmp(ev->key, "a"))
     {
        lateral_motion_indicator = 1;
        camera_move = -0.3;
     }
}

static void
_key_up(void *data,
        Evas *e EINA_UNUSED,
        Evas_Object *eo EINA_UNUSED,
        void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Real x, y, z;
   Scene_Data *scene = (Scene_Data *)data;

   if (!strcmp(ev->key, "space"))
     {
        if (space_move == 0.0)
          {
             space_move = 0.25;
             eo_do(scene->mediator_node,
                   evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
             old_space_position = y;
          }
     }

   if (!strcmp(ev->key, "s"))
     camera_move = 0;
   else if (!strcmp(ev->key, "w"))
     camera_move = 0;
   else if (!strcmp(ev->key, "d"))
     camera_move = 0;
   else if (!strcmp(ev->key, "a"))
     camera_move = 0;
}

/* animate camera, create artificial intelligence  of warrior */
static Eina_Bool
_animate_camera(void *data)
{
   Scene_Data *scene = (Scene_Data *)data;
   Evas_Real x, y, z;
   Evas_Real x1, y1, z1;
   Evas_Real x2, y2, z2;
   int i = 0;
   float x_move, x_move_lateral, z_move, z_move_lateral;

   angle_camera += d_angle_camera;
   x_move = camera_move * sin(angle_camera * M_PI / 180);
   z_move = camera_move * cos(angle_camera * M_PI / 180);
   x_move_lateral = camera_move * sin((angle_camera + 90) * M_PI / 180);
   z_move_lateral = camera_move * cos((angle_camera + 90) * M_PI / 180);

   eo_do(scene->mediator_node,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

   eo_do(scene->mesh_node_world,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x1, &y1, &z1));

   step_indicator = 0;
   for ( i = 1; i < 10; i++)
     {
        if (!_horizontal_position_resolution(scene->mesh_node_level[i],x, z))
          step_indicator = i;
     }

   if (space_move != 0.0 && y + space_move > old_space_position + JUMP_HIGHT)
     space_move = -0.25;
   else if (space_move != 0.0 && y + space_move < old_space_position)
     {
        space_move = 0.0;
     }

   if (fall_speed != 0)
     {
        if (y - fall_speed < 0)
          {
             eo_do(scene->mediator_node,
                   evas_3d_node_position_set(x, 0, z));
             fall_speed = 0.0;
          }
        else
          {
             eo_do(scene->mediator_node,
                   evas_3d_node_position_set(x, y - fall_speed, z));
             fall_speed += 0.04;
          }
     }
   else if ((lateral_motion_indicator == 0) &&
            (_horizontal_circle_resolution(scene->mesh_node_level[10], x + x_move, z + z_move, SOLDIER_RADIUS_1)) &&
            (_horizontal_circle_resolution(scene->mesh_node_level[0], x + x_move, z + z_move, WALL_RADIUS)))
     {
        eo_do(scene->mediator_node,
              evas_3d_node_position_set(x + x_move, y + space_move, z + z_move));
        eo_do(scene->mesh_node_world,
              evas_3d_node_position_set(x1 + x_move, y1, z1 + z_move));
     }
   else if ((_horizontal_circle_resolution(scene->mesh_node_level[10], x + x_move_lateral, z + z_move_lateral, SOLDIER_RADIUS_1))&&
            (_horizontal_circle_resolution(scene->mesh_node_level[0], x + x_move_lateral, z + z_move_lateral, WALL_RADIUS)))
     {
        eo_do(scene->mediator_node,
              evas_3d_node_position_set(x + x_move_lateral, y + space_move, z + z_move_lateral));
        eo_do(scene->mesh_node_world,
              evas_3d_node_position_set(x1 + x_move_lateral, y1, z1 + z_move_lateral));
     }

   eo_do(scene->mediator_node,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

   for ( i = 1; i < 10; i++)
     {
        if (!_horizontal_position_resolution(scene->mesh_node_level[i], x, z) && step_indicator == i - 1)
          {
             eo_do(scene->mediator_node,
                   evas_3d_node_position_set(x, y + 2, z));
          }
        else if (_horizontal_position_resolution(scene->mesh_node_level[i], x, z) && (step_indicator == i) && y != 0)
          {
             if (!_horizontal_position_resolution(scene->mesh_node_level[i - 1], x, z))
               {
                  eo_do(scene->mediator_node,
                        evas_3d_node_position_set(x, y - 2, z));
               }
             else if (_horizontal_position_resolution(scene->mesh_node_level[i + 1], x, z))
               fall_speed = 0.02;
          }
     }

   eo_do(scene->mediator_node,
         evas_3d_node_orientation_angle_axis_set(angle_camera, 0, 1.0, 0.0),
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

   eo_do(scene->mesh_node_level[10],
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x1, &y1, &z1));
   eo_do(scene->mesh_node_snake,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x2, &y2, &z2));

   if (_horizontal_circle_resolution(scene->mesh_node_level[10], (x1 - x) * 0.001 + x, (z1 - z) * 0.001 + z, SOLDIER_RADIUS_2))
     {
        eo_do(scene->mesh_node_level[10],
              evas_3d_node_position_set((x - x1) * 0.001 + x1, y1, (z - z1) * 0.001 + z1));
     }

   if (_horizontal_circle_resolution(scene->mesh_node_snake, (x2 - x) * 0.001 + x, (z2 - z) * 0.001 + z, SOLDIER_RADIUS_2))
     {
        eo_do(scene->mesh_node_snake,
              evas_3d_node_position_set((x - x2) * 0.001 + x2, y2, (z - z2) * 0.001 + z2));
     }

   eo_do(scene->mesh_node_level[10],
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, -1000.0, 0.0, EVAS_3D_SPACE_PARENT,
                                  (z - z1) / sqrt(pow(z - z1, 2) + pow(x1 - x, 2)), 0, (x1 - x) / sqrt(pow(z - z1, 2) + pow(x1 - x, 2))));
   eo_do(scene->mesh_node_snake,
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, -1000.0, 0.0, EVAS_3D_SPACE_PARENT,
                                  (z - z2)/sqrt(pow(z - z2, 2) + pow(x2 - x, 2)), 0, (x2 - x) / sqrt(pow(z - z2, 2) + pow(x2 - x, 2))));

   return EINA_TRUE;
}

static Eina_Bool
_animate_scene_gun(void *data)
{
   static float angle = 0.0f;
   static float gun_angle = 0.0f;
   static float rocket_parametr = 0.0f;
   static float gun_parametr = 0.0f;
   int i = 0;
   Evas_Real x, y, z;
   Evas_Real x1, y1, z1;
   static int frame = 0;
   Scene_Data *scene = (Scene_Data *)data;

   float sinus = sin(angle_camera * M_PI / 180);
   float cosinus = cos(angle_camera * M_PI / 180);

   /* rocket and gun movement */
   angle_eagle += d_angle_eagle;

   gun_angle += 3 * d_angle_eagle;

   eo_do(scene->mesh_node_eagle,
         evas_3d_node_orientation_angle_axis_set(angle_eagle - 180, 0.0, 1.0, 0.0),
         /* equation of eagle flight */
         evas_3d_node_position_set(20 * sin((angle_eagle + 90) * M_PI / 180), 20 + 10 * sin(angle_eagle * M_PI / 180),
                                   20.0 * cos((angle_eagle + 90) * M_PI / 180)));

   for ( i = 0; i < 3; i++)
      {
         eo_do(scene->mesh_node_gun_bling[i],
               evas_3d_node_orientation_angle_axis_set(1, 1, 1, 0));
      }
   eo_do(scene->mesh_node_gun,
         evas_3d_node_orientation_angle_axis_set(1, 1, 1, 0));
   eo_do(scene->mesh_node_gun_butt,
         evas_3d_node_orientation_angle_axis_set(1, 1, 1, 0));
   eo_do(scene->mesh_node_gun_cage,
         evas_3d_node_orientation_angle_axis_set(1, 1, 1, 0));

   if (rocket_indicator == 3)
     {
        rocket_indicator = 0;
     }
   else if (rocket_indicator == 0)
     {
        /* almost hidden rocket */
        _scale(scene->mesh_node_rocket, 0.004);
     }
   else if (rocket_indicator == 1)
     {
        _scale(scene->mesh_node_rocket, 0.4);
        eo_do(scene->rocket,
              evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.8, 0.8, 0.8, 1.0),
              evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.2, 0.2, 0.0, 1.0),
              evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 0.5, 0.5, 0.5, 1.0));

        rocket_indicator = 2;
        eo_do(scene->mediator_node,
              evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

        eo_do(scene->mesh_node_rocket,
              evas_3d_node_position_set(2, -2, 0));
        rocket_parametr = 0;
     }
   else if (rocket_indicator == 2)
     {
        rocket_parametr += 0.02;
        /* equation of rocket flight */
        eo_do(scene->mesh_node_rocket,
              evas_3d_node_position_set(2, -2, 0.0 - 105 * rocket_parametr));
     }

   if (gun_indicator == 1)
     {
        gun_indicator = 2;
        eo_do(scene->mesh_node_gun,
              evas_3d_node_position_set(2, -2, -4.0));
        eo_do(scene->mesh_node_gun_bling[0],
              evas_3d_node_position_set(1.9, -1.3, -3.0));
        eo_do(scene->mesh_node_gun_bling[1],
              evas_3d_node_position_set(1.9, -1.4, -2.5));
        eo_do(scene->mesh_node_gun_bling[2],
              evas_3d_node_position_set(1.9, -1.4, -3.5));
        gun_parametr = 0.0;
     }
   /* parametric equations of gun movement, (GUN_DIR_X, GUN_DIR_Y, GUN_DIR_Z) is direction vector */
   else if (gun_indicator == 2)
     {
        gun_parametr += GUN_SPEED;
        eo_do(scene->mesh_node_gun,
              evas_3d_node_position_set(2 + GUN_DIR_X * gun_parametr,
                                        -2 + GUN_DIR_Y * gun_parametr, -4.0 + GUN_DIR_Z * gun_parametr));
        eo_do(scene->mesh_node_gun_butt,
              evas_3d_node_position_set(2 + GUN_DIR_X * gun_parametr / 3,
                                        -2 + GUN_DIR_Y * gun_parametr / 3, -3.0 + GUN_DIR_Z * gun_parametr / 3));
        eo_do(scene->mesh_node_gun_cage,
              evas_3d_node_position_set(1 + GUN_DIR_X * gun_parametr / 3,
                                        -2 + GUN_DIR_Y * gun_parametr / 3, -3.0 + GUN_DIR_Z * gun_parametr / 3));
        for (i = 0; i < 3; i++)
          {
             eo_do(scene->mesh_node_gun_bling[i],
                   evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z),
                   evas_3d_node_position_set(x + GUN_DIR_X * GUN_SPEED / 3, y + GUN_DIR_Y * GUN_SPEED / 3, z + GUN_DIR_Z * GUN_SPEED / 3));
          }
        if (-4.0 + 105 * gun_parametr >= -2.0)
          {
             gun_indicator = 3;
          }
     }
   else if (gun_indicator == 3)
     {
        gun_parametr-= GUN_SPEED / 2;
        eo_do(scene->mesh_node_gun,
              evas_3d_node_position_set(2 + GUN_DIR_X * gun_parametr,
                                        -2 + GUN_DIR_Y * gun_parametr, -4.0 + GUN_DIR_Z * gun_parametr));
        eo_do(scene->mesh_node_gun_butt,
              evas_3d_node_position_set(2 + GUN_DIR_X * gun_parametr / 3,
                                        -2 + GUN_DIR_Y * gun_parametr / 3, -3.0 + GUN_DIR_Z * gun_parametr / 3));
        eo_do(scene->mesh_node_gun_cage,
              evas_3d_node_position_set(1 + GUN_DIR_X * gun_parametr / 3,
                                        -2 + GUN_DIR_Y * gun_parametr / 3, -3.0 + GUN_DIR_Z * gun_parametr / 3));
        for (i = 0; i < 3; i++)
          {
             eo_do(scene->mesh_node_gun_bling[i],
                   evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z),
                   evas_3d_node_position_set(x - GUN_DIR_X * 0.5 * GUN_SPEED / 3,
                                             y - GUN_DIR_Y * 0.5 * GUN_SPEED / 3, z - GUN_DIR_Z * 0.5 * GUN_SPEED / 3));
          }
        eo_do(scene->mesh_node_gun,
              evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

        if (z <= -4.0)
          {
             gun_indicator = 4;
             gun_parametr = 0;
          }
     }

   if (angle_eagle > 360.0) angle -= 360.0f;

   eo_do(scene->mesh_node_level[10],
         evas_3d_node_orientation_angle_axis_set(90, 1.0, 0.0, 0.0),
         evas_3d_node_mesh_frame_set(scene->mesh_level[2], frame));

   eo_do(scene->mesh_node_snake,
         evas_3d_node_mesh_frame_set(scene->mesh_snake, frame));

   /* create bounding box */
   _mesh_aabb(&scene->mesh_cube, scene);

   /* one way to stop the rocket */
   if (rocket_indicator == 2)
     {
        Box3 boxy;

        eo_do(scene->mesh_node_level[10],
              evas_3d_node_bounding_box_get(&x, &y, &z, &x1, &y1, &z1));
        evas_box3_set(&boxy, x, y, z, x1, y1, z1);

        eo_do(scene->mesh_node_rocket,
              evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

        eo_do(scene->mediator_node,
              evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x1, &y1, &z1));

        if (((z * sinus + x * cosinus + x1) >= boxy.p0.x) && ((z * sinus + x * cosinus + x1) <= boxy.p1.x) &&
            ((z * cosinus - x * sinus + z1) >= boxy.p0.z) && ((z * cosinus - x * sinus + z1) <= boxy.p1.z))
          {
             eo_do(scene->rocket,
                   evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.8, 0.8, 0.8, 1.0),
                   evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.9, 0.1, 0.1, 1.0),
                   evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 0.5, 0.5, 0.5, 1.0));

             _scale(scene->mesh_node_rocket, 1);
             rocket_indicator = 3;
          }
     }

   frame += 32;
   if (frame > 256 * 20) frame = 0;

   return ECORE_CALLBACK_RENEW;
}

static void
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *info)
{
   Evas_Event_Mouse_Down *ev = info;

   if (ev->button == 1)
     {
        rocket_indicator = 1;
        gun_indicator = 1;
     }

   /* camera default settings */
   d_angle_camera = 0.0;
}

/* emit signals independently */
static Eina_Bool
_continue_camera(void *data)
{
   if (camera_indicator == 1)
     {
        evas_event_feed_mouse_down(evas, 3, EVAS_BUTTON_NONE, 0, &data);
     }
   return EINA_TRUE;
}

/* calculate new angle of rotation */
static void
_reload_camera(void *data, Evas *evas EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void   *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Evas_Real s, t;

   eo_do(scene,
         evas_3d_scene_pick(ev->canvas.x, ev->canvas.y, &n, &m, &s, &t));

   d_angle_camera = 0;
   if (n || m)
     {
        mouse_diff_x = ev->canvas.x - mouse_old_x;
        mouse_diff_y = ev->canvas.y - mouse_old_y;
        mouse_old_x = ev->canvas.x;
        mouse_old_y = ev->canvas.y;

        if (camera_indicator == 1)
          d_angle_camera = (float) -mouse_diff_x / 100;
     }

   if (ev->button == 3)
     {
        if (camera_indicator == 0)
          {
             ecore_timer_add(0.160, _continue_camera, &data);
          }
        camera_indicator = 1;
     }
}

static void
_play_scene(void *data EINA_UNUSED,
            Evas *e EINA_UNUSED,
            Evas_Object *eo EINA_UNUSED,
            void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button == 2)
     {
        d_angle_eagle = 0.1;
        d_i = 1;

        if (camera_indicator == 1) camera_indicator = 2;
        d_angle_camera = 0;
     }
}

void
_world_init(int precision)
{
   int              i, j;
   unsigned short   *index;
   double           lati, y , r, longi;
   vec3        e1, e2;
   float            du1, du2, dv1, dv2, f;
   vec3        tangent;

   vertex_count = (precision + 1) * (precision + 1);
   index_count = precision * precision * 6;

   /* Allocate buffer. */
   vertices = malloc(sizeof(vertex) * vertex_count);
   indices = malloc(sizeof(unsigned short) * index_count);

   for (i = 0; i <= precision; i++)
     {
        lati = (M_PI * (double)i) / (double)precision;
        y = cos(lati);
        r = fabs(sin(lati));

        for (j = 0; j <= precision; j++)
          {
             longi = (M_PI * 2.0 * j) / precision;
             vertex *v = &vertices[i * (precision  + 1) + j];

             if (j == 0 || j == precision)
               v->position.x = 0.0;
             else
               v->position.x = r * sin(longi);

             v->position.y = y;

             if (j == 0 || j == precision)
               v->position.z = r;
             else
               v->position.z = r * cos(longi);

             v->normal = v->position;

             if (v->position.x > 0.0)
               {
                  v->tangent.x = -v->normal.y;
                  v->tangent.y =  v->normal.x;
                  v->tangent.z =  v->normal.z;
               }
             else
               {
                  v->tangent.x =  v->normal.y;
                  v->tangent.y = -v->normal.x;
                  v->tangent.z =  v->normal.z;
               }

             v->color.x = v->position.x;
             v->color.y = v->position.y;
             v->color.z = v->position.z;
             v->color.w = 1.0;

             if (j == precision)
               v->texcoord.x = 1.0;
             else if (j == 0)
               v->texcoord.x = 0.0;
             else
               v->texcoord.x = (double)j / (double)precision;

             if (i == precision)
               v->texcoord.y = 1.0;
             else if (i == 0)
               v->texcoord.y = 0.0;
             else
               v->texcoord.y = 1.0 - (double)i / (double)precision;
          }
     }

     index = &indices[0];

     for (i = 0; i < precision; i++)
       {
          for (j = 0; j < precision; j++)
            {
               *index++ = i * (precision + 1) + j;
               *index++ = i * (precision + 1) + j + 1;
               *index++ = (i + 1) * (precision + 1) + j;

               *index++ = (i + 1) * (precision + 1) + j;
               *index++ = i * (precision + 1) + j + 1;
               *index++ = (i + 1) * (precision + 1) + j + 1;
            }
       }

     for (i = 0; i < index_count; i += 3)
       {
          vertex *v0 = &vertices[indices[i + 0]];
          vertex *v1 = &vertices[indices[i + 1]];
          vertex *v2 = &vertices[indices[i + 2]];

          e1.x = v1->position.x - v0->position.x;
          e1.y = v1->position.y - v0->position.y;
          e1.z = v1->position.z - v0->position.z;

          e2.x = v2->position.x - v0->position.x;
          e2.y = v2->position.y - v0->position.y;
          e2.z = v2->position.z - v0->position.z;

          du1 = v1->texcoord.x - v0->texcoord.x;
          dv1 = v1->texcoord.y - v0->texcoord.y;

          du2 = v2->texcoord.x - v0->texcoord.x;
          dv2 = v2->texcoord.y - v0->texcoord.y;

          f = 1.0 / (du1 * dv2 - du2 * dv1);

          tangent.x = f * (dv2 * e1.x - dv1 * e2.x);
          tangent.y = f * (dv2 * e1.y - dv1 * e2.y);
          tangent.z = f * (dv2 * e1.z - dv1 * e2.z);

          v0->tangent = tangent;
       }

     for (i = 0; i <= precision; i++)
       {
          for (j = 0; j <= precision; j++)
            {
               if (j == precision)
                 {
                    vertex *v = &vertices[i * (precision  + 1) + j];
                    v->tangent = vertices[i * (precision + 1)].tangent;
                 }
            }
       }
}

void
_mesh_setup_gun_planet(Scene_Data *data)
{
   _world_init(100);

   /* Setup material and texture for world. */

   data->material_world = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material_world,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.0, 0.0, 0.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,  0.5, 0.5, 0.5, 0.9),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

   /* Setup material and texture for eagle. */

   data->mesh_eagle = eo_add(EVAS_3D_MESH_CLASS, evas);
   eo_do(data->mesh_eagle,
         evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_OBJ, "shooter/eagle.obj", NULL));

   data->material_eagle = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->mesh_eagle,
         evas_3d_mesh_frame_material_set(0, data->material_eagle));

   data->texture_diffuse_eagle = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(data->texture_diffuse_eagle,
         evas_3d_texture_file_set("eagle.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_eagle,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture_diffuse_eagle),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   eo_do(data->mesh_eagle,
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));

   data->mesh_node_eagle = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                         evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->mesh_node_eagle,
         evas_3d_node_position_set(-20, 20, 0));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_eagle));

   eo_do(data->mesh_node_eagle,
         evas_3d_node_mesh_add(data->mesh_eagle));

   _scale(data->mesh_node_eagle, 0.05);

   /* Setup mesh for world. */

   data->mesh_world = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_world,
         evas_3d_mesh_vertex_count_set(vertex_count),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            sizeof(vertex), &vertices[0].position),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            sizeof(vertex), &vertices[0].normal),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TANGENT,
                                            sizeof(vertex), &vertices[0].tangent),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            sizeof(vertex), &vertices[0].color);
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            sizeof(vertex), &vertices[0].texcoord),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES));

   data->mesh_node_world = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                         evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_world));

   eo_do(data->mesh_node_world,
         evas_3d_node_mesh_add(data->mesh_world));

   eo_do(data->mesh_world,
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_frame_material_set(0, data->material_world));

   eo_do(data->mesh_node_world,
         evas_3d_node_position_set(0.0, 0.0, 0.0));

   _scale(data->mesh_node_world, 70.0);

   /* Setup mesh for gun */

   data->gun = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   float curve [12] ={ 0.55, -2.5,  0.50, -2.25,  0.4, -2,  0.5, 0, 0.5, 1,  0.5, 5};
   _add_solid_of_revolution(&curve[0], 5, wave_vertices, wave_indices);

   eo_do(data->gun,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.9, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   data->mesh_gun = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_gun,
         evas_3d_mesh_vertex_count_set(10000),
         evas_3d_mesh_frame_add(0),
         /* 12 = 3 + 3 + 4 +2 : size of one vertex without TANGENT */
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &wave_vertices[ 0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                      12 * sizeof(float), &wave_vertices[ 3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float), &wave_vertices[ 6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &wave_vertices[10]),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     10000, &wave_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->gun));

   data->mesh_node_gun = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                       evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->mediator_node,
         evas_3d_node_member_add(data->mesh_node_gun));
   eo_do(data->mesh_node_gun,
         evas_3d_node_mesh_add(data->mesh_gun),
         evas_3d_node_position_set(2, -2, -4.0));

   /* Setup mesh for gun butt */

   data->material_gun_butt = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   float curve_butt [22] = { 0.5, -1,  0.61, -0.8,  0.67, -0.6,  0.72, -0.4,  0.75, -0.2,  0.77, 0,  0.75, 0.2,  0.72, 0.4,  0.67, 0.6,   0.61, 0.8,  0.5, 1,};
   _add_solid_of_revolution(&curve_butt[0], 10, wave_vertices_butt, wave_indices_butt);

   eo_do(data->material_gun_butt,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.9, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   data->mesh_gun_butt = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_gun_butt,
         evas_3d_mesh_vertex_count_set(10000),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &wave_vertices_butt[ 0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float), &wave_vertices_butt[ 3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float), &wave_vertices_butt[ 6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &wave_vertices_butt[10]),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     10000, &wave_indices_butt[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->material_gun_butt));

   data->mesh_node_gun_butt = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                            evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->mediator_node,
          evas_3d_node_member_add(data->mesh_node_gun_butt));

   eo_do(data->mesh_node_gun_butt,
         evas_3d_node_mesh_add(data->mesh_gun_butt),
         evas_3d_node_position_set(2, -2, -3.0));

   /* Setup mesh for gun cage */

   data->material_gun_cage = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material_gun_cage,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(70.0));

   data->mesh_gun_cage = eo_add(EVAS_3D_MESH_CLASS, evas);
   eo_do(data->mesh_gun_cage,
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &cage_vertices[ 0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float), &cage_vertices[ 3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float), &cage_vertices[ 6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &cage_vertices[10]),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     36, &cube_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->material_gun_cage));

   data->mesh_node_gun_cage = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                            evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->mediator_node,
         evas_3d_node_member_add(data->mesh_node_gun_cage));
   eo_do(data->mesh_node_gun_cage,
         evas_3d_node_mesh_add(data->mesh_gun_cage),
         evas_3d_node_position_set(1, -2, -3.0));

   _scale(data->mesh_node_gun_cage, 0.5);

   /* Setup mesh for rocket */

   data->rocket = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   float rocket_curve [18] = { 0.0, -1, 0.5, -0.866,  0.707, -0.707,  0.866, -0.5,  1,  0,  0.866,  0.5,  0.707, 0.707,  0.5, 0.866,   0, 1 };
   _add_solid_of_revolution(&rocket_curve[0], 8, wave_vertices2, wave_indices2);

   eo_do(data->rocket,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.2, 0.2, 0.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 0.5, 0.5, 0.5, 1.0),
         evas_3d_material_shininess_set(100.0));

   data->mesh_rocket = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_rocket,
         evas_3d_mesh_vertex_count_set(10000),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &wave_vertices2[ 0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float), &wave_vertices2[ 3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float), &wave_vertices2[ 6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &wave_vertices2[10]),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     10000, &wave_indices2[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->rocket));

   data->mesh_node_rocket = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                          evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->mediator_node,
         evas_3d_node_member_add(data->mesh_node_rocket));
   eo_do(data->mesh_node_rocket,
         evas_3d_node_mesh_add(data->mesh_rocket),
         evas_3d_node_position_set(2, -2, 0.0));

   _scale(data->mesh_node_rocket, 0.4);

   /* Set material for bounding box */

   data->material_cube = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   eo_do(data->material_cube,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE));

   data->mesh_cube = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_cube,
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     36, &cube_indices[0]),
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_LINES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_frame_material_set(0, data->material_cube));

   data->mesh_node_cube = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                        evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_cube));
   eo_do(data->mesh_node_cube,
         evas_3d_node_mesh_add(data->mesh_cube));
}

void
_mesh_setup_grass(Scene_Data *data, int index)
{
   /* Setup material and texture for grass. */
   data->material_grass = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   data->texture_diffuse_grass = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(data->texture_diffuse_grass,
          evas_3d_texture_file_set("shooter/grass6.png", NULL),
          evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_LINEAR, EVAS_3D_TEXTURE_FILTER_LINEAR));

   eo_do(data->material_grass,
          evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture_diffuse_grass),
          evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
          evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
          evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
          evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
          evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,  0.8, 0.8, 0.8, 1.0),
          evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
          evas_3d_material_shininess_set(50.0));

   /* Setup mesh for grass. */
   data->mesh_grass[index] = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_grass[index],
         evas_3d_mesh_vertex_count_set(vertex_count),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &grass_vertices[ 0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float), &grass_vertices[ 3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float), &grass_vertices[ 6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &grass_vertices[10]),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     6, &grass_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES));

   data->mesh_node_grass[index] = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                                evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_grass[index]));
   eo_do(data->mesh_node_grass[index],
         evas_3d_node_mesh_add(data->mesh_grass[index]));

   eo_do(data->mesh_grass[index],
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_frame_material_set(0, data->material_grass));

   /* placing of grass carpet on the floor grid */
   eo_do(data->mesh_node_grass[index],
         evas_3d_node_position_set(-36 + 26 * fmod(index, 5), -10.0, -52 + 16 * (index - fmod(index, 5)) / 5));
   _scale(data->mesh_node_grass[index], 2.0);
}

void
_mesh_setup_gun_bling(Scene_Data *data, int index)
{
   /* Setup mesh for gun bling */

   data->material_gun_bling = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material_gun_bling,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.4, 0.4, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.9, 0.9, 0.9, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 0.0, 1.0, 0.0, 1.0),
         evas_3d_material_shininess_set(500.0));

   data->mesh_gun_bling[index] = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_gun_bling[index],
         evas_3d_mesh_vertex_count_set(vertex_count),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            sizeof(vertex), &vertices[0].position),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            sizeof(vertex), &vertices[0].normal),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TANGENT,
                                            sizeof(vertex), &vertices[0].tangent),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            sizeof(vertex), &vertices[0].color),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            sizeof(vertex), &vertices[0].texcoord),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES));

   data->mesh_node_gun_bling[index] = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->mediator_node,
         evas_3d_node_member_add(data->mesh_node_gun_bling[index]));

   eo_do(data->mesh_node_gun_bling[index],
         evas_3d_node_mesh_add(data->mesh_gun_bling[index]));
   eo_do(data->mesh_gun_bling[index],
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->material_gun_bling));

   _scale(data->mesh_node_gun_bling[index], 0.2);
   if (index == 0)
     eo_do(data->mesh_node_gun_bling[index],
           evas_3d_node_position_set(1.9, -1.3, -3.0));
   else if (index == 1)
     eo_do(data->mesh_node_gun_bling[index],
           evas_3d_node_position_set(1.9, -1.4, -2.5));
   else
     eo_do(data->mesh_node_gun_bling[index],
           evas_3d_node_position_set(1.9, -1.4, -3.5));
}

void
_mesh_setup(Scene_Data *data, float *isource, int index)
{
   /* Setup mesh. */
   data->mesh_level[index] = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_level[index],
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &isource[ 0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float), &isource[ 3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &isource[10]));

   data->material_level = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->mesh_level[index],
         evas_3d_mesh_frame_material_set(0, data->material_level));

   eo_do(data->material_level,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   data->texture = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   if (index)
     eo_do(data->texture,
           evas_3d_texture_file_set("shooter/brick-stone.png", NULL));
   else
     eo_do(data->texture,
          evas_3d_texture_file_set("shooter/stena_kirpichi.png", NULL));

   eo_do(data->texture,
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_level,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE));

   eo_do(data->mesh_level[index],
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     36, &cube_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));
}

void
_mesh_setup_warrior(Scene_Data *data, int index)
{
   data->mesh_level[index] = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(data->mesh_level[index],
         evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, "shooter/warrior.md2", NULL));

   data->material_level = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   eo_do(data->mesh_level[index],
         evas_3d_mesh_frame_material_set(0, data->material_level));

   data->texture = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(data->texture,
         evas_3d_texture_file_set("shooter/warrior.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_level,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   eo_do(data->mesh_level[index],
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));
}

void
_mesh_setup_snake(Scene_Data *data)
{
   data->mesh_snake = eo_add(EVAS_3D_MESH_CLASS, evas);
   eo_do(data->mesh_snake,
         evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, "shooter/snake.md2", NULL));

   data->material_snake = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   eo_do(data->mesh_snake,
         evas_3d_mesh_frame_material_set(0, data->material_snake));

   data->texture_snake = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(data->texture_snake,
         evas_3d_texture_file_set("shooter/snake.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_snake,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture_snake),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   eo_do(data->mesh_snake,
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));

   data->mesh_node_snake = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                         evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->mesh_node_snake,
         evas_3d_node_position_set(-15, -10, 0),
         evas_3d_node_scale_set(0.15, 0.15, 0.15));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_snake));
   eo_do(data->mesh_node_snake,
         evas_3d_node_mesh_add(data->mesh_snake));

   _scale(data->mesh_node_snake, 0.15);
}

void
_scene_setup(Scene_Data *data)
{
   scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   eo_do(scene,
         evas_3d_scene_size_set(WIDTH, HEIGHT),
         evas_3d_scene_background_color_set(0.5, 0.5, 0.5, 0.0));

   data->root_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                   evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup_gun_planet(data);
   _mesh_setup_snake(data);
   int i = 0;

   for ( i = 0; i < 3; i++)
     _mesh_setup_gun_bling(data, i);

   for ( i = 0; i < 30; i++)
     _mesh_setup_grass(data, i);
   _mesh_setup(data, bricks_vertices, 0);
   _mesh_setup(data, wall_vertices, 1);

   data->mesh_node_level[0] = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                            evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_level[0]));

   eo_do(data->mesh_node_level[0],
         evas_3d_node_mesh_add(data->mesh_level[0]),
         evas_3d_node_position_set(0, 0, -15));
   int offset = 0;
   Box3 box, cbox;
   evas_box3_empty_set(&box);
   evas_box3_empty_set(&cbox);
   evas_box3_set(&box, -8.0, -8.0, 0.0, -8.0, -8.0, 0.0);
   Evas_Real bx, by, bz;
   for ( i = 1; i < 10; ++i)
     {
        data->mesh_node_level[i] = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                                 evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

        eo_do(data->mesh_node_level[i],
              evas_3d_node_position_set(-8 + offset, -8 + offset, -10),
              evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &bx, &by, &bz));

        evas_box3_set(&cbox, 0.0, 0.0, 0.0, -8 + offset, -8 + offset, 5);
        evas_box3_union(&box, &box, &cbox);

        eo_do(data->root_node,
              evas_3d_node_member_add(data->mesh_node_level[i]));
        eo_do(data->mesh_node_level[i],
              evas_3d_node_mesh_add(data->mesh_level[1]));
        offset += 2;
     }
   _mesh_setup_warrior(data, 2);

   data->mesh_node_level[10] = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->mesh_node_level[10],
         evas_3d_node_position_set(17, -10, 0),
         evas_3d_node_scale_set(0.15, 0.15, 0.15));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_level[10]));
   eo_do(data->mesh_node_level[10],
         evas_3d_node_mesh_add(data->mesh_level[2]));

   _mesh_aabb(&data->mesh_cube, data);

   eo_do(scene,
         evas_3d_scene_root_node_set(data->root_node),
         evas_3d_scene_camera_node_set(data->camera_node));

}

int
main(void)
{
   Scene_Data data;

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new("opengl_x11", 0, 0, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

   /* Add a background rectangle objects. */
   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   /* Add an image object for 3D scene rendering. */
   image = evas_object_image_filled_add(evas);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   /* Set the image object as render target for 3D scene. */
   eo_do(image,
         evas_obj_image_scene_set(scene));

   evas_object_focus_set(image, EINA_TRUE);

   /* Add animation timer callback. */
   Timers timers[4];


   timers[0].t = ecore_timer_add(0.016, _animate_scene_gun, &data);
   timers[0].data = &data;
   timers[1].t = ecore_timer_add(0.016, _animate_camera, &data);
   timers[0].data = &data;

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _reload_camera, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, &timers);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_UP, _play_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _key_down, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_UP, _key_up, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
