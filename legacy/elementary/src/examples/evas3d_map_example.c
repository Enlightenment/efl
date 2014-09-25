#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Elementary.h>

#define WIDTH               1024
#define HEIGHT              1024
#define IMG_SIZE            1024
#define SKYBOX_IMG_WIDTH    4096
#define SKYBOX_IMG_HEIGHT   3072

#define MIN_3D_ZOOM_FACTOR  1.0
#define MAX_3D_ZOOM_FACTOR  4.9
#define MIN_2D_ZOOM_FACTOR  5.0
#define MAX_2D_ZOOM_FACTOR  16.0

#define MAX_CAMERA_DISTANCE 10.0

#define ROUND(a) ((a < 0) ? ((int)((a - 0.05) * 10) * 0.1) : ((int)((a + 0.05) * 10) * 0.1))

typedef struct _mat3
{
   float m[9];
} mat3;

typedef struct _vec4
{
   float x;
   float y;
   float z;
   float w;
} vec4;

typedef struct _vec2
{
   float x;
   float y;
} vec2;

typedef struct _vec3
{
   float x;
   float y;
   float z;
} vec3;

typedef struct _vertex
{
   vec3 position;
   vec3 normal;
   vec3 tangent;
   vec4 color;
   vec3 texcoord;
} vertex;

typedef enum _Map_Dimension_State
{
   MAP_DIMENSION_STATE_2D = 0,
   MAP_DIMENSION_STATE_3D
} Map_Dimension_State;

static Evas            *evas            = NULL;
static Evas_Object     *win             = NULL;
static Evas_Object     *bg              = NULL;
static Evas_Object     *image           = NULL;
static Evas_Object     *map             = NULL;
static Evas_Object     *normal_map      = NULL;
static Evas_Object     *control_layout  = NULL;
static Evas_Object     *hoversel        = NULL;
static Evas_Object     *rotation_toggle = NULL;
static Evas_Object     *light_toggle    = NULL;
static Evas_Object     *zoom_slider     = NULL;
static Evas_Object     *menu            = NULL;
static Elm_Object_Item *menu_it         = NULL;
static Ecore_Animator  *animator        = NULL;

static Evas_3D_Scene    *scene           = NULL;
static Evas_3D_Node     *root_node       = NULL;
static Evas_3D_Node     *camera_node     = NULL;
static Evas_3D_Camera   *camera          = NULL;
static Evas_3D_Node     *light_node      = NULL;
static Evas_3D_Light    *light           = NULL;
static Evas_3D_Node     *mesh_node       = NULL;
static Evas_3D_Mesh     *mesh            = NULL;
static Evas_3D_Material *material        = NULL;
static Evas_3D_Texture  *texture_diffuse = NULL;
static Evas_3D_Texture  *texture_normal  = NULL;

static int             vertex_count = 0;
static int             index_count  = 0;
static vertex         *vertices     = NULL;
static unsigned short *indices      = NULL;

static vec2      mouse_down_pos;
static vec3      camera_pos;
static vec3      camera_up_vec;
static vec3      camera_right_vec;
static double    zoom_factor      = MIN_3D_ZOOM_FACTOR;
static Eina_Bool is_mouse_pressed = EINA_FALSE;
static Map_Dimension_State map_dimension_state = MAP_DIMENSION_STATE_3D;

static const float skybox_vertices[] =
{
   /* Front */
   -40.0,  40.0,  40.0,     0.0,  0.0, -1.0,     1.0, 0.0, 0.0, 1.0,     0.99, 0.334,
   40.0,  40.0,  40.0,     0.0,  0.0, -1.0,     1.0, 0.0, 0.0, 1.0,     0.75, 0.334,
   -40.0, -40.0,  40.0,     0.0,  0.0, -1.0,     1.0, 0.0, 0.0, 1.0,     0.99, 0.666,
   40.0, -40.0,  40.0,     0.0,  0.0, -1.0,     1.0, 0.0, 0.0, 1.0,     0.75, 0.666,

   /* Back */
   40.0,  40.0, -40.0,     0.0,  0.0,  1.0,     0.0, 0.0, 1.0, 1.0,     0.50, 0.334,
   -40.0,  40.0, -40.0,     0.0,  0.0,  1.0,     0.0, 0.0, 1.0, 1.0,     0.25, 0.334,
   40.0, -40.0, -40.0,     0.0,  0.0,  1.0,     0.0, 0.0, 1.0, 1.0,     0.50, 0.666,
   -40.0, -40.0, -40.0,     0.0,  0.0,  1.0,     0.0, 0.0, 1.0, 1.0,     0.25, 0.666,

   /* Left */
   -40.0,  40.0, -40.0,     1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.25, 0.334,
   -40.0,  40.0,  40.0,     1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.01, 0.334,
   -40.0, -40.0, -40.0,     1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.25, 0.666,
   -40.0, -40.0,  40.0,     1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.01, 0.666,

   /* Right */
   40.0,  40.0,  40.0,    -1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.75, 0.334,
   40.0,  40.0, -40.0,    -1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.50, 0.334,
   40.0, -40.0,  40.0,    -1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.75, 0.666,
   40.0, -40.0, -40.0,    -1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.50, 0.666,

   /* Top */
   -40.0,  40.0, -40.0,     0.0, -1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.25, 0.334,
   40.0,  40.0, -40.0,     0.0, -1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.50, 0.334,
   -40.0,  40.0,  40.0,     0.0, -1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.25, 0.01,
   40.0,  40.0,  40.0,     0.0, -1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.50, 0.01,

   /* Bottom */
   40.0, -40.0, -40.0,     0.0,  1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.50, 0.666,
   -40.0, -40.0, -40.0,     0.0,  1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.25, 0.666,
   40.0, -40.0,  40.0,     0.0,  1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.50, 0.99,
   -40.0, -40.0,  40.0,     0.0,  1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.25, 0.99,
};

