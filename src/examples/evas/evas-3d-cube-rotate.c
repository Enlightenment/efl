/*
* This example shows how the work of events with callback of the mouse could be useful in node rotation.
*
* After clicking the mouse on cube callbacks began to emit autmatically,
* depending on mouse's position change the cube rotates with a certain
* speed while mouse is still on the cube, also rotation slows down when
* leaving the boundaries of the cube, all this goes on while the mouse
* is pressed. Note, that rotation is taking place around one axis only.
*
* Compile with "gcc -o evas-3d-cube-rotate evas-3d-cube-rotate.c -g `pkg-config --libs --cflags evas ecore ecore-evas eo` -lm"
*/

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Eo.h>


#define  WIDTH          400
#define  HEIGHT         400

typedef struct _Scene_Data
{
   Eo    *scene;
   Eo    *root_node;
   Eo    *camera_node;
   Eo    *light_node;
   Eo     *mesh_node;

   Eo               *camera;
   Eo               *light;
   Eo               *mesh;
   Eo               *material;
   float            angle;
   Evas_Coord       mouse_old_x;
   int              mouse_diff_x;
   int              mouse_indicator;
} Scene_Data;

Eo               *scene       = NULL;
Ecore_Evas       *ecore_evas  = NULL;
Evas             *evas        = NULL;
Eo               *background  = NULL;
Eo               *image       = NULL;
static float     angle        = 0.0f;
float            d_angle      = 0.5;
float            diff_angle_x = 1;
float            diff_angle_y = 1;
int              mouse_diff_x = 0;
int              mouse_diff_y = 0;
int              mouse_old_x  = 0;
int              mouse_old_y  = 0;
int              indicator    = 0;


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
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

   evas_object_resize(background, w, h);
   evas_object_resize(image, w, h);
   evas_object_move(image, 0, 0);
}

static Eina_Bool
_continue_scene(void *data)
{
   Scene_Data *d = (Scene_Data *)data;
   if (d->mouse_indicator == 1)
     {
        evas_event_feed_mouse_down(evas, 1, EVAS_BUTTON_NONE, 0, &data);
     }
   return EINA_TRUE;
}


static void
_rotate_x_reload(Evas_3D_Scene *scene, Evas_Event_Mouse_Down *ev, Evas_3D_Node *node, Evas_3D_Mesh *mesh,
                             float *angle_diff, Evas_Coord *mouse_coord)
{
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Evas_Real s, t;
   int mouse_diff;

   eo_do(scene, evas_3d_scene_pick(ev->canvas.x, ev->canvas.y, &n, &m, &s, &t));

   if ((n == node) && (m == mesh))
     {
        mouse_diff = ev->canvas.x - (*mouse_coord);
        *mouse_coord = ev->canvas.x;
        *angle_diff = (mouse_diff)/4;
     }
   else
     {
        if (*angle_diff > 2)
          (*angle_diff)-=1;
        else if (*angle_diff >0)
          (*angle_diff)-=0.25;
        else if(*angle_diff < -2)
          (*angle_diff)+=1;
        else if (*angle_diff <0)
          (*angle_diff)+=0.25;
     }
}

static void
_play_scene(void *data, Evas *evas EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void   *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Scene_Data *d = (Scene_Data *)data;

   _rotate_x_reload(scene, ev, d->mesh_node, d->mesh, &d->angle, &d->mouse_old_x);

   /* Run this cycle only once to avoid the callback tree. */
   if (d->mouse_indicator == 0)
     ecore_timer_add(0.016, _continue_scene, data);

   d->mouse_indicator = 1;
}

static void
_stop_scene(void *data, Evas *evas EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info EINA_UNUSED)
{
    Scene_Data *d = (Scene_Data *)data;
    d->angle = 0.0;
    d->mouse_indicator = 2;
}

static Eina_Bool
_animate_scene(void *data)
{
   Scene_Data *scene = (Scene_Data *)data;

   angle += scene->angle;

   eo_do(scene->mesh_node, evas_3d_node_orientation_angle_axis_set(angle, 0, 1.0, 0.0));

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(data->camera,
         evas_3d_camera_projection_perspective_set(60.0, 1.0, 2.0, 50.0));

   data->camera_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                     evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(data->camera_node,
         evas_3d_node_camera_set(data->camera),
         evas_3d_node_position_set(3.0, 3.0, 10.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));

   eo_do(data->root_node, evas_3d_node_member_add(data->camera_node));
}

static void
_light_setup(Scene_Data *data)
{
   data->light = eo_add(EVAS_3D_LIGHT_CLASS,evas);

   eo_do(data->light,
         evas_3d_light_ambient_set(0.2, 0.2, 0.2, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0));

   data->light_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));

   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(5.0, 5.0, 10.0),
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
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   /* Setup mesh. */
   data->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   eo_do(data->mesh,
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &cube_vertices[ 0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float), &cube_vertices[ 3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float), &cube_vertices[ 6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &cube_vertices[10]),

         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->material));

   data->mesh_node = eo_add(EVAS_3D_NODE_CLASS, evas, evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node));
   eo_do(data->mesh_node, evas_3d_node_mesh_add(data->mesh));
}

static void
_scene_setup(Scene_Data *data)
{
   scene = eo_add(EVAS_3D_SCENE_CLASS, evas);
   data->root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                   evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   eo_do(scene,
        evas_3d_scene_size_set(WIDTH, HEIGHT),
        evas_3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   eo_do(scene,
         evas_3d_scene_root_node_set(data->root_node),
         evas_3d_scene_camera_node_set(data->camera_node));
}

int
main(void)
{
   Scene_Data data;

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new("opengl_x11", 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

   data.angle = 0.5;
   data.mouse_old_x = 0;
   data.mouse_diff_x = 0;
   data.mouse_indicator = 0;

   /* Add a background rectangle objects. */
   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   /* Add an image object for 3D scene rendering. */
   image = evas_object_image_filled_add(evas);
   evas_object_image_size_set(image, WIDTH, HEIGHT);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   /* Set the image object as render target for 3D scene. */
   eo_do(image,
         evas_obj_image_scene_set(scene));

   evas_object_focus_set(image, EINA_TRUE);

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _play_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_UP, _stop_scene, &data);

   evas_event_feed_mouse_down(evas, 1, EVAS_BUTTON_NONE, 0, &data);

   /* Add animation timer callback. */
   ecore_timer_add(0.016, _animate_scene, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
