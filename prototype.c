#define EFL_BETA_API_SUPPORT 1

#define EFL_EO_API_SUPPORT
#define EO_BETA_API
#include <Elementary.h>

#define ERR(fmt, args...) printf("%s %d: " fmt "\n", __func__, __LINE__, ##args)

/////////////////////////////////////////////////
static int g_font = 20;
static Eo* g_font_slider = NULL;
//static Eo* g_radius_slider = NULL;
//static Eo* g_angle_slider = NULL;
static Eo* g_source = NULL;
static Eo* g_preview = NULL;
static Eo* g_line1 = NULL;
static Eo* g_line2 = NULL;
//static Eo* g_ctrl_pt[4];
//static Eo* g_ctrl_line[4];
static Eo* g_vg = NULL;
static int g_pts[4][2] =
{
     {20, 550},
     {260, 300},
     {520, 200},
     {780, 600}
};

Eo *g_win;

/////////////////////////////////////////////////

#define PI 3.14159265
#define Slice_MAX 200
//#define Slice_MAX 3
#define Slice_DEFAULT_CNT 69
//#define Slice_DEFAULT_CNT 4

typedef enum {
     EFL_UI_TEXTPATH_MODE_CIRCLE,
     EFL_UI_TEXTPATH_MODE_POINTS,
     EFL_UI_TEXTPATH_MODE_LAST
} Efl_Ui_TextPath_Draw_Mode;

typedef struct {
     double x;
     double y;
} path_point;

typedef struct _Efl_Ui_TextPath_Segment Efl_Ui_TextPath_Segment;
typedef struct _Efl_Ui_TextPath_Data Efl_Ui_TextPath_Data;


struct _Efl_Ui_TextPath_Segment
{
   EINA_INLIST;
   int length;
   /*double px0, py0;
     double ctrl_x0, ctrl_y0;
     double ctrl_x1, ctrl_y1;
     double px1, py1;*/
   Eina_Bezier bezier;
};

struct _Efl_Ui_TextPath_Data
{
   //Eo* proxies[Slice_MAX];
   Eo *proxy;
   Eo* content;
   int slice;
   Efl_Ui_TextPath_Draw_Mode mode;
   int dir;

   struct {
        double radius;
        double start_angle;
   } circle;

   struct {
        path_point* pts;
        int count;
   } points;

   Eina_Bool autofit: 1;

   //Eina_List *segments;
   Eina_Inlist *segments;
   int total_length;


   Eo *vg;

};


static double
_deg_to_rad(double degree)
{
   return degree / 180 * PI;
}

#if 0
static double
_rad_to_deg(double radian)
{
   return radian * 180 / PI;
}
#endif

static void
_transform_coord(double x, double y, double rad, double cx, double cy,
                 int *tx, int *ty)
{
    //transform: rotate vector x,y with rad angle
    //x,y: initial position
   *tx = (int) ((x * cos(rad) - y * sin(rad)) + cx);
   *ty = (int) ((x * sin(rad) + y * cos(rad)) + cy);
}

/*
static void
_clear_proxies(Efl_Ui_TextPath_Data *pd)
{
   int i = 0;
   for (i = 0; i < Slice_MAX; i++)
     {
        if (pd->proxies[i])
          {
             efl_del(pd->proxies[i]);
             pd->proxies[i] = NULL;
          }
     }
}*/

static void
_draw_test_point(Efl_Ui_TextPath_Data *pd, Evas_Coord x, Evas_Coord y)
{
    //draw all points
    Evas_Coord w = 10, h = 10;
    Evas_Object *img = evas_object_image_filled_add(evas_object_evas_get(pd->content));

    evas_object_image_file_set(img, "bubble.png", NULL);
    efl_gfx_size_set(img, w, w);
    efl_gfx_position_set(img, x - w/2, y - h/2);
    efl_gfx_visible_set(img, EINA_TRUE);

    //center
    if (x == 200 && y == 200)
    {
	efl_gfx_color_set(img, 255, 255, 0, 255);
	printf("%s %d: draw center\n", __func__, __LINE__);
    }
}