static const unsigned short skybox_indices[] =
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

static void zoom_factor_set(double new_zoom_factor);

static inline void
vec3_scale(vec3 *out,
           const vec3 *v,
           const double scale)
{
   out->x = scale * v->x;
   out->y = scale * v->y;
   out->z = scale * v->z;
}

static inline void
vec3_normalize(vec3 *out,
               const vec3 *vec)
{
   double length = sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);

   out->x = vec->x / length;
   out->y = vec->y / length;
   out->z = vec->z / length;
}

static inline void
mat3_multiply_vec3(vec3 *out,
                   const mat3 *mat,
                   const vec3 *vec)
{
   out->x = mat->m[0] * vec->x + mat->m[1] * vec->y + mat->m[2] * vec->z;
   out->y = mat->m[3] * vec->x + mat->m[4] * vec->y + mat->m[5] * vec->z;
   out->z = mat->m[6] * vec->x + mat->m[7] * vec->y + mat->m[8] * vec->z;
}

static inline void
vec3_rotate_angle_axis(vec3 *out,
                       const vec3 *vec,
                       const double radian,
                       const vec3 *axis)
{
   mat3   mat;
   double cos_r           = cos(radian);
   double sin_r           = sin(radian);
   double one_minus_cos_r = 1.0 - cos_r;

   mat.m[0] = cos_r + one_minus_cos_r * (axis->x * axis->x);
   mat.m[1] = (-axis->z * sin_r) + one_minus_cos_r * (axis->x * axis->y);
   mat.m[2] = (axis->y * sin_r) + one_minus_cos_r * (axis->x * axis->z);

   mat.m[3] = (axis->z * sin_r) + one_minus_cos_r * (axis->x * axis->y);
   mat.m[4] = cos_r + one_minus_cos_r * (axis->y * axis->y);
   mat.m[5] = (-axis->x * sin_r) + one_minus_cos_r * (axis->y * axis->z);

   mat.m[6] = (-axis->y * sin_r) + one_minus_cos_r * (axis->x * axis->z);
   mat.m[7] = (axis->x * sin_r) + one_minus_cos_r * (axis->y * axis->z);
   mat.m[8] = cos_r + one_minus_cos_r * (axis->z * axis->z);

   mat3_multiply_vec3(out, &mat, vec);
}

static Eina_Bool
animate_scene(void *data)
{
   static float angle = 0.0f;

   angle += 0.05;

   eo_do((Evas_3D_Node *)data,
         evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0));

   /* Rotate */
   if (angle > 360.0)
     angle -= 360.0f;

   return EINA_TRUE;
}

