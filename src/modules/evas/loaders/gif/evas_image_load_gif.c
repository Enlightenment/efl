#include "evas_common_private.h"
#include "evas_private.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <gif_lib.h>

#ifndef MIN
# define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct _Gif_Frame Gif_Frame;

typedef enum _Frame_Load_Type
{
   LOAD_FRAME_NONE = 0,
   LOAD_FRAME_INFO = 1,
   LOAD_FRAME_DATA = 2,
   LOAD_FRAME_DATA_INFO = 3
} Frame_Load_Type;

struct _Gif_Frame
{
   struct {
      /* Image descriptor */
      int        x;
      int        y;
      int        w;
      int        h;
      int        interlace;
   } image_des;

   struct {
      /* Graphic Control*/
      int        disposal;
      int        transparent;
      int        delay;
      int        input;
   } frame_info;
   int bg_val;
};

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
   Evas_Image_Animated *animated;
};

static Eina_Bool evas_image_load_specific_frame(Eina_File *f, const Evas_Image_Load_Opts *opts, Evas_Image_Property *prop, Evas_Image_Animated *animated, int frame_index, int *error);

#define byte2_to_int(a,b)         (((b)<<8)|(a))

#define FRAME_MAX 1024

/* find specific frame in image entry */
static Eina_Bool
_find_frame(Evas_Image_Animated *animated, int frame_index, Image_Entry_Frame **frame)
{
   Eina_List *l;
   Image_Entry_Frame *hit_frame = NULL;

   if (!animated->frames) return EINA_FALSE;

   EINA_LIST_FOREACH(animated->frames, l, hit_frame)
     {
        if (hit_frame->index == frame_index)
          {
             *frame = hit_frame;
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static Eina_Bool
_find_close_frame(Evas_Image_Animated *animated, int frame_index, Image_Entry_Frame **frame)
{
  Eina_Bool hit = EINA_FALSE;
  int i;

  i = frame_index -1;

  if (!animated->frames) return EINA_FALSE;

  for (; i > 0; i--)
    {
       hit = _find_frame(animated, i, frame);
       if (hit)
         return  EINA_TRUE;
    }
  return EINA_FALSE;
}

static Eina_Bool
_evas_image_skip_frame(GifFileType *gif, int frame)
{
   int                 remain_frame = 0;
   GifRecordType       rec;

   if (!gif) return EINA_FALSE;
   if (frame == 0) return EINA_TRUE; /* no need to skip */
   if (frame < 0 || frame > FRAME_MAX) return EINA_FALSE;

   remain_frame = frame;

   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR) return EINA_FALSE;

        if (rec == EXTENSION_RECORD_TYPE)
          {
             int                 ext_code;
             GifByteType        *ext;

             ext = NULL;
             DGifGetExtension(gif, &ext_code, &ext);
             while (ext)
               { /*skip extention */
                  ext = NULL;
                  DGifGetExtensionNext(gif, &ext);
               }
          }

        if (rec == IMAGE_DESC_RECORD_TYPE)
          {
             int                 img_code;
             GifByteType        *img;

             if (DGifGetImageDesc(gif) == GIF_ERROR) return EINA_FALSE;

             remain_frame --;
             /* we have to count frame, so use DGifGetCode and skip decoding */
             if (DGifGetCode(gif, &img_code, &img) == GIF_ERROR) return EINA_FALSE;

             while (img)
               {
                  img = NULL;
                  DGifGetCodeNext(gif, &img);
               }
             if (remain_frame < 1) return EINA_TRUE;
          }
        if (rec == TERMINATE_RECORD_TYPE) return EINA_FALSE;  /* end of file */

     } while ((rec != TERMINATE_RECORD_TYPE) && (remain_frame > 0));
   return EINA_FALSE;
}

static Eina_Bool
_evas_image_load_frame_graphic_info(Image_Entry_Frame *frame, GifByteType  *ext)
{
   Gif_Frame *gif_frame = NULL;
   if ((!frame) || (!ext)) return EINA_FALSE;

   gif_frame = (Gif_Frame *) frame->info;

   /* transparent */
   if ((ext[1] & 0x1) != 0)
     gif_frame->frame_info.transparent = ext[4];
   else
     gif_frame->frame_info.transparent = -1;

   gif_frame->frame_info.input = (ext[1] >>1) & 0x1;
   gif_frame->frame_info.disposal = (ext[1] >>2) & 0x7;
   gif_frame->frame_info.delay = byte2_to_int(ext[2], ext[3]);
   return EINA_TRUE;
}

static Eina_Bool
_evas_image_load_frame_image_des_info(GifFileType *gif, Image_Entry_Frame *frame)
{
   Gif_Frame *gif_frame = NULL;
   if ((!gif) || (!frame)) return EINA_FALSE;

   gif_frame = (Gif_Frame *) frame->info;
   gif_frame->image_des.x = gif->Image.Left;
   gif_frame->image_des.y = gif->Image.Top;
   gif_frame->image_des.w = gif->Image.Width;
   gif_frame->image_des.h = gif->Image.Height;
   gif_frame->image_des.interlace = gif->Image.Interlace;
   return EINA_TRUE;
}

static Eina_Bool
_evas_image_load_frame_image_data(Eina_File *f,
                                  const Evas_Image_Load_Opts *opts,
                                  Evas_Image_Property *prop,
                                  Evas_Image_Animated *animated,
                                  GifFileType *gif, Image_Entry_Frame *frame, int *error)
{
   ColorMapObject *cmap;
   GifRowType     *rows;
   GifPixelType   *tmp = NULL; /*for skip gif line */
   DATA32         *ptr;
   Gif_Frame      *gif_frame = NULL;

   double          per;
   double          per_inc;
   size_t          siz;
   int             intoffset[] = { 0, 4, 2, 1 };
   int             intjump[] = { 8, 8, 4, 2 };
   int x, y, w, h;
   int i, j;
   int bg;
   int r, g, b, alpha;
   int cache_w, cache_h;
   int cur_h, cur_w;
   int disposal = 0;
   int bg_val = 0;
   /* for scale down decoding */
   int scale_ratio = 1;
   int scale_w, scale_h, scale_x, scale_y;

   if ((!gif) || (!frame)) return EINA_FALSE;

   gif_frame = (Gif_Frame *) frame->info;
   w = gif->Image.Width;
   h = gif->Image.Height;
   x = gif->Image.Left;
   y = gif->Image.Top;
   cache_w = prop->w;
   cache_h = prop->h;

   /* if user don't set scale down, default scale_ratio is 1 */
   if (opts->scale_down_by > 1) scale_ratio = opts->scale_down_by;
   scale_w = w / scale_ratio;
   scale_h = h / scale_ratio;
   scale_x = x / scale_ratio;
   scale_y = y / scale_ratio;

   rows = malloc(scale_h * sizeof(GifRowType *));

   if (!rows)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }
   for (i = 0; i < scale_h; i++)
     {
        rows[i] = NULL;
     }
   /* alloc memory according to scaled size */
   for (i = 0; i < scale_h; i++)
     {
        rows[i] = malloc(w * sizeof(GifPixelType));
        if (!rows[i])
          {
             for (i = 0; i < scale_h; i++)
               {
                  if (rows[i])
                    {
                       free(rows[i]);
                    }
               }
             free(rows);
             *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             return EINA_FALSE;
          }
     }

   if (scale_ratio > 1)
     {
        tmp = malloc(w * sizeof(GifPixelType));
        if (!tmp)
          {
             *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             goto error;
          }
     }

   if (gif->Image.Interlace)
     {
        Eina_Bool multiple;
        int scale_j;
        for (i = 0; i < 4; i++)
          {
             for (j = intoffset[i]; j < h; j += intjump[i])
               {
                  scale_j = j / scale_ratio;
                  multiple = ((j % scale_ratio) ? EINA_FALSE : EINA_TRUE);

                  if (multiple && (scale_j < scale_h))
                    DGifGetLine(gif, rows[scale_j], w);
                  else
                    DGifGetLine(gif, tmp, w);
               }
          }
     }
   else
     {
        for (i = 0; i < scale_h; i++)
          {
             if (DGifGetLine(gif, rows[i], w) != GIF_OK)
               {
                  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                  goto error;
              }
             if (scale_ratio > 1)
               {
                  /* we use down sample method for scale down, so skip other line */
                  for (j = 0; j < (scale_ratio - 1); j++)
                    {
                       if (DGifGetLine(gif, tmp, w) != GIF_OK)
                         {
                            *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                            goto error;
                         }
                    }
               }
          }
     }

   if (scale_ratio > 1)
     {
        if (tmp) free(tmp);
        tmp = NULL;
     }

   alpha = gif_frame->frame_info.transparent;
   siz = cache_w *cache_h * sizeof(DATA32);
   frame->data = malloc(siz);
   if (!frame->data)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto error;
     }
   ptr = frame->data;
   bg = gif->SBackGroundColor;
   cmap = (gif->Image.ColorMap ? gif->Image.ColorMap : gif->SColorMap);

   if (!cmap)
     {
        DGifCloseFile(gif);
        for (i = 0; i < scale_h; i++)
          {
             free(rows[i]);
          }
        free(rows);
        if (frame->data) free(frame->data);
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   per_inc = 100.0 / (((double)w) * h);
   per = 0.0;
   cur_h = scale_h;
   cur_w = scale_w;

   if (cur_h > cache_h) cur_h = cache_h;
   if (cur_w > cache_w) cur_w = cache_w;

   if (frame->index > 1)
     {
        /* get previous frame only frame index is bigger than 1 */
        DATA32            *ptr_src;
        Image_Entry_Frame *new_frame = NULL;
        int                cur_frame = frame->index;
        int                start_frame = 1;

        if (_find_close_frame(animated, cur_frame, &new_frame))
          start_frame = new_frame->index + 1;

        if ((start_frame < 1) || (start_frame > cur_frame))
          {
             *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
             goto error;
          }
        /* load previous frame of cur_frame */
        for (j = start_frame; j < cur_frame ; j++)
          {
             // FIXME : that one -v
             if (!evas_image_load_specific_frame(f, opts, prop, animated, j, error))
               {
                  *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
                  goto error;
               }
          }
        if (!_find_frame(animated, cur_frame - 1, &new_frame))
          {
             *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
             goto error;
          }
        else
          {
             Gif_Frame *gif_frame2 = NULL;
             ptr_src = new_frame->data;
             if (new_frame->info)
               {
                  gif_frame2 = (Gif_Frame *)(new_frame->info);
                  disposal = gif_frame2->frame_info.disposal;
                  gif_frame->bg_val = gif_frame2->bg_val;
                  bg_val = gif_frame->bg_val;
               }
             switch(disposal) /* we only support disposal flag 0,1,2 */
               {
                case 1: /* Do not dispose. need previous frame*/
                  memcpy(ptr, ptr_src, siz);
                  /* only decoding image descriptor's region */
                  ptr = ptr + cache_w * scale_y;
                  
                  for (i = 0; i < cur_h; i++)
                    {
                       ptr = ptr + scale_x;
                       for (j = 0; j < cur_w; j++)
                         {
                            if (rows[i][j * scale_ratio] == alpha)
                              {
                                 ptr++ ;
                              }
                            else
                              {
                                 r = cmap->Colors[rows[i][j * scale_ratio]].Red;
                                 g = cmap->Colors[rows[i][j * scale_ratio]].Green;
                                 b = cmap->Colors[rows[i][j * scale_ratio]].Blue;
                                 *ptr++ = ARGB_JOIN(0xff, r, g, b);
                              }
                            per += per_inc;
                         }
                       ptr = ptr + (cache_w - (scale_x + cur_w));
                    }
                  break;
                case 2: /* Restore to background color */
                   memcpy(ptr, ptr_src, siz);
                   /* composite frames */
                   for (i = 0; i < cache_h; i++)
                    {
                       if ((i < scale_y) || (i >= (scale_y + cur_h)))
                         {
                            for (j = 0; j < cache_w; j++)
                              {
                                 *ptr = bg_val;
                                 ptr++;
                              }
                         }
                       else
                         {
                            int i1, j1;
                            i1 = i - scale_y;
                            
                            for (j = 0; j < cache_w; j++)
                              {
                                 j1 = j - scale_x;
                                 if ((j < scale_x) || (j >= (scale_x + cur_w)))
                                   {
                                      *ptr = bg_val;
                                      ptr++;
                                   }
                                 else
                                   {
                                      if (rows[i][j * scale_ratio] == alpha)
                                        {
                                           ptr++ ;
                                        }
                                      else
                                        {
                                           r = cmap->Colors[rows[i1][j1 * scale_ratio]].Red;
                                           g = cmap->Colors[rows[i1][j1 * scale_ratio]].Green;
                                           b = cmap->Colors[rows[i1][j1 * scale_ratio]].Blue;
                                           *ptr++ = ARGB_JOIN(0xff, r, g, b);
                                        }
                                   }
                              }
                         }
                    }
                   break;
                case 0: /* No disposal specified */
                default:
                   memset(ptr, 0, siz);
                   for (i = 0; i < cache_h; i++)
                     {
                        if ((i < scale_y) || (i >= (scale_y + cur_h)))
                          {
                             for (j = 0; j < cache_w; j++)
                               {
                                  *ptr = bg_val;
                                  ptr++;
                               }
                          }
                        else
                          {
                             int i1, j1;
                             i1 = i - scale_y;

                             for (j = 0; j < cache_w; j++)
                               {
                                  j1 = j - scale_x;
                                  if ((j < scale_x) || (j >= (scale_x + cur_w)))
                                    {
                                       *ptr = bg_val;
                                       ptr++;
                                    }
                                  else
                                    {
                                      if (rows[i][j * scale_ratio] == alpha)
                                        {
                                           ptr++ ;
                                        }
                                      else
                                        {
                                           r = cmap->Colors[rows[i1][j1 * scale_ratio]].Red;
                                           g = cmap->Colors[rows[i1][j1 * scale_ratio]].Green;
                                           b = cmap->Colors[rows[i1][j1 * scale_ratio]].Blue;
                                           *ptr++ = ARGB_JOIN(0xff, r, g, b);
                                        }
                                    }
                               }
                          }
                     }
                   break;
               }
          }
     }
   else /* first frame decoding */
     {
        /* get the background value */
        r = cmap->Colors[bg].Red;
        g = cmap->Colors[bg].Green;
        b = cmap->Colors[bg].Blue;
        bg_val =  ARGB_JOIN(0xff, r, g, b);
        gif_frame->bg_val = bg_val;

        memset(ptr, 0, siz);

        /* fill background color */
        for (i = 0; i < cache_h; i++)
          {
             /* the row's of logical screen not overap with frame */
             if ((i < scale_y) || (i >= (scale_y + cur_h)))
               {
                  for (j = 0; j < cache_w; j++)
                    {
                       *ptr = bg_val;
                       ptr++;
                    }
               }
             else
               {
                  int i1, j1;
                  i1 = i -scale_y;

                  for (j = 0; j < cache_w; j++)
                    {
                       j1 = j - scale_x;
                       if ((j < scale_x) || (j >= (scale_x + cur_w)))
                         {
                            *ptr = bg_val;
                            ptr++;
                         }
                       else
                         {
                            if (rows[i1][j1 * scale_ratio] == alpha)
                              {
                                 ptr++;
                              }
                            else
                              {
                                 r = cmap->Colors[rows[i1][j1 * scale_ratio]].Red;
                                 g = cmap->Colors[rows[i1][j1 * scale_ratio]].Green;
                                 b = cmap->Colors[rows[i1][j1 * scale_ratio]].Blue;
                                 *ptr++ = ARGB_JOIN(0xff, r, g, b);
                              }
                         }
                    }
               }
          }
     }

   for (i = 0; i < scale_h; i++)
     {
        if (rows[i]) free(rows[i]);
     }
   if (rows) free(rows);
   frame->loaded = EINA_TRUE;
   return EINA_TRUE;
error:
   for (i = 0; i < scale_h; i++)
     {
        if (rows[i]) free(rows[i]);
     }
   if (rows) free(rows);
   if (tmp) free(tmp);
   return EINA_FALSE;
}

