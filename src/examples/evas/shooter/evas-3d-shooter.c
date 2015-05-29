/*
* This is example of 3d-shooter done with help of evas-3d
*
* Basic control: 'w' - to  go forward, 's' - to go back, 'a' - to the left, 'd' - to the right,
* right mouse down - to activate camera rotation, mouse wheel down - to deactivate camera rotation,
* left mouse down - to fire, 'space' - to jump, 'S' - to activate or deactivete shadow,
* 'F' - to activate or deactivate fog, 'N' - to activate or deactivate normal mapping.
* The warrior is atackable, the snake isn't, this possibility is given by presence of the bounding box,
* which fixes the rocket entry. The warrior isn't passable for camera,
* neither is wall, stairs and columns. There is a possibility to go upstairs and break down.
*
* Compile with "gcc -g evas-3d-shooter.c evas-3d-shooter-header.c -o evas-3d-shooter `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm"
*
* Run program with flag "-s=TRUE" to turn on shadows, with "-f=TRUE" to turn on the fog, with "-b=TRUE" to turn on the blending.
*/

#include "evas-3d-shooter-header.h"

Evas_3D_Scene    *global_scene      = NULL;
Ecore_Evas       *ecore_evas        = NULL;
static float     angle_camera       = 0.0f;
static float     angle_eagle        = 180.0f;
static float     d_angle_camera     = 0.0;
float            d_angle_eagle      = 0.5;
float            world_position     = 0.0;
float            camera_position    = 25.0;
float            camera_move        = 0.0;
float            space_move         = 0.0;
float            old_space_position = 0.0;
float            fall_speed         = 0.0;
float            rocket_parametr[10];
float            new_y              = 0.0;
int              rocket_indicator[10];
int              soldier_mode       = 0;
int              cage_indicator     = 0;
int              gun_indicator      = 0;
int              tommy_indicator    = 0;
int              mouse_diff_x       = 0;
int              mouse_diff_y       = 0;
int              mouse_old_x        = 0;
int              mouse_old_y        = 0;
int              camera_indicator   = 0;
int              lateral_motion_indicator   = 0;
int              step_indicator     = 0;
int              fall_indicator     = 0;
int              aabb_index         = 0;

typedef struct _Ecore_Timers
{
   Ecore_Timer *t;
   void *data;
} Timers;

static float cube_vertices[] =
{
   /* Front */
   -1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,
    1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,
   -1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,
    1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,

   /* Back */
    1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,
   -1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,
    1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,
   -1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,

   /* Left */
   -1.0,  1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,
   -1.0,  1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,
   -1.0, -1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,
   -1.0, -1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,

   /* Right */
    1.0,  1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,
    1.0,  1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,
    1.0, -1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,
    1.0, -1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,

   /* Top */
   -1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,
    1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,
   -1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,
    1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,

   /* Bottom */
    1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,
   -1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,
    1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,
   -1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,
};

