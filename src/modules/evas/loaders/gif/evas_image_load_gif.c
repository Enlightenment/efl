#include "evas_common_private.h"
#include "evas_private.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <gif_lib.h>

typedef struct _Frame_Info Frame_Info;
typedef struct _Loader_Info Loader_Info;
typedef struct _File_Info File_Info;

struct _File_Info
{
   unsigned char *map;
   int pos, len; // yes - gif uses ints for file sizes.
};

struct _Loader_Info
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
   Evas_Image_Animated *animated;
   GifFileType *gif;
   int imgnum;
   File_Info fi;
};

struct _Frame_Info
{
   int x, y, w, h;
   unsigned short delay; // delay time in 1/100ths of a sec
   short transparent : 10; // -1 == not, anything else == index 
   short dispose : 6; // 0, 1, 2, 3 (others invalid)
   short interlace : 1; // interlaced or not
};

#ifndef MIN
# define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#define LOADERR(x) \
do { \
   *error = (x); \
   goto on_error; \
} while (0)
#define PIX(_x, _y) rows[yin + _y][xin + _x]
#define CMAP(_v) cmap->Colors[_v]
#define PIXLK(_p) ARGB_JOIN(0xff, CMAP(_p).Red, CMAP(_p).Green, CMAP(_p).Blue)

// utility funcs...

// brute force find frame index - gifs are normally saml so ok for now
static Image_Entry_Frame *
_find_frame(Evas_Image_Animated *animated, int index)
{
   Eina_List *l;
   Image_Entry_Frame *frame;

   if (!animated->frames) return NULL;
   EINA_LIST_FOREACH(animated->frames, l, frame)
     {
        if (frame->index == index) return frame;
     }
   return NULL;
}

// fill in am image with a specific rgba color value
static void
_fill_image(DATA32 *data, int rowpix, DATA32 val, int x, int y, int w, int h)
{
   int xx, yy;
   DATA32 *p;
   
   for (yy = 0; yy < h; yy++)
     {
        p = data + ((y + yy) * rowpix) + x;
        for (xx = 0; xx < w; xx++)
          {
             *p = val;
             p++;
          }
     }
}

// fix coords and work out an x and y inset in orig data if out of image bounds
static void
_clip_coords(int imw, int imh, int *xin, int *yin,
             int x0, int y0, int w0, int h0,
             int *x, int *y, int *w, int *h)
{
   if (x0 < 0)
     {
        w0 += x0;
        *xin = -x0;
        x0 = 0;
     }
   if ((x0 + w0) > imw) w0 = imw - x0;
   if (y0 < 0)
     {
        h0 += y0;
        *yin = -y0;
        y0 = 0;
     }
   if ((y0 + h0) > imh) h0 = imh - y0;
   *x = x0;
   *y = y0;
   *w = w0;
   *h = h0;
}

// file a rgba data pixle blob with a frame color (bg or trans) depending...
static void
_fill_frame(DATA32 *data, int rowpix, GifFileType *gif, Frame_Info *finfo,
            int x, int y, int w, int h)
{
   // solid color fill for pre frame region
   if (finfo->transparent < 0)
     {
        ColorMapObject *cmap;
        int bg;
        
        // work out color to use from cmap
        if (gif->Image.ColorMap) cmap = gif->Image.ColorMap;
        else cmap = gif->SColorMap;
        bg = gif->SBackGroundColor;
        // and do the fill
        _fill_image
          (data, rowpix,
           ARGB_JOIN(0xff, CMAP(bg).Red, CMAP(bg).Green, CMAP(bg).Blue),
           x, y, w, h);
     }
   // fill in region with 0 (transparent)
   else
     _fill_image(data, rowpix, 0, x, y, w, h);
}

// store common fields from gif file info into frame info
static void
_store_frame_info(GifFileType *gif, Frame_Info *finfo)
{
   finfo->x = gif->Image.Left;
   finfo->y = gif->Image.Top;
   finfo->w = gif->Image.Width;
   finfo->h = gif->Image.Height;
   finfo->interlace = gif->Image.Interlace;
}

