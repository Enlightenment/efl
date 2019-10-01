#ifndef _EVAS_FILTER_H
#define _EVAS_FILTER_H

#include "evas_common_private.h"
#include "evas_private.h"

#include "efl_canvas_filter_internal.eo.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef DEBUG
# define FILTERS_DEBUG
#endif

typedef struct _Evas_Filter_Instruction Evas_Filter_Instruction;
typedef struct _Evas_Filter_Buffer Evas_Filter_Buffer;
typedef struct _Evas_Filter_Proxy_Binding Evas_Filter_Proxy_Binding;
typedef struct _Evas_Filter_Padding Evas_Filter_Padding;
typedef enum _Evas_Filter_Mode Evas_Filter_Mode;
typedef enum _Evas_Filter_Blur_Type Evas_Filter_Blur_Type;
typedef enum _Evas_Filter_Channel Evas_Filter_Channel;
typedef enum _Evas_Filter_Displacement_Flags Evas_Filter_Displacement_Flags;
typedef enum _Evas_Filter_Bump_Flags Evas_Filter_Bump_Flags;
typedef enum _Evas_Filter_Fill_Mode Evas_Filter_Fill_Mode;
typedef enum _Evas_Filter_Transform_Flags Evas_Filter_Transform_Flags;

typedef void (* Evas_Filter_Cb) (Evas_Filter_Context *ctx, void *data, Eina_Bool success);

#define EVAS_FILTER_BUFFER_RGBA  EINA_FALSE
#define EVAS_FILTER_BUFFER_ALPHA EINA_TRUE

#define EVAS_FILTER_BUFFER_INPUT_ID  1
#define EVAS_FILTER_BUFFER_OUTPUT_ID 2

/** @internal */
enum _Evas_Filter_Mode
{
   EVAS_FILTER_MODE_SKIP,         /**< No operation */
   EVAS_FILTER_MODE_BLEND,        /**< Blend with current context render_op */
   EVAS_FILTER_MODE_BLUR,         /**< @see Evas_Filter_Blur_Type */
   EVAS_FILTER_MODE_CURVE,        /**< Apply color curve */
   EVAS_FILTER_MODE_DISPLACE,     /**< Apply XY displacement based on RG mask */
   EVAS_FILTER_MODE_FILL,         /**< Fill a buffer with a solid color */
   EVAS_FILTER_MODE_MASK,         /**< Apply Alpha or RGBA texture on image */
   EVAS_FILTER_MODE_BUMP,         /**< Apply bump mapping (light effect) */
   EVAS_FILTER_MODE_TRANSFORM,    /**< Apply a simple geometrical transformation */
   EVAS_FILTER_MODE_PADDING_SET,  /**< Special padding_set instruction to force a specific padding value */
   EVAS_FILTER_MODE_GRAYSCALE,    /**< Leave only grayscale information */
   EVAS_FILTER_MODE_LAST
};

/** @internal */
enum _Evas_Filter_Blur_Type
{
   EVAS_FILTER_BLUR_DEFAULT  = 0x0, // Default blur (GAUSSIAN or series of BOX)
   EVAS_FILTER_BLUR_BOX      = 0x1, // Optimizable on CPU. But, UGLY. O(n)
   EVAS_FILTER_BLUR_GAUSSIAN = 0x2, // Gaussian blur (using sine curve)
   EVAS_FILTER_BLUR_LAST,
};

/** @internal */
enum _Evas_Filter_Channel
{
   EVAS_FILTER_CHANNEL_ALPHA = 0,
   EVAS_FILTER_CHANNEL_RED   = 1,
   EVAS_FILTER_CHANNEL_GREEN = 2,
   EVAS_FILTER_CHANNEL_BLUE  = 3,
   EVAS_FILTER_CHANNEL_RGB   = 4
};

/** @internal */
enum _Evas_Filter_Displacement_Flags
{
   EVAS_FILTER_DISPLACE_NEAREST  = 0x0,   /**< Interpolate between pixels (linear interpolation) */
   EVAS_FILTER_DISPLACE_LINEAR   = 0x1,   /**< Interpolate between pixels (linear interpolation) */
   EVAS_FILTER_DISPLACE_BLACK    = 0x0,   /**< Use black (or transparent) when going out of bounds) */
   EVAS_FILTER_DISPLACE_STRETCH  = 0x2,   /**< Stretch border pixels when going out of bounds */
   EVAS_FILTER_DISPLACE_BITMASK  = 0x3
};