static float grass_vertices[] =
{
   /* Top */
   -6.5,  0.0, -4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  4.0,
    6.5,  0.0, -4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     4.0,  4.0,
   -6.5,  0.0,  4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    6.5,  0.0,  4.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     4.0,  0.0,

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
   int i = 0;
   if (!strcmp(ev->key, "space"))
     {
        if (space_move == 0.0)
          {
             eo_do(scene->mesh_node_tommy,
                   evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
             old_space_position = y;
             space_move = JUMP;
          }
     }
   else if (!strcmp(ev->key, "s"))
     KEY_MOTION(0, CAMERA_MOVE)
   else if (!strcmp(ev->key, "w"))
     KEY_MOTION(0, -CAMERA_MOVE)
   else if (!strcmp(ev->key, "d"))
     KEY_MOTION(1, CAMERA_MOVE)
   else if (!strcmp(ev->key, "a"))
     KEY_MOTION(1, -CAMERA_MOVE)
   else if (!strcmp(ev->key, "B"))
     {
        if (!scene->blending)
          {
             scene->blending = EINA_TRUE;
             eo_do(scene->texture_diffuse_carpet,
                   evas_3d_texture_file_set(gazebo_t_trans_path, NULL));
             eo_do(scene->mesh_carpet,
                   evas_3d_mesh_blending_enable_set(EINA_TRUE),
                   evas_3d_mesh_blending_func_set(EVAS_3D_BLEND_SRC_ALPHA, EVAS_3D_BLEND_ONE_MINUS_SRC_ALPHA));
          }
        else
          {
             scene->blending = EINA_FALSE;
             eo_do(scene->texture_diffuse_carpet,
                   evas_3d_texture_file_set(gazebo_t_path, NULL));
             eo_do(scene->mesh_carpet,
                   evas_3d_mesh_blending_enable_set(EINA_FALSE));
          }
     }
   else if (!strcmp(ev->key, "S"))
     {
        if (!scene->blending)
          {
             scene->blending = EINA_TRUE;
             eo_do(global_scene,
                   evas_3d_scene_shadows_enable_set(EINA_TRUE));
          }
        else
          {
             scene->blending = EINA_FALSE;
             eo_do(global_scene,
                   evas_3d_scene_shadows_enable_set(EINA_FALSE));
          }
     }
   else if (!strcmp(ev->key, "F"))
     {
        if (!scene->fog)
          {
             scene->fog = EINA_TRUE;
             ENABLE_FOG(scene->mesh_soldier)
             ENABLE_FOG(scene->mesh_soldier_jump)
             ENABLE_FOG(scene->mesh_warrior)
             ENABLE_FOG(scene->mesh_tommy)
             ENABLE_FOG(scene->mesh_gazebo)
             ENABLE_FOG(scene->mesh_carpet)
             ENABLE_FOG(scene->mesh_eagle)
             ENABLE_FOG(scene->mesh_world)
             ENABLE_FOG(scene->mesh_snake)
             ENABLE_FOG(scene->mesh_column_c)
             ENABLE_FOG(scene->mesh_grass)
             for (i = 0; i < 4; i++)
               {
                  ENABLE_FOG(scene->mesh_wall[i])
                  ENABLE_FOG(scene->mesh_column[i])
               }
             for (i = 0; i < 10; i++)
               {
                  ENABLE_FOG(scene->mesh_level[i])
                  ENABLE_FOG(scene->mesh_rocket[i])
               }
          }
        else
          {
             scene->fog = EINA_FALSE;
             DISABLE_FOG(scene->mesh_soldier)
             DISABLE_FOG(scene->mesh_soldier_jump)
             DISABLE_FOG(scene->mesh_warrior)
             DISABLE_FOG(scene->mesh_tommy)
             DISABLE_FOG(scene->mesh_gazebo)
             DISABLE_FOG(scene->mesh_carpet)
             DISABLE_FOG(scene->mesh_eagle)
             DISABLE_FOG(scene->mesh_world)
             DISABLE_FOG(scene->mesh_snake)
             DISABLE_FOG(scene->mesh_column_c)
             DISABLE_FOG(scene->mesh_grass)
             for (i = 0; i < 4; i++)
               {
                  DISABLE_FOG(scene->mesh_wall[i])
                  DISABLE_FOG(scene->mesh_column[i])
               }
             for (i = 0; i < 10; i++)
               {
                  DISABLE_FOG(scene->mesh_level[i])
                  DISABLE_FOG(scene->mesh_rocket[i])
               }
          }
     }
   else if (!strcmp(ev->key, "N"))
     {
        if (!scene->normal)
          {
             scene->normal = EINA_TRUE;
             ENABLE_NORMAL(scene->mesh_gazebo)
             ENABLE_NORMAL(scene->mesh_carpet)
             ENABLE_NORMAL(scene->mesh_snake)
             ENABLE_NORMAL(scene->mesh_column_c)
             ENABLE_NORMAL(scene->mesh_grass)
             for (i = 0; i < 4; i++)
               {
                  ENABLE_NORMAL(scene->mesh_wall[i])
                  ENABLE_NORMAL(scene->mesh_column[i])
               }
             for (i = 0; i < 10; i++)
               {
                  ENABLE_NORMAL(scene->mesh_level[i])
               }
          }
        else
          {
             scene->normal = EINA_FALSE;
             DISABLE_NORMAL(scene->mesh_gazebo)
             DISABLE_NORMAL(scene->mesh_carpet)
             DISABLE_NORMAL(scene->mesh_snake)
             DISABLE_NORMAL(scene->mesh_column_c)
             DISABLE_NORMAL(scene->mesh_grass)
             for (i = 0; i < 4; i++)
               {
                  DISABLE_NORMAL(scene->mesh_wall[i])
                  DISABLE_NORMAL(scene->mesh_column[i])
               }
             for (i = 0; i < 10; i++)
               {
                  DISABLE_NORMAL(scene->mesh_level[i])
               }
          }
     }
   else if (!strcmp(ev->key, "F1"))
     eo_do(scene->camera_node,
           evas_3d_node_position_set(0.0, 80.0, 30.0);
           evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   else if (!strcmp(ev->key, "F2"))
     eo_do(scene->camera_node,
           evas_3d_node_position_set(-2.0, 0.0, 4.0),
           evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, -100.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   else if (!strcmp(ev->key, "A"))
     aabb_index++;
}

static void
_key_up(void *data EINA_UNUSED,
        Evas *e EINA_UNUSED,
        Evas_Object *eo EINA_UNUSED,
        void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (!strcmp(ev->key, "space"))
     space_move = 0;
   else if ((!strcmp(ev->key, "s")) || (!strcmp(ev->key, "w")) ||
       (!strcmp(ev->key, "d")) || (!strcmp(ev->key, "a")))
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
   Evas_Real x_move, x_move_lateral, z_move, z_move_lateral;

   angle_camera += d_angle_camera;
   x_move = camera_move * sin(angle_camera * TO_RADIAN);
   z_move = camera_move * cos(angle_camera * TO_RADIAN);
   x_move_lateral = camera_move * sin((angle_camera + 90) * TO_RADIAN);
   z_move_lateral = camera_move * cos((angle_camera + 90) * TO_RADIAN);

   eo_do(scene->mediator_node,
         evas_3d_node_position_get(EVAS_3D_SPACE_WORLD, &x, &y, &z));

   eo_do(scene->mesh_node_world,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x1, &y1, &z1));

   eo_do(scene->mesh_node_tommy,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x2, &y2, &z2));

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
             fall_speed += FALL_SPEED_UP;
          }
     }
   else if ((lateral_motion_indicator == 0) && (SOLDIER_FRAME_X1 < x + x_move) && (x + x_move < SOLDIER_FRAME_X2)
            && (SOLDIER_FRAME_Z1 < z + z_move) && (z + z_move < SOLDIER_FRAME_Z2) &&
            (_horizontal_circle_position_resolution(scene->mesh_node_warrior, x + x_move, z + z_move, SOLDIER_RADIUS)) &&
            (_horizontal_circle_resolution(scene->mesh_node_level[0], x + x_move, z + z_move, WALL_RADIUS)))
     {
        int under_breaks = 0;
        for (i = 2; i < 5; i++)
           {
              if (!_horizontal_position_resolution(scene->mesh_node_level[i],
                                                   x + x_move, z + z_move))
                {
                   under_breaks++;
                   break;
                }
           }

        if ((!under_breaks || step_indicator) &&
            _horizontal_circle_position_resolution(scene->mesh_node_column[0], x + x_move, z + z_move, SOLDIER_RADIUS) &&
            _horizontal_circle_position_resolution(scene->mesh_node_column[1], x + x_move, z + z_move, SOLDIER_RADIUS) &&
            _horizontal_circle_position_resolution(scene->mesh_node_column[2], x + x_move, z + z_move, SOLDIER_RADIUS) &&
            _horizontal_circle_position_resolution(scene->mesh_node_column[3], x + x_move, z + z_move, SOLDIER_RADIUS))
          {
             eo_do(scene->mediator_node,
                   evas_3d_node_position_set(x + x_move, y, z + z_move));
             eo_do(scene->mesh_node_world,
                   evas_3d_node_position_set(x1 + x_move, y1, z1 + z_move));
          }
     }
   else if ((SOLDIER_FRAME_X1 < x + x_move_lateral) && (x + x_move_lateral < SOLDIER_FRAME_X2) &&
            (SOLDIER_FRAME_Z1 < z + z_move_lateral) && (z + z_move_lateral < SOLDIER_FRAME_Z2) &&
            (_horizontal_circle_position_resolution(scene->mesh_node_warrior, x + x_move_lateral, z + z_move_lateral, SOLDIER_RADIUS))&&
            (_horizontal_circle_resolution(scene->mesh_node_level[0], x + x_move_lateral, z + z_move_lateral, WALL_RADIUS)))
     {
        int under_breaks = 0;
        for (i = 2; i < 5; i++)
           {
              if (!_horizontal_position_resolution(scene->mesh_node_level[i],
                                                   x + x_move_lateral, z + z_move_lateral))
                {
                   under_breaks++;
                   break;
                }
           }
        if ((!under_breaks || step_indicator) &&
            _horizontal_circle_position_resolution(scene->mesh_node_column[0], x + x_move_lateral, z + z_move_lateral, SOLDIER_RADIUS) &&
            _horizontal_circle_position_resolution(scene->mesh_node_column[1], x + x_move_lateral, z + z_move_lateral, SOLDIER_RADIUS) &&
            _horizontal_circle_position_resolution(scene->mesh_node_column[2], x + x_move_lateral, z + z_move_lateral, SOLDIER_RADIUS) &&
            _horizontal_circle_position_resolution(scene->mesh_node_column[3], x + x_move_lateral, z + z_move_lateral, SOLDIER_RADIUS))
          {
             eo_do(scene->mediator_node,
                   evas_3d_node_position_set(x + x_move_lateral, y, z + z_move_lateral));
             eo_do(scene->mesh_node_world,
                   evas_3d_node_position_set(x1 + x_move_lateral, y1, z1 + z_move_lateral));
          }
     }

   eo_do(scene->mediator_node,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

   if (!_horizontal_position_resolution(scene->mesh_node_level[step_indicator+1], x, z))
     {
        eo_do(scene->mediator_node,
              evas_3d_node_position_set(x, y + STEP, z));
        old_space_position += STEP;
        step_indicator++;
     }
   else if (!_horizontal_position_resolution(scene->mesh_node_level[step_indicator-1], x, z) && step_indicator > 0)
     {
        eo_do(scene->mediator_node,
              evas_3d_node_position_set(x, y - STEP, z));
        old_space_position -= STEP;
        step_indicator--;
     }
   else if (_horizontal_position_resolution(scene->mesh_node_level[step_indicator], x, z) && (step_indicator > 0))
     {
        fall_speed += FALL_SPEED_UP;
        step_indicator = 0;
     }

   eo_do(scene->mediator_node,
         evas_3d_node_orientation_angle_axis_set(angle_camera, 0, 1.0, 0.0),
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

   eo_do(scene->mesh_node_warrior,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x1, &y1, &z1));
   eo_do(scene->mesh_node_snake,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x2, &y2, &z2));

   float snake_x = (x2 - x) / _distance(x, z, x2, z2);
   float snake_z = (z2 - z) / _distance(x, z, x2, z2);
   float looking = sin(angle_camera* TO_RADIAN)* snake_x + cos(angle_camera* TO_RADIAN)* snake_z;

   if (_horizontal_circle_position_resolution(scene->mesh_node_warrior, (x1 - x) * WARRIOR_SPEED + x,
                                              (z1 - z) * WARRIOR_SPEED + z, SOLDIER_RADIUS))
     {
        if (_horizontal_circle_object_resolution(scene->mesh_node_warrior, scene->mesh_node_column[0], COLUMN_RADIUS) &&
            _horizontal_circle_object_resolution(scene->mesh_node_warrior, scene->mesh_node_column[1], COLUMN_RADIUS) &&
            _horizontal_circle_object_resolution(scene->mesh_node_warrior, scene->mesh_node_column[2], COLUMN_RADIUS) &&
            _horizontal_circle_object_resolution(scene->mesh_node_warrior, scene->mesh_node_column[3], COLUMN_RADIUS) &&
            _horizontal_circle_object_resolution(scene->mesh_node_level[0], scene->mesh_node_warrior, COLUMN_RADIUS))
           eo_do(scene->mesh_node_warrior,
                 evas_3d_node_position_set((x - x1) * WARRIOR_SPEED + x1, y1, (z - z1) * WARRIOR_SPEED + z1));
        else
          eo_do(scene->mesh_node_warrior,
                evas_3d_node_position_set((z - z1) * WARRIOR_SPEED2 + x1, y1, -(x - x1) * WARRIOR_SPEED2 + z1));
     }

   /* if soldier is looking at the snake */
   if (looking < 0)
     {
        /* if snake is located at the left hand of the soldier */
        if (snake_x * cos(angle_camera * TO_RADIAN) - snake_z * sin(angle_camera* TO_RADIAN) < 0)
          {
             if ((_horizontal_object_resolution(scene->mesh_node_snake, SNAKE_FRAME_X1, SNAKE_FRAME_Z1,
                                                SNAKE_FRAME_X2, SNAKE_FRAME_Z2, snake_z * SNAKE_SPEED, -snake_x * SNAKE_SPEED)))
               eo_do(scene->mesh_node_snake,
                     evas_3d_node_position_set(snake_z * SNAKE_SPEED + x2, y2, -snake_x * SNAKE_SPEED + z2));
             else if (_horizontal_circle_resolution(scene->mesh_node_snake, (x2 - x) * SNAKE_SPEED2 + x,
                                                    (z2 - z) * SNAKE_SPEED2 + z, SOLDIER_RADIUS)) 
               eo_do(scene->mesh_node_snake,
                     evas_3d_node_position_set((x - x2) * SNAKE_SPEED2 + x2, y2, (z - z2) * SNAKE_SPEED2 + z2));
          }
        /* if snake is located at the right hand of the soldier */
        else
          {
             if ((_horizontal_object_resolution(scene->mesh_node_snake, SNAKE_FRAME_X1, SNAKE_FRAME_Z1,
                                                SNAKE_FRAME_X2, SNAKE_FRAME_Z2, -snake_z * SNAKE_SPEED, snake_x * SNAKE_SPEED)))
               eo_do(scene->mesh_node_snake,
                     evas_3d_node_position_set(-snake_z * SNAKE_SPEED + x2, y2, snake_x * SNAKE_SPEED + z2));
             else if (_horizontal_circle_resolution(scene->mesh_node_snake, (x2 - x) * SNAKE_SPEED2 + x,
                                                    (z2 - z) * SNAKE_SPEED2 + z, SOLDIER_RADIUS))
               eo_do(scene->mesh_node_snake,
                     evas_3d_node_position_set((x - x2) * SNAKE_SPEED2 + x2, y2, (z - z2) * SNAKE_SPEED2 + z2));
          }
     }
   else if (_horizontal_circle_resolution(scene->mesh_node_snake, (x2 - x) * SNAKE_SPEED2 + x, (z2 - z) * SNAKE_SPEED2 + z, SOLDIER_RADIUS))
     eo_do(scene->mesh_node_snake,
           evas_3d_node_position_set((x - x2) * SNAKE_SPEED2 + x2, y2, (z - z2) * SNAKE_SPEED2 + z2));

   eo_do(scene->mesh_node_snake,
         evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x2, &y2, &z2));

   eo_do(scene->mesh_node_warrior,
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, -1000.0, 0.0, EVAS_3D_SPACE_PARENT,
                                  (z - z1) / _distance(x, z, x1, z1), 0, (x1 - x) / _distance(x, z, x1, z1)));

   eo_do(scene->mesh_node_snake,
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 2 * x2 - x, y2, 2 * z2 - z, EVAS_3D_SPACE_PARENT, 0, 1, 0));

   if (!_horizontal_frame_resolution(scene->mesh_node_snake, SNAKE_FRAME_X1, SNAKE_FRAME_Z1,
                                                SNAKE_FRAME_X2, SNAKE_FRAME_Z2, &x_move, &z_move))
     eo_do(scene->mesh_node_snake,
           evas_3d_node_position_set(x2 + x_move, y2, z2 + z_move));

   return EINA_TRUE;
}

