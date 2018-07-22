/**
 * Example of setting the texture source of an object in Evas-3D.
 *
 * Data which will be used as texture can be generated directly in application.
 *
 * @verbatim
 * gcc -o evas-3d-proxy evas-3d-proxy.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#include <math.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>

#define  WIDTH          400
#define  HEIGHT         400

#define  IMG_WIDTH       256
#define  IMG_HEIGHT      256

// undef this to test the legacy API for images
#define USE_EO_IMAGE

typedef struct _Scene_Data
{
   Eo *scene;
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *mesh_node;

   Eo *camera;
   Eo *light;
   Eo *cube;
   Eo *mesh;
   Eo *material;
   Eo *texture;
} Scene_Data;

static Ecore_Evas *ecore_evas = NULL;
static Evas *evas = NULL;
static Eo *background = NULL;
static Eo *image = NULL;
static Eo *source = NULL;

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
   efl_gfx_entity_size_set(background, EINA_SIZE2D(w,  h));
   efl_gfx_entity_size_set(image, EINA_SIZE2D(w,  h));
}

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;
   unsigned int *pixels;
   int i, j, stride;

   angle += 0.5;

   evas_canvas3d_node_orientation_angle_axis_set(scene->mesh_node, angle, 1.0, 1.0, 1.0);

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

#ifdef USE_EO_IMAGE
   Eina_Rw_Slice slice;
   slice = efl_gfx_buffer_map(source, EFL_GFX_BUFFER_ACCESS_MODE_WRITE, NULL,
                              EFL_GFX_COLORSPACE_ARGB8888, 0, &stride);
   pixels = slice.mem;
   if (!pixels) return EINA_TRUE;
#else
   pixels = evas_object_image_data_get(source, EINA_TRUE);
   stride = evas_object_image_stride_get(source);
#endif

   for (i = 0; i < IMG_HEIGHT; i++)
     {
        unsigned int *row = (unsigned int *)((char *)pixels + stride * i);

        for (j = 0; j < IMG_WIDTH; j++)
          {
             *row++ = rand() | 0xff000000;
          }
     }

#ifdef USE_EO_IMAGE
   efl_gfx_buffer_unmap(source, slice);
   efl_gfx_buffer_update_add(source, NULL);
#else
   evas_object_image_data_set(source, pixels);
   evas_object_image_data_update_add(source, 0, 0, IMG_WIDTH, IMG_HEIGHT);
#endif

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   evas_canvas3d_camera_projection_perspective_set(data->camera, 30.0, 1.0, 2.0, 50.0);

   data->camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 0.0, 0.0, 5.0);
   evas_canvas3d_node_look_at_set(data->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
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
   /* Setup material. */
   data->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   data->texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);

   evas_canvas3d_texture_source_set(data->texture, source);
   evas_canvas3d_texture_source_visible_set(data->texture, EINA_TRUE);

   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material, 100.0);
   evas_canvas3d_material_texture_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, data->texture);

   /* Set data of primitive */
   data->cube = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->cube, EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE);

   /* Setup mesh. */
   data->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(data->mesh, 0, data->cube);
   evas_canvas3d_mesh_shader_mode_set(data->mesh, EVAS_CANVAS3D_SHADER_MODE_FLAT);
   evas_canvas3d_mesh_frame_material_set(data->mesh, 0, data->material);

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
main(void)
{
   // Unless Evas 3D supports Software renderer, we force use of the gl backend.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   Scene_Data data;

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   /* Add a background rectangle object. */
   background = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_color_set(background, 0, 0, 0, 255);
   efl_gfx_entity_size_set(background, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(background, EINA_TRUE);

   /* Add a background image. */
#ifdef USE_EO_IMAGE
   source = efl_add(EFL_CANVAS_IMAGE_CLASS, evas);
   efl_gfx_buffer_copy_set(source, NULL, EINA_SIZE2D(IMG_WIDTH, IMG_HEIGHT), 0, EFL_GFX_COLORSPACE_ARGB8888, 0);
   efl_gfx_entity_position_set(source, EINA_POSITION2D((WIDTH / 2), (HEIGHT / 2)));
   efl_gfx_entity_size_set(source, EINA_SIZE2D((WIDTH / 2),  (HEIGHT / 2)));
   efl_gfx_entity_visible_set(source, EINA_TRUE);
#else
   source = evas_object_image_filled_add(evas);
   evas_object_image_size_set(source, IMG_WIDTH, IMG_HEIGHT);
   evas_object_move(source, (WIDTH / 2), (HEIGHT / 2));
   evas_object_resize(source, (WIDTH / 2), (HEIGHT / 2));
   evas_object_show(source);
#endif

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_size_set(image, EINA_SIZE2D((WIDTH / 2),  (HEIGHT / 2)));
   efl_gfx_entity_visible_set(image, EINA_TRUE);

   /* Setup scene */
   _scene_setup(&data);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, data.scene);

   /* Add animation timer callback. */
   ecore_timer_add(0.016, _animate_scene, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
