#include "config.h"
#include "evas_options.h"

#define SWAP32(x) (x) = ((((x) & 0x000000ff ) << 24) | (((x) & 0x0000ff00 ) << 8) | (((x) & 0x00ff0000 ) >> 8) | (((x) & 0xff000000 ) >> 24))

#ifdef BUILD_LOADER_PNG
#include <png.h>
#include <setjmp.h>
#define PNG_BYTES_TO_CHECK 4
#endif

#ifdef BUILD_LOADER_JPEG
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#endif

#ifdef BUILD_LOADER_EET
#include <Eet.h>
#endif

#ifdef BUILD_LOADER_EDB
#include <Edb.h>
#include <zlib.h>
#endif

#ifdef BUILD_LOADER_TEMPLATE
#include <Template.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

#ifdef BUILD_LOADER_PNG
static int load_image_file_head_png(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_head_png(RGBA_Image *im, const char *file, const char *key)
{
   png_uint_32 w32, h32;
   FILE *f;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   int bit_depth, color_type, interlace_type;
   unsigned char buf[PNG_BYTES_TO_CHECK];
   char hasa, hasg;

   if ((!file)) return -1;
   hasa = 0;
   hasg = 0;
   f = fopen(file, "rb");
   if (!f) return -1;
	
   /* if we havent read the header before, set the header data */
   fread(buf, 1, PNG_BYTES_TO_CHECK, f);
   if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
     {
	fclose(f);
	return -1;
     }
   rewind(f);
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	fclose(f);
	return -1;
     }
   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	fclose(f);
	return -1;
     }
   if (setjmp(png_ptr->jmpbuf))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(f);
	return -1;
     }
   png_init_io(png_ptr, f);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   if (!im->image)
     im->image = evas_common_image_surface_new();
   if (!im->image)
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	fclose(f);
	return -1;
     }
   im->image->w = (int) w32;
   im->image->h = (int) h32;
   if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);
   if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) hasa = 1;
   if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
     {
	hasa = 1;
	hasg = 1;
     }
   if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY) hasg = 1;
   if (hasa) im->flags |= RGBA_IMAGE_HAS_ALPHA;
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
   fclose(f);
   return 1;
   key = 0;
}

static int load_image_file_data_png(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_data_png(RGBA_Image *im, const char *file, const char *key)
{
   png_uint_32 w32, h32;
   int w, h;
   FILE *f;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   int bit_depth, color_type, interlace_type;
   unsigned char buf[PNG_BYTES_TO_CHECK];
   unsigned char **lines;
   char hasa, hasg;
   int i;

   if ((!file)) return -1;
   hasa = 0;
   hasg = 0;
   f = fopen(file, "rb");
   if (!f) return -1;
	
   /* if we havent read the header before, set the header data */
   fread(buf, 1, PNG_BYTES_TO_CHECK, f);
   if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
     {
	fclose(f);
	return -1;
     }
   rewind(f);
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	fclose(f);
	return -1;
     }
   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	fclose(f);
	return -1;
     }
   if (setjmp(png_ptr->jmpbuf))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(f);
	return -1;
     }
   png_init_io(png_ptr, f);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   im->image->w = (int) w32;
   im->image->h = (int) h32;
   if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);
   if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) hasa = 1;
   if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
     {
	hasa = 1;
	hasg = 1;
     }
   if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY) hasg = 1;
   if (hasa) im->flags |= RGBA_IMAGE_HAS_ALPHA;

   w = im->image->w;
   h = im->image->h;
   if (hasa) png_set_expand(png_ptr);
   /* we want ARGB */
#ifdef WORDS_BIGENDIAN
   png_set_swap_alpha(png_ptr);
   png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
#else
   png_set_bgr(png_ptr);
   png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
#endif
   /* 16bit color -> 8bit color */
   png_set_strip_16(png_ptr);
   /* pack all pixels to byte boundaires */
   png_set_packing(png_ptr);
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	evas_common_image_surface_free(im->image);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	fclose(f);
	return -1;
     }
   lines = (unsigned char **) malloc(h * sizeof(unsigned char *));
   
   if (!lines)
     {
	evas_common_image_surface_free(im->image);   
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	fclose(f);
	return -1;
     }
   if (hasg)
     {
	png_set_gray_to_rgb(png_ptr);
	if (png_get_bit_depth(png_ptr, info_ptr) < 8)
	  png_set_gray_1_2_4_to_8(png_ptr);
     }
   for (i = 0; i < h; i++)
     lines[i] = ((unsigned char *)(im->image->data)) + (i * w * sizeof(DATA32));
   png_read_image(png_ptr, lines);
   free(lines);
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
   fclose(f);
   return 1;
   key = 0;
}
#endif

