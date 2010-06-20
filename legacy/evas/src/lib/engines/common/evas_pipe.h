#ifndef _EVAS_PIPE_H
#define _EVAS_PIPE_H

#ifdef BUILD_PTHREAD
typedef struct _Thinfo
{
   int                    thread_num;
   pthread_t              thread_id;
   pthread_barrier_t     *barrier;
   RGBA_Pipe_Thread_Info *info;
#ifdef EVAS_FRAME_QUEUING
   void		         *fq_info;
#endif
} Thinfo;
#endif

#ifdef EVAS_FRAME_QUEUING
struct _Evas_Surface
{
   EINA_INLIST;
   RGBA_Image *im;
   int x, y, w, h;
   int dontpush; // dont push the surface out after drawing done
};
typedef struct _Evas_Surface Evas_Surface;

struct _Evas_Frame
{
   EINA_INLIST;
   Evas_Surface *surfaces;
   void *data;
   int in_process;
   int ready;
   int dont_schedule;
   struct timeval ready_time;

   void (*output_redraws_next_update_push) (void *data, void *surface, int x, int y, int w, int h);
   void (*output_flush)  (void *data);
   void (*output_set_priv)(void *data, void *cur, void *prev);
};
typedef struct _Evas_Frame Evas_Frame;


struct _Evas_FrameQ
{
   int initialised;
   Evas_Frame   *frames;
   pthread_cond_t cond_new;
   pthread_cond_t cond_ready;
   pthread_cond_t cond_done;
   LK(mutex);

   int             thread_num;
   Thinfo          thinfo[TH_MAX];
   int		   frameq_sz;

   Evas_Frame *cur_frame;	 
};
typedef struct _Evas_FrameQ Evas_FrameQ;
#define FRAMEQ_SZ_PER_THREAD 2

struct _Evas_Frameq_Thread_Info
{
   Evas_FrameQ *frameq;
};
typedef struct _Evas_Frameq_Thread_Info Evas_Frameq_Thread_Info;

EAPI Evas_Surface *evas_common_frameq_new_surface(void *surface, int x, int y, int w, int h);
EAPI void evas_common_frameq_add_surface(Evas_Surface *surface);
EAPI void evas_common_frameq_set_frame_data(void *data, 
     void (*fn_output_redraws_next_update_push) (void *data, void *surface, int x, int y, int w, int h),
     void (*fn_output_flush)  (void *data),
     void (*fn_output_set_priv)(void *data, void *cur, void *prev));
EAPI void evas_common_frameq_prepare_frame(void);
EAPI void evas_common_frameq_ready_frame(void);
EAPI void evas_common_frameq_init(void);
EAPI void evas_common_frameq_flush(void);
EAPI void evas_common_frameq_flush_ready(void);
#endif

/* image rendering pipelines... new optional system - non-immediate and
 * threadable
 */

EAPI void evas_common_pipe_free(RGBA_Image *im);
EAPI void evas_common_pipe_rectangle_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);
EAPI void evas_common_pipe_line_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1);
EAPI void evas_common_pipe_poly_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points, int x, int y);
EAPI void evas_common_pipe_grad_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient *gr);
EAPI void evas_common_pipe_grad2_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient2 *gr);
EAPI void evas_common_pipe_text_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *text);
EAPI void evas_common_pipe_image_load(RGBA_Image *im);
EAPI void evas_common_pipe_image_draw(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int smooth, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI void evas_common_pipe_map4_draw(RGBA_Image *src, RGBA_Image *dst,
				     RGBA_Draw_Context *dc, RGBA_Map_Point *p,
				     int smooth, int level);
EAPI void evas_common_pipe_flush(RGBA_Image *im);

#ifdef EVAS_FRAME_QUEUING
EAPI void evas_common_pipe_op_grad_flush(RGBA_Gradient *gr);
EAPI void evas_common_pipe_op_grad2_flush(RGBA_Gradient2 *gr);
EAPI void evas_common_pipe_op_text_flush(RGBA_Font *fn);
EAPI void evas_common_pipe_op_image_flush(RGBA_Image *im);
#endif

#endif /* _EVAS_PIPE_H */
