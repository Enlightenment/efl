#ifndef EVAS_FILTER_PRIVATE_H
#define EVAS_FILTER_PRIVATE_H

#include "evas_filter.h"
#include "evas_private.h"

/* logging variables */
extern int _evas_filter_log_dom;
#define EVAS_FILTER_LOG_COLOR EINA_COLOR_LIGHTBLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_filter_log_dom, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_filter_log_dom, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_filter_log_dom, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_filter_log_dom, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_filter_log_dom, __VA_ARGS__)

#ifdef FILTERS_DEBUG
# define XDBG(...) DBG(__VA_ARGS__)
#else
# define XDBG(...) do {} while (0)
#endif

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
#undef ENFN
#undef ENC
#define ENFN ctx->evas->engine.func
#define ENC _evas_engine_context(ctx->evas)

#define CMD_ENC _evas_engine_context(cmd->ctx->evas)
#define FB_ENC _evas_engine_context(fb->ctx->evas)

#define BUFFERS_LOCK() do { if (cmd->input) cmd->input->locked = 1; if (cmd->output) cmd->output->locked = 1; if (cmd->mask) cmd->mask->locked = 1; } while (0)
#define BUFFERS_UNLOCK() do { if (cmd->input) cmd->input->locked = 0; if (cmd->output) cmd->output->locked = 0; if (cmd->mask) cmd->mask->locked = 0; } while (0)

#if DEBUG_TIME
# define DEBUG_TIME_BEGIN() \
   struct timespec ts1, ts2; \
   clock_gettime(CLOCK_MONOTONIC, &ts1);
# define DEBUG_TIME_END() \
   clock_gettime(CLOCK_MONOTONIC, &ts2); \
   long long int t = 1000000LL * (ts2.tv_sec - ts1.tv_sec) \
   + (ts2.tv_nsec - ts1.tv_nsec) / 1000LL; (void) t; \
   XDBG("TIME SPENT: %lldus", t);
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

typedef Evas_Filter_Buffer * (*evas_filter_buffer_scaled_get_func)(Evas_Filter_Context *ctx, Evas_Filter_Buffer *src, unsigned w, unsigned h);


struct _Evas_Filter_Context
{
   Evas_Public_Data *evas;
   Eina_Inlist *commands;
   Eina_List *buffers; // Evas_Filter_Buffer *
   int last_buffer_id;
   int last_command_id;
   void *user_data; // used by textblock

   // ugly hack (dlsym fail)
   evas_filter_buffer_scaled_get_func buffer_scaled_get;

   // Variables changing at each run
   int x, y; // Position of the object (for GL downscaling of snapshots)
   int w, h; // Dimensions of the input/output buffers

   struct {
      // Padding in the current input/output buffers
      Evas_Filter_Padding calculated, final;
   } pad;

   struct {
      // Useless region: obscured by other objects
      Eina_Rectangle real, effective;
   } obscured;

   struct
   {
      /** Post-processing callback. The context can be safely destroyed here. */
      Evas_Filter_Cb cb;
      void *data;
   } post_run;

   struct
   {
      void *surface;
      int x, y;
      int cx, cy, cw, ch; // clip
      int r, g, b, a; // clip color
      void *mask; // mask
      int mask_x, mask_y; // mask offset
      Evas_Render_Op rop;
      RGBA_Map *map;
      Eina_Bool clip_use : 1;
      Eina_Bool color_use : 1;
   } target;

   short        run_count;
   short        refcount;

   Eina_Bool running : 1;
   Eina_Bool async : 1;
   Eina_Bool has_proxies : 1;
   Eina_Bool gl : 1;

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
         float dx, dy;
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
      Efl_Gfx_Render_Op rop;
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
      struct {
         int factor_x, factor_y;
         int pad_x, pad_y;
         Eina_Bool down;
      } scale;
      Evas_Filter_Fill_Mode fillmode;
      Eina_Bool alphaonly : 1;
      Eina_Bool clip_use : 1;
      Eina_Bool clip_mode_lrtb : 1;
      Eina_Bool need_temp_buffer : 1;
      Eina_Bool output_was_dirty : 1;
   } draw;
};

