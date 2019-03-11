#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif
#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Eina.h>

#include "graphical_struct.h"

#define  FOG_COLOR      0.5, 0.5, 0.5
#define  FOG_FACTOR     0.1

static inline vec3
_normalize(const vec3 *v)
{
    double  l = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    vec3    vec;

    vec.x = v->x / l;
    vec.y = v->y / l;
    vec.z = v->z / l;

    return vec;
}
static void
_sphere_count(int prec, float type_init)
{
   int i, j;

   globalGraphical.vertex_count = (prec + 1) * (prec + 1);

   /* Allocate buffer. */
   if (globalGraphical.places) free(globalGraphical.places);
   globalGraphical.places = malloc(sizeof(place) * globalGraphical.vertex_count);

   for (i = 0; i <= prec; i++)
     {
        double lati = M_PI * (type_init + (double)i * (1 - 2 * type_init) / (double)prec);
        double y = cos(lati);
        double r = fabs(sin(lati));

        for (j = 0; j <= prec; j++)
          {
             double longi = (M_PI * 2.0 * j) / (prec + 1);
             place *v = &globalGraphical.places[i * (prec  + 1) + j];
             v->position.x = r * sin(longi);
             v->position.y = y;
             v->position.z = r * cos(longi);
          }
      }
}
static void
_sphere_init(int prec)
{
   int i, j;
   unsigned short *index;

   globalGraphical.vertex_count = (prec + 1) * (prec + 1);
   globalGraphical.index_count = prec * prec * 6;

   /* Allocate buffer. */
   if (globalGraphical.vertices) free(globalGraphical.vertices);
   if (globalGraphical.indices) free(globalGraphical.indices);

   globalGraphical.vertices = malloc(sizeof(vertex) * globalGraphical.vertex_count);
   globalGraphical.indices = malloc(sizeof(unsigned short) * globalGraphical.index_count);

   for (i = 0; i <= prec; i++)
     {
        double lati = (M_PI * (double)i) / (double)prec;
        double y = cos(lati);
        double r = fabs(sin(lati));

        for (j = 0; j <= prec; j++)
          {
             double longi = (M_PI * 2.0 * j) / prec;
             vertex *v = &globalGraphical.vertices[i * (prec  + 1) + j];

             if (j == 0 || j == prec) v->position.x = 0.0;
             else v->position.x = r * sin(longi);

             v->position.y = y;

             if (j == 0 || j == prec) v->position.z = r;
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

             if (j == prec) v->texcoord.x = 1.0;
             else if (j == 0) v->texcoord.x = 0.0;
             else v->texcoord.x = (double)j / (double)prec;

             if (i == prec) v->texcoord.y = 1.0;
             else if (i == 0) v->texcoord.y = 0.0;
             else v->texcoord.y = 1.0 - (double)i / (double)prec;
          }
     }

   index = &globalGraphical.indices[0];

   for (i = 0; i < prec; i++)
     {
        for (j = 0; j < prec; j++)
          {
             *index++ = i * (prec + 1) + j;
             *index++ = i * (prec + 1) + j + 1;
             *index++ = (i + 1) * (prec + 1) + j;

             *index++ = (i + 1) * (prec + 1) + j;
             *index++ = i * (prec + 1) + j + 1;
             *index++ = (i + 1) * (prec + 1) + j + 1;
          }
     }

   for (i = 0; i < globalGraphical.index_count; i += 3)
     {
        vertex *v0 = &globalGraphical.vertices[globalGraphical.indices[i + 0]];
        vertex *v1 = &globalGraphical.vertices[globalGraphical.indices[i + 1]];
        vertex *v2 = &globalGraphical.vertices[globalGraphical.indices[i + 2]];

        vec3 e1, e2;
        float du1, du2, dv1, dv2, f;
        vec3 tangent;

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

        f = 1.0 / (du1 * dv2 - du2 * dv1);

        tangent.x = f * (dv2 * e1.x - dv1 * e2.x);
        tangent.y = f * (dv2 * e1.y - dv1 * e2.y);
        tangent.z = f * (dv2 * e1.z - dv1 * e2.z);

        v0->tangent = tangent;
     }

   for (i = 0; i <= prec; i++)
     {
        for (j = 0; j <= prec; j++)
          {
             if (j == prec)
               {
                  vertex *v = &globalGraphical.vertices[i * (prec  + 1) + j];
                  v->tangent = globalGraphical.vertices[i * (prec + 1)].tangent;
               }
          }
     }
}
/*FIXME Need to be from bounding of root_node*/
void _init_bounding()
{

   float cube_vertices[] =
     {
        /* Front */
       -50.0,  1.0,  50.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,
        50.0,  1.0,  50.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,
       -50.0, -1.0,  50.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,
        50.0, -1.0,  50.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,

        /* Back */
        50.0,  1.0, -50.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,
       -50.0,  1.0, -50.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,
        50.0, -1.0, -50.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,
       -50.0, -1.0, -50.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,

        /* Left */
       -50.0,  1.0, -50.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,
       -50.0,  1.0,  50.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,
       -50.0, -1.0, -50.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,
       -50.0, -1.0,  50.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,

        /* Right */
        50.0,  1.0,  50.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,
        50.0,  1.0, -50.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,
        50.0, -1.0,  50.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,
        50.0, -1.0, -50.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,

        /* Top */
       -50.0,  1.0, -50.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
        50.0,  1.0, -50.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
       -50.0,  1.0,  50.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
        50.0,  1.0,   50.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

       /* Bottom */
        50.0, -1.0, -50.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,
       -50.0, -1.0, -50.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,
        50.0, -1.0,  50.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,
       -50.0, -1.0,  50.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,
     };

   unsigned short cube_indices[] =
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

   globalGraphical.material_box = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, globalGraphical.evas);

   evas_canvas3d_material_enable_set(globalGraphical.material_box, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(globalGraphical.material_box, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(globalGraphical.material_box, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(globalGraphical.material_box, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(globalGraphical.material_box, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(globalGraphical.material_box, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(globalGraphical.material_box, 100.0);

   /* Setup mesh. */
   globalGraphical.mesh_box = efl_add(EVAS_CANVAS3D_MESH_CLASS, globalGraphical.evas);
   evas_canvas3d_mesh_vertex_count_set(globalGraphical.mesh_box, 24);
   evas_canvas3d_mesh_frame_add(globalGraphical.mesh_box, 0);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh_box, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, 12 * sizeof(float), &cube_vertices[ 0]);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh_box, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, 12 * sizeof(float), &cube_vertices[ 3]);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh_box, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, 12 * sizeof(float), &cube_vertices[ 6]);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh_box, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, 12 * sizeof(float), &cube_vertices[10]);
   evas_canvas3d_mesh_index_data_copy_set(globalGraphical.mesh_box, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]);
   evas_canvas3d_mesh_vertex_assembly_set(globalGraphical.mesh_box, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(globalGraphical.mesh_box, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(globalGraphical.mesh_box, 0, globalGraphical.material_box);

   globalGraphical.mesh_nodebox =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, globalGraphical.evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH), evas_canvas3d_node_position_set(efl_added, 0, -30.0, 0.0));
   evas_canvas3d_node_member_add(globalGraphical.root_node, globalGraphical.mesh_nodebox);
   evas_canvas3d_node_mesh_add(globalGraphical.mesh_nodebox, globalGraphical.mesh_box);
}
static void
_change_scene_setup()
{
   Eo *node = NULL, *m = NULL;
   Eina_List *l = NULL;
   int i = 0;
   int quantity = (globalGraphical.count + 1) * (globalGraphical.count + 1);
   evas_canvas3d_camera_projection_perspective_set(globalGraphical.camera, globalGraphical.angle, 1.0, 2.0, 1000);

   EINA_LIST_FOREACH (globalGraphical.list_nodes, l, node)
     {
        evas_canvas3d_node_member_del(globalGraphical.root_node, node);
        globalGraphical.list_nodes = eina_list_remove(globalGraphical.list_nodes, node);
        /*efl_del(node);Unless evas_canvas3d_destructors work properly*/
     }
   eina_list_free(globalGraphical.list_nodes);
   eina_list_free(l);


   EINA_LIST_FOREACH (globalGraphical.list_meshes, l, m)
     {
        globalGraphical.list_meshes = eina_list_remove(globalGraphical.list_meshes, m);
        /*efl_del(m); Unless evas_canvas3d_destructors work properly*/
     }
   eina_list_free(globalGraphical.list_meshes);
   eina_list_free(l);

   /* Add the mesh with target precision */
   _sphere_init(globalGraphical.precision);
   for (i = 0; i < quantity; i++)
     {
        globalGraphical.mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, globalGraphical.evas);
        if (!globalGraphical.model_path)
          {
              evas_canvas3d_mesh_vertex_count_set(globalGraphical.mesh, globalGraphical.vertex_count);
              evas_canvas3d_mesh_frame_add(globalGraphical.mesh, 0);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, sizeof(vertex), &globalGraphical.vertices[0].position);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, sizeof(vertex), &globalGraphical.vertices[0].normal);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT, sizeof(vertex), &globalGraphical.vertices[0].tangent);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, sizeof(vertex), &globalGraphical.vertices[0].color);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, sizeof(vertex), &globalGraphical.vertices[0].texcoord);
              evas_canvas3d_mesh_index_data_copy_set(globalGraphical.mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, globalGraphical.index_count, &globalGraphical.indices[0]);
              evas_canvas3d_mesh_vertex_assembly_set(globalGraphical.mesh, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
              evas_canvas3d_mesh_shader_mode_set(globalGraphical.mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
           }
         else
           {
              efl_file_simple_load(globalGraphical.mesh, globalGraphical.model_path, NULL);
              evas_canvas3d_mesh_frame_material_set(globalGraphical.mesh, 0, globalGraphical.material);
              evas_canvas3d_mesh_shader_mode_set(globalGraphical.mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
           }

         if (globalGraphical.flags.fog_enable)
           {
              evas_canvas3d_mesh_fog_enable_set(globalGraphical.mesh, EINA_TRUE);
              evas_canvas3d_mesh_fog_color_set(globalGraphical.mesh, FOG_COLOR, FOG_FACTOR);
           }
         else
           {
              evas_canvas3d_mesh_fog_enable_set(globalGraphical.mesh, EINA_FALSE);
           }
         if (globalGraphical.flags.colorpick_enable)
           evas_canvas3d_mesh_color_pick_enable_set(globalGraphical.mesh, EINA_TRUE);
         else
           evas_canvas3d_mesh_color_pick_enable_set(globalGraphical.mesh, EINA_FALSE);

         if (globalGraphical.flags.blend_enable)
           {
              evas_canvas3d_mesh_blending_enable_set(globalGraphical.mesh, EINA_TRUE);
              evas_canvas3d_mesh_blending_func_set(globalGraphical.mesh, EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA, EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
           }
         else
           evas_canvas3d_mesh_blending_enable_set(globalGraphical.mesh, EINA_FALSE);

         evas_canvas3d_mesh_frame_material_set(globalGraphical.mesh, 0, globalGraphical.material);
         globalGraphical.list_meshes = eina_list_append(globalGraphical.list_meshes, globalGraphical.mesh);
      }

   /*Add target count nodes*/
   _sphere_count(globalGraphical.count, 0.2);
   for (i = 0; i < quantity; i++)
     {
        globalGraphical.mesh_node =
           efl_add(EVAS_CANVAS3D_NODE_CLASS, globalGraphical.evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH), evas_canvas3d_node_position_set(efl_added, globalGraphical.places[i].position.x * 20, globalGraphical.places[i].position.y * 20, globalGraphical.places[i].position.z * 20));
        if (globalGraphical.model_path)
          evas_canvas3d_node_scale_set(globalGraphical.mesh_node, 0.2, 0.2, 0.2);
        evas_canvas3d_node_member_add(globalGraphical.root_node, globalGraphical.mesh_node);
        evas_canvas3d_node_mesh_add(globalGraphical.mesh_node, (Eo*)eina_list_nth(globalGraphical.list_meshes, i));
        globalGraphical.list_nodes = eina_list_append(globalGraphical.list_nodes, globalGraphical.mesh_node);
     }


   if (!globalGraphical.flags.fps_enable)
     ecore_animator_frametime_set(1.0 / (globalGraphical.speed));
   else
     ecore_animator_frametime_set(0.0001);

   if (globalGraphical.flags.shadow_enable)
     evas_canvas3d_scene_shadows_enable_set(globalGraphical.scene, EINA_TRUE);
   else
     evas_canvas3d_scene_shadows_enable_set(globalGraphical.scene, EINA_FALSE);

   if (globalGraphical.flags.colorpick_enable)
     {
        evas_canvas3d_scene_color_pick_enable_set(globalGraphical.scene, EINA_TRUE);
     }
   else
     evas_canvas3d_scene_color_pick_enable_set(globalGraphical.scene, EINA_FALSE);
}

