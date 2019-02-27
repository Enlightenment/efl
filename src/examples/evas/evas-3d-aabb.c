/**
 * Example of getting and drawing an axis-aligned bounding box in Evas-3D.
 *
 * @see _redraw_aabb();
 * Rotate axes (keys 1-4) for model and bounding box view from another angle.
 *
 * @verbatim
 * gcc -o evas-3d-aabb evas-3d-aabb.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "evas-common.h"

#define  WIDTH 400
#define  HEIGHT 400

static const char *model_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sonic.md2";
static const char *image_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/sonic.png";

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;
Eo *cube = NULL;
Eo *scene = NULL;
Eo *root_node = NULL;
Eo *camera_node = NULL;
Eo *light_node = NULL;
Eo *camera = NULL;
Eo *mesh_node = NULL;
Eo *mesh_box_node = NULL;
Eo *mesh = NULL;
Eo *mesh_box = NULL;
Eo *material_box = NULL;
Eo *material = NULL;
Eo *texture = NULL;
Eo *light = NULL;

static Eina_Bool
_redraw_aabb();

static Eina_Bool
_animate_scene(void *data)
{
   static int frame = 0;

   evas_canvas3d_node_mesh_frame_set((Evas_Canvas3D_Node *)data, mesh, frame);

   _redraw_aabb();

   frame += 32;

   if (frame > 256 * 50) frame = 0;

   return EINA_TRUE;
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Canvas3D_Node *node = (Evas_Canvas3D_Node *)data;

   switch(atoi(ev->key))
     {
      case 1:
        {
           evas_canvas3d_node_orientation_angle_axis_set(node, 90, 1.0, 0.0, 0.0);
           break;
        }
      case 2:
        {
           evas_canvas3d_node_orientation_angle_axis_set(node, 90, 0.0, 1.0, 0.0);
           break;
        }
      case 3:
        {
           evas_canvas3d_node_orientation_angle_axis_set(node, 90, 0.0, 0.0, 1.0);
           break;
        }
      case 4:
        {
           evas_canvas3d_node_orientation_angle_axis_set(node, 90, 1.0, 1.0, 0.0);
           break;
        }
     }
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
_redraw_aabb()
{
   Evas_Real x0, y0, z0, x1, y1, z1;

   evas_canvas3d_node_bounding_box_get(mesh_node, &x0, &y0, &z0, &x1, &y1, &z1);
   evas_canvas3d_node_position_set(mesh_box_node, (x0 + x1)/2, (y0 + y1)/2, (z0 + z1)/2);
   evas_canvas3d_node_scale_set(mesh_box_node, x1 - x0, y1 - y0, z1 - z0);

   return EINA_TRUE;
}

int
main(void)
{
   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new("opengl_x11", 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   evas_canvas3d_camera_projection_perspective_set(camera, 60.0, 1.0, 1.0, 500.0);

   camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(camera_node, camera);
   evas_canvas3d_node_member_add(root_node, camera_node);
   evas_canvas3d_node_position_set(camera_node, 100.0, 50.0, 20.0);
   evas_canvas3d_node_look_at_set(camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 20.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 1.0);
   light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_diffuse_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_directional_set(light, EINA_TRUE);

   light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(light_node, light);
   evas_canvas3d_node_position_set(light_node, 1000.0, 0.0, 1000.0);
   evas_canvas3d_node_look_at_set(light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(root_node, light_node);

   mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   efl_file_simple_load(mesh, model_path, NULL);
   evas_canvas3d_mesh_frame_material_set(mesh, 0, material);
   evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);

   texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   efl_file_simple_load(texture, image_path, NULL);
   evas_canvas3d_texture_filter_set(texture, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(texture, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   evas_canvas3d_material_texture_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, texture);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.01, 0.01, 0.01, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(material, 50.0);

   mesh_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   mesh_box_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   material_box = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   evas_canvas3d_material_enable_set(material_box, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);

   cube = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(cube, EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE);

   mesh_box = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(mesh_box, 0, cube);
   evas_canvas3d_mesh_vertex_assembly_set(mesh_box, EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES);
   evas_canvas3d_mesh_shader_mode_set(mesh_box, EVAS_CANVAS3D_SHADER_MODE_VERTEX_COLOR);
   evas_canvas3d_mesh_frame_material_set(mesh_box, 0, material_box);
   _redraw_aabb();

   evas_canvas3d_node_member_add(root_node, mesh_box_node);
   evas_canvas3d_node_mesh_add(mesh_box_node, mesh_box);

   evas_canvas3d_node_member_add(root_node, mesh_node);
   evas_canvas3d_node_mesh_add(mesh_node, mesh);

   evas_canvas3d_scene_root_node_set(scene, root_node);
   evas_canvas3d_scene_camera_node_set(scene, camera_node);
   evas_canvas3d_scene_size_set(scene, WIDTH, HEIGHT);

   background = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_color_set(background, 0, 0, 0, 255);
   efl_gfx_entity_size_set(background, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(background, EINA_TRUE);

   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);
   evas_object_focus_set(image, EINA_TRUE);
   efl_canvas_scene3d_set(image, scene);

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, mesh_node);

   ecore_timer_add(0.01, _animate_scene, mesh_node);

   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