/** @internal */
enum _Evas_Filter_Bump_Flags
{
   EVAS_FILTER_BUMP_NORMAL       = 0x0,
   EVAS_FILTER_BUMP_COMPENSATE   = 0x1    /**< Compensate for darkening (diffuse light) or brightening (specular light) of zero gradient surfaces */
};

/** @internal */
enum _Evas_Filter_Fill_Mode
{
   EVAS_FILTER_FILL_MODE_NONE               = 0x0,
   EVAS_FILTER_FILL_MODE_STRETCH_X          = 0x1,
   EVAS_FILTER_FILL_MODE_STRETCH_Y          = 0x2,
   EVAS_FILTER_FILL_MODE_REPEAT_X           = 0x4,
   EVAS_FILTER_FILL_MODE_REPEAT_Y           = 0x8,
   EVAS_FILTER_FILL_MODE_REPEAT_X_STRETCH_Y = EVAS_FILTER_FILL_MODE_REPEAT_X | EVAS_FILTER_FILL_MODE_STRETCH_Y,
   EVAS_FILTER_FILL_MODE_REPEAT_Y_STRETCH_X = EVAS_FILTER_FILL_MODE_REPEAT_Y | EVAS_FILTER_FILL_MODE_STRETCH_X,
   EVAS_FILTER_FILL_MODE_REPEAT_XY          = EVAS_FILTER_FILL_MODE_REPEAT_X | EVAS_FILTER_FILL_MODE_REPEAT_Y,
   EVAS_FILTER_FILL_MODE_STRETCH_XY         = EVAS_FILTER_FILL_MODE_STRETCH_X | EVAS_FILTER_FILL_MODE_STRETCH_Y
};

/** @internal */
enum _Evas_Filter_Transform_Flags
{
   EVAS_FILTER_TRANSFORM_VFLIP = 1
};

/** @internal */
struct _Evas_Filter_Padding
{
   int l, r, t, b;
};

#define EFL_CANVAS_FILTER_STATE_DEFAULT { {}, { 255, 255, 255, 255 }, { "default", 0.0 }, {}, 0, 0, 1.0, 0.0 }

/* Parser stuff (high level API) */
EAPI Evas_Filter_Program *evas_filter_program_new(const char *name, Eina_Bool input_alpha);
EAPI Eina_Bool           evas_filter_program_state_set(Evas_Filter_Program *pgm, const Efl_Canvas_Filter_State *state);
EAPI Eina_Bool           evas_filter_program_parse(Evas_Filter_Program *pgm, const char *str);
EAPI void                evas_filter_program_del(Evas_Filter_Program *pgm);
EAPI Eina_Bool           evas_filter_program_padding_get(Evas_Filter_Program *pgm, Evas_Filter_Padding *final, Evas_Filter_Padding *calc);
EAPI void                evas_filter_program_source_set_all(Evas_Filter_Program *pgm, Eina_Hash *sources);
void                     evas_filter_program_data_set_all(Evas_Filter_Program *pgm, Eina_Inlist *data);

/* Filter context (low level) */
Evas_Filter_Context     *evas_filter_context_new(Evas_Public_Data *evas, Eina_Bool async, void *user_data);
void                    *evas_filter_context_data_get(Evas_Filter_Context *ctx);
Eina_Bool                evas_filter_context_async_get(Evas_Filter_Context *ctx);
void                     evas_filter_context_size_get(Evas_Filter_Context *ctx, int *w, int *H);
int                      evas_filter_context_ref(Evas_Filter_Context *ctx);
void                     evas_filter_context_unref(Evas_Filter_Context *ctx);
Eina_Bool                evas_filter_context_program_use(void *engine, void *output, Evas_Filter_Context *ctx, Evas_Filter_Program *pgm, Eina_Bool reuse, int object_x, int object_y);
void                     evas_filter_context_proxy_render_all(Evas_Filter_Context *ctx, Eo *eo_obj, void *output, Eina_Bool do_async);
void                     evas_filter_context_post_run_callback_set(Evas_Filter_Context *ctx, Evas_Filter_Cb cb, void *data);
Eina_Bool                evas_filter_context_buffers_allocate_all(Evas_Filter_Context *ctx);
void                     evas_filter_context_obscured_region_set(Evas_Filter_Context *ctx, Eina_Rectangle rect);

