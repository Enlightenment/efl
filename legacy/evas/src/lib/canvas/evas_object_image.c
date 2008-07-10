/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"
#include "../engines/common/evas_convert_color.h"
#include "../engines/common/evas_convert_colorspace.h"
#include "../engines/common/evas_convert_yuv.h"

/* private magic number for image objects */
static const char o_type[] = "image";

/* private struct for rectangle object internal data */
typedef struct _Evas_Object_Image      Evas_Object_Image;

struct _Evas_Object_Image
{
   DATA32            magic;

   struct {
      Evas_Coord_Rectangle fill;
      struct {
	 short       w, h, stride;
      } image;
      struct {
	 short         l, r, t, b;
	 unsigned char fill;
      } border;

      const char    *file;
      const char    *key;
      int            cspace;

      unsigned char  smooth_scale : 1;
      unsigned char  has_alpha :1;
   } cur, prev;

   int               pixels_checked_out;
   int               load_error;
   Evas_List        *pixel_updates;

   struct {
      unsigned char  scale_down_by;
      double         dpi;
      short          w, h;
   } load_opts;

   struct {
      void            (*get_pixels) (void *data, Evas_Object *o);
      void             *get_pixels_data;
   } func;

   void             *engine_data;

   unsigned char     changed : 1;
   unsigned char     dirty_pixels : 1;
};

/* private methods for image objects */
static void evas_object_image_unload(Evas_Object *obj);
static void evas_object_image_load(Evas_Object *obj);
static Evas_Coord evas_object_image_figure_x_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);
static Evas_Coord evas_object_image_figure_y_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);

static void evas_object_image_init(Evas_Object *obj);
static void *evas_object_image_new(void);
static void evas_object_image_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_image_free(Evas_Object *obj);
static void evas_object_image_render_pre(Evas_Object *obj);
static void evas_object_image_render_post(Evas_Object *obj);

static int evas_object_image_is_opaque(Evas_Object *obj);
static int evas_object_image_was_opaque(Evas_Object *obj);
static int evas_object_image_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

static void *evas_object_image_data_convert_internal(Evas_Object_Image *o, void *data, Evas_Colorspace to_cspace);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_image_free,
   evas_object_image_render,
   evas_object_image_render_pre,
   evas_object_image_render_post,
   /* these are optional. NULL = nothing */
   NULL,
   NULL,
   NULL,
   NULL,
   evas_object_image_is_opaque,
   evas_object_image_was_opaque,
   evas_object_image_is_inside,
   NULL,
   NULL
};

/**
 * @defgroup Evas_Object_Image Image Object Functions
 *
 * Functions used to create and manipulate image objects.
 *
 * Note - Image objects may return or accept "image data" in multiple formats.
 * This is based on the colorspace of an object. Here is a rundown on formats:
 *
 * EVAS_COLORSPACE_ARGB8888:
 *
 * This pixel format is a linear block of pixels, starting at the top-left row
 * by row until the bottom right of the image or pixel region. All pixels are
 * 32-bit unsigned int's with the high-byte being alpha and the low byte being
 * blue in the format ARGB. Alpha may or may not be used by evas depending on
 * the alpha flag of the image, but if not used, should be set to 0xff anyway.
 *
 * This colorspace uses premultiplied alpha. That means that R, G and B cannot
 * exceed A in value. The conversion from non-premultiplied colorspace is:
 *
 * R = (r * a) / 255; G = (g * a) / 255; B = (b * a) / 255;
 *
 * So 50% transparent blue will be: 0x80000080. This will not be "dark" - just
 * 50% transparent. Values are 0 == black, 255 == solid or full red, green or
 * blue.
 *
 * EVAS_COLORSPACE_YCBCR422P601_PL:
 *
 * This is a pointer-list indirected set of YUV (YCbCr) pixel data. This means
 * that the data returned or set is not actual pixel data, but pointers TO
 * lines of pixel data. The list of pointers will first be N rows of pointers
 * to the Y plane - pointing to the first pixel at the start of each row in
 * the Y plane. N is the height of the image data in pixels. Each pixel in the
 * Y, U and V planes is 1 byte exactly, packed. The next N / 2 pointers will
 * point to rows in the U plane, and the next N / 2 pointers will point to
 * the V plane rows. U and V planes are half the horizontal and vertical
 * resolution of the U plane.
 *
 * Row order is top to bottom and row pixels are stored left to right.
 *
 * There is a limitation that these images MUST be a multiple of 2 pixels in
 * size horizontally or vertically. This is due to the U and V planes being
 * half resolution. Also note that this assumes the itu601 YUV colorspace
 * specification. This is defined for standard television and mpeg streams.
 * HDTV may use the itu709 specification.
 *
 * Values are 0 to 255, indicating full or no signal in that plane
 * respectively.
 *
 * EVAS_COLORSPACE_YCBCR422P709_PL:
 *
 * Not implemented yet.
 *
 * EVAS_COLORSPACE_RGB565_A5P:
 *
 * In the process of being implemented in 1 engine only. This may change.
 *
 * This is a pointer to image data for 16-bit half-word pixel data in 16bpp
 * RGB 565 format (5 bits red, 6 bits green, 5 bits blue), with the high-byte
 * containing red and the low byte containing blue, per pixel. This data is
 * packed row by row from the top-left to the bottom right.
 *
 * If the image has an alpha channel enabled there will be an extra alpha plane
 * after the color pixel plane. If not, then this data will not exist and
 * should not be accessed in any way. This plane is a set of pixels with 1
 * byte per pixel defining the alpha values of all pixels in the image from
 * the top-left to the bottom right of the image, row by row. Even though
 * the values of the alpha pixels can be 0 to 255, only values 0 through to 32
 * are used, 32 being solid and 0 being transparent.
 *
 * RGB values can be 0 to 31 for red and blue and 0 to 63 for green, with 0
 * being black and 31 or 63 being full red, green or blue respectively. This
 * colorspace is also pre-multiplied like EVAS_COLORSPACE_ARGB8888 so:
 *
 * R = (r * a) / 32; G = (g * a) / 32; B = (b * a) / 32;
 */

/**
 * Creates a new image object on the given evas.
 *
 * @param e The given evas.
 * @return The created image object.
 * @ingroup Evas_Object_Image
 */
EAPI Evas_Object *
evas_object_image_add(Evas *e)
{
   Evas_Object *obj;
   Evas_Object_Image *o;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_image_init(obj);
   evas_object_inject(obj, e);
   o = (Evas_Object_Image *)(obj->object_data);
   o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output,
								       o->engine_data);
   return obj;
}

/**
 * @defgroup Evas_Object_Image_File_Group Image Object File Functions
 *
 * Functions that write to or retrieve images from files.
 */

