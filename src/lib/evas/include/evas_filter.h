#ifndef _EVAS_FILTER_H
#define _EVAS_FILTER_H

#include "evas_common_private.h"

typedef struct _Evas_Filter_Context Evas_Filter_Context;
typedef struct _Evas_Filter_Command Evas_Filter_Command;
typedef struct _Evas_Filter_Program Evas_Filter_Program;
typedef struct _Evas_Filter_Instruction Evas_Filter_Instruction;
typedef struct _Evas_Filter_Buffer Evas_Filter_Buffer;
typedef enum _Evas_Filter_Mode Evas_Filter_Mode;
typedef enum _Evas_Filter_Blur_Type Evas_Filter_Blur_Type;
typedef enum _Evas_Filter_Channel Evas_Filter_Channel;
typedef enum _Evas_Filter_Displacement_Flags Evas_Filter_Displacement_Flags;
typedef enum _Evas_Filter_Bump_Flags Evas_Filter_Bump_Flags;
typedef enum _Evas_Filter_Fill_Mode Evas_Filter_Fill_Mode;

typedef Eina_Bool (* Evas_Filter_Apply_Func) (Evas_Filter_Command *cmd);
typedef void (* Evas_Filter_Cb) (Evas_Filter_Context *ctx, void *data);

#define EVAS_FILTER_BUFFER_RGBA  EINA_FALSE
#define EVAS_FILTER_BUFFER_ALPHA EINA_TRUE

enum _Evas_Filter_Mode
{
   EVAS_FILTER_MODE_BLEND,        /**< Blend with current context render_op */
   EVAS_FILTER_MODE_BLUR,         /**< @see Evas_Filter_Blur_Type */
   EVAS_FILTER_MODE_CURVE,        /**< Apply color curve */
   EVAS_FILTER_MODE_DISPLACE,     /**< Apply XY displacement based on RG mask */
   EVAS_FILTER_MODE_FILL,         /**< Fill a buffer with a solid color */
   EVAS_FILTER_MODE_MASK,         /**< Apply Alpha or RGBA texture on image */
   EVAS_FILTER_MODE_BUMP,         /**< Apply bump mapping (light effect) */
   EVAS_FILTER_MODE_LAST
};

enum _Evas_Filter_Blur_Type
{
   EVAS_FILTER_BLUR_GAUSSIAN = 0x0, // Gaussian or sine curve. O(nm)
   EVAS_FILTER_BLUR_BOX      = 0x1, // Optimizable. But, UGLY. O(n)
   EVAS_FILTER_BLUR_MOTION   = 0x2, // Not implemented
   EVAS_FILTER_BLUR_LAST,
   EVAS_FILTER_BLUR_DEFAULT  = EVAS_FILTER_BLUR_GAUSSIAN
};

enum _Evas_Filter_Channel
{
   EVAS_FILTER_CHANNEL_ALPHA = 0,
   EVAS_FILTER_CHANNEL_RED   = 1,
   EVAS_FILTER_CHANNEL_GREEN = 2,
   EVAS_FILTER_CHANNEL_BLUE  = 3,
   EVAS_FILTER_CHANNEL_RGB   = 4
};

enum _Evas_Filter_Displacement_Flags
{
   EVAS_FILTER_DISPLACE_ALPHA    = 0x0,   /**< Displace based on Alpha values */
   EVAS_FILTER_DISPLACE_X        = 0x1,   /**< X displacement */
   EVAS_FILTER_DISPLACE_X_ALPHA  = 0x1,   /**< X displacement */
   EVAS_FILTER_DISPLACE_Y        = 0x2,   /**< Y displacement */
   EVAS_FILTER_DISPLACE_Y_ALPHA  = 0x2,   /**< Y displacement */
   EVAS_FILTER_DISPLACE_RG       = 0x4,   /**< Displace based on R(G) values */
   EVAS_FILTER_DISPLACE_XY_ALPHA = 0x3,   /**< XY displacement based on alpha */
   EVAS_FILTER_DISPLACE_XY_RG    = 0x7,   /**< Full 2-D displacement based on RG values */
   EVAS_FILTER_DISPLACE_NEAREST  = 0x0,   /**< Interpolate between pixels (linear interpolation) */
   EVAS_FILTER_DISPLACE_LINEAR   = 0x8,   /**< Interpolate between pixels (linear interpolation) */
   EVAS_FILTER_DISPLACE_BLACK    = 0x0,   /**< Use black (or transparent) when going out of bounds) */
   EVAS_FILTER_DISPLACE_STRETCH  = 0x8,   /**< Stretch border pixels when going out of bounds */
   //EVAS_FILTER_DISPLACE_WRAP     = 0x10,  /**< Wrap around input when displacement goes out of bounds */
   EVAS_FILTER_DISPLACE_BITMASK  = 0xf
};

enum _Evas_Filter_Bump_Flags
{
   EVAS_FILTER_BUMP_NORMAL       = 0x0,
   EVAS_FILTER_BUMP_COMPENSATE   = 0x1    /**< Compensate for darkening (diffuse light) or brightening (specular light) of zero gradient surfaces */
};

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

/* Parser stuff (high level API) */
Evas_Filter_Program     *evas_filter_program_new(const char *name);
Eina_Bool                evas_filter_program_parse(Evas_Filter_Program *pgm, const char *str);
void                     evas_filter_program_del(Evas_Filter_Program *pgm);
Eina_Bool                evas_filter_context_program_use(Evas_Filter_Context *ctx, Evas_Object *eo_obj, Evas_Filter_Program *pgm);
Eina_Bool                evas_filter_program_padding_get(Evas_Filter_Program *pgm, int *l, int *r, int *t, int *b);
void                     evas_filter_program_proxy_source_bind(Evas_Filter_Program *pgm, const char *name, Evas_Object *object);
Evas_Object             *evas_filter_program_proxy_source_get(Evas_Filter_Program *pgm, const char *name);
void                     evas_filter_context_proxy_render_all(Evas_Filter_Context *ctx, Eo *eo_obj, Eina_Bool do_async);