static void
sphere_init(int precision)
{
   int             i, j;
   vertex         *v;
   unsigned short *idx;

   vertex_count = (precision + 1) * (precision + 1);
   index_count = precision * precision * 6;

   /* Allocate buffer. */
   vertices = malloc(sizeof(vertex) * vertex_count);
   indices = malloc(sizeof(unsigned short) * index_count);

   for (i = 0; i <= precision; i++)
     {
        double lati = M_PI * ((double)i / (double)precision);
        double y = cos(lati);
        double r = fabs(sin(lati));

        for (j = 0; j <= precision; j++)
          {
             double longi = (M_PI * 2.0) * ((double)j / (double)precision);
             v = &vertices[i * (precision + 1) + j];

             if (j == 0 || j == precision) v->position.x = 0.0;
             else v->position.x = r * sin(longi);

             v->position.y = y;

             if (j == 0 || j == precision) v->position.z = r;
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

             v->color.x = 1.0;
             v->color.y = 1.0;
             v->color.z = 1.0;
             v->color.w = 1.0;

             if (j == precision) v->texcoord.x = 1.0;
             else if (j == 0) v->texcoord.x = 0.0;
             else v->texcoord.x = (double)j / (double)precision;

             if (i == precision) v->texcoord.y = 1.0;
             else if (i == 0) v->texcoord.y = 0.0;
             else v->texcoord.y = (double)i / (double)precision;
          }
     }

   idx = &indices[0];

   for (i = 0; i < precision; i++)
     {
        for (j = 0; j < precision; j++)
          {
             *idx++ = i * (precision + 1) + j;
             *idx++ = i * (precision + 1) + j + 1;
             *idx++ = (i + 1) * (precision + 1) + j;

             *idx++ = (i + 1) * (precision + 1) + j;
             *idx++ = i * (precision + 1) + j + 1;
             *idx++ = (i + 1) * (precision + 1) + j + 1;
          }
     }

   for (i = 0; i < index_count; i += 3)
     {
        vertex *v0 = &vertices[indices[i + 0]];
        vertex *v1 = &vertices[indices[i + 1]];
        vertex *v2 = &vertices[indices[i + 2]];

        vec3  e1, e2;
        float du1, du2, dv1, dv2, f;
        vec3  tangent;

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
                  v = &vertices[i * (precision  + 1) + j];
                  v->tangent = vertices[i * (precision + 1)].tangent;
               }
          }
     }
}

static void
sphere_fini(void)
{
   if (vertices)
     free(vertices);

   if (indices)
     free(indices);
}

static void
change_to_2d_map(void)
{
   if (map_dimension_state == MAP_DIMENSION_STATE_2D) return;
   map_dimension_state = MAP_DIMENSION_STATE_2D;

   elm_object_style_set(map, "default");
   elm_scroller_policy_set
      (map, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_MANUAL);
   elm_map_paused_set(map, EINA_TRUE);

   eo_do(texture_diffuse, evas_3d_texture_source_visible_set(EINA_TRUE));

   evas_object_hide(image);

   elm_slider_unit_format_set(zoom_slider, "X%1.0f");
   elm_slider_indicator_format_set(zoom_slider, "%1.0f");
   elm_slider_min_max_set(zoom_slider, MIN_2D_ZOOM_FACTOR, MAX_2D_ZOOM_FACTOR);
}

static void
change_to_3d_map(void)
{
   if (map_dimension_state == MAP_DIMENSION_STATE_3D) return;
   map_dimension_state = MAP_DIMENSION_STATE_3D;

   elm_object_style_set(map, "evas3d");
   elm_scroller_policy_set
      (map, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_AUTO_FILL);

   evas_object_show(image);

   elm_slider_unit_format_set(zoom_slider, "X%1.1f");
   elm_slider_indicator_format_set(zoom_slider, "%1.1f");
   elm_slider_min_max_set(zoom_slider, MIN_3D_ZOOM_FACTOR, MAX_3D_ZOOM_FACTOR);
}