int                      evas_filter_buffer_empty_new(Evas_Filter_Context *ctx, int w, int h, Eina_Bool alpha_only);
int                      evas_filter_buffer_proxy_new(Evas_Filter_Context *ctx, Evas_Filter_Proxy_Binding *pb, int *w, int *h);
void                    *evas_filter_buffer_backing_get(Evas_Filter_Context *ctx, int bufid, Eina_Bool render);
Eina_Bool                evas_filter_buffer_backing_set(Evas_Filter_Context *ctx, int bufid, void *engine_buffer);

Eina_Bool                evas_filter_context_run(void *engine, void *output, Evas_Filter_Context *ctx);

Eina_Bool                evas_filter_font_draw(Evas_Filter_Context *ctx, void *engine, void *output, void *draw_context, int bufid, Evas_Font_Set *font, int x, int y, Evas_Text_Props *text_props, Eina_Bool do_async);
Eina_Bool                evas_filter_target_set(Evas_Filter_Context *ctx, void *draw_context, void *surface, int x, int y, const RGBA_Map *map);

// utility function
void                     _evas_filter_source_hash_free_cb(void *data);

/**
 * @brief Blend a source buffer into a destination buffer, allowing X,Y offsets, Alpha to RGBA conversion with color
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context. Current color is used when inbuf is ALPHA and outbuf is RGBA.
 * @param inbuf          Source buffer: ALPHA or RGBA
 * @param outbuf         Destination buffer: ALPHA or RGBA (note: must be RGBA if inbuf is RGBA)
 * @param ox             X offset in the destination buffer
 * @param oy             Y offset in the destination buffer
 * @param fillmode       Specifies whether to repeat or stretch the input onto its destination, and on which axes
 * @param alphaonly      If true, discard RGB during RGBA -> Alpha conversions.
 * @return               Filter command ID or -1 in case of error
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_blend_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, int ox, int oy, Evas_Filter_Fill_Mode fillmode, Eina_Bool alphaonly);

/**
 * @brief Apply a blur effect on a buffer
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context. Current color is used when inbuf is ALPHA and outbuf is RGBA.
 * @param inbuf          Source buffer: ALPHA or RGBA
 * @param outbuf         Destination buffer: ALPHA or RGBA (note: must be RGBA if inbuf is RGBA)
 * @param type           Type of blur: BOX, GAUSSIAN or MOTION
 * @param dx             X radius of blur. Can be negative ONLY for MOTION blur
 * @param dy             Y radius of blur. Can be negative ONLY for MOTION blur
 * @param ox             X offset in the destination buffer
 * @param oy             Y offset in the destination buffer
 * @param count          Number of times to repeat the operation (used for smooth fast blurs with box blur)
 * @param alphaonly      If true, discard RGB during RGBA -> Alpha conversions.
 * @return               Filter command ID or -1 in case of error
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_blur_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, Evas_Filter_Blur_Type type, int dx, int dy, int ox, int oy, int count, Eina_Bool alphaonly);

/**
 * @brief Fill a buffer with the current color
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context. Current color is used when buf is RGBA, and clip is used to specify the fill area.
 * @param buf            Buffer: ALPHA or RGBA
 * @return               Filter command ID or -1 in case of error
 * @note The current draw context's render operation is ignored (always uses COPY mode).
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_fill_add(Evas_Filter_Context *ctx, void *draw_context, int buf);

/**
 * @brief evas_filter_command_curve_add
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context. Current color is used when buf is RGBA, and clip is used to specify the fill area.
 * @param inbuf          Input buffer, ALPHA or RGBA.
 * @param outbuf         Output buffer, must have same colorspace as inbuf.
 * @param curve          The data points to use, must contain 256 values.
 * @param channel        Which channel to apply the curve (red, green, blue, alpha or RGB)
 * @return               Filter command ID or -1 in case of error
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_curve_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, DATA8 *curve /* 256 elements */, Evas_Filter_Channel channel);