static Eina_Bool
_evas_image_load_frame(Eina_File *f, const Evas_Image_Load_Opts *opts,
                       Evas_Image_Property *prop, Evas_Image_Animated *animated,
                       GifFileType *gif, Image_Entry_Frame *frame, Frame_Load_Type type, int *error)
{
   GifRecordType       rec;
   int                 gra_res = 0, img_res = 0;
   Eina_Bool           res = EINA_FALSE;

   if ((!gif) || (!frame)) return EINA_FALSE;
   if (type > LOAD_FRAME_DATA_INFO) return EINA_FALSE;

   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR) return EINA_FALSE;
        if (rec == IMAGE_DESC_RECORD_TYPE)
          {
             img_res++;
             break;
          }
        else if (rec == EXTENSION_RECORD_TYPE)
          {
             int           ext_code;
             GifByteType  *ext;

             ext = NULL;
             DGifGetExtension(gif, &ext_code, &ext);
             while (ext)
               {
                  if (ext_code == 0xf9) /* Graphic Control Extension */
                    {
                       gra_res++;
                       /* fill frame info */
                       if ((type == LOAD_FRAME_INFO) || (type == LOAD_FRAME_DATA_INFO))
                         _evas_image_load_frame_graphic_info(frame,ext);
                    }
                  ext = NULL;
                  DGifGetExtensionNext(gif, &ext);
               }
          }
     } while ((rec != TERMINATE_RECORD_TYPE) && (img_res == 0));
   if (img_res != 1) return EINA_FALSE;
   if (DGifGetImageDesc(gif) == GIF_ERROR) return EINA_FALSE;
   if ((type == LOAD_FRAME_INFO) || (type == LOAD_FRAME_DATA_INFO))
     _evas_image_load_frame_image_des_info(gif, frame);

   if ((type == LOAD_FRAME_DATA) || (type == LOAD_FRAME_DATA_INFO))
     {
        res = _evas_image_load_frame_image_data(f, opts, prop, animated,
                                                gif, frame, error);
        if (!res) return EINA_FALSE;
     }
   return EINA_TRUE;
}


