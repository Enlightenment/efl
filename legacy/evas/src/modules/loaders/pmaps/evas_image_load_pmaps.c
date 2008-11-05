
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

#define FILE_BUFFER_SIZE 1024
#define FILE_BUFFER_UNREAD_SIZE 16

static int evas_image_load_file_head_pmaps(Image_Entry *ie,
				    const char *file, const char *key);
static int evas_image_load_file_data_pmaps(Image_Entry *ie,
				    const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_pmaps_func = {
   evas_image_load_file_head_pmaps,
   evas_image_load_file_data_pmaps
};

/* The buffer to load pmaps images */
typedef struct Pmaps_Buffer Pmaps_Buffer;

struct Pmaps_Buffer
{
   FILE *file;

   /* the buffer */
   DATA8 buffer[FILE_BUFFER_SIZE];
   DATA8 unread[FILE_BUFFER_UNREAD_SIZE];
   DATA8 *current;
   DATA8 *end;
   char type[3];
   unsigned char unread_len:7;
   unsigned char last_buffer:1;

   /* image properties */
   int w;
   int h;
   int max;

   /* interface */
   int (*int_get) (Pmaps_Buffer *b, int *val);
   int (*color_get) (Pmaps_Buffer *b, DATA32 *color);
};

/* internal used functions */
static int pmaps_buffer_open(Pmaps_Buffer *b, const char *filename);
static void pmaps_buffer_close(Pmaps_Buffer *b);
static int pmaps_buffer_header_parse(Pmaps_Buffer *b);
static int pmaps_buffer_plain_int_get(Pmaps_Buffer *b, int *val);
static int pmaps_buffer_1byte_int_get(Pmaps_Buffer *b, int *val);
static int pmaps_buffer_2byte_int_get(Pmaps_Buffer *b, int *val);
static int pmaps_buffer_gray_get(Pmaps_Buffer *b, DATA32 *color);
static int pmaps_buffer_rgb_get(Pmaps_Buffer *b, DATA32 *color);
static int pmaps_buffer_plain_bw_get(Pmaps_Buffer *b, DATA32 *color);

static size_t pmaps_buffer_plain_update(Pmaps_Buffer *b);
static size_t pmaps_buffer_raw_update(Pmaps_Buffer *b);
static int pmaps_buffer_comment_skip(Pmaps_Buffer *b);

static int
evas_image_load_file_head_pmaps(Image_Entry *ie, const char *file,
				const char *key)
{
   Pmaps_Buffer b;

   if ((!file))
      return 0;

   if (!pmaps_buffer_open(&b, file))
     {
	pmaps_buffer_close(&b);
	return 0;
     }

   if (!pmaps_buffer_header_parse(&b))
     {
	pmaps_buffer_close(&b);
	return 0;
     }

   ie->w = b.w;
   ie->h = b.h;

   pmaps_buffer_close(&b);
   return 1;
   /* we don't have a use for key, skip warnings */
   key = NULL;
}

static int
evas_image_load_file_data_pmaps(Image_Entry *ie, const char *file,
				const char *key)
{
   Pmaps_Buffer b;
   int pixels;
   DATA32 *ptr;

   if ((!file))
      return 0;

   if (!pmaps_buffer_open(&b, file))
     {
	pmaps_buffer_close(&b);
	return 0;
     }

   if (!pmaps_buffer_header_parse(&b))
     {
	pmaps_buffer_close(&b);
	return 0;
     }

   pixels = b.w * b.h;

   evas_cache_image_surface_alloc(ie, b.w, b.h);
   if (!evas_cache_image_pixels(ie))
     {
	pmaps_buffer_close(&b);
	return 0;
     }

   ptr = evas_cache_image_pixels(ie);

   if (b.type[1] != '4')
     {
	while (pixels > 0 && b.color_get(&b, ptr))
	  {
	     pixels--;
	     ptr++;
	  }
     }
   else
     {
	while (pixels > 0
	       && (b.current != b.end || pmaps_buffer_raw_update(&b)))
	  {
	     int i;

	     for (i = 7; i >= 0 && pixels > 0; i--)
	       {
		  if (*b.current & (1 << i))
		     *ptr = 0xff000000;
		  else
		     *ptr = 0xffffffff;
		  ptr++;
		  pixels--;
	       }
	     b.current++;
	  }
     }

   /* if there are some pix missing, give them a proper default */
   memset(ptr, 0xff, 4 * pixels);
   pmaps_buffer_close(&b);

   return 1;
   /* we don't have a use for key, skip warnings */
   key = NULL;
}

/* internal used functions */
static int
pmaps_buffer_open(Pmaps_Buffer *b, const char *filename)
{
   size_t len;

   b->file = fopen(filename, "rb");
   if (!b->file)
      return 0;

   *b->buffer = 0;
   *b->unread = 0;
   b->last_buffer = 0;
   b->unread_len = 0;

   len = pmaps_buffer_plain_update(b);

   if (len < 3)
      return 0;

   /* copy the type */
   b->type[0] = b->buffer[0];
   b->type[1] = b->buffer[1];
   b->type[2] = 0;
   /* skip the PX */
   b->current = b->buffer + 2;

   return 1;
}

static void
pmaps_buffer_close(Pmaps_Buffer *b)
{
   if (b->file)
      fclose(b->file);
}

static int
pmaps_buffer_header_parse(Pmaps_Buffer *b)
{
   /* if there is no P at the beginning it is not a file we can parse */
   if (b->type[0] != 'P')
      return 0;

   /* get the width */
   if (!pmaps_buffer_plain_int_get(b, &(b->w)) || b->w < 1)
      return 0;

   /* get the height */
   if (!pmaps_buffer_plain_int_get(b, &(b->h)) || b->h < 1)
      return 0;

   /* get the maximum value. P1 and P4 don't have a maximum value. */
   if (!(b->type[1] == '1' || b->type[1] == '4')
       && (!pmaps_buffer_plain_int_get(b, &(b->max)) || b->max < 1))
      return 0;

   /* set up the color get callback */
   switch (b->type[1])
     {
	/* Black and White */
     case '1':
	b->color_get = pmaps_buffer_plain_bw_get;
	break;
     case '4':
	/* Binary black and white use another format */
	b->color_get = NULL;
	break;
     case '2':
     case '5':
	b->color_get = pmaps_buffer_gray_get;
	break;
     case '3':
     case '6':
	b->color_get = pmaps_buffer_rgb_get;
	break;
     case '7':
	/* XXX write me */
	return 0;
	break;
     default:
	return 0;
     }
   /* set up the int get callback */
   switch (b->type[1])
     {
	/* RAW */
     case '5':
     case '6':
	if (b->max < 256)
	   b->int_get = pmaps_buffer_1byte_int_get;
	else
	   b->int_get = pmaps_buffer_2byte_int_get;

	if (b->current == b->end && !pmaps_buffer_raw_update(b))
	   return 0;

	b->current++;
	break;
	/* Plain */
     case '2':
     case '3':
	b->int_get = pmaps_buffer_plain_int_get;
	break;
	/* Black and White Bitmaps don't use that callback */
     case '1':
     case '4':
	b->int_get = NULL;
	/* we need to skip the next character fpr P4 it
	 * doesn't hurt if we do it for the P1 as well */
	b->current++;
	break;
     }
   return 1;
}

static size_t
pmaps_buffer_plain_update(Pmaps_Buffer *b)
{
   size_t r;
   size_t steps = 0;

   /* if we already are in the last buffer we can not update it */
   if (b->last_buffer)
      return 0;

   /* if we have unread bytes we need to put them before the new read
    * stuff */
   if (b->unread_len)
      memcpy(b->buffer, b->unread, b->unread_len);

   r = fread(&b->buffer[b->unread_len], 1,
	     FILE_BUFFER_SIZE - b->unread_len - 1, b->file) + b->unread_len;

   /* we haven't read anything nor have we bytes in the unread buffer */
   if (r == 0)
     {
	b->buffer[0] = '\0';
	b->end = b->buffer;
	b->last_buffer = 1;
	return 0;
     }

   if (r < FILE_BUFFER_SIZE - 1)
     {
	/*we reached eof */ ;
	b->last_buffer = 1;
     }

   b->buffer[r] = 0;
   r--;

   while (steps < (FILE_BUFFER_UNREAD_SIZE - 2)
	  && r > 1 && !isspace(b->buffer[r]))
     {
	steps++;
	r--;
     }

   if (steps != 0)
     {
	memcpy(b->unread, &b->buffer[r], steps + 1);
	b->unread_len = steps + 1;
     }
   else
     {
	b->unread[0] = '\0';
	b->unread_len = 0;
     }

   b->buffer[r] = '\0';
   b->current = b->buffer;
   b->end = b->buffer + r;

   return r;
}

static size_t
pmaps_buffer_raw_update(Pmaps_Buffer *b)
{
   size_t r;

   if (b->last_buffer)
      return 0;

   if (b->unread_len)
      memcpy(b->buffer, b->unread, b->unread_len);

   r = fread(&b->buffer[b->unread_len], 1,
	     FILE_BUFFER_SIZE - b->unread_len - 1, b->file) + b->unread_len;

   if (r < FILE_BUFFER_SIZE - 1)
     {
	/*we reached eof */ ;
	b->last_buffer = 1;
     }

   b->buffer[r] = 0;
   b->end = b->buffer + r;
   b->current = b->buffer;

   if (b->unread_len)
     {
	/* the buffer is now read */
	*b->unread = '\0';
	b->unread_len = 0;
     }

   return r;
}

static int
pmaps_buffer_plain_int_get(Pmaps_Buffer *b, int *val)
{
   char *start;
   DATA8 lastc;

   /* first skip all white space
    * Note: we are skipping here actually every character than is not 
    * a digit */
   while (!isdigit(*b->current))
     {
	if (*b->current == '\0')
	  {
	     if (!pmaps_buffer_plain_update(b))
		return 0;

	     continue;
	  }
	if (*b->current == '#' && !pmaps_buffer_comment_skip(b))
	   return 0;
	b->current++;
     }

   start = (char *)b->current;
   /* now find the end of the number */
   while (isdigit(*b->current))
      b->current++;

   lastc = *b->current;
   *b->current = '\0';
   *val = atoi(start);
   *b->current = lastc;

   return 1;
}

static int
pmaps_buffer_1byte_int_get(Pmaps_Buffer *b, int *val)
{
   /* are we at the end of the buffer? */
   if (b->current == b->end && !pmaps_buffer_raw_update(b))
      return 0;

   *val = *b->current;
   b->current++;

   return 1;
}
static int
pmaps_buffer_2byte_int_get(Pmaps_Buffer *b, int *val)
{
   /* are we at the end of the buffer? */
   if (b->current == b->end && !pmaps_buffer_raw_update(b))
      return 0;

   *val = (int)(*b->current << 8);
   b->current++;

   /* are we at the end of the buffer? */
   if (b->current == b->end && !pmaps_buffer_raw_update(b))
      return 0;

   *val |= *b->current;
   b->current++;

   return 1;
}

static int
pmaps_buffer_comment_skip(Pmaps_Buffer *b)
{
   while (*b->current != '\n')
     {
	if (*b->current == '\0')
	  {
	     if (!pmaps_buffer_plain_update(b))
		return 0;

	     continue;
	  }
	b->current++;
     }
   return 1;
}

static int
pmaps_buffer_rgb_get(Pmaps_Buffer *b, DATA32 *color)
{
   int vr, vg, vb;

   if (!b->int_get(b, &vr) || !b->int_get(b, &vg) || !b->int_get(b, &vb))
      return 0;

   if (b->max != 255)
     {
	vr = (vr * 255) / b->max;
	vg = (vg * 255) / b->max;
	vb = (vb * 255) / b->max;
     }
   if (vr > 255)
      vr = 255;
   if (vg > 255)
      vg = 255;
   if (vb > 255)
      vb = 255;

   *color = 0xff000000 | (vr << 16) | (vg << 8) | vb;

   return 1;
}

static int
pmaps_buffer_gray_get(Pmaps_Buffer *b, DATA32 *color)
{
   int val;

   if (!b->int_get(b, &val))
      return 0;

   if (b->max != 255)
      val = (val * 255) / b->max;
   if (val > 255)
      val = 255;
   *color = 0xff000000 | (val << 16) | (val << 8) | val;

   return 1;
}

static int
pmaps_buffer_plain_bw_get(Pmaps_Buffer *b, DATA32 *val)
{
   /* first skip all white space
    * Note: we are skipping here actually every character than is not 
    * a digit */
   while (!isdigit(*b->current))
     {
	if (*b->current == '\0')
	  {
	     if (!pmaps_buffer_raw_update(b))
		return 0;

	     continue;
	  }
	if (*b->current == '#' && !pmaps_buffer_comment_skip(b))
	   return 0;
	b->current++;
     }

   if (*b->current == '0')
      *val = 0xffffffff;
   else
      *val = 0xff000000;

   b->current++;

   return 1;
}

/* external functions */
EAPI int
module_open(Evas_Module *em)
{
   if (!em)
      return 0;
   em->functions = (void *)(&evas_image_load_pmaps_func);
   return 1;
}

EAPI void
module_close(void)
{

}

EAPI Evas_Module_Api evas_modapi = {
   EVAS_MODULE_API_VERSION,
   EVAS_MODULE_TYPE_IMAGE_LOADER,
   "pmaps",
   "none"
};

