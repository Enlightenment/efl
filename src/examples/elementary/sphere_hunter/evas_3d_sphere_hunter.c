/*
* Catch all the cubes to score 10 points and win.
* w - up
* s - down
* a - left
* d - right
* space - jump
* key Up - scale +
* key Down - scale -
* n - normal scale
* TODO: add more levels.
*
* Compile with:
* edje_cc sphere_hunter.edc
* gcc -o evas_3d_sphere_hunter evas_3d_sphere_hunter.c -g `pkg-config --libs --cflags evas ecore elementary eina eo` -lm
*/

#define EFL_BETA_API_SUPPORT

#include <Elementary.h>
#include <Ecore.h>
#include <Evas.h>
#include <Eina.h>
#include <stdio.h>
#include <math.h>
#include <Eo.h>

#define  WIDTH          700
#define  HEIGHT         800
#define  ANGLE_IT       0.006
#define  GE             -0.1
#define  KE             1

typedef struct _Scene_Data
{
   Eo   *root_node;
   Eo   *camera_node;
   Eo   *light_node;
   Eo   *mesh_node_ball;

   Eina_List *items;

   Eo   *scene;
   Eo   *camera;
   Eo   *light;
   Eo   *mesh_cube;
   Eo   *material_cube;
   Eo   *mesh_ball;
   Eo   *material_ball;
} Scene_Data;

typedef struct _vec4
{
   float   x;
   float   y;
   float   z;
   float   w;
} vec4;

typedef struct _vec3
{
   float   x;
   float   y;
   float   z;
} vec3;

typedef struct _vec2
{
   float   x;
   float   y;
} vec2;

typedef struct _vertex
{
   vec3    position;
   vec3    normal;
   vec3    tangent;
   vec4    color;
   vec3    texcoord;
} vertex;

Evas_Object      *win               = NULL;
Evas_Object      *layout            = NULL;
Evas_Object      *text              = NULL;
Evas_Object      *btn_restart        = NULL;
Evas_Object      *btn_quit          = NULL;
Eina_List        *nodes             = NULL;

Ecore_Animator   *anim_cube         = NULL;
Ecore_Animator   *anim_camera       = NULL;
Ecore_Animator   *anim_ball         = NULL;
Evas             *evas              = NULL;
Eo               *image             = NULL;


Evas_Real        ball_position_x    = 0.0;
Evas_Real        ball_position_y    = -1.0;
Evas_Real        ball_position_z    = 0.0;
Evas_Real        ball_vel_x         = 0.0;
Evas_Real        ball_vel_y         = 0.0;
Evas_Real        ball_vel_z         = 0.0;
Evas_Real        ball_ac_x          = 0.0;
Evas_Real        ball_ac_y          = GE;
Evas_Real        ball_ac_z          = 0.0;
Evas_Real        plane_alpha_x      = 0.0;
Evas_Real        plane_alpha_z      = 0.0;
Eina_Bool        reload             = EINA_TRUE;
Eina_Bool        jump               = EINA_FALSE;
Eina_Bool        game               = EINA_TRUE;

vertex           *vertices          = NULL;
int              score              = 0;
int              vertex_count       = 0;
int              index_count        = 0;
unsigned short   *indices           = NULL;
float            camera_y           = 40.0;
float            camera_z           = 70.0;
int              hight              = 1;


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
_sphere_fini()
{
   free(vertices);
   free(indices);
}

