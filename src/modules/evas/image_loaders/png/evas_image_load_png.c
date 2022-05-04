#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <png.h>
#include <setjmp.h>

#include "evas_common_private.h"
#include "evas_private.h"

#define PNG_BYTES_TO_CHECK 4
#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000

typedef struct _Evas_PNG_Info Evas_PNG_Info;
struct _Evas_PNG_Info
{
   unsigned char *map;
   size_t length;
   size_t position;

   png_structp png_ptr;
   png_infop info_ptr;
   png_uint_32 w32, h32;
   int bit_depth, color_type, interlace_type;

   volatile Eina_Bool hasa;
};

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
};

static const Evas_Colorspace cspace_grey[2] = {
   EVAS_COLORSPACE_GRY8,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspace_grey_alpha[2] = {
   EVAS_COLORSPACE_AGRY88,
   EVAS_COLORSPACE_ARGB8888
};

static void
_evas_image_png_update_x_content(Eina_Rectangle *r, int index)
{
   if (r->x == 0)
     {
        r->x = index;
        r->w = 1;
     }
   else
     {
        r->w = index - r->x;
     }
}

static void
_evas_image_png_update_y_content(Eina_Rectangle *r, int index)
{
   if (r->y == 0)
     {
        r->y = index;
        r->h = 1;
     }
   else
     {
        r->h = index - r->y;
     }
}

static void
_evas_image_png_read(png_structp png_ptr, png_bytep out, png_size_t count)
{
   Evas_PNG_Info *epi = png_get_io_ptr(png_ptr);

   if (!epi) return;
   if (epi->position == epi->length) return;

   if (epi->position + count > epi->length) count = epi->length - epi->position;
   memcpy(out, epi->map + epi->position, count);
   epi->position += count;
}

static void *
evas_image_load_file_open_png(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                              Evas_Image_Load_Opts *opts,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error)
{
   Evas_Loader_Internal *loader;

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   loader->f = f;
   loader->opts = opts;
   return loader;
}

static void
evas_image_load_file_close_png(void *loader_data)
{
   free(loader_data);
}

static Eina_Bool
_evas_image_load_file_internal_head_png(Evas_Loader_Internal *loader,
                                        Evas_Image_Property *prop,
                                        Evas_PNG_Info *epi,
                                        int *error, Eina_Bool is_for_head)
{
   Evas_Image_Load_Opts *opts = loader->opts;
   Eina_File *f = loader->f;
   volatile Eina_Bool r = EINA_FALSE;
   const char *filename;
   unsigned int filename_len = 0;

   *error = EVAS_LOAD_ERROR_NONE;

   epi->hasa = 0;
   if (!is_for_head)
     epi->map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   else
     epi->map = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!epi->map)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto close_file;
     }
   epi->length = eina_file_size_get(f);
   epi->position = 0;

   if (epi->length < PNG_BYTES_TO_CHECK)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto close_file;
     }
   if (png_sig_cmp(epi->map, 0, PNG_BYTES_TO_CHECK))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto close_file;
     }
   epi->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!epi->png_ptr)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto close_file;
     }
   epi->info_ptr = png_create_info_struct(epi->png_ptr);
   if (!epi->info_ptr)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto close_file;
     }
   png_set_read_fn(epi->png_ptr, epi, _evas_image_png_read);
   if (setjmp(png_jmpbuf(epi->png_ptr)))
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto close_file;
     }

   png_read_info(epi->png_ptr, epi->info_ptr);
   png_get_IHDR(epi->png_ptr, epi->info_ptr, (png_uint_32 *) (&epi->w32),
		(png_uint_32 *) (&epi->h32), &epi->bit_depth, &epi->color_type,
		&epi->interlace_type, NULL, NULL);
   if ((epi->w32 < 1) || (epi->h32 < 1) ||
       (epi->w32 > IMG_MAX_SIZE) || (epi->h32 > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(epi->w32, epi->h32))
     {
	if (IMG_TOO_BIG(epi->w32, epi->h32))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	goto close_file;
     }

   if (opts->emile.region.w > 0 && opts->emile.region.h > 0)
     {
        if (((int) epi->w32 < opts->emile.region.x + opts->emile.region.w) ||
            ((int) epi->h32 < opts->emile.region.y + opts->emile.region.h))
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto close_file;
          }
        if(opts->emile.scale_down_by > 1)
          {
             prop->info.w = opts->emile.region.w / opts->emile.scale_down_by;
             prop->info.h = opts->emile.region.h / opts->emile.scale_down_by;
          }
        else
          {
             prop->info.w = opts->emile.region.w;
             prop->info.h = opts->emile.region.h;
          }
     }
   else if (opts->emile.scale_down_by > 1)
     {
        prop->info.w = (int) epi->w32 / opts->emile.scale_down_by;
        prop->info.h = (int) epi->h32 / opts->emile.scale_down_by;
        if ((prop->info.w < 1) || (prop->info.h < 1))
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto close_file;
          }
     }
   else
     {
        prop->info.w = (int) epi->w32;
        prop->info.h = (int) epi->h32;
     }

   if (png_get_valid(epi->png_ptr, epi->info_ptr, PNG_INFO_tRNS))
     {
        /* expand transparency entry -> alpha channel if present */
        if (!is_for_head) png_set_tRNS_to_alpha(epi->png_ptr);
        epi->hasa = 1;
     }

   switch (epi->color_type)
     {
      case PNG_COLOR_TYPE_RGB_ALPHA:
         epi->hasa = 1;
         break;
      case PNG_COLOR_TYPE_GRAY_ALPHA:
         epi->hasa = 1;
         prop->info.cspaces = cspace_grey_alpha;
         break;
      case PNG_COLOR_TYPE_GRAY:
         if (!epi->hasa) prop->info.cspaces = cspace_grey;
         break;
     }
   if (epi->hasa) prop->info.alpha = 1;

   filename = eina_file_filename_get(f);
   if (filename) filename_len = strlen(filename);
   prop->need_data = (filename_len > 6 && filename[filename_len - 7] != '/') &&
                     (eina_str_has_extension(filename, ".9.png"));
   if (prop->need_data)
     {
        // Adjust size to take into account the 9 patch pixels information
        prop->info.w -= 2;
        prop->info.h -= 2;
     }

   r = EINA_TRUE;

   if (!is_for_head) return r;

 close_file:
   if (epi->png_ptr) png_destroy_read_struct(&epi->png_ptr,
                                        epi->info_ptr ? &epi->info_ptr : NULL,
                                        NULL);
   if (epi->map) eina_file_map_free(f, epi->map);
   memset(epi, 0, sizeof (Evas_PNG_Info));

   return r;
}