static void
_init_scene(Evas_Object *img)
{
   int i, x, y, w, h;
   int quantity = (globalGraphical.count + 1) * (globalGraphical.count + 1);
   /*Allocate memory - eina_mempool*/

   /* Add a scene object .*/
   globalGraphical.scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, globalGraphical.evas);

   /* Add the root node for the scene. */
   globalGraphical.root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, globalGraphical.evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   /* Add the camera. */
   globalGraphical.camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, globalGraphical.evas);
   evas_canvas3d_camera_projection_perspective_set(globalGraphical.camera, globalGraphical.angle, 1.0, 2.0, 1000.0);

   globalGraphical.camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, globalGraphical.evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(globalGraphical.camera_node, globalGraphical.camera);
   evas_canvas3d_node_position_set(globalGraphical.camera_node, 0.0, 0.0, 25.0);
   evas_canvas3d_node_look_at_set(globalGraphical.camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(globalGraphical.root_node, globalGraphical.camera_node);

   globalGraphical.light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, globalGraphical.evas);
   evas_canvas3d_light_ambient_set(globalGraphical.light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_diffuse_set(globalGraphical.light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(globalGraphical.light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_projection_perspective_set(globalGraphical.light, globalGraphical.angle / 5, 1.0, 1.0, 1000.0);
   evas_canvas3d_light_spot_cutoff_set(globalGraphical.light, globalGraphical.angle / 15);

   globalGraphical.light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, globalGraphical.evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(globalGraphical.light_node, globalGraphical.light);
   evas_canvas3d_node_position_set(globalGraphical.light_node, 0.0, 100.0, 1.0);
   evas_canvas3d_node_look_at_set(globalGraphical.light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(globalGraphical.root_node, globalGraphical.light_node);


   globalGraphical.material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, globalGraphical.evas);
   globalGraphical.texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, globalGraphical.evas);
   efl_file_simple_load(globalGraphical.texture, "target_texture.png", NULL);
   evas_canvas3d_texture_filter_set(globalGraphical.texture, EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR, EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR);
   evas_canvas3d_texture_wrap_set(globalGraphical.texture, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   evas_canvas3d_material_texture_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, globalGraphical.texture);
   evas_canvas3d_material_texture_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, globalGraphical.texture);
   evas_canvas3d_material_enable_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 1.0, 0.2, 0.2, 0.2);
   evas_canvas3d_material_color_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 0.0, 0.0, 0.2);
   evas_canvas3d_material_color_set(globalGraphical.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 0.2);
   /* Add the mesh with target precision */
   _sphere_init(globalGraphical.precision);

   for (i = 0; i < quantity; i++)
     {
        globalGraphical.mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, globalGraphical.evas);
        if (!globalGraphical.model_path)
          {
              evas_canvas3d_mesh_vertex_count_set(globalGraphical.mesh, globalGraphical.vertex_count);
              evas_canvas3d_mesh_frame_add(globalGraphical.mesh, 0);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, sizeof(vertex), &globalGraphical.vertices[0].position);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, sizeof(vertex), &globalGraphical.vertices[0].normal);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT, sizeof(vertex), &globalGraphical.vertices[0].tangent);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, sizeof(vertex), &globalGraphical.vertices[0].color);
              evas_canvas3d_mesh_frame_vertex_data_copy_set(globalGraphical.mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, sizeof(vertex), &globalGraphical.vertices[0].texcoord);
              evas_canvas3d_mesh_index_data_copy_set(globalGraphical.mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, globalGraphical.index_count, &globalGraphical.indices[0]);
              evas_canvas3d_mesh_vertex_assembly_set(globalGraphical.mesh, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
              evas_canvas3d_mesh_shader_mode_set(globalGraphical.mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
           }
         else
           {
              efl_file_simple_load(globalGraphical.mesh, globalGraphical.model_path, NULL);
              evas_canvas3d_mesh_frame_material_set(globalGraphical.mesh, 0, globalGraphical.material);
              evas_canvas3d_mesh_shader_mode_set(globalGraphical.mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
           }
         evas_canvas3d_mesh_frame_material_set(globalGraphical.mesh, 0, globalGraphical.material);
         globalGraphical.list_meshes = eina_list_append(globalGraphical.list_meshes, globalGraphical.mesh);
      }

   /*Add target count meshes*/
   _sphere_count(globalGraphical.count, 0.2);

   for (i = 0; i < quantity; i++)
     {
        globalGraphical.mesh_node =
           efl_add(EVAS_CANVAS3D_NODE_CLASS, globalGraphical.evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH), evas_canvas3d_node_position_set(efl_added, globalGraphical.places[i].position.x * 20, globalGraphical.places[i].position.y * 20, globalGraphical.places[i].position.z * 20));
        if (globalGraphical.model_path)
          evas_canvas3d_node_scale_set(globalGraphical.mesh_node, 0.2, 0.2, 0.2);
        evas_canvas3d_node_member_add(globalGraphical.root_node, globalGraphical.mesh_node);
        evas_canvas3d_node_mesh_add(globalGraphical.mesh_node, (Eo*)eina_list_nth(globalGraphical.list_meshes, i));
        globalGraphical.list_nodes = eina_list_append(globalGraphical.list_nodes, globalGraphical.mesh_node);
     }

  _init_bounding();
   evas_object_geometry_get(img, &x, &y, &w, &h);
   evas_canvas3d_scene_root_node_set(globalGraphical.scene, globalGraphical.root_node);
   evas_canvas3d_scene_camera_node_set(globalGraphical.scene, globalGraphical.camera_node);
   evas_canvas3d_scene_size_set(globalGraphical.scene, w, h);

   ecore_animator_frametime_set(1.0 / (globalGraphical.speed));
}

