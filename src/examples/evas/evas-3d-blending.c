/**
 * Example illustrating usage of blending modes. Press "Up" or "Down" key for change source blending factor.
 * Press "Left" or "Right" for change destination blending factors.
 * For more detailes see https://www.opengl.org/sdk/docs/man2/xhtml/glBlendFunc.xml
 *
 * @verbatim
 * gcc -o evas-3d-blending evas-3d-blending.c `pkg-config --libs --cflags evas ecore ecore-evas eo`-lm
 * @endverbatim
 */

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <math.h>
#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#define  WIDTH          1024
#define  HEIGHT         1024

static const char *blend_func_names[] =
{
   /**< The scale factors for color components is (0, 0, 0, 0)*/
   "EVAS_3D_BLEND_ZERO",
   /**< The scale factors for color components is (1, 1, 1, 1)*/
   "EVAS_3D_BLEND_ONE",
   /**< The scale factors for color components is (Rs/kR, Gs/kG, Bs/kB, As/kA)*/
   "EVAS_3D_BLEND_SRC_COLOR",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Rs/kR, Gs/kG, Bs/kB, As/kA)*/
   "EVAS_3D_BLEND_ONE_MINUS_SRC_COLOR",
   /**< The scale factors for color components is (Rd/kR, Gd/kG, Bd/kB, Ad/kA)*/
   "EVAS_3D_BLEND_DST_COLOR",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Rd/kR, Gd/kG, Bd/kB, Ad/kA)*/
   "EVAS_3D_BLEND_ONE_MINUS_DST_COLOR",
   /**< The scale factors for color components is (As/kA, As/kA, As/kA, As/kA)*/
   "EVAS_3D_BLEND_SRC_ALPHA",
   /**< The scale factors for color components is (1, 1, 1, 1) - (As/kA, As/kA, As/kA, As/kA)*/
   "EVAS_3D_BLEND_ONE_MINUS_SRC_ALPHA",
   /**< The scale factors for color components is (Ad/kA, Ad/kA, Ad/kA, Ad/kA)*/
   "EVAS_3D_BLEND_DST_ALPHA",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Ad/kA, Ad/kA, Ad/kA, Ad/kA)*/
   "EVAS_3D_BLEND_ONE_MINUS_DST_ALPHA",
   /**< The scale factors for color components is (Rc, Gc, Bc, Ac)*/
   "EVAS_3D_BLEND_CONSTANT_COLOR",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Rc, Gc, Bc, Ac)*/
   "EVAS_3D_BLEND_ONE_MINUS_CONSTANT_COLOR",
   /**< The scale factors for color components is (Ac, Ac, Ac, Ac)*/
   "EVAS_3D_BLEND_CONSTANT_ALPHA",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Ac, Ac, Ac, Ac)*/
   "EVAS_3D_BLEND_ONE_MINUS_CONSTANT_ALPHA",
   /**< The scale factors for color components is (i, i, i, 1) where i = min(As, kA, Ad)/kA*/
   "EVAS_3D_BLEND_SRC_ALPHA_SATURATE",
};

typedef struct _Scene_Data
{
   Eo     *scene;
   Eo     *root_node;
   Eo     *camera_node;
   Eo     *light_node;
   Eo     *mesh_node;
   Eo     *mesh_node1;

   Eo     *camera;
   Eo     *light;
   Eo     *mesh;
   Eo     *mesh1;
   Eo     *material;
   Eo     *material1;
} Scene_Data;

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

Evas             *evas        = NULL;
Eo               *background  = NULL;
Eo               *image       = NULL;