// check if image fills "screen space" and if so, if it is transparent
// at all then the image could be transparent - OR if image doesnt fill,
// then it could be trasnparent (full coverage of screen). some gifs will
// be recognized as solid here for faster rendering, but not all.
static void
_check_transparency(Eina_Bool *full, Frame_Info *finfo, int w, int h)
{
   if ((finfo->x == 0) && (finfo->y == 0) &&
       (finfo->w == w) && (finfo->h == h))
     {
        if (finfo->transparent >= 0) *full = EINA_FALSE;
     }
   else *full = EINA_FALSE;
}

// allocate frame and frame info and append to list and store fields
static Frame_Info *
_new_frame(Evas_Image_Animated *animated,
           int transparent, int dispose, int delay,
           int index)
{
   Image_Entry_Frame *frame;
   Frame_Info *finfo;

   // allocate frame and frame info data (MUSt be separate)
   frame = calloc(1, sizeof(Image_Entry_Frame));
   if (!frame) return NULL;
   finfo = calloc(1, sizeof(Frame_Info));
   if (!finfo)
     {
        free(frame);
        return NULL;
     }
   // record transparent index to be used or -1 if none
   // for this SPECIFIC frame
   finfo->transparent = transparent;
   // record dispose mode (3 bits)
   finfo->dispose = dispose;
   // record delay (2 bytes so max 65546 /100 sec)
   finfo->delay = delay;
   // record the index number we are at
   frame->index = index;
   // that frame is stored AT image/screen size
   frame->info = finfo;
   animated->frames = eina_list_append(animated->frames, frame);
   return finfo;
}

// decode a gif image into rows then expand to 32bit into the destination
// data pointer
static Eina_Bool
_decode_image(GifFileType *gif, DATA32 *data, int rowpix, int xin, int yin,
              int transparent, int x, int y, int w, int h, Eina_Bool fill)
{
   int intoffset[] = { 0, 4, 2, 1 };
   int intjump[] = { 8, 8, 4, 2 };
   int i, xx, yy, pix;
   GifRowType *rows;
   Eina_Bool ret = EINA_FALSE;
   ColorMapObject *cmap;
   DATA32 *p;

   // build a blob of memory to have pointers to rows of pixels
   // AND store the decoded gif pixels (1 byte per pixel) as welll
   rows = malloc((h * sizeof(GifRowType *)) + (w * h * sizeof(GifPixelType)));
   if (!rows) goto on_error;
   
   // fill in the pointers at the start
   for (yy = 0; yy < h; yy++)
     {
        rows[yy] = ((unsigned char *)rows) + (h * sizeof(GifRowType *)) +
          (yy * w * sizeof(GifPixelType));
     }
   
   // if give is interlaced, walk interlace pattern and decode into rows
   if (gif->Image.Interlace)
     {
        for (i = 0; i < 4; i++)
          {
             for (yy = intoffset[i]; yy < h; yy += intjump[i])
               {
                  if (DGifGetLine(gif, rows[yy], w) != GIF_OK)
                    goto on_error;
               }
          }
     }
   // normal top to bottom - decode into rows
   else
     {
        for (yy = 0; yy < h; yy++)
          {
             if (DGifGetLine(gif, rows[yy], w) != GIF_OK)
               goto on_error;
          }
     }

   // work out what colormap to use
   if (gif->Image.ColorMap) cmap = gif->Image.ColorMap;
   else cmap = gif->SColorMap;

   // if we need to deal with transparent pixels at all...
   if (transparent >= 0)
     {
        // if we are told to FILL (overwrite with transparency kept)
        if (fill)
          {
             for (yy = 0; yy < h; yy++)
               {
                  p = data + ((y + yy) * rowpix) + x;
                  for (xx = 0; xx < w; xx++)
                    {
                       pix = PIX(xx, yy);
                       if (pix != transparent) *p = PIXLK(pix);
                       else *p = 0;
                       p++;
                    }
               }
          }
        // paste on top with transparent pixels untouched
        else
          {
             for (yy = 0; yy < h; yy++)
               {
                  p = data + ((y + yy) * rowpix) + x;
                  for (xx = 0; xx < w; xx++)
                    {
                       pix = PIX(xx, yy);
                       if (pix != transparent) *p = PIXLK(pix);
                       p++;
                    }
               }
          }
     }
   else
     {
        // walk pixels without worring about transparency at all
        for (yy = 0; yy < h; yy++)
          {
             p = data + ((y + yy) * rowpix) + x;
             for (xx = 0; xx < w; xx++)
               {
                  pix = PIX(xx, yy);
                  *p = PIXLK(pix);
                  p++;
               }
          }
     }
   ret = EINA_TRUE;

on_error:
   free(rows);
   return ret;
}