static Eina_Bool
_animate_scene_gun(void *data)
{
   static float angle = 0.0f;

   static float tommy_parametr = 0.0f;
   int i = 0;
   Evas_Real x, y, z;
   Evas_Real x1, y1, z1;
   static int frame = 0;
   static int frame_soldier, frame_eagle, frame_snake;
   Scene_Data *scene = (Scene_Data *)data;
   Box3 boxy;

   float sinus = sin(angle_camera * TO_RADIAN);
   float cosinus = cos(angle_camera * TO_RADIAN);

   /* rocket and gun movement */
   angle_eagle += d_angle_eagle;

   eo_do(scene->mesh_node_eagle,
         frame_eagle = evas_3d_node_mesh_frame_get(scene->mesh_eagle));
   if (frame_eagle > 19500) frame_eagle = 0;
   frame_eagle += 150;

   eo_do(scene->mesh_node_snake,
         frame_snake = evas_3d_node_mesh_frame_get(scene->mesh_snake));
   if (frame_snake > 16000) frame_snake = 0;
   frame_snake += 64;

   eo_do(scene->mesh_node_eagle,
         evas_3d_node_orientation_angle_axis_set(angle_eagle, 0.0, 1.0, 0.0),
         /* equation of eagle flight */
         evas_3d_node_position_set(38 * sin((angle_eagle + 90) * TO_RADIAN), 20 + 20 * sin(angle_eagle * TO_RADIAN),
                                   -10 + 20.0 * cos((angle_eagle + 90) * TO_RADIAN)),
         evas_3d_node_mesh_frame_set(scene->mesh_eagle, frame_eagle));

   for (i = 0; i < 10; i ++)
     {
        if (rocket_indicator[i] == 3)
          rocket_indicator[i] = 0;
        else if (rocket_indicator[i] == 0)
          /* almost hidden rocket */
          _scale(scene->mesh_node_rocket[i], 0.004);
        else if (rocket_indicator[i] == 1)
          {
             _scale(scene->mesh_node_rocket[i], 0.2);
             eo_do(scene->material_rocket[i],
                   evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.8, 0.8, 0.8, 1.0),
                   evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.2, 0.2, 0.0, 1.0),
                   evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 0.5, 0.5, 0.5, 1.0));

             rocket_indicator[i] = 2;

             eo_do(scene->mesh_node_rocket[i],
                   evas_3d_node_position_set(ROCKET_POS_X, ROCKET_POS_Y, ROCKET_POS_Z));
             rocket_parametr[i] = 0;
          }
        else if (rocket_indicator[i] == 2)
          {
             rocket_parametr[i] += ROCKET_SPEED;
             /* equation of rocket flight */

             eo_do(scene->mesh_node_rocket[i],
                   evas_3d_node_position_get(EVAS_3D_SPACE_WORLD, &x, &y, &z));

             if ((ROCKET_FRAME_X1 < x) && (x < ROCKET_FRAME_X2) && (ROCKET_FRAME_Z1 < z) && (z < ROCKET_FRAME_Z2))
                eo_do(scene->mesh_node_rocket[i],
                      evas_3d_node_position_set(ROCKET_POS_X, ROCKET_POS_Y, ROCKET_POS_Z - rocket_parametr[i]));
             else
               rocket_indicator[i] = 3;

             /* the way to stop the rocket */
             eo_do(scene->mesh_node_warrior,
                   evas_3d_node_bounding_box_get(&x, &y, &z, &x1, &y1, &z1));
             evas_box3_set(&boxy, x, y, z, x1, y1, z1);

             eo_do(scene->mesh_node_rocket[i],
                   evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));

             eo_do(scene->mediator_node,
                   evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x1, &y1, &z1));

             if (((z * sinus + x * cosinus + x1) >= boxy.p0.x) && ((z * sinus + x * cosinus + x1) <= boxy.p1.x) &&
                 ((z * cosinus - x * sinus + z1) >= boxy.p0.z) && ((z * cosinus - x * sinus + z1) <= boxy.p1.z))
               {
                  eo_do(scene->material_rocket[i],
                        evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.8, 0.8, 0.8, 1.0),
                        evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.9, 0.1, 0.1, 1.0),
                        evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 0.5, 0.5, 0.5, 1.0));

                  _scale(scene->mesh_node_rocket[i], 2.3);
                  rocket_indicator[i] = 3;
               }
          }
     }

   if (tommy_indicator == 1)
     {
        tommy_indicator = 2;
        eo_do(scene->mesh_node_tommy,
              evas_3d_node_position_set(TOMMY_POS_X, TOMMY_POS_Y, TOMMY_POS_Z));
        tommy_parametr = 0.0;
     }
   /* parametric equations of gun movement, (GUN_DIR_X, GUN_DIR_Y, GUN_DIR_Z) is direction vector */
   else if (tommy_indicator == 2)
     {
        tommy_parametr += GUN_SPEED;
        eo_do(scene->mesh_node_tommy,
              evas_3d_node_position_set(TOMMY_POS_X + GUN_DIR_X * tommy_parametr,
                                        TOMMY_POS_Y + GUN_DIR_Y * tommy_parametr,
                                        TOMMY_POS_Z + GUN_DIR_Z * tommy_parametr));

        if (TOMMY_POS_Z + GUN_DIR_Z * tommy_parametr >= -8.3)
          tommy_indicator = 3;
     }
   else if (tommy_indicator == 3)
     {
        tommy_parametr-= GUN_SPEED / 2;
        eo_do(scene->mesh_node_tommy,
              evas_3d_node_position_set(TOMMY_POS_X + GUN_DIR_X * tommy_parametr,
                                        TOMMY_POS_Y + GUN_DIR_Y * tommy_parametr,
                                        TOMMY_POS_Z + GUN_DIR_Z * tommy_parametr));

        eo_do(scene->mesh_node_tommy,
              evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        if (z <= TOMMY_POS_Z)
          {
             tommy_indicator = 4;
             tommy_parametr = 0;
          }
     }

   if (angle_eagle > 360.0) angle -= 360.0f;

   eo_do(scene->mesh_node_warrior,
         evas_3d_node_mesh_frame_set(scene->mesh_warrior, frame));

   eo_do(scene->mesh_node_snake,
         evas_3d_node_mesh_frame_set(scene->mesh_snake, frame_snake));

   if (space_move && (soldier_mode == 0))
     {
        eo_do(scene->mesh_node_soldier,
              evas_3d_node_mesh_add(scene->mesh_soldier_jump),
              evas_3d_node_mesh_del(scene->mesh_soldier),
              evas_3d_node_mesh_frame_set(scene->mesh_soldier_jump, 0));
        soldier_mode = 1;
        eo_do(scene->mesh_node_tommy,
              evas_3d_node_position_set(TOMMY_JUMP_X, TOMMY_JUMP_Y, TOMMY_JUMP_Z),
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 1000.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT,
                                  0, 0, 1));
     }
   else if (soldier_mode == 1)
     {
        eo_do(scene->mesh_node_soldier,
              frame_soldier = evas_3d_node_mesh_frame_get(scene->mesh_soldier_jump));
        frame_soldier += 124;
        eo_do(scene->mesh_node_soldier,
              evas_3d_node_mesh_frame_set(scene->mesh_soldier_jump, frame_soldier));
        if (frame_soldier < 2480)
          eo_do(scene->mesh_node_tommy,
                evas_3d_node_position_set(TOMMY_JUMP_X, TOMMY_JUMP_Y + 4.5 * frame_soldier / 2480, TOMMY_JUMP_Z));
        else
          eo_do(scene->mesh_node_tommy,
                evas_3d_node_position_set(TOMMY_JUMP_X, TOMMY_JUMP_Y + 4.5 * (4960 - frame_soldier) / 2480, TOMMY_JUMP_Z));
        if (frame_soldier > 4960)
          {
             eo_do(scene->mesh_node_soldier,
                   evas_3d_node_mesh_del(scene->mesh_soldier_jump));
             eo_do(scene->mesh_node_soldier,
                   evas_3d_node_mesh_add(scene->mesh_soldier),
                   evas_3d_node_mesh_frame_set(scene->mesh_soldier, 0));
             soldier_mode = 0;
             eo_do(scene->mesh_node_tommy,
                   evas_3d_node_position_set(TOMMY_POS_X, TOMMY_POS_Y, TOMMY_POS_Z),
                   evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, -1000.0, 0.0,
                                            EVAS_3D_SPACE_PARENT, 2, 0, 1.5));
          }
     }
   else if (camera_move)
     {
        eo_do(scene->mesh_node_soldier,
              frame_soldier = evas_3d_node_mesh_frame_get(scene->mesh_soldier));
        frame_soldier += 124;
        if (frame_soldier > 6200) frame_soldier = 0;
        eo_do(scene->mesh_node_soldier,
              evas_3d_node_mesh_frame_set(scene->mesh_soldier, frame_soldier));
     }

   /* create bounding box */
   _mesh_aabb(&scene->mesh_cube, scene, aabb_index);

   frame += 32;
   if (frame > 108 * 20) frame = 192;

   return ECORE_CALLBACK_RENEW;
}

