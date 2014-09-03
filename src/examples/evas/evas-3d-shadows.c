/*
 * This example illustrating use of shadows in the scene.
 *
 * @see evas_3d_scene_shadows_enable_set(Eina_Bool _shadows_enabled)
 *
 * Compile with "gcc -o evas-3d-shadows evas-3d-shadows.c `pkg-config --libs --cflags evas ecore ecore-evas eo` -lm"
 */


#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <math.h>

#define  WIDTH 1024
#define  HEIGHT 1024

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;

Eo *scene = NULL;
Eo *root_node = NULL;
Eo *camera_node = NULL;
Eo *light_node = NULL;
Eo *camera = NULL;
Eo *mesh_node = NULL;
Eo *mesh_node1 = NULL;
Eo *mesh = NULL;
Eo *mesh1 = NULL;
Eo *material = NULL;
Eo *material1 = NULL;
Eo *texture = NULL;
Eo *light = NULL;
Ecore_Animator *anim = NULL;

static Eina_Bool
_animate_scene(void *data)
{
   static int frame = 0;

   eo_do((Evas_3D_Node *)data, evas_3d_node_mesh_frame_set(mesh, frame));

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
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   eo_do(background, evas_obj_size_set(w, h));
   eo_do(image, evas_obj_size_set(w, h));
}

static double pi = 3.14159265359;
typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

typedef struct _vec4
{
    float   x;
    float   y;
    float   z;
    float   w;
} vec4;

static void
_set_ball(Eo *mesh, double r, double x, double y, double z, int p, Evas_3D_Material *material)
{
   int vcount, icount, vccount, i, j;
   double dtheta, dfi, sinth, costh, fi, theta, sinfi, cosfi;
   unsigned short *indices, *index;

   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   dtheta = pi / p;
   dfi = 2 * pi / p;

   vec3 *vertices = malloc(sizeof(vec3) * vcount);
   vec3 *normals = malloc(sizeof(vec3) * vcount);

  for (j = 0; j < vccount; j++)
     {
        theta = j * dtheta;
        sinth = sin(theta);
        costh = cos(theta);
        for (i = 0; i < vccount; i++)
          {
             fi = i * dfi;
             sinfi = sin(fi);
             cosfi = cos(fi);
             vertices[i + j * vccount].x = r * sinth * cosfi + x;
             vertices[i + j * vccount].y = r * sinth * sinfi + y;
             vertices[i + j * vccount].z = r * costh + z;

             normals[i + j * vccount].x = sinth * cosfi;
             normals[i + j * vccount].y = sinth * sinfi;
             normals[i + j * vccount].z = costh;
          }
     }

   indices = malloc(sizeof(short) * icount);
   index = &indices[0];

   for(j = 0; j < p; j++)
     for(i = 0; i < p; i++)
       {
          *index++ = (unsigned short)(i + vccount * j);
          *index++ = i + vccount * (j + 1);
          *index++ = i + 1 + vccount * (j + 1);

          *index++ =  i + vccount * j;
          *index++ =  i + 1 +  vccount * j;
          *index++ =  i + vccount * (j + 1) + 1;
       }

   eo_do(mesh, evas_3d_mesh_vertex_count_set(vcount),
            evas_3d_mesh_frame_add(0);
            evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION,
                                       sizeof(vec3), &vertices[0]);
            evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_NORMAL,
                                       sizeof(vec3), &normals[0]);
            evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                icount , &indices[0]);
            evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES);
            evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG);
            evas_3d_mesh_frame_material_set(0, material));

   free(vertices);
   free(normals);
   free(indices);
}

int
main(void)
{
   //Unless Evas 3D supports Software renderer, we set gl backened forcely.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   /* Add a scene object .*/
   scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   /* Add the root node for the scene. */
   root_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   /* Add the camera. */
   camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(camera,
         evas_3d_camera_projection_perspective_set(60.0, 1.0, 1.0, 1000.0));

   camera_node =
      eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_3d_node_camera_set(camera));
   eo_do(root_node,
         evas_3d_node_member_add(camera_node));
   eo_do(camera_node,
         evas_3d_node_position_set(100.0, 0.0, 20.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 20.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));
   /* Add the light. */
   light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(light,
         evas_3d_light_ambient_set(0.2, 0.2, 0.2, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_projection_perspective_set(45.0, 1.0, 2.0, 1000.0));

   light_node =
      eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(light_node,
         evas_3d_node_light_set(light),
         evas_3d_node_position_set(100.0, 30.0, 20.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 20.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));
   eo_do(root_node,
         evas_3d_node_member_add(light_node));

   /* Add the mesh. */
   mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(mesh,
         evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, "sonic.md2", NULL),
         evas_3d_mesh_frame_material_set(0, material),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));

   texture = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(texture,
         evas_3d_texture_file_set("sonic.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(material,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, texture),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_AMBIENT, texture),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

   mesh_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(root_node,
         evas_3d_node_member_add(mesh_node));
   eo_do(mesh_node,
         evas_3d_node_mesh_add(mesh));

   material1 = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(material1,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   mesh1 = eo_add(EVAS_3D_MESH_CLASS, evas);
   _set_ball(mesh1, 100, -200, 0, 0, 100, material1);


   mesh_node1 =
      eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(root_node, evas_3d_node_member_add(mesh_node1));
   eo_do(mesh_node1, evas_3d_node_mesh_add(mesh1));

   /* Set up scene. */
   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT),
         evas_3d_scene_shadows_enable_set(EINA_TRUE));

   /* Add a background rectangle objects. */
   background = eo_add(EVAS_RECTANGLE_CLASS, evas);
   eo_unref(background);
   eo_do(background,
         evas_obj_color_set(0, 0, 0, 255),
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));

   /* Add an image object for 3D scene rendering. */
   image = evas_object_image_filled_add(evas);
   eo_do(image,
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(scene));

   /* Add animator. */
   ecore_animator_frametime_set(0.008);
   anim = ecore_animator_add(_animate_scene, mesh_node);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