// flush out older rgba frame images to save memory but skip current frame
// and previous frame (prev needed for dispose mode 3)
static void
_flush_older_frames(Evas_Image_Animated *animated,
                    int w, int h,
                    Image_Entry_Frame *thisframe,
                    Image_Entry_Frame *prevframe)
{
   Eina_List *l;
   Image_Entry_Frame *frame;
   // target is the amount of memory we want to be under for stored frames
   int total = 0, target = 512 * 1024;

   // total up the amount of memory used by stored frames for this image
   EINA_LIST_FOREACH(animated->frames, l, frame)
     {
        if (frame->data) total++;
     }
   total *= (w * h * sizeof(DATA32));
   // if we use less than target (512k) for frames - dont flush
   if (total < target) return;
   // clean oldest frames first and go until below target or until we loop
   // around back to this frame (curent)
   EINA_LIST_FOREACH(animated->frames, l, frame)
     {
        if (frame == thisframe) break;
     }
   if (!l) return;
   // start on next frame after thisframe
   l = l->next;
   // handle wrap to start
   if (!l) l = animated->frames;
   // now walk until we hit thisframe again... then stop walk.
   while (l)
     {
        frame = l->data;
        if (frame == thisframe) break;
        if (frame->data)
          {
             if ((frame != thisframe) && (frame != prevframe))
               {
                  free(frame->data);
                  frame->data = NULL;
                  // subtract memory used and if below target - stop flush
                  total -= (w * h * sizeof(DATA32));
                  if (total < target) break;
               }
          }
        // go to next - handle wrap to start
        l = l->next;
        if (!l) l = animated->frames;
     }
}
                    
static int
_file_read(GifFileType *gft, GifByteType *buf, int len)
{
   File_Info *fi = gft->UserData;

   if (fi->pos >= fi->len) return 0; // if at or past end - no
   if ((fi->pos + len) >= fi->len) len = fi->len - fi->pos; 
   memcpy(buf, fi->map + fi->pos, len);
   fi->pos += len;
   return len;
}

static Eina_Bool
evas_image_load_file_head_gif2(void *loader_data,
                               Evas_Image_Property *prop,
                               int *error)
{
   Loader_Info *loader = loader_data;
   Evas_Image_Animated *animated = loader->animated;
   Eina_File *f = loader->f;
   Eina_Bool ret = EINA_FALSE;
   File_Info fi;
   GifRecordType rec;
   GifFileType *gif = NULL;
   // it is possible which gif file have error midle of frames,
   // in that case we should play gif file until meet error frame.
   int imgnum = 0;
   int loop_count = -1;
   Frame_Info *finfo = NULL;
   Eina_Bool full = EINA_TRUE;

   // init prop struct with some default null values
   prop->w = 0;
   prop->h = 0;

   // map the file and store/track info
   fi.map = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!fi.map) LOADERR(EVAS_LOAD_ERROR_CORRUPT_FILE);
   fi.len = eina_file_size_get(f);
   fi.pos = 0;

   // actually ask libgif to open the file
#if GIFLIB_MAJOR >= 5
   gif = DGifOpen(&fi, _file_read, NULL);
#else
   gif = DGifOpen(&fi, _file_read);
