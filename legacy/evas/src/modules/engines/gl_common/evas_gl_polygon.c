#include "evas_gl_private.h"

#ifdef _WIN32
# define EFL_STDCALL __stdcall
#else
# define EFL_STDCALL
#endif

#define GLU_TESS

Evas_GL_Polygon *
evas_gl_common_poly_point_add(Evas_GL_Polygon *poly, int x, int y)
{
   Evas_GL_Polygon_Point *pt;

   if (!poly) poly = calloc(1, sizeof(Evas_GL_Polygon));
   if (!poly) return NULL;
   pt = calloc(1, sizeof(Evas_GL_Polygon_Point));
   if (!pt) return NULL;
   pt->x = x;
   pt->y = y;
   poly->points = eina_list_append(poly->points, pt);
   poly->changed = 1;
   return poly;
}

Evas_GL_Polygon *
evas_gl_common_poly_points_clear(Evas_GL_Polygon *poly)
{
   if (!poly) return NULL;
   while (poly->points)
     {
	Evas_GL_Polygon_Point *pt;

	pt = poly->points->data;
	poly->points = eina_list_remove(poly->points, pt);
	free(pt);
     }
   if (poly->dl > 0) glDeleteLists(poly->dl, 1);
   free(poly);
   return NULL;
}

#ifdef GLU_TESS
static void EFL_STDCALL _evas_gl_tess_begin_cb(GLenum which);
static void EFL_STDCALL _evas_gl_tess_end_cb(void);
static void EFL_STDCALL _evas_gl_tess_error_cb(GLenum errorcode);
static void EFL_STDCALL _evas_gl_tess_vertex_cb(GLvoid *vertex);
static void EFL_STDCALL _evas_gl_tess_combine_cb(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **data_out);

static void EFL_STDCALL
_evas_gl_tess_begin_cb(GLenum which)
{
   glBegin(which);
}

static void EFL_STDCALL
_evas_gl_tess_end_cb(void)
{
   glEnd();
}

static void EFL_STDCALL
_evas_gl_tess_error_cb(GLenum errorcode __UNUSED__)
{
}

static void EFL_STDCALL
_evas_gl_tess_vertex_cb(GLvoid *vertex)
{
   GLdouble *v;

   v = vertex;
   glVertex2d(v[0], v[1]);
}

static void EFL_STDCALL
_evas_gl_tess_combine_cb(GLdouble coords[3], GLdouble *vertex_data[4] __UNUSED__, GLfloat weight[4] __UNUSED__, GLdouble **data_out)
{
   GLdouble *vertex;

   vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));
   vertex[0] = coords[0];
   vertex[1] = coords[1];
   *data_out = vertex;
}
#endif

void
evas_gl_common_poly_draw(Evas_GL_Context *gc, Evas_GL_Polygon *poly)
{
   int r, g, b, a;
   Eina_List *l;
   static void *tess = NULL;
   GLdouble *glp = NULL;
   int i, num;
   RGBA_Draw_Context *dc = gc->dc;
   Evas_GL_Polygon_Point *p;

   a = (dc->col.col >> 24) & 0xff;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   evas_gl_common_context_color_set(gc, r, g, b, a);
   if (a < 255) evas_gl_common_context_blend_set(gc, 1);
   else evas_gl_common_context_blend_set(gc, 0);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   evas_gl_common_context_texture_set(gc, NULL, 0, 0, 0);
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);

   if (poly->changed || poly->dl <= 0)
     {
	if (poly->dl > 0) glDeleteLists(poly->dl, 1);
	poly->dl = glGenLists(1);

	glNewList(poly->dl, GL_COMPILE_AND_EXECUTE);
#ifdef GLU_TESS
	if (!tess)
	  {
	     tess = gluNewTess();

	     gluTessCallback(tess, GLU_TESS_BEGIN, _evas_gl_tess_begin_cb);
	     gluTessCallback(tess, GLU_TESS_END, _evas_gl_tess_end_cb);
	     gluTessCallback(tess, GLU_TESS_ERROR, _evas_gl_tess_error_cb);
	     gluTessCallback(tess, GLU_TESS_VERTEX, _evas_gl_tess_vertex_cb);
	     gluTessCallback(tess, GLU_TESS_COMBINE, _evas_gl_tess_combine_cb);
	  }
	num = 0;
	num = eina_list_count(poly->points);
	i = 0;
	glp = malloc(num * 6 * sizeof(GLdouble));
	gluTessNormal(tess, 0, 0, 1);
	gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
	gluTessBeginPolygon(tess, NULL);
	gluTessBeginContour(tess);
	EINA_LIST_FOREACH(poly->points, l, p)
	  {
	     glp[i++] = p->x;
	     glp[i++] = p->y;
	     glp[i++] = 0;
	     gluTessVertex(tess, &(glp[i - 3]), &(glp[i - 3]));
	     i += 3;
	  }
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	free(glp);
#else
	glBegin(GL_POLYGON);
	EINA_LIST_FOREACH(poly->points, l, p)
	  glVertex2i(p->x, p->y);
	glEnd();
#endif
	glEndList();

	poly->changed = 0;

	return ;
     }

   glCallList(poly->dl);
}
