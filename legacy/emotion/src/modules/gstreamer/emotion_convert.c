#include "emotion_gstreamer.h"

static inline void
_evas_video_bgrx_step(unsigned char *evas_data, const unsigned char *gst_data,
                      unsigned int w, unsigned int h __UNUSED__, unsigned int output_height, unsigned int step)
{
   unsigned int x;
   unsigned int y;

   for (y = 0; y < output_height; ++y)
     {
        for (x = 0; x < w; x++)
          {
             evas_data[0] = gst_data[0];
             evas_data[1] = gst_data[1];
             evas_data[2] = gst_data[2];
             evas_data[3] = 255;
             gst_data += step;
             evas_data += 4;
          }
     }
}

static void
_evas_video_bgr(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   _evas_video_bgrx_step(evas_data, gst_data, w, h, output_height, 3);
}

static void
_evas_video_bgrx(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   _evas_video_bgrx_step(evas_data, gst_data, w, h, output_height, 4);
}

static void
_evas_video_bgra(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h __UNUSED__, unsigned int output_height)
{
   unsigned int x;
   unsigned int y;

   for (y = 0; y < output_height; ++y)
     {
        unsigned char alpha;

        for (x = 0; x < w; ++x)
          {
             alpha = gst_data[3];
             evas_data[0] = (gst_data[0] * alpha) / 255;
             evas_data[1] = (gst_data[1] * alpha) / 255;
             evas_data[2] = (gst_data[2] * alpha) / 255;
             evas_data[3] = alpha;
             gst_data += 4;
             evas_data += 4;
          }
     }
}

static void
_evas_video_i420(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h __UNUSED__, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < rh; i++)
     rows[i] = &gst_data[i * w];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + j * (w / 2)];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + rh * (w / 4) + j * (w / 2)];
}

static void
_evas_video_yv12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h __UNUSED__, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < rh; i++)
     rows[i] = &gst_data[i * w];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + rh * (w / 4) + j * (w / 2)];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + j * (w / 2)];
}

static void
_evas_video_yuy2(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h __UNUSED__, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < output_height; i++)
     rows[i] = &gst_data[i * w * 2];
}

static void
_evas_video_nv12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h __UNUSED__, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < rh; i++)
     rows[i] = &gst_data[i * w];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[rh * w + j * w];
}

static void
_evas_video_mt12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height __UNUSED__)
{
   const unsigned char **rows;
   unsigned int i;
   unsigned int j;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < (h / 32) / 2; i++)
     rows[i] = &gst_data[i * w * 2 * 32];

   if ((h / 32) % 2)
     {
        rows[i] = &gst_data[i * w * 2 * 32];
        i++;
     }

   for (j = 0; j < ((h / 2) / 32) / 2; ++j, ++i)
     rows[i] = &gst_data[h * w + j * (w / 2) * 2 * 16];
}

void
_evas_video_st12_multiplane(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height __UNUSED__)
{
   const GstMultiPlaneImageBuffer *mp_buf = (const GstMultiPlaneImageBuffer *) gst_data;
   const unsigned char **rows;
   unsigned int i;
   unsigned int j;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < (h / 32) / 2; i++)
     rows[i] = mp_buf->uaddr[0] + i * w * 2 * 32;
   if ((h / 32) % 2)
     {
        rows[i] = mp_buf->uaddr[0] + i * w * 2 * 32;
        i++;
     }

   for (j = 0; j < ((h / 2) / 16) / 2; j++, i++)
     {
       rows[i] = mp_buf->uaddr[1] + j * w * 2 * 16 * 2;
     }
   if (((h / 2) / 16) % 2)
     rows[i] = mp_buf->uaddr[1] + j * w * 2 * 16 * 2;
}

void
_evas_video_st12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w __UNUSED__, unsigned int h, unsigned int output_height __UNUSED__)
{
   const SCMN_IMGB *imgb = (const SCMN_IMGB *) gst_data;
   const unsigned char **rows;
   unsigned int i, j;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < (h / 32) / 2; i++)
     rows[i] = imgb->uaddr[0] + i * imgb->stride[0] * 2 * 32;
   if ((h / 32) % 2)
     {
        rows[i] = imgb->uaddr[0] + i * imgb->stride[0] * 2 * 32;
        i++;
     }

   for (j = 0; j < (unsigned int) imgb->elevation[1] / 32 / 2; j++, i++)
     rows[i] = imgb->uaddr[1] + j * imgb->stride[1] * 32 * 2;
   if ((imgb->elevation[1] / 32) % 2)
     rows[i++] = imgb->uaddr[1] + j * imgb->stride[1] * 32 * 2;
}

const ColorSpace_FourCC_Convertion colorspace_fourcc_convertion[] = {
  { "I420", GST_MAKE_FOURCC('I', '4', '2', '0'), EVAS_COLORSPACE_YCBCR422P601_PL, _evas_video_i420, EINA_TRUE },
  { "YV12", GST_MAKE_FOURCC('Y', 'V', '1', '2'), EVAS_COLORSPACE_YCBCR422P601_PL, _evas_video_yv12, EINA_TRUE },
  { "YUY2", GST_MAKE_FOURCC('Y', 'U', 'Y', '2'), EVAS_COLORSPACE_YCBCR422601_PL, _evas_video_yuy2, EINA_FALSE },
  { "NV12", GST_MAKE_FOURCC('N', 'V', '1', '2'), EVAS_COLORSPACE_YCBCR420NV12601_PL, _evas_video_nv12, EINA_TRUE },
  { "TM12", GST_MAKE_FOURCC('T', 'M', '1', '2'), EVAS_COLORSPACE_YCBCR420TM12601_PL, _evas_video_mt12, EINA_TRUE },
  { NULL, 0, 0, NULL, 0 }
};

const ColorSpace_Format_Convertion colorspace_format_convertion[] = {
  { "BGR", GST_VIDEO_FORMAT_BGR, EVAS_COLORSPACE_ARGB8888, _evas_video_bgr },
  { "BGRx", GST_VIDEO_FORMAT_BGRx, EVAS_COLORSPACE_ARGB8888, _evas_video_bgrx },
  { "BGRA", GST_VIDEO_FORMAT_BGRA, EVAS_COLORSPACE_ARGB8888, _evas_video_bgra },
  { NULL, 0, 0, NULL }
};