static Eina_Bool
_xml_attr_data(void *data EINA_UNUSED, const char *key, const char *value)
{
   char *format = NULL, *tmp = NULL, *a = NULL;
   int i;

   if (!strcmp("path", key))
     {
        if (!strcmp("none", value))
          {
             globalGraphical.model_path = NULL;
          }
        else
          {
             globalGraphical.model_path = strdup(value);/*Don't forget update UI*/
             tmp = strdup(globalGraphical.model_path);
             a = strrchr(tmp,'.');
             format = malloc(sizeof (char) * (strlen(a) - 1));
             for (i = 0; i <= (int)strlen(a) - 1; i++)
               format[i] = a[i + 1];
             if (strcmp(format, "md2") && strcmp(format, "obj") && strcmp(format, "ply"))
               {
                  printf("\nUnsupported fromat file\n");
                  globalGraphical.model_path = NULL;
               }
             free(format);
             free(tmp);
          }
     }
   else if (!strcmp("count", key))
     globalGraphical.count = atoi(value);
   else if (!strcmp("speed", key))
     globalGraphical.speed = atoi(value);
   else if (!strcmp("precision", key))
     globalGraphical.precision = atoi(value);
   else if (!strcmp("angle", key))
     globalGraphical.angle = atoi(value);

   return EINA_TRUE;
}