/**
 * Sets the filename and key of the given image object.
 *
 * If the file supports multiple data stored in it as eet,
 * you can specify the key to be used as the index of the
 * image in this file.
 *
 * @param obj The given image object.
 * @param file The image filename.
 * @param key The image key in file, or NULL.
 * @ingroup Evas_Object_Image_File_Group
 */
EAPI void
evas_object_image_file_set(Evas_Object *obj, const char *file, const char *key)
{
   Evas_Object_Image *o;
   Evas_Image_Load_Opts lo;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.file) && (file) && (!strcmp(o->cur.file, file)))
     {
	if ((!o->cur.key) && (!key))
	  return;
	if ((o->cur.key) && (key) && (!strcmp(o->cur.key, key)))
	  return;
     }
   if (o->cur.file) evas_stringshare_del(o->cur.file);
   if (o->cur.key) evas_stringshare_del(o->cur.key);
   if (file) o->cur.file = evas_stringshare_add(file);
   else o->cur.file = NULL;
   if (key) o->cur.key = evas_stringshare_add(key);
   else o->cur.key = NULL;
   o->prev.file = NULL;
   o->prev.key = NULL;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->load_error = EVAS_LOAD_ERROR_NONE;
   lo.scale_down_by = o->load_opts.scale_down_by;
   lo.dpi = o->load_opts.dpi;
   lo.w = o->load_opts.w;
   lo.h = o->load_opts.h;
   o->engine_data = obj->layer->evas->engine.func->image_load(obj->layer->evas->engine.data.output,
							      o->cur.file,
							      o->cur.key,
							      &o->load_error,
							      &lo);
   if (o->engine_data)
     {
	int w, h;
	int stride;

	obj->layer->evas->engine.func->image_size_get(obj->layer->evas->engine.data.output,
						      o->engine_data, &w, &h);
	if (obj->layer->evas->engine.func->image_stride_get)
	  obj->layer->evas->engine.func->image_stride_get(obj->layer->evas->engine.data.output,
							  o->engine_data, &stride);
	else
	  stride = w;
	o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get(obj->layer->evas->engine.data.output,
									  o->engine_data);
	o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output,
									    o->engine_data);
	o->cur.image.w = w;
	o->cur.image.h = h;
	o->cur.image.stride = stride;
     }
   else
     {
	o->load_error = EVAS_LOAD_ERROR_GENERIC;
	o->cur.has_alpha = 1;
	o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
	o->cur.image.w = 0;
	o->cur.image.h = 0;
	o->cur.image.stride = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the filename and key of the given image object.
 *
 * @param obj The given image object.
 * @param file Location to store the image filename, or NULL.
 * @param key Location to store the image key, or NULL.
 * @ingroup Evas_Object_Image_File_Group
 */
EAPI void
evas_object_image_file_get(const Evas_Object *obj, const char **file, const char **key)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   if (file) *file = o->cur.file;
   if (key) *key = o->cur.key;
}

/**
 * @defgroup Evas_Object_Image_Border_Group Image Object Border Functions
 *
 * Functions that adjust the unscaled image border of image objects.
 */

/**
 * Sets how much of each border of the given image object is not
 * to be scaled.
 *
 * When rendering, the image may be scaled to fit the size of the
 * image object. This function sets what area around the border of
 * the image is not to be scaled. This sort of function is useful for
 * widget theming, where, for example, buttons may be of varying
 * sizes, but the border size must remain constant.
 *
 * The units used for @p l, @p r, @p t and @p b are output units.
 *
 * @param obj The given image object.
 * @param l Distance of the left border that is not to be stretched.
 * @param r Distance of the right border that is not to be stretched.
 * @param t Distance of the top border that is not to be stretched.
 * @param b Distance of the bottom border that is not to be stretched.
 * @ingroup Evas_Object_Image_Border_Group
 */
EAPI void
evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (l < 0) l = 0;
   if (r < 0) r = 0;
   if (t < 0) t = 0;
   if (b < 0) b = 0;
   if ((o->cur.border.l == l) &&
       (o->cur.border.r == r) &&
       (o->cur.border.t == t) &&
       (o->cur.border.b == b)) return;
   o->cur.border.l = l;
   o->cur.border.r = r;
   o->cur.border.t = t;
   o->cur.border.b = b;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves how much of each border of the given image object is not to
 * be scaled.
 *
 * See @ref evas_object_image_border_set for more details.
 *
 * @param obj The given image object.
 * @param l Location to store the left border width in, or NULL.
 * @param r Location to store the right border width in, or NULL.
 * @param t Location to store the top border width in, or NULL.
 * @param b Location to store the bottom border width in, or NULL.
 * @ingroup Evas_Object_Image_Border_Group
 */
EAPI void
evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   if (l) *l = o->cur.border.l;
   if (r) *r = o->cur.border.r;
   if (t) *t = o->cur.border.t;
   if (b) *b = o->cur.border.b;
}

/**
 * @defgroup Evas_Object_Image_Fill_Group Image Object Fill Rectangle Functions
 *
 * Functions that deal with what areas of the image object are to be
 * tiled with the given image.
 */

/**
 * Sets if the center part of the given image object (not the border)
 * should be drawn.
 *
 * When rendering, the image may be scaled to fit the size of the
 * image object. This function sets if the center part of the scaled image
 * is to be drawn or left completely blank. Very useful for frames and
 * decorations.
 *
 * @param obj The given image object.
 * @param fill Whether the center should be drawn.
 * @ingroup Evas_Object_Image_Fill_Group
 */
EAPI void
evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Bool fill)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (((o->cur.border.fill) && (fill)) ||
       ((!o->cur.border.fill) && (!fill)))
     return;
   o->cur.border.fill = fill;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves if the center of the given image object is to be drawn
 * or not.
 *
 * See @ref evas_object_image_fill_set for more details.
 *
 * @param obj The given image object.
 * @return If the center is to be drawn or not.
 * @ingroup Evas_Object_Image_Fill_Group
 */
EAPI Evas_Bool
evas_object_image_border_center_fill_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.border.fill;
}

/**
 * Sets the rectangle of the given image object that the image will
 * be drawn to.
 *
 * Note that the image will be tiled around this one rectangle. To have
 * only one copy of the image drawn, @p x and @p y must be 0 and @p w
 * and @p h need to be the width and height of the image object
 * respectively.
 *
 * The default values for the fill parameters is @p x = 0, @p y = 0,
 * @p w = 32 and @p h = 32.
 *
 * @param obj The given image object.
 * @param x The X coordinate for the top left corner of the image.
 * @param y The Y coordinate for the top left corner of the image.
 * @param w The width of the image.
 * @param h The height of the image.
 * @ingroup Evas_Object_Image_Fill_Group
 */
