/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"

static RGBA_Pipe *evas_common_pipe_add(RGBA_Pipe *pipe, RGBA_Pipe_Op **op);
static void evas_common_pipe_draw_context_copy(RGBA_Draw_Context *dc, RGBA_Pipe_Op *op);
static void evas_common_pipe_op_free(RGBA_Pipe_Op *op);

/* utils */
static RGBA_Pipe *
evas_common_pipe_add(RGBA_Pipe *pipe, RGBA_Pipe_Op **op)
{
   RGBA_Pipe *p;
   int first_pipe = 0;

   if (!pipe)
     {
	first_pipe = 1;
	p = calloc(1, sizeof(RGBA_Pipe));
	if (!p) return NULL;
	pipe = (RGBA_Pipe *)eina_inlist_append(EINA_INLIST_GET(pipe), EINA_INLIST_GET(p));
     }
   p = (RGBA_Pipe *)(EINA_INLIST_GET(pipe))->last;
   if (p->op_num == PIPE_LEN)
     {
	p = calloc(1, sizeof(RGBA_Pipe));
	if (!p) return NULL;
	pipe = (RGBA_Pipe *)eina_inlist_append(EINA_INLIST_GET(pipe), EINA_INLIST_GET(p));
     }
   p->op_num++;
   *op = &(p->op[p->op_num - 1]);
   if (first_pipe)
     {
	/* FIXME: PTHREAD init any thread locks etc */
     }
   return pipe;
}

static void
evas_common_pipe_draw_context_copy(RGBA_Draw_Context *dc, RGBA_Pipe_Op *op)
{
   memcpy(&(op->context), dc, sizeof(RGBA_Draw_Context));
   if (op->context.cutout.active > 0)
     {
	op->context.cutout.rects = malloc(sizeof(Cutout_Rect) * op->context.cutout.active);
	memcpy(op->context.cutout.rects, dc->cutout.rects, sizeof(Cutout_Rect) * op->context.cutout.active);
     }
   else
     op->context.cutout.rects = NULL;
}

static void
evas_common_pipe_op_free(RGBA_Pipe_Op *op)
{
   evas_common_draw_context_apply_clean_cutouts(&op->context.cutout);
}

/* main api calls */
#ifdef BUILD_PTHREAD
typedef struct _Thinfo
{
   int                    thread_num;
   pthread_t              thread_id;
   pthread_barrier_t     *barrier;
   RGBA_Pipe_Thread_Info *info;
} Thinfo;

static void *
evas_common_pipe_thread(void *data)
{
   Thinfo *thinfo;

//   printf("TH [...........\n");
   thinfo = data;
   for (;;)
     {
	RGBA_Pipe_Thread_Info *info;
	RGBA_Pipe *p;

	/* wait for start signal */
//	printf(" TH %i START...\n", thinfo->thread_num);
	pthread_barrier_wait(&(thinfo->barrier[0]));
	info = thinfo->info;
//	if (info)
//	  {
//	     thinfo->info = NULL;
//	     printf(" TH %i GO\n", thinfo->thread_num);
	EINA_INLIST_FOREACH(EINA_INLIST_GET(info->im->pipe), p)
	       {
		  int i;

		  for (i = 0; i < p->op_num; i++)
		    {
		       if (p->op[i].op_func)
			 p->op[i].op_func(info->im, &(p->op[i]), info);
		    }
	       }
	     free(info);
//	  }
//	printf(" TH %i DONE\n", thinfo->thread_num);
	/* send finished signal */
	pthread_barrier_wait(&(thinfo->barrier[1]));
     }
   return NULL;
}
#endif

#ifdef BUILD_PTHREAD
static int               thread_num = 0;
static Thinfo            thinfo[TH_MAX];
static pthread_barrier_t thbarrier[2];
#endif