struct _Evas_Filter_Buffer
{
   EINA_REFCOUNT;

   int id;
   Evas_Filter_Context *ctx;

   Evas_Object *source;
   Eina_Stringshare *source_name;
   Ector_Buffer *buffer;
   int w, h;

   Eina_Bool used : 1;        // This buffer is in use (useful for reuse of context)
   Eina_Bool alpha_only : 1;  // 1 channel (A) instead of 4 (RGBA)
   Eina_Bool transient : 1;   // temporary buffer (automatic allocation)
   Eina_Bool locked : 1;      // internal flag
   Eina_Bool dirty : 1;       // Marked as dirty as soon as a command writes to it
   Eina_Bool is_render : 1;   // Is render target of a filter using engine functions (ie. needs FBO in GL)
   Eina_Bool cleanup : 1;     // Needs cleaning up if not allocated
};

enum _Evas_Filter_Interpolation_Mode
{
   EVAS_FILTER_INTERPOLATION_MODE_NONE,
   EVAS_FILTER_INTERPOLATION_MODE_LINEAR
};

enum _Evas_Filter_Support
{
   EVAS_FILTER_SUPPORT_NONE = 0,
   EVAS_FILTER_SUPPORT_CPU,
   EVAS_FILTER_SUPPORT_GL
};

void                     evas_filter_mixin_init(void);
void                     evas_filter_mixin_shutdown(void);

void                     evas_filter_context_clear(Evas_Filter_Context *ctx, Eina_Bool keep_buffers);
void                     evas_filter_context_source_set(Evas_Filter_Context *ctx, Evas_Object *eo_proxy, Evas_Object *eo_source, int bufid, Eina_Stringshare *name);

/* Utility functions */
void _clip_to_target(int *sx, int *sy, int sw, int sh, int ox, int oy, int dw, int dh, int *dx, int *dy, int *rows, int *cols);
Eina_Bool evas_filter_buffer_alloc(Evas_Filter_Buffer *fb, int w, int h);
Evas_Filter_Buffer *_filter_buffer_get(Evas_Filter_Context *ctx, int bufid);
Evas_Filter_Buffer *evas_filter_temporary_buffer_get(Evas_Filter_Context *ctx, int w, int h, Eina_Bool alpha_only, Eina_Bool clean);
Evas_Filter_Buffer *evas_filter_buffer_scaled_get(Evas_Filter_Context *ctx, Evas_Filter_Buffer *src, unsigned w, unsigned h);
Eina_Bool           evas_filter_interpolate(DATA8* output /* 256 values */, int *points /* 256 values */, Evas_Filter_Interpolation_Mode mode);
int evas_filter_smallest_pow2_larger_than(int val);

void _evas_filter_context_program_reuse(void *engine, void *output, Evas_Filter_Context *ctx);
void evas_filter_parser_shutdown(void);

#define E_READ  ECTOR_BUFFER_ACCESS_FLAG_READ
#define E_WRITE ECTOR_BUFFER_ACCESS_FLAG_WRITE
#define E_ALPHA EFL_GFX_COLORSPACE_GRY8
#define E_ARGB  EFL_GFX_COLORSPACE_ARGB8888

static inline void *
_buffer_map_all(Ector_Buffer *buf, unsigned int *len, Ector_Buffer_Access_Flag mode, Efl_Gfx_Colorspace cspace, unsigned int *stride)
{
   void *ret = NULL;
   int w, h;
   if (!buf) return NULL;
   ector_buffer_size_get(buf, &w, &h);
   ret = ector_buffer_map(buf, len, mode, 0, 0, w, h, cspace, stride);
   return ret;
}

#endif // EVAS_FILTER_PRIVATE_H