EAPI void
evas_object_image_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Object_Image *o;

   if (w < 0) w = -w;
   if (h < 0) h = -h;
   if (w == 0.0) return;
   if (h == 0.0) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.fill.x == x) &&
       (o->cur.fill.y == y) &&
       (o->cur.fill.w == w) &&
       (o->cur.fill.h == h)) return;
   o->cur.fill.x = x;
   o->cur.fill.y = y;
   o->cur.fill.w = w;
   o->cur.fill.h = h;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the dimensions of the rectangle of the given image object
 * that the image will be drawn to.
 *
 * See @ref evas_object_image_fill_set for more details.
 *
 * @param obj The given image object.
 * @param x Location to store the X coordinate for the top left corner of the image in, or NULL.
 * @param y Location to store the Y coordinate for the top left corner of the image in, or NULL.
 * @param w Location to store the width of the image in, or NULL.
 * @param h Location to store the height of the image in, or NULL.
 * @ingroup Evas_Object_Image_Fill_Group
 */
EAPI void
evas_object_image_fill_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = o->cur.fill.x;
   if (y) *y = o->cur.fill.y;
   if (w) *w = o->cur.fill.w;
   if (h) *h = o->cur.fill.h;
}

/**
 * @defgroup Evas_Object_Image_Size Image Object Image Size Functions
 *
 * Functions that change the size of the image used by an image object.
 */

/**
 * Sets the size of the given image object.
 *
 * This function will scale down or crop the image so that it is
 * treated as if it were at the given size. If the size given is
 * smaller than the image, it will be cropped. If the size given is
 * larger, then the image will be treated as if it were in the upper
 * left hand corner of a larger image that is otherwise transparent.
 *
 * @param obj The given image object.
 * @param w The new width of the image.
 * @param h The new height of the image.
 * @ingroup Evas_Object_Image_Size
 */
EAPI void
evas_object_image_size_set(Evas_Object *obj, int w, int h)
{
   Evas_Object_Image *o;
   int stride;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if (w > 32768) return;
   if (h > 32768) return;
   if ((w == o->cur.image.w) &&
       (h == o->cur.image.h)) return;
   o->cur.image.w = w;
   o->cur.image.h = h;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_size_set(obj->layer->evas->engine.data.output,
								    o->engine_data,
								    w, h);
   else
     o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data
     (obj->layer->evas->engine.data.output, w, h, NULL, o->cur.has_alpha,
      o->cur.cspace);

   if (obj->layer->evas->engine.func->image_stride_get)
     obj->layer->evas->engine.func->image_stride_get(obj->layer->evas->engine.data.output,
						     o->engine_data, &stride);
   else
     stride = w;
   o->cur.image.stride = stride;

/* FIXME - in engine call above
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
*/
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the size of the given image object.
 *
 * See @ref evas_object_image_size_set for more details.
 *
 * @param obj The given image object.
 * @param w Location to store the width of the image in, or NULL.
 * @param h Location to store the height of the image in, or NULL.
 * @ingroup Evas_Object_Image_Size
 */
EAPI void
evas_object_image_size_get(const Evas_Object *obj, int *w, int *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (w) *w = o->cur.image.w;
   if (h) *h = o->cur.image.h;
}

/**
 * Retrieves the row stride of the given image object,
 *
 * The row stride is the number of units between the start of a
 * row and the start of the next row.
 *
 * @param obj The given image object.
 * @return The stride of the image.
 * @ingroup Evas_Object_Image_Size
 */
EAPI int
evas_object_image_stride_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.image.stride;
}

/**
 * Retrieves a number representing any error that occurred during the last
 * load of the given image object.
 *
 * @param obj The given image object.
 * @return A value giving the last error that occurred. It should be one of
 *         the @c EVAS_LOAD_ERROR_* values.  @c EVAS_LOAD_ERROR_NONE is
 *         returned if there was no error.
 * @ingroup Evas_Object_Image
 */
EAPI int
evas_object_image_load_error_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->load_error;
}

/**
 * @defgroup Evas_Object_Image_Data Image Object Image Data Functions
 *
 * Functions that allow you to access or modify the image pixel data of an
 * image object.
 */

/**
 * Converts the raw image data of the given image object to the
 * specified colorspace.
 *
 * Note that this function does not modify the raw image data.
 * If the requested colorspace is the same as the image colorspace
 * nothing is done and NULL is returned. You should use
 * evas_object_image_colorspace_get() to check the current image
 * colorspace.
 *
 * See @ref evas_object_image_colorspace_get.
 *
 * @param obj The given image object.
 * @param to_cspace The colorspace to which the image raw data will be converted.
 * @return data A newly allocated data in the format specified by to_cspace.
 * @ingroup Evas_Object_Image_Data
 */
EAPI void *
evas_object_image_data_convert(Evas_Object *obj, Evas_Colorspace to_cspace)
{
   Evas_Object_Image *o;
   DATA32 *data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!o->engine_data) return NULL;
   if (!o->cur.cspace == to_cspace) return NULL;
   data = NULL;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  0,
								  &data);
   return evas_object_image_data_convert_internal(o, data, to_cspace);
}

/**
 * Sets the raw image data of the given image object.
 *
 * Note that the raw data must be of the same size and colorspace
 * of the image. If data is NULL the current image data will be freed.
 *
 * @param obj The given image object.
 * @param data The raw data, or NULL.
 * @ingroup Evas_Object_Image_Data
 */
EAPI void
evas_object_image_data_set(Evas_Object *obj, void *data)
{
   Evas_Object_Image *o;
   void *p_data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   p_data = o->engine_data;
   if (data)
     {
	if (o->engine_data)
	  o->engine_data = obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
									 o->engine_data,
									 data);
	else
	  o->engine_data = obj->layer->evas->engine.func->image_new_from_data(obj->layer->evas->engine.data.output,
									      o->cur.image.w,
									      o->cur.image.h,
									      data,
									      o->cur.has_alpha,
									      o->cur.cspace);
     }
   else
     {
	if (o->engine_data)
	  obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
						    o->engine_data);
	o->load_error = EVAS_LOAD_ERROR_NONE;
	o->cur.image.w = 0;
	o->cur.image.h = 0;
	o->cur.image.stride = 0;
	o->engine_data = NULL;
     }
