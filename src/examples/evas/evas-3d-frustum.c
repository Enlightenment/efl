/*
 * This example shows how to work frustum culling.
 * Use 'w'/'s' key to move far frustum plane from/to the camera.
 * Use 't'/'g' key to move near frustum plane from/to the camera.
 * Use '1'/'2' key to set camera to the first/second position.
 * Use '3'/'4'/'5' key to set bounding sphere/aabb/central point mode.
 * Use 'i', 'k', 'j', 'l', 'u' and 'o' keys to move mesh node in 3D.
 * Use 'z', 'x', 'c', 'Z', 'X' and 'C' keys to change scaling constants of mesh.
 * See in terminal output value distance to far plane of frustum and value of visibility of node
 * @see evas_3d_camera_node_visible_get.
 * Compile with "gcc -o evas-3d-frustum evas-3d-frustum.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm"
 */

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Eo.h>
#include <math.h>

#define  WIDTH          800
#define  HEIGHT         600

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
   Eo *mesh;
   Eo *material_model;
   Eo *material;
   Eo *texture_model;
} Scene_Data;

Evas *evas;
Evas_Object *background,*image;
Evas_Real obj_x = 0.0, obj_y = 0.0, obj_z = 0.0, obj_sc_x = 1.0, obj_sc_y = 1.0, obj_sc_z = 1.0;
Evas_Real fleft = -5, fright = 5, fbottom = -5, fup = 5, fnear = 20, ffar = 1000;
Evas_Real radius = 0;
Evas_3D_Frustum_Mode key = EVAS_3D_FRUSTUM_MODE_AABB;

typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

static void
_set_ball(Eo *mesh, double r, double x, double y, double z, int p);

static void
_mesh_aabb(Eo *mesh_b, Eo *mesh_node);

static void
_show_help()
{
   fprintf(stdout, "Press 'w'/'s' key to move far frustum plane from/to the camera\n");
   fprintf(stdout, "Press 't'/'g' key to move near frustum plane from/to the camera\n");
   fprintf(stdout, "Press '1'/'2' key to set camera to the first/second position\n");
   fprintf(stdout, "Press '3'/'4'/'5' key to set bounding sphere/aabb/central point mode\n");
   fprintf(stdout, "Press 'i', 'k', 'j', 'l', 'u' and 'o' keys to move mesh node in 3D\n");
   fprintf(stdout, "Press 'z', 'x', 'c', 'Z', 'X' and 'C' keys to change scaling constants of mesh\n");
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
   Evas_Real x, y, z;
   Scene_Data *scene = (Scene_Data *)data;

   eo_do(scene->mesh_node_model, evas_3d_node_bounding_sphere_get(&x, &y, &z, &radius));
   if (key == EVAS_3D_FRUSTUM_MODE_BSPHERE)
     _set_ball(scene->mesh, radius, x, y, z, 100);
   else if (key == EVAS_3D_FRUSTUM_MODE_AABB)
     _mesh_aabb(scene->mesh, scene->mesh_node_model);
   else if (key == EVAS_3D_FRUSTUM_MODE_CENTRAL_POINT)
     _set_ball(scene->mesh, 1, x, y, z, 1);

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
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if(!strcmp("s", ev->key))
     {
        ffar -= 20;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if(!strcmp("t", ev->key))
     {
        fnear += 2;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if(!strcmp("g", ev->key))
     {
        fnear -= 2;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if(!strcmp("i", ev->key))
     {
        obj_y++;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("k", ev->key))
     {
        obj_y--;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("o", ev->key))
     {
        obj_z++;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("u", ev->key))
     {
        obj_z--;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("l", ev->key))
     {
        obj_x++;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("j", ev->key))
     {
        obj_x--;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("z", ev->key))
     {
        obj_sc_y+=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("Z", ev->key))
     {
        obj_sc_y-=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("x", ev->key))
     {
        obj_sc_z+=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("X", ev->key))
     {
        obj_sc_z-=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("c", ev->key))
     {
        obj_sc_x+=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("C", ev->key))
     {
        obj_sc_x-=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("Return", ev->key))
     {
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if (!strcmp("1", ev->key))
     {
        eo_do(scene->camera_node, evas_3d_node_position_set(0.0, 0.0, 300.0),
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
        fprintf(stdout, "Changed position and view of camera\n");
     }
   else if (!strcmp("2", ev->key))
     {
        eo_do(scene->camera_node, evas_3d_node_position_set(300.0, 300.0, 300.0);
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
        fprintf(stdout, "Changed position and view of camera\n");
     }
   else if (!strcmp("3", ev->key))
     {
        key = EVAS_3D_FRUSTUM_MODE_BSPHERE;
     }
   else if (!strcmp("4", ev->key))
     {
        key = EVAS_3D_FRUSTUM_MODE_AABB;
     }
   else if (!strcmp("5", ev->key))
     {
        key = EVAS_3D_FRUSTUM_MODE_CENTRAL_POINT;
     }
   else
     {
        _show_help();
     }

   _redraw_bounding_object(data);
   eo_do(scene->camera, frustum = evas_3d_camera_node_visible_get(scene->camera_node, scene->mesh_node_model, key));
   fprintf(stdout, "far - %f frustum - %d radius - %f\n", ffar, frustum, radius);
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   data->camera_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                     evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(data->camera_node,
         evas_3d_node_camera_set(data->camera),
         evas_3d_node_position_set(0.0, 0.0, 300.0);
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));

   eo_do(data->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));

   eo_do(data->root_node, evas_3d_node_member_add(data->camera_node));
}

static void
_light_setup(Scene_Data *data)
{
   data->light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(data->light,
         evas_3d_light_ambient_set( 0.2, 0.2, 0.2, 1.0);
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0);
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0));

   data->light_node = eo_add(EVAS_3D_NODE_CLASS,evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(0.0, 0.0, 200.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));

   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
}

static void
_mesh_aabb(Eo *mesh_b, Eo *mesh_node)
{
   Evas_Real x0, y0, z0, x1, y1, z1;

   eo_do(mesh_node, evas_3d_node_bounding_box_get(&x0, &y0, &z0, &x1, &y1, &z1));

   float vertices[] =
   {
      x0,  y0,  z1,
      x0,  y1,  z1,
      x1,  y1,  z1,
      x1,  y0,  z1,

      x0,  y0,  z0,
      x1,  y0,  z0,
      x0,  y1,  z0,
      x1,  y1,  z0,

      x0,  y0,  z0,
      x0,  y1,  z0,
      x0,  y0,  z1,
      x0,  y1,  z1,

      x1,  y0,  z0,
      x1,  y1,  z0,
      x1,  y1,  z1,
      x1,  y0,  z1,

      x0,  y1,  z0,
      x1,  y1,  z0,
      x0,  y1,  z1,
      x1,  y1,  z1,

      x0,  y0,  z0,
      x1,  y0,  z0,
      x1,  y0,  z1,
      x0,  y0,  z1
   };

   unsigned short indices[] =
   {
      0,  1,  2,  3,  1,  2,  0,  3,
      4,  5,  5,  7,  7,  6,  6,  4,
      8,  9,  9,  11, 11, 10, 10, 8,
      12, 13, 13, 14, 14, 15, 15, 12,
      16, 17, 17, 19, 19, 18, 18, 16,
      20, 21, 21, 22, 22, 23, 23, 20
   };

   float *cube_vertices = (float *) malloc(1 * sizeof(vertices));
   unsigned short *cube_indices = (unsigned short *) malloc(1 * sizeof(indices));
   memcpy(cube_vertices, vertices, sizeof(vertices));
   memcpy(cube_indices, indices, sizeof(indices));

   eo_do(mesh_b,
         evas_3d_mesh_vertex_count_set(24));

   eo_do(mesh_b,
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION,
                                                 3 * sizeof(float),
                                                 &cube_vertices[ 0]),
         evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                          48, &cube_indices[0]));
   free(cube_vertices);
   free(cube_indices);
}