static void
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *info)
{
   Evas_Event_Mouse_Down *ev = info;

   if ((ev->button == 1) && !space_move)
     {
        cage_indicator = fmod(++cage_indicator, 10);
        rocket_indicator[cage_indicator] = 1;
        tommy_indicator = 1;
     }

   /* camera default settings */
   d_angle_camera = 0.0;
}

/* emit signals independently */
static Eina_Bool
_continue_camera(void *data)
{
   if (camera_indicator == 1)
     evas_event_feed_mouse_down(evas, 3, EVAS_BUTTON_NONE, 0, &data);

   return EINA_TRUE;
}

/* calculate new angle of rotation */
static void
_reload_camera(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void   *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Evas_Real s, t;

   eo_do(global_scene,
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

        if (ev->canvas.x < 0)
          d_angle_camera = CAMERA_SPEED;
        else if (ev->canvas.x > WIDTH - 100)
          d_angle_camera = -CAMERA_SPEED;
     }

   if (ev->button == 3)
     {
        if (camera_indicator == 0)
          ecore_timer_add(0.160, _continue_camera, &data);
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
   vec3             e1, e2;
   float            du1, du2, dv1, dv2, f;
   vec3             tangent;

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
   _world_init(20);

   /* Setup material and texture for world. */

   SETUP_MESH(world, world, 0.5, 0.5, 1.0);
   SETUP_MESH_NODE(world);

   eo_do(data->mesh_world,
         evas_3d_mesh_vertex_count_set(vertex_count),
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
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &indices[0]));

   eo_do(data->mesh_node_world,
         evas_3d_node_scale_set(150, 150, 150),
         evas_3d_node_position_set(0.0, 0.0, 0.0));

   /* Setup material and texture for player */

   SETUP_MESH(soldier, soldier, 0.2, 0.6, 1.0)
   MATERIAL_TEXTURE_SET(soldier, soldier, soldier_path, soldier_tex_path)

   SETUP_MESH_NODE(soldier)

   eo_do(data->mesh_node_soldier,
         evas_3d_node_position_set(0, -10, 0),
         evas_3d_node_scale_set(5, 5, 5));

   eo_do(data->mesh_node_soldier,
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, -10.0, -100.0, EVAS_3D_SPACE_PARENT,
                                  0, 1, 0),
         evas_3d_node_mesh_frame_set(data->mesh_soldier, 0));

   SETUP_MESH(soldier, soldier_jump, 1.0, 1.0, 1.0)
   MATERIAL_TEXTURE_SET(soldier, soldier_jump, soldier_jump_path, soldier_tex_path)

   /* Setup material and texture for tommy gun */

   SETUP_MESH(tommy, tommy, 0.0, 0.3, 1.0)
   SETUP_MESH_NODE(tommy)
   eo_do(data->mesh_tommy,
         efl_file_set(gun_path, NULL),
         efl_file_save("try.obj", NULL, NULL),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->material_tommy));

   eo_do(data->mesh_node_tommy,
         evas_3d_node_scale_set(1.5, 1.5, 1.5),
         evas_3d_node_position_set(TOMMY_POS_X, TOMMY_POS_Y, TOMMY_POS_Z),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, -1000.0, 0.0, EVAS_3D_SPACE_PARENT,
                                  2, 0, 1.5));

   /* Setup material and texture for gazebo */

   SETUP_MESH(gazebo, gazebo, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(gazebo)
   MATERIAL_TEXTURE_SET(gazebo, gazebo, gazebo_bot_path, gazebo_b_path)
   NORMAL_SET(gazebo, gazebo, gazebo_b_n_path)

   eo_do(data->mesh_node_gazebo,
         evas_3d_node_scale_set(0.18, 0.18, 0.18),
         evas_3d_node_orientation_angle_axis_set(180, 0.0, 1.0, 0.0),
         evas_3d_node_position_set(-20, -10, -35));

   /* Setup material and texture for carpet */

   SETUP_MESH(carpet, carpet, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(carpet)
   MATERIAL_TEXTURE_SET(carpet, carpet, gazebo_top_path, gazebo_t_path)
   NORMAL_SET(carpet, carpet, gazebo_t_n_path)

   if (data->blending)
     {
        eo_do(data->texture_diffuse_carpet,
              evas_3d_texture_file_set(gazebo_t_trans_path, NULL));
        eo_do(data->mesh_carpet,
              evas_3d_mesh_blending_enable_set(EINA_TRUE),
              evas_3d_mesh_blending_func_set(EVAS_3D_BLEND_SRC_ALPHA, EVAS_3D_BLEND_ONE_MINUS_SRC_ALPHA));
     }

   eo_do(data->mesh_node_carpet,
         evas_3d_node_scale_set(0.18, 0.18, 0.18),
         evas_3d_node_position_set(-20, -10, -35));
   /* Setup material and texture for eagle. */

   SETUP_MESH(eagle, eagle, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(eagle)
   MATERIAL_TEXTURE_SET(eagle, eagle, eagle_path, eagle_tex_path)

   eo_do(data->mesh_node_eagle,
         evas_3d_node_scale_set(0.1, 0.1, 0.1),
         evas_3d_node_position_set(-20, 20, 0));
   /* Set material for bounding box */

   SETUP_MESH(cube, cube, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(cube)

   eo_do(data->mesh_cube,
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     36, &cube_indices[0]),
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_LINES));

   /* Setup mesh for column carpet */

   SETUP_MESH(column_c, column_c, 0.2, 0.8, 1.0)
   SETUP_MESH_NODE(column_c)
   TEXTCOORDS_SET(cube, 5.0, 0.1, 5.0, 0.1, 5.0, 0.4)
   CUBE_TEXTURE_SET(column_c, column_c, 24, cube_vertices, cube_textcoords, 36, cube_indices, red_brick_path)
   NORMAL_SET(column_c, column_c, red_brick_n_path)

   eo_do(data->mesh_node_column_c,
         evas_3d_node_orientation_angle_axis_set(90, 0, 1.0, 0.0),
         evas_3d_node_scale_set(48.0, 0.8 , 2.4 ),
         evas_3d_node_position_set(10 , 9.0, -12));

   /* Setup mesh for bounding sphere */
   data->material_ball = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   eo_do(data->material_ball,
          evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
          evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
          evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
          evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 1.0, 1.0, 1.0, 1.0),
          evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,  0.0, 5.0, 1.0, 1.0),
          evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
          evas_3d_material_shininess_set(50.0));

   data->mesh_ball = eo_add(EVAS_3D_MESH_CLASS, evas);

   _set_ball(data->mesh_ball, 10);

   eo_do(data->mesh_ball,
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_LINES),
         evas_3d_mesh_frame_material_set(0, data->material_ball));

   SETUP_MESH_NODE(ball)

   _scale(data->mesh_node_ball, 10.0);
}