/* FIXME - in engine call above
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
*/
   if (o->pixels_checked_out > 0) o->pixels_checked_out--;
   if (p_data != o->engine_data)
     {
	EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
	o->pixels_checked_out = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Get a pointer to the raw image data of the given image object.
 *
 * This function returns a pointer to an image object's internal pixel buffer,
 * for reading only or read/write. If you request it for writing, the image
 * will be marked dirty so that it gets redrawn at the next update.
 *
 * This is best suited when you want to modify an existing image,
 * without changing its dimensions.
 *
 * @param obj The given image object.
 * @param for_writing Whether the data being retrieved will be modified.
 * @return The raw image data.
 * @ingroup Evas_Object_Image_Data
 */
EAPI void *
evas_object_image_data_get(const Evas_Object *obj, Evas_Bool for_writing)
{
   Evas_Object_Image *o;
   DATA32 *data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!o->engine_data) return NULL;
   data = NULL;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  for_writing,
								  &data);
   o->pixels_checked_out++;
   if (for_writing)
     {
	EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
     }

   return data;
}

/**
 * Replaces the raw image data of the given image object.
 *
 * This function lets the application replace an image object's internal pixel
 * buffer with a user-allocated one. For best results, you should generally
 * first call evas_object_image_size_set() with the width and height for the
 * new buffer.
 *
 * This call is best suited for when you will be using image data with
 * different dimensions than the existing image data, if any. If you only need
 * to modify the existing image in some fashion, then using
 * evas_object_image_data_get() is probably what you are after.
 *
 * Note that the caller is responsible for freeing the buffer when finished
 * with it, as user-set image data will not be automatically freed when the
 * image object is deleted.
 *
 * See @ref evas_object_image_data_get for more details.
 *
 * @param obj The given image object.
 * @param data The raw data.
 * @ingroup Evas_Object_Image_Data
 */
EAPI void
evas_object_image_data_copy_set(Evas_Object *obj, void *data)
{
   Evas_Object_Image *o;

   if (!data) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.image.w <= 0) ||
       (o->cur.image.h <= 0)) return;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data(obj->layer->evas->engine.data.output,
									      o->cur.image.w,
									      o->cur.image.h,
									      data,
									      o->cur.has_alpha,
									      o->cur.cspace);
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
   o->pixels_checked_out = 0;
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
}

/**
 * Mark a sub-region of the given image object to be redrawn.
 *
 * This function schedules a particular rectangular region of an image
 * object to be updated (redrawn) at the next render.
 *
 * @param obj The given image object.
 * @param x X-offset of the region to be updated.
 * @param y Y-offset of the region to be updated.
 * @param w Width of the region to be updated.
 * @param h Height of the region to be updated.
 * @ingroup Evas_Object_Image_Data
 */
EAPI void
evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h)
{
   Evas_Object_Image *o;
   Evas_Rectangle *r;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur.image.w, o->cur.image.h);
   if ((w <= 0)  || (h <= 0)) return;
   NEW_RECT(r, x, y, w, h);
   if (r) o->pixel_updates = evas_list_append(o->pixel_updates, r);
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * @defgroup Evas_Object_Image_Alpha Image Object Image Alpha Functions
 *
 * Functions that change the alpha of an image object.
 */

/**
 * Enable or disable alpha channel of the given image object.
 *
 * This function sets a flag on an image object indicating whether or not to
 * use alpha channel data. A value of 1 indicates to use alpha channel data,
 * and 0 indicates to ignore any alpha channel data. Note that this has
 * nothing to do with an object's color as manipulated by
 * evas_object_color_set().
 *
 * @param obj The given image object.
 * @param has_alpha Whether to use alpha channel data or not.
 * @ingroup Evas_Object_Image_Alpha
 */
EAPI void
evas_object_image_alpha_set(Evas_Object *obj, Evas_Bool has_alpha)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (((has_alpha) && (o->cur.has_alpha)) ||
       ((!has_alpha) && (!o->cur.has_alpha)))
     return;
   o->cur.has_alpha = has_alpha;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
   evas_object_image_data_update_add(obj, 0, 0, o->cur.image.w, o->cur.image.h);
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
}


/**
 * Retrieves the alpha channel setting of the given image object.
 *
 * This function returns 1 if the image object's alpha channel is being used,
 * or 0 otherwise.
 *
 * See @ref evas_object_image_alpha_set for more details.
 *
 * @param obj The given image object.
 * @return Whether the alpha channel data is being used.
 * @ingroup Evas_Object_Image_Alpha
 */
EAPI Evas_Bool
evas_object_image_alpha_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.has_alpha;
}

/**
 * @defgroup Evas_Object_Image_Scale Image Object Image Scaling Functions
 *
 * Functions that change the scaling quality of an image object.
 */

/**
 * Sets whether to use of high-quality image scaling algorithm
 * of the given image object.
 *
 * When enabled, a higher quality image scaling algorithm is used when scaling
 * images to sizes other than the source image. This gives better results but
 * is more computationally expensive.
 *
 * @param obj The given image object.
 * @param smooth_scale Whether to use smooth scale or not.
 * @ingroup Evas_Object_Image_Scale
 */
EAPI void
evas_object_image_smooth_scale_set(Evas_Object *obj, Evas_Bool smooth_scale)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (((smooth_scale) && (o->cur.smooth_scale)) ||
       ((!smooth_scale) && (!o->cur.smooth_scale)))
     return;
   o->cur.smooth_scale = smooth_scale;
}

/**
 * Retrieves whether the given image object is using use a high-quality
 * image scaling algorithm.
 *
 * See @ref evas_object_image_smooth_scale_set for more details.
 *
 * @param obj The given image object.
 * @return Whether smooth scale is being used.
 * @ingroup Evas_Object_Image_Scale
 */
