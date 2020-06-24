#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <float.h>

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

#define MY_CLASS ECTOR_RENDERER_SOFTWARE_IMAGE_CLASS

typedef struct _Ector_Renderer_Software_Image_Data Ector_Renderer_Software_Image_Data;

struct _Ector_Renderer_Software_Image_Data
{
   Ector_Software_Surface_Data *surface;
   Ector_Renderer_Image_Data   *image;
   Ector_Renderer_Data         *base;
   Ector_Buffer                *comp;
   Efl_Gfx_Vg_Composite_Method comp_method;
   int                          opacity;
   Eina_Matrix3                 inv_m;
   struct {
      int x1, y1, x2, y2;
   } boundary;
};

static Eina_Bool
_ector_renderer_software_image_ector_renderer_prepare(Eo *obj,
                                                      Ector_Renderer_Software_Image_Data *pd)
{
   if (!pd->surface)
     pd->surface = efl_data_xref(pd->base->surface, ECTOR_SOFTWARE_SURFACE_CLASS, obj);

   if (!pd->image->buffer || !pd->surface->rasterizer->fill_data.raster_buffer)
     return EINA_FALSE;

   Eina_Matrix3 m;
   double m11, m12, m21, m22, m31, m32;
   int x = pd->surface->x + (int)pd->base->origin.x;
   int y = pd->surface->y + (int)pd->base->origin.y;
   int image_w, image_h;
   ector_buffer_size_get(pd->image->buffer, &image_w, &image_h);

   double px[4] = {x, x + image_w, x, x + image_w};
   double py[4] = {y, y, y + image_h, y + image_h};

   //Only use alpha color
   pd->opacity = pd->base->color.a;
   /*ector_software_rasterizer_color_set(pd->surface->rasterizer,
     pd->base->color.r,
     pd->base->color.g,
     pd->base->color.b,
     pd->base->color.a);*/

   if (!pd->base->m)
     {
        eina_matrix3_identity(&m);
        eina_matrix3_scale(&m, (double)pd->surface->rasterizer->fill_data.raster_buffer->generic->w / (double)image_w,
                           (double)pd->surface->rasterizer->fill_data.raster_buffer->generic->h / (double)image_h);
     }
   else
     eina_matrix3_copy(&m, pd->base->m);
   eina_matrix3_values_get(&m, &m11, &m12, NULL,
                           &m21, &m22, NULL,
                           &m31, &m32, NULL);
   //Calc draw boundbox
   pd->boundary.x1 = MAX(pd->surface->rasterizer->fill_data.raster_buffer->generic->w , (unsigned int)image_w);
   pd->boundary.y1 = MAX(pd->surface->rasterizer->fill_data.raster_buffer->generic->h , (unsigned int)image_h);
   pd->boundary.x2 = 0; pd->boundary.y2 = 0;
   for (int i = 0; i < 4; i++)
     {
        pd->boundary.x1 = MIN(pd->boundary.x1, (int)(((px[i] * m11) + (py[i] * m21) + m31) + 0.5));
        pd->boundary.y1 = MIN(pd->boundary.y1, (int)(((px[i] * m12) + (py[i] * m22) + m32) + 0.5));

        pd->boundary.x2 = MAX(pd->boundary.x2, (int)(((px[i] * m11) + (py[i] * m21) + m31) + 0.5));
        pd->boundary.y2 = MAX(pd->boundary.y2, (int)(((px[i] * m12) + (py[i] * m22) + m32) + 0.5));
     }

   eina_matrix3_inverse(&m, &pd->inv_m);

   return EINA_TRUE;
}