static void
zoom_factor_set(double new_zoom_factor)
{
   double        cur_zoom_factor;
   double        s, t;
   double        lon, lat;
   vec3          camera_pos_unit_vec;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Evas_Coord    src_x, src_y;
   Evas_Coord    src_size;

   cur_zoom_factor = zoom_factor;
   zoom_factor = ROUND(new_zoom_factor);

   if (cur_zoom_factor == zoom_factor) return;

   /* 2D Zoom */
   if (zoom_factor >= MIN_2D_ZOOM_FACTOR)
     {
        /* Transition from 3D to 2D */
        if (cur_zoom_factor < MIN_2D_ZOOM_FACTOR)
          {
             zoom_factor = MIN_2D_ZOOM_FACTOR;
             eo_do(scene, evas_3d_scene_pick(IMG_SIZE / 2, IMG_SIZE / 2,
                                             &n, &m, &s, &t));
             src_size = IMG_SIZE * cur_zoom_factor;
             src_x = (Evas_Coord)(src_size * s);
             src_y = (Evas_Coord)(src_size * t);
             elm_map_canvas_to_region_convert(map, src_x, src_y, &lon, &lat);

             change_to_2d_map();
             evas_object_resize(map, IMG_SIZE, IMG_SIZE);
             elm_map_zoom_set(map, (int)zoom_factor);
             elm_map_region_show(map, lon, lat);
          }
        else
          elm_map_zoom_set(map, (int)zoom_factor);
     }
   /* 3D Zoom */
   else
     {
        if (zoom_factor < MIN_3D_ZOOM_FACTOR)
          zoom_factor = MIN_3D_ZOOM_FACTOR;

        /* Transition from 2D to 3D */
        if (cur_zoom_factor >= MIN_2D_ZOOM_FACTOR)
          {
             change_to_3d_map();
          }

        /* Update texture source. */
        src_size = IMG_SIZE * zoom_factor;
        evas_object_resize(map, src_size, src_size);

        /* Apply updated texture source. */
        texture_diffuse = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
        eo_do(texture_diffuse,
              evas_3d_texture_source_set(map),
              evas_3d_texture_source_visible_set(EINA_FALSE));

        eo_do(material,
              evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, texture_diffuse));

        /* Update camera position. */
        vec3_normalize(&camera_pos_unit_vec, &camera_pos);
        camera_pos.x = (camera_pos_unit_vec.x * MAX_CAMERA_DISTANCE) / zoom_factor;
        camera_pos.y = (camera_pos_unit_vec.y * MAX_CAMERA_DISTANCE) / zoom_factor;
        camera_pos.z = (camera_pos_unit_vec.z * MAX_CAMERA_DISTANCE) / zoom_factor;

        eo_do(camera_node,
              evas_3d_node_position_set(camera_pos.x, camera_pos.y, camera_pos.z));
     }

   /* Update zoom slider. */
   elm_slider_value_set(zoom_slider, zoom_factor);
}

static void
zoom_in_cb(void *data,
           Evas_Object *obj,
           void *event_info)
{
   zoom_factor_set(zoom_factor + 1);
}

static void
zoom_out_cb(void *data,
            Evas_Object *obj,
            void *event_info)
{
   zoom_factor_set(zoom_factor - 1);
}

static void
zoom_to_2d_map_cb(void *data,
                  Evas_Object *obj,
                  void *event_info)
{
   double        s, t;
   double        lon, lat;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Evas_Coord    src_x, src_y;
   Evas_Coord    src_size;

   if (map_dimension_state == MAP_DIMENSION_STATE_2D) return;

   eo_do(scene, evas_3d_scene_pick(mouse_down_pos.x, mouse_down_pos.y, &n, &m, &s, &t));
   src_size = (Evas_Coord)(IMG_SIZE * zoom_factor);
   src_x = (Evas_Coord)(src_size * s);
   src_y = (Evas_Coord)(src_size * t);
   elm_map_canvas_to_region_convert(map, src_x, src_y, &lon, &lat);

   zoom_factor_set(MIN_2D_ZOOM_FACTOR);

   elm_map_region_show(map, lon, lat);
}

static void
zoom_to_3d_map_cb(void *data,
                  Evas_Object *obj,
                  void *event_info)
{
   if (map_dimension_state == MAP_DIMENSION_STATE_3D) return;

   zoom_factor_set(MAX_3D_ZOOM_FACTOR);
}

static void
submenu_zoom_add(Elm_Object_Item *parent)
{
   if (!parent) return;

   elm_menu_item_add(menu, parent, NULL, "Zoom in", zoom_in_cb, NULL);
   elm_menu_item_add(menu, parent, NULL, "Zoom out", zoom_out_cb, NULL);
   elm_menu_item_add(menu, parent, NULL, "Zoom to 2D map", zoom_to_2d_map_cb, NULL);
   elm_menu_item_add(menu, parent, NULL, "Zoom to 3D map", zoom_to_3d_map_cb, NULL);
}

static void
submenu_info_add(Elm_Object_Item *parent)
{
   char          buf[50];
   double        s, t;
   double        lon, lat;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Evas_Coord    src_x, src_y;
   Evas_Coord    src_size;

   if (!parent) return;

   if (map_dimension_state == MAP_DIMENSION_STATE_3D)
     {
        eo_do(scene, evas_3d_scene_pick(mouse_down_pos.x, mouse_down_pos.y, &n, &m, &s, &t));
        src_size = (Evas_Coord) (IMG_SIZE * zoom_factor);
        src_x = (Evas_Coord)(src_size * s);
        src_y = (Evas_Coord)(src_size * t);
        elm_map_canvas_to_region_convert(map, src_x, src_y, &lon, &lat);
     }
   else
     {
        elm_map_canvas_to_region_convert(map, mouse_down_pos.x, mouse_down_pos.y, &lon, &lat);
     }

   snprintf(buf, sizeof(buf), "Longitude : %f", lon);
   elm_menu_item_add(menu, parent, NULL, buf, NULL, NULL);

   snprintf(buf, sizeof(buf), "Latitude : %f", lat);
   elm_menu_item_add(menu, parent, NULL, buf, NULL, NULL);
}