static void
_draw_text_on_path(Efl_Ui_TextPath_Data *pd, double px0, double py0, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1, double px1, double py1)
{
   printf("%s %d: in\n", __func__, __LINE__);

   int x = 0, y = 0, w = 0, h = 0;
   int i;
   double u0, u1, v0, v1;
   double dist, t, dt;
   double px, py, px2, py2;
   double rad;
   Eina_Vector2 vec, nvec, vec0, vec1, vec2, vec3;
   Eina_Matrix2 mat;
   Evas *e;
   Evas_Object *proxy;

   e = evas_object_evas_get(pd->content);
   efl_gfx_size_get(pd->content, &w, &h);
   dt = 1.0 / (double) pd->slice;
   dist = ((double) w / (double) pd->slice);
    //Compute Beziers.
   Eina_Bezier bezier;
   eina_bezier_values_set(&bezier, px0, py0,
                                   ctrl_x0, ctrl_y0,
                                   ctrl_x1, ctrl_y1,
                                   px1, py1);

   //length = eina_bezier_length_get(&bezier);
   rad = _deg_to_rad(90);
   eina_matrix2_values_set(&mat, cos(rad), -sin(rad), sin(rad), cos(rad));

   //index 0: v0, v3
   t = 0;
   eina_bezier_point_at(&bezier, t, &px, &py);
   eina_bezier_point_at(&bezier, t + dt, &px2, &py2);

   vec.x = (px2 - px);
   vec.y = (py2 - py);
   eina_vector2_normalize(&nvec, &vec);

   eina_vector2_transform(&vec, &mat, &nvec);
   eina_vector2_normalize(&nvec, &vec);
   eina_vector2_scale(&vec, &nvec, ((double) h) * 0.5);

   vec1.x = (vec.x + px);
   vec1.y = (vec.y + py);
   vec2.x = (-vec.x + px);
   vec2.y = (-vec.y + py);

   //Proxy Object
   proxy = evas_object_image_filled_add(e);
   //if (pd->proxy)
   //    evas_object_del(pd->proxy);
   pd->proxy = proxy;
   efl_gfx_size_set(proxy, w, h);
   evas_object_image_source_set(proxy, pd->content);
   efl_gfx_visible_set(proxy, EINA_TRUE);

   ////
   efl_gfx_position_set(proxy, 50, 50);
   ////

   Evas_Map *map = evas_map_new(pd->slice * 4);


   for (i = 0; i < pd->slice; i++)
     {
        //v0, v3
        vec0.x = vec1.x;
        vec0.y = vec1.y;
        vec3.x = vec2.x;
        vec3.y = vec2.y;

        //v1, v2
        t = ((double) (i + 1) * dt);
        eina_bezier_point_at(&bezier, t, &px, &py);
        eina_bezier_point_at(&bezier, t + dt, &px2, &py2);

        vec.x = (px2 - px);
        vec.y = (py2 - py);
        eina_vector2_normalize(&nvec, &vec);

        eina_vector2_transform(&vec, &mat, &nvec);
        eina_vector2_normalize(&nvec, &vec);
        eina_vector2_scale(&vec, &nvec, ((double) h) * 0.5);

        vec1.x = (vec.x + px);
        vec1.y = (vec.y + py);
        vec2.x = (-vec.x + px);
        vec2.y = (-vec.y + py);


        evas_map_point_coord_set(map, i * 4, (int) vec0.x + x, (int) vec0.y + y, 0);
        evas_map_point_coord_set(map, i * 4 + 1, (int) vec1.x + x, (int) vec1.y + y, 0);
        evas_map_point_coord_set(map, i * 4 + 2, (int) vec2.x + x, (int) vec2.y + y, 0);
        evas_map_point_coord_set(map, i * 4 + 3, (int) vec3.x + x, (int) vec3.y + y, 0);

        //UV
        u0 = (((double) i) * dist);
        u1 = (u0 + dist);
        v0 = (double) 0;
        v1 = (double) h;

        evas_map_point_image_uv_set(map, i * 4, u0, v0);
        evas_map_point_image_uv_set(map, i * 4 + 1, u1, v0);
        evas_map_point_image_uv_set(map, i * 4 + 2, u1, v1);
        evas_map_point_image_uv_set(map, i * 4 + 3, u0, v1);

	if (i < 5)
	{
	    printf("%s %d: map: %d %d :: %d %d :: %d %d :: %d %d\n", __func__, __LINE__, (int)vec0.x + x, (int)vec0.y + y, (int)vec1.x + x, (int)vec1.y + y, (int)vec2.x + x, (int)vec2.y + y, (int)vec3.x + x, (int)vec3.y + y);
	    printf("%s %d: map uv: %.1f %.1f :: %.1f %.1f\n", __func__, __LINE__, u0, v0, u1, v1);
	}
     }

   evas_object_map_enable_set(proxy, EINA_TRUE);
   evas_object_map_set(proxy, map);
   evas_map_free(map);
}


