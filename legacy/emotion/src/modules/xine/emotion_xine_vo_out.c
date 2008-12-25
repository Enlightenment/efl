/***************************************************************************/
/***                  emotion xine display engine                        ***/
/***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "emotion_private.h"
#include "emotion_xine.h"

#include <xine.h>
#include <xine/video_out.h>
#include <xine/xine_internal.h>
#include <xine/xineutils.h>
#include <xine/vo_scale.h>

#define BLEND_BYTE(dst, src, o) (((src)*o + ((dst)*(0xf-o)))/0xf)

/***************************************************************************/
typedef struct _Emotion_Frame  Emotion_Frame;
typedef struct _Emotion_Driver Emotion_Driver;
typedef struct _Emotion_Class  Emotion_Class;
typedef struct _Emotion_Lut    Emotion_Lut;

struct _Emotion_Frame
{
   vo_frame_t           vo_frame;
   int                  width;
   int                  height;
   double               ratio;
   int                  format;
   xine_t               *xine;
   
   Emotion_Xine_Video_Frame frame;
   unsigned char            in_use : 1;
};

struct _Emotion_Driver
{
   vo_driver_t          vo_driver;
   config_values_t     *config;
   int                  ratio;
   xine_t               *xine;
   Emotion_Xine_Video   *ev;
};

struct _Emotion_Class
{
   video_driver_class_t  driver_class;
   config_values_t      *config;
   xine_t               *xine;
};

struct _Emotion_Lut
{
   uint8_t cb    : 8;
   uint8_t cr    : 8;
   uint8_t y     : 8;
   uint8_t foo   : 8;
} __attribute__ ((packed));

/***************************************************************************/
static void        *_emotion_class_init            (xine_t *xine, void *visual);
static void         _emotion_class_dispose         (video_driver_class_t *driver_class);
static char        *_emotion_class_identifier_get  (video_driver_class_t *driver_class);
static char        *_emotion_class_description_get (video_driver_class_t *driver_class);

static vo_driver_t *_emotion_open                  (video_driver_class_t *driver_class, const void *visual);
static void         _emotion_dispose               (vo_driver_t *vo_driver);

static int          _emotion_redraw                (vo_driver_t *vo_driver);

static uint32_t     _emotion_capabilities_get      (vo_driver_t *vo_driver);
static int          _emotion_gui_data_exchange     (vo_driver_t *vo_driver, int data_type, void *data);

static int          _emotion_property_set          (vo_driver_t *vo_driver, int property, int value);
static int          _emotion_property_get          (vo_driver_t *vo_driver, int property);
static void         _emotion_property_min_max_get  (vo_driver_t *vo_driver, int property, int *min, int *max);

static vo_frame_t  *_emotion_frame_alloc           (vo_driver_t *vo_driver);
static void         _emotion_frame_dispose         (vo_frame_t *vo_frame);
static void         _emotion_frame_format_update   (vo_driver_t *vo_driver, vo_frame_t *vo_frame, uint32_t width, uint32_t height, double ratio, int format, int flags);
static void         _emotion_frame_display         (vo_driver_t *vo_driver, vo_frame_t *vo_frame);
static void         _emotion_frame_field           (vo_frame_t *vo_frame, int which_field);

static void         _emotion_frame_data_free       (Emotion_Frame *fr);
static void         _emotion_frame_data_unlock     (Emotion_Frame *fr);

static void         _emotion_overlay_begin         (vo_driver_t *vo_driver, vo_frame_t *vo_frame, int changed);
static void         _emotion_overlay_end           (vo_driver_t *vo_driver, vo_frame_t *vo_frame);
static void         _emotion_overlay_blend         (vo_driver_t *vo_driver, vo_frame_t *vo_frame, vo_overlay_t *vo_overlay);

static void         _emotion_overlay_mem_blend_8   (uint8_t *mem, uint8_t val, uint8_t o, size_t sz);
static void         _emotion_overlay_blend_yuv     (uint8_t *dst_base[3], vo_overlay_t * img_overl, int dst_width, int dst_height, int dst_pitches[3]);