#endif
   if (!gif) LOADERR(EVAS_LOAD_ERROR_UNKNOWN_FORMAT);
   // get the gif "screen size" (the actual image size)
   prop->w = gif->SWidth;
   prop->h = gif->SHeight;
   // if size is invalid - abort here
   if ((prop->w < 1) || (prop->h < 1) ||
       (prop->w > IMG_MAX_SIZE) || (prop->h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(prop->w, prop->h))
     {
        if (IMG_TOO_BIG(prop->w, prop->h))
          LOADERR(EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED);
        LOADERR(EVAS_LOAD_ERROR_GENERIC);
     }
   // walk through gif records in file to figure out info
   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR)
          {
             // if we have a gif that ends part way through a sequence
             // (or animation) consider it valid and just break - no error
             if (imgnum > 1) break;
             LOADERR(EVAS_LOAD_ERROR_UNKNOWN_FORMAT);
          }
        // get image description section
        if (rec == IMAGE_DESC_RECORD_TYPE)
          {
             int img_code;
             GifByteType *img;

             // get image desc
             if (DGifGetImageDesc(gif) == GIF_ERROR)
               LOADERR(EVAS_LOAD_ERROR_UNKNOWN_FORMAT);
             // skip decoding and just walk image to next
             if (DGifGetCode(gif, &img_code, &img) == GIF_ERROR)
               LOADERR(EVAS_LOAD_ERROR_UNKNOWN_FORMAT);
             // skip till next...
             while (img)
               {
                  img = NULL;
                  DGifGetCodeNext(gif, &img);
               }
             // store geometry in the last frame info data
             if (finfo)
               {
                  _store_frame_info(gif, finfo);
                  _check_transparency(&full, finfo, prop->w, prop->h);
               }
             // or if we dont have a finfo entry - create one even for stills
             else
               {
                  // allocate and save frame with field data
                  finfo = _new_frame(animated, -1, 0, 0, imgnum + 1);
                  if (!finfo)
                    LOADERR(EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED);
                  // store geometry info from gif image
                  _store_frame_info(gif, finfo);
                  // check for transparency/alpha
                  _check_transparency(&full, finfo, prop->w, prop->h);
               }
             imgnum++;
          }
        // we have an extension code block - for animated gifs for sure
        else if (rec == EXTENSION_RECORD_TYPE)
          {
             int ext_code;
             GifByteType *ext;

             ext = NULL;
             // get the first extension entry
             DGifGetExtension(gif, &ext_code, &ext);
             while (ext)
               {
                  // graphic control extension - for animated gif data
                  // and transparent index + flag
                  if (ext_code == 0xf9)
                    {
                       // create frame and store it in image
                       finfo = _new_frame
                         (animated,
                          (ext[1] & 1) ? ext[4] : -1, // transparency index
                          (ext[1] >> 2) & 0x7, // dispose mode
                          ((int)ext[3] << 8) | (int)ext[2], // delay
                          imgnum + 1);
                       if (!finfo)
                         LOADERR(EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED);
                    }
                  // netscape extension indicating loop count...
                  else if (ext_code == 0xff) /* application extension */
                    {
                       if (!strncmp((char *)(&ext[1]), "NETSCAPE2.0", 11) ||
                           !strncmp((char *)(&ext[1]), "ANIMEXTS1.0", 11))
                         {
                            ext = NULL;
                            DGifGetExtensionNext(gif, &ext);
                            if (ext[1] == 0x01)
                              {
                                 loop_count = ((int)ext[3] << 8) | (int)ext[2];
                                 if (loop_count > 0) loop_count++;
                              }
                         }
                    }
                  // and continue onto the next extension entry
                  ext = NULL;
                  DGifGetExtensionNext(gif, &ext);
               }
          }
     }
   while (rec != TERMINATE_RECORD_TYPE);

   // if the gif main says we have more than one image or our image counting
   // says so, then this image is animated - indicate this
   if ((gif->ImageCount > 1) || (imgnum > 1))
     {
        animated->animated = 1;
        animated->loop_count = loop_count;
        animated->loop_hint = EVAS_IMAGE_ANIMATED_HINT_LOOP;
        animated->frame_count = MIN(gif->ImageCount, imgnum);
     }
   if (!full) prop->alpha = 1;
   animated->cur_frame = 1;

   // no errors in header scan etc. so set err and return value
   *error = EVAS_LOAD_ERROR_NONE;
   ret = EINA_TRUE;