static void
_segment_draw(Efl_Ui_TextPath_Data *pd, int slice_no, double slice_len, int from_w, int to_w, int cmp, Evas_Map map, Eina_Bool final, Eina_Bezier bezier)
{
   printf("%s %d: in\n", __func__, __LINE__);

   int x = 0, y = 0, w = 0, h = 0;
   int i;
   double u0, u1, v0, v1;
   double dist, t, dt;
   double px, py, px2, py2;
   double rad;
   Eina_Vector2 vec, nvec, vec0, vec1, vec2, vec3;
   Eina_Matrix2 mat;
   Evas *e;
   Evas_Object *proxy;

   e = evas_object_evas_get(pd->content);
   efl_gfx_size_get(pd->content, &w, &h);
   dt = 1.0 / (double) pd->slice;
   //dist = ((double) w / (double) pd->slice);
   dist = slice_len;
    //Compute Beziers.

   //length = eina_bezier_length_get(&bezier);
   rad = _deg_to_rad(90);
   eina_matrix2_values_set(&mat, cos(rad), -sin(rad), sin(rad), cos(rad));

   //index 0: v0, v3
   eina_bezier_values_get(bezier, NULL, NULL, NULL, NULL, NULL, NULL, &px2, &py2);
   t = 0;
   eina_bezier_point_at(&bezier, t, &px, &py);
   eina_bezier_point_at(&bezier, t + dt, &px2, &py2);

   vec.x = (px2 - px);
   vec.y = (py2 - py);
   eina_vector2_normalize(&nvec, &vec);

   eina_vector2_transform(&vec, &mat, &nvec);
   eina_vector2_normalize(&nvec, &vec);
   eina_vector2_scale(&vec, &nvec, ((double) h) * 0.5);

   vec1.x = (vec.x + px);
   vec1.y = (vec.y + py);
   vec2.x = (-vec.x + px);
   vec2.y = (-vec.y + py);

   //Proxy Object
   proxy = evas_object_image_filled_add(e);
   //if (pd->proxy)
   //    evas_object_del(pd->proxy);
   pd->proxy = proxy;
   efl_gfx_size_set(proxy, w, h);
   evas_object_image_source_set(proxy, pd->content);
   efl_gfx_visible_set(proxy, EINA_TRUE);

   ////
   efl_gfx_position_set(proxy, 50, 50);
   ////

   //add points to map
   for (i = 0; i < slice_no; i++)
     {
        //v0, v3
        vec0.x = vec1.x;
        vec0.y = vec1.y;
        vec3.x = vec2.x;
        vec3.y = vec2.y;

        //v1, v2
        t = ((double) (i + 1) * dt);
        eina_bezier_point_at(&bezier, t, &px, &py);
        eina_bezier_point_at(&bezier, t + dt, &px2, &py2);

        vec.x = (px2 - px);
        vec.y = (py2 - py);
        eina_vector2_normalize(&nvec, &vec);

        eina_vector2_transform(&vec, &mat, &nvec);
        eina_vector2_normalize(&nvec, &vec);
        eina_vector2_scale(&vec, &nvec, ((double) h) * 0.5);

        vec1.x = (vec.x + px);
        vec1.y = (vec.y + py);
        vec2.x = (-vec.x + px);
        vec2.y = (-vec.y + py);


        evas_map_point_coord_set(map, cmp * 4, (int) vec0.x + x, (int) vec0.y + y, 0);
        evas_map_point_coord_set(map, cmp * 4 + 1, (int) vec1.x + x, (int) vec1.y + y, 0);
        evas_map_point_coord_set(map, cmp * 4 + 2, (int) vec2.x + x, (int) vec2.y + y, 0);
        evas_map_point_coord_set(map, cmp * 4 + 3, (int) vec3.x + x, (int) vec3.y + y, 0);

        //UV
        u0 = (((double) cmp) * dist);
        u1 = (u0 + dist);
        v0 = (double) 0;
        v1 = (double) h;

        evas_map_point_image_uv_set(map, cmp * 4, u0, v0);
        evas_map_point_image_uv_set(map, cmp * 4 + 1, u1, v0);
        evas_map_point_image_uv_set(map, cmp * 4 + 2, u1, v1);
        evas_map_point_image_uv_set(map, cmp * 4 + 3, u0, v1);

	if (i < 5)
	{
	    printf("%s %d: map: %d %d :: %d %d :: %d %d :: %d %d\n", __func__, __LINE__, (int)vec0.x + x, (int)vec0.y + y, (int)vec1.x + x, (int)vec1.y + y, (int)vec2.x + x, (int)vec2.y + y, (int)vec3.x + x, (int)vec3.y + y);
	    printf("%s %d: map uv: %.1f %.1f :: %.1f %.1f\n", __func__, __LINE__, u0, v0, u1, v1);
	}
     }

   //if (final)
     {
        printf("%s %d: final: set map\n", __func__,  __LINE__);
        evas_object_map_enable_set(proxy, EINA_TRUE);
        evas_object_map_set(proxy, map);
        //evas_map_free(map);
     }

}

static void
_bezier_segment_store(Efl_Ui_TextPath_Data *pd, double px0, double py0, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1, double px1, double py1)
{
   Eina_Bezier bz;
   eina_bezier_values_set(&bz, px0, py0,
                          ctrl_x0, ctrl_y0,
                          ctrl_x1, ctrl_y1,
                          px1, py1);
   int len = eina_bezier_length_get(bz);
   Efl_Ui_TextPath_Segment *seg = malloc(sizeof(Efl_Ui_TextPath_Segment));
   if (!seg)
     {
        ERR("Failed to allocate segment");
        return;
     }
   seg->length = len;
   seg->bezier = bz;

   pd->segments = eina_inlist_append(pd->segments, EINA_INLIST_GET(seg));

   pd->total_length += len;
}

static void
_text_on_path_draw(Efl_Ui_TextPath_Data *pd)
{
   Efl_Ui_TextPath_Segment *seg;
   int slice_unit = pd->slice / pd->total_length;
   double slice_len = 1.0 / slice_unit;
   int w, h, w1, w2;
   int remained_w;
   int total_slice;
   int cur_map_point = 0;
   Evas_Map map;

   efl_gfx_size_get(pd->content, &w, &h);
   total_slice = w / slice_len + 1;
   remained_w = w;
   map = evas_map_new(total_slice * 4);
   ERR("total slices: total_slice %d", slice_no);


   //for each segment
   //- calculate number of slices
   //- draw text on that segment
   EINA_INLIST_FOREACH(pd->segments, seg)
     {
        int slice_no;
        int len = seg->length;
        if (remained_w < seg->length)
          {
             len = remained_w;
          }
        slice_no = len * slice_unit;
        w1 = w - remained_w;
        w2 = w1 + len;
        _segment_draw(pd, slice_no, slice_len, w1, w2, cur_map_point, map, EINA_FALSE, seg->bezier);
        cur_map_point += slice_no * 4;
     }
}

