#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <math.h>
#include <Evas_3D.h>

#define  WIDTH          1024
#define  HEIGHT         1024

Ecore_Evas       *ecore_evas	    = NULL;
Evas             *evas		    = NULL;
Evas_Object      *background	    = NULL;
Evas_Object      *image		    = NULL;

Evas_3D_Scene    *scene		    = NULL;
Evas_3D_Node     *root_node	    = NULL;
Evas_3D_Node     *camera_node	    = NULL;
Evas_3D_Node	 *light_node	    = NULL;
Evas_3D_Camera   *camera	    = NULL;
Evas_3D_Node	 *mesh_node	    = NULL;
Evas_3D_Mesh	 *mesh		    = NULL;
Evas_3D_Material *material	    = NULL;
Evas_3D_Texture	 *texture	    = NULL;
Evas_3D_Light	 *light		    = NULL;

static Eina_Bool
_animate_scene(void *data)
{
   static int frame = 0;

   evas_3d_node_mesh_frame_set((Evas_3D_Node *)data, mesh, frame);

   frame += 32;

   if (frame > 256 * 50)
       frame = 0;

   return EINA_TRUE;
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

int
main(void)
{
   if (!ecore_evas_init())
     return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas)
     return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   /* Add a scene object .*/
   scene = evas_3d_scene_add(evas);

   /* Add the root node for the scene. */
   root_node = evas_3d_node_add(evas, EVAS_3D_NODE_TYPE_NODE);

   /* Add the camera. */
   camera = evas_3d_camera_add(evas);
   evas_3d_camera_projection_perspective_set(camera, 60.0, 1.0, 1.0, 500.0);

   camera_node = evas_3d_node_add(evas, EVAS_3D_NODE_TYPE_CAMERA);
   evas_3d_node_camera_set(camera_node, camera);
   evas_3d_node_member_add(root_node, camera_node);
   evas_3d_node_position_set(camera_node, 100.0, 0.0, 20.0);
   evas_3d_node_look_at_set(camera_node, EVAS_3D_SPACE_PARENT, 0.0, 0.0, 20.0,
			    EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0);

   /* Add the light. */
   light = evas_3d_light_add(evas);
   evas_3d_light_ambient_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_3d_light_diffuse_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_3d_light_specular_set(light, 1.0, 1.0, 1.0, 1.0);

   light_node = evas_3d_node_add(evas, EVAS_3D_NODE_TYPE_LIGHT);
   evas_3d_node_light_set(light_node, light);
   evas_3d_node_member_add(root_node, light_node);
   evas_3d_node_position_set(light_node, 1000.0, 0.0, 1000.0);
   evas_3d_light_directional_set(light, EINA_TRUE);
   evas_3d_node_look_at_set(light_node, EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
			    EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0);

   /* Add the mesh. */
   mesh = evas_3d_mesh_add(evas);
   evas_3d_mesh_file_set(mesh, EVAS_3D_MESH_FILE_TYPE_MD2, "sonic.md2", NULL);

   material = evas_3d_material_add(evas);
   evas_3d_mesh_frame_material_set(mesh, 0, material);

   texture = evas_3d_texture_add(evas);

   evas_3d_texture_file_set(texture,  "sonic.png", NULL);
   evas_3d_texture_filter_set(texture, EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST);
   evas_3d_texture_wrap_set(texture, EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT);

   evas_3d_material_texture_set(material, EVAS_3D_MATERIAL_DIFFUSE, texture);

   evas_3d_material_enable_set(material, EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE);
   evas_3d_material_enable_set(material, EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE);
   evas_3d_material_enable_set(material, EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE);
   evas_3d_material_enable_set(material, EVAS_3D_MATERIAL_NORMAL, EINA_TRUE);

   evas_3d_material_color_set(material, EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0);
   evas_3d_material_color_set(material, EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_3d_material_color_set(material, EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_3d_material_shininess_set(material, 50.0);

   mesh_node = evas_3d_node_add(evas, EVAS_3D_NODE_TYPE_MESH);
   evas_3d_node_member_add(root_node, mesh_node);
   evas_3d_node_mesh_add(mesh_node, mesh);
   evas_3d_mesh_shade_mode_set(mesh, EVAS_3D_SHADE_MODE_PHONG);

   /* Set up scene. */
   evas_3d_scene_root_node_set(scene, root_node);
   evas_3d_scene_camera_node_set(scene, camera_node);
   evas_3d_scene_size_set(scene, WIDTH, HEIGHT);

   /* Add evas objects. */
   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   image = evas_object_image_filled_add(evas);
   evas_object_image_size_set(image, WIDTH, HEIGHT);
   evas_object_image_t3d_scene_set(image, scene);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   ecore_timer_add(0.01, _animate_scene, mesh_node);

   printf ("Enter main loop\n");
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
