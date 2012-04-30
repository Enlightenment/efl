// THIS IS DEPRECATED. WILL GO EVENTUALLTY. NO NEED TO SUPPORT ANYMORE

#include "evas_common.h"
#include <unistd.h>

#ifdef BUILD_PIPE_RENDER
static RGBA_Pipe *evas_common_pipe_add(RGBA_Pipe *pipe, RGBA_Pipe_Op **op);
static void evas_common_pipe_draw_context_copy(RGBA_Draw_Context *dc, RGBA_Pipe_Op *op);
static void evas_common_pipe_op_free(RGBA_Pipe_Op *op);

/* utils */
static RGBA_Pipe *
evas_common_pipe_add(RGBA_Pipe *rpipe, RGBA_Pipe_Op **op)
{
   RGBA_Pipe *p;
   int first_pipe = 0;

   if (!rpipe)
     {
        first_pipe = 1;
        p = calloc(1, sizeof(RGBA_Pipe));
        if (!p) return NULL;
        rpipe = (RGBA_Pipe *)eina_inlist_append(EINA_INLIST_GET(rpipe), EINA_INLIST_GET(p));
     }
   p = (RGBA_Pipe *)(EINA_INLIST_GET(rpipe))->last;
   if (p->op_num == PIPE_LEN)
     {
        p = calloc(1, sizeof(RGBA_Pipe));
        if (!p) return NULL;
        rpipe = (RGBA_Pipe *)eina_inlist_append(EINA_INLIST_GET(rpipe), EINA_INLIST_GET(p));
     }
   p->op_num++;
   *op = &(p->op[p->op_num - 1]);
   if (first_pipe)
     {
        /* FIXME: PTHREAD init any thread locks etc */
     }
   return rpipe;
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
     {
        op->context.cutout.rects = NULL;
     }
}

static void
evas_common_pipe_op_free(RGBA_Pipe_Op *op)
{
   evas_common_draw_context_apply_clean_cutouts(&op->context.cutout);
}

