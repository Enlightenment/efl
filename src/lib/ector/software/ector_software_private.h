#ifndef ECTOR_SOFTWARE_PRIVATE_H_
# define ECTOR_SOFTWARE_PRIVATE_H_

#include "sw_ft_raster.h"
#include "sw_ft_stroker.h"

#ifndef DATA32
typedef unsigned int DATA32;
#endif

#ifndef uint
typedef unsigned int uint;
#endif

typedef struct _Ector_Software_Surface_Data Ector_Software_Surface_Data;

#define CHECK_SOFTWARE(Parent) (!(Parent && Parent->software))

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
   Ector_Renderer_Generic_Gradient_Data *gd;
   union {
      Ector_Renderer_Generic_Gradient_Linear_Data *gld;
      Ector_Renderer_Generic_Gradient_Radial_Data *grd;
   };
   union {
      Software_Gradient_Linear_Data linear;
      Software_Gradient_Radial_Data radial;
   };
   uint* colorTable;
} Ector_Renderer_Software_Gradient_Data;


// Rasterizer related structure
typedef struct _Raster_Buffer
{
   int            width;
   int            height;
   DATA32        *buffer;
} Raster_Buffer;

typedef struct _Shape_Rle_Data
{
   unsigned short   alloc;
   unsigned short   size;
   SW_FT_Span      *spans;// array of Scanlines.
} Shape_Rle_Data;

typedef struct _Clip_Data
{
   Eina_Array           *clips; //Eina_Rectangle
   Shape_Rle_Data       *path;
   unsigned int          enabled : 1;
   unsigned int          hasRectClip : 1;
   unsigned int          hasPathClip : 1;
} Clip_Data;


typedef enum _Span_Data_Type {
  None,
  Solid,
  LinearGradient,
  RadialGradient,
  Image
} Span_Data_Type;

typedef struct _Span_Data
{
   Raster_Buffer    raster_buffer;

   SW_FT_SpanFunc   blend;
   SW_FT_SpanFunc   unclipped_blend;

   int              offx, offy;
   Clip_Data        clip;
   Eina_Matrix3     inv;
   Span_Data_Type   type;
   Eina_Bool        fast_matrix ;
   DATA32           mul_col;
   Ector_Rop        op;
   union {
      DATA32 color;
      Ector_Renderer_Software_Gradient_Data *gradient;
      //ImageData    texture;
   };
} Span_Data;

typedef struct _Software_Rasterizer
{
   SW_FT_Raster     raster;
   SW_FT_Stroker    stroker;

   Span_Data        fillData;
   Eina_Matrix3    *transform;
   Eina_Rectangle   systemClip;

} Software_Rasterizer;

struct _Ector_Software_Surface_Data
{
   Software_Rasterizer *software;
};


void ector_software_rasterizer_init(Software_Rasterizer *rasterizer);
void ector_software_rasterizer_done(Software_Rasterizer *rasterizer);

void ector_software_rasterizer_stroke_set(Software_Rasterizer *rasterizer, double width,
                                          Efl_Gfx_Cap cap_style, Efl_Gfx_Join join_style);

void ector_software_rasterizer_transform_set(Software_Rasterizer *rasterizer, Eina_Matrix3 *t);
void ector_software_rasterizer_color_set(Software_Rasterizer *rasterizer, int r, int g, int b, int a);
void ector_software_rasterizer_linear_gradient_set(Software_Rasterizer *rasterizer, Ector_Renderer_Software_Gradient_Data *linear);
void ector_software_rasterizer_radial_gradient_set(Software_Rasterizer *rasterizer, Ector_Renderer_Software_Gradient_Data *radial);
void ector_software_rasterizer_clip_rect_set(Software_Rasterizer *rasterizer, Eina_Array *clips);
void ector_software_rasterizer_clip_shape_set(Software_Rasterizer *rasterizer, Shape_Rle_Data *clip);



Shape_Rle_Data * ector_software_rasterizer_generate_rle_data(Software_Rasterizer *rasterizer, SW_FT_Outline *outline);
Shape_Rle_Data * ector_software_rasterizer_generate_stroke_rle_data(Software_Rasterizer *rasterizer, SW_FT_Outline *outline, Eina_Bool closePath);

void ector_software_rasterizer_draw_rle_data(Software_Rasterizer *rasterizer, int x, int y, uint mul_col, Ector_Rop op, Shape_Rle_Data* rle);

void ector_software_rasterizer_destroy_rle_data(Shape_Rle_Data *rle);



// Gradient Api
void update_color_table(Ector_Renderer_Software_Gradient_Data *gdata);
void destroy_color_table(Ector_Renderer_Software_Gradient_Data *gdata);
void fetch_linear_gradient(uint *buffer, Span_Data *data, int y, int x, int length);
void fetch_radial_gradient(uint *buffer, Span_Data *data, int y, int x, int length);

#endif
