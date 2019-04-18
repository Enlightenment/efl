#ifndef EMILE_IMAGE_H
#define EMILE_IMAGE_H

/**
 * @defgroup Emile_Image_Group Top level functions
 * @ingroup Emile
 * Function that allow reading/saving image.
 *
 * @{
 */

/**
 * @typedef Emile_Colorspace
 *
 * Flags that describe all colorspace known by EFL. Some routine may not know all of them.
 * All the value from below enum should be the same as in Evas_Loader.h
 *
 * @see Evas_Colorspace
 * @see Eet_Colorspace
 *
 * @since 1.14
 */
typedef enum _Emile_Colorspace
{
  EMILE_COLORSPACE_ARGB8888,/**< ARGB 32 bits per pixel, high-byte is Alpha, accessed 1 32bit word at a time */
  EMILE_COLORSPACE_YCBCR422P601_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-601 specifications. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
  EMILE_COLORSPACE_YCBCR422P709_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-709 specifications. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
  EMILE_COLORSPACE_RGB565_A5P, /**< 16bit rgb565 + Alpha plane at end - 5 bits of the 8 being used per alpha byte */
  EMILE_COLORSPACE_GRY8 = 4,
  EMILE_COLORSPACE_YCBCR422601_PL, /**<  YCbCr 4:2:2, ITU.BT-601 specifications. The data pointed to is just an array of row pointer, pointing to line of Y,Cb,Y,Cr bytes */
  EMILE_COLORSPACE_YCBCR420NV12601_PL, /**< YCbCr 4:2:0, ITU.BT-601 specification. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb,Cr rows. */
  EMILE_COLORSPACE_YCBCR420TM12601_PL, /**< YCbCr 4:2:0, ITU.BT-601 specification. The data pointed to is just an array of tiled row pointer, pointing to the Y rows, then the Cb,Cr rows. */
  EMILE_COLORSPACE_AGRY88 = 8, /**< AY 8bits Alpha and 8bits Grey, accessed 1 16bits at a time */
  EMILE_COLORSPACE_ETC1 = 9, /**< OpenGL ETC1 encoding of RGB texture (4 bit per pixel) @since 1.10 */
  EMILE_COLORSPACE_RGB8_ETC2 = 10, /**< OpenGL GL_COMPRESSED_RGB8_ETC2 texture compression format (4 bit per pixel) @since 1.10 */
  EMILE_COLORSPACE_RGBA8_ETC2_EAC = 11,	/**< OpenGL GL_COMPRESSED_RGBA8_ETC2_EAC texture compression format, supports alpha (8 bit per pixel) @since 1.10 */
  EMILE_COLORSPACE_ETC1_ALPHA = 12, /**< ETC1 with alpha support using two planes: ETC1 RGB and ETC1 grey for alpha @since 1.11 */
  EMILE_COLORSPACE_RGB_S3TC_DXT1 = 13,	/**< OpenGL COMPRESSED_RGB_S3TC_DXT1_EXT format with RGB only. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT1 = 14,	/**< OpenGL COMPRESSED_RGBA_S3TC_DXT1_EXT format with RGBA punchthrough. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT2 = 15,	/**< DirectDraw DXT2 format with premultiplied RGBA. Not supported by OpenGL itself. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT3 = 16,	/**< OpenGL COMPRESSED_RGBA_S3TC_DXT3_EXT format with RGBA. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT4 = 17,	/**< DirectDraw DXT4 format with premultiplied RGBA. Not supported by OpenGL itself. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT5 = 18	/**< OpenGL COMPRESSED_RGBA_S3TC_DXT5_EXT format with RGBA. @since 1.11 */
} Emile_Colorspace;

/**
 * @typedef Emile_Image_Encoding
 *
 * Flags that describe the supported encoding. Some routine may not know all of them.
 * The value are the same as the one provided before in Eet.h
 *
 * @see Eet_Image_Encoding
 *
 * @since 1.14
 */
typedef enum _Emile_Image_Encoding
{
  EMILE_IMAGE_LOSSLESS = 0,
  EMILE_IMAGE_JPEG = 1,
  EMILE_IMAGE_ETC1 = 2,
  EMILE_IMAGE_ETC2_RGB = 3,
  EMILE_IMAGE_ETC2_RGBA = 4,
  EMILE_IMAGE_ETC1_ALPHA = 5
} Emile_Image_Encoding;

/**
 * @typedef Emile_Image_Scale_Hint
 *
 * Flags that describe the scale hint used by the loader infrastructure.
 *
 * @see Evas_Image_Scale_Hint
 *
 * @since 1.14
 */
