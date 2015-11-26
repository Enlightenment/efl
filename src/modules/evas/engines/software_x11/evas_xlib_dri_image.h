#include "evas_engine.h"

typedef struct _tbm_bufmgr *tbm_bufmgr;
typedef struct _tbm_bo *tbm_bo;

typedef union _tbm_bo_handle
{
   void     *ptr;
   int32_t  s32;
   uint32_t u32;
   int64_t  s64;
   uint64_t u64;
} tbm_bo_handle;

typedef struct
{
   unsigned int attachment;
   unsigned int name;
   unsigned int pitch;
   unsigned int cpp;
   unsigned int flags;
} DRI2Buffer;

#define DRI2_BUFFER_TYPE_WINDOW 0x0
#define DRI2_BUFFER_TYPE_PIXMAP 0x1
#define DRI2_BUFFER_TYPE_FB     0x2

typedef union
{
   unsigned int flags;
   struct
   {
      unsigned int type:1;
      unsigned int is_framebuffer:1;
      unsigned int is_mapped:1;
      unsigned int is_reused:1;
      unsigned int idx_reuse:3;
   }
   data;
} DRI2BufferFlags;

typedef struct
{
   unsigned int name;
   tbm_bo   buf_bo;
} Buffer;

typedef struct _Evas_DRI_Image Evas_DRI_Image;
typedef struct _DRI_Native DRI_Native;

struct _Evas_DRI_Image
{
   Display         *dis;
   Visual          *visual;
   int              depth;
   int              w, h;
   int              bpl, bpp, rows;
   unsigned char   *data;
   Drawable        draw;
   tbm_bo          buf_bo;
   DRI2Buffer      *buf;
   void            *buf_data;
   int             buf_w, buf_h;
   Buffer          *buf_cache;
};

struct _DRI_Native
{
   Evas_Native_Surface ns;
   Pixmap              pixmap;
   Visual             *visual;
   Display            *d;

   Evas_DRI_Image       *exim;
};

Evas_DRI_Image *evas_xlib_image_dri_new(int w, int h, Visual *vis, int depth);

void evas_xlib_image_dir_free(Evas_DRI_Image *exim);
Eina_Bool evas_xlib_image_get_buffers(RGBA_Image *im);
void evas_xlib_image_buffer_unmap(Evas_DRI_Image *exim);
Eina_Bool evas_xlib_image_dri_init(Evas_DRI_Image *exim, Display *display);
Eina_Bool evas_xlib_image_dri_used(void);
void *evas_xlib_image_dri_native_set(void *data, void *image, void *native);
