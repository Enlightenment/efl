#include "evas_common.h"
#include "evas_engine.h"

#include <X11/XCB/shm.h>
#include <X11/XCB/xcb_image.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct _Xcb_Output_Buffer
{
   XCBConnection     *connection;
   XCBImage          *image;
   XCBShmSegmentInfo *shm_info;
   void              *data;
};

static int _xcb_err = 0;

void
evas_software_x11_x_software_xcb_write_mask_line(Xcb_Output_Buffer *xcbob,
						 DATA32            *src,
						 int                w,
						 int                y)
{
   int x;
   
   for (x = 0; x < w; x++)
     {
	XCBImagePutPixel(xcbob->image, x, y, A_VAL(&(src[x])) >> 7);
     }
}

int
evas_software_x11_x_software_xcb_can_do_shm(XCBConnection *c)
{
   XCBGetGeometryRep *geom;
   XCBDRAWABLE        drawable;
   int                depth;
     
   drawable.window = XCBConnSetupSuccessRepRootsIter (XCBGetSetup(c)).data->root;
   geom = XCBGetGeometryReply (c, XCBGetGeometry(c, drawable), 0);
   if(!geom)
     return 0;
   
   depth = geom->depth;
   free (geom);

   if (XCBShmInit(c))
     {
	Xcb_Output_Buffer *xcbob;
	
	xcbob = evas_software_x11_x_software_xcb_output_buffer_new(c, 
								   depth,
								   16,
								   16,
								   2,
								   NULL);
	if (!xcbob)
	  return 0;
	evas_software_x11_x_software_xcb_output_buffer_free(xcbob);
	return 1;
     }
   return 0;
}

/* 
 * FIXME: no error mechanism
 */

/* static void */
/* x_software_x11_output_tmp_x11_err(XCBConnection *c, XErrorEvent * ev) */
/* { */
/*    _xcb_err = 1; */
/*    return; */
/*    d = NULL; */
/*    ev = NULL; */
/* } */

Xcb_Output_Buffer *
evas_software_x11_x_software_xcb_output_buffer_new(XCBConnection *c,
						   int            depth,
						   int            w,
						   int            h,
						   int            try_shm,
						   void          *data)
{
   Xcb_Output_Buffer *xcbob;

   xcbob = calloc(1, sizeof(Xcb_Output_Buffer));
   if (!xcbob) return NULL;
   
   xcbob->connection = c;
   xcbob->image      = NULL;
   xcbob->shm_info   = NULL;

   if (try_shm > 0)
     {
	xcbob->shm_info = malloc(sizeof(XCBShmSegmentInfo));
	if (xcbob->shm_info)
	  {
	     xcbob->shm_info->shmseg = XCBShmSEGNew(c);
	     xcbob->image = XCBImageSHMCreate(c, depth, ZPixmap, NULL, w, h);
	     if (xcbob->image)
	       {
		  xcbob->shm_info->shmid = shmget(IPC_PRIVATE, 
						  xcbob->image->bytes_per_line * 
						  xcbob->image->height,
						  IPC_CREAT | 0777);
		  if (xcbob->shm_info->shmid >= 0)
		    {
		       xcbob->shm_info->shmaddr = xcbob->image->data = 
			 shmat(xcbob->shm_info->shmid, 0, 0);
		       if (xcbob->shm_info->shmaddr != NULL)
			 {
			   /* 
			    * FIXME: no error mechanism
			    */

/* 			    XErrorHandler ph; */
			    
			    XCBSync(c, 0);
			    _xcb_err = 0;
/* 			    ph = XSetErrorHandler((XErrorHandler) */
/* 						  x_software_x11_output_tmp_x_err); */
			    XCBShmAttach(c,
					 xcbob->shm_info->shmseg,
					 xcbob->shm_info->shmid, 0);
			    XCBSync(c, 0);
/* 			    XSetErrorHandler((XErrorHandler)ph); */
			    if (!_xcb_err)
			      {
				 return xcbob;
			      }
			 }
		       shmdt(xcbob->shm_info->shmaddr);		       
		       shmctl(xcbob->shm_info->shmid, IPC_RMID, 0);   
		    }
		  if (xcbob->image) XCBImageSHMDestroy(xcbob->image);
		  xcbob->image = NULL;
	       }
	     if (xcbob->shm_info) free(xcbob->shm_info);
	     xcbob->shm_info = NULL;
	  }
     }   

   if (try_shm > 1) return NULL;   
   
   xcbob->image = XCBImageCreate(c, depth, ZPixmap, 0, data, w, h, 32, 0);   
   if (!xcbob->image)
     {
	free(xcbob);
	return NULL;
     }
   
   xcbob->data = data;
   
   if (!xcbob->image->data)
     {
	xcbob->image->data = malloc(xcbob->image->bytes_per_line * xcbob->image->height);
	if (!xcbob->image->data)
	  {
	     XCBImageDestroy(xcbob->image);	
	     free(xcbob);
	     return NULL;
	  }
     }
   return xcbob;
}

void
evas_software_x11_x_software_xcb_output_buffer_free(Xcb_Output_Buffer *xcbob)
{
   if (xcbob->shm_info)
     {
	XCBSync(xcbob->connection, 0);
	XCBShmDetach(xcbob->connection, xcbob->shm_info->shmseg);
	XCBImageSHMDestroy(xcbob->image);
	shmdt(xcbob->shm_info->shmaddr);
	shmctl(xcbob->shm_info->shmid, IPC_RMID, 0);
	free(xcbob->shm_info);
     }
   else
     {
	if (xcbob->data) xcbob->image->data = NULL;
	XCBImageDestroy(xcbob->image);
     }   
   free(xcbob);
}

void
evas_software_x11_x_software_xcb_output_buffer_paste(Xcb_Output_Buffer *xcbob,
						     XCBDRAWABLE        d,
						     XCBGCONTEXT        gc,
						     int                x,
						     int                y)
{
   if (xcbob->shm_info)
     {
	XCBImageSHMPut(xcbob->connection, d, gc,
		       xcbob->image, *xcbob->shm_info,
		       0, 0,
		       x, y,
		       xcbob->image->width, xcbob->image->height,
		       0);
	XCBSync(xcbob->connection, 0);
     }
   else
     {
	XCBImagePut(xcbob->connection, d, gc, xcbob->image, 0, 0, x, y, xcbob->image->width, xcbob->image->height);
     }
}

DATA8 *
evas_software_x11_x_software_xcb_output_buffer_data(Xcb_Output_Buffer *xcbob,
						    int               *bytes_per_line_ret)
{
   if (bytes_per_line_ret) *bytes_per_line_ret = xcbob->image->bytes_per_line;
   return xcbob->image->data;
}

int
evas_software_x11_x_software_xcb_output_buffer_depth(Xcb_Output_Buffer *xcbob)
{
   return xcbob->image->bits_per_pixel;
}

int
evas_software_x11_x_software_xcb_output_buffer_byte_order(Xcb_Output_Buffer *xcbob)
{
   return xcbob->image->image_byte_order;
}