static void
map_zoom_change_cb(void *data,
                   Evas_Object *obj,
                   void *event_info)
{
   int cur_zoom_factor;

   if (map_dimension_state == MAP_DIMENSION_STATE_3D) return;

   cur_zoom_factor = elm_map_zoom_get(map);

   if (elm_map_paused_get(map))
     elm_map_paused_set(map, EINA_FALSE);

   zoom_factor_set(cur_zoom_factor);
}

static void
map_mouse_down_cb(void *data,
                  Evas *e,
                  Evas_Object *o,
                  void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord             x, y, w, h;
   Evas_Coord             obj_x, obj_y;
   int                    scene_w, scene_h;
   double                 scene_x, scene_y;
   double                 s, t;
   Evas_3D_Node          *n;
   Evas_3D_Mesh          *m;
   Eina_Bool              pick;

   mouse_down_pos.x = ev->canvas.x;
   mouse_down_pos.y = ev->canvas.y;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   obj_x = ev->canvas.x - x;
   obj_y = ev->canvas.y - y;

   eo_do(scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (double)w;
   scene_y = obj_y * scene_h / (double)h;

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

   if (ev->button == 3)
     {
        if (!menu)
          {
             menu = elm_menu_add(o);
             elm_menu_parent_set(menu, o);
             menu_it = elm_menu_item_add(menu, NULL, "", "Zoom", NULL, NULL);
             submenu_zoom_add(menu_it);
             menu_it = elm_menu_item_add(menu, NULL, "", "Info", NULL, NULL);
          }

        elm_menu_item_subitems_clear(menu_it);
        submenu_info_add(menu_it);

        elm_menu_move(menu, ev->canvas.x, ev->canvas.y);
        evas_object_show(menu);
     }

}

static void
mouse_down_cb(void *data,
              Evas *e,
              Evas_Object *o,
              void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord             x, y, w, h;
   Evas_Coord             obj_x, obj_y;
   int                    scene_w, scene_h;
   double                 scene_x, scene_y;
   double                 s, t;
   Evas_3D_Node          *n;
   Evas_3D_Mesh          *m;
   Eina_Bool              pick;

   mouse_down_pos.x = ev->canvas.x;
   mouse_down_pos.y = ev->canvas.y;

   /* Stop rotating earth. */
   if (animator)
     {
        ecore_animator_del(animator);
        animator = NULL;
     }

   evas_object_geometry_get(o, &x, &y, &w, &h);

   obj_x = ev->canvas.x - x;
   obj_y = ev->canvas.y - y;

   eo_do(scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (double)w;
   scene_y = obj_y * scene_h / (double)h;

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

   if (ev->button == 1)
     is_mouse_pressed = EINA_TRUE;
   else if (ev->button == 3)
     {
        elm_check_state_set(rotation_toggle, EINA_FALSE);
        if (!menu)
          {
             menu = elm_menu_add(o);
             elm_menu_parent_set(menu, o);
             menu_it = elm_menu_item_add(menu, NULL, "", "Zoom", NULL, NULL);
             submenu_zoom_add(menu_it);
             menu_it = elm_menu_item_add(menu, NULL, "", "Info", NULL, NULL);
          }

        elm_menu_item_subitems_clear(menu_it);
        submenu_info_add(menu_it);

        elm_menu_move(menu, ev->canvas.x, ev->canvas.y);
        evas_object_show(menu);
     }
}

static void
mouse_move_cb(void *data,
              Evas *e,
              Evas_Object *o,
              void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   double                 distance_to_origin;
   double                 radian_camera_up_axis, radian_camera_right_axis;
   vec3                   camera_pos_unit_vec;

   if ((ev->buttons == 1) && is_mouse_pressed)
     {
        radian_camera_up_axis = (ev->prev.canvas.x - ev->cur.canvas.x) * 0.01;
        radian_camera_right_axis = (ev->prev.canvas.y - ev->cur.canvas.y) * 0.01;

        distance_to_origin = sqrt(pow(camera_pos.x, 2) + pow(camera_pos.y, 2) + pow(camera_pos.z, 2));

        /* Update camera position, up vector and right vector. */
        vec3_rotate_angle_axis(&camera_pos, &camera_pos, radian_camera_up_axis, &camera_up_vec);
        vec3_rotate_angle_axis(&camera_right_vec, &camera_right_vec, radian_camera_up_axis, &camera_up_vec);
        vec3_normalize(&camera_right_vec, &camera_right_vec);

        vec3_rotate_angle_axis(&camera_pos, &camera_pos, radian_camera_right_axis, &camera_right_vec);
        vec3_rotate_angle_axis(&camera_up_vec, &camera_up_vec, radian_camera_right_axis, &camera_right_vec);
        vec3_normalize(&camera_up_vec, &camera_up_vec);
        vec3_normalize(&camera_pos_unit_vec, &camera_pos);
        vec3_scale(&camera_pos, &camera_pos_unit_vec, distance_to_origin);

        eo_do(camera_node,
              evas_3d_node_position_set(camera_pos.x, camera_pos.y, camera_pos.z),
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                       EVAS_3D_SPACE_PARENT, camera_up_vec.x, camera_up_vec.y, camera_up_vec.z));

        if (elm_check_state_get(light_toggle))
          {
             /* Update light position as the same as camera position. */
             eo_do(light_node,
                   evas_3d_node_position_set(camera_pos.x, camera_pos.y, camera_pos.z),
                   evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                            EVAS_3D_SPACE_PARENT, camera_up_vec.x, camera_up_vec.y, camera_up_vec.z));
          }
     }
}