static void
_path_draw(Efl_Ui_TextPath_Data *pd)
{
   double px_0, py_0, px_1, py_1, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1;
   //path
   {
       const Efl_Gfx_Path_Command *cmd;
       //const int *cmd;
       const double *points;
       
       printf("%s %d: In\n", __func__, __LINE__);
       efl_gfx_path_get(pd->vg, &cmd, &points);
       if (cmd)
       {
	   int pos = -1;
	   while(*cmd != EFL_GFX_PATH_COMMAND_TYPE_END)
	   {
	       //move point to correct pos
	       //
	       printf("cmd: %d\n", *cmd);
	       if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO)
	       {
		   //pos += 2; //1 point;
		   pos++;
		   px_0 = points[pos]; //points[0];
		   pos++;
		   py_0 = points[pos]; //points[1];
	       }
	       else if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO)
	       {
		   //efl_gfx_path_current_get(pd->vg, &px_0, &py_0);
		   pos++;
		   ctrl_x0 = points[pos]; //points[2];
		   pos++;
		   ctrl_y0 = points[pos]; //points[3];
		   _draw_test_point(pd, px_0, py_0);
		   pos++;
		   ctrl_x1 = points[pos]; //points[4];
		   pos++;
		   ctrl_y1 = points[pos]; //points[5];
		   pos++;
		   px_1 = points[pos]; //points[6];
		   pos++;
		   py_1 = points[pos]; //points[7];
		   _draw_test_point(pd, px_1, py_1);
		   printf("%s %d: points: %.1f, %.1f; %.1f, %.1f; %.1f, %.1f; %.1f, %.1f\n", __func__, __LINE__, px_0, py_0, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1, px_1, py_1);

		   _bezier_segment_store(pd, px_0, py_0, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1, px_1, py_1);


		   //_draw_text_on_path(pd, px_0, py_0, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1, px_1, py_1);

		   px_0 = px_1;
		   py_0 = py_1;

		   //test
		   //break;
	       }
	       cmd++;

	   }
       }
   }
   _text_on_path_draw(pd);

}

static void
_points_draw(Efl_Ui_TextPath_Data *pd)
{
//FOR DEBUG
   int x, y, w, h;
     {
        efl_del(g_vg);
        int w, h;
        efl_gfx_geometry_get(g_preview, &x, &y, &w, &h);

        Eo* vg = evas_object_vg_add(evas_object_evas_get(pd->content));
        g_vg = vg;
        efl_gfx_position_set(vg, x, y);
        efl_gfx_size_set(vg, w, h);
        efl_gfx_visible_set(vg, EINA_TRUE);

        Efl_VG* root = evas_object_vg_root_node_get(vg);
        Efl_VG* shape = evas_vg_shape_add(root);
        evas_vg_node_origin_set(shape, 0, 0);
        evas_vg_shape_append_move_to(shape, g_pts[0][0] - x, g_pts[0][1] - y);
        evas_vg_shape_append_cubic_to(shape, g_pts[1][0] - x, g_pts[1][1] - y,
                                      g_pts[2][0] - x, g_pts[2][1] - y,
                                      g_pts[3][0] - x, g_pts[3][1] - y);
        evas_vg_shape_stroke_width_set(shape, 0.25);
        evas_vg_shape_stroke_color_set(shape, 255, 255, 0, 255);
     }

//////////////
   int i;
   double u0, u1, v0, v1;
   double dist, t, dt;
   double px, py, px2, py2;
   double rad;
   Eina_Vector2 vec, nvec, vec0, vec1, vec2, vec3;
   Eina_Matrix2 mat;
   Eo* proxy;
   Evas* e;

   if (!pd->content) return;

   e = evas_object_evas_get(pd->content);
   efl_gfx_size_get(pd->content, &w, &h);
   dt = 1 / (double) pd->slice;
   dist = ((double) w / (double) pd->slice);

   //Compute Beziers.
   Eina_Bezier bezier;
   eina_bezier_values_set(&bezier, g_pts[0][0] - x, g_pts[0][1] - y,
                                   g_pts[1][0] - x, g_pts[1][1] - y,
                                   g_pts[2][0] - x, g_pts[2][1] - y,
                                   g_pts[3][0] - x, g_pts[3][1] - y);

   //length = eina_bezier_length_get(&bezier);
   rad = _deg_to_rad(90);
   eina_matrix2_values_set(&mat, cos(rad), -sin(rad), sin(rad), cos(rad));

   //index 0: v0, v3
   t = 0;
   eina_bezier_point_at(&bezier, t, &px, &py);
   eina_bezier_point_at(&bezier, t + dt, &px2, &py2);

   vec.x = (px2 - px);
   vec.y = (py2 - py);
   eina_vector2_normalize(&nvec, &vec);

   eina_vector2_transform(&vec, &mat, &nvec);
   eina_vector2_normalize(&nvec, &vec);
   eina_vector2_scale(&vec, &nvec, ((double) h) * 0.5);

   vec1.x = (vec.x + px);
   vec1.y = (vec.y + py);
   vec2.x = (-vec.x + px);
   vec2.y = (-vec.y + py);

   for (i = 0; i < pd->slice; i++)
     {
        //v0, v3
        vec0.x = vec1.x;
        vec0.y = vec1.y;
        vec3.x = vec2.x;
        vec3.y = vec2.y;

        //v1, v2
        t = ((double) (i + 1) * dt);
        eina_bezier_point_at(&bezier, t, &px, &py);
        eina_bezier_point_at(&bezier, t + dt, &px2, &py2);

        vec.x = (px2 - px);
        vec.y = (py2 - py);
        eina_vector2_normalize(&nvec, &vec);

        eina_vector2_transform(&vec, &mat, &nvec);
        eina_vector2_normalize(&nvec, &vec);
        eina_vector2_scale(&vec, &nvec, ((double) h) * 0.5);

        vec1.x = (vec.x + px);
        vec1.y = (vec.y + py);
        vec2.x = (-vec.x + px);
        vec2.y = (-vec.y + py);

        //Proxy Object
        proxy = evas_object_image_filled_add(e);
        //pd->proxies[i] = proxy;
	pd->proxy = proxy;
        efl_gfx_size_set(proxy, w, h);
        evas_object_image_source_set(proxy, pd->content);
        efl_gfx_visible_set(proxy, EINA_TRUE);

        Evas_Map *map = evas_map_new(4);

        evas_map_point_coord_set(map, 0, (int) vec0.x + x, (int) vec0.y + y, 0);
        evas_map_point_coord_set(map, 1, (int) vec1.x + x, (int) vec1.y + y, 0);
        evas_map_point_coord_set(map, 2, (int) vec2.x + x, (int) vec2.y + y, 0);
        evas_map_point_coord_set(map, 3, (int) vec3.x + x, (int) vec3.y + y, 0);

        //UV
        u0 = (((double) i) * dist);
        u1 = (u0 + dist);
        v0 = (double) 0;
        v1 = (double) h;

        evas_map_point_image_uv_set(map, 0, u0, v0);
        evas_map_point_image_uv_set(map, 1, u1, v0);
        evas_map_point_image_uv_set(map, 2, u1, v1);
        evas_map_point_image_uv_set(map, 3, u0, v1);

        evas_object_map_enable_set(proxy, EINA_TRUE);
        evas_object_map_set(proxy, map);
        evas_map_free(map);
     }
}