//FIXME: We need to implement that apply op, clips and mul_col.
static Eina_Bool
_ector_renderer_software_image_ector_renderer_draw(Eo *obj EINA_UNUSED,
                                                   Ector_Renderer_Software_Image_Data *pd,
                                                   Efl_Gfx_Render_Op op EINA_UNUSED, Eina_Array *clips EINA_UNUSED,
                                                   unsigned int mul_col EINA_UNUSED)
{
   if (!pd->image->buffer || !pd->surface->rasterizer->fill_data.raster_buffer->pixels.u32)
          return EINA_FALSE;

   if (pd->opacity == 0)
      return EINA_TRUE;

   const int pix_stride = pd->surface->rasterizer->fill_data.raster_buffer->stride / 4;
   Ector_Software_Buffer_Base_Data *comp = pd->comp ? efl_data_scope_get(pd->comp, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN) : NULL;
   Ector_Software_Buffer_Base_Data *bpd = efl_data_scope_get(pd->image->buffer, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   double im11, im12, im21, im22, im31, im32;
   uint32_t *dst_buf, *src_buf;
   int image_w, image_h;
   ector_buffer_size_get(pd->image->buffer, &image_w, &image_h);

   dst_buf = pd->surface->rasterizer->fill_data.raster_buffer->pixels.u32;
   src_buf = bpd->pixels.u32;

   eina_matrix3_values_get(&pd->inv_m, &im11, &im12, NULL,
                                       &im21, &im22, NULL,
                                       &im31, &im32, NULL);

   //Draw
   for (int local_y = pd->boundary.y1; local_y < pd->boundary.y2; local_y++)
     {
        for (int local_x = pd->boundary.x1; local_x <  pd->boundary.x2; local_x++)
          {
             uint32_t *dst = dst_buf + ((int)local_x + ((int)local_y * pix_stride));
             int rx, ry;
             rx = (int)(((double)local_x * im11) + ((double)local_y * im21) + im31 + 0.5);
             ry = (int)(((double)local_x * im12) + ((double)local_y * im22) + im32 + 0.5);
             if (rx < 0 || rx >= image_w || ry < 0 || ry >= image_h)
               continue;
             uint32_t *src = src_buf + (rx + (ry * image_w));  //FIXME: use to stride
             uint32_t temp = 0x0;
             if (comp)
               {
                  uint32_t *m = comp->pixels.u32 + ((int)local_x + ((int)local_y * comp->generic->w));
                  //FIXME : This comping can work only matte case.
                  //        We need consider to inverse matte case.
                  temp = draw_mul_256((((*m)>>24) * pd->opacity)>>8, *src);
               }
             else
               {
                  temp = draw_mul_256(pd->opacity, *src);
               }
             int inv_alpha = 255 - ((temp) >> 24);
             *dst = temp + draw_mul_256(inv_alpha, *dst);
          }
     }

   return EINA_TRUE;
}

static Eo *
_ector_renderer_software_image_efl_object_constructor(Eo *obj, Ector_Renderer_Software_Image_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   if (!obj) return NULL;

   pd->image = efl_data_xref(obj, ECTOR_RENDERER_IMAGE_MIXIN, obj);
   pd->base = efl_data_xref(obj, ECTOR_RENDERER_CLASS, obj);

   return obj;
}

static void
_ector_renderer_software_image_efl_object_destructor(Eo *obj, Ector_Renderer_Software_Image_Data *pd)
{
   efl_data_xunref(pd->base->surface, pd->surface, obj);
   efl_data_xunref(obj, pd->base, obj);
   efl_data_xunref(obj, pd->image, obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

unsigned int
_ector_renderer_software_image_ector_renderer_crc_get(const Eo *obj,
                                                      Ector_Renderer_Software_Image_Data *pd)
{
   unsigned int crc;

   crc = ector_renderer_crc_get(efl_super(obj, MY_CLASS));

   crc = eina_crc((void*) pd->image, sizeof (Ector_Renderer_Image_Data), crc, EINA_FALSE);
   return crc;
}

static void
_ector_renderer_software_image_ector_renderer_comp_method_set(Eo *obj EINA_UNUSED,
                                                              Ector_Renderer_Software_Image_Data *pd,
                                                              Ector_Buffer *comp,
                                                              Efl_Gfx_Vg_Composite_Method method)
{
   pd->comp = comp;
   pd->comp_method = method;
}

#include "ector_renderer_software_image.eo.c"