static void         _emotion_yuy2_to_bgra32        (int width, int height, unsigned char *src, unsigned char *dst);

/***************************************************************************/
static vo_info_t _emotion_info = 
{
   1,                        /* priority */
   XINE_VISUAL_TYPE_NONE   /* visual type */
};

plugin_info_t emotion_xine_plugin_info[] =
{
     { PLUGIN_VIDEO_OUT, 21, "emotion", XINE_VERSION_CODE, &_emotion_info, _emotion_class_init },
     { PLUGIN_NONE, 0, "", 0, NULL, NULL }
};

/***************************************************************************/
static void *
_emotion_class_init(xine_t *xine, void *visual)
{
   Emotion_Class *cl;
   
//   printf("emotion: _emotion_class_init()\n");
   cl = (Emotion_Class *) malloc(sizeof(Emotion_Class));
   if (!cl) return NULL;
   cl->driver_class.open_plugin     = _emotion_open;
   cl->driver_class.get_identifier  = _emotion_class_identifier_get;
   cl->driver_class.get_description = _emotion_class_description_get;
   cl->driver_class.dispose         = _emotion_class_dispose;
   cl->config                       = xine->config;
   cl->xine                         = xine;

   return cl;
}

static void
_emotion_class_dispose(video_driver_class_t *driver_class)
{
   Emotion_Class *cl;
   
   cl = (Emotion_Class *)driver_class;
   free(cl);
}

static char *
_emotion_class_identifier_get(video_driver_class_t *driver_class)
{
   Emotion_Class *cl;
   
   cl = (Emotion_Class *)driver_class;
   return "emotion";
}

static char *
_emotion_class_description_get(video_driver_class_t *driver_class)
{
   Emotion_Class *cl;
   
   cl = (Emotion_Class *)driver_class;
   return "Emotion xine video output plugin";
}

/***************************************************************************/
static vo_driver_t *
_emotion_open(video_driver_class_t *driver_class, const void *visual)
{
   Emotion_Class *cl;
   Emotion_Driver *dv;
   
   cl = (Emotion_Class *)driver_class;
   /* visual here is the data ptr passed to xine_open_video_driver() */
//   printf("emotion: _emotion_open()\n");
   dv = (Emotion_Driver *)malloc(sizeof(Emotion_Driver));
   if (!dv) return NULL;
   
   dv->config                         = cl->config;
   dv->xine                           = cl->xine;
   dv->ratio                          = XINE_VO_ASPECT_AUTO;
   dv->vo_driver.get_capabilities     = _emotion_capabilities_get;
   dv->vo_driver.alloc_frame          = _emotion_frame_alloc;
   dv->vo_driver.update_frame_format  = _emotion_frame_format_update;
   dv->vo_driver.overlay_begin        = _emotion_overlay_begin;
   dv->vo_driver.overlay_blend        = _emotion_overlay_blend;
   dv->vo_driver.overlay_end          = _emotion_overlay_end;
   dv->vo_driver.display_frame        = _emotion_frame_display;
   dv->vo_driver.get_property         = _emotion_property_get;
   dv->vo_driver.set_property         = _emotion_property_set;
   dv->vo_driver.get_property_min_max = _emotion_property_min_max_get;
   dv->vo_driver.gui_data_exchange    = _emotion_gui_data_exchange;
   dv->vo_driver.dispose              = _emotion_dispose;
   dv->vo_driver.redraw_needed        = _emotion_redraw;
   dv->ev                             = (Emotion_Xine_Video *)visual;
   dv->ev->have_vo = 1;
   printf("emotion: _emotion_open = %p\n", &dv->vo_driver);
   return &dv->vo_driver;
}    

static void
_emotion_dispose(vo_driver_t *vo_driver)
{
   Emotion_Driver *dv;
   
   dv = (Emotion_Driver *)vo_driver;
   dv->ev->have_vo = 0;
   printf("emotion: _emotion_dispose(%p)\n", dv);
   free(dv);
}

