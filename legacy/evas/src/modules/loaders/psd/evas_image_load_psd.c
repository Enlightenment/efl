#define _XOPEN_SOURCE

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
#include <fcntl.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
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
                                                FILE *file,
                                                unsigned short *rle_table,
                                                unsigned int *chanlen);

static int
read_ushort(FILE *file, unsigned short *ret)
{
   unsigned char b[2];
   if (fread(b, sizeof(unsigned char), 2, file) != 2) return 0;
   // FIXME: need to check order
   *ret = (b[0] << 8) | b[1];
   return 1;
}

static int
read_uint(FILE *file, unsigned int *ret)
{
   unsigned char       b[4];
   if (fread(b, sizeof(unsigned char), 4, file) != 4) return 0;
   // FIXME: need to check order
   *ret = ARGB_JOIN(b[0], b[1], b[2], b[3]);
   return 1;
}

// Internal function used to get the Psd header from the current file.
Eina_Bool
psd_get_header(PSD_Header *header, FILE * file)
{
   unsigned short tmp;

#define CHECK_RET(Call, Value)                  \
   if (Call != Value) return EINA_FALSE;

   CHECK_RET(fread(header->signature, sizeof (unsigned char), 4, file), 4);
   CHECK_RET(read_ushort(file, &header->version), 1);
   CHECK_RET(fread(header->reserved, sizeof (unsigned char), 6, file), 6);
   CHECK_RET(read_ushort(file, &header->channels), 1);
   CHECK_RET(read_uint(file, &header->height), 1);
   CHECK_RET(read_uint(file, &header->width), 1);
   CHECK_RET(read_ushort(file, &header->depth), 1);

   CHECK_RET(read_ushort(file, &tmp), 1);
   header->mode = tmp;

#undef CHECK_RET

   /* fprintf(stderr, "<<<<<<<<<<<\nsignature : %c%c%c%c\n", */
   /*         header->signature[0], */
   /*         header->signature[1], */
   /*         header->signature[2], */
   /*         header->signature[3]); */
   /* fprintf(stderr, "version : %i\n", header->version); */
   /* fprintf(stderr, "channels : %i\n", header->channels); */
   /* fprintf(stderr, "width x height : %dx%d\n", header->width, header->height); */
   /* fprintf(stderr, "depth : %i\n", header->depth); */
   /* fprintf(stderr, "mode : %i\n>>>>>>>>>>>>\n", header->mode); */

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

static Eina_Bool
evas_image_load_file_head_psd(Image_Entry *ie, const char *FileName, 
                              const char *key __UNUSED__, int *error)
{
   FILE	*f;
   PSD_Header header;
   Eina_Bool correct;

   *error = EVAS_LOAD_ERROR_NONE;

   f = fopen(FileName, "rb");
   if (f == NULL)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }
   correct = psd_get_header(&header, f);
   fclose(f);

   if (!correct || !is_psd(&header))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

   ie->w = header.width;
   ie->h = header.height;
   if (header.channels == 3) ie->flags.alpha = 0;
   else ie->flags.alpha = 1;

   return EINA_TRUE;
}

