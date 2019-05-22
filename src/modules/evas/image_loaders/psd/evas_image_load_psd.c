#define _XOPEN_SOURCE 600

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

typedef struct _PSD_Header PSD_Header;

typedef enum _PSD_Mode
  {
    PSD_GREYSCALE = 1,
    PSD_INDEXED = 2,
    PSD_RGB = 3,
    PSD_CMYK = 4
  } PSD_Mode;

struct _PSD_Header
{
   unsigned char signature[4];
   unsigned short version;
   unsigned char reserved[9];
   unsigned short channels;
   unsigned int height;
   unsigned int width;
   unsigned short depth;

   unsigned short channel_num;

   PSD_Mode mode;
};

enum {
  READ_COMPRESSED_SUCCESS,
  READ_COMPRESSED_ERROR_FILE_CORRUPT,
  READ_COMPRESSED_ERROR_FILE_READ_ERROR
};

static Eina_Bool get_compressed_channels_length(PSD_Header *Head,
                                                const unsigned char *map, size_t length, size_t *position,
                                                unsigned short *rle_table,
                                                unsigned int *chanlen);

static int
read_ushort(const unsigned char *map, size_t length, size_t *position, unsigned short *ret)
{
   if (*position + 2 > length) return 0;
   // FIXME: need to check order
   *ret = (map[(*position) + 0] << 8) | map[(*position) + 1];
   *position += 2;
   return 1;
}

static int
read_uint(const unsigned char *map, size_t length, size_t *position, unsigned int *ret)
{
   if (*position + 4 > length) return 0;
   // FIXME: need to check order
   *ret = ARGB_JOIN(map[(*position) + 0], map[(*position) + 1], map[(*position) + 2], map[(*position) + 3]);
   *position += 4;
   return 1;
}

static int
read_block(const unsigned char *map, size_t length, size_t *position, void *target, size_t size)
{
   if (*position + size > length) return 0;
   memcpy(target, map + *position, size);
   *position += size;
   return 1;
}

// Internal function used to get the Psd header from the current file.
Eina_Bool
psd_get_header(PSD_Header *header, const unsigned char *map, size_t length, size_t *position)
{
   unsigned short tmp;

#define CHECK_RET(Call)                         \
   if (!Call) return EINA_FALSE;

   CHECK_RET(read_block(map, length, position, header->signature, 4));
   CHECK_RET(read_ushort(map, length, position, &header->version));
   CHECK_RET(read_block(map, length, position, header->reserved, 6));
   CHECK_RET(read_ushort(map, length, position, &header->channels));
   CHECK_RET(read_uint(map, length, position, &header->height));
   CHECK_RET(read_uint(map, length, position, &header->width));
   CHECK_RET(read_ushort(map, length, position, &header->depth));

   CHECK_RET(read_ushort(map, length, position, &tmp));
   header->mode = tmp;

#undef CHECK_RET

   return EINA_TRUE;
}


// Internal function used to check if the HEADER is a valid Psd header.
Eina_Bool
is_psd(PSD_Header *header)
{
   if (strncmp((char*)header->signature, "8BPS", 4))
     return EINA_FALSE;
   if (header->version != 1)
     return EINA_FALSE;
   if (header->channels < 1 || header->channels > 24)
     return EINA_FALSE;
   if (header->height < 1 || header->width < 1)
     return EINA_FALSE;
   if (header->depth != 1 && header->depth != 8 && header->depth != 16)
     return EINA_FALSE;

   return EINA_TRUE;
}

static void *
evas_image_load_file_open_psd(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			      Evas_Image_Load_Opts *opts EINA_UNUSED,
			      Evas_Image_Animated *animated EINA_UNUSED,
			      int *error EINA_UNUSED)
{
   return f;
}

