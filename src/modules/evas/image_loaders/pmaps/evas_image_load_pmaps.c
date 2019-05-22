#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

#define FILE_BUFFER_SIZE 1024 * 32
#define FILE_BUFFER_UNREAD_SIZE 16

/* The buffer to load pmaps images */
typedef struct Pmaps_Buffer Pmaps_Buffer;

struct Pmaps_Buffer
{
   Eina_File *file;
   unsigned char *map;
   size_t position;

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
static Eina_Bool pmaps_buffer_open(Pmaps_Buffer *b, Eina_File *f, Eina_Bool header, int *error);
static void pmaps_buffer_close(Pmaps_Buffer *b);
static Eina_Bool pmaps_buffer_header_parse(Pmaps_Buffer *b, int *error);
static int pmaps_buffer_plain_int_get(Pmaps_Buffer *b, int *val);
static int pmaps_buffer_1byte_int_get(Pmaps_Buffer *b, int *val);
static int pmaps_buffer_2byte_int_get(Pmaps_Buffer *b, int *val);
static int pmaps_buffer_gray_get(Pmaps_Buffer *b, DATA32 *color);
static int pmaps_buffer_rgb_get(Pmaps_Buffer *b, DATA32 *color);
static int pmaps_buffer_plain_bw_get(Pmaps_Buffer *b, DATA32 *color);

static size_t pmaps_buffer_plain_update(Pmaps_Buffer *b);
static size_t pmaps_buffer_raw_update(Pmaps_Buffer *b);
static int pmaps_buffer_comment_skip(Pmaps_Buffer *b);

static void *
evas_image_load_file_open_pmaps(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
				Evas_Image_Load_Opts *opts EINA_UNUSED,
				Evas_Image_Animated *animated EINA_UNUSED,
				int *error EINA_UNUSED)
{
   return f;
}

static void
evas_image_load_file_close_pmaps(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_pmaps(void *loader_data,
                                Evas_Image_Property *prop,
                                int *error)
{
   Eina_File *f = loader_data;
   Pmaps_Buffer b;

   if (!pmaps_buffer_open(&b, f, EINA_TRUE, error))
     {
	pmaps_buffer_close(&b);
	return EINA_FALSE;
     }

   if (!pmaps_buffer_header_parse(&b, error))
     {
	pmaps_buffer_close(&b);
	return EINA_FALSE;
     }

   prop->w = b.w;
   prop->h = b.h;

   pmaps_buffer_close(&b);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_pmaps(void *loader_data,
				Evas_Image_Property *prop,
				void *pixels,
				int *error)
{
   Eina_File *f = loader_data;
   Pmaps_Buffer b;
   int size;
   DATA32 *ptr;
   unsigned short count = 0;
   Eina_Bool r = EINA_FALSE;

   if (!pmaps_buffer_open(&b, f, EINA_FALSE, error))
     goto on_error;

   if (!pmaps_buffer_header_parse(&b, error))
     goto on_error;

   size = b.w * b.h;
   if ((int) prop->w != b.w ||
       (int) prop->h != b.h)
     goto on_error;

   ptr = pixels;
   if (b.type[1] != '4')
     {
        while (size > 0 && b.color_get(&b, ptr))
          {
             size--;
             ptr++;

             EVAS_MODULE_TASK_CHECK(count, 0x3FF, error, on_error);
          }
     }
   else
     {
	while (size > 0
	       && (b.current != b.end || pmaps_buffer_raw_update(&b)))
	  {
	     int i;

	     for (i = 7; i >= 0 && size > 0; i--)
	       {
		  if (*b.current & (1 << i))
		     *ptr = 0xff000000;
		  else
		     *ptr = 0xffffffff;
		  ptr++;
		  size--;
	       }
             b.current++;

             EVAS_MODULE_TASK_CHECK(count, 0x3FF, error, on_error);
          }
     }

   /* if there are some pix missing, give them a proper default */
   memset(ptr, 0xff, 4 * size);
   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 on_error:
   pmaps_buffer_close(&b);
   return r;
}

/* internal used functions */
static Eina_Bool
pmaps_buffer_open(Pmaps_Buffer *b, Eina_File *f, Eina_Bool header, int *error)
{
   size_t len;

   b->file = f;
   b->map = eina_file_map_all(b->file, header ? EINA_FILE_RANDOM : EINA_FILE_SEQUENTIAL);
   if (!b->map)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   b->position = 0;
   *b->buffer = 0;
   *b->unread = 0;
   b->last_buffer = 0;
   b->unread_len = 0;

   len = pmaps_buffer_plain_update(b);

   if (len < 3)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        eina_file_map_free(b->file, b->map);
        b->map = NULL;
	return EINA_FALSE;
     }

   /* copy the type */
   b->type[0] = b->buffer[0];
   b->type[1] = b->buffer[1];
   b->type[2] = 0;
   /* skip the PX */
   b->current = b->buffer + 2;

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static void
pmaps_buffer_close(Pmaps_Buffer *b)
{
   if (b->file)
     {
        if (b->map) eina_file_map_free(b->file, b->map);
        b->map = NULL;
        b->file = NULL;
     }
}

static Eina_Bool
pmaps_buffer_header_parse(Pmaps_Buffer *b, int *error)
{
   /* if there is no P at the beginning it is not a file we can parse */
   if (b->type[0] != 'P')
     {
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }

   /* get the width */
   if (!pmaps_buffer_plain_int_get(b, &(b->w)) || b->w < 1)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }

   /* get the height */
   if (!pmaps_buffer_plain_int_get(b, &(b->h)) || b->h < 1)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }

   /* get the maximum value. P1 and P4 don't have a maximum value. */
   if (!(b->type[1] == '1' || b->type[1] == '4')
       && (!pmaps_buffer_plain_int_get(b, &(b->max)) || b->max < 1))
     {
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }

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
   size_t max;

   /* if we already are in the last buffer we can not update it */
   if (b->last_buffer)
      return 0;

   /* if we have unread bytes we need to put them before the new read
    * stuff */
   if (b->unread_len)
      memcpy(b->buffer, b->unread, b->unread_len);

   max = FILE_BUFFER_SIZE - b->unread_len - 1;
   if (b->position + max > eina_file_size_get(b->file))
     max = eina_file_size_get(b->file) - b->position;

   memcpy(&b->buffer[b->unread_len], b->map + b->position, max);
   b->position += max;
   r = max + b->unread_len;

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

   b->unread[0] = '\0';
   b->unread_len = 0;

   b->current = b->buffer;
   b->end = b->buffer + r;

   return r;
}

static size_t
pmaps_buffer_raw_update(Pmaps_Buffer *b)
{
   size_t r;
   size_t max;

   if (b->last_buffer)
      return 0;

   if (b->unread_len)
      memcpy(b->buffer, b->unread, b->unread_len);

   max = FILE_BUFFER_SIZE - b->unread_len;
   if (b->position + max > eina_file_size_get(b->file))
     max = eina_file_size_get(b->file) - b->position;

   memcpy(&b->buffer[b->unread_len], b->map + b->position, max);
   b->position += max;
   r = max + b->unread_len;

   if (r < FILE_BUFFER_SIZE)
     {
	/*we reached eof */
	b->last_buffer = 1;
     }

   b->end = b->buffer + r;
   b->current = b->buffer;

   if (b->unread_len)
     {
	/* the buffer is now read */
	*b->unread = 0;
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

   *color = ARGB_JOIN(0xff, vr, vg, vb);

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
   *color = ARGB_JOIN(0xff, val, val, val);

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
Evas_Image_Load_Func evas_image_load_pmaps_func = {
   evas_image_load_file_open_pmaps,
   evas_image_load_file_close_pmaps,
   evas_image_load_file_head_pmaps,
   evas_image_load_file_data_pmaps,
   NULL,
   EINA_TRUE,
   EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em)
      return 0;
   em->functions = (void *)(&evas_image_load_pmaps_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi = {
   EVAS_MODULE_API_VERSION,
   "pmaps",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, pmaps);

#ifndef EVAS_STATIC_BUILD_PMAPS
EVAS_EINA_MODULE_DEFINE(image_loader, pmaps);
#endif