#ifdef BUILD_PTHREAD
/* main api calls */
static void *
evas_common_pipe_thread(void *data)
{
   Thinfo *thinfo;

// INF("TH [...........");
   thinfo = data;
   for (;;)
     {
        RGBA_Pipe_Thread_Info *info;
        RGBA_Pipe *p;

        /* wait for start signal */
// INF(" TH %i START...", thinfo->thread_num);
        pthread_barrier_wait(&(thinfo->barrier[0]));
        info = thinfo->info;
// if (info)
//   {
//      thinfo->info = NULL;
//      INF(" TH %i GO", thinfo->thread_num);
        EINA_INLIST_FOREACH(EINA_INLIST_GET(info->im->cache_entry.pipe), p)
          {
             int i;

             for (i = 0; i < p->op_num; i++)
               {
                  if (p->op[i].op_func)
                  p->op[i].op_func(info->im, &(p->op[i]), info);
               }
          }
        free(info);
//   }
// INF(" TH %i DONE", thinfo->thread_num);
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

static void
evas_common_pipe_begin(RGBA_Image *im)
{
#ifdef BUILD_PTHREAD
   int i, y, h;

   if (!im->cache_entry.pipe) return;
   if (thread_num == 1) return;
   y = 0;
   h = im->cache_entry.h / thread_num;
   if (h < 1) h = 1;
   for (i = 0; i < thread_num; i++)
     {
        RGBA_Pipe_Thread_Info *info;

//      if (y >= im->cache_entry.h) break;
        info = calloc(1, sizeof(RGBA_Pipe_Thread_Info));
        info->im = im;
        info->x = 0;
        info->y = y;
        info->w = im->cache_entry.w;
        if (i == (thread_num - 1))
          {
             info->h = im->cache_entry.h - y;
          }
        else
          {
             info->h = h;
          }
        y += info->h;
        thinfo[i].info = info;
     }
   /* tell worker threads to start */
   pthread_barrier_wait(&(thbarrier[0]));
#endif
}

EAPI void
evas_common_pipe_flush(RGBA_Image *im)
{
   if (!im->cache_entry.pipe) return;
#ifdef BUILD_PTHREAD
   if (thread_num > 1)
     {
       /* sync worker threads */
       pthread_barrier_wait(&(thbarrier[1]));
     }
   else
#endif
     {
       RGBA_Pipe *p;
       int i;
       
       /* process pipe - 1 thead */
       for (p = im->cache_entry.pipe; p; p = (RGBA_Pipe *)(EINA_INLIST_GET(p))->next)
	 {
	   for (i = 0; i < p->op_num; i++)
	     {
	       if (p->op[i].op_func)
		 {
		   p->op[i].op_func(im, &(p->op[i]), NULL);
		 }
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

   if (!im->cache_entry.pipe) return;
   /* FIXME: PTHREAD join all threads here (if not finished) */

   /* free pipe */
   while (im->cache_entry.pipe)
     {
        p = im->cache_entry.pipe;
        for (i = 0; i < p->op_num; i++)
          {
             if (p->op[i].free_func)
               {
                  p->op[i].free_func(&(p->op[i]));
               }
          }
        im->cache_entry.pipe = (RGBA_Pipe *)eina_inlist_remove(EINA_INLIST_GET(im->cache_entry.pipe), EINA_INLIST_GET(p));
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
        evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
        evas_common_rectangle_draw(dst, &(context),
               op->op.rect.x, op->op.rect.y,
               op->op.rect.w, op->op.rect.h);
     }
   else
     {
        evas_common_rectangle_draw(dst, &(op->context),
               op->op.rect.x, op->op.rect.y,
               op->op.rect.w, op->op.rect.h);
     }
}

EAPI void
evas_common_pipe_rectangle_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   RGBA_Pipe_Op *op;

   if ((w < 1) || (h < 1)) return;
   dst->cache_entry.pipe = evas_common_pipe_add(dst->cache_entry.pipe, &op);
   if (!dst->cache_entry.pipe) return;
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
        evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
        evas_common_line_draw(dst, &(context),
               op->op.line.x0, op->op.line.y0,
               op->op.line.x1, op->op.line.y1);
     }
   else
     {
        evas_common_line_draw(dst, &(op->context),
               op->op.line.x0, op->op.line.y0,
               op->op.line.x1, op->op.line.y1);
     }
}

EAPI void
evas_common_pipe_line_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
                           int x0, int y0, int x1, int y1)
{
   RGBA_Pipe_Op *op;

   dst->cache_entry.pipe = evas_common_pipe_add(dst->cache_entry.pipe, &op);
   if (!dst->cache_entry.pipe) return;
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
        evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
        evas_common_polygon_draw(dst, &(context),
                     op->op.poly.points, 0, 0);
     }
   else
     {
        evas_common_polygon_draw(dst, &(op->context),
                     op->op.poly.points, 0, 0);
     }
}

EAPI void
evas_common_pipe_poly_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
                           RGBA_Polygon_Point *points, int x, int y)
{
   RGBA_Pipe_Op *op;
   RGBA_Polygon_Point *pts = NULL, *p, *pp;

   if (!points) return;
   dst->cache_entry.pipe = evas_common_pipe_add(dst->cache_entry.pipe, &op);
   if (!dst->cache_entry.pipe) return;
   /* FIXME: copy points - maybe we should refcount? */
   for (p = points; p; p = (RGBA_Polygon_Point *)(EINA_INLIST_GET(p))->next)
     {
        pp = calloc(1, sizeof(RGBA_Polygon_Point));
        if (pp)
          {
             pp->x = p->x + x;
             pp->y = p->y + y;
             pts = (RGBA_Polygon_Point *)eina_inlist_append(EINA_INLIST_GET(pts), EINA_INLIST_GET(pp));
          }
     }
   op->op.poly.points = pts;
   op->op_func = evas_common_pipe_poly_draw_do;
   op->free_func = evas_common_pipe_op_poly_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** TEXT ******************/
static void
evas_common_pipe_op_text_free(RGBA_Pipe_Op *op)
{
   evas_common_text_props_content_unref(&(op->op.text.intl_props));
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_text_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
        RGBA_Draw_Context context;

        memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
        evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);
        evas_common_font_draw(dst, &(context), op->op.text.x, op->op.text.y, &op->op.text.intl_props);
     }
   else
     {
        evas_common_font_draw(dst, &(op->context), op->op.text.x, op->op.text.y, &op->op.text.intl_props);
     }
}