EAPI void
evas_common_pipe_begin(RGBA_Image *im)
{
#ifdef BUILD_PTHREAD
   int i, y, h;

   if (!im->pipe) return;
   if (thread_num == 1) return;
   if (thread_num == 0)
     {
	int cpunum;

	cpunum = evas_common_cpu_count();
	thread_num = cpunum;
	if (thread_num == 1) return;
	pthread_barrier_init(&(thbarrier[0]), NULL, thread_num + 1);
	pthread_barrier_init(&(thbarrier[1]), NULL, thread_num + 1);
	for (i = 0; i < thread_num; i++)
	  {
	     pthread_attr_t attr;
	     cpu_set_t cpu;

	     pthread_attr_init(&attr);
	     CPU_ZERO(&cpu);
	     CPU_SET(i % cpunum, &cpu);
	     pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);
	     thinfo[i].thread_num = i;
	     thinfo[i].info = NULL;
	     thinfo[i].barrier = thbarrier;
	     /* setup initial locks */
	     pthread_create(&(thinfo[i].thread_id), &attr,
			    evas_common_pipe_thread, &(thinfo[i]));
	     pthread_attr_destroy(&attr);
	  }
     }
   y = 0;
   h = im->cache_entry.h / thread_num;
   if (h < 1) h = 1;
   for (i = 0; i < thread_num; i++)
     {
	RGBA_Pipe_Thread_Info *info;

//	     if (y >= im->cache_entry.h) break;
	info = calloc(1, sizeof(RGBA_Pipe_Thread_Info));
	info->im = im;
#ifdef EVAS_SLI
	info->x = 0;
	info->w = im->cache_entry.w;
	info->y = i;
	info->h = thread_num;
#else
	info->x = 0;
	info->y = y;
	info->w = im->cache_entry.w;
	if (i == (thread_num - 1))
	  info->h = im->cache_entry.h - y;
	else
	  info->h = h;
	y += info->h;
#endif
	thinfo[i].info = info;
     }
   /* tell worker threads to start */
   pthread_barrier_wait(&(thbarrier[0]));
#endif
}

EAPI void
evas_common_pipe_flush(RGBA_Image *im)
{

   RGBA_Pipe *p;
   int i;

   if (!im->pipe) return;
#ifdef BUILD_PTHREAD
   if (thread_num > 1)
     {
	/* sync worker threads */
	pthread_barrier_wait(&(thbarrier[1]));
     }
   else
#endif
     {
	/* process pipe - 1 thead */
	for (p = im->pipe; p; p = (RGBA_Pipe *)(EINA_INLIST_GET(p))->next)
	  {
	     for (i = 0; i < p->op_num; i++)
	       {
		  if (p->op[i].op_func)
		    p->op[i].op_func(im, &(p->op[i]), NULL);
	       }
	  }
     }
   evas_common_cpu_end_opt();
   evas_common_pipe_free(im);
}

EAPI void
evas_common_pipe_free(RGBA_Image *im)
{

   RGBA_Pipe *p;
   int i;

   if (!im->pipe) return;
   /* FIXME: PTHREAD join all threads here (if not finished) */

   /* free pipe */
   while (im->pipe)
     {
	p = im->pipe;
	for (i = 0; i < p->op_num; i++)
	  {
	     if (p->op[i].free_func)
	       p->op[i].free_func(&(p->op[i]));
	  }
	im->pipe = (RGBA_Pipe *)eina_inlist_remove(EINA_INLIST_GET(im->pipe), EINA_INLIST_GET(p));
	free(p);
     }
}



/* draw ops */
/**************** RECT ******************/
static void
evas_common_pipe_rectangle_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
	RGBA_Draw_Context context;

	memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
#ifdef EVAS_SLI
	evas_common_draw_context_set_sli(&(context), info->y, info->h);
#else
	evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
#endif
	evas_common_rectangle_draw(dst, &(context),
				   op->op.rect.x, op->op.rect.y,
				   op->op.rect.w, op->op.rect.h);
     }
   else
     evas_common_rectangle_draw(dst, &(op->context),
				op->op.rect.x, op->op.rect.y,
				op->op.rect.w, op->op.rect.h);
}

EAPI void
evas_common_pipe_rectangle_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
				int x, int y, int w, int h)
{
   RGBA_Pipe_Op *op;

   if ((w < 1) || (h < 1)) return;
   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) return;
   op->op.rect.x = x;
   op->op.rect.y = y;
   op->op.rect.w = w;
   op->op.rect.h = h;
   op->op_func = evas_common_pipe_rectangle_draw_do;
   op->free_func = evas_common_pipe_op_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** LINE ******************/
static void
evas_common_pipe_line_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
	RGBA_Draw_Context context;

	memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
#ifdef EVAS_SLI
	evas_common_draw_context_set_sli(&(context), info->y, info->h);
#else
	evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
#endif
	evas_common_line_draw(dst, &(context),
			      op->op.line.x0, op->op.line.y0,
			      op->op.line.x1, op->op.line.y1);
     }
   else
     evas_common_line_draw(dst, &(op->context),
			   op->op.line.x0, op->op.line.y0,
			   op->op.line.x1, op->op.line.y1);
}