/***************************************************************************/
static int
_emotion_redraw(vo_driver_t *vo_driver)
{
   Emotion_Driver *dv;
   
   dv = (Emotion_Driver *)vo_driver;
//   printf("emotion: _emotion_redraw()\n");
   return 0;
}

/***************************************************************************/
static uint32_t
_emotion_capabilities_get(vo_driver_t *vo_driver)
{
   Emotion_Driver *dv;
   
   dv = (Emotion_Driver *)vo_driver;
//   printf("emotion: _emotion_capabilities_get()\n");
   return VO_CAP_YV12 | VO_CAP_YUY2;
}

/***************************************************************************/
static int
_emotion_gui_data_exchange(vo_driver_t *vo_driver, int data_type, void *data)
{
   Emotion_Driver *dv;
   
   dv = (Emotion_Driver *)vo_driver;
//   printf("emotion: _emotion_gui_data_exchange()\n");
   switch (data_type)
     {
      case XINE_GUI_SEND_COMPLETION_EVENT:
	break;
      case XINE_GUI_SEND_DRAWABLE_CHANGED:
	break;
      case XINE_GUI_SEND_EXPOSE_EVENT:
	break;
      case XINE_GUI_SEND_TRANSLATE_GUI_TO_VIDEO:
	break;
      case XINE_GUI_SEND_VIDEOWIN_VISIBLE:
	break;
      case XINE_GUI_SEND_SELECT_VISUAL:
	break;
      default:
	break;
     }
  return 0;
}

/***************************************************************************/
static int
_emotion_property_set(vo_driver_t *vo_driver, int property, int value)
{
   Emotion_Driver *dv;
   
   dv = (Emotion_Driver *)vo_driver;
//   printf("emotion: _emotion_property_set()\n");
   switch (property)
     {
      case VO_PROP_ASPECT_RATIO:
	if (value >= XINE_VO_ASPECT_NUM_RATIOS)
	  value = XINE_VO_ASPECT_AUTO;
//	printf("DRIVER RATIO SET %i!\n", value);
	dv->ratio = value;
	break;
      default:
	break;
     }
   return value;
}

static int
_emotion_property_get(vo_driver_t *vo_driver, int property)
{
   Emotion_Driver *dv;
   
   dv = (Emotion_Driver *)vo_driver;
//   printf("emotion: _emotion_property_get()\n");
   switch (property)
     {
      case VO_PROP_ASPECT_RATIO:
	return dv->ratio;
	break;
      default:
	break;
     }
  return 0;
}

static void
_emotion_property_min_max_get(vo_driver_t *vo_driver, int property, int *min, int *max)
{
   Emotion_Driver *dv;
   
   dv = (Emotion_Driver *)vo_driver;
//   printf("emotion: _emotion_property_min_max_get()\n");
   *min = 0;
   *max = 0;
}

/***************************************************************************/
static vo_frame_t *
_emotion_frame_alloc(vo_driver_t *vo_driver)
{
   Emotion_Driver *dv;
   Emotion_Frame *fr;
   
   dv = (Emotion_Driver *)vo_driver;
//   printf("emotion: _emotion_frame_alloc()\n");
   fr = (Emotion_Frame *)calloc(1, sizeof(Emotion_Frame));
   if (!fr) return NULL;
   
   fr->vo_frame.base[0]    = NULL;
   fr->vo_frame.base[1]    = NULL;
   fr->vo_frame.base[2]    = NULL;
   
   fr->vo_frame.proc_slice = NULL;
   fr->vo_frame.proc_frame = NULL;
   fr->vo_frame.field      = _emotion_frame_field;
   fr->vo_frame.dispose    = _emotion_frame_dispose;
   fr->vo_frame.driver     = vo_driver;
   
   return (vo_frame_t *)fr;
}

static void
_emotion_frame_dispose(vo_frame_t *vo_frame)
{
   Emotion_Frame *fr;
   
   fr = (Emotion_Frame *)vo_frame;
//   printf("emotion: _emotion_frame_dispose()\n");
   _emotion_frame_data_free(fr);  
   free(fr);
}