EAPI Evas_Bool
evas_object_image_smooth_scale_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.smooth_scale;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_image_reload(Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((!o->cur.file) ||
       (o->pixels_checked_out > 0)) return;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
   evas_object_image_unload(obj);
   evas_object_image_load(obj);
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Save the given image object to a file.
 *
 * Note that you should pass the filename extension when saving.
 * If the file supports multiple data stored in it as eet,
 * you can specify the key to be used as the index of the
 * image in this file.
 *
 * You can specify some flags when saving the image.
 * Currently acceptable flags are quality and compress.
 * Eg.: "quality=100 compress=9"
 *
 * @param obj The given image object.
 * @param file The filename to be used to save the image.
 * @param key The image key in file, or NULL.
 * @param flags String containing the flags to be used.
 * @ingroup Evas_Object_Image
 */
EAPI Evas_Bool
evas_object_image_save(const Evas_Object *obj, const char *file, const char *key, const char *flags)
{
   Evas_Object_Image *o;
   DATA32 *data = NULL;
   int quality = 80, compress = 9, ok = 0;
   RGBA_Image *im;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();

   if (!o->engine_data) return 0;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  0,
								  &data);
   if (flags)
     {
	char *p, *pp;
	char *tflags;
	
	tflags = alloca(strlen(flags) + 1);
	strcpy(tflags, flags);
	p = tflags;
	while (p)
	  {
	     pp = strchr(p, ' ');
	     if (pp) *pp = 0;
	     sscanf(p, "quality=%i", &quality);
	     sscanf(p, "compress=%i", &compress);
	     if (pp) p = pp + 1;
	     else break;
	  }
     }
   im = (RGBA_Image*) evas_cache_image_data(evas_common_image_cache_get(),
                                            o->cur.image.w,
                                            o->cur.image.h,
                                            data,
                                            o->cur.has_alpha,
                                            EVAS_COLORSPACE_ARGB8888);
   if (im)
     {
	if (o->cur.cspace == EVAS_COLORSPACE_ARGB8888)
	  im->image.data = data;
	else
	  im->image.data = evas_object_image_data_convert_internal(o,
								   data,
								   EVAS_COLORSPACE_ARGB8888);
	if (im->image.data)
	  {
	     ok = evas_common_save_image_to_file(im, file, key, quality, compress);

	     if (o->cur.cspace != EVAS_COLORSPACE_ARGB8888)
	       free(im->image.data);
	  }

	evas_cache_image_drop(&im->cache_entry);
     }
   return ok;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Bool
evas_object_image_pixels_import(Evas_Object *obj, Evas_Pixel_Import_Source *pixels)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();

   if ((pixels->w != o->cur.image.w) || (pixels->h != o->cur.image.h)) return 0;
   switch (pixels->format)
     {
#if 0
      case EVAS_PIXEL_FORMAT_ARGB32:
	  {
	     if (o->engine_data)
	       {
		  DATA32 *image_pixels = NULL;

		  o->engine_data =
		    obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  1,
								  &image_pixels);
/* FIXME: need to actualyl support this */
/*		  memcpy(image_pixels, pixels->rows, o->cur.image.w * o->cur.image.h * 4);*/
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  image_pixels);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								   o->engine_data,
								   o->cur.has_alpha);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
	break;
#endif
#ifdef BUILD_CONVERT_YUV
      case EVAS_PIXEL_FORMAT_YUV420P_601:
	  {
	     if (o->engine_data)
	       {
		  DATA32 *image_pixels = NULL;

		  o->engine_data =
		    obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  1,
								  &image_pixels);
		  if (image_pixels)
		    evas_common_convert_yuv_420p_601_rgba((DATA8 **) pixels->rows,
							  (DATA8 *) image_pixels,
							  o->cur.image.w,
							  o->cur.image.h);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  image_pixels);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								   o->engine_data,
								   o->cur.has_alpha);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
	break;
#endif
      default:
	return 0;
	break;
     }
   return 1;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_image_pixels_get_callback_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *o), void *data)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   o->func.get_pixels = func;
   o->func.get_pixels_data = data;
}

/**
 * Mark whether the given image object is dirty (needs to be redrawn).
 *
 * @param obj The given image object.
 * @param dirty Whether the image is dirty.
 * @ingroup Evas_Object_Image
 */
EAPI void
evas_object_image_pixels_dirty_set(Evas_Object *obj, Evas_Bool dirty)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (dirty) o->dirty_pixels = 1;
   else o->dirty_pixels = 0;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves whether the given image object is dirty (needs to be redrawn).
 *
 * @param obj The given image object.
 * @return Whether the image is dirty.
 * @ingroup Evas_Object_Image
 */
