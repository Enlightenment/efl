#ifndef EVAS_ENGINE_DFB_H
#define EVAS_ENGINE_DFB_H
#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine_api_directfb.h"
#include "Evas_Engine_DirectFB.h"
#include "evas_engine_dfb_image_objects.h"

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Tilebuf            *tb;
   Tilebuf_Rect       *rects;
   Evas_Object_List   *cur_rect;
   IDirectFB          *dfb;
   IDirectFBSurface   *surface;
   IDirectFBSurface   *backbuf;	/* do we need an outbuf beyond this? */
   RGBA_Image         *rgba_image;
   int                 end:1;
};

void               *evas_engine_directfb_info(Evas * evas);
void                evas_engine_directfb_info_free(Evas *e, void *info);
void                evas_engine_directfb_setup(Evas * evas, void *in);
void               *evas_engine_directfb_output_setup(int w, int h,
						      IDirectFB * dfb,
						      IDirectFBSurface * surf,
						      DFBSurfaceDrawingFlags
						      flags);
void                evas_engine_directfb_output_free(void *data);
void                evas_engine_directfb_output_resize(void *data, int w,
						       int h);
void                evas_engine_directfb_output_tile_size_set(void *data, int w,
							      int h);
void                evas_engine_directfb_output_redraws_rect_add(void *data,
								 int x, int y,
								 int w, int h);
void                evas_engine_directfb_output_redraws_rect_del(void *data,
								 int x, int y,
								 int w, int h);
void                evas_engine_directfb_output_redraws_clear(void *data);
void               *evas_engine_directfb_output_redraws_next_update_get(void
									*data,
									int *x,
									int *y,
									int *w,
									int *h,
									int *cx,
									int *cy,
									int *cw,
									int
									*ch);
void                evas_engine_directfb_output_redraws_next_update_push(void
									 *data,
									 void
									 *surface,
									 int x,
									 int y,
									 int w,
									 int h);
void                evas_engine_directfb_output_flush(void *data);
void               *evas_engine_directfb_context_new(void *data);
void                evas_engine_directfb_context_free(void *data,
						      void *context);
void                evas_engine_directfb_context_clip_set(void *data,
							  void *context, int x,
							  int y, int w, int h);
void                evas_engine_directfb_context_clip_clip(void *data,
							   void *context, int x,
							   int y, int w, int h);
void                evas_engine_directfb_context_clip_unset(void *data,
							    void *context);
int                 evas_engine_directfb_context_clip_get(void *data,
							  void *context, int *x,
							  int *y, int *w,
							  int *h);
void                evas_engine_directfb_context_color_set(void *data,
							   void *context, int r,
							   int g, int b, int a);
int                 evas_engine_directfb_context_color_get(void *data,
							   void *context,
							   int *r, int *g,
							   int *b, int *a);
void                evas_engine_directfb_context_multiplier_set(void *data,
								void *context,
								int r, int g,
								int b, int a);
void                evas_engine_directfb_context_multiplier_unset(void *data,
								  void
								  *context);
int                 evas_engine_directfb_context_multiplier_get(void *data,
								void *context,
								int *r, int *g,
								int *b, int *a);
void                evas_engine_directfb_context_cutout_add(void *data,
							    void *context,
							    int x, int y, int w,
							    int h);
void                evas_engine_directfb_context_cutout_clear(void *data,
							      void *context);
void                evas_engine_directfb_draw_rectangle(void *data,
							void *context,
							void *surface, int x,
							int y, int w, int h);
void                evas_engine_directfb_line_draw(void *data, void *context,
						   void *surface, int x1,
						   int y1, int x2, int y2);
void               *evas_engine_directfb_polygon_point_add(void *data,
							   void *context,
							   void *polygon, int x,
							   int y);
void               *evas_engine_directfb_polygon_points_clear(void *data,
							      void *context,
							      void *polygon);
void                evas_engine_directfb_polygon_draw(void *data, void *context,
						      void *surface,
						      void *polygon);
void               *evas_engine_directfb_gradient_color_add(void *data,
							    void *context,
							    void *gradient,
							    int r, int g, int b,
							    int a,
							    int distance);
void               *evas_engine_directfb_gradient_colors_clear(void *data,
							       void *context,
							       void *gradient);
void                evas_engine_directfb_gradient_draw(void *data,
						       void *context,
						       void *surface,
						       void *gradient, int x,
						       int y, int w, int h,
						       double angle);
void               *evas_engine_directfb_font_load(void *data, char *name,
						   int size);
void                evas_engine_directfb_font_free(void *data, void *font);
int                 evas_engine_directfb_font_ascent_get(void *data,
							 void *font);
int                 evas_engine_directfb_font_descent_get(void *data,
							  void *font);
int                 evas_engine_directfb_font_max_ascent_get(void *data,
							     void *font);
int                 evas_engine_directfb_font_max_descent_get(void *data,
							      void *font);
void                evas_engine_directfb_font_string_size_get(void *data,
							      void *font,
							      char *text,
							      int *w, int *h);
int                 evas_engine_directfb_font_inset_get(void *data, void *font,
							char *text);
int                 evas_engine_directfb_font_h_advance_get(void *data,
							    void *font,
							    char *text);
int                 evas_engine_directfb_font_v_advance_get(void *data,
							    void *font,
							    char *text);
int                 evas_engine_directfb_font_char_coords_get(void *data,
							      void *font,
							      char *text,
							      int pos, int *cx,
							      int *cy, int *cw,
							      int *ch);
int                 evas_engine_directfb_font_char_at_coords_get(void *data,
								 void *font,
								 char *text,
								 int x, int y,
								 int *cx,
								 int *cy,
								 int *cw,
								 int *ch);
void                evas_engine_directfb_font_draw(void *data, void *context,
						   void *surface, void *font,
						   int x, int y, int w, int h,
						   int ow, int oh, char *text);
void                evas_engine_directfb_font_cache_flush(void *data);
void                evas_engine_directfb_font_cache_set(void *data, int bytes);
int                 evas_engine_directfb_font_cache_get(void *data);

void                rectangle_draw_internal(void *dst, void *dc, int x, int y,
					    int w, int h);

#endif