static unsigned int
read_compressed_channel(FILE* file,
			const unsigned int channel_length __UNUSED__, 
                        unsigned int size,
			unsigned char* channel)
{
   // FIXME: what does channel_length means, and why is it not used
   unsigned int i;
   char headbyte, c;

#define CHECK_RET(Call, Value)                                          \
   if (Call != Value) return READ_COMPRESSED_ERROR_FILE_READ_ERROR;

   for (i = 0; i < size; )
     {
        CHECK_RET(fread(&headbyte, 1, 1, file), 1);

        if (headbyte >= 0)
          {
             if (i + headbyte > size)
	       return READ_COMPRESSED_ERROR_FILE_CORRUPT;

	     CHECK_RET(fread(channel + i, headbyte + 1, 1, file), 1);

             i += headbyte + 1;
          }
        else if (headbyte >= -127 && headbyte <= -1)
          {
             int run;

	     CHECK_RET(fread(&c, 1, 1, file), 1);

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
psd_get_data(Image_Entry *ie __UNUSED__,
	     PSD_Header *head,
	     FILE *f,
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
        fprintf(stderr, "unsupported file format.\n");
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

#define CHECK_RET(Call, Value)			\
   if (Call != Value)				\
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

                  CHECK_RET(fread(tmp, pixels_count, 1, f), 1);

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

                  CHECK_RET(fread(channel, pixels_count, 1, f), 1);

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

                  CHECK_RET(fread(channel, pixels_count * 2, 1, f), 1);

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

		  CHECK_RET(fread(channel, pixels_count * 2, 1, f), 1);

		  for (y = 0; y < head->height * bps2; y += bps2)
                    {
                       for (x = 0; x < (unsigned int)bps2; x += bpp, shortptr)
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
        if (!get_compressed_channels_length(head, f, rle_table, chanlen))
	  goto file_read_error;

        for (c = 0; c < numchan; c++)
          {
	     unsigned char *tmp = channel;
	     int err;

	     err = read_compressed_channel(f,
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

                  err = read_compressed_channel(f,
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
get_single_channel(Image_Entry *ie __UNUSED__,
		   PSD_Header *head,
		   FILE *f,
		   unsigned char *buffer,
		   Eina_Bool compressed)
{
   unsigned int i, bpc;
   unsigned short *tmp;
   char headbyte;
   int c;
   int pixels_count;

   tmp = (unsigned short*)buffer;
   bpc = (head->depth / 8);
   pixels_count = head->width * head->height;

#define CHECK_RET(Call, Value)                  \
   if (Call != Value) return EINA_FALSE;

   if (!compressed)
     {
        if (bpc == 1)
          {
             CHECK_RET(fread(buffer, pixels_count, 1, f), 1);
          }
        else
          {  // Bpc == 2
             CHECK_RET(fread(buffer, pixels_count * 2, 1, f), 1);
          }
     }
   else
     {
        for (i = 0; i < (unsigned int)pixels_count; )
          {
             CHECK_RET(fread(&headbyte, 1, 1, f), 1);

             if (headbyte >= 0)
               {  //  && HeadByte <= 127
                  CHECK_RET(fread(buffer + i, headbyte + 1, 1, f), 1);

                  i += headbyte + 1;
               }
             if (headbyte >= -127 && headbyte <= -1)
               {
                  int run;

                  CHECK_RET(fread(&c, 1, 1, f), 1);

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
read_psd_grey(Image_Entry *ie, PSD_Header *head, FILE * f, int *error)
{
   unsigned int color_mode, resource_size, misc_info;
   unsigned short compressed;
   unsigned int type;
   void *surface = NULL;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

#define CHECK_RET(Call, Value)                  \
   if (Call != Value) return EINA_FALSE;

   CHECK_RET(read_uint(f, &color_mode), 1);
   // Skip over the 'color mode data section'
   CHECK_RET(fseek(f, color_mode, SEEK_CUR), 0);

   CHECK_RET(read_uint(f, &resource_size), 1);
   // Read the 'image resources section'

   CHECK_RET(fseek(f, resource_size, SEEK_CUR), 0);

   CHECK_RET(read_uint(f, &misc_info), 1);
   CHECK_RET(fseek(f, misc_info, SEEK_CUR), 0);

   CHECK_RET(read_ushort(f, &compressed), 1);

   ie->w = head->width;
   ie->h = head->height;
   if (head->channels == 3) ie->flags.alpha = 0;
   else ie->flags.alpha = 1;

   head->channel_num = head->channels;
   // Temporary to read only one channel...some greyscale .psd files have 2.
   head->channels = 1;

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

   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto cleanup_error;
     }

   if (!psd_get_data(ie, head, f, surface, compressed, error))
     goto cleanup_error;

   return EINA_TRUE;

#undef CHECK_RET

 cleanup_error:
   return EINA_FALSE;
}


Eina_Bool
read_psd_indexed(Image_Entry *ie, PSD_Header *head, FILE * f, int *error)
{
   unsigned int color_mode, resource_size, misc_info;
   unsigned short compressed;
   void *surface;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

#define CHECK_RET(Call, Value)                  \
   if (Call != Value) return EINA_FALSE;

   CHECK_RET(read_uint(f, &color_mode), 1);
   CHECK_RET((color_mode % 3), 0);
   /*
     Palette = (unsigned char*)malloc(Colormode);
     if (Palette == NULL)
     return EINA_FALSE;
     if (fread(&Palette, 1, Colormode, file) != Colormode)
     goto cleanup_error;
   */
   // Skip over the 'color mode data section'
   CHECK_RET(fseek(f, color_mode, SEEK_CUR), 0);

   // Read the 'image resources section'
   CHECK_RET(read_uint(f, &resource_size), 1);
   CHECK_RET(fseek(f, resource_size, SEEK_CUR), 0);

   CHECK_RET(read_uint(f, &misc_info), 1);
   CHECK_RET(fseek(f, misc_info, SEEK_CUR), 0);

   CHECK_RET(read_ushort(f, &compressed), 1);

   if (head->channels != 1 || head->depth != 8)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }
   head->channel_num = head->channels;

   ie->w = head->width;
   ie->h = head->height;
   if (head->channels == 3) ie->flags.alpha = 0;
   else ie->flags.alpha = 1;

   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   if (!psd_get_data(ie, head, f, surface, compressed, error))
     return EINA_FALSE;
   return EINA_TRUE;

#undef CHECK_RET
}

Eina_Bool
read_psd_rgb(Image_Entry *ie, PSD_Header *head, FILE *f, int *error)
{
   unsigned int color_mode, resource_size, misc_info;
   unsigned short compressed;
   unsigned int type;
   void *surface;

#define CHECK_RET(Call, Value)                  \
   if (Call != Value) return EINA_FALSE;

   CHECK_RET(read_uint(f, &color_mode), 1);
   // Skip over the 'color mode data section'
   CHECK_RET(fseek(f, color_mode, SEEK_CUR), 0);

   // Read the 'image resources section'
   CHECK_RET(read_uint(f, &resource_size), 1);
   CHECK_RET(fseek(f, resource_size, SEEK_CUR), 0);

   CHECK_RET(read_uint(f, &misc_info), 1);
   CHECK_RET(fseek(f, misc_info, SEEK_CUR), 0);

   CHECK_RET(read_ushort(f, &compressed), 1);

   head->channel_num = head->channels;

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
   ie->w = head->width;
   ie->h = head->height;
   if (head->channels == 3) ie->flags.alpha = 0;
   else ie->flags.alpha = 1;

   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto cleanup_error;
     }

   if (!psd_get_data(ie, head, f, surface, compressed, error))
     goto cleanup_error;

   evas_common_image_premul(ie);
   return EINA_TRUE;

#undef CHECK_RET

 cleanup_error:
   return EINA_FALSE;
}

Eina_Bool
read_psd_cmyk(Image_Entry *ie, PSD_Header *head, FILE *f, int *error)
{
   unsigned int color_mode, resource_size, misc_info, size, i, j, data_size;
   unsigned short compressed;
   unsigned int format, type;
   unsigned char *kchannel = NULL;
   void *surface;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

#define CHECK_RET(Call, Value)                  \
   if (Call != Value) return EINA_FALSE;

   CHECK_RET(read_uint(f, &color_mode), 1);
   // Skip over the 'color mode data section'
   CHECK_RET(fseek(f, color_mode, SEEK_CUR), 0);

   CHECK_RET(read_uint(f, &resource_size), 1);
   // Read the 'image resources section'
   CHECK_RET(fseek(f, resource_size, SEEK_CUR), 0);

   CHECK_RET(read_uint(f, &misc_info), 1);
   CHECK_RET(fseek(f, misc_info, SEEK_CUR), 0);

   CHECK_RET(read_ushort(f, &compressed), 1);

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

   ie->w = head->width;
   ie->h = head->height;
   if (head->channels == 3) ie->flags.alpha = 0;
   else ie->flags.alpha = 1;

   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto cleanup_error;
     }

   if (!psd_get_data(ie, head, f, surface, compressed, error))
     goto cleanup_error;

   size = type * ie->w * ie->h;
   kchannel = malloc(size);
   if (kchannel == NULL)
     goto cleanup_error;
   if (!get_single_channel(ie, head, f, kchannel, compressed))
     goto cleanup_error;

   data_size = head->channels * type * ie->w * ie->h;
   if (format == 0x1907)
     {
        unsigned char *tmp = surface;
        const unsigned char *limit = tmp + data_size;

        for (i = 0, j = 0; tmp < limit; tmp++, j++)
          {
             int k;

             for (k = 0; k < 3; k++)
               *tmp = (*tmp * kchannel[j]) >> 8;

             // FIXME: tmp[i+3] = 255;
          }
     }
   else
     {  // RGBA
        unsigned char *tmp = surface;
        const unsigned char *limit = tmp + data_size;

        // The KChannel array really holds the alpha channel on this one.
        for (i = 0, j = 0; tmp < limit; tmp += 4, j++)
          {
             tmp[0] = (tmp[0] * tmp[3]) >> 8;
             tmp[1] = (tmp[1] * tmp[3]) >> 8;
             tmp[2] = (tmp[2] * tmp[3]) >> 8;
             tmp[3] = kchannel[j];  // Swap 'K' with alpha channel.
          }
     }

   free(kchannel);

   evas_common_image_premul(ie);
   return EINA_TRUE;

 cleanup_error:
   free(kchannel);
   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_data_psd(Image_Entry *ie,
                              const char *file,
                              const char *key __UNUSED__,
                              int *error)
{
   FILE *f;
   PSD_Header header;
   Eina_Bool bpsd = EINA_FALSE;

   f = fopen(file, "rb");
   if (f == NULL)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return bpsd;
     }

   if (!psd_get_header(&header, f) || !is_psd(&header))
     {
        fclose(f);
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

   ie->w = header.width;
   ie->h = header.height;

   *error = EVAS_LOAD_ERROR_NONE;

   switch (header.mode)
     {
      case PSD_GREYSCALE:  // Greyscale
         bpsd = read_psd_grey(ie, &header, f, error);
         break;
      case PSD_INDEXED:  // Indexed
         bpsd = read_psd_indexed(ie, &header, f, error);
         break;
      case PSD_RGB:  // RGB
         bpsd = read_psd_rgb(ie, &header, f, error);
         break;
      case PSD_CMYK:  // CMYK
         bpsd = read_psd_cmyk(ie, &header, f, error);
         break;
      default :
         *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
         bpsd = EINA_FALSE;
     }
   fclose(f);

   return bpsd;
}

static Eina_Bool
get_compressed_channels_length(PSD_Header *head,
			       FILE * file,
			       unsigned short *rle_table,
			       unsigned int *chanlen)
{
   unsigned int j;
   unsigned int c;

   if (fread(rle_table,
	     sizeof(unsigned short),
	     head->height * head->channel_num,
	     file) != head->height * head->channel_num)
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
  EINA_TRUE,
  evas_image_load_file_head_psd,
  evas_image_load_file_data_psd
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_psd_func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
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