EAPI void
evas_common_pipe_text_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			   int x, int y, const Evas_Text_Props *intl_props)
{
   RGBA_Pipe_Op *op;

   dst->cache_entry.pipe = evas_common_pipe_add(dst->cache_entry.pipe, &op);
   if (!dst->cache_entry.pipe) return;
   op->op.text.x = x;
   op->op.text.y = y;
   evas_common_text_props_content_copy_and_ref(&(op->op.text.intl_props),
         intl_props);
   op->op_func = evas_common_pipe_text_draw_do;
   op->free_func = evas_common_pipe_op_text_free;
   evas_common_pipe_draw_context_copy(dc, op);
   evas_common_pipe_text_prepare(intl_props);
}

/**************** IMAGE *****************/
static void
evas_common_pipe_op_image_free(RGBA_Pipe_Op *op)
{
   op->op.image.src->ref--;
   if (op->op.image.src->ref == 0)
     {
        evas_cache_image_drop(&op->op.image.src->cache_entry);
     }
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_image_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
        RGBA_Draw_Context context;

        memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
        evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);

#ifdef SCALECACHE
        evas_common_rgba_image_scalecache_do((Image_Entry *)(op->op.image.src),
                                             dst, &(context),
                                             op->op.image.smooth,
                                             op->op.image.sx,
                                             op->op.image.sy,
                                             op->op.image.sw,
                                             op->op.image.sh,
                                             op->op.image.dx,
                                             op->op.image.dy,
                                             op->op.image.dw,
                                             op->op.image.dh);
