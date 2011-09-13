#include "ecore_xcb_private.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <xcb/xcb_event.h>
#include <xcb/shm.h>

struct _Ecore_X_Image 
{
   xcb_shm_segment_info_t shminfo;
   xcb_image_t *xim;
   Ecore_X_Visual vis;
   int depth, w, h;
   int bpl, bpp, rows;
   unsigned char *data;
   Eina_Bool shm : 1;
};

/* local function prototypes */
static void _ecore_xcb_image_shm_check(void);
static void _ecore_xcb_image_shm_create(Ecore_X_Image *im);
static xcb_format_t *_ecore_xcb_image_find_format(const xcb_setup_t *setup, uint8_t depth);

/* local variables */
static int _ecore_xcb_image_shm_can = -1;

EAPI Ecore_X_Image *
ecore_x_image_new(int w, int h, Ecore_X_Visual vis, int depth) 
{
   Ecore_X_Image *im;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(im = calloc(1, sizeof(Ecore_X_Image)))) return NULL;
   im->w = w;
   im->h = h;
   im->vis = vis;
   im->depth = depth;
   _ecore_xcb_image_shm_check();
   im->shm = _ecore_xcb_image_shm_can;
   return im;
}

EAPI void 
ecore_x_image_free(Ecore_X_Image *im) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!im) return;
   if (im->shm) 
     {
        if (im->xim) 
          {
             xcb_shm_detach(_ecore_xcb_conn, im->shminfo.shmseg);
             xcb_image_destroy(im->xim);
             shmdt(im->shminfo.shmaddr);
             shmctl(im->shminfo.shmid, IPC_RMID, 0);
          }
     }
   else if (im->xim) 
     {
        if (im->xim->data) free(im->xim->data);
        im->xim->data = NULL;
        xcb_image_destroy(im->xim);
     }

   free(im);
//   ecore_x_flush();
}

EAPI Eina_Bool 
ecore_x_image_get(Ecore_X_Image *im, Ecore_X_Drawable draw, int x, int y, int sx, int sy, int w, int h) 
{
   Eina_Bool ret = EINA_TRUE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (im->shm) 
     {
        if (!im->xim) _ecore_xcb_image_shm_create(im);
        if (!im->xim) return EINA_FALSE;

        if ((sx == 0) && (w == im->w)) 
          {
             im->xim->data = (uint8_t *)im->data + (im->xim->stride * sy) + 
               (sx * im->bpp);
             im->xim->width = w;
             im->xim->height = h;

             ecore_x_grab();
             if (!xcb_image_shm_get(_ecore_xcb_conn, draw, im->xim, 
                                    im->shminfo, x, y, 0xffffffff)) 
               {
                  DBG("\tImage Shm Get Failed");
                  ret = EINA_FALSE;
               }
             ecore_x_ungrab();
             ecore_x_sync(); // needed
          }
        else 
          {
             Ecore_X_Image *tim;

             tim = ecore_x_image_new(w, h, im->vis, im->depth);
             if (tim) 
               {
                  ret = ecore_x_image_get(tim, draw, x, y, 0, 0, w, h);
                  if (ret) 
                    {
                       unsigned char *spixels, *pixels;
                       int sbpp = 0, sbpl = 0, srows = 0;
                       int bpp = 0, bpl = 0, rows = 0;

                       spixels = 
                         ecore_x_image_data_get(tim, &sbpl, &srows, &sbpp);
                       pixels = ecore_x_image_data_get(im, &bpl, &rows, &bpp);
                       if ((spixels) && (pixels)) 
                         {
                            unsigned char *p, *sp;
                            int r = 0;

                            p = (pixels + (sy * bpl) + (sx * bpp));
                            sp = spixels;
                            for (r = srows; r > 0; r--) 
                              {
                                 memcpy(p, sp, sbpl);
                                 p += bpl;
                                 sp += sbpl;
                              }
                         }
                    }
                  ecore_x_image_free(tim);
               }
          }
     }
   else 
     {
        ret = EINA_FALSE;
        ecore_x_grab();
        im->xim = 
          xcb_image_get(_ecore_xcb_conn, draw, x, y, w, h, 
                        0xffffffff, XCB_IMAGE_FORMAT_Z_PIXMAP);
        if (!im->xim) ret = EINA_FALSE;
        ecore_x_ungrab();
        ecore_x_sync(); // needed

        if (im->xim) 
          {
             im->data = (unsigned char *)im->xim->data;
             im->bpl = im->xim->stride;
             im->rows = im->xim->height;
             if (im->xim->bpp <= 8)
               im->bpp = 1;
             else if (im->xim->bpp <= 16)
               im->bpp = 2;
             else
               im->bpp = 4;
          }
     }

   return ret;
}