typedef enum _Emile_Image_Scale_Hint
{
  EMILE_IMAGE_SCALE_HINT_NONE = 0, /**< No scale hint at all */
  EMILE_IMAGE_SCALE_HINT_DYNAMIC = 1, /**< Image is being re-scaled over time, thus turning scaling cache @b off for its data */
  EMILE_IMAGE_SCALE_HINT_STATIC = 2 /**< Image is not being re-scaled over time, thus turning scaling cache @b on for its data */
} Emile_Image_Scale_Hint;

/**
 * @typedef Emile_Image_Animated_Loop_Hint
 *
 * Flags describing the behavior of animation from a loaded image.
 *
 * @see Evas_Image_Animated_Loop_Hint
 *
 * @since 1.14
 */
typedef enum _Emile_Image_Animated_Loop_Hint
{
  EMILE_IMAGE_ANIMATED_HINT_NONE = 0,
  EMILE_IMAGE_ANIMATED_HINT_LOOP = 1,
  EMILE_IMAGE_ANIMATED_HINT_PINGPONG = 2
} Emile_Image_Animated_Loop_Hint;

/**
 * @typedef Emile_Image_Load_Error
 *
 * Flags describing error state as discovered by an image loader.
 *
 * @see Evas_Load_Error
 *
 * @since 1.14
 */
typedef enum _Emile_Image_Load_Error
{
  EMILE_IMAGE_LOAD_ERROR_NONE = 0,  /**< No error on load */
  EMILE_IMAGE_LOAD_ERROR_GENERIC = 1,  /**< A non-specific error occurred */
  EMILE_IMAGE_LOAD_ERROR_DOES_NOT_EXIST = 2,  /**< File (or file path) does not exist */
  EMILE_IMAGE_LOAD_ERROR_PERMISSION_DENIED = 3,	 /**< Permission denied to an existing file (or path) */
  EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4,  /**< Allocation of resources failure prevented load */
  EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE = 5,  /**< File corrupt (but was detected as a known format) */
  EMILE_IMAGE_LOAD_ERROR_UNKNOWN_FORMAT = 6,  /**< File is not a known format */
  EMILE_IMAGE_LOAD_ERROR_CANCELLED = 7 /**< File loading has been cancelled */
} Emile_Image_Load_Error; /**< Emile image load error codes one can get - see emile_load_error_str() too. */

/**
 * @typedef Emile_Image
 *
 * Internal type representing an opened image.
 *
 * @since 1.14
 */
typedef struct _Emile_Image Emile_Image;

/**
 * @typedef Emile_Image_Load_Opts
 *
 * Description of the possible load option.
 *
 * @since 1.14
 */
typedef struct _Emile_Image_Load_Opts Emile_Image_Load_Opts;

/**
 * @typedef Emile_Image_Animated
 *
 * Description animation.
 *
 * @since 1.14
 */
typedef struct _Emile_Image_Animated Emile_Image_Animated;

/**
 * @typedef Emile_Image_Property
 *
 * Description of a loaded image property.
 *
 * @since 1.14
 */
typedef struct _Emile_Image_Property Emile_Image_Property;

/**
 * @enum _Emile_Action
 * @typedef Emile_Action
 * What action emile is referring to.
 * @since 1.19
 */
typedef enum _Emile_Action
{
  EMILE_ACTION_NONE = 0,
  EMILE_ACTION_CANCELLED = 1
} Emile_Action;

/**
 * @typedef Emile_Action_Cb
 * A callback triggered by emile to learn what to do about a specific action.
 * @since 1.19
 */
typedef Eina_Bool (*Emile_Action_Cb)(void *data, Emile_Image *image, Emile_Action action);

struct _Emile_Image_Property
{
  struct
  {
    unsigned char l, r, t, b;
  } borders;

  const Emile_Colorspace *cspaces;
  Emile_Colorspace cspace;

  Emile_Image_Encoding encoding;

  unsigned int w;
  unsigned int h;
  unsigned int row_stride;

  unsigned char scale;

  Eina_Bool rotated;
  Eina_Bool alpha;
  Eina_Bool premul;
  Eina_Bool alpha_sparse;

  Eina_Bool flipped;
  Eina_Bool comp;
};

struct _Emile_Image_Animated
{
  Eina_List *frames;

  Emile_Image_Animated_Loop_Hint loop_hint;

  int frame_count;
  int loop_count;
  int cur_frame;

  Eina_Bool animated;
};

struct _Emile_Image_Load_Opts
{
  Eina_Rectangle region;
  struct
  {
    int src_x, src_y, src_w, src_h;
    int dst_w, dst_h;
    int smooth;

    /* This should have never been part of this structure, but we keep it
       for ABI/API compatibility with Evas_Loader */
    Emile_Image_Scale_Hint scale_hint;
  } scale_load;
  double dpi;
  unsigned int w, h;
  unsigned int degree;
  int scale_down_by;

  Eina_Bool orientation;
};

// FIXME: should we set region at load time, instead of head time
// FIXME: should we regive the animated structure for head and data ?

