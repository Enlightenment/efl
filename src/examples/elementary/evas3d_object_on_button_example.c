/*
 * This example shows the mechanism of elementary widget adding (button) to the 3d scene object (cube) and
 * illustrates the work of callback of event from mouse.
 *
 * Compile with:
 * gcc -o evas3d_object_on_button_example evas3d_object_on_button_example.c -g `pkg-config --libs --cflags evas ecore eo elementary`
 */
#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif
#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Elementary.h>

#define  WIDTH          400
#define  HEIGHT         400

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
   Eo *texture;
} Scene_Data;

Evas_Object *win = NULL;
static Evas *evas = NULL;
static Eo *image = NULL;
static Eo *btn = NULL;
static float d_angle = 0.5;

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

static void
_stop_scene(void *data,
            Evas *e EINA_UNUSED,
            Evas_Object *eo EINA_UNUSED,
            void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Canvas3D_Node *n;
   Evas_Canvas3D_Mesh *m;
   Evas_Real s, t;
   Scene_Data *d = (Scene_Data *)data;

   if (ev->button == 1)
     {
        Eina_Bool ret;

        evas_canvas3d_scene_color_pick_enable_set(d->scene, EINA_TRUE);
        ret = evas_canvas3d_scene_pick(d->scene, ev->canvas.x, ev->canvas.y, &n, &m, &s, &t);
        if (ret)
          {
             d_angle = 0.0;
             elm_object_signal_emit(btn, "mouse,down,1", "event");
             evas_canvas3d_node_position_set(d->mesh_node, 0.0, 0.0, -0.2);
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
   elm_object_signal_emit(btn, "mouse,up,1", "event");
   evas_canvas3d_node_position_set(d->mesh_node, 0.0, 0.0, 0.0);
}


static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += d_angle;

   evas_canvas3d_node_orientation_angle_axis_set(scene->mesh_node, angle, 1.0, 1.0, 1.0);

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);

   evas_canvas3d_camera_projection_perspective_set(data->camera, 20.0, 1.0, 2.0, 50.0);

   data->camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 0.0, 0.0, 15.0);
   evas_canvas3d_node_look_at_set(data->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, -1.0, 0.0);
   evas_canvas3d_node_member_add(data->root_node, data->camera_node);
}

static void
_light_setup(Scene_Data *data)
{
   data->light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(data->light, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_light_diffuse_set(data->light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(data->light, 1.0, 1.0, 1.0, 1.0);

   data->light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, 0.0, 0.0, 10.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}
static void
_mesh_setup(Scene_Data *data)
{
   /* Setup material and texture as widget button. */
   data->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   data->texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);

   evas_canvas3d_texture_source_set(data->texture, btn);
   evas_canvas3d_texture_source_visible_set(data->texture, EINA_FALSE);

   evas_canvas3d_material_texture_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, data->texture);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material, 100.0);

   /* Setup mesh. */
   data->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_vertex_count_set(data->mesh, 24);
   evas_canvas3d_mesh_frame_add(data->mesh, 0);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, 12 * sizeof(float), &cube_vertices[ 0]);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, 12 * sizeof(float), &cube_vertices[ 3]);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, 12 * sizeof(float), &cube_vertices[ 6]);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, 12 * sizeof(float), &cube_vertices[10]);
   evas_canvas3d_mesh_index_data_set(data->mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh, 0, data->material);
   evas_canvas3d_mesh_color_pick_enable_set(data->mesh, EINA_TRUE);

   data->mesh_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node);
   evas_canvas3d_node_mesh_add(data->mesh_node, data->mesh);
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);
   evas_canvas3d_scene_size_set(data->scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_background_color_set(data->scene, 0.0, 0.0, 0.0, 0.0);

   data->root_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   evas_canvas3d_scene_root_node_set(data->scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(data->scene, data->camera_node);
}

int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Scene_Data data;

   elm_config_accel_preference_set("3d");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("evas3d-object-button", "A button on the 3d object");
   elm_win_autodel_set(win, EINA_TRUE);

   evas = evas_object_evas_get(win);

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);

   btn = elm_button_add(win);
   evas_object_resize(btn, (WIDTH * 2) / 3, (HEIGHT * 2) / 3);
   elm_object_text_set(btn, "3D Button");
   elm_object_scale_set(btn, 3.0);
   evas_object_show(btn);

   /* Set the image object as render target for 3D scene. */
   _scene_setup(&data);
   evas_object_focus_set(image, EINA_TRUE);
   efl_canvas_scene3d_set(image, data.scene);

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _stop_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_UP, _play_scene, &data);

   /* Add animation timer callback. */
   ecore_timer_add(0.016, _animate_scene, &data);

   evas_object_resize(win, WIDTH, HEIGHT);
   evas_object_show(win);

   /* Enter main loop. */
   elm_run();

   return 0;
}
ELM_MAIN()
