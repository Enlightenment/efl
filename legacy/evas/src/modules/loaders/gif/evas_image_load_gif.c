#include "evas_common.h"
#include "evas_private.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <gif_lib.h>

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
};

static Eina_Bool evas_image_load_file_data_gif_internal(Image_Entry *ie, Image_Entry_Frame *frame, int *error);

static Eina_Bool evas_image_load_file_head_gif(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_gif(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static double evas_image_load_frame_duration_gif(Image_Entry *ie, const char *file, int start_frame, int frame_num) ;
static Eina_Bool evas_image_load_specific_frame(Image_Entry *ie, const char *file, int frame_index, int *error);

static Evas_Image_Load_Func evas_image_load_gif_func =
{
  EINA_TRUE,
  evas_image_load_file_head_gif,
  evas_image_load_file_data_gif,
  evas_image_load_frame_duration_gif
};
#define byte2_to_int(a,b)         (((b)<<8)|(a))

#define FRAME_MAX 1024

/* find specific frame in image entry */
static Eina_Bool
_find_frame(Image_Entry *ie, int frame_index, Image_Entry_Frame **frame)
{
   Eina_List *l;
   Image_Entry_Frame *hit_frame = NULL;

   if (!ie) return EINA_FALSE;
   if (!ie->frames) return EINA_FALSE;

   EINA_LIST_FOREACH(ie->frames, l, hit_frame)
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
_find_close_frame(Image_Entry *ie, int frame_index, Image_Entry_Frame **frame)
{
  int i;
  Eina_Bool hit = EINA_FALSE;
  i = frame_index -1;

  if (!ie) return EINA_FALSE;
  if (!ie->frames) return EINA_FALSE;

  for (; i > 0; i--)
    {
       hit = _find_frame(ie, i, frame);
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
_evas_image_load_frame_image_data(Image_Entry *ie, GifFileType *gif, Image_Entry_Frame *frame, int *error)
{
   int                 w;
   int                 h;
   int                 x;
   int                 y;
   int                 i,j;
   int                 bg;
   int                 r;
   int                 g;
   int                 b;
   int                 alpha;
   double              per;
   double              per_inc;
   ColorMapObject     *cmap;
   GifRowType         *rows;
   int                 intoffset[] = { 0, 4, 2, 1 };
   int                 intjump[] = { 8, 8, 4, 2 };
   size_t              siz;
   int                 cache_w;
   int                 cache_h;
   int                 cur_h;
   int                 cur_w;
   DATA32             *ptr;
   Gif_Frame          *gif_frame = NULL;

   if ((!gif) || (!frame)) return EINA_FALSE;

   gif_frame = (Gif_Frame *) frame->info;
   w = gif->Image.Width;
   h = gif->Image.Height;
   x = gif->Image.Left;
   y = gif->Image.Top;
   cache_w = ie->w;
   cache_h = ie->h;

   rows = malloc(h * sizeof(GifRowType *));
   if (!rows)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
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
             for (i = 0; i < h; i++)
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
             if (DGifGetLine(gif, rows[i], w) != GIF_OK)
               {
                  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                  goto error;
              }
          }
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

   per_inc = 100.0 / (((double)w) * h);
   cur_h = h;
   cur_w = w;
   if (cur_h > cache_h) cur_h = cache_h;
   if (cur_w > cache_w) cur_w = cache_w;

   if (frame->index > 1)
     {
        /* get previous frame only frame index is bigger than 1 */
        DATA32            *ptr_src;
        Image_Entry_Frame *new_frame = NULL;
        int                cur_frame = frame->index;

        if (!_find_close_frame(ie, cur_frame,  &new_frame))
          {
             if (!evas_image_load_specific_frame(ie, ie->file, cur_frame-1, error))
               {
                  *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
                  goto error;
               }
          }
        else
          {
             ptr_src = new_frame->data;
             memcpy(ptr, ptr_src, siz);
          }

        /* composite frames */
        ptr = ptr + cache_w * y;

        for (i = 0; i < cur_h; i++)
          {
             ptr = ptr + x;
             for (j = 0; j < cur_w; j++)
               {
                  if (rows[i][j] == alpha)
                    {
                       ptr++ ;
                    }
                  else
                    {
                       r = cmap->Colors[rows[i][j]].Red;
                       g = cmap->Colors[rows[i][j]].Green;
                       b = cmap->Colors[rows[i][j]].Blue;
                       *ptr++ = ARGB_JOIN(0xff, r, g, b);
                    }
                  per += per_inc;
               }
             ptr = ptr + (cache_w - (x + cur_w));
          }
     }
   else
     {
        ptr = ptr + cache_w * y;

        for (i = 0; i < cur_h; i++)
          {
             ptr = ptr + x;
             for (j = 0; j < cur_w; j++)
               {
                  r = cmap->Colors[rows[i][j]].Red;
                  g = cmap->Colors[rows[i][j]].Green;
                  b = cmap->Colors[rows[i][j]].Blue;
                  *ptr++ = ARGB_JOIN(0xff, r, g, b);

                  per += per_inc;
               }
             ptr = ptr + (cache_w - (x + cur_w));
          }
     }

   for (i = 0; i < h; i++)
     {
        if (rows[i]) free(rows[i]);
     }
   if (rows) free(rows);
   frame->loaded = EINA_TRUE;
   return EINA_TRUE;
error:
   for (i = 0; i < h; i++)
     {
        if (rows[i]) free(rows[i]);
     }
   if (rows) free(rows);
   return EINA_FALSE;
}

static Eina_Bool
_evas_image_load_frame(Image_Entry *ie, GifFileType *gif, Image_Entry_Frame *frame, Frame_Load_Type type, int *error)
{
   GifRecordType       rec;
   int                 gra_res = 0, img_res = 0;
   Eina_Bool           res = EINA_FALSE;
   Gif_Frame          *gif_frame = NULL;

   if ((!gif) || (!frame)) return EINA_FALSE;
   gif_frame = (Gif_Frame *) frame->info;

   if (LOAD_FRAME_NONE > type || LOAD_FRAME_DATA_INFO < type) return EINA_FALSE;

   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR) return EINA_FALSE;
        if (rec == IMAGE_DESC_RECORD_TYPE)
          {
             img_res++;
             break;
          }
        else if (rec = EXTENSION_RECORD_TYPE)
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
        res = _evas_image_load_frame_image_data(ie, gif,frame, error);
        if (!res) return EINA_FALSE;
     }
   return EINA_TRUE;
}


/* set frame data to cache entry's data */
static Eina_Bool
evas_image_load_file_data_gif_internal(Image_Entry *ie, Image_Entry_Frame *frame, int *error)
{
   int        w;
   int        h;
   int        dst_x;
   int        dst_y;
   DATA32    *dst;
   DATA32    *src;
   int        cache_w, cache_h;
   size_t     siz;
   Gif_Frame *gif_frame = NULL;

   gif_frame = (Gif_Frame *) frame->info;
   cache_w = ie->w;
   cache_h = ie->h;
   w = gif_frame->image_des.w;
   h = gif_frame->image_des.h;
   dst_x = gif_frame->image_des.x;
   dst_y = gif_frame->image_des.y;

   src = frame->data;

   if (!evas_cache_image_pixels(ie))
     {
        evas_cache_image_surface_alloc(ie, cache_w, cache_h);
     }

   if (!evas_cache_image_pixels(ie))
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   /* only copy real frame part */
   siz = cache_w * cache_h *  sizeof(DATA32);
   dst = evas_cache_image_pixels(ie);

   memcpy(dst, src, siz);

   evas_common_image_premul(ie);

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_head_gif(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   int                 fd;
   GifFileType        *gif;
   GifRecordType       rec;
   int                 w;
   int                 h;
   int                 alpha;
   int                 loop_count = -1;

   w = 0;
   h = 0;
   alpha = -1;

#ifndef __EMX__
   fd = open(file, O_RDONLY);
#else
   fd = open(file, O_RDONLY | O_BINARY);
#endif
   if (fd < 0)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   gif = DGifOpenFileHandle(fd);
   if (!gif)
     {
        if (fd) close(fd);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

   /* check logical screen size */
   w = gif->SWidth;
   h = gif->SHeight;

   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
        DGifCloseFile(gif);
        if (IMG_TOO_BIG(w, h))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }
   ie->w = w;
   ie->h = h;

   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR)
          {
             /* PrintGifError(); */
             DGifCloseFile(gif);
             *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
             return EINA_FALSE;
          }

        /* image descript info */
        if (rec == IMAGE_DESC_RECORD_TYPE)
          {
             int img_code;
             GifByteType *img;

             if (DGifGetImageDesc(gif) == GIF_ERROR)
               {
                  /* PrintGifError(); */
                  DGifCloseFile(gif);
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  return EINA_FALSE;
               }
             /* we have to count frame, so use DGifGetCode and skip decoding */
             if (DGifGetCode(gif, &img_code, &img) == GIF_ERROR)
               {
                  /* PrintGifError(); */
                  DGifCloseFile(gif);
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  return EINA_FALSE;
               }
             while (img)
               {
                  img = NULL;
                  DGifGetCodeNext(gif, &img);
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
                       if ((ext[1] & 1) && (alpha < 0)) alpha = (int)ext[4];
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

   if (alpha >= 0) ie->flags.alpha = 1;

   if (gif->ImageCount > 1)
     {
        ie->flags.animated = 1;
        ie->loop_count = loop_count;
        ie->loop_hint = EVAS_IMAGE_ANIMATED_HINT_LOOP;
        ie->frame_count = gif->ImageCount;
        ie->frames = NULL;
     }

   DGifCloseFile(gif);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_specific_frame(Image_Entry *ie, const char *file, int frame_index, int *error)
{
   int                fd;
   GifFileType       *gif;
   Image_Entry_Frame *frame = NULL;
   Gif_Frame         *gif_frame = NULL;

#ifndef __EMX__
   fd = open(file, O_RDONLY);
#else
   fd = open(file, O_RDONLY | O_BINARY);
#endif
   if (fd < 0)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   gif = DGifOpenFileHandle(fd);
   if (!gif)
     {
        if (fd) close(fd);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }
   if (!_evas_image_skip_frame(gif, frame_index-1))
     {
        if (fd) close(fd);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

   frame = malloc(sizeof (Image_Entry_Frame));
   if (!frame)
     {
        if (fd) close(fd);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   gif_frame = malloc(sizeof (Gif_Frame));
   if (!gif_frame)
     {
        if (fd) close(fd);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }
   frame->info = gif_frame;
   frame->index = frame_index;
   if (!_evas_image_load_frame(ie,gif, frame, LOAD_FRAME_DATA_INFO,error))
     {
        if (fd) close(fd);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

   ie->frames = eina_list_append(ie->frames, frame);
   DGifCloseFile(gif);
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_gif(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   int                cur_frame_index;
   Image_Entry_Frame *frame = NULL;
   Eina_Bool          hit;

   if(!ie->flags.animated)
     cur_frame_index = 1;
   else
     cur_frame_index = ie->cur_frame;

   if ((ie->flags.animated) &&
       ((cur_frame_index <0) || (cur_frame_index > FRAME_MAX) || (cur_frame_index > ie->frame_count)))
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

   /* first time frame is set to be 0. so default is 1 */
   if (cur_frame_index == 0) cur_frame_index++;

   /* Check current frame exists in hash table */
   hit = _find_frame(ie, cur_frame_index, &frame);

   /* if current frame exist in has table, check load flag */
   if (hit)
     {
        if (frame->loaded)
          evas_image_load_file_data_gif_internal(ie,frame,error);
        else
          {
             int           fd;
             GifFileType  *gif;

#ifndef __EMX__
             fd = open(file, O_RDONLY);
#else
             fd = open(file, O_RDONLY | O_BINARY);
#endif
             if (fd < 0)
               {
                  *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
                  return EINA_FALSE;
               }

             gif = DGifOpenFileHandle(fd);
             if (!gif)
               {
                  if (fd) close(fd);
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  return EINA_FALSE;
               }
             _evas_image_skip_frame(gif, cur_frame_index-1);
             if (!_evas_image_load_frame(ie, gif, frame, LOAD_FRAME_DATA,error))
               {
                  if (fd) close(fd);
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  return EINA_FALSE;
               }
             if (!evas_image_load_file_data_gif_internal(ie, frame, error))
               {
                  if (fd) close(fd);
                  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                  return EINA_FALSE;
               }
             DGifCloseFile(gif);
             *error = EVAS_LOAD_ERROR_NONE;
             return EINA_TRUE;
          }
     }
   /* current frame does is not exist */
   else
     {
        if (!evas_image_load_specific_frame(ie, file, cur_frame_index, error))
          {
             return EINA_FALSE;
          }
        hit = EINA_FALSE;
        frame = NULL;
        hit = _find_frame(ie, cur_frame_index, &frame);
        if (!hit) return EINA_FALSE;
        if (!evas_image_load_file_data_gif_internal(ie, frame, error))
          {
             *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             return EINA_FALSE;
          }
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static double
evas_image_load_frame_duration_gif(Image_Entry *ie, const char *file, const int start_frame, const int frame_num)
{
   int                 fd;
   GifFileType        *gif;
   GifRecordType       rec;
   int                 done;
   int                 current_frame = 1;
   int                 remain_frames = frame_num;
   double              duration = 0;
   int                 frame_count = 0;

   frame_count = ie->frame_count;

   if (!ie->flags.animated) return -1;
   if ((start_frame + frame_num) > frame_count) return -1;
   if (frame_num < 0) return -1;

   done = 0;

#ifndef __EMX__
   fd = open(file, O_RDONLY);
#else
   fd = open(file, O_RDONLY | O_BINARY);
#endif
   if (fd < 0) return -1;

   gif = DGifOpenFileHandle(fd);
   if (!gif)
   {
      if (fd) close(fd);
      return -1;
   }

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

   DGifCloseFile(gif);
   return duration;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_gif_func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
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
