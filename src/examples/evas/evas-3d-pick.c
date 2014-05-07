#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <math.h>
#include <Evas.h>

#define  WIDTH          400
#define  HEIGHT         400

Ecore_Evas       *ecore_evas	    = NULL;
Evas             *evas		    = NULL;
Evas_Object      *background	    = NULL;
Evas_Object      *image		    = NULL;

Evas_3D_Scene    *scene		    = NULL;
Evas_3D_Node     *root_node	    = NULL;
Evas_3D_Node     *camera_node	    = NULL;
Evas_3D_Camera   *camera	    = NULL;
Evas_3D_Node	 *mesh_node	    = NULL;
Evas_3D_Mesh	 *mesh		    = NULL;
Evas_3D_Material *material	    = NULL;
Evas_3D_Texture	 *texture_diffuse   = NULL;

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;

   angle += 0.3;

   eo_do((Evas_3D_Node *)data,
         evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0));

   /* Rotate */
   if (angle > 360.0)
     angle -= 360.0f;

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

static int		vertex_count = 0;
static vertex	       *vertices = NULL;

static int		index_count = 0;
static unsigned short  *indices = NULL;

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
_sphere_fini()
{
    if (vertices)
	free(vertices);

    if (indices)
	free(indices);
}

static void
_sphere_init(int precision)
{
    int		    i, j;
    unsigned short *index;

    vertex_count = (precision + 1) * (precision + 1);
    index_count = precision * precision * 6;

    /* Allocate buffer. */
    vertices = malloc(sizeof(vertex) * vertex_count);
    indices = malloc(sizeof(unsigned short) * index_count);

    for (i = 0; i <= precision; i++)
    {
	double lati = (M_PI * (double)i) / (double)precision;
	double y = cos(lati);
	double r = fabs(sin(lati));

	for (j = 0; j <= precision; j++)
	{
	    double longi = (M_PI * 2.0 * j) / precision;
	    vertex *v = &vertices[i * (precision  + 1) + j];

	    if (j == 0 || j == precision)
		v->position.x = 0.0;
	    else
		v->position.x = r * sin(longi);

	    v->position.y = y;

	    if (j == 0 || j == precision)
		v->position.z = r;
	    else
		v->position.z = r * cos(longi);

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

	    if (j == precision)
		v->texcoord.x = 1.0;
	    else if (j == 0)
		v->texcoord.x = 0.0;
	    else
		v->texcoord.x = (double)j / (double)precision;

	    if (i == precision)
		v->texcoord.y = 1.0;
	    else if (i == 0)
		v->texcoord.y = 0.0;
	    else
		v->texcoord.y = 1.0 - (double)i / (double)precision;
	}
    }

    index = &indices[0];

    for (i = 0; i < precision; i++)
    {
	for (j = 0; j < precision; j++)
	{
	    *index++ = i * (precision + 1) + j;
	    *index++ = i * (precision + 1) + j + 1;
	    *index++ = (i + 1) * (precision + 1) + j;

	    *index++ = (i + 1) * (precision + 1) + j;
	    *index++ = i * (precision + 1) + j + 1;
	    *index++ = (i + 1) * (precision + 1) + j + 1;
	}
    }

    for (i = 0; i < index_count; i += 3)
    {
	vertex *v0 = &vertices[indices[i + 0]];
	vertex *v1 = &vertices[indices[i + 1]];
	vertex *v2 = &vertices[indices[i + 2]];

	vec3	e1, e2;
	float	du1, du2, dv1, dv2, f;
	vec3	tangent;

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

    for (i = 0; i <= precision; i++)
    {
	for (j = 0; j <= precision; j++)
	{
	    if (j == precision)
	    {
		vertex *v = &vertices[i * (precision  + 1) + j];
		v->tangent = vertices[i * (precision + 1)].tangent;
	    }
	}
    }
}

static void
_on_mouse_down(void	    *data EINA_UNUSED,
	       Evas	    *e    EINA_UNUSED,
	       Evas_Object  *o,
	       void	    *einfo)
{
   Evas_Event_Mouse_Down *ev = einfo;
   Evas_Coord x, y, w, h;
   Evas_Coord obj_x, obj_y;
   int scene_w, scene_h;
   Evas_Real scene_x, scene_y;
   Evas_Real s, t;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Eina_Bool pick;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   obj_x = ev->canvas.x - x;
   obj_y = ev->canvas.y - y;

   eo_do(scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (Evas_Real)w;
   scene_y = obj_y * scene_h / (Evas_Real)h;

   eo_do(scene, pick = evas_3d_scene_pick(scene_x, scene_y, &n, &m, &s, &t));
   if (pick)
     printf("Picked     : ");
   else
     printf("Not picked : ");

    printf("output(%d, %d) canvas(%d, %d) object(%d, %d) scene(%f, %f) texcoord(%f, %f) "
           "node(%p) mesh(%p)\n",
           ev->output.x, ev->output.y,
           ev->canvas.x, ev->canvas.y,
           obj_x, obj_y,
           scene_x, scene_y,
           s, t, n, m);
}

int
main(void)
{
   //Unless Evas 3D supports Software renderer, we set gl backened forcely.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

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
   scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   /* Add the root node for the scene. */
   root_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   /* Add the camera. */
   camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(camera, evas_3d_camera_projection_perspective_set(30.0, 1.0, 1.0, 100.0));

   camera_node =
      eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_3d_node_camera_set(camera),
         evas_3d_node_position_set(0.0, 0.0, 5.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(root_node, evas_3d_node_member_add(camera_node));

   /* Add the cube mesh. */
   _sphere_init(100);

   mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   eo_do(mesh,
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
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES));

   material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   eo_do(mesh, evas_3d_mesh_frame_material_set(0, material));

   texture_diffuse = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(texture_diffuse,
         evas_3d_texture_file_set("EarthDiffuse.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_LINEAR, EVAS_3D_TEXTURE_FILTER_LINEAR));
   eo_do(material,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, texture_diffuse),

         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

   mesh_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(root_node, evas_3d_node_member_add(mesh_node));
   eo_do(mesh_node, evas_3d_node_mesh_add(mesh));
   eo_do(mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));

   /* Set up scene. */
   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));

   /* Add evas objects. */
   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   image = evas_object_image_filled_add(evas);
   evas_object_image_size_set(image, WIDTH, HEIGHT);
   evas_object_image_scene_set(image, scene);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, NULL);

   ecore_timer_add(0.01, _animate_scene, mesh_node);

   printf ("Enter main loop\n");
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
   _sphere_fini();

   return 0;
}