int func1 = EVAS_3D_BLEND_SRC_ALPHA;
int func2 = EVAS_3D_BLEND_ONE_MINUS_SRC_ALPHA;

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Scene_Data *scene = (Scene_Data *)data;
   Evas_Event_Key_Down *ev = event_info;

   if (!strcmp("Up", ev->key))
     func1++;
   if (!strcmp("Down", ev->key))
     func1--;
   if (!strcmp("Left", ev->key))
     func2++;
   if (!strcmp("Right", ev->key))
     func2--;

   if(func1 < EVAS_3D_BLEND_ZERO)
     func1 = EVAS_3D_BLEND_SRC_ALPHA_SATURATE;
   if(func1 > EVAS_3D_BLEND_SRC_ALPHA_SATURATE)
     func1 = EVAS_3D_BLEND_ZERO;

   if(func2 < EVAS_3D_BLEND_ZERO)
     func2 = EVAS_3D_BLEND_SRC_ALPHA_SATURATE;
   if(func2 > EVAS_3D_BLEND_SRC_ALPHA_SATURATE)
     func2 = EVAS_3D_BLEND_ZERO;
   eo_do(scene->mesh1, evas_3d_mesh_blending_func_set(func1, func2));
   printf("sfactor = %s, dfactor = %s\n", blend_func_names[func1], blend_func_names[func2]);
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
   eo_do(background, evas_obj_size_set(w, h));
   eo_do(image, evas_obj_size_set(w, h));
}

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += 0.02;

   eo_do(scene->mesh_node,
         evas_3d_node_position_set(3 * sin(angle), 0.0, 3 * cos(angle)));

   if (angle > 2 * M_PI) angle = 0;

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
_set_ball(Eo *mesh, int p, Evas_3D_Material *material)
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
             vertices[i + j * vccount].x = sinth * cosfi;
             vertices[i + j * vccount].y = sinth * sinfi;
             vertices[i + j * vccount].z = costh;

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
            evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                       sizeof(vec3), &vertices[0]);
            evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                       sizeof(vec3), &normals[0]);
            evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                icount , &indices[0]);
            evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG);
            evas_3d_mesh_frame_material_set(0, material));
}

static void
_mesh_setup(Scene_Data *data)
{
   data->material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 1.0, 0.2, 0.2, 0.2),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 0.0, 0.0, 0.2),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 0.2),
         evas_3d_material_shininess_set(100.0));

   data->material1 = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material1,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.0, 0.2, 0.2, 0.2),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.0, 0.0, 1.0, 0.2),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 0.2),
         evas_3d_material_shininess_set(100.0));

   data->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   data->mesh1 = eo_add(EVAS_3D_MESH_CLASS, evas);

   _set_ball(data->mesh, 100, data->material);
   _set_ball(data->mesh1, 100, data->material1);

   data->mesh_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node));
   eo_do(data->mesh_node, evas_3d_node_mesh_add(data->mesh));

   data->mesh_node1 =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node1));
   eo_do(data->mesh_node1, evas_3d_node_mesh_add(data->mesh1));

   eo_do(data->mesh1, evas_3d_mesh_blending_enable_set(EINA_TRUE),
      evas_3d_mesh_blending_func_set(func1, func2));

   eo_do(data->mesh_node1,
         evas_3d_node_scale_set(2.5, 2.5, 2.5));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   eo_do(data->scene,
         evas_3d_scene_size_set(WIDTH, HEIGHT);
         evas_3d_scene_background_color_set(0.5, 0.5, 0.5, 1));

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

int
main(void)
{
   Scene_Data data;
   Ecore_Animator *anim;
   Ecore_Evas *ecore_evas = NULL;

   if (!ecore_evas_init()) return 0;

   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);
   ecore_evas = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

   /* Add a background rectangle objects. */
   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   /* Add an image object for 3D scene rendering. */
   image = evas_object_image_filled_add(evas);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);
   evas_object_focus_set(image, EINA_TRUE);

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(data.scene));


   /* Add animation timer callback. */
   ecore_animator_frametime_set(0.008);
   anim = ecore_animator_add(_animate_scene, &data);

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);
   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
   ecore_animator_del(anim);

   return 0;
}
