/*
* This example shows the mechanism of scene object adding (3D cube) to the widget button and
* illustrates the work of callback of event from mouse.
*
* Compile with:
* gcc -o 3d_scene_on_button_example 3d_scene_on_button example.c -g `pkg-config --libs --cflags evas ecore eo elementary`
*/

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#define WIDTH 500
#define HEIGHT 500
#define d_w 100
#define d_h 100


#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Elementary.h>
#include <stdio.h>

typedef struct _Scene_Data
{
   Eo *scene;
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *mesh_node;

   Eo *camera;
   Eo *light;
   Eo *mesh;
   Eo *material;
} Scene_Data;

Evas_Object *win = NULL;
Evas_Object *btn = NULL;
float d_angle = 0.5;
static Evas *evas = NULL;
static Eo *image = NULL;

static const float cube_vertices[] =
{
   /* Front */
   -1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,

   /* Back */
    1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,
   -1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Left */
   -1.0,  1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,
   -1.0,  1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Right */
    1.0,  1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Top */
   -1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
   -1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Bottom */
    1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,
   -1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,
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

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += d_angle;

   eo_do(scene->mesh_node,
         evas_3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, 1.0));

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);

   eo_do(data->camera,
         evas_3d_camera_projection_perspective_set(60.0, 1.0, 2.0, 50.0));

   data->camera_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(data->camera_node,
         evas_3d_node_camera_set(data->camera),
         evas_3d_node_position_set(0.0, 0.0, 10.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(data->root_node, evas_3d_node_member_add(data->camera_node));
}

static void
_light_setup(Scene_Data *data)
{
   data->light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(data->light,
         evas_3d_light_ambient_set(0.2, 0.2, 0.2, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0));

   data->light_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(0.0, 0.0, 10.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
}
static void
_mesh_setup(Scene_Data *data)
{
   /* Setup material. */
   data->material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,
                                    0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,
                                    0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   /* Setup mesh. */
   data->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   eo_do(data->mesh,
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_frame_add(0),

         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float),
                                            &cube_vertices[ 0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float),
                                            &cube_vertices[ 3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float),
                                            &cube_vertices[ 6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float),
                                            &cube_vertices[10]),

         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     36, &cube_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->material));

   data->mesh_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node));
   eo_do(data->mesh_node, evas_3d_node_mesh_add(data->mesh));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);
   eo_do(data->scene,
         evas_3d_scene_size_set(WIDTH - d_w, HEIGHT - d_h);
         evas_3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0));

   data->root_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   eo_do(data->scene,
         evas_3d_scene_root_node_set(data->root_node),
         evas_3d_scene_camera_node_set(data->camera_node));
}

static void
_stop_scene(void *data,
            Evas *e EINA_UNUSED,
            Evas_Object *eo EINA_UNUSED,
            void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Scene_Data *d = (Scene_Data *)data;

   eo_do(d->mesh_node,
         evas_3d_node_scale_set(0.97, 0.97, 0.97));

   if (ev->button == 1)
     {
        if (eo_do(d->scene,
                  evas_3d_scene_exist((ev->canvas.x - (d_w / 2)),
                                      (ev->canvas.y - (d_h / 2)),
                                      d->mesh_node)))
          {
             d_angle = 0;
          }
     }
}

static void
_play_scene(void *data,
            Evas *e EINA_UNUSED,
            Evas_Object *eo EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   Scene_Data *d = (Scene_Data *)data;

   d_angle = 0.5;

   eo_do(d->mesh_node,
         evas_3d_node_scale_set(1.0, 1.0, 1.0));
}

int
elm_main(int argc, char **argv)
{
   Scene_Data data;

   elm_config_preferred_engine_set("opengl_x11");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("evas3d-scene-button", "3d object on the button");
   if (!win) return 0;
   elm_win_autodel_set(win, EINA_TRUE);

   evas = evas_object_evas_get(win);
   if (!evas) return 0;

   _scene_setup(&data);

   image = evas_object_image_filled_add(evas);
   eo_do(image,
         evas_obj_visibility_set(EINA_TRUE));

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(data.scene));

   /* Setup scene to the widget button. */
   btn = elm_button_add(win);
   elm_object_content_set(btn, image);
   evas_object_resize(btn, (WIDTH - d_w), (HEIGHT - d_h));
   evas_object_move(btn, (d_w / 2), (d_h / 2));
   evas_object_show(btn);

   evas_object_event_callback_add(btn, EVAS_CALLBACK_MOUSE_DOWN, _stop_scene,
                                  &data);
   evas_object_event_callback_add(btn, EVAS_CALLBACK_MOUSE_UP, _play_scene,
                                  &data);

   /* Add animation timer callback. */
   ecore_timer_add(0.016, _animate_scene, &data);

   evas_object_resize(win, WIDTH, HEIGHT);
   evas_object_show(win);

   /* Enter main loop. */
   elm_run();

   elm_shutdown();

   return 0;
}
ELM_MAIN()