EAPI void *
ecore_x_image_data_get(Ecore_X_Image *im, int *bpl, int *rows, int *bpp) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!im) return NULL;
   if (!im->xim) _ecore_xcb_image_shm_create(im);
   if (!im->xim) return NULL;

   if (bpl) *bpl = im->bpl;
   if (rows) *rows = im->rows;
   if (bpp) *bpp = im->bpp;

   return im->data;
}

EAPI void 
ecore_x_image_put(Ecore_X_Image *im, Ecore_X_Drawable draw, Ecore_X_GC gc, int x, int y, int sx, int sy, int w, int h) 
{
   Ecore_X_GC tgc = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!gc) 
     {
        uint32_t mask, values[1];

        tgc = xcb_generate_id(_ecore_xcb_conn);
        mask = XCB_GC_SUBWINDOW_MODE;
        values[0] = XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS;
        xcb_create_gc(_ecore_xcb_conn, tgc, draw, mask, values);
        gc = tgc;
     }
   if (!im->xim) _ecore_xcb_image_shm_create(im);
   if (im->xim) 
     {
        if (im->shm) 
          xcb_shm_put_image(_ecore_xcb_conn, draw, gc, im->xim->width, 
                            im->xim->height, sx, sy, w, h, x, y, 
                            im->xim->depth, im->xim->format, 0, 
                            im->shminfo.shmseg, 
                            im->xim->data - im->shminfo.shmaddr);
//          xcb_image_shm_put(_ecore_xcb_conn, draw, gc, im->xim, 
//                            im->shminfo, sx, sy, x, y, w, h, 0);
        else 
          xcb_image_put(_ecore_xcb_conn, draw, gc, im->xim, sx, sy, 0);

     }
   if (tgc) ecore_x_gc_free(tgc);
   ecore_x_sync();
}