static void
mouse_up_cb(void *data,
            Evas *e,
            Evas_Object *o,
            void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord           x, y, w, h;
   Evas_Coord           obj_x, obj_y;
   int                  scene_w, scene_h;
   double               scene_x, scene_y;
   double               s, t;
   Evas_3D_Node        *n;
   Evas_3D_Mesh        *m;
   Eina_Bool            pick;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   obj_x = ev->canvas.x - x;
   obj_y = ev->canvas.y - y;

   eo_do(scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (double)w;
   scene_y = obj_y * scene_h / (double)h;

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

   /* Move camera position. */
   if (ev->button == 1)
     is_mouse_pressed = EINA_FALSE;

   if (elm_check_state_get(rotation_toggle))
     {
        /* Restart rotating earth. */
        animator = ecore_animator_add(animate_scene, mesh_node);
     }
}

static void
mouse_wheel_cb(void *data,
               Evas *e,
               Evas_Object *o,
               void *event_info)
{
   Evas_Event_Mouse_Wheel *ev = event_info;
   double                  new_zoom_factor;

   if (map_dimension_state == MAP_DIMENSION_STATE_2D) return;

   new_zoom_factor = zoom_factor - (ev->z * 0.1);
   zoom_factor_set(new_zoom_factor);
}

static void
skybox_setup(void)
{
   Evas_3D_Node     *skybox_mesh_node;
   Evas_3D_Mesh     *skybox_mesh;
   Evas_3D_Material *skybox_material;
   Evas_3D_Texture  *skybox_texture_diffuse;
   Evas_Object      *skybox_image;
   char              buf[PATH_MAX];

   skybox_mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   eo_do(skybox_mesh,
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_frame_add(0),

         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &skybox_vertices[0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float), &skybox_vertices[3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float), &skybox_vertices[6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &skybox_vertices[10]),

         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &skybox_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES));

   /* Set skybox texture source image. */
   skybox_image = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/space.png", elm_app_data_dir_get());
   elm_image_file_set(skybox_image, buf, NULL);
   evas_object_resize(skybox_image, SKYBOX_IMG_WIDTH, SKYBOX_IMG_HEIGHT);
   evas_object_show(skybox_image);

   /* Set skybox texture material. */
   skybox_material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   eo_do(skybox_mesh, evas_3d_mesh_frame_material_set(0, skybox_material));

   skybox_texture_diffuse = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(skybox_texture_diffuse,
         evas_3d_texture_source_set(skybox_image),
         evas_3d_texture_source_visible_set(EINA_FALSE));

   eo_do(skybox_material,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, skybox_texture_diffuse),

         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.1, 0.1, 0.1, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 0.1, 0.1, 0.1, 1.0),
         evas_3d_material_shininess_set(50.0));

   skybox_mesh_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(root_node, evas_3d_node_member_add(skybox_mesh_node));
   eo_do(skybox_mesh_node, evas_3d_node_mesh_add(skybox_mesh));
   eo_do(skybox_mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));
}