static void
_circle_draw(Efl_Ui_TextPath_Data *pd)
{
   int x, y, w, h, tx, ty;
   double dist, total_angle, angle, radian, radian_start;
   double center_x, center_y;
   double u0, u1, v0, v1;
   int slice;
   int i;
   Evas *e;
   Eo* proxy;

   if (!pd->content) return;

   efl_gfx_geometry_get(g_preview, &x, &y, &w, &h);
   ///test
   /*x = 200;
   y = 200;
   w = 200;
   h = 200;*/
   ///
   center_x = (double) x + ((double) w * 0.5);
   center_y = (double) y + ((double) h * 0.5);

   /////
   center_x = 500;
   center_y = 200;
   /////

   e = evas_object_evas_get(pd->content);
   efl_gfx_geometry_get(pd->content, &x, &y, &w, &h);

   if (pd->autofit)
     {
        //compute angle
        total_angle = (360 * w) / (2 * PI * (pd->circle.radius - ((double) h)));
        if (total_angle > 360) total_angle = 360;

        //compute optimal slice
        slice = ((pd->slice * total_angle) / 360) + 1;
        if (slice > pd->slice) slice = pd->slice;
     }
   else
     {
        total_angle = 360;
        slice = pd->slice;
     }

   dist = ((double) w / (double) slice);
   angle = ((double) total_angle / (double) slice);
   radian = _deg_to_rad(angle);

   //Compute Start Angle
   if (pd->dir == 1)
     radian_start = _deg_to_rad(pd->circle.start_angle);
   else if (pd->dir == -1)
     radian_start = _deg_to_rad(pd->circle.start_angle - total_angle);
   else
     radian_start = _deg_to_rad(pd->circle.start_angle - (total_angle * 0.5));
   ////
   radian_start = 0.0;
   ////

   //Proxy Object
   //if (pd->proxy)
   //    efl_del(pd->proxy);
   proxy = evas_object_image_filled_add(e);
   //pd->proxy = proxy;
   efl_gfx_size_set(proxy, w, h);
   evas_object_image_source_set(proxy, pd->content);
   efl_gfx_visible_set(proxy, EINA_TRUE);

   Evas_Map *map = evas_map_new(slice * 4);

   ///To reverse direction
   //radian = -radian;
   //reverse map points also (3,2,1,0 instead of 0,1,2,3)
   ////
   for (i = 0; i < slice; i++)
     {
        //Point Coord

        //_transform_coord(0, -(pd->circle.radius + h/2),
        _transform_coord(pd->circle.radius + h/2, 0,
                         (((double) i) * radian) + radian_start,
                         center_x, center_y,
                         &tx, &ty);
        evas_map_point_coord_set(map, i * 4, tx, ty, 0);

        //_transform_coord(0, -(pd->circle.radius + h/2),
        _transform_coord(pd->circle.radius + h/2, 0,
                         (((double) (i + 1)) * radian) + radian_start,
                         center_x, center_y,
                         &tx, &ty);
        evas_map_point_coord_set(map, i * 4 + 1, tx, ty, 0);

        //_transform_coord(0, -(pd->circle.radius - (double) h/2),
        _transform_coord(pd->circle.radius - h/2, 0,
                         (((double) (i + 1)) * radian) + radian_start,
                         center_x, center_y,
                         &tx, &ty);
        evas_map_point_coord_set(map, i * 4 + 2, tx, ty, 0);

        //_transform_coord(0, -(pd->circle.radius - (double) h/2),
        _transform_coord(pd->circle.radius - h/2, 0,
                         (((double) i) * radian) + radian_start,
                         center_x, center_y,
                         &tx, &ty);
        evas_map_point_coord_set(map, i * 4 + 3, tx, ty, 0);

        //UV
        u0 = (((double) i) * dist);
        u1 = (u0 + dist);
        v0 = (double) 0;
        v1 = (double) h;

        evas_map_point_image_uv_set(map, i * 4, u0, v0);
        evas_map_point_image_uv_set(map, i * 4  + 1, u1, v0);
        evas_map_point_image_uv_set(map, i * 4  + 2, u1, v1);
        evas_map_point_image_uv_set(map, i * 4  + 3, u0, v1);

		//break;
     }
   evas_object_map_enable_set(proxy, EINA_TRUE);
   evas_object_map_set(proxy, map);
   evas_map_free(map);
}