EAPI Eina_Bool 
ecore_x_image_is_argb32_get(Ecore_X_Image *im) 
{
   xcb_visualtype_t *vis;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   vis = (xcb_visualtype_t *)im->vis;
   if (!im->xim) _ecore_xcb_image_shm_create(im);

   if (((vis->_class == XCB_VISUAL_CLASS_TRUE_COLOR) || 
        (vis->_class == XCB_VISUAL_CLASS_DIRECT_COLOR)) && 
       (im->depth >= 24) && (vis->red_mask == 0xff0000) && 
       (vis->green_mask == 0x00ff00) && (vis->blue_mask == 0x0000ff))
     {
#ifdef WORDS_BIGENDIAN
        if (im->xim->byte_order == XCB_IMAGE_ORDER_LSB_FIRST) 
          return EINA_TRUE;
#else
        if (im->xim->byte_order == XCB_IMAGE_ORDER_MSB_FIRST) 
          return EINA_TRUE;
#endif
     }

   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_image_to_argb_convert(void *src, int sbpp, int sbpl, Ecore_X_Colormap c, Ecore_X_Visual v, int x, int y, int w, int h, unsigned int *dst, int dbpl, int dx, int dy) 
{
   xcb_visualtype_t *vis;
   uint32_t *cols;
   int n = 0, nret = 0, i, row, mode = 0;
   unsigned int pal[256], r, g, b;
   enum
     {
        rgbnone = 0,
        rgb565,
        bgr565,
        rgbx555,
        argbx888,
        abgrx888,
        rgba888x,
        bgra888x,
        argbx666
     };

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   sbpp *= 8;

   vis = (xcb_visualtype_t *)v;
   n = vis->colormap_entries;
   if ((n <= 256) &&
       ((vis->_class == XCB_VISUAL_CLASS_PSEUDO_COLOR) ||
           (vis->_class == XCB_VISUAL_CLASS_STATIC_COLOR) ||
           (vis->_class == XCB_VISUAL_CLASS_GRAY_SCALE) ||
           (vis->_class == XCB_VISUAL_CLASS_STATIC_GRAY)))
     {
        xcb_query_colors_cookie_t cookie;
        xcb_query_colors_reply_t *reply;

        if (!c) 
          {
             c = (xcb_colormap_t)((xcb_screen_t *)
                                  _ecore_xcb_screen)->default_colormap;
          }

        cols = alloca(n * sizeof(uint32_t));
        for (i = 0; i < n; i++)
          cols[i] = i;

        cookie = xcb_query_colors_unchecked(_ecore_xcb_conn, c, n, cols);
        reply = xcb_query_colors_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) 
          {
             xcb_rgb_iterator_t iter;
             xcb_rgb_t *ret;

             iter = xcb_query_colors_colors_iterator(reply);
             ret = xcb_query_colors_colors(reply);
             if (ret) 
               {
                  for (i = 0; iter.rem; xcb_rgb_next(&iter), i++) 
                    {
                       pal[i] = 0xff000000 | 
                         ((iter.data->red   >> 8) << 16) |
                         ((iter.data->green >> 8) << 8) |
                         ((iter.data->blue  >> 8));
                    }
                  nret = n;
               }
             free(reply);
          }
     }
   else if ((vis->_class == XCB_VISUAL_CLASS_TRUE_COLOR) || 
            (vis->_class == XCB_VISUAL_CLASS_DIRECT_COLOR))
     {
        if ((vis->red_mask   == 0x00ff0000) &&
            (vis->green_mask == 0x0000ff00) &&
            (vis->blue_mask  == 0x000000ff))
          mode = argbx888;
        else if ((vis->red_mask   == 0x000000ff) &&
                 (vis->green_mask == 0x0000ff00) &&
                 (vis->blue_mask  == 0x00ff0000))
          mode = abgrx888;
        else if ((vis->red_mask   == 0xff000000) &&
                 (vis->green_mask == 0x00ff0000) &&
                 (vis->blue_mask  == 0x0000ff00))
          mode = rgba888x;
        else if ((vis->red_mask   == 0x0000ff00) &&
                 (vis->green_mask == 0x00ff0000) &&
                 (vis->blue_mask  == 0xff000000))
          mode = bgra888x;
        else if ((vis->red_mask   == 0x0003f000) &&
                 (vis->green_mask == 0x00000fc0) &&
                 (vis->blue_mask  == 0x0000003f))
          mode = argbx666;
        else if ((vis->red_mask   == 0x0000f800) &&
                 (vis->green_mask == 0x000007e0) &&
                 (vis->blue_mask  == 0x0000001f))
          mode = rgb565;
        else if ((vis->red_mask   == 0x0000001f) &&
                 (vis->green_mask == 0x000007e0) &&
                 (vis->blue_mask  == 0x0000f800))
          mode = bgr565;
        else if ((vis->red_mask   == 0x00007c00) &&
                 (vis->green_mask == 0x000003e0) &&
                 (vis->blue_mask  == 0x0000001f))
          mode = rgbx555;
        else
          return EINA_FALSE;
     }
   for (row = 0; row < h; row++)
     {
        unsigned char *s8;
        unsigned short *s16;
        unsigned int *s32, *dp, *de;

        dp = ((unsigned int *)(((unsigned char *)dst) + 
                               ((dy + row) * dbpl))) + dx;
        de = dp + w;
        switch (sbpp)
          {
           case 8:
             s8 = ((unsigned char *)(((unsigned char *)src) + 
                                     ((y + row) * sbpl))) + x;
             if (nret > 0)
               {
                  while (dp < de)
                    {
                       *dp = pal[*s8];
                       s8++; dp++;
                    }
               }
             else
               return EINA_FALSE;
             break;
           case 16:
             s16 = ((unsigned short *)(((unsigned char *)src) + 
                                       ((y + row) * sbpl))) + x;
             switch (mode)
               {
                case rgb565:
                  while (dp < de)
                    {
                       r = (*s16 & 0xf800) << 8;
                       g = (*s16 & 0x07e0) << 5;
                       b = (*s16 & 0x001f) << 3;
                       r |= (r >> 5) & 0xff0000;
                       g |= (g >> 6) & 0x00ff00;
                       b |= (b >> 5);
                       *dp = 0xff000000 | r | g | b;
                       s16++; dp++;
                    }
                  break;
                case bgr565:
                  while (dp < de)
                    {
                       r = (*s16 & 0x001f) << 19;
                       g = (*s16 & 0x07e0) << 5;
                       b = (*s16 & 0xf800) >> 8;
                       r |= (r >> 5) & 0xff0000;
                       g |= (g >> 6) & 0x00ff00;
                       b |= (b >> 5);
                       *dp = 0xff000000 | r | g | b;
                       s16++; dp++;
                    }
                  break;
                case rgbx555:
                  while (dp < de)
                    {
                       r = (*s16 & 0x7c00) << 9;
                       g = (*s16 & 0x03e0) << 6;
                       b = (*s16 & 0x001f) << 3;
                       r |= (r >> 5) & 0xff0000;
                       g |= (g >> 5) & 0x00ff00;
                       b |= (b >> 5);
                       *dp = 0xff000000 | r | g | b;
                       s16++; dp++;
                    }
                  break;
                default:
                  return EINA_FALSE;
                  break;
               }
             break;
           case 24:
           case 32:
             s32 = ((unsigned int *)(((unsigned char *)src) + 
                                     ((y + row) * sbpl))) + x;
             switch (mode)
               {
                case argbx888:
                  while (dp < de)
                    {
                       *dp = 0xff000000 | *s32;
                       s32++; dp++;
                    }
                  break;
                case abgrx888:
                  while (dp < de)
                    {
                       r = *s32 & 0x000000ff;
                       g = *s32 & 0x0000ff00;
                       b = *s32 & 0x00ff0000;
                       *dp = 0xff000000 | (r << 16) | (g) | (b >> 16);
                       s32++; dp++;
                    }
                  break;
                case rgba888x:
                  while (dp < de)
                    {
                       *dp = 0xff000000 | (*s32 >> 8);
                       s32++; dp++;
                    }
                  break;
                case bgra888x:
                  while (dp < de)
                    {
                       r = *s32 & 0x0000ff00;
                       g = *s32 & 0x00ff0000;
                       b = *s32 & 0xff000000;
                       *dp = 0xff000000 | (r << 8) | (g >> 8) | (b >> 24);
                       s32++; dp++;
                    }
                  break;
                case argbx666:
                  while (dp < de)
                    {
                       r = (*s32 & 0x3f000) << 6;
                       g = (*s32 & 0x00fc0) << 4;
                       b = (*s32 & 0x0003f) << 2;
                       r |= (r >> 6) & 0xff0000;
                       g |= (g >> 6) & 0x00ff00;
                       b |= (b >> 6);
                       *dp = 0xff000000 | r | g | b;
                       s32++; dp++;
                    }
                  break;
                default:
                  return EINA_FALSE;
                  break;
               }
             break;
             break;
           default:
             return EINA_FALSE;
             break;
          }
     }
   return EINA_TRUE;
}