static void
texture_source_setup(void)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/examples/evas3d_map_example.edj", elm_app_data_dir_get());
   elm_theme_extension_add(NULL, buf);

   /* Add a map object for the use of the texture source. */
   map = elm_map_add(win);
   elm_object_style_set(map, "evas3d");
   elm_map_source_set(map, ELM_MAP_SOURCE_TYPE_TILE, "MapQuest");
   elm_scroller_policy_set
      (map, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_AUTO_FILL);
   evas_object_resize(map, IMG_SIZE, IMG_SIZE);
   evas_object_show(map);

   evas_object_smart_callback_add(map, "zoom,change",
                                  map_zoom_change_cb, NULL);
   evas_object_event_callback_add(map, EVAS_CALLBACK_MOUSE_DOWN,
                                  map_mouse_down_cb, map);

   /* Add a normal map object for the use of the normal texture source. */
   normal_map = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/earth_normal.png", elm_app_data_dir_get());
   elm_image_file_set(normal_map, buf, NULL);
   evas_object_resize(normal_map, IMG_SIZE, IMG_SIZE);
   evas_object_show(normal_map);
}

static void
camera_setup(void)
{
   /* Add the camera. */
   camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(camera, evas_3d_camera_projection_perspective_set(60.0, 1.0, 1.0, 100.0));

   /* Set camera position. */
   camera_pos.x = 0.0;
   camera_pos.y = 0.0;
   camera_pos.z = MAX_CAMERA_DISTANCE;

   /* Set camera up vector. */
   camera_up_vec.x = 0.0;
   camera_up_vec.y = 1.0;
   camera_up_vec.z = 0.0;

   /* Set camera right vector. */
   camera_right_vec.x = 1.0;
   camera_right_vec.y = 0.0;
   camera_right_vec.z = 0.0;

   camera_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                               evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_3d_node_camera_set(camera),
         evas_3d_node_position_set(camera_pos.x, camera_pos.y, camera_pos.z),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, camera_up_vec.x, camera_up_vec.y, camera_up_vec.z));
   eo_do(root_node, evas_3d_node_member_add(camera_node));
}

static void
light_setup(void)
{
   light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(light,
         evas_3d_light_ambient_set(0.2, 0.2, 0.2, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(0.2, 0.2, 0.2, 1.0));

   light_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                              evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(light_node,
         evas_3d_node_light_set(light),
         evas_3d_node_position_set(0.0, 0.0, MAX_CAMERA_DISTANCE),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(root_node, evas_3d_node_member_add(light_node));
}

static void
mesh_setup(void)
{
   /* Create a sphere mesh. */
   sphere_init(50);

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
         evas_3d_texture_source_set(map),
         evas_3d_texture_source_visible_set(EINA_FALSE));

   texture_normal = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(texture_normal,
         evas_3d_texture_source_set(normal_map),
         evas_3d_texture_source_visible_set(EINA_FALSE));

   eo_do(material,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, texture_diffuse),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL, texture_normal),

         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.1, 0.1, 0.1, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 0.1, 0.1, 0.1, 1.0),
         evas_3d_material_shininess_set(50.0));

   mesh_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(root_node, evas_3d_node_member_add(mesh_node));
   eo_do(mesh_node, evas_3d_node_mesh_add(mesh));
   eo_do(mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP));
}

static void
hoversel_cb(void *data,
            Evas_Object *obj,
            void *event_info)
{
   elm_map_source_set(map, ELM_MAP_SOURCE_TYPE_TILE, (char *)data);
   zoom_factor_set(zoom_factor);
}

static void
hoversel_setup(Evas_Object *parent)
{
   hoversel = elm_hoversel_add(parent);

   elm_hoversel_hover_parent_set(hoversel, parent);
   elm_object_text_set(hoversel, "Map Sources");
   elm_hoversel_item_add(hoversel, "Mapnik", NULL, ELM_ICON_NONE, hoversel_cb, "Mapnik");
   elm_hoversel_item_add(hoversel, "CycleMap", NULL, ELM_ICON_NONE, hoversel_cb, "CycleMap");
   elm_hoversel_item_add(hoversel, "MapQuest", NULL, ELM_ICON_NONE, hoversel_cb, "MapQuest");

   elm_object_part_content_set(parent, "elm.swallow.hoversel", hoversel);
   evas_object_show(hoversel);
}

static void
rotation_toggle_changed_cb(void *data,
                           Evas_Object *obj,
                           void *event_info)
{
   /* Stop rotating earth. */
   if (animator)
     {
        ecore_animator_del(animator);
        animator = NULL;
     }
   if (elm_check_state_get(obj))
     {
        /* Restart rotating earth. */
        animator = ecore_animator_add(animate_scene, mesh_node);
     }
}