#else
        if (op->op.image.smooth)
          {
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
          }
        else
          {
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
#endif
     }
   else
     {
#ifdef SCALECACHE
        evas_common_rgba_image_scalecache_do((Image_Entry *)(op->op.image.src),
                                             dst, &(op->context),
                                             op->op.image.smooth,
                                             op->op.image.sx,
                                             op->op.image.sy,
                                             op->op.image.sw,
                                             op->op.image.sh,
                                             op->op.image.dx,
                                             op->op.image.dy,
                                             op->op.image.dw,
                                             op->op.image.dh);
#else
        if (op->op.image.smooth)
          {
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
          }
        else
       	 {
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
#endif
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
   dst->cache_entry.pipe = evas_common_pipe_add(dst->cache_entry.pipe, &op);
   if (!dst->cache_entry.pipe) return;
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

   evas_common_pipe_image_load(src);
}

static void
evas_common_pipe_op_map_free(RGBA_Pipe_Op *op)
{
   op->op.map.src->ref--;
   if (op->op.map.src->ref == 0)
     evas_cache_image_drop(&op->op.map.src->cache_entry);
   free(op->op.map.p);
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_map_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
{
   if (info)
     {
	RGBA_Draw_Context context;

	memcpy(&(context), &(op->context), sizeof(RGBA_Draw_Context));
	evas_common_draw_context_clip_clip(&(context), info->x, info->y, info->w, info->h);

	evas_common_map_rgba(op->op.map.src, dst,
                             &context, op->op.map.npoints, op->op.map.p,
                             op->op.map.smooth, op->op.map.level);
     }
   else
     {
	evas_common_map_rgba(op->op.map.src, dst,
                             &(op->context), op->op.map.npoints, op->op.map.p,
                             op->op.map.smooth, op->op.map.level);
     }
}

EAPI void
evas_common_pipe_map_draw(RGBA_Image *src, RGBA_Image *dst,
                          RGBA_Draw_Context *dc, int npoints, RGBA_Map_Point *p,
                          int smooth, int level)
{
   RGBA_Pipe_Op *op;
   RGBA_Map_Point *pts_copy;
   int i;

   if (!src) return;
   pts_copy = malloc(sizeof (RGBA_Map_Point) * 4);
   if (!pts_copy) return;
   dst->cache_entry.pipe = evas_common_pipe_add(dst->cache_entry.pipe, &op);
   if (!dst->cache_entry.pipe) 
     {
       free(pts_copy);
       return; 
     }

   for (i = 0; i < 4; ++i)
     pts_copy[i] = p[i];

   op->op.map.npoints = npoints;
   op->op.map.smooth = smooth;
   op->op.map.level = level;
   src->ref++;
   op->op.map.src = src;
   op->op.map.p = pts_copy;
   op->op_func = evas_common_pipe_map_draw_do;
   op->free_func = evas_common_pipe_op_map_free;
   evas_common_pipe_draw_context_copy(dc, op);

   evas_common_pipe_image_load(src);
}

static void
evas_common_pipe_map_render(RGBA_Image *root)
{
  RGBA_Pipe *p;
  int i;

  /* Map imply that we need to process them recursively first. */
  for (p = root->cache_entry.pipe; p; p = (RGBA_Pipe *)(EINA_INLIST_GET(p))->next)
    {
      for (i = 0; i < p->op_num; i++) 
	{
	  if (p->op[i].op_func == evas_common_pipe_map_draw_do)
	    {
	      if (p->op[i].op.map.src->cache_entry.pipe)
		evas_common_pipe_map_render(p->op[i].op.map.src);
	    }
	  else if (p->op[i].op_func == evas_common_pipe_image_draw_do)
	    {
	      if (p->op[i].op.image.src->cache_entry.pipe)
		evas_common_pipe_map_render(p->op[i].op.image.src);
	    }
	}
    }

  evas_common_pipe_begin(root);
  evas_common_pipe_flush(root);
}

#ifdef BUILD_PTHREAD
static Eina_List *im_task = NULL;
static Eina_List *text_task = NULL;
static Thinfo task_thinfo[TH_MAX];
static pthread_barrier_t task_thbarrier[2];
static LK(im_task_mutex);
static LK(text_task_mutex);
#endif

#ifdef BUILD_PTHREAD
static void*
evas_common_pipe_load(void *data)
{
  Thinfo *tinfo;

  tinfo = data;
  for (;;)
    {
      /* wait for start signal */
      pthread_barrier_wait(&(tinfo->barrier[0]));

      while (im_task)
	{
	  RGBA_Image *im = NULL;

	  LKL(im_task_mutex);
	  im = eina_list_data_get(im_task);
	  im_task = eina_list_remove_list(im_task, im_task);
	  LKU(im_task_mutex);

	  if (im)
	    {
	      if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
		evas_cache_image_load_data(&im->cache_entry);
	      evas_common_image_colorspace_normalize(im);

	      im->flags &= ~RGBA_IMAGE_TODO_LOAD;
	    }
	}

      while (text_task)
	{
           Evas_Text_Props *text_props;
           RGBA_Font_Int *fi;
	  
           LKL(text_task_mutex);
           fi = eina_list_data_get(text_task);
           text_task = eina_list_remove_list(text_task, text_task);
           LKU(text_task_mutex);

           if (fi)
             {
                LKL(fi->ft_mutex);
                EINA_LIST_FREE(fi->task, text_props)
		  {
                     evas_common_font_draw_prepare(text_props);
                     text_props->changed = EINA_FALSE;
		  }
                LKU(fi->ft_mutex);
             }
	}

      /* send finished signal */    
      pthread_barrier_wait(&(tinfo->barrier[1]));
    }

  return NULL;
}
#endif

static volatile int bval = 0;

static void
evas_common_pipe_load_do(void)
{
#ifdef BUILD_PTHREAD
  /* Notify worker thread. */
  pthread_barrier_wait(&(task_thbarrier[0]));

  /* sync worker threads */
  pthread_barrier_wait(&(task_thbarrier[1]));
#endif
}

static Eina_Bool
evas_common_pipe_init(void)
{
#ifdef BUILD_PTHREAD
   if (thread_num == 0)
     {
	int cpunum;
	int i;

	cpunum = eina_cpu_count();
	thread_num = cpunum;
	fprintf(stderr, "number of cpu: %i\n", cpunum);
// on  single cpu we still want this initted.. otherwise we block forever
// waiting onm pthread barriers for async rendering on a single core!
//	if (thread_num == 1) return EINA_FALSE;

	eina_threads_init();

        LKI(im_task_mutex);
	LKI(text_task_mutex);

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

	pthread_barrier_init(&(task_thbarrier[0]), NULL, thread_num + 1);
	pthread_barrier_init(&(task_thbarrier[1]), NULL, thread_num + 1);
	for (i = 0; i < thread_num; i++)
	  {
	     pthread_attr_t attr;
	     cpu_set_t cpu;

	     pthread_attr_init(&attr);
	     CPU_ZERO(&cpu);
	     CPU_SET(i % cpunum, &cpu);
	     pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);
	     task_thinfo[i].thread_num = i;
	     task_thinfo[i].info = NULL;
	     task_thinfo[i].barrier = task_thbarrier;
	     /* setup initial locks */
	     pthread_create(&(task_thinfo[i].thread_id), &attr,
			    evas_common_pipe_load, &(task_thinfo[i]));
	     pthread_attr_destroy(&attr);
	  }
     }

   if (thread_num == 1) return EINA_FALSE;
   return EINA_TRUE;
#endif
   return EINA_FALSE;
}

EAPI void
evas_common_pipe_image_load(RGBA_Image *im)
{
  if (im->flags & RGBA_IMAGE_TODO_LOAD)
    return ;

  if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888
      && !evas_cache_image_is_loaded(&(im->cache_entry)))
    goto add_task;

  if ((!im->cs.data) || ((!im->cs.dirty) && (!(im->flags & RGBA_IMAGE_IS_DIRTY))))
    goto add_task;

  return ;

 add_task:
  LKL(im_task_mutex);
  im_task = eina_list_append(im_task, im);
  LKU(im_task_mutex);
  im->flags |= RGBA_IMAGE_TODO_LOAD;
}