EAPI void
evas_common_pipe_line_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			   int x0, int y0, int x1, int y1)
{
   RGBA_Pipe_Op *op;

   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) return;
   op->op.line.x0 = x0;
   op->op.line.y0 = y0;
   op->op.line.x1 = x1;
   op->op.line.y1 = y1;
   op->op_func = evas_common_pipe_line_draw_do;
   op->free_func = evas_common_pipe_op_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** POLY ******************/
static void
evas_common_pipe_op_poly_free(RGBA_Pipe_Op *op)
{
   RGBA_Polygon_Point *p;

   while (op->op.poly.points)
     {
	p = op->op.poly.points;
	op->op.poly.points = (RGBA_Polygon_Point *)eina_inlist_remove(EINA_INLIST_GET(op->op.poly.points),
								      EINA_INLIST_GET(p));
	free(p);
     }
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_poly_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
	RGBA_Draw_Context context;

	memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
#ifdef EVAS_SLI
	evas_common_draw_context_set_sli(&(context), info->y, info->h);
#else
	evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
#endif
	evas_common_polygon_draw(dst, &(context),
				 op->op.poly.points);
     }
   else
     evas_common_polygon_draw(dst, &(op->context),
			      op->op.poly.points);
}

EAPI void
evas_common_pipe_poly_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			   RGBA_Polygon_Point *points)
{
   RGBA_Pipe_Op *op;
   RGBA_Polygon_Point *pts = NULL, *p, *pp;

   if (!points) return;
   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) return;
   /* FIXME: copy points - maybe we should refcount? */
   for (p = points; p; p = (RGBA_Polygon_Point *)(EINA_INLIST_GET(p))->next)
     {
	pp = calloc(1, sizeof(RGBA_Polygon_Point));
	if (pp)
	  {
	     pp->x = p->x;
	     pp->y = p->y;
	     pts = (RGBA_Polygon_Point *)eina_inlist_append(EINA_INLIST_GET(pts), EINA_INLIST_GET(pp));
	  }
     }
   op->op.poly.points = pts;
   op->op_func = evas_common_pipe_poly_draw_do;
   op->free_func = evas_common_pipe_op_poly_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** GRAD ******************/
static void
evas_common_pipe_op_grad_free(RGBA_Pipe_Op *op)
{
   evas_common_gradient_free(op->op.grad.grad);
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_grad_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
	RGBA_Draw_Context context;

	memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
#ifdef EVAS_SLI
	evas_common_draw_context_set_sli(&(context), info->y, info->h);
#else
	evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
#endif
	evas_common_gradient_draw(dst, &(context),
				  op->op.grad.x, op->op.grad.y,
				  op->op.grad.w, op->op.grad.h,
				  op->op.grad.grad);
     }
   else
     evas_common_gradient_draw(dst, &(op->context),
			       op->op.grad.x, op->op.grad.y,
			       op->op.grad.w, op->op.grad.h,
			       op->op.grad.grad);
}

EAPI void
evas_common_pipe_grad_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			   int x, int y, int w, int h, RGBA_Gradient *gr)
{
   RGBA_Pipe_Op *op;

   if (!gr) return;
   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) return;
   op->op.grad.x = x;
   op->op.grad.y = y;
   op->op.grad.w = w;
   op->op.grad.h = h;
   gr->references++;
   op->op.grad.grad = gr;
   op->op_func = evas_common_pipe_grad_draw_do;
   op->free_func = evas_common_pipe_op_grad_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** GRAD2 ******************/
static void
evas_common_pipe_op_grad2_free(RGBA_Pipe_Op *op)
{
   evas_common_gradient2_free(op->op.grad2.grad);
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_grad2_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
	RGBA_Draw_Context context;
	
	memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
#ifdef EVAS_SLI
	evas_common_draw_context_set_sli(&(context), info->y, info->h);
#else	
	evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
#endif	
	evas_common_gradient2_draw(dst, &(context),
				  op->op.grad2.x, op->op.grad2.y,
				  op->op.grad2.w, op->op.grad2.h,
				  op->op.grad2.grad);
     }
   else
     evas_common_gradient2_draw(dst, &(op->context),
			       op->op.grad2.x, op->op.grad2.y,
			       op->op.grad2.w, op->op.grad2.h,
			       op->op.grad2.grad);
}

EAPI void
evas_common_pipe_grad2_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			   int x, int y, int w, int h, RGBA_Gradient2 *gr)
{
   RGBA_Pipe_Op *op;

   if (!gr) return;
   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) return;
   op->op.grad2.x = x;
   op->op.grad2.y = y;
   op->op.grad2.w = w;
   op->op.grad2.h = h;
   gr->references++;
   op->op.grad2.grad = gr;
   op->op_func = evas_common_pipe_grad2_draw_do;
   op->free_func = evas_common_pipe_op_grad2_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** TEXT ******************/
