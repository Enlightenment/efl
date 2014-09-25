/*
* This example illustrates the work of different callbacks of events from mouse
* and keyboard.
*
* Next tests are available:
* 1. All animations of scene will stop on mouse click on the background.
* 2. Clicking of any object on scene causes stop of its movement, but if click on
* the moon while it is in front of a planet will stop them both.
* 3. Camera position can be changed by pressing 'down' or 'up' key. Position of
* the camera changes in the respective direction.
* 4. The 'n' key pressing returns camera to its default position.
* 5. One of four types of vertex assembly of planet could be chosen by clicking on
* a number from 1 to 4 on keyboard.

* Compile with:
* gcc -o evas-3d-moon-space evas-3d-moon-space.c -g `pkg-config --libs --cflags evas ecore ecore-evas eo` -lm
*/

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Evas.h>
#include <stdio.h>
#include <math.h>
#include <Eo.h>

#define  WIDTH          1024
#define  HEIGHT         1024

typedef struct _Scene_Data
{
   Eo   *texture_diffuse_moon;
   Eo   *texture_diffuse_planet;
   Eo   *root_node;
   Eo   *camera_node;
   Eo   *light_node;
   Eo   *mesh_node_cube;
   Eo   *mesh_node_planet;
   Eo   *mesh_node_moon;

   Eo   *scene;
   Eo   *camera;
   Eo   *light;
   Eo   *mesh_cube;
   Eo   *mesh_planet;
   Eo   *mesh_moon;
   Eo   *material_planet;
   Eo   *material_moon;
   Eo   *material_cube;
} Scene_Data;

Evas_3D_Vertex_Assembly assembly    = EVAS_3D_VERTEX_ASSEMBLY_LINES;
Ecore_Evas       *ecore_evas        = NULL;
Evas             *evas              = NULL;
Eo               *background        = NULL;
Eo               *image             = NULL;
float            d_angle_planet     = 0.1;
float            d_angle_moon       = 0.4;
float            d_angle_cube       = 0.5;
float            camera_position    = 15.0;
int              d_i                = 1;

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

static int vertex_count = 0;
static vertex *vertices = NULL;
static int index_count = 0;
static unsigned short *indices = NULL;

