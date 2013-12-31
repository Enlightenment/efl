#ifndef EVAS_FILTER_PRIVATE_H
#define EVAS_FILTER_PRIVATE_H

#include "evas_filter.h"
#include "evas_private.h"

// This is a potential optimization.
#define DIV_USING_BITSHIFT 1

#ifdef LITTLE_ENDIAN
#define ALPHA 3
#define RGB0 0
#define RGB3 3
#define RED 2
#define GREEN 1
#define BLUE 0
#else
#define ALPHA 0
#define RGB0 1
#define RGB3 4
#define RED 0
#define GREEN 1
#define BLUE 2
#endif

// RGBA = (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))
#define ALPHA_OF(a) ((a) >> 24)
#define RED_OF(a)   (((a) >> 16) & 0xff)
#define GREEN_OF(a) (((a) >> 8) & 0xff)
#define BLUE_OF(a)  ((a) & 0xff)

// Helpers
#define ENFN ctx->evas->engine.func
#define ENDT ctx->evas->engine.data.output

struct _Evas_Filter_Context
{
   Evas_Public_Data *evas;
   Eina_Inlist *commands;
   Eina_List *buffers; // Evas_Filter_Buffer *
   int last_buffer_id;
   int last_command_id;
   int w, h; // Change at each run

   struct
   {
      /** Post-processing callback. The context can be safely destroyed here. */
      Evas_Filter_Cb cb;
      void *data;
   } post_run;
};

struct _Evas_Filter_Command
{
   EINA_INLIST;

   int id;
   Evas_Filter_Mode mode;
   Evas_Filter_Context *ctx;

   Evas_Filter_Buffer *input;
   Evas_Filter_Buffer *mask;
   Evas_Filter_Buffer *output;

   union
   {
      struct
      {
         int dx, dy;
         Evas_Filter_Blur_Type type;
      } blur;

      struct
      {
         DATA8 *data; // Pointer to 256 char array
         Evas_Filter_Channel channel;
      } curve;

      struct
      {
         // mask is an Alpha or RG(BA) texture, see flags. Must be of the same size as the input & output buffers (for now, FIXME)
         Evas_Filter_Displacement_Flags flags;
         int intensity; // Max displacement in pixels
      } displacement;

      struct
      {
         float xyangle; // in degrees: 0-360 (modulo)
         float zangle;  // degrees: 0-90 (defaults to 0)
         float specular_factor; // range TBD: 0-...
         float elevation;
         DATA32 dark;
         DATA32 color;
         DATA32 white;
         Eina_Bool compensate : 1; // Compensate for darkening
         //Eina_Bool specular : 1; // Use specular light as well (needs specular_factor > 0)
      } bump;
   };

   struct {
      int render_op;
      int R, G, B, A;
      int ox, oy;
      int clipx, clipy, clipw, cliph;
   } draw;
};

struct _Evas_Filter_Buffer
{
   EINA_REFCOUNT;

   int id;
   Evas_Filter_Context *ctx;

   Evas_Object *source;
   void *backing;
   int w, h;

   Eina_Bool alpha_only : 1;  // 1 channel (A) instead of 4 (RGBA)
   Eina_Bool allocated : 1;   // allocated on demand, belongs to this context
   Eina_Bool transient : 1;   // temporary buffer (automatic allocation)
   Eina_Bool locked : 1;      // internal flag
};

void                     evas_filter_context_clear(Evas_Filter_Context *ctx);
void                     evas_filter_context_proxy_bind(Evas_Filter_Context *ctx, Evas_Object *eo_proxy, Evas_Object *eo_source, int bufid);

/* FIXME: CPU filters entry points. Move these to the Evas Engine itself. */
Evas_Filter_Apply_Func   evas_filter_blend_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_blur_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_bump_map_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_curve_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_displace_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_fill_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_mask_cpu_func_get(Evas_Filter_Command *cmd);

/* Utility functions */
void _clip_to_target(int *sx, int *sy, int sw, int sh, int ox, int oy, int dw, int dh, int *dx, int *dy, int *rows, int *cols);
Eina_Bool evas_filter_buffer_alloc(Evas_Filter_Buffer *fb, int w, int h);
Evas_Filter_Buffer *_filter_buffer_get(Evas_Filter_Context *ctx, int bufid);

#endif // EVAS_FILTER_PRIVATE_H
