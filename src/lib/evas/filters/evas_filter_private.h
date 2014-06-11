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

// Enable debug if you're working on optimizations
#define DEBUG_TIME 1

// Windows build will break if CLOCK_MONOTONIC is used
#if !defined(_POSIX_MONOTONIC_CLOCK) || (_POSIX_MONOTONIC_CLOCK < 0)
# undef DEBUG_TIME
# define DEBUG_TIME 0
#endif

// The 'restrict' keyword is part of C99
#if __STDC_VERSION__ < 199901L
# define restrict
#endif

// Helpers
#define ENFN ctx->evas->engine.func
#define ENDT ctx->evas->engine.data.output

#define BUFFERS_LOCK() do { if (cmd->input) cmd->input->locked = 1; if (cmd->output) cmd->output->locked = 1; if (cmd->mask) cmd->mask->locked = 1; } while (0)
#define BUFFERS_UNLOCK() do { if (cmd->input) cmd->input->locked = 0; if (cmd->output) cmd->output->locked = 0; if (cmd->mask) cmd->mask->locked = 0; } while (0)

#if DEBUG_TIME
# define DEBUG_TIME_BEGIN() \
   struct timespec ts1, ts2; \
   clock_gettime(CLOCK_MONOTONIC, &ts1);
# define DEBUG_TIME_END() \
   clock_gettime(CLOCK_MONOTONIC, &ts2); \
   long long int t = 1000000LL * (ts2.tv_sec - ts1.tv_sec) \
   + (ts2.tv_nsec - ts1.tv_nsec) / 1000LL; \
   INF("TIME SPENT: %lldus", t);
#else
# define DEBUG_TIME_BEGIN() do {} while(0)
# define DEBUG_TIME_END() do {} while(0)
#endif

#if DIV_USING_BITSHIFT
# define DEFINE_DIVIDER(div) const int pow2 = evas_filter_smallest_pow2_larger_than((div) << 10); const int numerator = (1 << pow2) / (div);
# define DIVIDE(val) (((val) * numerator) >> pow2)
#else
# define DEFINE_DIVIDER(div) const int divider = (div);
# define DIVIDE(val) ((val) / divider)
#endif

typedef enum _Evas_Filter_Interpolation_Mode Evas_Filter_Interpolation_Mode;

struct _Evas_Filter_Context
{
   Evas_Public_Data *evas;
   Eina_Inlist *commands;
   Eina_List *buffers; // Evas_Filter_Buffer *
   int last_buffer_id;
   int last_command_id;

   // Variables changing at each run
   int w, h; // Dimensions of the input/output buffers
   int padl, padt, padr, padb; // Padding in the current input/output buffers

   struct
   {
      /** Post-processing callback. The context can be safely destroyed here. */
      Evas_Filter_Cb cb;
      void *data;
      Eina_List *buffers_to_free; // Some buffers should be queued for deletion
   } post_run;

   struct
   {
      int bufid;
      void *context;
      int x, y;
      int cx, cy, cw, ch; // clip
      int r, g, b, a; // clip color
      Eina_Bool clip_use : 1;
      Eina_Bool color_use : 1;
   } target;

   Eina_Bool async : 1;
   Eina_Bool gl_engine : 1;
   Eina_Bool running : 1;
   Eina_Bool has_proxies : 1;
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
         int count;
         Evas_Filter_Blur_Type type;
         Eina_Bool auto_count : 1; // If true, BOX blur will be smooth using
      } blur;

      struct
      {
         DATA8 *data; // Pointer to 256 char array
         Evas_Filter_Channel channel;
      } curve;

      struct
      {
         // mask contains the map data
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

      struct
      {
         Evas_Filter_Transform_Flags flags;
      } transform;
   };

   struct {
      int render_op;
      int R, G, B, A;
      int ox, oy;
      union {
         struct {
            int x, y, w, h;
         };
         struct {
            int l, r, t, b;
         };
      } clip;
      Evas_Filter_Fill_Mode fillmode;
      Eina_Bool clip_use : 1;
      Eina_Bool clip_mode_lrtb : 1;
      Eina_Bool need_temp_buffer : 1;
   } draw;
};

struct _Evas_Filter_Buffer
{
   EINA_REFCOUNT;

   int id;
   Evas_Filter_Context *ctx;

   Evas_Object *source;
   Eina_Stringshare *source_name;
   RGBA_Image *backing;
   void *glimage;
   int w, h;

   Evas_Object *proxy;

   Eina_Bool alpha_only : 1;  // 1 channel (A) instead of 4 (RGBA)
   Eina_Bool allocated : 1;   // allocated on demand, belongs to this context
   Eina_Bool allocated_gl : 1; // allocated on demand the glimage
   Eina_Bool transient : 1;   // temporary buffer (automatic allocation)
   Eina_Bool locked : 1;      // internal flag
   Eina_Bool stolen : 1;      // stolen by the client
   Eina_Bool delete_me : 1;   // request delete asap (after released by client)
   Eina_Bool dirty : 1;       // Marked as dirty as soon as a command writes to it
};

enum _Evas_Filter_Interpolation_Mode
{
   EVAS_FILTER_INTERPOLATION_MODE_NONE,
   EVAS_FILTER_INTERPOLATION_MODE_LINEAR
};

void                     evas_filter_context_clear(Evas_Filter_Context *ctx);
void                     evas_filter_context_source_set(Evas_Filter_Context *ctx, Evas_Object *eo_proxy, Evas_Object *eo_source, int bufid, Eina_Stringshare *name);

/* FIXME: CPU filters entry points. Move these to the Evas Engine itself. */
Evas_Filter_Apply_Func   evas_filter_blend_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_blur_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_bump_map_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_curve_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_displace_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_fill_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_mask_cpu_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func   evas_filter_transform_cpu_func_get(Evas_Filter_Command *cmd);

/* Utility functions */
void _clip_to_target(int *sx, int *sy, int sw, int sh, int ox, int oy, int dw, int dh, int *dx, int *dy, int *rows, int *cols);
Eina_Bool evas_filter_buffer_alloc(Evas_Filter_Buffer *fb, int w, int h);
Evas_Filter_Buffer *_filter_buffer_get(Evas_Filter_Context *ctx, int bufid);
Eina_Bool           _filter_buffer_data_set(Evas_Filter_Context *ctx, int bufid, void *data, int w, int h, Eina_Bool alpha_only);
Evas_Filter_Buffer *_filter_buffer_data_new(Evas_Filter_Context *ctx, void *data, int w, int h, Eina_Bool alpha_only);
#define             evas_filter_buffer_alloc_new(ctx, w, h, a) _filter_buffer_data_new(ctx, NULL, w, h, a)
Evas_Filter_Buffer *evas_filter_temporary_buffer_get(Evas_Filter_Context *ctx, int w, int h, Eina_Bool alpha_only);
Evas_Filter_Buffer *evas_filter_buffer_scaled_get(Evas_Filter_Context *ctx, Evas_Filter_Buffer *src, unsigned w, unsigned h);
Eina_Bool evas_filter_interpolate(DATA8* output /* 256 values */, DATA8* points /* pairs x + y */, int point_count, Evas_Filter_Interpolation_Mode mode);
Evas_Filter_Command *_evas_filter_command_get(Evas_Filter_Context *ctx, int cmdid);
int evas_filter_smallest_pow2_larger_than(int val);

#endif // EVAS_FILTER_PRIVATE_H
