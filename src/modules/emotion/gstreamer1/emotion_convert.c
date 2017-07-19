#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "emotion_gstreamer.h"

static inline void
_evas_video_bgrx_step(unsigned char *evas_data, const unsigned char *gst_data,
                      unsigned int w, unsigned int h EINA_UNUSED,
                      unsigned int output_height, unsigned int step)
{
   unsigned int x, y;

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
_evas_video_bgr(unsigned char *evas_data, const unsigned char *gst_data,
                unsigned int w, unsigned int h, unsigned int output_height,
                Emotion_Convert_Info *info EINA_UNUSED)
{
   // XXX: need to check offset and stride that gst provide and what they
   // mean with a non-planar format like bgra
   _evas_video_bgrx_step(evas_data, gst_data, w, h, output_height, 3);
}

static void
_evas_video_bgrx(unsigned char *evas_data, const unsigned char *gst_data,
                 unsigned int w, unsigned int h, unsigned int output_height,
                 Emotion_Convert_Info *info EINA_UNUSED)
{
   // XXX: need to check offset and stride that gst provide and what they
   // mean with a non-planar format like bgra
   _evas_video_bgrx_step(evas_data, gst_data, w, h, output_height, 4);
}

static void
_evas_video_bgra(unsigned char *evas_data, const unsigned char *gst_data,
                 unsigned int w, unsigned int h EINA_UNUSED,
                 unsigned int output_height,
                 Emotion_Convert_Info *info EINA_UNUSED)
{
   unsigned int x, y;

   // XXX: need to check offset and stride that gst provide and what they
   // mean with a non-planar format like bgra
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
_evas_video_i420(unsigned char *evas_data,
                 const unsigned char *gst_data EINA_UNUSED,
                 unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED,
                 unsigned int output_height,
                 Emotion_Convert_Info *info)
{
   const unsigned char **rows, *ptr;
   unsigned int i, j, jump, rh;

   if (info->bpp[0] != 1) ERR("Plane 0 bpp != 1");
   if (info->bpp[1] != 1) ERR("Plane 1 bpp != 1");
   if (info->bpp[2] != 1) ERR("Plane 2 bpp != 1");

   rh = output_height;
   rows = (const unsigned char **)evas_data;

   ptr = info->plane_ptr[0];
   jump = info->stride[0];
   for (i = 0; i < rh; i++, ptr += jump) rows[i] = ptr;

   ptr = info->plane_ptr[1];
   jump = info->stride[1];
   for (j = 0; j < (rh / 2); j++, i++, ptr += jump) rows[i] = ptr;

   ptr = info->plane_ptr[2];
   jump = info->stride[2];
   for (j = 0; j < (rh / 2); j++, i++, ptr += jump) rows[i] = ptr;
}

static void
_evas_video_yv12(unsigned char *evas_data,
                 const unsigned char *gst_data EINA_UNUSED,
                 unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED,
                 unsigned int output_height,
                 Emotion_Convert_Info *info)
{
   const unsigned char **rows, *ptr;
   unsigned int i, j, jump, rh;

   if (info->bpp[0] != 1) ERR("Plane 0 bpp != 1");
   if (info->bpp[1] != 1) ERR("Plane 1 bpp != 1");
   if (info->bpp[2] != 1) ERR("Plane 2 bpp != 1");

   rh = output_height;
   rows = (const unsigned char **)evas_data;

   ptr = info->plane_ptr[0];
   jump = info->stride[0];
   for (i = 0; i < rh; i++, ptr += jump) rows[i] = ptr;

   ptr = info->plane_ptr[1];
   jump = info->stride[1];
   for (j = 0; j < (rh / 2); j++, i++, ptr += jump) rows[i] = ptr;

   ptr = info->plane_ptr[2];
   jump = info->stride[2];
   for (j = 0; j < (rh / 2); j++, i++, ptr += jump) rows[i] = ptr;
}

static void
_evas_video_yuy2(unsigned char *evas_data, const unsigned char *gst_data,
                 unsigned int w, unsigned int h EINA_UNUSED,
                 unsigned int output_height,
                 Emotion_Convert_Info *info EINA_UNUSED)
{
   const unsigned char **rows;
   unsigned int i, stride;

   // XXX: need to check offset and stride that gst provide and what they
   // mean with a non-planar format like yuy2
   rows = (const unsigned char **)evas_data;

   stride = GST_ROUND_UP_4(w * 2);

   for (i = 0; i < output_height; i++) rows[i] = &gst_data[i * stride];
}

static void
_evas_video_nv12(unsigned char *evas_data,
                 const unsigned char *gst_data EINA_UNUSED,
                 unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED,
                 unsigned int output_height, Emotion_Convert_Info *info)
{
   const unsigned char **rows, *ptr;
   unsigned int i, j, jump, rh;

   if (info->bpp[0] != 1) ERR("Plane 0 bpp != 1");
   // XXX: not sure this should be 1 but 2 bytes per pixel... no?
   //if (info->bpp[1] != 1) ERR("Plane 1 bpp != 1");

   rh = output_height;
   rows = (const unsigned char **)evas_data;

   ptr = info->plane_ptr[0];
   jump = info->stride[0];
   for (i = 0; i < rh; i++, ptr += jump) rows[i] = ptr;

   ptr = info->plane_ptr[1];
   jump = info->stride[1];
   for (j = 0; j < (rh / 2); j++, i++, ptr += jump) rows[i] = ptr;
}

const ColorSpace_Format_Convertion colorspace_format_convertion[] = {
  { "I420-709", GST_VIDEO_FORMAT_I420, GST_VIDEO_COLOR_MATRIX_BT709,
     EVAS_COLORSPACE_YCBCR422P709_PL, _evas_video_i420, EINA_TRUE },
  { "I420", GST_VIDEO_FORMAT_I420, GST_VIDEO_COLOR_MATRIX_BT601,
     EVAS_COLORSPACE_YCBCR422P601_PL, _evas_video_i420, EINA_TRUE },

  { "YV12-709", GST_VIDEO_FORMAT_YV12, GST_VIDEO_COLOR_MATRIX_BT709,
     EVAS_COLORSPACE_YCBCR422P709_PL, _evas_video_yv12, EINA_TRUE },
  { "YV12", GST_VIDEO_FORMAT_YV12, GST_VIDEO_COLOR_MATRIX_BT601,
     EVAS_COLORSPACE_YCBCR422P601_PL, _evas_video_yv12, EINA_TRUE },

  { "YUY2", GST_VIDEO_FORMAT_YUY2, GST_VIDEO_COLOR_MATRIX_BT601,
     EVAS_COLORSPACE_YCBCR422601_PL, _evas_video_yuy2, EINA_FALSE },
  { "NV12", GST_VIDEO_FORMAT_NV12, GST_VIDEO_COLOR_MATRIX_BT601,
     EVAS_COLORSPACE_YCBCR420NV12601_PL, _evas_video_nv12, EINA_TRUE },
   // XXX:
   // XXX: need to add nv12 709 colorspace support to evas itself.
   // XXX: this makes gst streams work when they are nv12 709 but maybe
   // XXX: will display in slightly off color.. but in the end this needs
   // XXX: fixing to display correctly.
   // XXX:
  { "NV12-709", GST_VIDEO_FORMAT_NV12, GST_VIDEO_COLOR_MATRIX_BT709,
     EVAS_COLORSPACE_YCBCR420NV12601_PL, _evas_video_nv12, EINA_TRUE },
   // XXX:
   // XXX:
   // XXX:

  { "BGR", GST_VIDEO_FORMAT_BGR, GST_VIDEO_COLOR_MATRIX_UNKNOWN,
     EVAS_COLORSPACE_ARGB8888, _evas_video_bgr, EINA_FALSE },
  { "BGRx", GST_VIDEO_FORMAT_BGRx, GST_VIDEO_COLOR_MATRIX_UNKNOWN,
     EVAS_COLORSPACE_ARGB8888, _evas_video_bgrx, EINA_FALSE },
  { "BGRA", GST_VIDEO_FORMAT_BGRA, GST_VIDEO_COLOR_MATRIX_UNKNOWN,
     EVAS_COLORSPACE_ARGB8888, _evas_video_bgra, EINA_FALSE },

  { NULL, 0, 0, 0, NULL, 0 }
};