static void
rotation_toggle_setup(Evas_Object *parent)
{
   rotation_toggle = elm_check_add(parent);

   elm_object_style_set(rotation_toggle, "toggle");
   elm_object_text_set(rotation_toggle, "Earth Rotation");
   elm_object_part_text_set(rotation_toggle, "on", "ON");
   elm_object_part_text_set(rotation_toggle, "off", "OFF");
   elm_check_state_set(rotation_toggle, EINA_TRUE);

   elm_object_part_content_set(parent, "elm.swallow.rotation_toggle", rotation_toggle);
   evas_object_show(rotation_toggle);

   evas_object_smart_callback_add(rotation_toggle, "changed", rotation_toggle_changed_cb, NULL);
}

static void
light_toggle_changed_cb(void *data,
                        Evas_Object *obj,
                        void *event_info)
{
   if (elm_check_state_get(obj))
     {
        /* Set light position as the same as camera position. */
        eo_do(light_node,
              evas_3d_node_position_set(camera_pos.x, camera_pos.y, camera_pos.z),
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                       EVAS_3D_SPACE_PARENT, camera_up_vec.x, camera_up_vec.y, camera_up_vec.z));
     }
   else
     {
        /* Set light position to default position. */
        eo_do(light_node,
              evas_3d_node_position_set(0.0, 0.0, MAX_CAMERA_DISTANCE),
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                       EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
     }
}

static void
light_toggle_setup(Evas_Object *parent)
{
   light_toggle = elm_check_add(parent);

   elm_object_style_set(light_toggle, "toggle");
   elm_object_text_set(light_toggle, "Light on Camera");
   elm_object_part_text_set(light_toggle, "on", "ON");
   elm_object_part_text_set(light_toggle, "off", "OFF");
   elm_check_state_set(light_toggle, EINA_FALSE);

   elm_object_part_content_set(parent, "elm.swallow.light_toggle", light_toggle);
   evas_object_show(light_toggle);

   evas_object_smart_callback_add(light_toggle, "changed", light_toggle_changed_cb, NULL);
}


static void
slider_changed_cb(void *data,
                  Evas_Object *obj,
                  void *event_info)
{
   double new_zoom_factor = ROUND(elm_slider_value_get(obj));

   if (new_zoom_factor == zoom_factor) return;

   zoom_factor_set(new_zoom_factor);
}

static void
zoom_slider_setup(Evas_Object *parent)
{
   zoom_slider = elm_slider_add(parent);

   elm_object_text_set(zoom_slider, "Zoom Factor");
   elm_slider_unit_format_set(zoom_slider, "X%1.1f");
   elm_slider_indicator_format_set(zoom_slider, "%1.1f");
   elm_slider_span_size_set(zoom_slider, 100);
   elm_slider_min_max_set(zoom_slider, MIN_3D_ZOOM_FACTOR, MAX_3D_ZOOM_FACTOR);

   elm_object_part_content_set(parent, "elm.swallow.zoom_slider", zoom_slider);
   evas_object_show(zoom_slider);

   evas_object_smart_callback_add(zoom_slider, "changed", slider_changed_cb, NULL);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "examples/evas3d_map_example.edj");

   elm_config_preferred_engine_set("opengl_x11");

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   /* Add a window. */
   win = elm_win_util_standard_add("elm_map_evas3d", "Elm_Map_Evas3d");
   elm_win_autodel_set(win, EINA_TRUE);

   evas = evas_object_evas_get(win);

   /* Add a background. */
   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 0, 0, 0, 255);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_show(bg);

   /* Add an image which shows a scene. */
   image = evas_object_image_filled_add(evas);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, mouse_down_cb, NULL);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_MOVE, mouse_move_cb, NULL);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_UP, mouse_up_cb, NULL);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_WHEEL, mouse_wheel_cb, NULL);

   /* Add a scene object. */
   scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   /* Add a root node for the scene. */
   root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   skybox_setup();
   texture_source_setup();
   camera_setup();
   light_setup();
   mesh_setup();

   /* Set up scene. */
   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));
   eo_do(image, evas_obj_image_scene_set(scene));

   /* Add a layout for controlers. */
   control_layout = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/examples/evas3d_map_example.edj", elm_app_data_dir_get());
   elm_layout_file_set(control_layout, buf, "control_layout");
   evas_object_resize(control_layout, WIDTH, HEIGHT);
   evas_object_show(control_layout);

   /* Set up controlers. */
   hoversel_setup(control_layout);
   rotation_toggle_setup(control_layout);
   light_toggle_setup(control_layout);
   zoom_slider_setup(control_layout);

   /* Start rotating earth */
   animator = ecore_animator_add(animate_scene, mesh_node);

   evas_object_resize(win, WIDTH, HEIGHT);
   evas_object_show(win);

   elm_run();
   elm_shutdown();
   sphere_fini();

   return 0;
}
ELM_MAIN();