/**
 * @brief Grow/Shrink an image, as defined in image processing (this is not a scale algorithm!)
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context. Current color is used when inbuf is ALPHA and outbuf is RGBA.
 * @param inbuf          Source buffer: ALPHA or RGBA
 * @param outbuf         Destination buffer: ALPHA or RGBA (note: must be RGBA if inbuf is RGBA)
 * @param radius         Number of pixels to grow by. If negative, shrink instead of grow
 * @param smooth         Use smooth blur and curve for grow (default: true)
 * @param alphaonly      If true, discard RGB during RGBA -> Alpha conversions.
 * @return               Filter command ID or -1 in case of error
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_grow_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, int radius, Eina_Bool smooth, Eina_Bool alphaonly);

/**
 * @brief Apply a displacement map to a buffer. This will move pixels from the source to the destination based on pixel per pixel offset, as defined in the displacement map
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context (ignored)
 * @param inbuf          Input buffer (Alpha or RGBA)
 * @param outbuf         Output buffer (Alpha or RGBA), same size as inbuf
 * @param dispbuf        Displacement map. Should be an RGBA buffer, where the Red and Green channels are the displacement maps for X and Y. Can be also ALPHA buffer, in which case only one dimension can be specified (X or Y).
 * @param flags          Alters how the map is interpreted, @see Evas_Filter_Displacement_Flags
 * @param intensity      Maximum offset possible, if the map's value is maximal at this point (ie. 0 or 255)
 * @param fillmode       Specifies how to repeat and stretch the map to fit the target size
 * @return               Filter command ID or -1 in case of error
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_displacement_map_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, int dispbuf, Evas_Filter_Displacement_Flags flags, int intensity, Evas_Filter_Fill_Mode fillmode);

/**
 * @brief Apply a texture to a buffer
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context (ignored)
 * @param inbuf          Input buffer (Alpha or RGBA)
 * @param maskbuf        Texture buffer (Alpha or RGBA)
 * @param outbuf         Output buffer (Alpha or RGBA)
 * @param fillmode       Specifies how to repeat and stretch the mask to fit the target size
 * @return               Filter command ID or -1 in case of error
 * @note For the moment, inbuf can only be ALPHA, and output must be RGBA if mask is RGBA as well
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_mask_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int maskbuf, int outbuf, Evas_Filter_Fill_Mode fillmode);

/**
 * @brief Apply a relief effect based on a bump map (Z map)
 * @param ctx              Current filter chain
 * @param draw_context     Current Evas draw context (ignored)
 * @param inbuf            Input buffer (Alpha or RGBA)
 * @param bumpbuf          Bump map (Alpha only), same size as inbuf. By definition, lows are black (alpha 0) and highs are white (alpha 255).
 * @param outbuf           Output buffer (Alpha or RGBA), same size as inbuf
 * @param azimuth          CCW angle in degrees from the X axis of the light direction. 0 is light from the right, 90 from the top, 180 from the left, 270 from the bottom. All values are valid.
 * @param elevation        Angle in degrees between the XY plane and the light. Only values from 0 (light is perfectly horizontal) to 90 (light comes from the viewer herself) are acceptable.
 * @param depth            Max depth in the bump map. Default value is 10.
 * @param specular_factor  Factor for the specular light effect (shininess). Ranges from 1.0 to 1000+ with logarithmic effects
 * @param black            Darkest color, defines the ambiant light
 * @param color            Light's normal color
 * @param white            Brightest color, used in the shininess effect
 * @param flags            Optional flags: compensation for darkening
 * @param fillmode         Specifies how to repeat and stretch the map to fit the target size
 * @return                 Filter command ID or -1 in case of error
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_bump_map_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int bumpbuf, int outbuf, float azimuth, float elevation, float depth, float specular_factor, DATA32 black, DATA32 color, DATA32 white, Evas_Filter_Bump_Flags flags, Evas_Filter_Fill_Mode fillmode);

/**
 * @brief Apply a geometrical transformation to the buffer
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context (ignored)
 * @param inbuf          Input buffer (Alpha or RGBA)
 * @param outbuf         Output buffer (Alpha or RGBA), same size as inbuf
 * @param flags          Specifies the operation to apply (eg. vflip)
 * @param ox             X offset
 * @param oy             Y offset
 * @return               Filter command ID or -1 in case of error
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_transform_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, Evas_Filter_Transform_Flags flags, int ox, int oy);

/**
 * @brief Remove color information from the buffer
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context (ignored)
 * @param inbuf          Input buffer (Alpha or RGBA)
 * @param outbuf         Output buffer (Alpha or RGBA), same size as inbuf
 * @return               Filter command or NULL in case of error
 * @internal
 */
Evas_Filter_Command     *evas_filter_command_grayscale_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf);

/* Simple binding between a filter object and its sources */
struct _Evas_Filter_Proxy_Binding
{
   Evas_Object *eo_proxy;
   Evas_Object *eo_source;
   Eina_Stringshare *name;
};

struct _Evas_Filter_Data_Binding
{
   EINA_INLIST;
   Eina_Stringshare *name;
   Eina_Stringshare *value;
   Eina_Bool execute : 1;
};

#undef EAPI
#define EAPI

#endif