static void
_emotion_frame_format_update(vo_driver_t *vo_driver, vo_frame_t *vo_frame, uint32_t width, uint32_t height, double ratio, int format, int flags)
{
   Emotion_Driver *dv;
   Emotion_Frame *fr;
   
   dv = (Emotion_Driver *)vo_driver;
   fr = (Emotion_Frame *)vo_frame;
   
   if ((fr->width != width) ||  (fr->height != height) || 
       (fr->format != format) || (!fr->vo_frame.base[0]))
     {
//	printf("emotion: _emotion_frame_format_update()\n");
	_emotion_frame_data_free(fr);
	
	fr->width  = width;
	fr->height = height;
	fr->format = format;
	
	switch (format)
	  {
	   case XINE_IMGFMT_YV12: 
	       {
		  int y_size, uv_size;
		  
		  fr->frame.format = EMOTION_FORMAT_YV12;
		  fr->vo_frame.pitches[0] = 8 * ((width + 7) / 8);
		  fr->vo_frame.pitches[1] = 8 * ((width + 15) / 16);
		  fr->vo_frame.pitches[2] = 8 * ((width + 15) / 16);
		  
		  y_size  = fr->vo_frame.pitches[0] * height;
		  uv_size = fr->vo_frame.pitches[1] * ((height + 1) / 2);
		  
		  fr->vo_frame.base[0] = malloc(y_size + (2 * uv_size));
		  fr->vo_frame.base[1] = fr->vo_frame.base[0] + y_size + uv_size;
		  fr->vo_frame.base[2] = fr->vo_frame.base[0] + y_size;
		  fr->frame.w = fr->width;
		  fr->frame.h = fr->height;
		  fr->frame.ratio = fr->vo_frame.ratio;
		  fr->frame.y = fr->vo_frame.base[0];
		  fr->frame.u = fr->vo_frame.base[1];
		  fr->frame.v = fr->vo_frame.base[2];
		  fr->frame.bgra_data = NULL;
		  fr->frame.y_stride = fr->vo_frame.pitches[0];
		  fr->frame.u_stride = fr->vo_frame.pitches[1];
		  fr->frame.v_stride = fr->vo_frame.pitches[2];
		  fr->frame.obj = dv->ev->obj;
	       }
	     break;
	   case XINE_IMGFMT_YUY2: 
	       {
		  fr->frame.format = EMOTION_FORMAT_BGRA;
		  fr->vo_frame.pitches[0] = 8 * ((width + 3) / 4);
		  fr->vo_frame.pitches[1] = 0;
		  fr->vo_frame.pitches[2] = 0;
		  
		  fr->vo_frame.base[0] = malloc(fr->vo_frame.pitches[0] * height);
		  fr->vo_frame.base[1] = NULL;
		  fr->vo_frame.base[2] = NULL;
		  
		  fr->frame.w = fr->width;
		  fr->frame.h = fr->height;
		  fr->frame.ratio = fr->vo_frame.ratio;
		  fr->frame.y = NULL;
		  fr->frame.u = NULL;
		  fr->frame.v = NULL;
		  fr->frame.bgra_data = malloc(fr->width * fr->height * 4);
		  fr->frame.y_stride = 0;
		  fr->frame.u_stride = 0;
		  fr->frame.v_stride = 0;
		  fr->frame.obj = dv->ev->obj;
	       }
	     break;
	   default:
	     break;
	  }
	if (((format == XINE_IMGFMT_YV12)
	     && ((fr->vo_frame.base[0] == NULL)
		 || (fr->vo_frame.base[1] == NULL)
		 || (fr->vo_frame.base[2] == NULL)))
	    || ((format == XINE_IMGFMT_YUY2)
		&& ((fr->vo_frame.base[0] == NULL)
		    || (fr->frame.bgra_data == NULL))))
	  {
	     _emotion_frame_data_free(fr);
	  }
     }
   fr->frame.ratio = fr->vo_frame.ratio;
   fr->ratio = ratio;
}