static Eina_Bool
evas_image_load_file_head_png(void *loader_data,
                              Evas_Image_Property *prop,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_PNG_Info epi;

   memset(&epi, 0, sizeof (Evas_PNG_Info));

   if (!_evas_image_load_file_internal_head_png(loader, prop, &epi, error, EINA_TRUE))
     return EINA_FALSE;

   return EINA_TRUE;
}

static inline Eina_Bool
_is_black(DATA32 *ptr)
{
   if (*ptr == BLACK) return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_head_with_data_png(void *loader_data,
                                        Evas_Image_Property *prop,
                                        void *pixels,
                                        int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Load_Opts *opts;
   Eina_File *f;

   unsigned char *src_ptr;
   unsigned char *pixels2;
   unsigned char *surface;
   Evas_PNG_Info epi;
   Eina_Rectangle region;
   unsigned int pack_offset;
   int w, h;
   char passes;
   int i, j, p, k;
   volatile int scale_ratio = 1;
   int image_w = 0, image_h = 0;
   volatile Eina_Bool r = EINA_FALSE;
   Eina_Bool nine_patch = EINA_FALSE;

   opts = loader->opts;
   f = loader->f;

   memset(&epi, 0, sizeof (Evas_PNG_Info));
   region = opts->emile.region;

   if (!_evas_image_load_file_internal_head_png(loader, prop, &epi, error, EINA_FALSE))
     return EINA_FALSE;

   if (setjmp(png_jmpbuf(epi.png_ptr)))
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto close_file;
     }

   image_w = epi.w32;
   image_h = epi.h32;

   // We are leveragging region code to not load the border information pixels
   // from 9patch files into the surface used by Evas
   if (prop->need_data)
     {
        nine_patch = EINA_TRUE;

        region.x += 1;
        region.y += 1;

        if (region.w > 0 && region.h > 0)
          {
             if (region.x + region.w + 1 < image_w) region.w += 1;
             else region.w = image_w - region.x - 1;
             if (region.y + region.h + 1 < image_h) region.h += 1;
             else region.h = image_h - region.y - 1;
          }
        else
          {
             region.w = image_w - region.x - 1;
             region.h = image_h - region.y - 1;
          }
     }

   surface = pixels;

   /* Prep for transformations...  ultimately we want ARGB */
   /* expand palette -> RGB if necessary */
   if (epi.color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(epi.png_ptr);
   /* expand gray (w/reduced bits) -> 8-bit RGB if necessary */
   if ((epi.color_type == PNG_COLOR_TYPE_GRAY) ||
       (epi.color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
     {
        if (prop->info.cspace == EVAS_COLORSPACE_ARGB8888)
          png_set_gray_to_rgb(epi.png_ptr);
	if (epi.bit_depth < 8) png_set_expand_gray_1_2_4_to_8(epi.png_ptr);
     }
   /* reduce 16bit color -> 8bit color if necessary */
   if (epi.bit_depth > 8) png_set_strip_16(epi.png_ptr);
   /* pack all pixels to byte boundaries */
   png_set_packing(epi.png_ptr);

   w = prop->info.w;
   h = prop->info.h;

   switch (prop->info.cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
         /* we want ARGB */
#ifdef WORDS_BIGENDIAN
         png_set_swap_alpha(epi.png_ptr);
         if (!epi.hasa) png_set_filler(epi.png_ptr, 0xff, PNG_FILLER_BEFORE);
#else
         png_set_bgr(epi.png_ptr);
         if (!epi.hasa) png_set_filler(epi.png_ptr, 0xff, PNG_FILLER_AFTER);
#endif
         pack_offset = sizeof(DATA32);
         break;
      case EVAS_COLORSPACE_AGRY88:
         /* we want AGRY */
#ifdef WORDS_BIGENDIAN
         png_set_swap_alpha(epi.png_ptr);
         if (!epi.hasa) png_set_filler(epi.png_ptr, 0xff, PNG_FILLER_BEFORE);
#else
         if (!epi.hasa) png_set_filler(epi.png_ptr, 0xff, PNG_FILLER_AFTER);
#endif
         pack_offset = sizeof(DATA16);
         break;
      case EVAS_COLORSPACE_GRY8: pack_offset = sizeof(DATA8); break;
      default: abort();
     }

   passes = png_set_interlace_handling(epi.png_ptr);

   /* we read image line by line in all case because of .9.png */
   pixels2 = malloc(image_w * image_h * pack_offset);
   if (!pixels2)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto close_file;
     }

   for (p = 0; p < passes; p++)
     {
        for (i = 0; i < image_h; i++)
          png_read_row(epi.png_ptr, pixels2 + (i * image_w * pack_offset), NULL);
     }
   png_read_end(epi.png_ptr, epi.info_ptr);

   if (nine_patch && pack_offset != sizeof (DATA32))
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        free(pixels2);
        goto close_file;
     }

   if (nine_patch)
     {
        DATA32 *src_ptr1;
        DATA32 *src_ptr2;
        Eina_Bool stretchable = EINA_FALSE;
        Eina_Rectangle optional_content = { 0 };
        uint8_t current = 0;

        memset(&prop->content, 0, sizeof (Eina_Rectangle));

        // Top line of the image
        src_ptr1 = (DATA32*) pixels2 + 1;
        // Bottom line of the image
        src_ptr2 = src_ptr1 + (image_h - 1) * image_w;

        // Extract top stretch zone and horizontal content area
        for (i = 1; i < image_w - 1;
             i++, src_ptr1++, src_ptr2++)
          {
             Eina_Bool bp1 = _is_black(src_ptr1);
             Eina_Bool bp2 = _is_black(src_ptr2);

             // Updating horizontal area where content can be located
             if (bp2)
               _evas_image_png_update_x_content(&prop->content, i);

             // In case no content area is provided, let's make it up
             if (bp1)
               _evas_image_png_update_x_content(&optional_content, i);

             // Switching from a non stretchable to a stretchable zone or the opposite
             if (!((stretchable && (bp1)) ||
                   (!stretchable && (!bp1))))
               {
                  evas_loader_helper_stretch_region_push(&prop->stretch.horizontal.region,
                                                         &current, stretchable);
                  stretchable = !stretchable;
               }

             // Keep counting in the area we are in
             current++;

             if (current != 0x7F) continue;

             // The bucket is full
             evas_loader_helper_stretch_region_push(&prop->stretch.horizontal.region,
                                                    &current, stretchable);
          }
        // End with strechable, add length info
        if (stretchable)
          {
             evas_loader_helper_stretch_region_push(&prop->stretch.horizontal.region,
                                                    &current, stretchable);
             stretchable = !stretchable;
          }

        current = 0;

        // Left border of the image
        src_ptr1 = (DATA32*) pixels2 + image_w;
        // Right border of the image
        src_ptr2 = src_ptr1 + (image_w - 1);

        for (i = 1; i < image_h - 1;
             i++, src_ptr1 += image_w, src_ptr2 += image_w)
          {
             Eina_Bool bp1 = _is_black(src_ptr1);
             Eina_Bool bp2 = _is_black(src_ptr2);

             // Updating vertical area where content can be located
             if (bp2)
               _evas_image_png_update_y_content(&prop->content, i);

             // In case no content area is provided, let's make it up
             if (bp1)
               _evas_image_png_update_y_content(&optional_content, i);

             // Switching from a non stretchable to a stretchable zone or the opposite
             if (!((stretchable && (bp1)) ||
                   (!stretchable && (!bp1))))
               {
                  evas_loader_helper_stretch_region_push(&prop->stretch.vertical.region,
                                                         &current, stretchable);
                  stretchable = !stretchable;
               }

             // Keep counting in the area we are in
             current++;

             if (current != 0x7F) continue;

             // The bucket is full
             evas_loader_helper_stretch_region_push(&prop->stretch.vertical.region,
                                                    &current, stretchable);
          }
        // End with strechable, add length info
        if (stretchable)
          {
             evas_loader_helper_stretch_region_push(&prop->stretch.vertical.region,
                                                    &current, stretchable);
             stretchable = !stretchable;
          }

        // Content zone is optional, if not provided, we should use the one we guessed
        if (prop->content.x == 0 || prop->content.y == 0)
          memcpy(&prop->content, &optional_content, sizeof (Eina_Rectangle));

        // Check that the limit of our content zone are correct
        // This could be handled as an incorrect file completely,
        // but let's try to recover
        if (prop->content.x == 0 || prop->content.y == 0)
          {
             *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
             free(pixels2);
             goto close_file;
          }
        if ((prop->content.x + prop->content.w >= image_w - 1) &&
            (prop->content.y + prop->content.h >= image_h - 1))
          {
             *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
             free(pixels2);
             goto close_file;
          }

        // Correct the work space to be the same as the image one for intersection
        // with region and correct back afterward.
        prop->content.x++;
        prop->content.y++;
        if (eina_rectangle_intersection(&prop->content, &region))
          {
             prop->content.x--;
             prop->content.y--;
          }
     }

   src_ptr = pixels2 + (region.y * image_w * pack_offset) + region.x * pack_offset;

   //general case: 4 bytes pixel.
   if (pack_offset == sizeof(DATA32))
     {
        DATA32 *dst_ptr = (DATA32 *) surface;
        DATA32 *src_ptr2 = (DATA32 *) src_ptr;

        for (i = 0; i < h; i++)
          {
             for (j = 0; j < w; j++)
               {
                  *dst_ptr = *src_ptr2;
                  ++dst_ptr;
                  src_ptr2 += scale_ratio;
               }
             src_ptr2 += scale_ratio * (image_w - w);
          }
     }
   else
     {
        unsigned char *dst_ptr = surface;

        for (i = 0; i < h; i++)
          {
             for (j = 0; j < w; j++)
               {
                  for (k = 0; k < (int)pack_offset; k++)
                    dst_ptr[k] = src_ptr[k + scale_ratio * j * pack_offset];
                  dst_ptr += pack_offset;
               }
             src_ptr += scale_ratio * (image_w - w) * pack_offset;
          }
     }
   free(pixels2);

   if ((epi.hasa) && (pack_offset == sizeof(DATA32)))
     {
        DATA32 *dst_ptr = (DATA32 *) surface;
        int total = w * h;

        for (i = 0; i < total; i++)
          {
             if (A_VAL(dst_ptr) == 0) *dst_ptr = 0;
             dst_ptr++;
          }
     }
   prop->info.premul = EINA_TRUE;

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 close_file:
   if (epi.png_ptr) png_destroy_read_struct(&epi.png_ptr,
                                            epi.info_ptr ? &epi.info_ptr : NULL,
                                            NULL);
   if (epi.map) eina_file_map_free(f, epi.map);
   return r;
}

static Eina_Bool
evas_image_load_file_data_png(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Load_Opts *opts;
   Eina_File *f;

   unsigned char *surface;
   Evas_PNG_Info epi;
   unsigned int pack_offset;
   int w, h;
   char passes;
   int i, j, p, k;
   volatile int scale_ratio = 1;
   volatile int region_set = 0;
   int image_w = 0, image_h = 0;
   volatile Eina_Bool r = EINA_FALSE;

   opts = loader->opts;
   f = loader->f;

   memset(&epi, 0, sizeof (Evas_PNG_Info));

   if (!_evas_image_load_file_internal_head_png(loader, prop, &epi, error, EINA_FALSE))
     return EINA_FALSE;

   if (setjmp(png_jmpbuf(epi.png_ptr)))
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto close_file;
     }

   image_w = epi.w32;
   image_h = epi.h32;
   if (opts->emile.scale_down_by > 1)
     {
        scale_ratio = opts->emile.scale_down_by;
        epi.w32 /= scale_ratio;
        epi.h32 /= scale_ratio;
     }

   if ((opts->emile.region.w > 0 && opts->emile.region.h > 0) &&
       (opts->emile.region.w != image_w || opts->emile.region.h != image_h))
     {
        epi.w32 = opts->emile.region.w / scale_ratio;
        epi.h32 = opts->emile.region.h / scale_ratio;
        region_set = 1;
     }

   if (prop->info.w != epi.w32 ||
       prop->info.h != epi.h32)
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto close_file;
     }

   surface = pixels;

   /* Prep for transformations...  ultimately we want ARGB */
   /* expand palette -> RGB if necessary */
   if (epi.color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(epi.png_ptr);
   /* expand gray (w/reduced bits) -> 8-bit RGB if necessary */
   if ((epi.color_type == PNG_COLOR_TYPE_GRAY) ||
       (epi.color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
     {
        if (prop->info.cspace == EVAS_COLORSPACE_ARGB8888)
          png_set_gray_to_rgb(epi.png_ptr);
	if (epi.bit_depth < 8) png_set_expand_gray_1_2_4_to_8(epi.png_ptr);
     }
   /* reduce 16bit color -> 8bit color if necessary */
   if (epi.bit_depth > 8) png_set_strip_16(epi.png_ptr);
   /* pack all pixels to byte boundaries */
   png_set_packing(epi.png_ptr);

   w = epi.w32;
   h = epi.h32;

   switch (prop->info.cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
         /* we want ARGB */
#ifdef WORDS_BIGENDIAN
         png_set_swap_alpha(epi.png_ptr);
         if (!epi.hasa) png_set_filler(epi.png_ptr, 0xff, PNG_FILLER_BEFORE);
#else
         png_set_bgr(epi.png_ptr);
         if (!epi.hasa) png_set_filler(epi.png_ptr, 0xff, PNG_FILLER_AFTER);
#endif
         pack_offset = sizeof(DATA32);
         break;
      case EVAS_COLORSPACE_AGRY88:
         /* we want AGRY */
#ifdef WORDS_BIGENDIAN
         png_set_swap_alpha(epi.png_ptr);
         if (!epi.hasa) png_set_filler(epi.png_ptr, 0xff, PNG_FILLER_BEFORE);
#else
         if (!epi.hasa) png_set_filler(epi.png_ptr, 0xff, PNG_FILLER_AFTER);
#endif
         pack_offset = sizeof(DATA16);
         break;
      case EVAS_COLORSPACE_GRY8: pack_offset = sizeof(DATA8); break;
      default: abort();
     }

   passes = png_set_interlace_handling(epi.png_ptr);

   /* we read image line by line if scale down was set */
   if (scale_ratio == 1 && region_set == 0)
     {
        for (p = 0; p < passes; p++)
          {
             for (i = 0; i < h; i++)
               png_read_row(epi.png_ptr, surface + (i * w * pack_offset), NULL);
          }
        png_read_end(epi.png_ptr, epi.info_ptr);
     }
   else
     {
        unsigned char *src_ptr;
        unsigned char *dst_ptr = surface;
        int skip_row = 0, region_x = 0, region_y = 0;

        if (region_set)
          {
             region_x = opts->emile.region.x;
             region_y = opts->emile.region.y;
          }

        if (passes == 1)
          {
             int line_size = (image_w * pack_offset) - (region_x * pack_offset);
             unsigned char *tmp_line = (unsigned char *) alloca(image_w * pack_offset);
             //accumulate pixel color here.
             unsigned short *interp_buf = (unsigned short *) alloca(line_size * sizeof(unsigned short));
             unsigned short *pbuf;

             for (skip_row = 0; skip_row < region_y; skip_row++)
               png_read_row(epi.png_ptr, tmp_line, NULL);

             png_read_row(epi.png_ptr, tmp_line, NULL);
             src_ptr = tmp_line + (region_x * pack_offset);

             //The first pixel, of the first line
             for (k = 0; k < (int) pack_offset; k++)
               dst_ptr[k] = src_ptr[k];

             dst_ptr += pack_offset;
             src_ptr += (scale_ratio * pack_offset);

             for (j = 1; j < w; j++)
               {
                  //rgba
                  interp_buf[0] = 0;
                  interp_buf[1] = 0;
                  interp_buf[2] = 0;
                  interp_buf[3] = 0;

                  //horizontal interpolation.
                  for (p = 0; p < scale_ratio; p++)
                    {
                       for (k = 0; k < (int) pack_offset; k++)
                         interp_buf[k] += src_ptr[k - (int)(p * pack_offset)];
                    }
                  for (k = 0; k < (int) pack_offset; k++)
                    dst_ptr[k] = (interp_buf[k] / scale_ratio);

                  dst_ptr += pack_offset;
                  src_ptr += (scale_ratio * pack_offset);
               }

             //next lines
             for (i = 1; i < h; i++)
               {
                  memset(interp_buf, 0x00, line_size * sizeof(unsigned short));

                  //vertical interpolation.
                  for (j = 0; j < scale_ratio; j++)
                    {
                       png_read_row(epi.png_ptr, tmp_line, NULL);
                       src_ptr = tmp_line + (region_x * pack_offset);

                      for (p = 0; p < line_size; ++p)
                         interp_buf[p] += src_ptr[p];
                    }

                  for (p = 0; p < line_size; ++p)
                    interp_buf[p] /= scale_ratio;

                  pbuf = interp_buf;

                  //The first pixel of the current line
                  for (k = 0; k < (int) pack_offset; k++)
                    dst_ptr[k] = pbuf[k];

                  dst_ptr += pack_offset;
                  pbuf += scale_ratio * pack_offset;

                  for (j = 1; j < w; j++)
                    {
                       //horizontal interpolation.
                       for (p = 1; p < scale_ratio; ++p)
                         {
                            for (k = 0; k < (int) pack_offset; k++)
                              pbuf[k] += pbuf[k - (int)(p * pack_offset)];
                         }
                       for (k = 0; k < (int) pack_offset; k++)
                         dst_ptr[k] = (pbuf[k] / scale_ratio);

                       dst_ptr += pack_offset;
                       pbuf += (scale_ratio * pack_offset);
                    }
               }

             for (skip_row = region_y + h * scale_ratio; skip_row < image_h; skip_row++)
               png_read_row(epi.png_ptr, tmp_line, NULL);
          }
        else
          {
             //TODO: Scale-down interpolation for multi-pass?

             unsigned char *pixels2 = malloc(image_w * image_h * pack_offset);

             if (pixels2)
               {
                  for (p = 0; p < passes; p++)
                    {
                       for (i = 0; i < image_h; i++)
                         png_read_row(epi.png_ptr, pixels2 + (i * image_w * pack_offset), NULL);
                    }
                  png_read_end(epi.png_ptr, epi.info_ptr);

                  src_ptr = pixels2 + (region_y * image_w * pack_offset) + region_x * pack_offset;

                  //general case: 4 bytes pixel.
                  if (pack_offset == sizeof(DATA32))
                    {
                       DATA32 *dst_ptr2 = (DATA32 *) surface;
                       DATA32 *src_ptr2 = (DATA32 *) src_ptr;

                       for (i = 0; i < h; i++)
                         {
                            for (j = 0; j < w; j++)
                              {
                                 *dst_ptr2 = *src_ptr2;
                                 ++dst_ptr2;
                                 src_ptr2 += scale_ratio;
                              }
                            src_ptr2 += scale_ratio * image_w;
                         }
                    }
                  else
                    {
                       unsigned char *dst_ptr2 = surface;

                       for (i = 0; i < h; i++)
                         {
                            for (j = 0; j < w; j++)
                              {
                                 for (k = 0; k < (int)pack_offset; k++)
                                   dst_ptr2[k] = src_ptr[k + scale_ratio * j * pack_offset];
                                 dst_ptr2 += pack_offset;
                              }
                            src_ptr += scale_ratio * image_w * pack_offset;
                         }
                    }
                  free(pixels2);
               }
          }
     }

   if ((epi.hasa) && (pack_offset == sizeof(DATA32)))
     {
        DATA32 *dst_ptr = (DATA32 *) surface;
        int total = w * h;

        for (i = 0; i < total; i++)
          {
             if (A_VAL(dst_ptr) == 0) *dst_ptr = 0;
             dst_ptr++;
          }
     }
   prop->info.premul = EINA_TRUE;

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 close_file:
   if (epi.png_ptr) png_destroy_read_struct(&epi.png_ptr,
                                            epi.info_ptr ? &epi.info_ptr : NULL,
                                            NULL);
   if (epi.map) eina_file_map_free(f, epi.map);
   return r;
}

static Evas_Image_Load_Func evas_image_load_png_func =
{
  EVAS_IMAGE_LOAD_VERSION,
  evas_image_load_file_open_png,
  evas_image_load_file_close_png,
  evas_image_load_file_head_png,
  evas_image_load_file_head_with_data_png,
  evas_image_load_file_data_png,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_png_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "png",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, png);

#ifndef EVAS_STATIC_BUILD_PNG
EVAS_EINA_MODULE_DEFINE(image_loader, png);
#endif