static inline vec3
_normalize(const vec3 *v)
{
   double l;
   vec3 vec;

   l = sqrt((v->x * v->x) + (v->y * v->y) + (v->z * v->z));

   if (l != 0)
     {
        vec.x = v->x / l;
        vec.y = v->y / l;
        vec.z = v->z / l;
     }

   return vec;
}

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
   unsigned short *index;

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

   index = &indices[0];

   /* Calculate and fill in the buffer of indices,
      prepare stage for gl triangulation. */
   for (i = 0; i < precision; i++)
     {
        for (j = 0; j < precision; j++)
          {
             *index++ = (i * (precision + 1)) + j;
             *index++ = (i * (precision + 1)) + j + 1;
             *index++ = ((i + 1) * (precision + 1)) + j;

             *index++ = ((i + 1) * (precision + 1)) + j;
             *index++ = (i * (precision + 1)) + j + 1;
             *index++ = ((i + 1) * (precision + 1)) + j + 1;
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
}

static void
_key_down(void *data EINA_UNUSED,
         Evas *e EINA_UNUSED,
         Evas_Object *eo EINA_UNUSED,
         void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   if (!strcmp(ev->key, "Up"))
     {
        if (camera_position < 99.0)
        camera_position += 0.5;
     }
   else if (!strcmp(ev->key, "Down"))
     {
        if (camera_position > 5.0)
          camera_position -= 0.5;
     }
   else if (!strcmp(ev->key, "n"))
     camera_position = 15.0;
   else if (!strcmp(ev->key, "1"))
     {
        assembly = EVAS_3D_VERTEX_ASSEMBLY_LINES;
        printf("Vertex assembly = EVAS_3D_VERTEX_ASSEMBLY_LINES\n");
     }
   else if (!strcmp(ev->key, "2"))
     {
        assembly = EVAS_3D_VERTEX_ASSEMBLY_LINE_STRIP;
        printf("Vertex assembly = EVAS_3D_VERTEX_ASSEMBLY_LINE_STRIP\n");
     }
   else if (!strcmp(ev->key, "3"))
     {
        assembly = EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES;
        printf("Vertex assembly = EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES\n");
     }
   else if (!strcmp(ev->key, "4"))
     {
        assembly = EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_FAN;
        printf("Vertex assembly = EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_FAN\n");
     }
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(data->camera,
         evas_3d_camera_projection_perspective_set(30.0, 1.0, 1.0, 100.0));

   data->camera_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                     evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(data->camera_node,
         evas_3d_node_camera_set(data->camera),
         evas_3d_node_position_set(0.0, 0.0, 15.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));

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

   data->light_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(0.0, 0.0, 10.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
}

static void
_mesh_setup(Scene_Data *data)
{
   _sphere_init(100);

   data->mesh_planet = eo_add(EVAS_3D_MESH_CLASS, evas);
   data->mesh_cube = eo_add(EVAS_3D_MESH_CLASS, evas);
   data->mesh_moon = eo_add(EVAS_3D_MESH_CLASS, evas);

   data->material_planet = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   data->material_cube = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   data->material_moon = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   data->texture_diffuse_planet = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   data->texture_diffuse_moon = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   data->mesh_node_planet = eo_add(EVAS_3D_NODE_CLASS, evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   data->mesh_node_cube = eo_add(EVAS_3D_NODE_CLASS, evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   data->mesh_node_moon = eo_add(EVAS_3D_NODE_CLASS, evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   /* Setup material for cube. */
   eo_do(data->material_cube,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(70.0));

   /* Setup material and texture for planet. */
   eo_do(data->texture_diffuse_planet,
         evas_3d_texture_file_set("EarthDiffuse.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_LINEAR, EVAS_3D_TEXTURE_FILTER_LINEAR));

   eo_do(data->material_planet,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture_diffuse_planet),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

   /* Setup material and texture for moon. */
   eo_do(data->texture_diffuse_moon,
         evas_3d_texture_file_set("moon.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_LINEAR, EVAS_3D_TEXTURE_FILTER_LINEAR));

   eo_do(data->material_moon,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture_diffuse_moon),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

   /* Setup mesh for moon. */
   eo_do(data->mesh_moon,
         evas_3d_mesh_vertex_count_set(vertex_count),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                  sizeof(vertex), &vertices[0].position),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                  sizeof(vertex), &vertices[0].normal),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TANGENT,
                  sizeof(vertex), &vertices[0].tangent),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                  sizeof(vertex), &vertices[0].color),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                  sizeof(vertex), &vertices[0].texcoord),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_frame_material_set(0, data->material_moon));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_moon));

   eo_do(data->mesh_node_moon,
         evas_3d_node_mesh_add(data->mesh_moon));

   eo_do(data->mesh_node_moon, evas_3d_node_scale_set(0.2, 0.2, 0.2));

   /* Setup mesh for cube. */
   eo_do(data->mesh_cube,
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
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                              36, &cube_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
         evas_3d_mesh_frame_material_set(0, data->material_cube));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_cube));

   eo_do(data->mesh_node_cube,
         evas_3d_node_mesh_add(data->mesh_cube));

   eo_do(data->mesh_node_cube,
         evas_3d_node_scale_set(0.55, 0.55, 0.55));

   /* Setup mesh for planet. */
   eo_do(data->mesh_planet,
         evas_3d_mesh_vertex_count_set(vertex_count),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                  sizeof(vertex), &vertices[0].position),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                  sizeof(vertex), &vertices[0].normal),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TANGENT,
                  sizeof(vertex), &vertices[0].tangent),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                  sizeof(vertex), &vertices[0].color),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                  sizeof(vertex), &vertices[0].texcoord),
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_LINES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_frame_material_set(0, data->material_planet));

   eo_do(data->root_node,
         evas_3d_node_member_add(data->mesh_node_planet));

   eo_do(data->mesh_node_planet,
         evas_3d_node_mesh_add(data->mesh_planet));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   /* Add the root node for the scene. */
   data->root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   eo_do(data->scene,
         evas_3d_scene_size_set(WIDTH, HEIGHT),
         evas_3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   eo_do(data->scene,
         evas_3d_scene_root_node_set(data->root_node),
         evas_3d_scene_camera_node_set(data->camera_node));
}