EAPI void
evas_common_pipe_text_prepare(const Evas_Text_Props *text_props)
{
   RGBA_Font_Int *fi;
   const Evas_Text_Props *tmp_props;
   const Eina_List *l;

   if (!text_props->changed) return ;

   fi = text_props->font_instance;
   if (!fi) return ;

   LKL(fi->ft_mutex);

   if (!fi->task) 
     {
       LKL(text_task_mutex);
       text_task = eina_list_append(text_task, fi);
       LKU(text_task_mutex);
     }

   EINA_LIST_FOREACH(fi->task, l, tmp_props)
     if (tmp_props == text_props)
       goto end;

   fi->task = eina_list_append(fi->task, text_props);

 end:
   LKU(fi->ft_mutex);
}

EAPI void
evas_common_pipe_map_begin(RGBA_Image *root)
{
  if (!evas_common_pipe_init())
    {
      RGBA_Image *im;

      EINA_LIST_FREE(im_task, im)
	{
	  if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
	    evas_cache_image_load_data(&im->cache_entry);
	  evas_common_image_colorspace_normalize(im);
	  
	  im->flags &= ~RGBA_IMAGE_TODO_LOAD;
	}
    }

  evas_common_pipe_load_do();

  evas_common_pipe_map_render(root);
}

#endif