static void
evas_image_load_file_close_psd(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_psd(void *loader_data,
			      Evas_Image_Property *prop,
			      int *error)
{
   Eina_File *f = loader_data;
   void *map;
   size_t length;
   size_t position;
   PSD_Header header;
   Eina_Bool correct;
   Eina_Bool r = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_NONE;

   map = eina_file_map_all(f, EINA_FILE_RANDOM);
   length = eina_file_size_get(f);
   position = 0;
   if (!map || length < 1)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	goto on_error;
     }

   correct = psd_get_header(&header, map, length, &position);
   if (!correct || !is_psd(&header))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	goto on_error;
     }

   prop->w = header.width;
   prop->h = header.height;
   if (header.channels != 3)
     prop->alpha = 1;

   r = EINA_TRUE;

 on_error:
   eina_file_map_free(f, map);
   return r;
}

static unsigned int
read_compressed_channel(const unsigned char *map, size_t length, size_t *position,
			const unsigned int channel_length EINA_UNUSED, 
                        unsigned int size,
			unsigned char* channel)
{
   // FIXME: what does channel_length means, and why is it not used
   unsigned int i;
   signed char headbyte;
   unsigned char c;

#define CHECK_RET(Call)                                         \
   if (!Call) return READ_COMPRESSED_ERROR_FILE_READ_ERROR;	\

   for (i = 0; i < size; )
     {
        CHECK_RET(read_block(map, length, position, &headbyte, 1));

        if (headbyte >= 0)
          {
             if (i + headbyte > size)
               return READ_COMPRESSED_ERROR_FILE_CORRUPT;
	     CHECK_RET(read_block(map, length, position, channel + i, headbyte + 1));

             i += headbyte + 1;
          }
        else if (headbyte >= -127)
          {
             int run;

	     CHECK_RET(read_block(map, length, position, &c, 1));

             run = c;
             /* if (run == -1) */
	     /*   return READ_COMPRESSED_ERROR_FILE_READ_ERROR; */

             if (i + (-headbyte + 1) > size)
               return READ_COMPRESSED_ERROR_FILE_CORRUPT;

             memset(channel + i, run, -headbyte + 1);
             i += -headbyte + 1;
          }
     }

#undef CHECK_RET

   return READ_COMPRESSED_SUCCESS;
}