on_error: // jump here on any errors to clean up
   if (gif) DGifCloseFile(gif);
   if (fi.map) eina_file_map_free(f, fi.map);
   return ret;
}

static Eina_Bool
evas_image_load_file_data_gif2(void *loader_data,
                               Evas_Image_Property *prop,
                               void *pixels,
                               int *error)
{
   Loader_Info *loader = loader_data;
   Evas_Image_Animated *animated = loader->animated;
   Eina_File *f = loader->f;
   Eina_Bool ret = EINA_FALSE;
   GifRecordType rec;
   GifFileType *gif = NULL;
   Image_Entry_Frame *frame;
   int index = 0, imgnum = 0;
   Frame_Info *finfo;

   // XXX: this is so wrong - storing current frame IN the image
   // so we have to load multiple times to animate. what if the
   // same image is shared/loaded in 2 ore more places AND animated
   // there?
   
   // use index stored in image (XXX: yuk!)
   index = animated->cur_frame;
   // if index is invalid for animated image - error out
   if ((animated->animated) &&
       ((index <= 0) || (index > animated->frame_count)))
     LOADERR(EVAS_LOAD_ERROR_GENERIC);
   // find the given frame index
   frame = _find_frame(animated, index);
   if (frame)
     {
        if ((frame->loaded) && (frame->data))
          {
             // frame is already there and decoded - jump to end
             goto on_ok;
          }
     }
   else
     LOADERR(EVAS_LOAD_ERROR_CORRUPT_FILE);

open_file:
   // actually ask libgif to open the file
   gif = loader->gif;
   if (!gif)
     {
        // there was no file previously opened
        // map the file and store/track info
        loader->fi.map = eina_file_map_all(f, EINA_FILE_RANDOM);
        if (!loader->fi.map) LOADERR(EVAS_LOAD_ERROR_CORRUPT_FILE);
        loader->fi.len = eina_file_size_get(f);
        loader->fi.pos = 0;

#if GIFLIB_MAJOR >= 5
        gif = DGifOpen(&(loader->fi), _file_read, NULL);
#else
        gif = DGifOpen(&(loader->fi), _file_read);
#endif
        // if gif open failed... get out of here
        if (!gif)
          {
             if ((loader->fi.map) && (loader->f))
               eina_file_map_free(loader->f, loader->fi.map);
             loader->fi.map = NULL;
             LOADERR(EVAS_LOAD_ERROR_UNKNOWN_FORMAT);
          }
        loader->gif = gif;
        loader->imgnum = 1;
     }

   // if we want to go backwards, we likely need/want to re-decode from the
   // start as we have nothnig to build on
   if ((index > 0) && (index < loader->imgnum) && (animated->animated))
     {
        if (loader->gif) DGifCloseFile(loader->gif);
        if ((loader->fi.map) && (loader->f))
          eina_file_map_free(loader->f, loader->fi.map);
        loader->gif = NULL;
        loader->fi.map = NULL;
        loader->imgnum = 0;
        goto open_file;
     }

   // our current position is the previous frame we decoded from the file
   imgnum = loader->imgnum;

   // walk through gif records in file to figure out info
   do
     {
        if (DGifGetRecordType(gif, &rec) == GIF_ERROR)
          LOADERR(EVAS_LOAD_ERROR_UNKNOWN_FORMAT);
        if (rec == EXTENSION_RECORD_TYPE)
          {
             int                 ext_code;
             GifByteType        *ext;
             
             ext = NULL;
             DGifGetExtension(gif, &ext_code, &ext);
             while (ext)
               {
                  ext = NULL;
                  DGifGetExtensionNext(gif, &ext);
               }
          }
        // get image description section
        else if (rec == IMAGE_DESC_RECORD_TYPE)
          {
             int xin = 0, yin = 0, x = 0, y = 0, w = 0, h = 0;
             int img_code;
             GifByteType *img;
             Image_Entry_Frame *prevframe = NULL;
             Image_Entry_Frame *thisframe = NULL;

             // get image desc
             if (DGifGetImageDesc(gif) == GIF_ERROR)
               LOADERR(EVAS_LOAD_ERROR_UNKNOWN_FORMAT);
             // get the previous frame entry AND the current one to fill in
             prevframe = _find_frame(animated, imgnum - 1);
             thisframe = _find_frame(animated, imgnum);
             // if we have a frame AND we're animated AND we have no data...
             if ((thisframe) && (!thisframe->data) && (animated->animated))
               {
                  Eina_Bool first = EINA_FALSE;

                  // allocate it
                  thisframe->data =
                    malloc(prop->w * prop->h * sizeof(DATA32));
                  if (!thisframe->data)
                    LOADERR(EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED);
                  // if we have no prior frame OR prior frame data... empty
                  if ((!prevframe) || (!prevframe->data))
                    {
                       first = EINA_TRUE;
                       finfo = thisframe->info;
                       memset(thisframe->data, 0,
                              prop->w * prop->h * sizeof(DATA32));
                    }
                  // we have a prior frame to copy data from...
                  else
                    {
                       finfo = prevframe->info;

                       // fix coords of sub image in case it goes out...
                       _clip_coords(prop->w, prop->h, &xin, &yin,
                                    finfo->x, finfo->y, finfo->w, finfo->h,
                                    &x, &y, &w, &h);
                       // if dispose mode is not restore - then copy pre frame
                       if (finfo->dispose != 3) // GIF_DISPOSE_RESTORE
                         memcpy(thisframe->data, prevframe->data,
                              prop->w * prop->h * sizeof(DATA32));
                       // if dispose mode is "background" then fill with bg
                       if (finfo->dispose == 2) // GIF_DISPOSE_BACKGND
                         _fill_frame(thisframe->data, prop->w, gif,
                                     finfo, x, y, w, h);
                       else if (finfo->dispose == 3) // GIF_DISPOSE_RESTORE
                         {
                            Image_Entry_Frame *prevframe2;
                            
                            // we need to copy data from one frame back
                            // from the prev frame into the current frame
                            // (copy the whole image - at least the sample
                            // GifWin.cpp from libgif indicates this is what
                            // needs doing
                            prevframe2 = _find_frame(animated, imgnum - 2);
                            if (prevframe2)
                              memcpy(thisframe->data, prevframe2->data,
                                     prop->w * prop->h * sizeof(DATA32));
                         }
                    }
                  // now draw this frame on top
                  finfo = thisframe->info;
                  _clip_coords(prop->w, prop->h, &xin, &yin,
                               finfo->x, finfo->y, finfo->w, finfo->h,
                               &x, &y, &w, &h);
                  if (!_decode_image(gif, thisframe->data, prop->w,
                                     xin, yin, finfo->transparent,
                                     x, y, w, h, first))
                    LOADERR(EVAS_LOAD_ERROR_CORRUPT_FILE);
                  // mark as loaded and done
                  thisframe->loaded = EINA_TRUE;
                  // and flush old memory if needed (too much)
                  _flush_older_frames(animated, prop->w, prop->h,
                                      thisframe, prevframe);
               }
             // if we hve a frame BUT the image is not animated... different
             // path
             else if ((thisframe) && (!thisframe->data) && 
                      (!animated->animated))
               {
                  // if we don't have the data decoded yet - decode it
                  if ((!thisframe->loaded) || (!thisframe->data))
                    {
                       // use frame info but we WONT allocate frame pixels
                       finfo = thisframe->info;
                       _clip_coords(prop->w, prop->h, &xin, &yin,
                                    finfo->x, finfo->y, finfo->w, finfo->h,
                                    &x, &y, &w, &h);
                       // clear out all pixels
                       _fill_frame(pixels, prop->w, gif,
                                   finfo, 0, 0, prop->w, prop->h);
                       // and decode the gif with overwriting
                       if (!_decode_image(gif, pixels, prop->w,
                                          xin, yin, finfo->transparent,
                                          x, y, w, h, EINA_TRUE))
                         LOADERR(EVAS_LOAD_ERROR_CORRUPT_FILE);
                       // mark as loaded and done
                       thisframe->loaded = EINA_TRUE;
                    }
                  // flush mem we don't need (at expense of decode cpu)
               }
             else
               {
                  // skip decoding and just walk image to next
                  if (DGifGetCode(gif, &img_code, &img) == GIF_ERROR)
                    LOADERR(EVAS_LOAD_ERROR_UNKNOWN_FORMAT);
                  while (img)
                    {
                       img = NULL;
                       DGifGetCodeNext(gif, &img);
                    }
               }
             // if we found the image we wanted - get out of here
             if (imgnum >= index) break;
             imgnum++;
          }
     }
   while (rec != TERMINATE_RECORD_TYPE);

   // if we are at the end of the animation or not animated, close file
   loader->imgnum = imgnum;
   if ((animated->frame_count <= 1) || (rec == TERMINATE_RECORD_TYPE))
     {
        if (loader->gif) DGifCloseFile(loader->gif);
        if ((loader->fi.map) && (loader->f))
          eina_file_map_free(loader->f, loader->fi.map);
        loader->gif = NULL;
        loader->fi.map = NULL;
        loader->imgnum = 0;
     }
   
on_ok:   
   // no errors in header scan etc. so set err and return value
   *error = EVAS_LOAD_ERROR_NONE;
   ret = EINA_TRUE;
   
   // if it was an animated image we need to copy the data to the
   // pixels for the image from the frame holding the data
   if (animated->animated && frame->data)
     memcpy(pixels, frame->data, prop->w * prop->h * sizeof(DATA32));
   prop->premul = EINA_TRUE;
   
on_error: // jump here on any errors to clean up
   return ret;
}