static void
_sizing_eval(Efl_Ui_TextPath_Data *pd)
{
   if (!pd->content) return;

   evas_object_move(pd->content, 10, 100);
   //_clear_proxies(pd);
   if (pd->proxy)
   {
       efl_del(pd->proxy);
       pd->proxy = NULL;
   }

   /*if (pd->mode == EFL_UI_TEXTPATH_MODE_CIRCLE)
     _circle_draw(pd);
   else if (pd->mode == EFL_UI_TEXTPATH_MODE_POINTS)
     _points_draw(pd);
   else
     _path_draw(pd);*/

     _circle_draw(pd);
     _path_draw(pd);
//   if (pd->proxies[0]) evas_obj_no_render_set(pd->proxies[0], EINA_TRUE);
}

static Efl_Ui_TextPath_Data*
_Efl_Ui_TextPath_efl_object_constructor(Eo* obj, Efl_Ui_TextPath_Data *pd)
{
   pd = calloc(1, sizeof(Efl_Ui_TextPath_Data));
   pd->slice = Slice_DEFAULT_CNT;
   pd->mode = EFL_UI_TEXTPATH_MODE_LAST;

   return pd;
}

static void
_Efl_Ui_TextPath_efl_object_destructor(Eo* obj, Efl_Ui_TextPath_Data *pd)
{
   //_clear_proxies(pd);
   if (pd->proxy)
   {
       efl_del(pd->proxy);
       pd->proxy = NULL;
   }

   free(pd);
}

static void
_content_resize_cb(void *data, Evas *e, Eo *obj, void *event_info)
{
   _sizing_eval(data);
}

static Eo*
Efl_Ui_TextPath_content_get(Eo* obj, Efl_Ui_TextPath_Data *pd)
{
   return pd->content;
}

static Eina_Bool
Efl_Ui_TextPath_content_set(Eo* obj, Efl_Ui_TextPath_Data *pd, Eo *content)
{
   if (pd->content == content) return EINA_FALSE;

   if (pd->content) efl_del(pd->content);

   pd->content = content;

   evas_object_event_callback_add(content, EVAS_CALLBACK_RESIZE, _content_resize_cb, pd);

   _sizing_eval(pd);

   return EINA_TRUE;
}

/*
static void
Efl_Ui_TextPath_slice_set(Eo* obj, Efl_Ui_TextPath_Data *pd, int slice)
{
   if (slice <= 0) return;
   if (slice >= Slice_MAX) return;
   if (pd->slice == slice) return;
   pd->slice = slice;
   _sizing_eval(pd);
}

static int
Efl_Ui_TextPath_slice_get(Eo* obj, Efl_Ui_TextPath_Data *pd)
{
   return pd->slice;
}
*/
#if 0
static int
Efl_Ui_TextPath_direction_get(Eo* obj, Efl_Ui_TextPath_Data *pd)
{
   return pd->dir;
}
#endif

/*
static void
Efl_Ui_TextPath_circle_get(Eo* obj, Efl_Ui_TextPath_Data *pd, double *radius, double* start_angle)
{
   if (radius) *radius = pd->circle.radius;
   if (start_angle) *start_angle = pd->circle.start_angle;
}*/

#if 0
static double
Efl_Ui_TextPath_circle_length_get(Eo *obj, Efl_Ui_TextPath_Data *pd)
{
   if (!pd->content) return 0;

   int h;
   efl_gfx_size_get(pd->content, NULL, &h);
   if (h == 0) return 0;

   return (2 * (pd->circle.radius - (h / 2)) * PI);
}
#endif

/*
static void
Efl_Ui_TextPath_points_set(Eo* obj, Efl_Ui_TextPath_Data *pd, int pts[][2], int count)
{
   if (count < 0) return;

   pd->mode = EFL_UI_TEXTPATH_MODE_POINTS;

   free(pd->points.pts);
   pd->points.pts = malloc(sizeof(path_point) * count);

   int i = 0;
   for(i = 0; i < count; i++)
     {
        pd->points.pts[i].x = (double) pts[i][0];
        pd->points.pts[i].y = (double) pts[i][1];
     }

   pd->points.count = count;
   _sizing_eval(pd);
}*/

static void
Efl_Ui_TextPath_circle_set(Eo* obj, Efl_Ui_TextPath_Data *pd, double radius, double start_angle)
{
   //Duplicated request.
   /*if (pd->mode == EFL_UI_TEXTPATH_MODE_CIRCLE)
     {
        if ((pd->circle.radius == radius) &&
            (pd->circle.start_angle == start_angle))
          return;
     }*/

   pd->circle.radius = 100; //radius;
   pd->circle.start_angle = start_angle;
   pd->mode = EFL_UI_TEXTPATH_MODE_CIRCLE;
   pd->autofit = EINA_TRUE;


   _sizing_eval(pd);
}