static void
_sphere_init(int precision)
{
   int i, j;
   unsigned short *idx;

   vertex_count = (precision + 1) * (precision + 1);
   index_count = precision * precision * 6;

   /* Allocate buffer. */
   vertices = malloc(sizeof(vertex) * vertex_count);
   indices = malloc(sizeof(unsigned short) * index_count);

   /* Calculate vertices position of the sphere mesh by using
      splitting of sphere by latitude and longitude. */
   for (i = 0; i <= precision; i++)
     {
        double lati, y, r;

        lati = (M_PI * (double)i) / (double)precision;
        y = cos(lati);
        r = fabs(sin(lati));

        for (j = 0; j <= precision; j++)
          {
             double longi;
             vertex *v;

             longi = (M_PI * 2.0 * j) / precision;
             v = &vertices[(i * (precision  + 1)) + j];

             if ((j == 0) || (j == precision)) v->position.x = 0.0;
             else v->position.x = r * sin(longi);

             v->position.y = y;

             if ((j == 0) || (j == precision)) v->position.z = r;
             else v->position.z = r * cos(longi);

             v->normal = v->position;

             if (v->position.x > 0.0)
               {
                  v->tangent.x = -v->normal.y;
                  v->tangent.y =  v->normal.x;
                  v->tangent.z =  v->normal.z;
               }
             else
               {
                  v->tangent.x =  v->normal.y;
                  v->tangent.y = -v->normal.x;
                  v->tangent.z =  v->normal.z;
               }

             v->color.x = v->position.x;
             v->color.y = v->position.y;
             v->color.z = v->position.z;
             v->color.w = 1.0;

             if (j == precision) v->texcoord.x = 1.0;
             else if (j == 0) v->texcoord.x = 0.0;
             else v->texcoord.x = (double)j / (double)precision;

             if (i == precision) v->texcoord.y = 1.0;
             else if (i == 0) v->texcoord.y = 0.0;

             else v->texcoord.y = 1.0 - ((double)i / (double)precision);
          }
     }

   idx = &indices[0];

   /* Calculate and fill in the buffer of indices,
      prepare stage for gl triangulation. */
   for (i = 0; i < precision; i++)
     {
        for (j = 0; j < precision; j++)
          {
             *idx++ = (i * (precision + 1)) + j;
             *idx++ = (i * (precision + 1)) + j + 1;
             *idx++ = ((i + 1) * (precision + 1)) + j;

             *idx++ = ((i + 1) * (precision + 1)) + j;
             *idx++ = (i * (precision + 1)) + j + 1;
             *idx++ = ((i + 1) * (precision + 1)) + j + 1;
          }
     }

   /* Triangulation of sphere mesh in appliance with buffer of indices. */
   for (i = 0; i < index_count; i += 3)
     {
        vec3 e1, e2;
        float du1, du2, dv1, dv2, f;
        vec3 tangent;

        vertex *v0 = &vertices[indices[i + 0]];
        vertex *v1 = &vertices[indices[i + 1]];
        vertex *v2 = &vertices[indices[i + 2]];

        e1.x = v1->position.x - v0->position.x;
        e1.y = v1->position.y - v0->position.y;
        e1.z = v1->position.z - v0->position.z;

        e2.x = v2->position.x - v0->position.x;
        e2.y = v2->position.y - v0->position.y;
        e2.z = v2->position.z - v0->position.z;

        du1 = v1->texcoord.x - v0->texcoord.x;
        dv1 = v1->texcoord.y - v0->texcoord.y;

        du2 = v2->texcoord.x - v0->texcoord.x;
        dv2 = v2->texcoord.y - v0->texcoord.y;

        f = 1.0 / ((du1 * dv2) - (du2 * dv1));

        tangent.x = f * ((dv2 * e1.x) - (dv1 * e2.x));
        tangent.y = f * ((dv2 * e1.y) - (dv1 * e2.y));
        tangent.z = f * ((dv2 * e1.z) - (dv1 * e2.z));

        v0->tangent = tangent;
     }

   /* Coupling between vertices by calculation of tangent parametr correct value. */
   for (i = 0; i <= precision; i++)
     {
        for (j = 0; j <= precision; j++)
          {
             if (j == precision)
               {
                  vertex *v;
                  v = &vertices[(i * (precision  + 1)) + j];
                  v->tangent = vertices[i * (precision + 1)].tangent;
               }
          }
     }
}