/**
 * Open a TGV image from memory.
 *
 * @param source The Eina_Binbuf with TGV image in it.
 * @param opts Load option for the image to open (it can be @c NULL).
 * @param animated Description of the image animation property, set during head reading and updated for each frame read by data (can be @c NULL)
 * @param error Contain a valid error code if the function return @c NULL.
 * @return a handler of the image if successfully opened, otherwise @c NULL.
 *
 * @since 1.14
 */
EAPI Emile_Image *emile_image_tgv_memory_open(Eina_Binbuf * source, Emile_Image_Load_Opts * opts, Emile_Image_Animated * animated, Emile_Image_Load_Error * error);

/**
 * Open a TGV image from a file.
 *
 * @param source The Eina_File with TGV image in it.
 * @param opts Load option for the image to open (it can be @c NULL).
 * @param animated Description of the image animation property, set during head reading and updated for each frame read by data (can be @c NULL)
 * @param error Contain a valid error code if the function return @c NULL.
 * @return a handler of the image if successfully opened, otherwise @c NULL.
 *
 * @since 1.14
 */
EAPI Emile_Image *emile_image_tgv_file_open(Eina_File * source, Emile_Image_Load_Opts * opts, Emile_Image_Animated * animated, Emile_Image_Load_Error * error);


/**
 * Open a JPEG image from memory.
 *
 * @param source The Eina_Binbuf with JPEG image in it.
 * @param opts Load option for the image to open (it can be @c NULL).
 * @param animated Description of the image animation property, set during head reading and updated for each frame read by data (can be @c NULL)
 * @param error Contain a valid error code if the function return @c NULL.
 * @return a handler of the image if successfully opened, otherwise @c NULL.
 *
 * @since 1.14
 */
EAPI Emile_Image *emile_image_jpeg_memory_open(Eina_Binbuf * source, Emile_Image_Load_Opts * opts, Emile_Image_Animated * animated, Emile_Image_Load_Error * error);

/**
 * Open a JPEG image from file.
 *
 * @param source The Eina_File with JPEG image in it.
 * @param opts Load option for the image to open (it can be @c NULL).
 * @param animated Description of the image animation property, set during head reading and updated for each frame read by data (can be @c NULL)
 * @param error Contain a valid error code if the function return @c NULL.
 * @return a handler of the image if successfully opened, otherwise @c NULL.
 *
 * @since 1.14
 */
EAPI Emile_Image *emile_image_jpeg_file_open(Eina_File * source, Emile_Image_Load_Opts * opts, Emile_Image_Animated * animated, Emile_Image_Load_Error * error);

/**
 * Read the header of an image to fill Emile_Image_Property.
 *
 * @param image The Emile_Image handler.
 * @param prop The Emile_Image_Property to be filled.
 * @param property_size The size of the Emile_Image_Property as known during compilation.
 * @param error Contain a valid error code if the function return @c NULL.
 * @return @c EINA_TRUE if the header was successfully readed and prop properly filled.
 *
 * @since 1.14
 */
EAPI Eina_Bool emile_image_head(Emile_Image * image, Emile_Image_Property * prop, unsigned int property_size, Emile_Image_Load_Error * error);

/**
 * Read the pixels from an image file.
 *
 * @param image The Emile_Image handler.
 * @param prop The property to respect while reading this pixels.
 * @param property_size The size of the Emile_Image_Property as known during compilation.
 * @param pixels The actual pointer to the already allocated pixels buffer to fill.
 * @param error Contain a valid error code if the function return @c NULL.
 * @return @c EINA_TRUE if the data was successfully read and the pixels correctly filled.
 *
 * @since 1.14
 */
EAPI Eina_Bool emile_image_data(Emile_Image * image, Emile_Image_Property * prop, unsigned int property_size, void *pixels, Emile_Image_Load_Error * error);

/**
 * Register a callback for emile to ask what to do during the processing of an image
 *
 * @param image The Emile_Image handler to register on.
 * @param callback The callback to use
 * @param action The action this callback is triggered on.
 * @since 1.19
 */
EAPI void emile_image_callback_set(Emile_Image *image, Emile_Action_Cb callback, Emile_Action action, const void *data);

/**
 * Close an opened image handler.
 *
 * @param source The handler to close.
 *
 * @since 1.14
 */
EAPI void emile_image_close(Emile_Image * source);

/**
 * Convert an error code related to an image handler into a meaningful string.
 *
 * @param source The handler related to the error (can be @c NULL).
 * @param error The error code to get a message from.
 * @return a string that will be owned by Emile, either by the handler if it is not @c NULL or by the library directly if it is.
 *
 * @since 1.14
 */
EAPI const char *emile_load_error_str(Emile_Image * source, Emile_Image_Load_Error error);

/**
 * @}
 */

#endif