#ifdef BUILD_LOADER_JPEG
struct _JPEG_error_mgr
{
   struct     jpeg_error_mgr pub;
   jmp_buf    setjmp_buffer;
};
typedef struct _JPEG_error_mgr *emptr;

static void _JPEGFatalErrorHandler(j_common_ptr cinfo);
static void
_JPEGFatalErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;
   
   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   longjmp(errmgr->setjmp_buffer, 1);
   return;
}
      
static void _JPEGErrorHandler(j_common_ptr cinfo);
static void
_JPEGErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;
   
   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
}
      
static void _JPEGErrorHandler2(j_common_ptr cinfo, int msg_level);         
static void
_JPEGErrorHandler2(j_common_ptr cinfo, int msg_level)
{
   emptr errmgr;
   
   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
   msg_level = 0;
}

static int load_image_file_head_jpeg_internal(RGBA_Image *im, FILE *f);
static int
load_image_file_head_jpeg_internal(RGBA_Image *im, FILE *f)
{
   int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   
   if (!f) return -1;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);
   
/* head decoding */
   if (!im->image)
     im->image = evas_common_image_surface_new();
   if (!im->image)
     {
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   im->image->w = w = cinfo.output_width;
   im->image->h = h = cinfo.output_height;
/* end head decoding */
   
   jpeg_destroy_decompress(&cinfo);
   return 1;
}

static int load_image_file_data_jpeg_internal(RGBA_Image *im, FILE *f);
static int
load_image_file_data_jpeg_internal(RGBA_Image *im, FILE *f)
{
   int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2;
   int x, y, l, i, scans, count, prevy;
   
   if (!f) return -1;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);
   
/* head decoding */
   im->image->w = w = cinfo.output_width;
   im->image->h = h = cinfo.output_height;
/* end head decoding */   
/* data decoding */   
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   data = malloc(w * 16 * 3);
   if (!data)
     {
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	free(data);
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   ptr2 = im->image->data;
   count = 0;
   prevy = 0;
   if (cinfo.output_components == 3)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w * 3);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < w; x++)
		    {
		       *ptr2 =
			 (0xff000000) | ((ptr[0]) << 16) | ((ptr[1]) << 8) | (ptr[2]);
		       ptr += 3;
		       ptr2++;
		    }
	       }
	  }
     }
   else if (cinfo.output_components == 1)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < w; x++)
		    {
		       *ptr2 =
			 (0xff000000) | ((ptr[0]) << 16) | ((ptr[0]) << 8) | (ptr[0]);
		       ptr++;
		       ptr2++;
		    }
	       }
	  }
     }
   free(data);
/* end data decoding */   
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   return 1;
}

#if 0 /* not used at the moment */
static int load_image_file_data_jpeg_alpha_internal(RGBA_Image *im, FILE *f);
static int
load_image_file_data_jpeg_alpha_internal(RGBA_Image *im, FILE *f)
{
   int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2;
   int x, y, l, i, scans, count, prevy;
   
   if (!f) return -1;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);
   
/* head decoding */
   im->image->w = w = cinfo.output_width;
   im->image->h = h = cinfo.output_height;
/* end head decoding */   
/* data decoding */   
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   data = malloc(w * 16 * 3);
   if (!data)
     {
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   if (!im->image->data)
     {
	free(data);
	jpeg_destroy_decompress(&cinfo);
	return -1;
     }
   ptr2 = im->image->data;
   count = 0;
   prevy = 0;
   if (cinfo.output_components == 3)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w * 3);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < w; x++)
		    {
		       *ptr2 = 
			 ((*ptr2) & 0x00ffffff) | 
			 (((ptr[0] + ptr[1] + ptr[2]) / 3) << 24);
		       ptr += 3;
		       ptr2++;
		    }
	       }
	  }
     }
   else if (cinfo.output_components == 1)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < w; x++)
		    {
		       *ptr2 = 
			 ((*ptr2) & 0x00ffffff) |
			 ((ptr[0]) << 24);
		       ptr++;
		       ptr2++;
		    }
	       }
	  }
     }
   free(data);
/* end data decoding */   
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   return 1;
}
#endif 