static void
_restart_level(void *data, Evas_Object *btn, void *ev)
{
   Eina_List *l = NULL;
   Eo *item = NULL;
   Scene_Data *scene = (Scene_Data *)data;

   EINA_LIST_FOREACH(scene->items, l, item)
     {
        evas_canvas3d_node_member_del(scene->root_node, item);
        scene->items = eina_list_remove_list(scene->items, l);
     }

   EINA_LIST_FOREACH(nodes, l, item)
     scene->items = eina_list_append(scene->items, item);

   EINA_LIST_FOREACH(scene->items, l, item)
     evas_canvas3d_node_member_add(scene->root_node, item);

   ball_position_x    = 0.0;
   ball_position_y    = -1.0;
   ball_position_z    = 0.0;
   ball_vel_x         = 0.0;
   ball_vel_y         = 0.0;
   ball_vel_z         = 0.0;
   ball_ac_x          = 0.0;
   ball_ac_y          = GE;
   ball_ac_z          = 0.0;
   plane_alpha_x      = 0.0;
   plane_alpha_z      = 0.0;
   score = 0;

   reload = EINA_TRUE;
   game = EINA_TRUE;
}

static void
_key_down(void *data,
         Evas *e EINA_UNUSED,
         Evas_Object *eo EINA_UNUSED,
         void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Scene_Data *scene = (Scene_Data *)data;

   if (reload)
     {
        if (!strcmp(ev->key, "w"))
          {
             plane_alpha_z -= ANGLE_IT;
             ball_ac_z = sin(plane_alpha_z);
          }
        if (!strcmp(ev->key, "s"))
          {
             plane_alpha_z += ANGLE_IT;
             ball_ac_z = sin(plane_alpha_z);
          }
        if (!strcmp(ev->key, "d"))
          {
             plane_alpha_x += ANGLE_IT;
             ball_ac_x = sin(plane_alpha_x);
          }
        if (!strcmp(ev->key, "a"))
          {
             plane_alpha_x -= ANGLE_IT;
             ball_ac_x = sin(plane_alpha_x);
          }
        if (!strcmp(ev->key, "space"))
          {
             ball_vel_y += KE;
             ball_position_y += 0.1;
             jump = EINA_TRUE;
             ++hight;
          }
        if (!strcmp(ev->key, "Up"))
          {
             if (camera_y > 26.5)
               {
                  camera_y -= 0.5;
                  camera_z -= 0.5 * (7.0 / 4.0);
               }
          }
        if (!strcmp(ev->key, "Down"))
          {
             if (camera_y < 100.0)
               {
                  camera_y += 0.5;
                  camera_z += 0.5 * (7.0 / 4.0);
               }
          }
        if (!strcmp(ev->key, "n"))
          {
             camera_y = 40.0;
             camera_z = 70.0;
          }
        if (!strcmp(ev->key, "Return"))
          {
             elm_layout_signal_emit(btn_restart, "elm,anim,activate", "elm");
             _restart_level(scene, NULL, NULL);
          }
     }
}

static void
_quit_game(void *data EINA_UNUSED, Evas_Object *btn EINA_UNUSED, void *ev EINA_UNUSED)
{
   elm_exit();
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   evas_canvas3d_camera_projection_perspective_set(data->camera, 50.0, 1.0, 1.0, 1000.0);

   data->camera_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 0.0, 40, 70);
   evas_canvas3d_node_look_at_set(data->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);

   evas_canvas3d_node_member_add(data->root_node, data->camera_node);
}

static Eina_Bool
_animate_camera(void *data)
{
   Scene_Data *scene = (Scene_Data *)data;

   evas_canvas3d_node_position_set(scene->camera_node, 0.0, camera_y, camera_z);

   return EINA_TRUE;
}

static void
_light_setup(Scene_Data *data)
{
   data->light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);

   evas_canvas3d_light_ambient_set(data->light, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_light_diffuse_set(data->light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(data->light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_projection_perspective_set(data->light, 100.0, 1.0, 1.0, 1000.0);

   data->light_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, 0.1, 25.0, 0.1);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);

   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}