/* set frame data to cache entry's data */
static Eina_Bool
evas_image_load_file_data_gif_internal(Evas_Image_Property *prop,
				       Image_Entry_Frame *frame,
				       void *pixels,
				       int *error)
{
   /* only copy real frame part */
   memcpy(pixels, frame->data, prop->w * prop->h * sizeof (DATA32));
   prop->premul = EINA_TRUE;

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

typedef struct _Evas_GIF_Info Evas_GIF_Info;
struct _Evas_GIF_Info
{
   unsigned char *map;
   int length;
   int position;
};

static int
_evas_image_load_file_read(GifFileType* gft, GifByteType *buf,int length)
{
   Evas_GIF_Info *egi = gft->UserData;

   if (egi->position == egi->length) return 0;
   if (egi->position + length == egi->length) length = egi->length - egi->position;
   memcpy(buf, egi->map + egi->position, length);
   egi->position += length;

   return length;
}
static void *
evas_image_load_file_open_gif(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                               Evas_Image_Load_Opts *opts,
                               Evas_Image_Animated *animated,
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
   loader->animated = animated;

   return loader;
}

static void
evas_image_load_file_close_gif(void *loader_data)
{
   free(loader_data);
}

static Eina_Bool
evas_image_load_file_head_gif(void *loader_data,
			      Evas_Image_Property *prop,
			      int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Load_Opts *load_opts;
   Evas_Image_Animated *animated;
   Eina_File *f;

   Evas_GIF_Info  egi;
   GifRecordType  rec;
   GifFileType   *gif = NULL;
   int            loop_count = -1;
   int            a;
   Eina_Bool      r = EINA_FALSE;
   //it is possible which gif file have error midle of frames,
   //in that case we should play gif file until meet error frame.
   int            image_count = 0;

   f = loader->f;
   load_opts = loader->opts;
   animated = loader->animated;

   prop->w = 0;
   prop->h = 0;
   a = 0;

   egi.map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!egi.map)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }
   egi.length = eina_file_size_get(f);
   egi.position = 0;