static int load_image_file_head_jpeg(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_head_jpeg(RGBA_Image *im, const char *file, const char *key)
{
   int val;
   FILE *f;
   
   if ((!file)) return -1;
   f = fopen(file, "rb");
   if (!f) return -1;
   val = load_image_file_head_jpeg_internal(im, f);
   fclose(f);
   return val;
   key = 0;
}

static int load_image_file_data_jpeg(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_data_jpeg(RGBA_Image *im, const char *file, const char *key)
{
   int val;
   FILE *f;
   
   if ((!file)) return -1;
   f = fopen(file, "rb");
   if (!f) return -1;
   val = load_image_file_data_jpeg_internal(im, f);
   fclose(f);
   return val;
   key = 0;
}
#endif

#ifdef BUILD_LOADER_EET
static int load_image_file_head_eet(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_head_eet(RGBA_Image *im, const char *file, const char *key)
{
   int                  w, h, alpha, compression, quality, lossy;
   Eet_File            *ef;
   DATA32              *body;
   
   if ((!file) || (!key)) return -1;
   ef = eet_open((char *)file, EET_FILE_MODE_READ);
   if (!ef) return -1;
   body = eet_data_image_read(ef, (char *)key, 
			      &w, &h, &alpha, &compression, &quality, &lossy);
   if (!body)
     {
	eet_close(ef);
	return -1;
     }
   if ((w > 8192) || (h > 8192))
     {
	free(body);
	eet_close(ef);
	return -1;
     }
   if (alpha) im->flags |= RGBA_IMAGE_HAS_ALPHA;
   if (!im->image)
     im->image = evas_common_image_surface_new();
   if (!im->image)
     {
	free(body);
	eet_close(ef);
	return -1;
     }
   im->image->w = w;
   im->image->h = h;
   im->image->data = body;
   im->image->no_free = 0;
   eet_close(ef);
   return 1;
}

static int load_image_file_data_eet(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_data_eet(RGBA_Image *im, const char *file, const char *key)
{
   int                  w, h, alpha, compression, quality, lossy;
   Eet_File            *ef;
   int                  ok;
   
   if ((!file) || (!key)) return -1;
   ef = eet_open((char *)file, EET_FILE_MODE_READ);
   if (!ef) return -1;
   ok = eet_data_image_header_read(ef, (char *)key, 
				   &w, &h, &alpha, &compression, &quality, &lossy);
   if (!ok)
     {
	eet_close(ef);
	return -1;
     }
   if ((w > 8192) || (h > 8192))
     {
	eet_close(ef);
	return -1;
     }
   if (alpha) im->flags |= RGBA_IMAGE_HAS_ALPHA;
   if (!im->image)
     im->image = evas_common_image_surface_new();
   if (!im->image)
     {
	eet_close(ef);
	return -1;
     }
   im->image->w = w;
   im->image->h = h;
   eet_close(ef);
   return 1;
}
#endif

#ifdef BUILD_LOADER_EDB
static int load_image_file_head_edb(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_head_edb(RGBA_Image *im, const char *file, const char *key)
{
   int                  w, h, alpha, compression, size;
   E_DB_File           *db;
   DATA32              *ret;
   DATA32               header[8];
   
   if ((!file) || (!key)) return -1;
   db = e_db_open_read((char *)file);
   if (!db) return -1;
   ret = e_db_data_get(db, (char *)key, &size);
   if (!ret)
     {
	e_db_close(db);
	return -1;
     }
   if (size < 32)
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   memcpy(header, ret, 32);
#ifdef WORDS_BIGENDIAN
     {
	int i;
	
	for (i = 0; i < 8; i++) SWAP32(header[i]);
     }
#endif
   if (header[0] != 0xac1dfeed)
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   w = header[1];
   h = header[2];
   alpha = header[3];
   compression = header[4];
   if ((w > 8192) || (h > 8192))
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   if ((compression == 0) && (size < ((w * h * 4) + 32)))
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   if (alpha) im->flags |= RGBA_IMAGE_HAS_ALPHA;
   if (!im->image)
     im->image = evas_common_image_surface_new();
   if (!im->image)
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   im->image->w = w;
   im->image->h = h;
   free(ret);
   e_db_close(db);
   return 1;
}

static int load_image_file_data_edb(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_data_edb(RGBA_Image *im, const char *file, const char *key)
{
   int                  w, h, alpha, compression, size;
   E_DB_File           *db;
   DATA32              *ret;
   DATA32              *body;
   DATA32               header[8];

   if ((!file) || (!key)) return -1;
   db = e_db_open_read((char *)file);
   if (!db) return -1;
   ret = e_db_data_get(db, (char *)key, &size);
   if (!ret)
     {
	e_db_close(db);
	return -1;
     }
   if (size < 32)
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   memcpy(header, ret, 32);
#ifdef WORDS_BIGENDIAN
     {
	int i;
	
	for (i = 0; i < 8; i++) SWAP32(header[i]);
     }
#endif
   if (header[0] != 0xac1dfeed)
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   w = header[1];
   h = header[2];
   alpha = header[3];
   compression = header[4];
   if ((w > 8192) || (h > 8192))
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   if ((compression == 0) && (size < ((w * h * 4) + 32)))
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   if (alpha) im->flags |= RGBA_IMAGE_HAS_ALPHA;
   if (!im->image)
     im->image = evas_common_image_surface_new();
   if (!im->image)
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   im->image->w = w;
   im->image->h = h;
   body = &(ret[8]);
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	free(ret);
	e_db_close(db);
	return -1;
     }
   if (!compression)
     {
#ifdef WORDS_BIGENDIAN
	  {
	     int x;
	     
	     memcpy(im->image->data, body, w * h * sizeof(DATA32));
	     for (x = 0; x < (w * h); x++) SWAP32(im->image->data[x]);
	  }
#else
	memcpy(im->image->data, body, w * h * sizeof(DATA32));
#endif       
     }
   else
     {
	uLongf dlen;
	
	dlen = w * h * sizeof(DATA32);
	uncompress((Bytef *)im->image->data, &dlen, (Bytef *)body, 
		   (uLongf)(size - 32));
#ifdef WORDS_BIGENDIAN
	  {
	     int x;
	     
	     for (x = 0; x < (w * h); x++) SWAP32(im->image->data[x]);
	  }
#endif
     }   
   free(ret);
   e_db_close(db);
   return 1;
}
#endif

#ifdef BUILD_LOADER_TEMPLATE
static int load_image_file_head_template(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_head_template(RGBA_Image *im, const char *file, const char *key)
{
   return -1;
}

static int load_image_file_data_template(RGBA_Image *im, const char *file, const char *key);
static int
load_image_file_data_template(RGBA_Image *im, const char *file, const char *key)
{
   return -1;
}
#endif

RGBA_Image *
evas_common_load_image_from_file(const char *file, const char *key)
{
   char *real_file;
   RGBA_Image *im;
   int ok;
   DATA64 mod_time;

   real_file = evas_file_path_resolve(file);
   mod_time = 0;
   if (real_file) mod_time = evas_file_modified_time(real_file);
   else if (file) mod_time = evas_file_modified_time(file);
   im = evas_common_image_find(file, key, mod_time);
   if (im)
     {
	evas_common_image_ref(im);
	if (real_file) free(real_file);
	return im;
     }
   im = evas_common_image_new();
   if (!im)
     {
	if (real_file) free(real_file);
	return NULL;
     }
   ok = -1;
#ifdef BUILD_LOADER_PNG
   if (ok == -1)
     {
	ok = load_image_file_head_png(im, file, key);
	if (ok != -1) im->info.format = 1;
     }
#endif
#ifdef BUILD_LOADER_JPEG
   if (ok == -1)
     {
	ok = load_image_file_head_jpeg(im, file, key);
	if (ok != -1) im->info.format = 2;
     }
#endif
#ifdef BUILD_LOADER_EET
   if (ok == -1)
     {
	ok = load_image_file_head_eet(im, file, key);
	if (ok != -1) im->info.format = 3;
     }
#endif
#ifdef BUILD_LOADER_EDB
   if (ok == -1)
     {
	ok = load_image_file_head_edb(im, file, key);
	if (ok != -1) im->info.format = 4;
     }
#endif
   if (ok == -1)
     {
	evas_common_image_free(im);
	if (real_file) free(real_file);
	return NULL;
     }
   im->timestamp = mod_time;
   if (file)
     {
	im->info.file = strdup(file);
	im->info.real_file = real_file;
     }
   else
     {
	if (real_file) free(real_file);	
     }
   if (key)
     im->info.key = strdup(key);
   evas_common_image_ref(im);
   return im;
}

void
evas_common_load_image_data_from_file(RGBA_Image *im)
{
   int ok;
   
   if (im->image->data) return;
   ok = -1;
#ifdef BUILD_LOADER_PNG
   if (im->info.format == 1)
     ok = load_image_file_data_png(im, im->info.file, im->info.key);
#endif
#ifdef BUILD_LOADER_JPEG
   if (im->info.format == 2)
     ok = load_image_file_data_jpeg(im, im->info.file, im->info.key);
#endif
#ifdef BUILD_LOADER_EET
   if (im->info.format == 3)
     ok = load_image_file_data_eet(im, im->info.file, im->info.key);
#endif
#ifdef BUILD_LOADER_EDB
   if (im->info.format == 4)
     ok = load_image_file_data_edb(im, im->info.file, im->info.key);
#endif
   if (ok == -1)
     {
	evas_common_image_surface_alloc(im->image);
	if (!im->image->data)
	  {
	     const DATA32 pixel = 0xffffffff;
	     
	     im->image->w = 1;
	     im->image->h = 1;
	     im->image->data = (DATA32 *)&pixel;
	     im->image->no_free = 1;
	  }
     }
}