void
_mesh_setup_wall(Scene_Data *data, int index)
{
   /* Setup material and texture for wall. */

   SETUP_MESH(wall, wall[index], 0.2, 0.8, 1.0)
   SETUP_MESH_NODE(wall[index])
   TEXTCOORDS_SET(wall, 12.0, 1.0, 0.2, 1.0, 12.0, 0.2)
   CUBE_TEXTURE_SET(wall, wall[index], 24, cube_vertices, wall_textcoords, 36, cube_indices, red_brick_path)
   NORMAL_SET(wall, wall[index], red_brick_n_path)

   /* placing of wall carpet on the floor grid */
   if (index == 0)
     eo_do(data->mesh_node_wall[index],
           evas_3d_node_scale_set(6.7 * 9.75, 10.0 , 1.0 ),
           evas_3d_node_position_set(-39.7 + 18.6 * 3, 0.0, -60.3));
   else if (index == 1)
     {
        eo_do(data->mesh_node_wall[index],
              evas_3d_node_scale_set(5 * 9.75, 10.0 , 1.0 ),
              evas_3d_node_position_set(81.2, 0.0, -51.5 + 19.5 * 2),
              evas_3d_node_orientation_angle_axis_set(90, 0, 1.0, 0.0));
     }
   else if (index == 2)
     eo_do(data->mesh_node_wall[index],
           evas_3d_node_scale_set(6.7 * 9.75, 10.0 , 1.0 ),
           evas_3d_node_position_set(-39.7 + 18.6*3, 0.0, 35.3));
   else if (index == 3)
     {
        eo_do(data->mesh_node_wall[index],
              evas_3d_node_scale_set(5* 9.75, 10.0 , 1.0 ),
              evas_3d_node_position_set(-48.4, 0.0, -51.5 + 19.5 * 2),
              evas_3d_node_orientation_angle_axis_set(90, 0, 1.0, 0.0));
     }
}