static void
_emotion_frame_display(vo_driver_t *vo_driver, vo_frame_t *vo_frame)
{
   Emotion_Driver *dv;
   Emotion_Frame *fr;
   
   dv = (Emotion_Driver *)vo_driver;
   fr = (Emotion_Frame *)vo_frame;
//   printf("emotion: _emotion_frame_display()\n");
//   printf("EX VO: fq %i %p\n", dv->ev->fq, dv->ev);
// if my frame queue is too deep ( > 4 frames) simply block and wait for them
// to drain
//   while (dv->ev->fq > 4) usleep(1);
   if (dv->ev)
     {
	void *buf;
	int ret;

	if (dv->ev->closing) return;
	if (fr->format == XINE_IMGFMT_YUY2)
	  {
	     _emotion_yuy2_to_bgra32(fr->width, fr->height, fr->vo_frame.base[0], fr->frame.bgra_data);
	  }
	
	buf = &(fr->frame);
	fr->frame.timestamp = (double)fr->vo_frame.vpts / 90000.0;
	fr->frame.done_func = _emotion_frame_data_unlock;
	fr->frame.done_data = fr;
//	printf("FRAME FOR %p\n", dv->ev);
	ret = write(dv->ev->fd_write, &buf, sizeof(void *));
//	printf("-- FRAME DEC %p == %i\n", fr->frame.obj, ret);
	fr->in_use = 1;
	dv->ev->fq++;
     }
   /* hmm - must find a way to sanely copy data out... FIXME problem */
//   fr->vo_frame.free(&fr->vo_frame);
}

static void
_emotion_frame_field(vo_frame_t *vo_frame, int which_field)
{
   Emotion_Frame *fr;
   
   fr = (Emotion_Frame *)vo_frame;
//   printf("emotion: _emotion_frame_field()\n");
}

/***************************************************************************/
static void
_emotion_frame_data_free(Emotion_Frame *fr)
{
   if (fr->vo_frame.base[0])
     {
	free(fr->vo_frame.base[0]);
	fr->vo_frame.base[0] = NULL;
	fr->vo_frame.base[1] = NULL;
	fr->vo_frame.base[2] = NULL;
	fr->frame.y = fr->vo_frame.base[0];
	fr->frame.u = fr->vo_frame.base[1];
	fr->frame.v = fr->vo_frame.base[2];
     }
   if (fr->frame.bgra_data)
     {
	free(fr->frame.bgra_data);
	fr->frame.bgra_data = NULL;
     }
}

static void
_emotion_frame_data_unlock(Emotion_Frame *fr)
{
//   printf("emotion: _emotion_frame_data_unlock()\n");
   if (fr->in_use)
     {
	fr->vo_frame.free(&fr->vo_frame);
	fr->in_use = 0;
     }
}

/***************************************************************************/
static void
_emotion_overlay_begin(vo_driver_t *vo_driver, vo_frame_t *vo_frame, int changed)
{
   Emotion_Driver *dv;
   Emotion_Frame *fr;
   
   dv = (Emotion_Driver *)vo_driver;
   fr = (Emotion_Frame *)vo_frame;
//   printf("emotion: _emotion_overlay_begin()\n");
}

static void
_emotion_overlay_end(vo_driver_t *vo_driver, vo_frame_t *vo_frame)
{
   Emotion_Driver *dv;
   Emotion_Frame *fr;
   
   dv = (Emotion_Driver *)vo_driver;
   fr = (Emotion_Frame *)vo_frame;
//   printf("emotion: _emotion_overlay_end()\n");
}

static void
_emotion_overlay_blend(vo_driver_t *vo_driver, vo_frame_t *vo_frame, vo_overlay_t *vo_overlay)
{
   Emotion_Driver *dv;
   Emotion_Frame *fr;
   
   dv = (Emotion_Driver *)vo_driver;
   fr = (Emotion_Frame *)vo_frame;
//   printf("emotion: _emotion_overlay_blend()\n");
   _emotion_overlay_blend_yuv(fr->vo_frame.base, vo_overlay,
			      fr->width, fr->height, 
			      fr->vo_frame.pitches);
}