/* local functions */
static void 
_ecore_xcb_image_shm_check(void) 
{
//   xcb_shm_query_version_reply_t *reply;
   xcb_shm_segment_info_t shminfo;
   xcb_shm_get_image_cookie_t cookie;
   xcb_shm_get_image_reply_t *ireply;
   xcb_image_t *img = 0;
   uint8_t depth = 0;

   if (_ecore_xcb_image_shm_can != -1) return;

   /* reply =  */
   /*   xcb_shm_query_version_reply(_ecore_xcb_conn,  */
   /*                               xcb_shm_query_version(_ecore_xcb_conn), NULL); */
   /* if (!reply)  */
   /*   { */
   /*      _ecore_xcb_image_shm_can = 0; */
   /*      return; */
   /*   } */

   /* if ((reply->major_version < 1) ||  */
   /*     ((reply->major_version == 1) && (reply->minor_version == 0)))  */
   /*   { */
   /*      _ecore_xcb_image_shm_can = 0; */
   /*      free(reply); */
   /*      return; */
   /*   } */

   /* free(reply); */

   depth = ((xcb_screen_t *)_ecore_xcb_screen)->root_depth;

   ecore_x_sync(); // needed

   img = _ecore_xcb_image_create_native(1, 1, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                        depth, NULL, ~0, NULL);
   if (!img) 
     {
        _ecore_xcb_image_shm_can = 0;
        return;
     }

   shminfo.shmid = 
     shmget(IPC_PRIVATE, img->stride * img->height, (IPC_CREAT | 0666));
   if (shminfo.shmid == (uint32_t)-1) 
     {
        xcb_image_destroy(img);
        _ecore_xcb_image_shm_can = 0;
        return;
     }

   shminfo.shmaddr = shmat(shminfo.shmid, 0, 0);
   img->data = shminfo.shmaddr;
   if (img->data == (uint8_t *)-1) 
     {
        xcb_image_destroy(img);
        _ecore_xcb_image_shm_can = 0;
        return;
     }

   shminfo.shmseg = xcb_generate_id(_ecore_xcb_conn);
   xcb_shm_attach(_ecore_xcb_conn, shminfo.shmseg, shminfo.shmid, 0);

   cookie = 
     xcb_shm_get_image(_ecore_xcb_conn, 
                       ((xcb_screen_t *)_ecore_xcb_screen)->root, 
                       0, 0, img->width, img->height, 
                       0xffffffff, img->format, 
                       shminfo.shmseg, img->data - shminfo.shmaddr);

   ecore_x_sync(); // needed

   ireply = xcb_shm_get_image_reply(_ecore_xcb_conn, cookie, NULL);
   if (ireply) 
     {
        _ecore_xcb_image_shm_can = 1;
        free(ireply);
     }
   else
     _ecore_xcb_image_shm_can = 0;

   xcb_shm_detach(_ecore_xcb_conn, shminfo.shmseg);
   xcb_image_destroy(img);
   shmdt(shminfo.shmaddr);
   shmctl(shminfo.shmid, IPC_RMID, 0);
}

