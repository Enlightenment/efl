#ifndef ECTOR_SOFTWARE_PRIVATE_H_
# define ECTOR_SOFTWARE_PRIVATE_H_

#include "Ector_Software.h"
#include "sw_ft_raster.h"
#include "sw_ft_stroker.h"
#include "../ector_private.h"
#include "draw.h"

typedef struct _Ector_Software_Surface_Data Ector_Software_Surface_Data;
typedef struct _Ector_Software_Thread Ector_Software_Thread;

struct _Ector_Software_Thread
{
   Eina_Thread_Queue *queue;
   Eina_Thread thread;

   SW_FT_Raster  raster;
   SW_FT_Stroker stroker;
};

// Gradient related structure
typedef struct _Software_Gradient_Linear_Data
{
   float x1, y1, x2, y2;
   float dx, dy, l, off;
} Software_Gradient_Linear_Data;

typedef struct _Software_Gradient_Radial_Data
{
   float cx, cy, fx, fy, cradius, fradius;
   float dx, dy, dr, sqrfr, a, inv2a;
   Eina_Bool extended;
} Software_Gradient_Radial_Data;

typedef struct _Ector_Renderer_Software_Gradient_Data
{
   Ector_Software_Surface_Data *surface;
   Ector_Renderer_Gradient_Data *gd;
   union {
      Ector_Renderer_Gradient_Linear_Data *gld;
      Ector_Renderer_Gradient_Radial_Data *grd;
   };
   union {
      Software_Gradient_Linear_Data linear;
      Software_Gradient_Radial_Data radial;
   };
   uint32_t* color_table;

   Eina_Bool alpha;
   int ctable_status;       //Ready for color table?

   // GL atlas CRC cache: avoids re-hashing 4 KB of color_table on every
   // span push when the ramp content has not changed since the previous
   // compute.  Invalidated when the framework regenerates color_table
   // (status cycles through CTABLE_NOT_READY before becoming READY again).
   //
   // Lives here rather than on Span_Collector so the cache survives the
   // collector pool's high-water-mark slot reuse - collectors are
   // recycled across shapes between frames, but gradient_data is stable
   // per-gradient-renderer-object.
   //
   // Zero-initialised automatically: Eo private data is calloc'd by the
   // Eo framework, so cached_ctable_crc_valid starts as EINA_FALSE.
   unsigned int cached_ctable_crc;
   int          cached_ctable_status;   // ctable_status when crc was computed
   Eina_Bool    cached_ctable_crc_valid;
} Ector_Renderer_Software_Gradient_Data;

typedef struct _Shape_Rle_Data
{
   Eina_Rectangle   bbox;
   //ALLOC == SIZE?
   unsigned short   alloc;
   unsigned short   size;
   SW_FT_Span      *spans;// array of Scanlines.
} Shape_Rle_Data;

typedef struct _Clip_Data
{
   Eina_Array           *clips; //Eina_Rectangle
   Shape_Rle_Data       *path;
   unsigned int          enabled : 1;
   unsigned int          type : 1;   //0: rect, 1: path
} Clip_Data;

typedef enum _Span_Data_Type {
  None,
  Solid,
  LinearGradient,
  RadialGradient,
} Span_Data_Type;

// Function pointer type for allocating/reusing per-shape span collectors.
// Set on Span_Data by eng_ector_begin(); called by draw_rle_data() for each
// shape.  Keeps all span_collector_* calls inside the engine module (the only
// translation unit that includes evas_ector_gl_span.h).
typedef void *(*Span_Collector_Alloc_Fn)(void *data, int h,
                                         Span_Data_Type type,
                                         Eina_Bool is_stroke);

typedef struct _Span_Data
{
   // --- hot: touched on every rasterizer callback ---
   Ector_Software_Buffer_Base_Data *raster_buffer;
   SW_FT_SpanFunc   blend;
   SW_FT_SpanFunc   unclipped_blend;

   int              offx, offy;
   Clip_Data        clip;
   Span_Data_Type   type;
   uint32_t         mul_col;
   Efl_Gfx_Render_Op        op;
   union {
      uint32_t color;
      Ector_Renderer_Software_Gradient_Data *gradient;
      Ector_Software_Buffer_Base_Data *buffer;
   };

   // fields used on every draw but not in the innermost span callback
   Ector_Software_Buffer_Base_Data    *comp;
   Efl_Gfx_Vg_Composite_Method comp_method;
   Eina_Matrix3     inv;
   Eina_Bool        fast_matrix;

   // --- cold: span-buffer GL fields ---
   void            *span_collector;             // active collector for current shape
   Eina_Bool        span_is_stroke;             // EINA_TRUE during stroke pass
   SW_FT_SpanFunc   collector_solid;
   SW_FT_SpanFunc   collector_gradient;
   SW_FT_SpanFunc   collector_composite;
   // Callback set by eng_ector_begin() to allocate/reuse per-shape collectors.
   // draw_rle_data() calls this instead of calling span_collector_* directly,
   // keeping the span.h dependency inside the engine module only.
   Span_Collector_Alloc_Fn span_collector_alloc;
   void                   *span_collector_alloc_data; // Ector_Software_Surface_Data*
} Span_Data;