static void
_mesh_cube_setup(Scene_Data *data)
{
   data->mesh_cube = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   data->material_cube = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   evas_canvas3d_material_enable_set(data->material_cube, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_cube, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_cube, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material_cube, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(data->material_cube, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(data->material_cube, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material_cube, 50.0);

   evas_canvas3d_mesh_vertex_count_set(data->mesh_cube, 24);
   evas_canvas3d_mesh_frame_add(data->mesh_cube, 0);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_cube, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, 12 * sizeof(float), &cube_vertices[ 0]);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_cube, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, 12 * sizeof(float), &cube_vertices[ 3]);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_cube, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, 12 * sizeof(float), &cube_vertices[ 6]);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_cube, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, 12 * sizeof(float), &cube_vertices[10]);
   evas_canvas3d_mesh_index_data_set(data->mesh_cube, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_cube, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_cube, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh_cube, 0, data->material_cube);

}
static void
_create_bondar(Scene_Data *data, Evas_Real scale_x, Evas_Real scale_y, Evas_Real scale_z, Evas_Real x, Evas_Real y, Evas_Real z, Eina_Bool color)
{
   Eo *mesh = NULL;
   Eo *node = NULL;
   Eo *material = NULL;

   mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(material, 50.0);

   if (color)
     evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.0, 0.7, 1.0, 1.0);
   else
     evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.0, 1.0, 1.0, 1.0);

   evas_canvas3d_mesh_vertex_count_set(mesh, 24);
   evas_canvas3d_mesh_frame_add(mesh, 0);
   evas_canvas3d_mesh_frame_vertex_data_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, 12 * sizeof(float), &cube_vertices[ 0]);
   evas_canvas3d_mesh_frame_vertex_data_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, 12 * sizeof(float), &cube_vertices[ 3]);
   evas_canvas3d_mesh_frame_vertex_data_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, 12 * sizeof(float), &cube_vertices[ 6]);
   evas_canvas3d_mesh_frame_vertex_data_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, 12 * sizeof(float), &cube_vertices[10]);
   evas_canvas3d_mesh_index_data_set(mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]);
   evas_canvas3d_mesh_vertex_assembly_set(mesh, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(mesh, 0, material);

   evas_canvas3d_node_member_add(data->root_node, node);

   evas_canvas3d_node_mesh_add(node, mesh);
   evas_canvas3d_node_position_set(node, x, y, z);
   evas_canvas3d_node_scale_set(node, scale_x, scale_y, scale_z);

}
static void
_create_cubes(Scene_Data *data, Evas_Real r, int count)
{
   int i;
   Evas_Real alpha;
   Evas_Real d_alpha;

   data->items = NULL;
   d_alpha = 360 / (double)count;

   for (i = 0; i < count; i++)
     {
        Eo *node = NULL;

        alpha = d_alpha * i * M_PI / 180;
        node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

        evas_canvas3d_node_mesh_add(node, data->mesh_cube);
        evas_canvas3d_node_position_set(node, r * sin(alpha), 2.0 + (r / 4 * sin(alpha)), r * cos(alpha));
        evas_canvas3d_node_scale_set(node, 1.0, 1.0, 1.0);

        evas_canvas3d_node_member_add(data->root_node, node);

        data->items = eina_list_append(data->items, node);
     }
}

static void
_mesh_ball_setup(Scene_Data *data)
{
   _sphere_init(100);

   data->mesh_ball = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   data->material_ball = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   data->mesh_node_ball = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   /* Setup material for ball. */
   evas_canvas3d_material_enable_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(data->material_ball, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material_ball, 50.0);

   /* Setup mesh for ball. */
   evas_canvas3d_mesh_vertex_count_set(data->mesh_ball, vertex_count);
   evas_canvas3d_mesh_frame_add(data->mesh_ball, 0);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_ball, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, sizeof(vertex), &vertices[0].position);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_ball, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, sizeof(vertex), &vertices[0].normal);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_ball, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT, sizeof(vertex), &vertices[0].tangent);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_ball, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, sizeof(vertex), &vertices[0].color);
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_ball, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, sizeof(vertex), &vertices[0].texcoord);
   evas_canvas3d_mesh_index_data_set(data->mesh_ball, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &indices[0]);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_ball, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_ball, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh_ball, 0, data->material_ball);

   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_ball);

   evas_canvas3d_node_mesh_add(data->mesh_node_ball, data->mesh_ball);
}