static void 
_ecore_xcb_image_shm_create(Ecore_X_Image *im) 
{
   im->xim = 
     _ecore_xcb_image_create_native(im->w, im->h, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                    im->depth, NULL, ~0, NULL);
   if (!im->xim) return;

   im->shminfo.shmid = shmget(IPC_PRIVATE, im->xim->size, (IPC_CREAT | 0666));
   if (im->shminfo.shmid == (uint32_t)-1) 
     {
        xcb_image_destroy(im->xim);
        return;
     }

   im->shminfo.shmaddr = shmat(im->shminfo.shmid, 0, 0);
   im->xim->data = im->shminfo.shmaddr;
   if ((!im->xim->data) || (im->xim->data == (uint8_t *)-1))
     {
        DBG("Shm Create No Image Data");
        xcb_image_destroy(im->xim);
        shmdt(im->shminfo.shmaddr);
        shmctl(im->shminfo.shmid, IPC_RMID, 0);
        return;
     }

   im->shminfo.shmseg = xcb_generate_id(_ecore_xcb_conn);
   xcb_shm_attach(_ecore_xcb_conn, im->shminfo.shmseg, im->shminfo.shmid, 0);

   im->data = (unsigned char *)im->xim->data;
   im->bpl = im->xim->stride;
   im->rows = im->xim->height;
   if (im->xim->bpp <= 8)
     im->bpp = 1;
   else if (im->xim->bpp <= 16)
     im->bpp = 2;
   else
     im->bpp = 4;
}

xcb_image_t *
_ecore_xcb_image_create_native(int w, int h, xcb_image_format_t format, uint8_t depth, void *base, uint32_t bytes, uint8_t *data) 
{
   static uint8_t dpth = 0;
   static xcb_format_t *fmt = NULL;
   const xcb_setup_t *setup;
   xcb_image_format_t xif;

   /* NB: We cannot use xcb_image_create_native as it only creates images 
    * using MSB_FIRST, so this routine recreates that function and uses 
    * the endian-ness of the server setup */
   setup = xcb_get_setup(_ecore_xcb_conn);
   xif = format;

   if ((xif == XCB_IMAGE_FORMAT_Z_PIXMAP) && (depth == 1))
     xif = XCB_IMAGE_FORMAT_XY_PIXMAP;

   if (dpth != depth) 
     {
        dpth = depth;
        fmt = _ecore_xcb_image_find_format(setup, depth);
        if (!fmt) return 0;
     }

   switch (xif) 
     {
      case XCB_IMAGE_FORMAT_XY_BITMAP:
        if (depth != 1) return 0;
      case XCB_IMAGE_FORMAT_XY_PIXMAP:
      case XCB_IMAGE_FORMAT_Z_PIXMAP:
        return xcb_image_create(w, h, xif, 
                                fmt->scanline_pad, 
                                fmt->depth, fmt->bits_per_pixel, 
                                setup->bitmap_format_scanline_unit, 
                                setup->image_byte_order, 
                                setup->bitmap_format_bit_order, 
                                base, bytes, data);
      default:
        break;
     }

   return 0;
}

static xcb_format_t *
_ecore_xcb_image_find_format(const xcb_setup_t *setup, uint8_t depth) 
{
   xcb_format_t *fmt, *fmtend;

   fmt = xcb_setup_pixmap_formats(setup);
   fmtend = fmt + xcb_setup_pixmap_formats_length(setup);
   for (; fmt != fmtend; ++fmt)
     if (fmt->depth == depth) 
       return fmt;

   return 0;
}