typedef struct _Software_Rasterizer
{
   Span_Data        fill_data;
   //Necessary?:
   Eina_Matrix3    *transform;
   Eina_Rectangle   system_clip;
} Software_Rasterizer;

struct _Ector_Software_Surface_Data
{
   Software_Rasterizer *rasterizer;
   int x;
   int y;
   // Per-shape span collector arrays.  Each entry is a Span_Collector*.
   // Owned by the engine (eng_ector_destroy frees them).  Arrays grow
   // with high-water mark allocation - never shrunk, reallocated on
   // demand when more shapes are drawn in a single VG object.
   void **span_collectors_fill;
   int    span_collectors_fill_count;
   int    span_collectors_fill_alloc;
   void **span_collectors_stroke;
   int    span_collectors_stroke_count;
   int    span_collectors_stroke_alloc;

   // Scratch raster buffer for the GL span path, owned here rather than by
   // the ector buffer.  Handing it to ector_buffer_pixels_set() as a plain
   // pointer sets nofree, which matters because the VG blend path swaps the
   // surface's buffer out and back again: were the buffer to own the
   // allocation, that swap would free it and the restore would reinstate a
   // dangling pointer.  The buffer descriptor also only records the height
   // in use, not the height allocated, so growth is tracked here too.
   void  *span_pixels;
   size_t span_pixels_alloc;

   // GL composite mask for the current eng_ector_begin/end window.
   // Set by _efl_canvas_vg_container_render_pre() when a container has a
   // composite target whose mask was rendered into an FBO via _prepare_comp().
   // Read by eng_ector_end() to fill Span_Pipe_Params.mask_tex for each shape.
   // Cleared to NULL by eng_ector_end() after the draw loop completes.
   void *gl_comp_surface;            // Evas_GL_Image* of the mask FBO, or NULL
   int   gl_comp_method;             // Efl_Gfx_Vg_Composite_Method, 0 = NONE
};


ECTOR_API void  ector_software_surface_gl_comp_set(Ector_Surface *obj, void *gl_surface,
                                                   int comp_method);

int  ector_software_gradient_init(void);
void ector_software_rasterizer_init(Software_Rasterizer *rasterizer);

void ector_software_rasterizer_stroke_set(Ector_Software_Thread *thread, Software_Rasterizer *rasterizer,
                                          double width,
                                          Efl_Gfx_Cap cap_style, Efl_Gfx_Join join_style, Eina_Matrix3 *m, double miterlimit);

void ector_software_rasterizer_transform_set(Software_Rasterizer *rasterizer, Eina_Matrix3 *t);
void ector_software_rasterizer_color_set(Software_Rasterizer *rasterizer, int r, int g, int b, int a);
void ector_software_rasterizer_linear_gradient_set(Software_Rasterizer *rasterizer, Ector_Renderer_Software_Gradient_Data *linear);
void ector_software_rasterizer_radial_gradient_set(Software_Rasterizer *rasterizer, Ector_Renderer_Software_Gradient_Data *radial);
void ector_software_rasterizer_clip_rect_set(Software_Rasterizer *rasterizer, Eina_Array *clips);
void ector_software_rasterizer_clip_shape_set(Software_Rasterizer *rasterizer, Shape_Rle_Data *clip);



Shape_Rle_Data * ector_software_rasterizer_generate_rle_data(Ector_Software_Thread *thread, Software_Rasterizer *rasterizer, SW_FT_Outline *outline);
Shape_Rle_Data * ector_software_rasterizer_generate_stroke_rle_data(Ector_Software_Thread *thread, Software_Rasterizer *rasterizer, SW_FT_Outline *outline, Eina_Bool closePath);

void ector_software_rasterizer_draw_rle_data(Software_Rasterizer *rasterizer,
                                             int x, int y, uint32_t mul_col,
                                             Efl_Gfx_Render_Op op,
                                             Shape_Rle_Data* rle,
                                             Ector_Buffer *comp,
                                             Efl_Gfx_Vg_Composite_Method comp_method);

void ector_software_rasterizer_destroy_rle_data(Shape_Rle_Data *rle);


// Gradient Api
void destroy_color_table(Ector_Renderer_Software_Gradient_Data *gdata);
void fetch_linear_gradient(uint32_t *buffer, Span_Data *data, int y, int x, int length);
void fetch_radial_gradient(uint32_t *buffer, Span_Data *data, int y, int x, int length);

void ector_software_thread_init(Ector_Software_Thread *thread);
void ector_software_thread_shutdown(Ector_Software_Thread *thread);

typedef void (*Ector_Thread_Worker_Cb)(void *data, Ector_Software_Thread *thread);

void ector_software_wait(Ector_Thread_Worker_Cb cb, Eina_Free_Cb done, void *data);
void ector_software_schedule(Ector_Thread_Worker_Cb cb, Eina_Free_Cb done, void *data);

void ector_software_gradient_color_update(Ector_Renderer_Software_Gradient_Data *gdata);

#endif