Eina_Bool
psd_get_data(PSD_Header *head,
             const unsigned char *map, size_t length, size_t *position,
	     unsigned char *buffer, Eina_Bool compressed,
	     int *error)
{
   unsigned int c, x, y, numchan, bps, bpc, bpp;
   unsigned int pixels_count;
   unsigned char *channel = NULL;
   unsigned char *data = NULL;

   // Added 01-07-2009: This is needed to correctly load greyscale and
   //  paletted images.
   switch (head->mode)
     {
      case PSD_GREYSCALE:
      case PSD_INDEXED:
         numchan = 1;
         break;
      default:
         numchan = 3;
     }

   bpp = head->channels;
   bpc = head->depth / 8;
   pixels_count = head->width * head->height;

   data = malloc(sizeof (unsigned char) * pixels_count * bpp);
   if (!data) return EINA_FALSE;

   channel = malloc(sizeof (unsigned char) * pixels_count * bpc);
   if (!channel)
     {
        free(data);
        return EINA_FALSE;
     }

   bps = head->width * head->channels * bpc;
   // @TODO: Add support for this in, though I have yet to run across a .psd
   //	file that uses this.
   if (compressed && bpc == 2)
     {
        free(data);
	free(channel);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

#define CHECK_RET(Call)                         \
   if (!Call)                                   \
     {						\
        free(data);				\
        free(channel);				\
        return EINA_FALSE;			\
     }

   if (!compressed)
     {
        if (bpc == 1)
          {
             for (c = 0; c < numchan; c++)
               {
                  unsigned char *tmp = channel;

                  CHECK_RET(read_block(map, length, position, tmp, pixels_count));

                  for (y = 0; y < head->height * bps; y += bps)
                    {
                       for (x = 0; x < bps; x += bpp, tmp++)
                         {
                            data[y + x + c] = *tmp;
                         }
                    }
               }

             // Accumulate any remaining channels into a single alpha channel
             //@TODO: This needs to be changed for greyscale images.
             for (; c < head->channels; c++)
               {
                  unsigned char *tmp = channel;

                  CHECK_RET(read_block(map, length, position, channel, pixels_count));

                  for (y = 0; y < head->height * bps; y += bps)
                    {
                       for (x = 0; x < bps; x += bpp, tmp++)
                         {
                            unsigned short newval;

                            // previous formula was : (old / 255 * new / 255) * 255
                            newval = (*tmp) * data[y + x + 3];

                            data[y + x + 3] = newval >> 8;
                         }
                    }
               }
          }
        else
          {
             int bps2;

             bps2 = bps / 2;

             // iCurImage->Bpc == 2
             for (c = 0; c < numchan; c++)
               {
                  unsigned short *shortptr = (unsigned short*) channel;

                  CHECK_RET(read_block(map, length, position, channel, pixels_count * 2));

                  for (y = 0; y < head->height * bps2; y += bps2)
                    {
                       for (x = 0; x < (unsigned int)bps2; x += bpp, shortptr++)
                         {
                            ((unsigned short*)data)[y + x + c] = *shortptr;
                         }
                    }
               }

             // Accumulate any remaining channels into a single alpha channel
             //@TODO: This needs to be changed for greyscale images.
             for (; c < head->channels; c++)
               {
		  unsigned short *shortptr = (unsigned short*) channel;

		  CHECK_RET(read_block(map, length, position, channel, pixels_count * 2));

		  for (y = 0; y < head->height * bps2; y += bps2)
                    {
                       for (x = 0; x < (unsigned int)bps2; x += bpp, shortptr++)
                         {
                            unsigned int newval;

                            newval = *shortptr * ((unsigned short*)data)[y + x + 3];

                            ((unsigned short*)data)[y + x + 3] = newval >> 16;
                         }
                    }
               }
          }
     }
   else
     {
        unsigned short *rle_table;
	unsigned int *chanlen;

	rle_table = alloca(head->height * head->channel_num * sizeof (unsigned short));
	chanlen = alloca(head->channel_num * sizeof (unsigned int));
        if (!get_compressed_channels_length(head, map, length, position, rle_table, chanlen))
	  goto file_read_error;

        for (c = 0; c < numchan; c++)
          {
	     unsigned char *tmp = channel;
	     int err;

	     err = read_compressed_channel(map, length, position,
					   chanlen[c],
					   pixels_count,
					   channel);
             if (err == READ_COMPRESSED_ERROR_FILE_CORRUPT)
               goto file_corrupt;
             else if (err == READ_COMPRESSED_ERROR_FILE_READ_ERROR)
               goto file_read_error;

             for (y = 0; y < head->height * bps; y += bps)
               {
		  for (x = 0; x < bps; x += bpp, tmp++)
                    {
                       data[y + x + c] = *tmp;
                    }
               }
          }

        // Initialize the alpha channel to solid
        //@TODO: This needs to be changed for greyscale images.
        if (head->channels >= 4)
          {
	     for (y = 0; y < head->height * bps; y += bps)
               {
                  for (x = 0; x < bps; x += bpp)
		    {
                       data[y + x + 3] = 255;
		    }
               }

             for (; c < head->channels; c++)
               {
		  unsigned char *tmp = channel;
		  int err;

                  err = read_compressed_channel(map, length, position,
						chanlen[c],
						pixels_count,
						channel);
                  if (err == READ_COMPRESSED_ERROR_FILE_CORRUPT)
                    goto file_corrupt;
                  else if (err == READ_COMPRESSED_ERROR_FILE_READ_ERROR)
                    goto file_read_error;

                  for (y = 0; y < head->height * bps; y += bps)
                    {
		       for (x = 0; x < bps; x += bpp, tmp++)
                         {
			    unsigned short newval;

			    newval = *tmp * data[y + x + 3];

			    data[y + x + 3] = newval >> 8;
                         }
                    }
               }
          }
     }

   if (bpp == 3)
     {
        for (x = 0; x < pixels_count; x++)
          {
             buffer[x * 4 + 0] = data[x * 3 + 2];
             buffer[x * 4 + 1] = data[x * 3 + 1];
             buffer[x * 4 + 2] = data[x * 3 + 0];
             buffer[x * 4 + 3] = 255;
          }
     }
   else
     {
        // BRGA to RGBA
        for (x= 0; x < pixels_count; x++)
          {
             buffer[x * 4 + 0] = data[x * 4 + 2];
             buffer[x * 4 + 1] = data[x * 4 + 1];
             buffer[x * 4 + 2] = data[x * 4 + 0];
             buffer[x * 4 + 3] = data[x * 4 + 3];
          }
     }

   free(channel);
   free(data);
   return EINA_TRUE;

#undef CHECK_RET

 file_corrupt:
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

 file_read_error:
   free(channel);
   free(data);

   return EINA_FALSE;
}


Eina_Bool
get_single_channel(PSD_Header *head,
		   const unsigned char *map, size_t length, size_t *position,
		   unsigned char *buffer,
		   Eina_Bool compressed)
{
   unsigned int i, bpc;
   signed char headbyte;
   int c;
   int pixels_count;

   bpc = (head->depth / 8);
   pixels_count = head->width * head->height;

#define CHECK_RET(Call)                  \
   if (!Call) return EINA_FALSE;

   if (!compressed)
     {
        if (bpc == 1)
          {
             CHECK_RET(read_block(map, length, position, buffer, pixels_count));
          }
        else
          {  // Bpc == 2
             CHECK_RET(read_block(map, length, position, buffer, pixels_count * 2));
          }
     }
   else
     {
        for (i = 0; i < (unsigned int)pixels_count; )
          {
             CHECK_RET(read_block(map, length, position, &headbyte, 1));

             if (headbyte >= 0)
               {  //  && HeadByte <= 127
                  CHECK_RET(read_block(map, length, position, buffer + i, headbyte + 1));

                  i += headbyte + 1;
               }
             if (headbyte >= -127 && headbyte <= -1)
               {
                  int run;

                  CHECK_RET(read_block(map, length, position, &c, 1));

                  run = c;
                  if (run == -1) return EINA_FALSE;

                  memset(buffer + i, run, -headbyte + 1);
                  i += -headbyte + 1;
               }
          }
     }

#undef CHECK_RET

   return EINA_TRUE;
}

Eina_Bool
read_psd_grey(void *pixels, PSD_Header *head, const unsigned char *map, size_t length, size_t *position, int *error)
{
   unsigned int color_mode, resource_size, misc_info;
   unsigned short compressed;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

#define CHECK_RET(Call)                  \
   if (!Call) return EINA_FALSE;

   CHECK_RET(read_uint(map, length, position, &color_mode));
   // Skip over the 'color mode data section'
   *position += color_mode;

   CHECK_RET(read_uint(map, length, position, &resource_size));
   // Read the 'image resources section'
   *position += resource_size;

   CHECK_RET(read_uint(map, length, position, &misc_info));
   *position += misc_info;

   CHECK_RET(read_ushort(map, length, position, &compressed));

   head->channel_num = head->channels;
   // Temporary to read only one channel...some greyscale .psd files have 2.
   head->channels = 1;

   switch (head->depth)
     {
      case 8:
      case 16:
         break;
      default:
         *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
         return EINA_FALSE;
     }

   if (!psd_get_data(head, map, length, position, pixels, compressed, error))
     goto cleanup_error;

   return EINA_TRUE;

#undef CHECK_RET

 cleanup_error:
   return EINA_FALSE;
}


Eina_Bool
read_psd_indexed(void *pixels, PSD_Header *head, const unsigned char *map, size_t length, size_t *position, int *error)
{
   unsigned int color_mode, resource_size, misc_info;
   unsigned short compressed;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

#define CHECK_RET(Call)                  \
   if (!(Call)) return EINA_FALSE;

   CHECK_RET(read_uint(map, length, position, &color_mode));
   CHECK_RET(!(color_mode % 3));
   /*
     Palette = (unsigned char*)malloc(Colormode);
     if (Palette == NULL)
     return EINA_FALSE;
     if (fread(&Palette, 1, Colormode, file) != Colormode)
     goto cleanup_error;
   */
   // Skip over the 'color mode data section'
   *position += color_mode;

   // Read the 'image resources section'
   CHECK_RET(read_uint(map, length, position, &resource_size));
   *position += resource_size;

   CHECK_RET(read_uint(map, length, position, &misc_info));
   *position += misc_info;

   CHECK_RET(read_ushort(map, length, position, &compressed));

   if (head->channels != 1 || head->depth != 8)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }
   head->channel_num = head->channels;

   if (!psd_get_data(head, map, length, position, pixels, compressed, error))
     return EINA_FALSE;
   return EINA_TRUE;

#undef CHECK_RET
}