/*
static void
Efl_Ui_TextPath_direction_set(Eo* obj, Efl_Ui_TextPath_Data *pd, int dir)
{
   if ((dir < -1) || (dir > 1)) return;
   if (pd->dir == dir) return;
   pd->dir = dir;

   _sizing_eval(pd);
}

static void
Efl_Ui_TextPath_autofit_enable_set(Eo* obj, Efl_Ui_TextPath_Data *pd, Eina_Bool autofit)
{
   autofit = !!autofit;
   if (pd->autofit == autofit) return;
   pd->autofit = autofit;

   _sizing_eval(pd);
}*/

/*
static void
_update_lines()
{
   int x, y, w, h;
   int x2, y2, w2, h2;

   efl_gfx_geometry_get(g_ctrl_pt[0], &x, &y, &w, &h);
   efl_gfx_geometry_get(g_ctrl_pt[1], &x2, &y2, &w2, &h2);

   evas_object_line_xy_set(g_ctrl_line[0], x + (w/2), y + (h/2),
                           x2 + (w2/2), y2 + (h2/2));

   g_pts[0][0] =  x + (w/2);
   g_pts[0][1] =  y + (h/2);

   efl_gfx_geometry_get(g_ctrl_pt[1], &x, &y, &w, &h);
   efl_gfx_geometry_get(g_ctrl_pt[2], &x2, &y2, &w2, &h2);

   evas_object_line_xy_set(g_ctrl_line[1], x + (w/2), y + (h/2),
                           x2 + (w2/2), y2 + (h2/2));

   g_pts[1][0] =  x + (w/2);
   g_pts[1][1] =  y + (h/2);

   efl_gfx_geometry_get(g_ctrl_pt[2], &x, &y, &w, &h);
   efl_gfx_geometry_get(g_ctrl_pt[3], &x2, &y2, &w2, &h2);

   evas_object_line_xy_set(g_ctrl_line[2], x + (w/2), y + (h/2),
                           x2 + (w2/2), y2 + (h2/2));

   g_pts[2][0] =  x + (w/2);
   g_pts[2][1] =  y + (h/2);

   efl_gfx_geometry_get(g_ctrl_pt[3], &x, &y, &w, &h);
   efl_gfx_geometry_get(g_ctrl_pt[0], &x2, &y2, &w2, &h2);

   evas_object_line_xy_set(g_ctrl_line[3], x + (w/2), y + (h/2),
                           x2 + (w2/2), y2 + (h2/2));

   g_pts[3][0] =  x + (w/2);
   g_pts[3][1] =  y + (h/2);
}*/

/*
static void
ctrl_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   if (1 != (int)(long) evas_object_data_get(obj, "down")) return;

   int w, h;
   efl_gfx_size_get(obj, &w, &h);

   Evas_Event_Mouse_Move *ev = event_info;
   evas_object_move(obj, ev->cur.canvas.x - (w/2), ev->cur.canvas.y - (h/2));

   _update_lines();

   Efl_Ui_TextPath_points_set(NULL, data, g_pts, 4);
}

static void
ctrl_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   evas_object_data_set(obj, "down", (void*) 1);
}

static void
ctrl_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   evas_object_data_set(obj, "down", (void*) 0);
}

static void
_ui_control_pts(Eo* parent, int pts[4][2], int count, Efl_Ui_TextPath_Data *pd)
{
   int x, y, w, h;
   efl_gfx_geometry_get(g_preview, &x, &y, &w, &h);

   int i;
   for (i = 0; i < count; i++)
     {
        Eo* line = evas_object_line_add(evas_object_evas_get(parent));
        g_ctrl_line[i] = line;
        efl_gfx_color_set(line, 255, 0, 0, 255);
        efl_gfx_visible_set(line, EINA_TRUE);

        Eo* obj = evas_object_image_filled_add(evas_object_evas_get(parent));
        g_ctrl_pt[i] = obj;
        efl_gfx_stack_layer_set(obj, 1);
        evas_object_image_file_set(obj, "bubble.png", NULL);
        efl_gfx_size_set(obj, 20, 20);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                       ctrl_mouse_move_cb, pd);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                       ctrl_mouse_down_cb, pd);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                       ctrl_mouse_up_cb, pd);
        efl_gfx_position_set(obj, pts[i][0] - 10, pts[i][1] - 10);
        efl_gfx_visible_set(obj, EINA_TRUE);
     }

   _update_lines();
}*/

static void
content_cb(void *data, Evas_Object *obj, void *event_info)
{
   Eo *content;
   int type = 0;

   if (obj) type = elm_radio_state_value_get(obj);

   if (type == 0)
     {
        //content = evas_object_text_add(evas_object_evas_get(g_source));
        //evas_object_text_font_set(content, "century schoolbook l", g_font);
        //efl_text_set(content, "EFL curved test abc__wow eeek! !@#!$ ^$% *^&%   123590~~~---");
        //content = elm_label_add(g_source);
        content = elm_label_add(g_win);
        elm_object_text_set(content, "abcd&lt;g&gt;jkdg gdgdggqq. More");
	efl_gfx_position_set(content, 0, 0);
	efl_gfx_size_set(content, 150, 50);
        //elm_object_disabled_set(g_font_slider, EINA_FALSE);

     }
   else if (type == 1)
     {
        content = evas_object_image_filled_add(evas_object_evas_get(g_source));
        evas_object_image_file_set(content, "test.jpg", NULL);
        elm_object_disabled_set(g_font_slider, EINA_TRUE);
     }
   else
     {
        content = elm_button_add(obj);
        elm_object_text_set(content, "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 1234567890");

        Eo *img = evas_object_image_filled_add(evas_object_evas_get(g_source));
        evas_object_image_file_set(img, "test.jpg", NULL);
        elm_object_content_set(content, img);
        elm_object_disabled_set(g_font_slider, EINA_TRUE);
     }

   //////
   Eo *root = efl_add(EFL_VG_CONTAINER_CLASS, NULL);
   Eo *vg = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "rect"));
   efl_gfx_path_append_circle(vg, 200, 200, 100);
   Efl_Ui_TextPath_Data *pd = data;
   pd->vg = vg;
   //////
   Efl_Ui_TextPath_circle_set(NULL, pd, 100, 0);
   //////

   //content
   Evas_Coord x, y, w, h;
   efl_gfx_geometry_get(g_source, &x, &y, &w, &h);
   //efl_gfx_position_set(content, x, y);
   //efl_gfx_size_set(content, w, h);
   efl_gfx_visible_set(content, EINA_TRUE);

   Efl_Ui_TextPath_content_set(NULL, data, content);

   /////
   _draw_test_point(pd, 200, 200);
   /////
}