EAPI Evas_Bool
evas_object_image_pixels_dirty_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   if (o->dirty_pixels) return 1;
   return 0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_image_load_dpi_set(Evas_Object *obj, double dpi)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   o->load_opts.dpi = dpi;
   if (o->cur.file)
     {
	evas_object_image_unload(obj);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI double
evas_object_image_load_dpi_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0.0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0.0;
   MAGIC_CHECK_END();
   return o->load_opts.dpi;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_image_load_size_set(Evas_Object *obj, int w, int h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   o->load_opts.w = w;
   o->load_opts.h = h;
   if (o->cur.file)
     {
	evas_object_image_unload(obj);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

EAPI void
evas_object_image_load_size_get(const Evas_Object *obj, int *w, int *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (w) *w = o->load_opts.w;
   if (h) *h = o->load_opts.h;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   o->load_opts.scale_down_by = scale_down;
   if (o->cur.file)
     {
	evas_object_image_unload(obj);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI int
evas_object_image_load_scale_down_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->load_opts.scale_down_by;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_image_colorspace_set(Evas_Object *obj, Evas_Colorspace cspace)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   o->cur.cspace = cspace;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_colorspace_set(obj->layer->evas->engine.data.output,
							 o->engine_data,
							 cspace);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Colorspace
evas_object_image_colorspace_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_COLORSPACE_ARGB8888;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_COLORSPACE_ARGB8888;
   MAGIC_CHECK_END();
   return o->cur.cspace;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_image_native_surface_set(Evas_Object *obj, Evas_Native_Surface *surf)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   obj->layer->evas->engine.func->image_native_set(obj->layer->evas->engine.data.output,
						   o->engine_data,
						   surf);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Native_Surface *
evas_object_image_native_surface_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   return obj->layer->evas->engine.func->image_native_get(obj->layer->evas->engine.data.output,
							  o->engine_data);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_image_cache_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   e->engine.func->image_cache_flush(e->engine.data.output);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_image_cache_reload(Evas *e)
{
   Evas_Object_List *l;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_image_cache_flush(e);
   for (l = (Evas_Object_List *)e->layers; l; l = l->next)
     {
	Evas_Layer *layer;
	Evas_Object_List *l2;

	layer = (Evas_Layer *)l;
        for (l2 = (Evas_Object_List *)layer->objects; l2; l2 = l2->next)
	  {
	     Evas_Object *obj;
	     Evas_Object_Image *o;

	     obj = (Evas_Object *)l2;
	     o = (Evas_Object_Image *)(obj->object_data);
	     if (o->magic == MAGIC_OBJ_IMAGE)
	       {
		  evas_object_image_unload(obj);
	       }
	  }
     }
   evas_image_cache_flush(e);
   for (l = (Evas_Object_List *)e->layers; l; l = l->next)
     {
	Evas_Layer *layer;
	Evas_Object_List *l2;

	layer = (Evas_Layer *)l;
        for (l2 = (Evas_Object_List *)layer->objects; l2; l2 = l2->next)
	  {
	     Evas_Object *obj;
	     Evas_Object_Image *o;

	     obj = (Evas_Object *)l2;
	     o = (Evas_Object_Image *)(obj->object_data);
	     if (o->magic == MAGIC_OBJ_IMAGE)
	       {
		  evas_object_image_load(obj);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
     }
   evas_image_cache_flush(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_image_cache_set(Evas *e, int size)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (size < 0) size = 0;
   e->engine.func->image_cache_set(e->engine.data.output, size);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI int
evas_image_cache_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();

   return e->engine.func->image_cache_get(e->engine.data.output);
}

/* all nice and private */

static void
evas_object_image_unload(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);

   if ((!o->cur.file) ||
       (o->pixels_checked_out > 0)) return;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output,
									o->engine_data,
									0, 0,
									o->cur.image.w, o->cur.image.h);
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->engine_data = NULL;
   o->load_error = EVAS_LOAD_ERROR_NONE;
   o->cur.has_alpha = 1;
   o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
   o->cur.image.w = 0;
   o->cur.image.h = 0;
   o->cur.image.stride = 0;
}

static void
evas_object_image_load(Evas_Object *obj)
{
   Evas_Object_Image *o;
   Evas_Image_Load_Opts lo;

   o = (Evas_Object_Image *)(obj->object_data);
   if (o->engine_data) return;

   lo.scale_down_by = o->load_opts.scale_down_by;
   lo.dpi = o->load_opts.dpi;
   lo.w = o->load_opts.w;
   lo.h = o->load_opts.h;
   o->engine_data = obj->layer->evas->engine.func->image_load(obj->layer->evas->engine.data.output,
							      o->cur.file,
							      o->cur.key,
							      &o->load_error,
							      &lo);
   if (o->engine_data)
     {
	int w, h;
	int stride;

	obj->layer->evas->engine.func->image_size_get(obj->layer->evas->engine.data.output,
						      o->engine_data, &w, &h);
	if (obj->layer->evas->engine.func->image_stride_get)
	  obj->layer->evas->engine.func->image_stride_get(obj->layer->evas->engine.data.output,
							  o->engine_data, &stride);
	else
	  stride = w;
	o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get(obj->layer->evas->engine.data.output,
									  o->engine_data);
	o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output,
									    o->engine_data);
	o->cur.image.w = w;
	o->cur.image.h = h;
	o->cur.image.stride = stride;
     }
   else
     {
	o->load_error = EVAS_LOAD_ERROR_GENERIC;
     }
}

static Evas_Coord
evas_object_image_figure_x_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret)
{
   Evas_Coord w;

   w = ((size * obj->layer->evas->output.w) /
	(Evas_Coord)obj->layer->evas->viewport.w);
   if (size <= 0) size = 1;
   if (start > 0)
     {
	while (start - size > 0) start -= size;
     }
   else if (start < 0)
     {
	while (start < 0) start += size;
     }
   start = ((start * obj->layer->evas->output.w) /
	    (Evas_Coord)obj->layer->evas->viewport.w);
   *size_ret = w;
   return start;
}

static Evas_Coord
evas_object_image_figure_y_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret)
{
   Evas_Coord h;

   h = ((size * obj->layer->evas->output.h) /
	(Evas_Coord)obj->layer->evas->viewport.h);
   if (size <= 0) size = 1;
   if (start > 0)
     {
	while (start - size > 0) start -= size;
     }
   else if (start < 0)
     {
	while (start < 0) start += size;
     }
   start = ((start * obj->layer->evas->output.h) /
	    (Evas_Coord)obj->layer->evas->viewport.h);
   *size_ret = h;
   return start;
}

static void
evas_object_image_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_image_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0;
   obj->cur.geometry.y = 0;
   obj->cur.geometry.w = 0;
   obj->cur.geometry.h = 0;
   obj->cur.layer = 0;
   obj->cur.anti_alias = 0;
   obj->cur.render_op = EVAS_RENDER_BLEND;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_image_new(void)
{
   Evas_Object_Image *o;

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Image));
   o->magic = MAGIC_OBJ_IMAGE;
   o->cur.fill.w = 1;
   o->cur.fill.h = 1;
   o->cur.smooth_scale = 1;
   o->cur.border.fill = 1;
   o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
   o->prev = o->cur;
   return o;
}

static void
evas_object_image_free(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   if (o->cur.file) evas_stringshare_del(o->cur.file);
   if (o->cur.key) evas_stringshare_del(o->cur.key);
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->engine_data = NULL;
   o->magic = 0;
   while (o->pixel_updates)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)o->pixel_updates->data;
	o->pixel_updates = evas_list_remove(o->pixel_updates, r);
	free(r);
     }
   free(o);
}