Eina_Bool
read_psd_rgb(void *pixels, PSD_Header *head, const unsigned char *map, size_t length, size_t *position, int *error)
{
   unsigned int color_mode, resource_size, misc_info;
   unsigned short compressed;

#define CHECK_RET(Call)                  \
   if (!Call) return EINA_FALSE;

   CHECK_RET(read_uint(map, length, position, &color_mode));
   // Skip over the 'color mode data section'
   *position += color_mode;

   // Read the 'image resources section'
   CHECK_RET(read_uint(map, length, position, &resource_size));
   *position += resource_size;

   CHECK_RET(read_uint(map, length, position, &misc_info));
   *position += misc_info;

   CHECK_RET(read_ushort(map, length, position, &compressed));

   head->channel_num = head->channels;

   switch (head->depth)
     {
      case 8:
      case 16:
         break;
      default:
         *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
         return EINA_FALSE;
     }

   if (!psd_get_data(head, map, length, position, pixels, compressed, error))
     return EINA_FALSE;

   return EINA_TRUE;

#undef CHECK_RET
}

Eina_Bool
read_psd_cmyk(Evas_Image_Property *prop, void *pixels, PSD_Header *head, const unsigned char *map, size_t length, size_t *position, int *error)
{
   unsigned int color_mode, resource_size, misc_info, size, j, data_size;
   unsigned short compressed;
   unsigned int format, type;
   unsigned char *kchannel = NULL;
   Eina_Bool r = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

#define CHECK_RET(Call)                  \
   if (!Call) return EINA_FALSE;

   CHECK_RET(read_uint(map, length, position, &color_mode));
   // Skip over the 'color mode data section'
   *position += color_mode;

   CHECK_RET(read_uint(map, length, position, &resource_size));
   // Read the 'image resources section'
   *position += resource_size;

   CHECK_RET(read_uint(map, length, position, &misc_info));
   *position += misc_info;

   CHECK_RET(read_ushort(map, length, position, &compressed));

   switch (head->channels)
     {
      case 4:
         format = 0x1907;
         head->channel_num = 4;
         head->channels = 3;
         break;
      case 5:
         format = 0x1908;
         head->channel_num = 5;
         head->channels = 4;
         break;
      default:
         *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
         return EINA_FALSE;
     }

   switch (head->depth)
     {
      case 8:
         type = 1;
         break;
      case 16:
         type = 2;
         break;
      default:
         *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
         return EINA_FALSE;
     }

   if (!psd_get_data(head, map, length, position, pixels, compressed, error))
     goto cleanup_error;

   size = type * prop->w * prop->h;
   kchannel = malloc(size);
   if (kchannel == NULL)
     goto cleanup_error;
   if (!get_single_channel(head, map, length, position, kchannel, compressed))
     goto cleanup_error;

   data_size = head->channels * type * prop->w * prop->h;
   if (format == 0x1907)
     {
        unsigned char *tmp = pixels;
        const unsigned char *limit = tmp + data_size;

        for (j = 0; tmp < limit; tmp++, j++)
          {
             int k;

             for (k = 0; k < 3; k++)
               *tmp = (*tmp * kchannel[j]) >> 8;

             // FIXME: tmp[i+3] = 255;
          }
     }
   else
     {  // RGBA
        unsigned char *tmp = pixels;
        const unsigned char *limit = tmp + data_size;

        // The KChannel array really holds the alpha channel on this one.
        for (j = 0; tmp < limit; tmp += 4, j++)
          {
             tmp[0] = (tmp[0] * tmp[3]) >> 8;
             tmp[1] = (tmp[1] * tmp[3]) >> 8;
             tmp[2] = (tmp[2] * tmp[3]) >> 8;
             tmp[3] = kchannel[j];  // Swap 'K' with alpha channel.
          }
     }

   r = EINA_TRUE;

 cleanup_error:
   free(kchannel);
   return r;
}