static void
_scene_setup(Scene_Data *data, Evas_Real r, int count)
{
   Eina_List *l = NULL;
   Eo *item = NULL;
   data->scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   /* Add the root node for the scene. */
   data->root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   evas_canvas3d_scene_size_set(data->scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_shadows_enable_set(data->scene, EINA_TRUE);
   evas_canvas3d_scene_background_color_set(data->scene, 0.0, 0.0, 0.0, 0.0);

   _camera_setup(data);
   _light_setup(data);
   _mesh_ball_setup(data);
   _mesh_cube_setup(data);

   _create_cubes(data, r, count);
   _create_bondar(data, 20.0, 0.001, 20.0, 0.0, -2.0, 0.0, 1);
   _create_bondar(data, 0.5, 0.5, 20.5, -20.0, -1.5, 0.0, 0);
   _create_bondar(data, 0.5, 0.5, 20.5, 20.0, -1.5, 0.0, 0);
   _create_bondar(data, 20.5, 0.5, 0.5, 0.0, -1.5, -20.0, 0);
   _create_bondar(data, 20.5, 0.5, 0.5, 0.0, -1.5, 20.0, 0);

   EINA_LIST_FOREACH(data->items, l, item)
     nodes = eina_list_append(nodes, item);

   evas_canvas3d_scene_root_node_set(data->scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(data->scene, data->camera_node);
}

char score_buffer[32];

static char *
_score_get(int sc)
{
   if (sc >= 0 && sc <= 10)
     {
        eina_convert_itoa(sc, score_buffer);
        return score_buffer;
     }
   return "0";
}

static Eina_Bool
_animate_ball(void *data)
{
   Scene_Data *scene = (Scene_Data *)data;
   Eina_List *l = NULL;
   Eo *node = NULL;
   Evas_Real x, y, z;
   Evas_Real xx, yy, zz;
   Evas_Real d_x, d_y, d_z;
   int i = 9;

   if (scene->items && game)
     {
        ball_vel_x = ball_ac_x;
        ball_vel_z = ball_ac_z;
        ball_position_x += ball_vel_x;
        ball_position_z += ball_vel_z;
     }

   if (reload == EINA_TRUE)
     elm_object_signal_emit(layout, "play", "efl_game");

   evas_canvas3d_node_position_set(scene->mesh_node_ball, ball_position_x, ball_position_y, ball_position_z);

   if ((ball_position_x < -21.0) || (ball_position_x > 21.0) || (ball_position_z < -21.0) || (ball_position_z > 21.0))
     {
        ball_position_y -= 2.0;
        ball_vel_x = 0;
        ball_vel_z = 0;
        game = EINA_FALSE;

        if (ball_position_y <= -200.0)
          {
             ball_position_x = 0.0;
             ball_position_y = -1.0;
             ball_position_z = 0.0;

             elm_object_signal_emit(layout, "defeat", "efl_game");
             reload = EINA_FALSE;
          }
     }
   else if ((ball_position_x >= 18.5) && (ball_position_x <= 21.5))
     {
        ball_position_y = 0.0;
     }
   else if ((ball_position_z >= 18.5) && (ball_position_z <= 21.5))
     {
        ball_position_y = 0.0;
     }
   else if ((ball_position_x <= -18.5) && (ball_position_x >= -21.5))
     {
        ball_position_y = 0.0;
     }
   else if ((ball_position_z <= -18.5) && (ball_position_z >= -21.5))
     {
        ball_position_y = 0.0;
     }
   else if (ball_position_y > -1.0)
     {
        ball_vel_y += ball_ac_y;
        ball_position_y += ball_vel_y;
     }
   else
     {
        ball_vel_y = 0;
        ball_position_y = -1.0;
        if (jump)
          {
             ball_position_y = -0.9;
             ball_vel_y += (1.0 / 5 * hight);
             jump = EINA_FALSE;
             hight = 1;
          }
     }

   evas_canvas3d_node_position_get(scene->mesh_node_ball, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);

   EINA_LIST_FOREACH(scene->items, l, node)
     {
        evas_canvas3d_node_position_get(node, EVAS_CANVAS3D_SPACE_PARENT, &xx, &yy, &zz);

        d_x = xx - x;
        d_y = yy - y;
        d_z = zz - z;

        if (d_x < 2.0 && d_x > -2.0 && d_y < 2.0 && d_y > -2.0 && d_z < 2.0 && d_z > -2.0)
          {
             evas_canvas3d_node_member_del(scene->root_node, node);
             scene->items = eina_list_remove_list(scene->items, l);

             i--;
             score++;
          }
     }
   if (!scene->items)
     {
        elm_object_signal_emit(layout, "victory", "efl_game");
        ball_vel_x = 0;
        ball_vel_z = 0;
        reload = EINA_FALSE;
     }

   evas_object_text_text_set(text, _score_get(score));

   return EINA_TRUE;
}

static Eina_Bool
_animate_cubes(void *data)
{
   static float angle = 0.0f;
   Eina_List *l = NULL;
   Eo *cube = NULL;

   Scene_Data *scene = (Scene_Data *)data;

   angle += 0.5;

   EINA_LIST_FOREACH(scene->items, l, cube)
     evas_canvas3d_node_orientation_angle_axis_set(cube, angle, 1.0, 1.0, 1.0);

   /* Rotate */
   if (angle > 360.0)
     angle -= 360.0f;

   return EINA_TRUE;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Scene_Data data;
   Eina_Bool r = EINA_TRUE;

   elm_config_accel_preference_set("3d");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("__WIN__", "Sphere Hunter");
   elm_win_autodel_set(win, EINA_TRUE);

   layout = elm_layout_add(win);
   elm_layout_file_set(layout, "sphere_hunter.edj", "game");
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);

   text = evas_object_text_add(win);
   evas_object_text_font_set(text, "Sans", 75);
   elm_layout_content_set(layout, "swallow.score_text", text);
   evas_object_show(text);

   btn_restart = elm_button_add(win);
   elm_layout_content_set(layout, "swallow.reload", btn_restart);
   elm_object_text_set(btn_restart, "R E S T A R T     L E V E L");
   evas_object_show(btn_restart);

   btn_quit = elm_button_add(win);
   elm_layout_content_set(layout, "swallow.exit", btn_quit);
   elm_object_text_set(btn_quit, "E X I T");
   evas_object_show(btn_quit);

   evas = evas_object_evas_get(win);

   _scene_setup(&data, 10.0, 10);

   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_visible_set(image, EINA_TRUE);
   efl_canvas_scene3d_set(image, data.scene);

   evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_layout_content_set(layout, "swallow.scene", image);
   evas_object_show(layout);

   r &= evas_object_key_grab(win, "w", 0, 0, EINA_TRUE);
   r &= evas_object_key_grab(win, "s", 0, 0, EINA_TRUE);
   r &= evas_object_key_grab(win, "d", 0, 0, EINA_TRUE);
   r &= evas_object_key_grab(win, "a", 0, 0, EINA_TRUE);
   r &= evas_object_key_grab(win, "n", 0, 0, EINA_TRUE);
   r &= evas_object_key_grab(win, "Up", 0, 0, EINA_TRUE);
   r &= evas_object_key_grab(win, "Down", 0, 0, EINA_TRUE);
   r &= evas_object_key_grab(win, "space", 0, 0, EINA_TRUE);

   elm_object_focus_set(layout, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_KEY_DOWN, _key_down, &data);
   evas_object_smart_callback_add(btn_restart, "clicked", _restart_level, &data);
   evas_object_smart_callback_add(btn_quit, "clicked", _quit_game, NULL);

   ecore_animator_frametime_set(0.008);
   anim_cube = ecore_animator_add(_animate_cubes, &data);
   anim_camera = ecore_animator_add(_animate_camera, &data);
   anim_ball = ecore_animator_add(_animate_ball, &data);

   evas_object_show(win);

   elm_run();

   ecore_animator_del(anim_cube);
   ecore_animator_del(anim_camera);
   ecore_animator_del(anim_ball);

   _sphere_fini();

   return 0;
}
ELM_MAIN()
