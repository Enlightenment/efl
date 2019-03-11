/**
 * Example of frustum culling in Evas-3D.
 *
 * Use 'w'/'s' key to move far frustum plane from/to the camera.
 * Use 't'/'g' key to move near frustum plane from/to the camera.
 * Use '1'/'2' key to set camera to the first/second position.
 * Use '3'/'4'/'5' key to set bounding sphere/aabb/central point mode.
 * Use 'i', 'k', 'j', 'l', 'u' and 'o' keys to move mesh node (model) in 3D.
 * Use 'z', 'x', 'c', 'Z', 'X' and 'C' keys to change scaling constants of mesh (model).
 * See in terminal output value distance to far plane of frustum and value of visibility of node
 * @see evas_canvas3d_camera_node_visible_get.
 *
 * @verbatim
 * gcc -o evas-3d-frustum evas-3d-frustum.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Eo.h>
#include <math.h>
#include "evas-common.h"

#define  WIDTH          800
#define  HEIGHT         600

static const char *texture_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/sweet_home_reversed.png";
static const char *mesh_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sweet_home.obj";

typedef struct _Scene_Data
{
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *mesh_node_model;
   Eo *mesh_node;
   Eo *scene;
   Eo *camera;
   Eo *light;
   Eo *mesh_model;
   Eo *sphere;
   Eo *cube;
   Eo *mesh_sphere;
   Eo *mesh_aabb;
   Eo *material_model;
   Eo *material;
   Eo *texture_model;
} Scene_Data;

Evas *evas;
Evas_Object *background,*image;
Evas_Real obj_x = 0.0, obj_y = 0.0, obj_z = 0.0, obj_sc_x = 10.0, obj_sc_y = 10.0, obj_sc_z = 10.0;
Evas_Real fleft = -5, fright = 5, fbottom = -5, fup = 5, fnear = 20, ffar = 1000;
Evas_Real radius = 0;
Evas_Canvas3D_Frustum_Mode key = EVAS_CANVAS3D_FRUSTUM_MODE_AABB;

static void
_show_help()
{
   printf("Press 'w'/'s' key to move far frustum plane from/to the camera\n");
   printf("Press 't'/'g' key to move near frustum plane from/to the camera\n");
   printf("Press '1'/'2' key to set camera to the first/second position\n");
   printf("Press '3'/'4'/'5' key to set bounding sphere/aabb/central point mode\n");
   printf("Press 'i', 'k', 'j', 'l', 'u' and 'o' keys to move mesh node in 3D\n");
   printf("Press 'z', 'x', 'c', 'Z', 'X' and 'C' keys to change scaling constants of mesh\n");
}

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
_redraw_bounding_object(void *data)
{
   Evas_Real px, py, pz, sx, sy, sz;
   Evas_Real x0, y0, z0, x1, y1, z1, rad;
   Scene_Data *scene = (Scene_Data *)data;
   Eo *current_mesh;
   Eina_List *meshes = NULL, *l;

   meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(scene->mesh_node);
   EINA_LIST_FOREACH(meshes, l, current_mesh)
     {
        evas_canvas3d_node_mesh_del(scene->mesh_node, current_mesh);
     }

   switch (key)
     {
      case EVAS_CANVAS3D_FRUSTUM_MODE_BSPHERE:
      {
         evas_canvas3d_node_bounding_sphere_get(scene->mesh_node_model, &x0, &y0, &z0, &rad);
         current_mesh = scene->mesh_sphere;
         px = x0;
         py = y0;
         pz = z0;
         sx = sy = sz = 2.0 * rad;
         break;
      }
      case EVAS_CANVAS3D_FRUSTUM_MODE_AABB:
      {
         evas_canvas3d_node_bounding_box_get(scene->mesh_node_model, &x0, &y0, &z0, &x1, &y1, &z1);
         current_mesh = scene->mesh_aabb;
         px = (x1 + x0) / 2;
         py = (y1 + y0) / 2;
         pz = (z1 + z0) / 2;
         sx = x1 - x0;
         sy = y1 - y0;
         sz = z1 - z0;
         break;
      }
      case EVAS_CANVAS3D_FRUSTUM_MODE_CENTRAL_POINT:
      {
         evas_canvas3d_node_bounding_sphere_get(scene->mesh_node_model, &x0, &y0, &z0, &rad);
         current_mesh = scene->mesh_sphere;
         px = x0;
         py = y0;
         pz = z0;
         sx = sy = sz = 0.1 * rad;
         break;
      }
      default:
         return ECORE_CALLBACK_RENEW;
     }

   evas_canvas3d_node_mesh_add(scene->mesh_node, current_mesh);
   evas_canvas3d_node_position_set(scene->mesh_node, px, py, pz);
   evas_canvas3d_node_scale_set(scene->mesh_node, sx, sy, sz);

   return ECORE_CALLBACK_RENEW;
}

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Scene_Data *scene = (Scene_Data *)data;
   Evas_Event_Key_Down *ev = event_info;
   Eina_Bool frustum;

   if (!strcmp("w", ev->key))
     {
        ffar += 20;
        evas_canvas3d_camera_projection_frustum_set(scene->camera, fleft, fright, fbottom, fup, fnear, ffar);
     }
   else if(!strcmp("s", ev->key))
     {
        ffar -= 20;
        evas_canvas3d_camera_projection_frustum_set(scene->camera, fleft, fright, fbottom, fup, fnear, ffar);
     }
   else if(!strcmp("t", ev->key))
     {
        fnear += 2;
        evas_canvas3d_camera_projection_frustum_set(scene->camera, fleft, fright, fbottom, fup, fnear, ffar);
     }
   else if(!strcmp("g", ev->key))
     {
        fnear -= 2;
        evas_canvas3d_camera_projection_frustum_set(scene->camera, fleft, fright, fbottom, fup, fnear, ffar);
     }
   else if(!strcmp("i", ev->key))
     {
        obj_y++;
        evas_canvas3d_node_position_set(scene->mesh_node_model, obj_x, obj_y, obj_z);
     }
   else if(!strcmp("k", ev->key))
     {
        obj_y--;
        evas_canvas3d_node_position_set(scene->mesh_node_model, obj_x, obj_y, obj_z);
     }
   else if(!strcmp("o", ev->key))
     {
        obj_z++;
        evas_canvas3d_node_position_set(scene->mesh_node_model, obj_x, obj_y, obj_z);
     }
   else if(!strcmp("u", ev->key))
     {
        obj_z--;
        evas_canvas3d_node_position_set(scene->mesh_node_model, obj_x, obj_y, obj_z);
     }
   else if(!strcmp("l", ev->key))
     {
        obj_x++;
        evas_canvas3d_node_position_set(scene->mesh_node_model, obj_x, obj_y, obj_z);
     }
   else if(!strcmp("j", ev->key))
     {
        obj_x--;
        evas_canvas3d_node_position_set(scene->mesh_node_model, obj_x, obj_y, obj_z);
     }
   else if(!strcmp("z", ev->key))
     {
        obj_sc_y+=0.02;
        evas_canvas3d_node_scale_set(scene->mesh_node_model, obj_sc_x, obj_sc_y, obj_sc_z);
     }
   else if(!strcmp("Z", ev->key))
     {
        obj_sc_y-=0.02;
        evas_canvas3d_node_scale_set(scene->mesh_node_model, obj_sc_x, obj_sc_y, obj_sc_z);
     }
   else if(!strcmp("x", ev->key))
     {
        obj_sc_z+=0.02;
        evas_canvas3d_node_scale_set(scene->mesh_node_model, obj_sc_x, obj_sc_y, obj_sc_z);
     }
   else if(!strcmp("X", ev->key))
     {
        obj_sc_z-=0.02;
        evas_canvas3d_node_scale_set(scene->mesh_node_model, obj_sc_x, obj_sc_y, obj_sc_z);
     }
   else if(!strcmp("c", ev->key))
     {
        obj_sc_x+=0.02;
        evas_canvas3d_node_scale_set(scene->mesh_node_model, obj_sc_x, obj_sc_y, obj_sc_z);
     }
   else if(!strcmp("C", ev->key))
     {
        obj_sc_x-=0.02;
        evas_canvas3d_node_scale_set(scene->mesh_node_model, obj_sc_x, obj_sc_y, obj_sc_z);
     }
   else if(!strcmp("Return", ev->key))
     {
        evas_canvas3d_camera_projection_frustum_set(scene->camera, fleft, fright, fbottom, fup, fnear, ffar);
     }
   else if (!strcmp("1", ev->key))
     {
        evas_canvas3d_node_position_set(scene->camera_node, 0.0, 0.0, 300.0);
        evas_canvas3d_node_look_at_set(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
        printf("Changed position and view of camera\n");
     }
   else if (!strcmp("2", ev->key))
     {
        evas_canvas3d_node_position_set(scene->camera_node, 300.0, 300.0, 300.0);
        evas_canvas3d_node_look_at_set(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
        printf("Changed position and view of camera\n");
     }
   else if (!strcmp("3", ev->key))
     {
        key = EVAS_CANVAS3D_FRUSTUM_MODE_BSPHERE;
     }
   else if (!strcmp("4", ev->key))
     {
        key = EVAS_CANVAS3D_FRUSTUM_MODE_AABB;
     }
   else if (!strcmp("5", ev->key))
     {
        key = EVAS_CANVAS3D_FRUSTUM_MODE_CENTRAL_POINT;
     }
   else
     {
        _show_help();
     }

   _redraw_bounding_object(data);
   frustum = evas_canvas3d_camera_node_visible_get(scene->camera, scene->camera_node, scene->mesh_node_model, key);
   printf("far - %f frustum - %d radius - %f\n", ffar, frustum, radius);
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   data->camera_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 0.0, 0.0, 300.0);
   evas_canvas3d_node_look_at_set(data->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);

   evas_canvas3d_camera_projection_frustum_set(data->camera, fleft, fright, fbottom, fup, fnear, ffar);

   evas_canvas3d_node_member_add(data->root_node, data->camera_node);
}

static void
_light_setup(Scene_Data *data)
{
   data->light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(data->light, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_light_diffuse_set(data->light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(data->light, 1.0, 1.0, 1.0, 1.0);

   data->light_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, 0.0, 0.0, 200.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 1.0);

   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}

static void
_mesh_setup(Scene_Data *data)
{
   data->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material, 100.0);

   data->cube = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->cube, EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE);

   data->sphere = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->sphere, EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE);
   evas_canvas3d_primitive_precision_set(data->sphere, 20);

   data->mesh_aabb = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(data->mesh_aabb, 0, data->cube);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_aabb, EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_aabb, EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
   evas_canvas3d_mesh_frame_material_set(data->mesh_aabb, 0, data->material);

   data->mesh_sphere = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(data->mesh_sphere, 0, data->sphere);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_sphere, EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_sphere, EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
   evas_canvas3d_mesh_frame_material_set(data->mesh_sphere, 0, data->material);

   data->mesh_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node);
   evas_canvas3d_node_mesh_add(data->mesh_node, data->mesh_aabb);
}

static void
_mesh_setup_model(Scene_Data *data)
{
   data->mesh_model = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   data->material_model = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   data->texture_model = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);

   efl_file_simple_load(data->texture_model, texture_path, NULL);
   evas_canvas3d_texture_filter_set(data->texture_model, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(data->texture_model, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);

   evas_canvas3d_material_enable_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.01, 0.01, 0.01, 1.0);
   evas_canvas3d_material_color_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_texture_set(data->material_model, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, data->texture_model);
   evas_canvas3d_material_shininess_set(data->material_model, 100.0);

   efl_file_simple_load(data->mesh_model, mesh_path, NULL);
   evas_canvas3d_mesh_frame_material_set(data->mesh_model, 0, data->material_model);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_model, EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   data->root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   _light_setup(data);
   _mesh_setup_model(data);
   _camera_setup(data);

   data->mesh_node_model = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_position_set(data->mesh_node_model, obj_x, obj_y, obj_z);
   evas_canvas3d_node_orientation_angle_axis_set(data->mesh_node_model, -90, 1.0, 0.0, 0.0);
   evas_canvas3d_node_scale_set(data->mesh_node_model, obj_sc_x, obj_sc_y, obj_sc_z);
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_model);
   evas_canvas3d_node_mesh_add(data->mesh_node_model, data->mesh_model);

   _mesh_setup(data);

    evas_canvas3d_scene_size_set(data->scene, WIDTH, HEIGHT);
    evas_canvas3d_scene_background_color_set(data->scene, 0.5, 0.5, 0.5, 0.0);
    evas_canvas3d_scene_root_node_set(data->scene, data->root_node);
    evas_canvas3d_scene_camera_node_set(data->scene, data->camera_node);
}

int
main(void)
{
   Scene_Data data;
   Ecore_Evas *ecore_evas = NULL;

   if (!ecore_evas_init()) return 0;

   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);
   ecore_evas = ecore_evas_new("opengl_x11", 0, 0, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

   background = evas_object_rectangle_add(evas);

   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   evas_object_focus_set(image, EINA_TRUE);
   efl_canvas_scene3d_set(image, data.scene);

   _redraw_bounding_object(&data);
   _show_help();

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