static void
_source_geom_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Eo* content = Efl_Ui_TextPath_content_get(NULL, data);

   Evas_Coord x, y, w, h;
   efl_gfx_geometry_get(obj, &x, &y, &w, &h);
   //efl_gfx_position_set(content, x, y);
   //efl_gfx_size_set(content, w, h);
}

Eo*
_ui_source(Eo* parent, Efl_Ui_TextPath_Data *pd)
{
   Eo *rect = evas_object_rectangle_add(evas_object_evas_get(parent));
   g_source = rect;
   efl_gfx_color_set(rect, 50, 50, 50, 255);
   efl_gfx_size_hint_min_set(rect, 0, 50);
   efl_gfx_size_hint_weight_set(rect,
                                EFL_GFX_SIZE_HINT_EXPAND,
                                0);
   efl_gfx_size_hint_align_set(rect,
                                EFL_GFX_SIZE_HINT_FILL,
                                EFL_GFX_SIZE_HINT_FILL);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_RESIZE, _source_geom_cb,
                                  pd);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOVE, _source_geom_cb,
                                  pd);
   efl_gfx_visible_set(rect, EINA_TRUE);

   return rect;
}

static void
_preview_geom_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Coord x, y, w, h;
   Evas_Coord cx, cy;
   efl_gfx_geometry_get(obj, &x, &y, &w, &h);
   cx = x + (w/2);
   cy = y + (h/2);

   cx = 500;
   cy = 200;
   evas_object_line_xy_set(g_line1, cx - 100, cy, cx + 100, cy);
   evas_object_line_xy_set(g_line2, cx, cy - 100, cx, cy + 100);
}

Eo*
_ui_preview(Eo* parent, Efl_Ui_TextPath_Data *pd)
{
   Eo *rect = evas_object_rectangle_add(evas_object_evas_get(parent));
   g_preview = rect;
   efl_gfx_size_hint_weight_set(rect,
                                EFL_GFX_SIZE_HINT_EXPAND,
                                EFL_GFX_SIZE_HINT_EXPAND);
   efl_gfx_size_hint_align_set(rect,
                                EFL_GFX_SIZE_HINT_FILL,
                                EFL_GFX_SIZE_HINT_FILL);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_RESIZE, _preview_geom_cb,
                                  pd);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOVE, _preview_geom_cb,
                                  pd);
   return rect;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   //elm_config_accel_preference_set("hw:msaa_high");

   //Win
   Eo* win = elm_win_util_standard_add("textonpath", "Text on Path");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);
   g_win = win;

   //Axis
   Eo* line;
   line = evas_object_line_add(evas_object_evas_get(win));
   evas_object_pass_events_set(line, EINA_TRUE);
   efl_gfx_size_set(line, 600, 600);
   efl_gfx_color_set(line, 255, 255, 0, 255);
   efl_gfx_visible_set(line, EINA_TRUE);
   g_line1 = line;

   line = evas_object_line_add(evas_object_evas_get(win));
   evas_object_pass_events_set(line, EINA_TRUE);
   efl_gfx_size_set(line, 600, 600);
   efl_gfx_color_set(line, 255, 0, 0, 255);
   efl_gfx_visible_set(line, EINA_TRUE);
   g_line2 = line;

   //Path
   Efl_Ui_TextPath_Data *pd = _Efl_Ui_TextPath_efl_object_constructor(NULL, NULL);
   Efl_Ui_TextPath_circle_set(NULL, pd, 120, 0);

   //Box
   Eo *box = elm_box_add(win);
   efl_gfx_size_hint_weight_set(box,
                                EFL_GFX_SIZE_HINT_EXPAND,
                                EFL_GFX_SIZE_HINT_EXPAND);
   efl_gfx_size_hint_align_set(box,
                                EFL_GFX_SIZE_HINT_FILL,
                                EFL_GFX_SIZE_HINT_FILL);
   efl_gfx_visible_set(box, EINA_TRUE);
   elm_win_resize_object_add(win, box);

   //elm_box_pack_end(box, _ui_option(box, pd));
   elm_box_pack_end(box, _ui_preview(box, pd));
   elm_box_pack_end(box, _ui_source(box, pd));

   efl_gfx_size_set(win, 800, 800);
   efl_gfx_visible_set(win, EINA_TRUE);

   content_cb(pd, NULL, NULL);

   elm_run();

   _Efl_Ui_TextPath_efl_object_destructor(NULL, pd);

   return 0;
}

ELM_MAIN()
