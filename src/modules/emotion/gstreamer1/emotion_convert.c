#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "emotion_gstreamer.h"

static inline void
_evas_video_bgrx_step(unsigned char *evas_data, const unsigned char *gst_data,
                      unsigned int w, unsigned int h EINA_UNUSED, unsigned int output_height, unsigned int step)
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
_evas_video_bgra(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h EINA_UNUSED, unsigned int output_height)
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
_evas_video_i420(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;
   unsigned int stride_y, stride_uv;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   stride_y = GST_ROUND_UP_4(w);
   stride_uv = GST_ROUND_UP_8(w) / 2;

   for (i = 0; i < rh; i++)
     rows[i] = &gst_data[i * stride_y];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * stride_y + j * stride_uv];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * stride_y +
			 (rh / 2) * stride_uv +
			 j * stride_uv];
}

static void
_evas_video_yv12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;
   unsigned int stride_y, stride_uv;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   stride_y = GST_ROUND_UP_4(w);
   stride_uv = GST_ROUND_UP_8(w) / 2;

   for (i = 0; i < rh; i++)
     rows[i] = &gst_data[i * stride_y];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * stride_y +
			 (rh / 2) * stride_uv +
			 j * stride_uv];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * stride_y + j * stride_uv];
}

static void
_evas_video_yuy2(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h EINA_UNUSED, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i;
   unsigned int stride;

   rows = (const unsigned char **)evas_data;

   stride = GST_ROUND_UP_4(w * 2);

   for (i = 0; i < output_height; i++)
     rows[i] = &gst_data[i * stride];
}

static void
_evas_video_nv12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h EINA_UNUSED, unsigned int output_height)
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

const ColorSpace_Format_Convertion colorspace_format_convertion[] = {
  { "I420", GST_VIDEO_FORMAT_I420, EVAS_COLORSPACE_YCBCR422P601_PL, _evas_video_i420, EINA_TRUE },
  { "YV12", GST_VIDEO_FORMAT_YV12, EVAS_COLORSPACE_YCBCR422P601_PL, _evas_video_yv12, EINA_TRUE },
  { "YUY2", GST_VIDEO_FORMAT_YUY2, EVAS_COLORSPACE_YCBCR422601_PL, _evas_video_yuy2, EINA_FALSE },
  { "NV12", GST_VIDEO_FORMAT_NV12, EVAS_COLORSPACE_YCBCR420NV12601_PL, _evas_video_nv12, EINA_TRUE },
  { "BGR", GST_VIDEO_FORMAT_BGR, EVAS_COLORSPACE_ARGB8888, _evas_video_bgr, EINA_FALSE },
  { "BGRx", GST_VIDEO_FORMAT_BGRx, EVAS_COLORSPACE_ARGB8888, _evas_video_bgrx, EINA_FALSE },
  { "BGRA", GST_VIDEO_FORMAT_BGRA, EVAS_COLORSPACE_ARGB8888, _evas_video_bgra, EINA_FALSE },
  { NULL, 0, 0, NULL, 0 }
};