/* Filter context (low level) */
Evas_Filter_Context     *evas_filter_context_new(Evas_Public_Data *evas);
void                     evas_filter_context_destroy(Evas_Filter_Context *ctx);
void                     evas_filter_context_post_run_callback_set(Evas_Filter_Context *ctx, Evas_Filter_Cb cb, void *data);
#define                  evas_filter_context_autodestroy(ctx) evas_filter_context_post_run_callback_set(ctx, ((Evas_Filter_Cb) evas_filter_context_destroy), ctx)

int                      evas_filter_buffer_empty_new(Evas_Filter_Context *ctx, Eina_Bool alpha_only);
int                      evas_filter_buffer_image_new(Evas_Filter_Context *ctx, RGBA_Image *image);
int                      evas_filter_buffer_data_new(Evas_Filter_Context *ctx, void *data, int w, int h, Eina_Bool alpha_only);
#define                  evas_filter_buffer_alloc_new(ctx, w, h, a) evas_filter_buffer_data_new(ctx, NULL, w, h, a)
void                    *evas_filter_buffer_backing_get(Evas_Filter_Context *ctx, int bufid);
Eina_Bool                evas_filter_buffer_data_set(Evas_Filter_Context *ctx, int bufid, void *data, int w, int h, Eina_Bool alpha_only);

Eina_Bool                evas_filter_run(Evas_Filter_Context *ctx, Eina_Bool do_async);

/**
 * @brief Blend a source buffer into a destination buffer, allowing X,Y offsets, Alpha to RGBA conversion with color
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context. Current color is used when inbuf is ALPHA and outbuf is RGBA.
 * @param inbuf          Source buffer: ALPHA or RGBA
 * @param outbuf         Destination buffer: ALPHA or RGBA (note: must be RGBA if inbuf is RGBA)
 * @param ox             X offset in the destination buffer
 * @param oy             Y offset in the destination buffer
 * @return               Filter command ID or -1 in case of error
 */
int                      evas_filter_command_blend_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, int ox, int oy);

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
 * @return               Filter command ID or -1 in case of error
 */
int                      evas_filter_command_blur_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, Evas_Filter_Blur_Type type, int dx, int dy, int ox, int oy);

/**
 * @brief Fill a buffer with the current color
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context. Current color is used when buf is RGBA, and clip is used to specify the fill area.
 * @param buf            Buffer: ALPHA or RGBA
 * @return               Filter command ID or -1 in case of error
 * @note The current draw context's render operation is ignored (always uses COPY mode).
 */
int                      evas_filter_command_fill_add(Evas_Filter_Context *ctx, void *draw_context, int buf);
int                      evas_filter_command_curve_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, DATA8 *curve /* 256 elements */, Evas_Filter_Channel channel);

/**
 * @brief Grow/Shrink an image, as defined in image processing (this is not a scale algorithm!)
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context. Current color is used when inbuf is ALPHA and outbuf is RGBA.
 * @param inbuf          Source buffer: ALPHA or RGBA
 * @param outbuf         Destination buffer: ALPHA or RGBA (note: must be RGBA if inbuf is RGBA)
 * @param radius         Number of pixels to grow by. If negative, shrink instead of grow
 * @param smooth         Use smooth blur and curve for grow (default: true)
 * @return               Filter command ID or -1 in case of error
 */
int                      evas_filter_command_grow_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, int radius, Eina_Bool smooth);

/**
 * @brief Apply a displacement map to a buffer. This will move pixels from the source to the destination based on pixel per pixel offset, as defined in the displacement map
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context (ignored)
 * @param inbuf          Input buffer (Alpha or RGBA)
 * @param outbuf         Output buffer (Alpha or RGBA), same size as inbuf
 * @param dispbuf        Displacement map. Should be an RGBA buffer, where the Red and Green channels are the displacement maps for X and Y. Can be also ALPHA buffer, in which case only one dimension can be specified (X or Y).
 * @param flags          Alters how the map is interpreted, @see Evas_Filter_Displacement_Flags
 * @param intensity      Maximum offset possible, if the map's value is maximal at this point (ie. 0 or 255)
 * @return               Filter command ID or -1 in case of error
 */
int                      evas_filter_command_displacement_map_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int outbuf, int dispbuf, Evas_Filter_Displacement_Flags flags, int intensity);

/**
 * @brief Apply a texture to a buffer
 * @param ctx            Current filter chain
 * @param draw_context   Current Evas draw context (ignored)
 * @param inbuf          Input buffer (Alpha or RGBA)
 * @param maskbuf        Texture buffer (Alpha or RGBA)
 * @param outbuf         Output buffer (Alpha or RGBA)
 * @return               Filter command ID or -1 in case of error
 * @note For the moment, inbuf can only be ALPHA, and output must be RGBA if mask is RGBA as well
 */
int                      evas_filter_command_mask_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int maskbuf, int outbuf);

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
 * @return                 Filter command ID or -1 in case of error
 */
int                      evas_filter_command_bump_map_add(Evas_Filter_Context *ctx, void *draw_context, int inbuf, int bumpbuf, int outbuf, float azimuth, float elevation, float depth, float specular_factor, DATA32 black, DATA32 color, DATA32 white, Evas_Filter_Bump_Flags flags);

#endif