static void _emotion_overlay_mem_blend_8(uint8_t *mem, uint8_t val, uint8_t o, size_t sz)
{
   uint8_t *limit = mem + sz;
   while (mem < limit)
     {
	*mem = BLEND_BYTE(*mem, val, o);
	mem++;
     }
}

static void _emotion_overlay_blend_yuv(uint8_t *dst_base[3], vo_overlay_t * img_overl, int dst_width, int dst_height, int dst_pitches[3])
{
   Emotion_Lut *my_clut;
   uint8_t *my_trans;
   int src_width;
   int src_height;
   rle_elem_t *rle;
   rle_elem_t *rle_limit;
   int x_off;
   int y_off;
   int ymask, xmask;
   int rle_this_bite;
   int rle_remainder;
   int rlelen;
   int x, y;
   int hili_right;
   uint8_t clr = 0;
   
   src_width = img_overl->width;
   src_height = img_overl->height;
   rle = img_overl->rle;
   rle_limit = rle + img_overl->num_rle;
   x_off = img_overl->x;
   y_off = img_overl->y;
   
   if (!rle) return;
   
   uint8_t *dst_y = dst_base[0] + dst_pitches[0] * y_off + x_off;
   uint8_t *dst_cr = dst_base[2] + (y_off / 2) * dst_pitches[1] + (x_off / 2) + 1;
   uint8_t *dst_cb = dst_base[1] + (y_off / 2) * dst_pitches[2] + (x_off / 2) + 1;
   my_clut = (Emotion_Lut *) img_overl->hili_color;
   my_trans = img_overl->hili_trans;
   
   /* avoid wraping overlay if drawing to small image */
   if( (x_off + img_overl->hili_right) < dst_width )
     hili_right = img_overl->hili_right;
   else
     hili_right = dst_width - 1 - x_off;
   
   /* avoid buffer overflow */
   if( (src_height + y_off) >= dst_height )
     src_height = dst_height - 1 - y_off;
   
   rlelen=rle_remainder=0;
   for (y = 0; y < src_height; y++)
     {
	ymask = ((img_overl->hili_top > y) || (img_overl->hili_bottom < y));
	xmask = 0;
	
	for (x = 0; x < src_width;)
	  {
	     uint16_t o;
	     
	     if (rlelen == 0)
	       {
		  rle_remainder = rlelen = rle->len;
		  clr = rle->color;
		  rle++;
	       }
	     if (rle_remainder == 0)
	       {
		  rle_remainder = rlelen;
	       }
	     if ((rle_remainder + x) > src_width)
	       {
		  /* Do something for long rlelengths */
		  rle_remainder = src_width - x;
	       } 
	     
	     if (ymask == 0)
	       {
		  if (x <= img_overl->hili_left)
		    {
		       /* Starts outside clip area */
		       if ((x + rle_remainder - 1) > img_overl->hili_left )
			 {
			    /* Cutting needed, starts outside, ends inside */
			    rle_this_bite = (img_overl->hili_left - x + 1);
			    rle_remainder -= rle_this_bite;
			    rlelen -= rle_this_bite;
			    my_clut = (Emotion_Lut *) img_overl->color;
			    my_trans = img_overl->trans;
			    xmask = 0;
			 }
		       else
			 {
			    /* no cutting needed, starts outside, ends outside */
			    rle_this_bite = rle_remainder;
			    rle_remainder = 0;
			    rlelen -= rle_this_bite;
			    my_clut = (Emotion_Lut *) img_overl->color;
			    my_trans = img_overl->trans;
			    xmask = 0;
			 }
		    }
		  else if (x < hili_right)
		    {
		       /* Starts inside clip area */
		       if ((x + rle_remainder) > hili_right )
			 {
			    /* Cutting needed, starts inside, ends outside */
			    rle_this_bite = (hili_right - x);
			    rle_remainder -= rle_this_bite;
			    rlelen -= rle_this_bite;
			    my_clut = (Emotion_Lut *) img_overl->hili_color;
			    my_trans = img_overl->hili_trans;
			    xmask++;
			 }
		       else
			 {
			    /* no cutting needed, starts inside, ends inside */
			    rle_this_bite = rle_remainder;
			    rle_remainder = 0;
			    rlelen -= rle_this_bite;
			    my_clut = (Emotion_Lut *) img_overl->hili_color;
			    my_trans = img_overl->hili_trans;
			    xmask++;
			 }
		    }
		  else if (x >= hili_right)
		    {
		       /* Starts outside clip area, ends outsite clip area */
		       if ((x + rle_remainder ) > src_width )
			 {
			    /* Cutting needed, starts outside, ends at right edge */
			    /* It should never reach here due to the earlier test of src_width */
			    rle_this_bite = (src_width - x );
			    rle_remainder -= rle_this_bite;
			    rlelen -= rle_this_bite;
			    my_clut = (Emotion_Lut *) img_overl->color;
			    my_trans = img_overl->trans;
			    xmask = 0;
			 }
		       else
			 {
			    /* no cutting needed, starts outside, ends outside */
			    rle_this_bite = rle_remainder;
			    rle_remainder = 0;
			    rlelen -= rle_this_bite;
			    my_clut = (Emotion_Lut *) img_overl->color;
			    my_trans = img_overl->trans;
			    xmask = 0;
			 }
		    }
	       }
	     else
	       {
		  /* Outside clip are due to y */
		  /* no cutting needed, starts outside, ends outside */
		  rle_this_bite = rle_remainder;
		  rle_remainder = 0;
		  rlelen -= rle_this_bite;
		  my_clut = (Emotion_Lut *) img_overl->color;
		  my_trans = img_overl->trans;
		  xmask = 0;
	       }
	     o   = my_trans[clr];
	     if (o)
	       {
		  if (o >= 15)
		    {
		       memset(dst_y + x, my_clut[clr].y, rle_this_bite);
		       if (y & 1)
			 {
			    memset(dst_cr + (x >> 1), my_clut[clr].cr, (rle_this_bite+1) >> 1);
			    memset(dst_cb + (x >> 1), my_clut[clr].cb, (rle_this_bite+1) >> 1);
			 }
		    }
		  else
		    {
		       _emotion_overlay_mem_blend_8(dst_y + x, my_clut[clr].y, o, rle_this_bite);
		       if (y & 1)
			 {
			    /* Blending cr and cb should use a different function, with pre -128 to each sample */
			    _emotion_overlay_mem_blend_8(dst_cr + (x >> 1), my_clut[clr].cr, o, (rle_this_bite+1) >> 1);
			    _emotion_overlay_mem_blend_8(dst_cb + (x >> 1), my_clut[clr].cb, o, (rle_this_bite+1) >> 1);
			 }
		    }
	       }
	     x += rle_this_bite;
	     if (rle >= rle_limit)
	       {
		  break;
	       }
	  }
	if (rle >= rle_limit)
	  {
	     break;
	  }
	
	dst_y += dst_pitches[0];
	
	if (y & 1)
	  {
	     dst_cr += dst_pitches[2];
	     dst_cb += dst_pitches[1];
	  }
     }
}

//TODO: Really need to improve this converter! 
#define LIMIT(x)  ((x) > 0xff ? 0xff : ((x) < 0 ? 0 : (x)))

static void
_emotion_yuy2_to_bgra32(int width, int height, unsigned char *src, unsigned char *dst)
{
   int i, j;
   unsigned char *y, *u, *v;

   y = src;
   u = src + 1;
   v = src + 3;
   for (i = 0; i < width; i++)
     {
	for (j = 0; j < height; j++)
	  {
	     *dst++ = LIMIT(1.164 * (*y - 16) + 2.018 * (*u - 128));
	     *dst++ = LIMIT(1.164 * (*y - 16) - 0.813 * (*v - 128) - 0.391 * (*u - 128));
	     *dst++ = LIMIT(1.164 * (*y - 16) + 1.596 * (*v - 128));
	     *dst++ = 0;
	     
	     y += 2;
	     if (j % 2 == 1)
	       {
		  u += 4;
		  v += 4;
	       }
	  }
     }
}