static void
evas_common_pipe_op_text_free(RGBA_Pipe_Op *op)
{
   evas_common_font_free(op->op.text.font);
   free(op->op.text.text);
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_text_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
	RGBA_Draw_Context context;

	memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
#ifdef EVAS_SLI
	evas_common_draw_context_set_sli(&(context), info->y, info->h);
#else
	evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
#endif
	evas_common_font_draw(dst, &(context),
			      op->op.text.font, op->op.text.x, op->op.text.y,
			      op->op.text.text);
     }
   else
     evas_common_font_draw(dst, &(op->context),
			   op->op.text.font, op->op.text.x, op->op.text.y,
			   op->op.text.text);
}

EAPI void
evas_common_pipe_text_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			   RGBA_Font *fn, int x, int y, const char *text)
{
   RGBA_Pipe_Op *op;

   if ((!fn) || (!text)) return;
   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) return;
   op->op.text.x = x;
   op->op.text.y = y;
   op->op.text.text = strdup(text);
   fn->references++;
   op->op.text.font = fn;
   op->op_func = evas_common_pipe_text_draw_do;
   op->free_func = evas_common_pipe_op_text_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** IMAGE *****************/
static void
evas_common_pipe_op_image_free(RGBA_Pipe_Op *op)
{
   op->op.image.src->ref--;
   if (op->op.image.src->ref == 0)
     evas_cache_image_drop(&op->op.image.src->cache_entry);
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_image_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
	RGBA_Draw_Context context;

	memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
#ifdef EVAS_SLI
	evas_common_draw_context_set_sli(&(context), info->y, info->h);
#else
	evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
#endif
	if (op->op.image.smooth)
	  evas_common_scale_rgba_in_to_out_clip_smooth(op->op.image.src,
						       dst, &(context),
						       op->op.image.sx,
						       op->op.image.sy,
						       op->op.image.sw,
						       op->op.image.sh,
						       op->op.image.dx,
						       op->op.image.dy,
						       op->op.image.dw,
						       op->op.image.dh);
	else
	  evas_common_scale_rgba_in_to_out_clip_sample(op->op.image.src,
						       dst, &(context),
						       op->op.image.sx,
						       op->op.image.sy,
						       op->op.image.sw,
						       op->op.image.sh,
						       op->op.image.dx,
						       op->op.image.dy,
						       op->op.image.dw,
						       op->op.image.dh);
     }
   else
     {
	if (op->op.image.smooth)
	  evas_common_scale_rgba_in_to_out_clip_smooth(op->op.image.src,
						       dst, &(op->context),
						       op->op.image.sx,
						       op->op.image.sy,
						       op->op.image.sw,
						       op->op.image.sh,
						       op->op.image.dx,
						       op->op.image.dy,
						       op->op.image.dw,
						       op->op.image.dh);
	else
	  evas_common_scale_rgba_in_to_out_clip_sample(op->op.image.src,
						       dst, &(op->context),
						       op->op.image.sx,
						       op->op.image.sy,
						       op->op.image.sw,
						       op->op.image.sh,
						       op->op.image.dx,
						       op->op.image.dy,
						       op->op.image.dw,
						       op->op.image.dh);
     }
}

EAPI void
evas_common_pipe_image_draw(RGBA_Image *src, RGBA_Image *dst,
			   RGBA_Draw_Context *dc, int smooth,
			   int src_region_x, int src_region_y,
			   int src_region_w, int src_region_h,
			   int dst_region_x, int dst_region_y,
			   int dst_region_w, int dst_region_h)
{
   RGBA_Pipe_Op *op;

   if (!src) return;
//   evas_common_pipe_flush(src);
   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) return;
   op->op.image.smooth = smooth;
   op->op.image.sx = src_region_x;
   op->op.image.sy = src_region_y;
   op->op.image.sw = src_region_w;
   op->op.image.sh = src_region_h;
   op->op.image.dx = dst_region_x;
   op->op.image.dy = dst_region_y;
   op->op.image.dw = dst_region_w;
   op->op.image.dh = dst_region_h;
   src->ref++;
   op->op.image.src = src;
   op->op_func = evas_common_pipe_image_draw_do;
   op->free_func = evas_common_pipe_op_image_free;
   evas_common_pipe_draw_context_copy(dc, op);
}