#if GIFLIB_MAJOR >= 5
   gif = DGifOpen(&egi, _evas_image_load_file_read, NULL);
#else
   gif = DGifOpen(&egi, _evas_image_load_file_read);
#endif
   if (!gif)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   /* check logical screen size */
   prop->w = gif->SWidth;
   prop->h = gif->SHeight;
   /* support scale down feture in gif*/
   if (load_opts->scale_down_by > 1)
     {
       prop->w /= load_opts->scale_down_by;
       prop->h /= load_opts->scale_down_by;
     }

   if ((prop->w < 1) || (prop->h < 1) ||
       (prop->w > IMG_MAX_SIZE) || (prop->h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(prop->w, prop->h))
     {
        if (IMG_TOO_BIG(prop->w, prop->h))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error = EVAS_LOAD_ERROR_GENERIC;
        goto on_error;
     }

   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR)
          {
             if (image_count > 1) break; //we should show normal frames.
             /* PrintGifError(); */
             *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
             goto on_error;
          }

        /* image descript info */
        if (rec == IMAGE_DESC_RECORD_TYPE)
          {
             int img_code;
             GifByteType *img;

             if (DGifGetImageDesc(gif) == GIF_ERROR)
               {
                  /* PrintGifError(); */
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  goto on_error;
               }
             /* we have to count frame, so use DGifGetCode and skip decoding */
             if (DGifGetCode(gif, &img_code, &img) == GIF_ERROR)
               {
                  /* PrintGifError(); */
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  goto on_error;
               }
             while (img)
               {
                  img = NULL;
                  DGifGetCodeNext(gif, &img);
               }
             image_count++;
          }
        else if (rec == EXTENSION_RECORD_TYPE)
          {
             int                 ext_code;
             GifByteType        *ext;

             ext = NULL;
             DGifGetExtension(gif, &ext_code, &ext);
             while (ext)
               {
                  if (ext_code == 0xf9) /* Graphic Control Extension */
                    {
                       if ((ext[1] & 1) && (prop->alpha == 0))
			 prop->alpha = (int)ext[4];
                    }
                  else if (ext_code == 0xff) /* application extension */
                    {
                       if (!strncmp ((char*)(&ext[1]), "NETSCAPE2.0", 11) ||
                           !strncmp ((char*)(&ext[1]), "ANIMEXTS1.0", 11))
                         {
                            ext=NULL;
                            DGifGetExtensionNext(gif, &ext);

                            if (ext[1] == 0x01)
                              {
                                 loop_count = ext[2] + (ext[3] << 8);
                                 if (loop_count > 0) loop_count++;
                              }
                         }
                     }

                  ext = NULL;
                  DGifGetExtensionNext(gif, &ext);
               }
          }
   } while (rec != TERMINATE_RECORD_TYPE);

   if (a >= 0) prop->alpha = 1;

   if ((gif->ImageCount > 1) || (image_count > 1))
     {
        animated->animated = 1;
        animated->loop_count = loop_count;
        animated->loop_hint = EVAS_IMAGE_ANIMATED_HINT_LOOP;
        animated->frame_count = MIN(gif->ImageCount, image_count);
        animated->frames = NULL;
     }

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 on_error:
   if (gif) DGifCloseFile(gif);
   if (egi.map) eina_file_map_free(f, egi.map);
   return r;
}