void
_mesh_setup_column(Scene_Data *data, int index)
{
   /* Setup mesh for column */

   data->material_column = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material_column,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.4, 0.4, 0.4, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,  0.6, 0.6, 0.6, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

   data->mesh_column[index] = eo_add(EVAS_3D_MESH_CLASS, evas);

   SETUP_MESH_NODE(column[index])

   eo_do(data->mesh_column[index],
         evas_3d_mesh_from_primitive_set(0, data->cylinder_primitive),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_frame_material_set(0, data->material_column),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->material_column));

   data->texture_diffuse_column = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(data->texture_diffuse_column,
         evas_3d_texture_file_set(red_brick_path, NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_column,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE,
                                      data->texture_diffuse_column),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_AMBIENT,
                                      data->texture_diffuse_column));

   NORMAL_SET(column, column[index], red_brick_n_path)

   eo_do(data->mesh_node_column[index],
         evas_3d_node_scale_set(2.2, 2.2, 19.0),
         evas_3d_node_position_set(10 , 0, -45 + 21.25 * index),
         evas_3d_node_orientation_angle_axis_set(90, 1.0, 0.0, 0.0));
}

void
_mesh_setup_grass(Scene_Data *data)
{
   /* Setup material and texture for grass. */

   SETUP_MESH(grass, grass, 0.4, 0.8, 1.0)
   SETUP_MESH_NODE(grass)
   CUBE_TEXTURE_SET(grass, grass, 4, grass_vertices, &grass_vertices[10],
                         6, grass_indices, gray_brick_path)
   NORMAL_SET(grass, grass, gray_brick_n_path)

   /* placing of grass carpet on the floor grid */
   eo_do(data->mesh_node_grass,
         evas_3d_node_scale_set(10.0, 1.0, 12.0),
         evas_3d_node_position_set(16, -10.0, -12));
}