static void
evas_object_image_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Image *o;

   /* render object to surface with context, and offset by x,y */
   o = (Evas_Object_Image *)(obj->object_data);
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    255, 255, 255, 255);

   if ((obj->cur.cache.clip.r == 255) &&
       (obj->cur.cache.clip.g == 255) &&
       (obj->cur.cache.clip.b == 255) &&
       (obj->cur.cache.clip.a == 255))
     {
	obj->layer->evas->engine.func->context_multiplier_unset(output,
								context);
     }
   else
     obj->layer->evas->engine.func->context_multiplier_set(output,
							   context,
							   obj->cur.cache.clip.r,
							   obj->cur.cache.clip.g,
							   obj->cur.cache.clip.b,
							   obj->cur.cache.clip.a);

   obj->layer->evas->engine.func->context_render_op_set(output, context,
							obj->cur.render_op);
   if (o->engine_data)
     {
	Evas_Coord idw, idh, idx, idy;
	int ix, iy, iw, ih;

	if (o->dirty_pixels)
	  {
	     if (o->func.get_pixels)
	       {
		  o->func.get_pixels(o->func.get_pixels_data, obj);
		  o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
	       }
	     o->dirty_pixels = 0;
	  }
	o->engine_data = obj->layer->evas->engine.func->image_border_set(output, o->engine_data,
									 o->cur.border.l, o->cur.border.r,
									 o->cur.border.t, o->cur.border.b);
	idx = evas_object_image_figure_x_fill(obj, o->cur.fill.x, o->cur.fill.w, &idw);
	idy = evas_object_image_figure_y_fill(obj, o->cur.fill.y, o->cur.fill.h, &idh);
	if (idw < 1.0) idw = 1.0;
	if (idh < 1.0) idh = 1.0;
	if (idx > 0.0) idx -= idw;
	if (idy > 0.0) idy -= idh;
	while ((int)idx < obj->cur.geometry.w)
	  {
	     Evas_Coord ydy;
	     int dobreak_w = 0;

	     ydy = idy;
	     ix = idx;
	     if ((o->cur.fill.w == obj->cur.geometry.w) &&
		 (o->cur.fill.x == 0.0))
	       {
		  dobreak_w = 1;
		  iw = obj->cur.geometry.w;
	       }
	     else
	       iw = ((int)(idx + idw)) - ix;
	     while ((int)idy < obj->cur.geometry.h)
	       {
		  int dobreak_h = 0;

		  iy = idy;
		  if ((o->cur.fill.h == obj->cur.geometry.h) &&
		      (o->cur.fill.y == 0.0))
		    {
		       ih = obj->cur.geometry.h;
		       dobreak_h = 1;
		    }
		  else
		    ih = ((int)(idy + idh)) - iy;
		  if ((o->cur.border.l == 0) &&
		      (o->cur.border.r == 0) &&
		      (o->cur.border.t == 0) &&
		      (o->cur.border.b == 0) &&
		      (o->cur.border.fill != 0))
		    obj->layer->evas->engine.func->image_draw(output,
							      context,
							      surface,
							      o->engine_data,
							      0, 0,
							      o->cur.image.w,
							      o->cur.image.h,
							      obj->cur.geometry.x + ix + x,
							      obj->cur.geometry.y + iy + y,
							      iw, ih,
							      o->cur.smooth_scale);
		  else
		    {
		       int inx, iny, inw, inh, outx, outy, outw, outh;
		       int bl, br, bt, bb;
		       int imw, imh, ox, oy;

		       ox = obj->cur.geometry.x + ix + x;
		       oy = obj->cur.geometry.y + iy + y;
		       imw = o->cur.image.w;
		       imh = o->cur.image.h;
		       bl = o->cur.border.l;
		       br = o->cur.border.r;
		       bt = o->cur.border.t;
		       bb = o->cur.border.b;
		       if ((bl + br) > iw)
			 {
			    bl = iw / 2;
			    br = iw - bl;
			 }
		       if ((bl + br) > imw)
			 {
			    bl = imw / 2;
			    br = imw - bl;
			 }
		       if ((bt + bb) > ih)
			 {
			    bt = ih / 2;
			    bb = ih - bt;
			 }
		       if ((bt + bb) > imh)
			 {
			    bt = imh / 2;
			    bb = imh - bt;
			 }

		       inx = 0; iny = 0;
		       inw = bl; inh = bt;
		       outx = ox; outy = oy;
		       outw = bl; outh = bt;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = bl; iny = 0;
		       inw = imw - bl - br; inh = bt;
		       outx = ox + bl; outy = oy;
		       outw = iw - bl - br; outh = bt;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = imw - br; iny = 0;
		       inw = br; inh = bt;
		       outx = ox + iw - br; outy = oy;
		       outw = br; outh = bt;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);

		       inx = 0; iny = bt;
		       inw = bl; inh = imh - bt - bb;
		       outx = ox; outy = oy + bt;
		       outw = bl; outh = ih - bt - bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       if (o->cur.border.fill)
			 {
			    inx = bl; iny = bt;
			    inw = imw - bl - br; inh = imh - bt - bb;
			    outx = ox + bl; outy = oy + bt;
			    outw = iw - bl - br; outh = ih - bt - bb;
			    obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
			 }
		       inx = imw - br; iny = bt;
		       inw = br; inh = imh - bt - bb;
		       outx = ox + iw - br; outy = oy + bt;
		       outw = br; outh = ih - bt - bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);

		       inx = 0; iny = imh - bb;
		       inw = bl; inh = bb;
		       outx = ox; outy = oy + ih - bb;
		       outw = bl; outh = bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = bl; iny = imh - bb;
		       inw = imw - bl - br; inh = bb;
		       outx = ox + bl; outy = oy + ih - bb;
		       outw = iw - bl - br; outh = bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		       inx = imw - br; iny = imh - bb;
		       inw = br; inh = bb;
		       outx = ox + iw - br; outy = oy + ih - bb;
		       outw = br; outh = bb;
		       obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
		    }
		  idy += idh;
		  if (dobreak_h) break;
	       }
	     idx += idw;
	     idy = ydy;
	     if (dobreak_w) break;
	  }
     }
}