static Eina_Bool
evas_image_load_specific_frame(Eina_File *f,
                               const Evas_Image_Load_Opts *opts,
                               Evas_Image_Property *prop,
                               Evas_Image_Animated *animated, int frame_index,
                               int *error)
{
   GifFileType       *gif = NULL;
   Image_Entry_Frame *frame = NULL;
   Gif_Frame         *gif_frame = NULL;
   Evas_GIF_Info      egi;
   Eina_Bool          r = EINA_FALSE;

   egi.map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!egi.map)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }
   egi.length = eina_file_size_get(f);
   egi.position = 0;

#if GIFLIB_MAJOR >= 5
   gif = DGifOpen(&egi, _evas_image_load_file_read, NULL);
#else
   gif = DGifOpen(&egi, _evas_image_load_file_read);
#endif
   if (!gif)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }
   if (!_evas_image_skip_frame(gif, frame_index-1))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   frame = malloc(sizeof (Image_Entry_Frame));
   if (!frame)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error;
     }

   gif_frame = malloc(sizeof (Gif_Frame));
   if (!gif_frame)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error;
     }
   frame->info = gif_frame;
   frame->index = frame_index;
   if (!_evas_image_load_frame(f, opts, prop, animated, gif, frame, LOAD_FRAME_DATA_INFO, error))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   animated->frames = eina_list_append(animated->frames, frame);
   r = EINA_TRUE;

 on_error:
   if (gif) DGifCloseFile(gif);
   if (egi.map) eina_file_map_free(f, egi.map);
   return r;
}