void
_mesh_setup(Scene_Data *data, float h EINA_UNUSED, float w EINA_UNUSED, float d EINA_UNUSED, float *isource, int index)
{
   /* Setup mesh. */

   if (!index)
     {
        SETUP_MESH(level[0], level[index], 0.4, 0.8, 1.0)
        SETUP_MESH_NODE(level[index])
        CUBE_TEXTURE_SET(level[0], level[index], 24, cube_vertices, isource, 36, cube_indices, red_brick_path)
        NORMAL_SET(level[0], level[index], red_brick_n_path)

        eo_do(data->mesh_node_level[index],
              evas_3d_node_scale_set(9.75, 10.0, 1.0),
              evas_3d_node_position_set(50, 0, -15));
     }
   else
     {
        SETUP_MESH(level[1], level[index], 0.4, 0.8, 1.0)
        SETUP_MESH_NODE(level[index])
        CUBE_TEXTURE_SET(level[1], level[index], 24, cube_vertices,
                         isource, 36, cube_indices, gray_brick_path)
        NORMAL_SET(level[1], level[index], gray_brick_n_path)

        eo_do(data->mesh_node_level[index],
              evas_3d_node_scale_set(1.0, 1.0, 4.0),
              evas_3d_node_position_set(40 + (2 * index), -10 + (2 * index), -10));
     }
}

void
_mesh_setup_warrior(Scene_Data *data)
{
   /* Setup mesh for warrior */

   SETUP_MESH(warrior, warrior, 0.4, 0.6, 1.0)
   SETUP_MESH_NODE(warrior)
   MATERIAL_TEXTURE_SET(warrior, warrior, warrior_path, warrior_tex_path)

   eo_do(data->mesh_node_warrior,
         evas_3d_node_position_set(57, -10, 0),
         evas_3d_node_scale_set(0.15, 0.15, 0.15));
}