static void
_stop_scene(void *data,
            Evas *e EINA_UNUSED,
            Evas_Object *eo EINA_UNUSED,
            void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Evas_Real s, t;
   Scene_Data *d = (Scene_Data *)data;
   Evas_3D_Node *pick;
   const Eina_List *nodes = NULL;
   Eina_List *picked_nodes = NULL;
   int root_count = 0, picked_count = 0;

   eo_do(d->scene,
         evas_3d_scene_pick(ev->canvas.x, ev->canvas.y, &n, &m, &s, &t));

   if (d->mesh_cube == m)
     d_angle_cube = 0;
   else if (d->mesh_moon == m)
     {
        d_angle_moon = 0;
        d_i = 0;
     }
   else if (d->mesh_planet == m)
     d_angle_planet = 0;
   else
     {
        d_angle_planet = 0;
        d_angle_cube = 0;
        d_angle_moon = 0;
        d_i = 0;
     }

   pick = eo_do(d->scene,
                evas_3d_scene_exist(ev->canvas.x,  ev->canvas.y, d->mesh_node_planet));

   if (pick) d_angle_planet = 0;

   nodes = eo_do(d->root_node, evas_3d_node_member_list_get());
   picked_nodes = eo_do(d->scene, evas_3d_scene_pick_member_list_get(ev->canvas.x, ev->canvas.y));

   root_count = eina_list_count(nodes);
   picked_count = eina_list_count(picked_nodes);

   printf("Count of members in the root node = %d,  count of the picked members = %d\n", root_count, picked_count);
}

static void
_play_scene(void *data EINA_UNUSED,
            Evas *e EINA_UNUSED,
            Evas_Object *eo EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   d_angle_planet = 0.1;
   d_angle_cube = 0.5;
   d_angle_moon = 0.4;
   d_i = 1;
}

static Eina_Bool
_animate_planet(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += d_angle_planet;

   eo_do(scene->mesh_node_planet,
         evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0));
   eo_do(scene->mesh_planet,
         evas_3d_mesh_vertex_assembly_set(assembly));

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   return EINA_TRUE;
}

static Eina_Bool
_animate_moon(void *data)
{
   static float angle = 0.0f;
   static float longi = 0.0f;
   static int i = 0;
   Scene_Data *scene = (Scene_Data *)data;

   i += d_i;
   longi = (2 * M_PI * i)/2000;
   angle += d_angle_moon;

   eo_do(scene->mesh_node_moon,
         evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0),
         evas_3d_node_position_set(2.0 * sin(longi), 0.7 * sin(longi), 2.0 * cos(longi)));

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   return EINA_TRUE;
}

static Eina_Bool
_animate_cube(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += d_angle_cube;

   eo_do(scene->mesh_node_cube,
         evas_3d_node_orientation_angle_axis_set(angle, 1.0, 0.0, 1.0));

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   return EINA_TRUE;
}

static Eina_Bool
_animate_camera(void *data)
{
   Scene_Data *scene = (Scene_Data *)data;

   eo_do(scene->camera_node,
         evas_3d_node_position_set(0.0, 0.0, camera_position));

   return EINA_TRUE;
}

int
main(void)
{
   Scene_Data data;

   //Unless Evas 3D supports Software renderer, we set gl backened forcely.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

   /* Add evas objects. */
   background = evas_object_image_filled_add(evas);
   evas_object_image_file_set(background, "bg_space.jpg", NULL),
   evas_object_resize(background, WIDTH, HEIGHT),
   evas_object_show(background);

   image = evas_object_image_filled_add(evas);
   evas_object_resize(image, WIDTH, HEIGHT),
   evas_object_show(image),
   evas_object_focus_set(image, EINA_TRUE);

   eo_do(image,
         evas_obj_image_scene_set(data.scene));

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _stop_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_UP, _play_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _key_down, &data);

   ecore_timer_add(0.016, _animate_cube, &data);
   ecore_timer_add(0.016, _animate_camera, &data);
   ecore_timer_add(0.01, _animate_planet, &data);
   ecore_timer_add(0.01, _animate_moon, &data);

   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
   _sphere_fini();

   return 0;
}