static Eina_Bool
_xml_get_data(void *data EINA_UNUSED, Eina_Simple_XML_Type type, const char *content,
              unsigned offset EINA_UNUSED, unsigned length)
{
   char str[512];
   switch (type)
     {
      case EINA_SIMPLE_XML_OPEN:
        {
           if (!strncmp("modelpath", content, strlen("modelpath")))
             {
                const char *tags = eina_simple_xml_tag_attributes_find(content, length);
                eina_simple_xml_attributes_parse(tags, length - (tags - content), _xml_attr_data, str);
             }
           else if (!strncmp("modelcount", content, strlen("modelcount")))
             {
                const char *tags = eina_simple_xml_tag_attributes_find(content, length);
                eina_simple_xml_attributes_parse(tags, length - (tags - content), _xml_attr_data, str);
             }
           else if (!strncmp("animspeed", content, strlen("animspeed")))
             {
                const char *tags = eina_simple_xml_tag_attributes_find(content, length);
                eina_simple_xml_attributes_parse(tags, length - (tags - content), _xml_attr_data, str);
             }
           else if (!strncmp("sphereprecision", content, strlen("sphereprecision")))
             {
                const char *tags = eina_simple_xml_tag_attributes_find(content, length);
                eina_simple_xml_attributes_parse(tags, length - (tags - content), _xml_attr_data, str);
             }
           else if (!strncmp("perspective", content, strlen("perspective")))
             {
                const char *tags = eina_simple_xml_tag_attributes_find(content, length);
                eina_simple_xml_attributes_parse(tags, length - (tags - content), _xml_attr_data, str);
             }
        }
      default:
        break;
     }

   return EINA_TRUE;
}

