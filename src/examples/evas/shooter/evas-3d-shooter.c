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

#define LOCAL_IMAGE_FOLDER PACKAGE_EXAMPLES_DIR "" EVAS_PROJECT_IMAGE_FOLDER
#define LOCAL_MODEL_FOLDER PACKAGE_EXAMPLES_DIR "" EVAS_PROJECT_MODEL_FOLDER

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
static const char *gun_path = LOCAL_MODEL_FOLDER "/tommy.ply";

Evas_Canvas3D_Scene    *global_scene      = NULL;
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
int              motion_vec[4];

typedef struct _Ecore_Timers
{
   Ecore_Timer *t;
   void *data;
} Timers;

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

static void
_key_down(void *data,
          Evas *e EINA_UNUSED,
          Evas_Object *eo EINA_UNUSED,
          void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Real x, y, z;
   Scene_Data *scene = (Scene_Data *)data;
   Eina_Bool pressed = EINA_FALSE;
   int i = 0;
   if (!strcmp(ev->key, "space"))
     {
        if (space_move == 0.0)
          {
             evas_canvas3d_node_position_get(scene->mesh_node_tommy, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
             old_space_position = y;
             space_move = JUMP;
          }
     }
   else if (!strcmp(ev->key, "s"))
     KEY_MOTION(1)
   else if (!strcmp(ev->key, "w"))
     KEY_MOTION(2)
   else if (!strcmp(ev->key, "d"))
     KEY_MOTION(3)
   else if (!strcmp(ev->key, "a"))
     KEY_MOTION(4)
   else if (!strcmp(ev->key, "B"))
     {
        if (!scene->blending)
          {
             scene->blending = EINA_TRUE;
             efl_file_simple_load(scene->texture_diffuse_carpet, gazebo_t_trans_path, NULL);
             evas_canvas3d_mesh_blending_enable_set(scene->mesh_carpet, EINA_TRUE);
             evas_canvas3d_mesh_blending_func_set(scene->mesh_carpet, EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA, EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
          }
        else
          {
             scene->blending = EINA_FALSE;
             efl_file_simple_load(scene->texture_diffuse_carpet, gazebo_t_path, NULL);
             evas_canvas3d_mesh_blending_enable_set(scene->mesh_carpet, EINA_FALSE);
          }
     }
   else if (!strcmp(ev->key, "S"))
     {
        if (!scene->blending)
          {
             scene->blending = EINA_TRUE;
             evas_canvas3d_scene_shadows_enable_set(global_scene, EINA_TRUE);
          }
        else
          {
             scene->blending = EINA_FALSE;
             evas_canvas3d_scene_shadows_enable_set(global_scene, EINA_FALSE);
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
     {
        evas_canvas3d_node_position_set(scene->camera_node, 0.0, 80.0, 30.0);
        evas_canvas3d_node_look_at_set(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
     }
   else if (!strcmp(ev->key, "F2"))
     {
        evas_canvas3d_node_position_set(scene->camera_node, -2.0, 0.0, 4.0);
     evas_canvas3d_node_look_at_set(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, -100.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
     }
   else if (!strcmp(ev->key, "A"))
     {
        aabb_index++;
     }
}

static void
_key_up(void *data EINA_UNUSED,
        Evas *e EINA_UNUSED,
        Evas_Object *eo EINA_UNUSED,
        void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   int i = 0, j;

   if (!strcmp(ev->key, "space"))
     space_move = 0;
   else if (!strcmp(ev->key, "s"))
     KEY_MOTION_DOWN(1)
   else if (!strcmp(ev->key, "w"))
     KEY_MOTION_DOWN(2)
   else if (!strcmp(ev->key, "d"))
     KEY_MOTION_DOWN(3)
   else if (!strcmp(ev->key, "a"))
     KEY_MOTION_DOWN(4)

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

   evas_canvas3d_node_position_get(scene->mediator_node, EVAS_CANVAS3D_SPACE_WORLD, &x, &y, &z);

   evas_canvas3d_node_position_get(scene->mesh_node_world, EVAS_CANVAS3D_SPACE_PARENT, &x1, &y1, &z1);

   evas_canvas3d_node_position_get(scene->mesh_node_tommy, EVAS_CANVAS3D_SPACE_PARENT, &x2, &y2, &z2);

   if (fall_speed != 0)
     {
        if (y - fall_speed < 0)
          {
             evas_canvas3d_node_position_set(scene->mediator_node, x, 0, z);
             fall_speed = 0.0;
          }
        else
          {
             evas_canvas3d_node_position_set(scene->mediator_node, x, y - fall_speed, z);
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
             evas_canvas3d_node_position_set(scene->mediator_node, x + x_move, y, z + z_move);
             evas_canvas3d_node_position_set(scene->mesh_node_world, x1 + x_move, y1, z1 + z_move);
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
             evas_canvas3d_node_position_set(scene->mediator_node, x + x_move_lateral, y, z + z_move_lateral);
             evas_canvas3d_node_position_set(scene->mesh_node_world, x1 + x_move_lateral, y1, z1 + z_move_lateral);
          }
     }

   evas_canvas3d_node_position_get(scene->mediator_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);

   if (!_horizontal_position_resolution(scene->mesh_node_level[step_indicator+1], x, z))
     {
        evas_canvas3d_node_position_set(scene->mediator_node, x, y + STEP, z);
        old_space_position += STEP;
        step_indicator++;
     }
   else if (!_horizontal_position_resolution(scene->mesh_node_level[step_indicator-1], x, z) && step_indicator > 0)
     {
        evas_canvas3d_node_position_set(scene->mediator_node, x, y - STEP, z);
        old_space_position -= STEP;
        step_indicator--;
     }
   else if (_horizontal_position_resolution(scene->mesh_node_level[step_indicator], x, z) && (step_indicator > 0))
     {
        fall_speed += FALL_SPEED_UP;
        step_indicator = 0;
     }

   evas_canvas3d_node_orientation_angle_axis_set(scene->mediator_node, angle_camera, 0, 1.0, 0.0);
   evas_canvas3d_node_position_get(scene->mediator_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);

   evas_canvas3d_node_position_get(scene->mesh_node_warrior, EVAS_CANVAS3D_SPACE_PARENT, &x1, &y1, &z1);
   evas_canvas3d_node_position_get(scene->mesh_node_snake, EVAS_CANVAS3D_SPACE_PARENT, &x2, &y2, &z2);

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
           evas_canvas3d_node_position_set(scene->mesh_node_warrior, (x - x1) * WARRIOR_SPEED + x1, y1, (z - z1) * WARRIOR_SPEED + z1);
        else
          evas_canvas3d_node_position_set(scene->mesh_node_warrior, (z - z1) * WARRIOR_SPEED2 + x1, y1, -(x - x1) * WARRIOR_SPEED2 + z1);
     }

   /* if soldier is looking at the snake */
   if (looking < 0)
     {
        /* if snake is located at the left hand of the soldier */
        if (snake_x * cos(angle_camera * TO_RADIAN) - snake_z * sin(angle_camera* TO_RADIAN) < 0)
          {
             if ((_horizontal_object_resolution(scene->mesh_node_snake, SNAKE_FRAME_X1, SNAKE_FRAME_Z1,
                                                SNAKE_FRAME_X2, SNAKE_FRAME_Z2, snake_z * SNAKE_SPEED, -snake_x * SNAKE_SPEED)))
               evas_canvas3d_node_position_set(scene->mesh_node_snake, snake_z * SNAKE_SPEED + x2, y2, -snake_x * SNAKE_SPEED + z2);
             else if (_horizontal_circle_resolution(scene->mesh_node_snake, (x2 - x) * SNAKE_SPEED2 + x,
                                                    (z2 - z) * SNAKE_SPEED2 + z, SOLDIER_RADIUS)) 
               evas_canvas3d_node_position_set(scene->mesh_node_snake, (x - x2) * SNAKE_SPEED2 + x2, y2, (z - z2) * SNAKE_SPEED2 + z2);
          }
        /* if snake is located at the right hand of the soldier */
        else
          {
             if ((_horizontal_object_resolution(scene->mesh_node_snake, SNAKE_FRAME_X1, SNAKE_FRAME_Z1,
                                                SNAKE_FRAME_X2, SNAKE_FRAME_Z2, -snake_z * SNAKE_SPEED, snake_x * SNAKE_SPEED)))
               evas_canvas3d_node_position_set(scene->mesh_node_snake, -snake_z * SNAKE_SPEED + x2, y2, snake_x * SNAKE_SPEED + z2);
             else if (_horizontal_circle_resolution(scene->mesh_node_snake, (x2 - x) * SNAKE_SPEED2 + x,
                                                    (z2 - z) * SNAKE_SPEED2 + z, SOLDIER_RADIUS))
               evas_canvas3d_node_position_set(scene->mesh_node_snake, (x - x2) * SNAKE_SPEED2 + x2, y2, (z - z2) * SNAKE_SPEED2 + z2);
          }
     }
   else if (_horizontal_circle_resolution(scene->mesh_node_snake, (x2 - x) * SNAKE_SPEED2 + x, (z2 - z) * SNAKE_SPEED2 + z, SOLDIER_RADIUS))
     evas_canvas3d_node_position_set(scene->mesh_node_snake, (x - x2) * SNAKE_SPEED2 + x2, y2, (z - z2) * SNAKE_SPEED2 + z2);

   evas_canvas3d_node_position_get(scene->mesh_node_snake, EVAS_CANVAS3D_SPACE_PARENT, &x2, &y2, &z2);

   evas_canvas3d_node_look_at_set(scene->mesh_node_warrior, EVAS_CANVAS3D_SPACE_PARENT, 0.0, -1000.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, (z - z1) / _distance(x, z, x1, z1), 0, (x1 - x) / _distance(x, z, x1, z1));

   evas_canvas3d_node_look_at_set(scene->mesh_node_snake, EVAS_CANVAS3D_SPACE_PARENT, 2 * x2 - x, y2, 2 * z2 - z, EVAS_CANVAS3D_SPACE_PARENT, 0, 1, 0);

   if (!_horizontal_frame_resolution(scene->mesh_node_snake, SNAKE_FRAME_X1, SNAKE_FRAME_Z1,
                                                SNAKE_FRAME_X2, SNAKE_FRAME_Z2, &x_move, &z_move))
     evas_canvas3d_node_position_set(scene->mesh_node_snake, x2 + x_move, y2, z2 + z_move);

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

   frame_eagle = evas_canvas3d_node_mesh_frame_get(scene->mesh_node_eagle, scene->mesh_eagle);
   if (frame_eagle > 19500) frame_eagle = 0;
   frame_eagle += 150;

   frame_snake = evas_canvas3d_node_mesh_frame_get(scene->mesh_node_snake, scene->mesh_snake);
   if (frame_snake > 16000) frame_snake = 0;
   frame_snake += 64;

   evas_canvas3d_node_orientation_angle_axis_set(scene->mesh_node_eagle, angle_eagle, 0.0, 1.0, 0.0);
   /* equation of eagle flight */
         evas_canvas3d_node_position_set(scene->mesh_node_eagle, 38 * sin((angle_eagle + 90) * TO_RADIAN), 20 + 20 * sin(angle_eagle * TO_RADIAN), -10 + 20.0 * cos((angle_eagle + 90) * TO_RADIAN));
   evas_canvas3d_node_mesh_frame_set(scene->mesh_node_eagle, scene->mesh_eagle, frame_eagle);

   for (i = 0; i < 10; i ++)
     {
        if (rocket_indicator[i] == 3)
          rocket_indicator[i] = 0;
        else if (rocket_indicator[i] == 0)
          /* almost hidden rocket */
          _scale(scene->mesh_node_rocket[i], 0.004);
        else if (rocket_indicator[i] == 1)
          {
             _scale(scene->mesh_node_rocket[i], 0.4);
             evas_canvas3d_material_color_set(scene->material_rocket[i], EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.8, 0.8, 0.8, 1.0);
             evas_canvas3d_material_color_set(scene->material_rocket[i], EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.2, 0.2, 0.0, 1.0);
             evas_canvas3d_material_color_set(scene->material_rocket[i], EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 0.5, 0.5, 0.5, 1.0);

             rocket_indicator[i] = 2;

             evas_canvas3d_node_position_set(scene->mesh_node_rocket[i], ROCKET_POS_X, ROCKET_POS_Y, ROCKET_POS_Z);
             rocket_parametr[i] = 0;
          }
        else if (rocket_indicator[i] == 2)
          {
             rocket_parametr[i] += ROCKET_SPEED;
             /* equation of rocket flight */

             evas_canvas3d_node_position_get(scene->mesh_node_rocket[i], EVAS_CANVAS3D_SPACE_WORLD, &x, &y, &z);

             if ((ROCKET_FRAME_X1 < x) && (x < ROCKET_FRAME_X2) && (ROCKET_FRAME_Z1 < z) && (z < ROCKET_FRAME_Z2))
                evas_canvas3d_node_position_set(scene->mesh_node_rocket[i], ROCKET_POS_X, ROCKET_POS_Y, ROCKET_POS_Z - rocket_parametr[i]);
             else
               rocket_indicator[i] = 3;

             /* the way to stop the rocket */
             evas_canvas3d_node_bounding_box_get(scene->mesh_node_warrior, &x, &y, &z, &x1, &y1, &z1);
             evas_box3_set(&boxy, x, y, z, x1, y1, z1);

             evas_canvas3d_node_position_get(scene->mesh_node_rocket[i], EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);

             evas_canvas3d_node_position_get(scene->mediator_node, EVAS_CANVAS3D_SPACE_PARENT, &x1, &y1, &z1);

             if (((z * sinus + x * cosinus + x1) >= boxy.p0.x) && ((z * sinus + x * cosinus + x1) <= boxy.p1.x) &&
                 ((z * cosinus - x * sinus + z1) >= boxy.p0.z) && ((z * cosinus - x * sinus + z1) <= boxy.p1.z))
               {
                  evas_canvas3d_material_color_set(scene->material_rocket[i], EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.8, 0.8, 0.8, 1.0);
                  evas_canvas3d_material_color_set(scene->material_rocket[i], EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.9, 0.1, 0.1, 1.0);
                  evas_canvas3d_material_color_set(scene->material_rocket[i], EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 0.5, 0.5, 0.5, 1.0);

                  _scale(scene->mesh_node_rocket[i], 4.5);
                  rocket_indicator[i] = 3;
               }
          }
     }

   if (tommy_indicator == 1)
     {
        tommy_indicator = 2;
        evas_canvas3d_node_position_set(scene->mesh_node_tommy, TOMMY_POS_X, TOMMY_POS_Y, TOMMY_POS_Z);
        tommy_parametr = 0.0;
     }
   /* parametric equations of gun movement, (GUN_DIR_X, GUN_DIR_Y, GUN_DIR_Z) is direction vector */
   else if (tommy_indicator == 2)
     {
        tommy_parametr += GUN_SPEED;
        evas_canvas3d_node_position_set(scene->mesh_node_tommy, TOMMY_POS_X + GUN_DIR_X * tommy_parametr, TOMMY_POS_Y + GUN_DIR_Y * tommy_parametr, TOMMY_POS_Z + GUN_DIR_Z * tommy_parametr);

        if (TOMMY_POS_Z + GUN_DIR_Z * tommy_parametr >= -8.3)
          tommy_indicator = 3;
     }
   else if (tommy_indicator == 3)
     {
        tommy_parametr-= GUN_SPEED / 2;
        evas_canvas3d_node_position_set(scene->mesh_node_tommy, TOMMY_POS_X + GUN_DIR_X * tommy_parametr, TOMMY_POS_Y + GUN_DIR_Y * tommy_parametr, TOMMY_POS_Z + GUN_DIR_Z * tommy_parametr);

        evas_canvas3d_node_position_get(scene->mesh_node_tommy, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        if (z <= TOMMY_POS_Z)
          {
             tommy_indicator = 4;
             tommy_parametr = 0;
          }
     }

   if (angle_eagle > 360.0) angle -= 360.0f;

   evas_canvas3d_node_mesh_frame_set(scene->mesh_node_warrior, scene->mesh_warrior, frame);

   evas_canvas3d_node_mesh_frame_set(scene->mesh_node_snake, scene->mesh_snake, frame_snake);

   if (space_move && (soldier_mode == 0))
     {
        evas_canvas3d_node_mesh_add(scene->mesh_node_soldier, scene->mesh_soldier_jump);
        evas_canvas3d_node_mesh_del(scene->mesh_node_soldier, scene->mesh_soldier);
        evas_canvas3d_node_mesh_frame_set(scene->mesh_node_soldier, scene->mesh_soldier_jump, 0);
        soldier_mode = 1;
        evas_canvas3d_node_position_set(scene->mesh_node_tommy, TOMMY_JUMP_X, TOMMY_JUMP_Y, TOMMY_JUMP_Z);
        evas_canvas3d_node_look_at_set(scene->mesh_node_tommy, EVAS_CANVAS3D_SPACE_PARENT, 1000.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0, 0, 1);
     }
   else if (soldier_mode == 1)
     {
        frame_soldier = evas_canvas3d_node_mesh_frame_get(scene->mesh_node_soldier, scene->mesh_soldier_jump);
        frame_soldier += 124;
        evas_canvas3d_node_mesh_frame_set(scene->mesh_node_soldier, scene->mesh_soldier_jump, frame_soldier);
        if (frame_soldier < 2480)
          evas_canvas3d_node_position_set(scene->mesh_node_tommy, TOMMY_JUMP_X, TOMMY_JUMP_Y + 4.5 * frame_soldier / 2480, TOMMY_JUMP_Z);
        else
          evas_canvas3d_node_position_set(scene->mesh_node_tommy, TOMMY_JUMP_X, TOMMY_JUMP_Y + 4.5 * (4960 - frame_soldier) / 2480, TOMMY_JUMP_Z);
        if (frame_soldier > 4960)
          {
             evas_canvas3d_node_mesh_del(scene->mesh_node_soldier, scene->mesh_soldier_jump);
             evas_canvas3d_node_mesh_add(scene->mesh_node_soldier, scene->mesh_soldier);
             evas_canvas3d_node_mesh_frame_set(scene->mesh_node_soldier, scene->mesh_soldier, 0);
             soldier_mode = 0;
             evas_canvas3d_node_position_set(scene->mesh_node_tommy, TOMMY_POS_X, TOMMY_POS_Y, TOMMY_POS_Z);
             evas_canvas3d_node_look_at_set(scene->mesh_node_tommy, EVAS_CANVAS3D_SPACE_PARENT, 0.0, -1000.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 2, 0, 1.5);
          }
     }
   else if (camera_move)
     {
        frame_soldier = evas_canvas3d_node_mesh_frame_get(scene->mesh_node_soldier, scene->mesh_soldier);
        frame_soldier += 124;
        if (frame_soldier > 6200) frame_soldier = 0;
        evas_canvas3d_node_mesh_frame_set(scene->mesh_node_soldier, scene->mesh_soldier, frame_soldier);
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
   Evas_Canvas3D_Node *n;
   Evas_Canvas3D_Mesh *m;
   Evas_Real s, t;

   evas_canvas3d_scene_pick(global_scene, ev->canvas.x, ev->canvas.y, &n, &m, &s, &t);

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
_mesh_setup_gun_planet(Scene_Data *data)
{
   /* Setup material and texture for world. */

   ADD_MESH(world, world, 0.5, 0.5, 1.0);
   evas_canvas3d_mesh_from_primitive_set(data->mesh_world, 0, data->sphere_primitive);
   SETUP_DEFAULT_MESH(world, world, DIFFUSE);
   SETUP_MESH_NODE(world);

   evas_canvas3d_node_scale_set(data->mesh_node_world, 350, 350, 350);
   evas_canvas3d_node_position_set(data->mesh_node_world, 0.0, 0.0, 0.0);

   /* Setup material and texture for player */

   ADD_MESH(soldier, soldier, 0.2, 0.6, 1.0)
   MATERIAL_TEXTURE_SET(soldier, soldier, soldier_path, soldier_tex_path)

   SETUP_MESH_NODE(soldier)

   evas_canvas3d_node_position_set(data->mesh_node_soldier, 0, -10, 0);
   evas_canvas3d_node_scale_set(data->mesh_node_soldier, 5, 5, 5);

   evas_canvas3d_node_look_at_set(data->mesh_node_soldier, EVAS_CANVAS3D_SPACE_PARENT, 0.0, -10.0, -100.0, EVAS_CANVAS3D_SPACE_PARENT, 0, 1, 0);
   evas_canvas3d_node_mesh_frame_set(data->mesh_node_soldier, data->mesh_soldier, 0);

   ADD_MESH(soldier, soldier_jump, 1.0, 1.0, 1.0)
   MATERIAL_TEXTURE_SET(soldier, soldier_jump, soldier_jump_path, soldier_tex_path)

   /* Setup material and texture for tommy gun */

   ADD_MESH(tommy, tommy, 0.0, 0.3, 1.0)
   SETUP_MESH_NODE(tommy)
   efl_file_simple_load(data->mesh_tommy, gun_path, NULL);
   efl_file_save(data->mesh_tommy, "try.obj", NULL, NULL);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_tommy, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh_tommy, 0, data->material_tommy);

   evas_canvas3d_node_scale_set(data->mesh_node_tommy, 1.5, 1.5, 1.5);
   evas_canvas3d_node_position_set(data->mesh_node_tommy, TOMMY_POS_X, TOMMY_POS_Y, TOMMY_POS_Z);
   evas_canvas3d_node_look_at_set(data->mesh_node_tommy, EVAS_CANVAS3D_SPACE_PARENT, 0.0, -1000.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 2, 0, 1.5);

   /* Setup material and texture for gazebo */

   ADD_MESH(gazebo, gazebo, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(gazebo)
   MATERIAL_TEXTURE_SET(gazebo, gazebo, gazebo_bot_path, gazebo_b_path)
   NORMAL_SET(gazebo, gazebo, gazebo_b_n_path)

   evas_canvas3d_node_scale_set(data->mesh_node_gazebo, 0.18, 0.18, 0.18);
   evas_canvas3d_node_orientation_angle_axis_set(data->mesh_node_gazebo, 180, 0.0, 1.0, 0.0);
   evas_canvas3d_node_position_set(data->mesh_node_gazebo, -20, -10, -35);

   /* Setup material and texture for carpet */

   ADD_MESH(carpet, carpet, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(carpet)
   MATERIAL_TEXTURE_SET(carpet, carpet, gazebo_top_path, gazebo_t_path)
   NORMAL_SET(carpet, carpet, gazebo_t_n_path)

   if (data->blending)
     {
        evas_canvas3d_texture_atlas_enable_set(data->texture_diffuse_carpet, EINA_FALSE);
        efl_file_simple_load(data->texture_diffuse_carpet, gazebo_t_trans_path, NULL);
        evas_canvas3d_mesh_blending_enable_set(data->mesh_carpet, EINA_TRUE);
        evas_canvas3d_mesh_blending_func_set(data->mesh_carpet, EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA, EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
     }

   evas_canvas3d_node_scale_set(data->mesh_node_carpet, 0.18, 0.18, 0.18);
   evas_canvas3d_node_position_set(data->mesh_node_carpet, -20, -10, -35);
   /* Setup material and texture for eagle. */

   ADD_MESH(eagle, eagle, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(eagle)
   MATERIAL_TEXTURE_SET(eagle, eagle, eagle_path, eagle_tex_path)

   evas_canvas3d_node_scale_set(data->mesh_node_eagle, 0.1, 0.1, 0.1);
   evas_canvas3d_node_position_set(data->mesh_node_eagle, -20, 20, 0);
   /* Set material for bounding box */

   ADD_MESH(cube, cube, 0.01, 1.0, 1.0)
   evas_canvas3d_mesh_frame_add(data->mesh_cube, 0);
   SETUP_DEFAULT_MESH(cube, cube, DIFFUSE);
   SETUP_MESH_NODE(cube)

   evas_canvas3d_mesh_index_data_set(data->mesh_cube, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]);
   evas_canvas3d_mesh_vertex_count_set(data->mesh_cube, 24);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_cube, EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES);

   /* Setup mesh for column carpet */

   ADD_MESH(column_c, column_c, 0.2, 0.8, 1.0)
   SETUP_MESH_NODE(column_c)
   TEXTCOORDS_SET(cube, 5.0, 0.4, 5.0, 0.1, 5.0, 0.1)
   CUBE_TEXTURE_SET(column_c, column_c, cube_textcoords, red_brick_path)
   NORMAL_SET(column_c, column_c, red_brick_n_path)

   evas_canvas3d_node_orientation_angle_axis_set(data->mesh_node_column_c, 90, 0, 1.0, 0.0);
   evas_canvas3d_node_scale_set(data->mesh_node_column_c, 96.0, 1.6, 4.8);
   evas_canvas3d_node_position_set(data->mesh_node_column_c, 10, 9.0, -12);

   /* Setup mesh for bounding sphere */
   data->material_ball = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   evas_canvas3d_material_enable_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.0, 5.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material_ball, 50.0);

   data->mesh_ball = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);

   _set_ball(data->mesh_ball, 10);

   evas_canvas3d_mesh_shader_mode_set(data->mesh_ball, EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_ball, EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES);
   evas_canvas3d_mesh_frame_material_set(data->mesh_ball, 0, data->material_ball);

   SETUP_MESH_NODE(ball)

   _scale(data->mesh_node_ball, 10.0);
}

void
_mesh_setup_wall(Scene_Data *data, int index)
{
   /* Setup material and texture for wall. */

   ADD_MESH(wall, wall[index], 0.2, 0.8, 1.0)
   SETUP_MESH_NODE(wall[index])
   TEXTCOORDS_SET(wall, 12.0, 0.2, 0.2, 1.0, 12.0, 1.0)
   CUBE_TEXTURE_SET(wall, wall[index], wall_textcoords, red_brick_path)
   NORMAL_SET(wall, wall[index], red_brick_n_path)

   /* placing of wall carpet on the floor grid */
   if (index == 0)
     {
        evas_canvas3d_node_scale_set(data->mesh_node_wall[index], 6.7 * 19.5, 20.0, 2.0);
        evas_canvas3d_node_position_set(data->mesh_node_wall[index], -39.7 + 18.6 * 3, 0.0, -60.3);
     }
   else if (index == 1)
     {
        evas_canvas3d_node_scale_set(data->mesh_node_wall[index], 5 * 19.5, 20.0, 2.0);
        evas_canvas3d_node_position_set(data->mesh_node_wall[index], 81.2, 0.0, -51.5 + 19.5 * 2);
        evas_canvas3d_node_orientation_angle_axis_set(data->mesh_node_wall[index], 90, 0, 1.0, 0.0);
     }
   else if (index == 2)
     {
        evas_canvas3d_node_scale_set(data->mesh_node_wall[index], 6.7 * 19.5, 20.0, 2.0);
        evas_canvas3d_node_position_set(data->mesh_node_wall[index], -39.7 + 18.6*3, 0.0, 35.3);
     }
   else if (index == 3)
     {
        evas_canvas3d_node_scale_set(data->mesh_node_wall[index], 5* 19.5, 20.0, 2.0);
        evas_canvas3d_node_position_set(data->mesh_node_wall[index], -48.4, 0.0, -51.5 + 19.5 * 2);
        evas_canvas3d_node_orientation_angle_axis_set(data->mesh_node_wall[index], 90, 0, 1.0, 0.0);
     }
}

void
_mesh_setup_column(Scene_Data *data, int index)
{
   /* Setup mesh for column */

   data->material_column = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_enable_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.4, 0.4, 0.4, 1.0);
   evas_canvas3d_material_color_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.6, 0.6, 0.6, 1.0);
   evas_canvas3d_material_color_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material_column, 50.0);

   data->mesh_column[index] = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);

   SETUP_MESH_NODE(column[index])

   evas_canvas3d_mesh_from_primitive_set(data->mesh_column[index], 0, data->cylinder_primitive);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_column[index], EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_column[index], EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_column[index], EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh_column[index], 0, data->material_column);

   data->texture_diffuse_column = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);

   efl_file_simple_load(data->texture_diffuse_column, red_brick_path, NULL);
   evas_canvas3d_texture_atlas_enable_set(data->texture_diffuse_column, EINA_FALSE);
   evas_canvas3d_texture_filter_set(data->texture_diffuse_column, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(data->texture_diffuse_column, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);

   evas_canvas3d_material_texture_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, data->texture_diffuse_column);
   evas_canvas3d_material_texture_set(data->material_column, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, data->texture_diffuse_column);

   NORMAL_SET(column, column[index], red_brick_n_path)

   evas_canvas3d_node_scale_set(data->mesh_node_column[index], 2.2, 2.2, 19.0);
   evas_canvas3d_node_position_set(data->mesh_node_column[index], 10, 0, -45 + 21.25 * index);
   evas_canvas3d_node_orientation_angle_axis_set(data->mesh_node_column[index], 90, 1.0, 0.0, 0.0);
}

void
_mesh_setup_grass(Scene_Data *data)
{
   /* Setup material and texture for grass. */

   ADD_MESH(grass, grass, 0.4, 0.8, 1.0)
   SETUP_MESH_NODE(grass)
   TEXTCOORDS_SET(grass, 6.0, 3.0, 1.0, 1.0, 1.0, 1.0)
   CUBE_TEXTURE_SET(grass, grass, grass_textcoords, gray_brick_path)
   NORMAL_SET(grass, grass, gray_brick_n_path)

   /* placing of grass carpet on the floor grid */
   evas_canvas3d_node_scale_set(data->mesh_node_grass, 130.0, 1.0, 96.0);
   evas_canvas3d_node_position_set(data->mesh_node_grass, 16.5, -10.0, -12);
}

void
_mesh_setup(Scene_Data *data, float h EINA_UNUSED, float w EINA_UNUSED, float d EINA_UNUSED, float *isource, int index)
{
   /* Setup mesh. */

   if (!index)
     {
        ADD_MESH(level[0], level[index], 0.4, 0.8, 1.0)
        SETUP_MESH_NODE(level[index])
        CUBE_TEXTURE_SET(level[0], level[index], isource, red_brick_path)
        NORMAL_SET(level[0], level[index], red_brick_n_path)

        evas_canvas3d_node_scale_set(data->mesh_node_level[index], 19.5, 20.0, 2.0);
        evas_canvas3d_node_position_set(data->mesh_node_level[index], 50, 0, -15);
     }
   else
     {
        ADD_MESH(level[1], level[index], 0.4, 0.8, 1.0)
        SETUP_MESH_NODE(level[index])
        CUBE_TEXTURE_SET(level[1], level[index], isource, gray_brick_path)
        NORMAL_SET(level[1], level[index], gray_brick_n_path)

        evas_canvas3d_node_scale_set(data->mesh_node_level[index], 2.0, 2.0, 8.0);
        evas_canvas3d_node_position_set(data->mesh_node_level[index], 40 + (2 * index), -10 + (2 * index), -10);
     }
}

void
_mesh_setup_warrior(Scene_Data *data)
{
   /* Setup mesh for warrior */

   ADD_MESH(warrior, warrior, 0.4, 0.6, 1.0)
   SETUP_MESH_NODE(warrior)
   MATERIAL_TEXTURE_SET(warrior, warrior, warrior_path, warrior_tex_path)

   evas_canvas3d_node_position_set(data->mesh_node_warrior, 57, -10, 0);
   evas_canvas3d_node_scale_set(data->mesh_node_warrior, 0.15, 0.15, 0.15);
}

void
_mesh_setup_snake(Scene_Data *data)
{
   ADD_MESH(snake, snake, 0.01, 1.0, 1.0)
   SETUP_MESH_NODE(snake)
   MATERIAL_TEXTURE_SET(snake, snake, snake_path, snake_tex_path)
   NORMAL_SET(snake, snake, snake_tex_n_path)

   evas_canvas3d_node_position_set(data->mesh_node_snake, 20, -10, 20);
   evas_canvas3d_node_scale_set(data->mesh_node_snake, 4.0, 4.0, 4.0);
}

void
_mesh_setup_rocket(Scene_Data *data, int index)
{
   ADD_MESH(rocket[index], rocket[index], 0.8, 0.2, 0.5);
   evas_canvas3d_mesh_from_primitive_set(data->mesh_rocket[index], 0, data->sphere_primitive);
   SETUP_DEFAULT_MESH(rocket[index], rocket[index], DIFFUSE);
   SETUP_MESH_NODE(rocket[index]);

   evas_canvas3d_node_scale_set(data->mesh_node_rocket[index], 1, 1, 1);
   evas_canvas3d_node_position_set(data->mesh_node_rocket[index], ROCKET_POS_X, ROCKET_POS_Y, ROCKET_POS_Z);
}

void
_scene_setup(Scene_Data *data)
{
   int i = 0;

   for (i = 0; i < 4; i++)
      motion_vec[i] = 0;

   data->cube_primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->cube_primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE);
   evas_canvas3d_primitive_precision_set(data->cube_primitive, 10);

   data->sphere_primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->sphere_primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE);
   evas_canvas3d_primitive_precision_set(data->sphere_primitive, 50);

   global_scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   evas_canvas3d_scene_size_set(global_scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_background_color_set(global_scene, 0.5, 0.5, 0.9, 0.0);

   data->root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup_gun_planet(data);
   _mesh_setup_snake(data);

   for (i = 0; i < 10; i++)
     _mesh_setup_rocket(data, i);

   data->cylinder_primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_mode_set(data->cylinder_primitive, EVAS_CANVAS3D_PRIMITIVE_MODE_WITHOUT_BASE);
   evas_canvas3d_primitive_form_set(data->cylinder_primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_CYLINDER);
   evas_canvas3d_primitive_tex_scale_set(data->cylinder_primitive, 1.0, 1.0);
   evas_canvas3d_primitive_precision_set(data->cylinder_primitive, 50);

   for ( i = 0; i < 4; i++)
     _mesh_setup_column(data, i);

   _mesh_setup_grass(data);

   for ( i = 0; i < 4; i++)
     _mesh_setup_wall(data, i);

   TEXTCOORDS_SET(bricks1, 1.0, 0.2, 0.2, 1.0, 1.0, 1.0)
   _mesh_setup(data, 9.75, 10, 1, bricks1_textcoords, 0);

   TEXTCOORDS_SET(bricks2, 1.0, 1.5, 3.0, 0.5, 2.0, 0.5)
   for (i = 1; i < 10; i++)
     _mesh_setup(data, 1, 1, 4, bricks2_textcoords, i);

   _mesh_setup_warrior(data);

   _mesh_aabb(&data->mesh_cube, data, aabb_index);

   evas_canvas3d_scene_root_node_set(global_scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(global_scene, data->camera_node);

   if (data->shadow)
     evas_canvas3d_scene_shadows_enable_set(global_scene, EINA_TRUE);

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

   data->carp_mediator_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   evas_canvas3d_node_member_add(data->carp_mediator_node, data->mesh_node_carpet);

   evas_canvas3d_node_member_add(data->mediator_node, data->mesh_node_soldier);
   evas_canvas3d_node_member_add(data->mediator_node, data->mesh_node_tommy);

   for (i = 0; i < 10; i++)
        evas_canvas3d_node_member_add(data->mediator_node, data->mesh_node_rocket[i]);

   evas_canvas3d_node_member_add(data->root_node, data->carp_mediator_node);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_world);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_gazebo);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_eagle);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_cube);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_column_c);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_ball);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_grass);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_warrior);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_snake);

   for (i = 0; i < 4; i++)
     {
        evas_canvas3d_node_member_add(data->root_node, data->mesh_node_wall[i]);
        evas_canvas3d_node_member_add(data->root_node, data->mesh_node_column[i]);
     }
   for (i = 0; i < 10; i++)
        evas_canvas3d_node_member_add(data->root_node, data->mesh_node_level[i]);
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
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, global_scene);

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