// get the time between 2 frames in the timeline
static double
evas_image_load_frame_duration_gif2(void *loader_data,
                                    int start_frame,
                                    int frame_num)
{
   Loader_Info *loader = loader_data;
   Evas_Image_Animated *animated = loader->animated;
   Image_Entry_Frame *frame;
   int i, total = 0;

   // if its not animated or requested frame data is invalid
   if (!animated->animated) return -1.0;
   if ((start_frame + frame_num) > animated->frame_count) return -1.0;
   if (frame_num < 0) return -1.0;
   
   if (frame_num < 1) frame_num = 1;
   // walk frames from start frame though and total up delays
   for (i = start_frame; i < (start_frame + frame_num); i++)
     {
        Frame_Info *finfo;
        
        // find the frame
        frame = _find_frame(animated, i);
        // no frame? barf - bad file or i/o?
        if (!frame) return -1.0;
        // get delay and total it up
        finfo = frame->info;
        // if delay is sensible - use it else assume 10/100ths of a sec
        if (finfo->delay > 0) total += finfo->delay;
        else total += 10;
     }
   // return delay in seconds (since timing in gifs is in 1/100ths of a sec)
   return (double)total / 100.0;
}

// called on opening of a file load
static void *
evas_image_load_file_open_gif2(Eina_File *f,
                               Eina_Stringshare *key EINA_UNUSED, // XXX: we need to use key for frame #
                               Evas_Image_Load_Opts *opts,
                               Evas_Image_Animated *animated,
                               int *error)
{
   Loader_Info *loader = calloc(1, sizeof (Loader_Info));
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

// called on closing of an image file load (end of load)
static void
evas_image_load_file_close_gif2(void *loader_data)
{
   Loader_Info *loader = loader_data;
   if (loader->gif) DGifCloseFile(loader->gif);
   if ((loader->fi.map) && (loader->f))
     eina_file_map_free(loader->f, loader->fi.map);
   free(loader);
}

// general module delcaration stuff
static Evas_Image_Load_Func evas_image_load_gif_func =
{
  evas_image_load_file_open_gif2,
  evas_image_load_file_close_gif2,
  evas_image_load_file_head_gif2, 
  evas_image_load_file_data_gif2,
  evas_image_load_frame_duration_gif2,
  EINA_TRUE,
  EINA_FALSE
};

// raw module api that the rest of the world sees
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