static void _init_graphical()
{
   Eina_File *config = NULL;
   char *buffer = NULL;
   config = eina_file_open("initial_config.xml", EINA_FALSE);
   /*Set default values*/
   globalGraphical.count = 1;
   globalGraphical.speed = 10;
   globalGraphical.precision = 100;
   globalGraphical.angle = 120;
   globalGraphical.img = NULL;
   globalGraphical.init_scene = _init_scene;
   globalGraphical.change_scene_setup = _change_scene_setup;
   globalGraphical.flags.shadow_enable = EINA_FALSE;
   globalGraphical.flags.fog_enable = EINA_FALSE;
   globalGraphical.flags.colorpick_enable = EINA_FALSE;
   globalGraphical.flags.blend_enable = EINA_FALSE;
   globalGraphical.flags.fps_enable = EINA_FALSE;
   globalGraphical.model_path = NULL;

   if (!config)
     printf("Could not open initial_config.xml, count=4, speed=10, precision=100, perspective=120");
   else
     {
        buffer = (char*)(eina_file_map_all(config, EINA_FILE_RANDOM));
        eina_simple_xml_parse(buffer, eina_file_size_get(config), EINA_TRUE, _xml_get_data, NULL);
        eina_file_close(config);
     }
}
Evas_Object *
init_graphical_window(Evas_Object *img)
{
   _init_graphical();
   globalGraphical.evas = evas_object_evas_get(img);
   _init_scene(img);
   efl_canvas_scene3d_set(img, globalGraphical.scene);

   return img;
}

