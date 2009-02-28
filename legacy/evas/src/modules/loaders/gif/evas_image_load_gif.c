#include "evas_common.h"
#include "evas_private.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gif_lib.h>


int evas_image_load_file_head_gif(Image_Entry *ie, const char *file, const char *key);
int evas_image_load_file_data_gif(Image_Entry *ie, const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_gif_func =
{
  evas_image_load_file_head_gif,
  evas_image_load_file_data_gif
};


int
evas_image_load_file_head_gif(Image_Entry *ie, const char *file, const char *key __UNUSED__)
{
   int                 fd;
   GifFileType        *gif;
   GifRecordType       rec;
   int                 done;
   int                 w;
   int                 h;
   int                 alpha;

   done = 0;
   w = 0;
   h = 0;
   alpha = -1;

   if (!file) return 0;

#ifndef __EMX__
   fd = open(file, O_RDONLY);
#else
   fd = open(file, O_RDONLY | O_BINARY);
#endif
   if (fd < 0)
      return 0;

   gif = DGifOpenFileHandle(fd);
   if (!gif)
     {
        close(fd);
        return 0;
     }

   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR)
          {
             /* PrintGifError(); */
             rec = TERMINATE_RECORD_TYPE;
          }
        if ((rec == IMAGE_DESC_RECORD_TYPE) && (!done))
          {
             if (DGifGetImageDesc(gif) == GIF_ERROR)
               {
                  /* PrintGifError(); */
                  rec = TERMINATE_RECORD_TYPE;
               }
             w = gif->Image.Width;
             h = gif->Image.Height;
	     if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
	       {
		  DGifCloseFile(gif);
		  return 0;
	       }
	     done = 1;
          }
        else if (rec == EXTENSION_RECORD_TYPE)
          {
             int                 ext_code;
             GifByteType        *ext;
	     
             ext = NULL;
             DGifGetExtension(gif, &ext_code, &ext);
             while (ext)
               {
                  if ((ext_code == 0xf9) && (ext[1] & 1) && (alpha < 0))
                    {
                       alpha = (int)ext[4];
                    }
                  ext = NULL;
                  DGifGetExtensionNext(gif, &ext);
               }
          }
   } while (rec != TERMINATE_RECORD_TYPE);

   if (alpha >= 0) ie->flags.alpha = 1;
   ie->w = w;
   ie->h = h;

   DGifCloseFile(gif);
   return 1;
}

int
evas_image_load_file_data_gif(Image_Entry *ie, const char *file, const char *key __UNUSED__)
{
   int                 intoffset[] = { 0, 4, 2, 1 };
   int                 intjump[] = { 8, 8, 4, 2 };
   double              per;
   double              per_inc;
   int                 fd;
   GifFileType        *gif;
   GifRecordType       rec;
   GifRowType         *rows;
   ColorMapObject     *cmap;
   DATA32             *ptr;
   int                 done;
   int                 last_y;
   int                 last_per;
   int                 w;
   int                 h;
   int                 alpha;
   int                 i;
   int                 j;
   int                 bg;
   int                 r;
   int                 g;
   int                 b;

   rows = NULL;
   per = 0.0;
   done = 0;
   last_y = 0;
   last_per = 0;
   w = 0;
   h = 0;
   alpha = -1;

   if (!file) return 0;

#ifndef __EMX__
   fd = open(file, O_RDONLY);
#else
   fd = open(file, O_RDONLY | O_BINARY);
#endif
   if (fd < 0)
      return 0;

   gif = DGifOpenFileHandle(fd);
   if (!gif)
     {
        close(fd);
        return 0;
     }
   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR)
          {
             /* PrintGifError(); */
             rec = TERMINATE_RECORD_TYPE;
          }
        if ((rec == IMAGE_DESC_RECORD_TYPE) && (!done))
          {
             if (DGifGetImageDesc(gif) == GIF_ERROR)
               {
                  /* PrintGifError(); */
                  rec = TERMINATE_RECORD_TYPE;
               }
             w = gif->Image.Width;
             h = gif->Image.Height;
             rows = malloc(h * sizeof(GifRowType *));
             if (!rows)
               {
                  DGifCloseFile(gif);
                  return 0;
               }
             for (i = 0; i < h; i++)
               {
                  rows[i] = NULL;
               }
             for (i = 0; i < h; i++)
               {
                  rows[i] = malloc(w * sizeof(GifPixelType));
                  if (!rows[i])
                    {
                       DGifCloseFile(gif);
                       for (i = 0; i < h; i++)
                         {
                            if (rows[i])
                              {
                                 free(rows[i]);
                              }
                         }
                       free(rows);
                       return 0;
                    }
               }
             if (gif->Image.Interlace)
               {
                  for (i = 0; i < 4; i++)
                    {
                       for (j = intoffset[i]; j < h; j += intjump[i])
                         {
                            DGifGetLine(gif, rows[j], w);
                         }
                    }
               }
             else
               {
                  for (i = 0; i < h; i++)
                    {
                       DGifGetLine(gif, rows[i], w);
                    }
               }
             done = 1;
          }
        else if (rec == EXTENSION_RECORD_TYPE)
          {
             int                 ext_code;
             GifByteType        *ext;

             ext = NULL;
             DGifGetExtension(gif, &ext_code, &ext);
             while (ext)
               {
                  if ((ext_code == 0xf9) && (ext[1] & 1) && (alpha < 0))
                    {
                       alpha = (int)ext[4];
                    }
                  ext = NULL;
                  DGifGetExtensionNext(gif, &ext);
               }
          }
   } while (rec != TERMINATE_RECORD_TYPE);

   if (alpha >= 0) ie->flags.alpha = 1;
   evas_cache_image_surface_alloc(ie, w, h);
   if (!evas_cache_image_pixels(ie))
     {
        DGifCloseFile(gif);
        for (i = 0; i < h; i++)
          {
            free(rows[i]);
          }
        free(rows);
	return 0;
     }

   bg = gif->SBackGroundColor;
   cmap = (gif->Image.ColorMap ? gif->Image.ColorMap : gif->SColorMap);

   ptr = evas_cache_image_pixels(ie);
   per_inc = 100.0 / (((double)w) * h);

   for (i = 0; i < h; i++)
     {
       for (j = 0; j < w; j++)
         {
           if (rows[i][j] == alpha)
             {
               r = cmap->Colors[bg].Red;
               g = cmap->Colors[bg].Green;
               b = cmap->Colors[bg].Blue;
               *ptr++ = 0x00ffffff & ((r << 16) | (g << 8) | b);
             }
           else
             {
               r = cmap->Colors[rows[i][j]].Red;
               g = cmap->Colors[rows[i][j]].Green;
               b = cmap->Colors[rows[i][j]].Blue;
               *ptr++ = (0xff << 24) | (r << 16) | (g << 8) | b;
             }
           per += per_inc;
         }
     }
   evas_common_image_premul(ie);
   DGifCloseFile(gif);
   for (i = 0; i < h; i++)
     {
        free(rows[i]);
     }
   free(rows);

   return 1;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_gif_func);
   return 1;
}

EAPI void
module_close(void)
{
   
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_LOADER,
     "gif",
     "none"
};