static void
_set_ball(Eo *mesh, double r, double x, double y, double z, int p)
{
   int vcount, icount, vccount, i, j;
   double dtheta, dfi, sinth, costh, fi, theta, sinfi, cosfi;
   unsigned short *indices, *index;

   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   dtheta = M_PI / p;
   dfi = 2 * M_PI / p;

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

   eo_do(mesh,
         evas_3d_mesh_vertex_count_set(vcount),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION,
                                    sizeof(vec3), &vertices[0]),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_NORMAL,
                                    sizeof(vec3), &normals[0]),
         evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                    icount , &indices[0]));

   free(vertices);
   free(normals);
   free(indices);
}

static void
_mesh_setup(Scene_Data *data)
{
   data->material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   data->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   eo_do(data->mesh,
         evas_3d_mesh_frame_add(0);
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_LINES);
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_frame_material_set(0, data->material));


   data->mesh_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH),
                    evas_3d_node_position_set(obj_x, obj_y, obj_z),
                    evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node));
   eo_do(data->mesh_node, evas_3d_node_mesh_add(data->mesh));
}

static void
_mesh_setup_model(Scene_Data *data)
{
   data->mesh_model = eo_add(EVAS_3D_MESH_CLASS, evas);
   data->material_model = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   data->texture_model = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(data->texture_model,
         evas_3d_texture_file_set("eagle.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_model,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE);
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_texture_set( EVAS_3D_MATERIAL_DIFFUSE, data->texture_model),
         evas_3d_material_shininess_set(100.0));

   eo_do(data->mesh_model,
         efl_file_set("eagle.md2", NULL),
         evas_3d_mesh_frame_material_set(0, data->material_model),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   data->root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                   evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   _light_setup(data);
   _mesh_setup_model(data);
   _camera_setup(data);

   data->mesh_node_model = eo_add(EVAS_3D_NODE_CLASS, evas,
                                      evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->mesh_node_model,
         evas_3d_node_position_set(obj_x, obj_y, obj_z),
         evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node_model));
   eo_do(data->mesh_node_model, evas_3d_node_mesh_add(data->mesh_model));

   _mesh_setup(data);

    eo_do(data->scene,
         evas_3d_scene_size_set( WIDTH, HEIGHT),
         evas_3d_scene_background_color_set(0.5, 0.5, 0.5, 0.0),
         evas_3d_scene_root_node_set(data->root_node);
         evas_3d_scene_camera_node_set(data->camera_node));
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

   image = evas_object_image_filled_add(evas);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   evas_object_focus_set(image, EINA_TRUE);
   eo_do(image, evas_obj_image_scene_set(data.scene));

   _redraw_bounding_object(&data);
   _show_help();

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