void
_mesh_setup_snake(Scene_Data *data)
{
   SETUP_MESH(snake, snake, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(snake)
   MATERIAL_TEXTURE_SET(snake, snake, snake_path, snake_tex_path)
   NORMAL_SET(snake, snake, snake_tex_n_path)

   eo_do(data->mesh_node_snake,
         evas_3d_node_position_set(20, -10, 20),
         evas_3d_node_scale_set(4.0, 4.0, 4.0));
}

void
_mesh_setup_rocket(Scene_Data *data, int index)
{
   SETUP_MESH(rocket[index], rocket[index], 0.8, 0.2, 0.5);
   SETUP_MESH_NODE(rocket[index]);

   SPHERE_SET(rocket[index])

   eo_do(data->mesh_node_rocket[index],
         evas_3d_node_scale_set(1, 1, 1),
         evas_3d_node_position_set(ROCKET_POS_X, ROCKET_POS_Y, ROCKET_POS_Z));
}

void
_scene_setup(Scene_Data *data)
{
   int i = 0;
   global_scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   eo_do(global_scene,
         evas_3d_scene_size_set(WIDTH, HEIGHT),
         evas_3d_scene_background_color_set(0.5, 0.5, 0.9, 0.0));

   data->root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                   evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup_gun_planet(data);
   _mesh_setup_snake(data);

   for (i = 0; i < 10; i++)
     _mesh_setup_rocket(data, i);

   data->cylinder_primitive = eo_add(EVAS_3D_PRIMITIVE_CLASS, evas);
   eo_do(data->cylinder_primitive,
         evas_3d_primitive_mode_set(EVAS_3D_PRIMITIVE_MODE_WITHOUT_BASE),
         evas_3d_primitive_form_set(EVAS_3D_MESH_PRIMITIVE_CYLINDER),
         evas_3d_primitive_tex_scale_set(1.0, 1.0),
         evas_3d_primitive_precision_set(50));

   for ( i = 0; i < 4; i++)
     _mesh_setup_column(data, i);

   _mesh_setup_grass(data);

   for ( i = 0; i < 4; i++)
     _mesh_setup_wall(data, i);

   TEXTCOORDS_SET(bricks1, 1.0, 1.0, 0.2, 1.0, 1.0, 0.2)
   _mesh_setup(data, 9.75, 10, 1, bricks1_textcoords, 0);

   TEXTCOORDS_SET(bricks2, 1.0, 0.5, 3.0, 0.5, 2.0, 1.5)
   for (i = 1; i < 10; i++)
     _mesh_setup(data, 1, 1, 4, bricks2_textcoords, i);

   _mesh_setup_warrior(data);

   _mesh_aabb(&data->mesh_cube, data, aabb_index);

   eo_do(global_scene,
         evas_3d_scene_root_node_set(data->root_node),
         evas_3d_scene_camera_node_set(data->camera_node));

   if (data->shadow)
     eo_do(global_scene,evas_3d_scene_shadows_enable_set(EINA_TRUE));

   if (data->fog)
     {
        ENABLE_FOG(data->mesh_soldier)
        ENABLE_FOG(data->mesh_warrior)
        ENABLE_FOG(data->mesh_tommy)
        ENABLE_FOG(data->mesh_gazebo)
        ENABLE_FOG(data->mesh_carpet)
        ENABLE_FOG(data->mesh_eagle)
        ENABLE_FOG(data->mesh_world)
        ENABLE_FOG(data->mesh_snake)
        ENABLE_FOG(data->mesh_column_c)
        ENABLE_FOG(data->mesh_grass)
        for (i = 0; i < 4; i++)
          {
             ENABLE_FOG(data->mesh_wall[i])
             ENABLE_FOG(data->mesh_column[i])
          }
        for (i = 0; i < 10; i++)
          {
             ENABLE_FOG(data->mesh_level[i])
             ENABLE_FOG(data->mesh_rocket[i])
          }
     }

   data->carp_mediator_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                         evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->carp_mediator_node,
         evas_3d_node_member_add(data->mesh_node_carpet));

   eo_do(data->mediator_node,
         evas_3d_node_member_add(data->mesh_node_soldier),
         evas_3d_node_member_add(data->mesh_node_tommy));

   for (i = 0; i < 10; i++)
        eo_do(data->mediator_node,
              evas_3d_node_member_add(data->mesh_node_rocket[i]));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->carp_mediator_node),
         evas_3d_node_member_add(data->mesh_node_world),
         evas_3d_node_member_add(data->mesh_node_gazebo),
         evas_3d_node_member_add(data->mesh_node_eagle),
         evas_3d_node_member_add(data->mesh_node_cube),
         evas_3d_node_member_add(data->mesh_node_column_c),
         evas_3d_node_member_add(data->mesh_node_ball),
         evas_3d_node_member_add(data->mesh_node_grass),
         evas_3d_node_member_add(data->mesh_node_warrior),
         evas_3d_node_member_add(data->mesh_node_snake));

   for (i = 0; i < 4; i++)
        eo_do(data->root_node,
              evas_3d_node_member_add(data->mesh_node_wall[i]),
              evas_3d_node_member_add(data->mesh_node_column[i]));

   for (i = 0; i < 10; i++)
        eo_do(data->root_node,
              evas_3d_node_member_add(data->mesh_node_level[i]));
}

int
main(int argc, char *argv[])
{
   Scene_Data data;

   Eina_Bool shadow = EINA_FALSE;
   Eina_Bool colorpick = EINA_FALSE;
   Eina_Bool fog = EINA_FALSE;
   Eina_Bool blending = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
       ECORE_GETOPT_VALUE_BOOL(shadow),
       ECORE_GETOPT_VALUE_BOOL(colorpick),
       ECORE_GETOPT_VALUE_BOOL(fog),
       ECORE_GETOPT_VALUE_BOOL(blending),
       ECORE_GETOPT_VALUE_NONE
   };

   ecore_getopt_parse(&optdesc, values, argc, argv);

   data.shadow = shadow;
   data.colorpick = colorpick;
   data.fog = fog;
   data.blending = blending;
   data.normal = EINA_FALSE;

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new("opengl_x11", 0, 0, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   /* Add a background rectangle objects. */
   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   _scene_setup(&data);

   /* Add an image object for 3D scene rendering. */
   image = evas_object_image_filled_add(evas);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   /* Set the image object as render target for 3D scene. */
   eo_do(image,
         evas_obj_image_scene_set(global_scene));

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
