/**
 * Example of static LOD techniques in Evas-3D.
 *
 * Main idea using several meshes with different quantity of polygons,
 * render mesh depends on distance to the camera node.
 * Use key up/down to change animtion style:
 *   Up (default) object motion
 *   Down camera motion
 * @verbatim
 * gcc -o evas-3d-static-lod evas-3d-static-lod.c `pkg-config --libs --cflags efl evas ecore ecore-evas eina eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "evas-common.h"

#define  WIDTH          600
#define  HEIGHT         600
#define  NUMBER_MESHS   2
#define  DNEAR           90.0
#define  DFAR            1000.0

static const char *model_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/lod/iso_sphere";

typedef struct _Scene_Data
{
   Eo *scene;
   Eo *root_node;
   Eo *complex_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *lod_mesh_node1;
   Eo *lod_mesh_node2;

   Eo *camera;
   Eo *light;
   Eo *material;
} Scene_Data;

static Ecore_Evas *ecore_evas = NULL;
static Evas *evas = NULL;
static Eo *background = NULL;
static Eo *image = NULL;
Eina_Bool animation_type = EINA_FALSE;

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
   Evas_Real x, y, z;
   Scene_Data *scene = (Scene_Data *)data;
   static Eina_Bool pass = EINA_TRUE;
   if (animation_type) //Camera motion
     {
        evas_canvas3d_node_position_set(scene->complex_node, -20.0, 0.0, 50.0);
        evas_canvas3d_node_position_get(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        if (pass)
           z -= 4.0;
        else
           z += 4.0;

        if (z == DNEAR * 2.0) pass = EINA_FALSE;
        if (z == DFAR / 2.0) pass = EINA_TRUE;

        evas_canvas3d_node_position_set(scene->camera_node, x , y , z);
        evas_canvas3d_node_look_at_set(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                             EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
     }
   else //Object motion
     {
        evas_canvas3d_node_position_set(scene->camera_node, 0.0, 0.0, 300.0);
        evas_canvas3d_node_look_at_set(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                       EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
        evas_canvas3d_node_position_get(scene->complex_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set( scene->complex_node, sin(angle) * 100 - 20 , y , cos(angle) * 200);

        angle += 0.02f;
        if (angle > 360.0) angle -= 360.0f;
     }
   return EINA_TRUE;

}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);

   evas_canvas3d_camera_projection_perspective_set(data->camera, DNEAR, 1.0, 2.0, DFAR);

   data->camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 0.0, 0.0, 300.0);
   evas_canvas3d_node_look_at_set(data->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                     EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
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
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, 0.0, 0.0, 300.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}
static void
_mesh_setup(Scene_Data *data)
{
   int i = 0;
   Evas_Real distances[NUMBER_MESHS + 2] = {DNEAR, 150.0, 300.0, 500.0};
   Evas_Real m_distances[NUMBER_MESHS + 2] = {DNEAR, 200.0, 500.0, 500.0};

   /* Setup material. */
   data->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material, 100.0);

   data->lod_mesh_node1 =
         efl_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_position_set(data->lod_mesh_node1, 2.0, 0.0, 0.0);

   data->lod_mesh_node2 =
         efl_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   data->complex_node =
         efl_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));
   evas_canvas3d_node_scale_set(data->complex_node, 30.0, 30.0, 30.0);

   /* Setup mesh. One model, two meshes object with different lod distances.*/
   for (i = 0; i <= NUMBER_MESHS; i++)
     {
        Eo *mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
        Eina_Stringshare *str = eina_stringshare_printf("%s%d%s", model_path,
                                                        i, ".obj");
        efl_file_simple_load(mesh, str, NULL);
        evas_canvas3d_mesh_frame_material_set(mesh, 0, data->material);
        evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
        evas_canvas3d_mesh_lod_boundary_set(mesh, distances[i], distances[i + 1]);
        evas_canvas3d_node_mesh_add(data->lod_mesh_node1, mesh);

        mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
        efl_file_simple_load(mesh, str, NULL);
        evas_canvas3d_mesh_frame_material_set(mesh, 0, data->material);
        evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
        evas_canvas3d_mesh_lod_boundary_set(mesh, m_distances[i], m_distances[i + 1]);

        evas_canvas3d_node_mesh_add(data->lod_mesh_node2, mesh);
        eina_stringshare_del(str);
     }
   evas_canvas3d_node_lod_enable_set(data->lod_mesh_node1, EINA_TRUE);
   evas_canvas3d_node_lod_enable_set(data->lod_mesh_node2, EINA_TRUE);
   evas_canvas3d_node_member_add(data->complex_node, data->lod_mesh_node1);
   evas_canvas3d_node_member_add(data->complex_node, data->lod_mesh_node2);
   evas_canvas3d_node_member_add(data->root_node, data->complex_node);
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);
   evas_canvas3d_scene_size_set(data->scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_background_color_set(data->scene, 0.0, 0.0, 0.0, 0.0);

   data->root_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   evas_canvas3d_scene_root_node_set(data->scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(data->scene, data->camera_node);
}

static void
_on_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if(!strcmp("Up", ev->key))
     animation_type = EINA_FALSE;
   else if(!strcmp("Down", ev->key))
     animation_type = EINA_TRUE;
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

   _scene_setup(&data);

   /* Add a background rectangle object. */
   background = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_color_set(background, 0, 0, 0, 255);
   efl_gfx_entity_size_set(background, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(background, EINA_TRUE);

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);
   evas_object_focus_set(image, EINA_TRUE);
   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, data.scene);

   /* Add animation timer callback. */
   ecore_animator_add(_animate_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);
   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}