static Eina_Bool
evas_image_load_file_data_psd(void *loader_data,
			      Evas_Image_Property *prop,
			      void *pixels,
			      int *error)
{
   Eina_File *f = loader_data;

   void *map;
   size_t length;
   size_t position;
   PSD_Header header;
   Eina_Bool bpsd = EINA_FALSE;

   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   length = eina_file_size_get(f);
   position = 0;
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (!map || length < 1)
     goto on_error;

   *error = EVAS_LOAD_ERROR_GENERIC;
   if (!psd_get_header(&header, map, length, &position) || !is_psd(&header))
     goto on_error;

   if (header.width != prop->w ||
       header.height != prop->h)
     goto on_error;

   *error = EVAS_LOAD_ERROR_NONE;

   switch (header.mode)
     {
      case PSD_GREYSCALE:  // Greyscale
         bpsd = read_psd_grey(pixels, &header, map, length, &position, error);
         break;
      case PSD_INDEXED:  // Indexed
         bpsd = read_psd_indexed(pixels, &header, map, length, &position, error);
         break;
      case PSD_RGB:  // RGB
         bpsd = read_psd_rgb(pixels, &header, map, length, &position, error);
	 prop->premul = EINA_TRUE;
         break;
      case PSD_CMYK:  // CMYK
         bpsd = read_psd_cmyk(prop, pixels, &header, map, length, &position, error);
         prop->premul = EINA_TRUE;
         break;
      default :
         *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
         bpsd = EINA_FALSE;
     }

 on_error:
   if (map) eina_file_map_free(f, map);

   return bpsd;
}

static Eina_Bool
get_compressed_channels_length(PSD_Header *head,
                               const unsigned char *map, size_t length, size_t *position,
			       unsigned short *rle_table,
			       unsigned int *chanlen)
{
   unsigned int j;
   unsigned int c;

   if (!read_block(map, length, position, rle_table,
                   sizeof (unsigned short) * head->height * head->channel_num))
     return EINA_FALSE;

   memset(chanlen, 0, head->channel_num * sizeof(unsigned int));
   for (c = 0; c < head->channel_num; c++)
     {
        unsigned int i;

        j = c * head->height;
        for (i = 0; i < head->height; i++)
          {
	     chanlen[c] += rle_table[i + j];
          }
     }

   return EINA_TRUE;
}

static const Evas_Image_Load_Func evas_image_load_psd_func = {
  evas_image_load_file_open_psd,
  evas_image_load_file_close_psd,
  evas_image_load_file_head_psd,
  evas_image_load_file_data_psd,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_psd_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
  {
    EVAS_MODULE_API_VERSION,
    "psd",
    "none",
    {
      module_open,
      module_close
    }
  };

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, psd);


#ifndef EVAS_STATIC_BUILD_PSD
EVAS_EINA_MODULE_DEFINE(image_loader, psd);
#endif