static void
evas_object_image_render_pre(Evas_Object *obj)
{
   Evas_Rectangles rects = { 0, 0, NULL };
   Evas_Object_Image *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   o = (Evas_Object_Image *)(obj->object_data);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&rects, obj, is_v, was_v);
	if (!o->pixel_updates) goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&rects, obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed render op */
   if (obj->cur.render_op != obj->prev.render_op)
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed anti_alias */
   if (obj->cur.anti_alias != obj->prev.anti_alias)
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	if (!o->pixel_updates) goto done;
     }
   if (o->changed)
     {
	if (((o->cur.file) && (!o->prev.file)) ||
	    ((!o->cur.file) && (o->prev.file)) ||
	    ((o->cur.key) && (!o->prev.key)) ||
	    ((!o->cur.key) && (o->prev.key))
	    )
	  {
	     evas_object_render_pre_prev_cur_add(&rects, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if ((o->cur.image.w != o->prev.image.w) ||
	    (o->cur.image.h != o->prev.image.h) ||
	    (o->cur.has_alpha != o->prev.has_alpha) ||
	    (o->cur.cspace != o->prev.cspace) ||
	    (o->cur.smooth_scale != o->prev.smooth_scale))
	  {
	     evas_object_render_pre_prev_cur_add(&rects, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if ((o->cur.border.l != o->prev.border.l) ||
	    (o->cur.border.r != o->prev.border.r) ||
	    (o->cur.border.t != o->prev.border.t) ||
	    (o->cur.border.b != o->prev.border.b))
	  {
	     evas_object_render_pre_prev_cur_add(&rects, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if (o->dirty_pixels)
	  {
	     evas_object_render_pre_prev_cur_add(&rects, obj);
	     if (!o->pixel_updates) goto done;
	  }
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* caluclate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h)) &&
       (o->cur.fill.w == o->prev.fill.w) &&
       (o->cur.fill.h == o->prev.fill.h) &&
       ((o->cur.fill.x + obj->cur.geometry.x) == (o->prev.fill.x + obj->prev.geometry.x)) &&
       ((o->cur.fill.y + obj->cur.geometry.y) == (o->prev.fill.y + obj->prev.geometry.y)) &&
       (!o->pixel_updates)
       )
     {
	evas_rects_return_difference_rects(&rects,
					   obj->cur.geometry.x,
					   obj->cur.geometry.y,
					   obj->cur.geometry.w,
					   obj->cur.geometry.h,
					   obj->prev.geometry.x,
					   obj->prev.geometry.y,
					   obj->prev.geometry.w,
					   obj->prev.geometry.h);
	if (!o->pixel_updates) goto done;
     }
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h))
       )
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	if (!o->pixel_updates) goto done;
     }
   if (o->changed)
     {
	if ((o->cur.fill.x != o->prev.fill.x) ||
	    (o->cur.fill.y != o->prev.fill.y) ||
	    (o->cur.fill.w != o->prev.fill.w) ||
	    (o->cur.fill.h != o->prev.fill.h))
	  {
	     evas_object_render_pre_prev_cur_add(&rects, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if ((o->cur.border.l == 0) &&
	    (o->cur.border.r == 0) &&
	    (o->cur.border.t == 0) &&
	    (o->cur.border.b == 0))
	  {
	     while (o->pixel_updates)
	       {
		  Evas_Rectangle *rr;
		  Evas_Coord idw, idh, idx, idy;
		  int x, y, w, h;

		  rr = o->pixel_updates->data;
		  o->pixel_updates = evas_list_remove(o->pixel_updates, rr);
		  obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, rr->x, rr->y, rr->w, rr->h);
		  
		  idx = evas_object_image_figure_x_fill(obj, o->cur.fill.x, o->cur.fill.w, &idw);
		  idy = evas_object_image_figure_y_fill(obj, o->cur.fill.y, o->cur.fill.h, &idh);

		  if (idw < 1) idw = 1;
		  if (idh < 1) idh = 1;
		  if (idx > 0) idx -= idw;
		  if (idy > 0) idy -= idh;
		  while (idx < obj->cur.geometry.w)
		    {
		       Evas_Coord ydy;

		       ydy = idy;
		       x = idx;
		       w = ((int)(idx + idw)) - x;
		       while (idy < obj->cur.geometry.h)
			 {
			    Evas_Rectangle r;

			    y = idy;
			    h = ((int)(idy + idh)) - y;

			    r.x = ((rr->x - 1) * w) / o->cur.image.w;
			    r.y = ((rr->y - 1) * h) / o->cur.image.h;
			    r.w = ((rr->w + 2) * w) / o->cur.image.w;
			    r.h = ((rr->h + 2) * h) / o->cur.image.h;
			    r.x += obj->cur.geometry.x + x;
			    r.y += obj->cur.geometry.y + y;
			    evas_add_rect(&rects, r.x, r.y, r.w, r.h);
			    idy += h;
			 }
		       idx += idw;
		       idy = ydy;
		    }
		  free(rr);
	       }
	     goto done;
	  }
	else
	  {
	     if (o->pixel_updates)
	       {
		  while (o->pixel_updates)
		    {
		       Evas_Rectangle *r;

		       r = (Evas_Rectangle *)o->pixel_updates->data;
		       o->pixel_updates = evas_list_remove(o->pixel_updates, r);
		       free(r);
		    }
		  obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
		  evas_object_render_pre_prev_cur_add(&rects, obj);
		  goto done;
	       }
	  }
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* aren't fully opaque and we are visible */
   if (evas_object_is_visible(obj) &&
       evas_object_is_opaque(obj))
     obj->layer->evas->engine.func->output_redraws_rect_del(obj->layer->evas->engine.data.output,
							    obj->cur.cache.clip.x,
							    obj->cur.cache.clip.y,
							    obj->cur.cache.clip.w,
							    obj->cur.cache.clip.h);
   done:
   evas_object_render_pre_effect_updates(&rects, obj, is_v, was_v);
}

static void
evas_object_image_render_post(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Image *)(obj->object_data);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = evas_list_remove(obj->clip.changes, r);
	free(r);
     }
   while (o->pixel_updates)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)o->pixel_updates->data;
	o->pixel_updates = evas_list_remove(o->pixel_updates, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
   /* FIXME: copy strings across */
}

static int
evas_object_image_is_opaque(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire rectangle it occupies */
   o = (Evas_Object_Image *)(obj->object_data);
   if (((o->cur.border.l != 0) ||
	(o->cur.border.r != 0) ||
	(o->cur.border.t != 0) ||
	(o->cur.border.b != 0)) &&
       (!o->cur.border.fill)) return 0;
   if (!o->engine_data) return 0;
   if (obj->cur.render_op == EVAS_RENDER_COPY)
	return 1;
   if (o->cur.has_alpha) return 0;
   if (obj->cur.render_op != EVAS_RENDER_BLEND)
	return 0;
   return 1;
}

static int
evas_object_image_was_opaque(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire rectangle it occupies */
   o = (Evas_Object_Image *)(obj->object_data);
   if (((o->prev.border.l != 0) ||
	(o->prev.border.r != 0) ||
	(o->prev.border.t != 0) ||
	(o->prev.border.b != 0)) &&
       (!o->prev.border.fill)) return 0;
   if (!o->engine_data) return 0;
   if (obj->prev.render_op == EVAS_RENDER_COPY)
	return 1;
   if (o->prev.has_alpha) return 0;
   if (obj->prev.render_op != EVAS_RENDER_BLEND)
	return 0;
   return 1;
}

static int
evas_object_image_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Image *o;
   DATA32 *data;
   int w, h, stride;
   int a;

   o = (Evas_Object_Image *)(obj->object_data);

   x -= obj->cur.cache.clip.x;
   y -= obj->cur.cache.clip.y;
   w = o->cur.image.w;
   h = o->cur.image.h;

   if ((x > w) || (y > h))
     return 0;

   if (!o->cur.has_alpha)
     return 1;

   stride = o->cur.image.stride;

   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  0,
								  &data);
   if (!data)
     return 0;

   switch (o->cur.cspace)
     {
	case EVAS_COLORSPACE_ARGB8888:
	  data = ((DATA32*)(data) + ((y * stride) + x));
	  a = (*((DATA32*)(data)) >> 24) & 0xff;
	  break;
	case EVAS_COLORSPACE_RGB565_A5P:
           data = (void*) ((DATA16*)(data) + (h * stride));
	  data = (void*) ((DATA8*)(data) + ((y * stride) + x));
	  a = (*((DATA8*)(data))) & 0x1f;
	  break;
	default:
	  return 1;
	  break;
     }

   return (a != 0);
}

static void *
evas_object_image_data_convert_internal(Evas_Object_Image *o, void *data, Evas_Colorspace to_cspace)
{
   void *out = NULL;

   if (!data)
     return NULL;

   switch (o->cur.cspace)
     {
	case EVAS_COLORSPACE_ARGB8888:
	  out = evas_common_convert_argb8888_to(data,
						o->cur.image.w,
						o->cur.image.h,
						o->cur.image.stride,
						o->cur.has_alpha,
						to_cspace);
	  break;
	case EVAS_COLORSPACE_RGB565_A5P:
	  out = evas_common_convert_rgb565_a5p_to(data,
						  o->cur.image.w,
						  o->cur.image.h,
						  o->cur.image.stride,
						  o->cur.has_alpha,
						  to_cspace);
	  break;
	default:
	  break;
     }

   return out;
}