static Eina_Bool
evas_image_load_file_data_gif(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels, int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Load_Opts *opts;
   Evas_Image_Animated *animated;
   Eina_File *f;

   Image_Entry_Frame *frame = NULL;
   int cur_frame_index;
   Eina_Bool hit;

   opts = loader->opts;
   animated = loader->animated;
   f = loader->f;

   if(!animated->animated)
     cur_frame_index = 1;
   else
     cur_frame_index = animated->cur_frame;

   if ((animated->animated) &&
       ((cur_frame_index < 0) || (cur_frame_index > FRAME_MAX) || (cur_frame_index > animated->frame_count)))
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

   /* first time frame is set to be 0. so default is 1 */
   if (cur_frame_index == 0) cur_frame_index++;

   /* Check current frame exists in hash table */
   hit = _find_frame(animated, cur_frame_index, &frame);

   /* if current frame exist in has table, check load flag */
   if (hit)
     {
        if (frame->loaded)
	  {
             evas_image_load_file_data_gif_internal(prop, frame, pixels, error);
	  }
        else
          {
             Evas_GIF_Info egi;
             GifFileType  *gif = NULL;
             Eina_Bool     r = EINA_FALSE;

             egi.map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
             if (!egi.map)
               {
                  *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
                  goto on_error;
               }
             egi.length = eina_file_size_get(f);
             egi.position = 0;

#if GIFLIB_MAJOR >= 5
	     gif = DGifOpen(&egi, _evas_image_load_file_read, NULL);
#else
             gif = DGifOpen(&egi, _evas_image_load_file_read);
#endif
             if (!gif)
               {
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  goto on_error;
               }
             _evas_image_skip_frame(gif, cur_frame_index - 1);
             if (!_evas_image_load_frame(f, opts, prop, animated, gif, frame, LOAD_FRAME_DATA, error))
               {
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  goto on_error;
               }
             if (!evas_image_load_file_data_gif_internal(prop, frame, pixels, error))
               {
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  goto on_error;
               }
             *error = EVAS_LOAD_ERROR_NONE;
             r = EINA_TRUE;

          on_error:
             if (gif) DGifCloseFile(gif);
             if (egi.map) eina_file_map_free(f, egi.map);
             return r;
          }
     }
   /* current frame does is not exist */
   else
     {
        if (!evas_image_load_specific_frame(f, opts, prop, animated, cur_frame_index, error))
          return EINA_FALSE;
        hit = EINA_FALSE;
        frame = NULL;
        hit = _find_frame(animated, cur_frame_index, &frame);
        if (!hit) return EINA_FALSE;
        if (!evas_image_load_file_data_gif_internal(prop, frame, pixels, error))
          {
             *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             return EINA_FALSE;
          }
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static double
evas_image_load_frame_duration_gif(void *loader_data,
				   int start_frame, int frame_num)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Animated *animated;
   Eina_File *f;

   Evas_GIF_Info  egi;
   GifFileType   *gif = NULL;
   GifRecordType  rec;
   int            current_frame = 1;
   int            remain_frames = frame_num;
   double         duration = -1;
   int            frame_count = 0;

   animated = loader->animated;
   f = loader->f;
   frame_count = animated->frame_count;

   if (!animated->animated) return -1;
   if ((start_frame + frame_num) > frame_count) return -1;
   if (frame_num < 0) return -1;

   egi.map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!egi.map) goto on_error;
   egi.length = eina_file_size_get(f);
   egi.position = 0;        
#if GIFLIB_MAJOR >= 5
   gif = DGifOpen(&egi, _evas_image_load_file_read, NULL);
#else
   gif = DGifOpen(&egi, _evas_image_load_file_read);
#endif
   if (!gif) goto on_error;

   duration = 0;
   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR)
          {
             rec = TERMINATE_RECORD_TYPE;
          }
        if (rec == IMAGE_DESC_RECORD_TYPE)
          {
             int                 img_code;
             GifByteType        *img;

             if (DGifGetImageDesc(gif) == GIF_ERROR)
               {
                  /* PrintGifError(); */
                  rec = TERMINATE_RECORD_TYPE;
               }
             current_frame++;
             /* we have to count frame, so use DGifGetCode and skip decoding */
             if (DGifGetCode(gif, &img_code, &img) == GIF_ERROR)
               {
                  rec = TERMINATE_RECORD_TYPE;
               }
             while (img)
               {
                  img = NULL;
                  DGifGetExtensionNext(gif, &img);
               }
          }
        else if (rec == EXTENSION_RECORD_TYPE)
          {
             int                 ext_code;
             GifByteType        *ext;

             ext = NULL;
             DGifGetExtension(gif, &ext_code, &ext);
             while (ext)
               {
                  if (ext_code == 0xf9) /* Graphic Control Extension */
                    {
                       if ((current_frame  >= start_frame) && (current_frame <= frame_count))
                         {
                            int frame_duration = 0;
                            if (remain_frames < 0) break;
                            frame_duration = byte2_to_int (ext[2], ext[3]);
                            if (frame_duration == 0)
                              duration += 0.1;
                            else
                              duration += (double)frame_duration/100;
                            remain_frames --;
                         }
                    }
                  ext = NULL;
                  DGifGetExtensionNext(gif, &ext);
               }
         }
     } while (rec != TERMINATE_RECORD_TYPE);

 on_error:
   if (gif) DGifCloseFile(gif);
   if (egi.map) eina_file_map_free(f, egi.map);
   return duration;
}

static Evas_Image_Load_Func evas_image_load_gif_func =
{
  evas_image_load_file_open_gif,
  evas_image_load_file_close_gif,
  evas_image_load_file_head_gif, 
  evas_image_load_file_data_gif,
  evas_image_load_frame_duration_gif,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_gif_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
  EVAS_MODULE_API_VERSION,
  "gif",
  "none",
  {
    module_open,
    module_close
  }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, gif);

#ifndef EVAS_STATIC_BUILD_GIF
EVAS_EINA_MODULE_DEFINE(image_loader, gif);
#endif
