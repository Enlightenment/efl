/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

// THIS IS DEPRECATED. WILL GO EVENTUALLTY. NO NEED TO SUPPORT ANYMORE

#include "evas_common.h"

#ifdef BUILD_PIPE_RENDER

#ifdef EVAS_FRAME_QUEUING
#define SCALECACHE
static Evas_FrameQ gframeq;	// global frameQ

static Evas_Surface *
evas_common_surface_alloc(void *surface, int x, int y, int w, int h)
{
   Evas_Surface *e_surface;

   e_surface = calloc(1, sizeof(Evas_Surface));
   e_surface->im = surface;
   LKL(e_surface->im->ref_fq_add);
   e_surface->im->ref_fq[0]++;
   LKU(e_surface->im->ref_fq_add);
   e_surface->x = x;
   e_surface->y = y;
   e_surface->w = w;
   e_surface->h = h;

   return e_surface;
}

static void
evas_common_surface_dealloc(Evas_Surface *surface)
{
   Evas_Surface *d_surface;

   while(surface)
     {
        d_surface = surface;
        surface = (Evas_Surface *)eina_inlist_remove(EINA_INLIST_GET(surface), EINA_INLIST_GET(d_surface));
        LKL(d_surface->im->ref_fq_del);
        d_surface->im->ref_fq[1]++;
        LKU(d_surface->im->ref_fq_del);

        free(d_surface);

     }
}

static void
evas_common_surface_add(Evas_Frame *frame, Evas_Surface *surface)
{
   frame->surfaces = (Evas_Surface *)eina_inlist_append(EINA_INLIST_GET(frame->surfaces), EINA_INLIST_GET(surface));
}

static Evas_Frame * 
evas_common_frame_alloc()
{
   Evas_Frame *frame;

   frame = calloc(1, sizeof(Evas_Frame));
   frame->surfaces = NULL;
   return frame;
}

static void 
evas_common_frame_dealloc(Evas_Frame *frame)
{
   evas_common_surface_dealloc(frame->surfaces);
   free(frame);
}

static void
evas_common_frame_add(Evas_FrameQ *frameq, Evas_Frame *frame)
{
   Evas_Frame *temp_frame;
   
   LKL(frameq->mutex);
   while(eina_inlist_count(EINA_INLIST_GET(frameq->frames)) >= frameq->frameq_sz)
     {
        /* wait a worker thread finish previous frame */
        pthread_cond_wait(&(frameq->cond_done), &(frameq->mutex));
   }
   frameq->frames = (Evas_Frame *) eina_inlist_append(EINA_INLIST_GET(frameq->frames), EINA_INLIST_GET(frame));

   // this frame need not to be scheduled for flushing time
   EINA_INLIST_FOREACH(EINA_INLIST_GET(frameq->frames), temp_frame)
    {
       if (!temp_frame->ready)
         {
            break;
         }
     }
   if (temp_frame && temp_frame == frame)
      frame->dont_schedule = 1;

   LKU(frameq->mutex);

   pthread_cond_signal(&(frameq->cond_new));
}

EAPI Evas_Surface *
evas_common_frameq_new_surface (void *surface, int x, int y, int w, int h)
{
   return evas_common_surface_alloc(surface, x, y, w, h);
}

EAPI void
evas_common_frameq_add_surface(Evas_Surface *surface)
{
   evas_common_surface_add(gframeq.cur_frame, surface);
}

EAPI void 
evas_common_frameq_set_frame_data(void *data, 
     void (*fn_output_redraws_next_update_push) (void *data, void *surface, int x, int y, int w, int h),
     void (*fn_output_flush)  (void *data),
     void (*fn_output_set_priv)(void *data, void *cur, void *prev))
{
   if (gframeq.cur_frame) 
     {
        gframeq.cur_frame->data = data;
        gframeq.cur_frame->output_redraws_next_update_push = fn_output_redraws_next_update_push;
        gframeq.cur_frame->output_flush = fn_output_flush;
        gframeq.cur_frame->output_set_priv = fn_output_set_priv;
     }
}

EAPI void
evas_common_frameq_prepare_frame()
{
   if (!gframeq.cur_frame )
     {
        gframeq.cur_frame = evas_common_frame_alloc();
     }
}

EAPI void
evas_common_frameq_ready_frame()
{
   if (gframeq.cur_frame)
     {
        evas_common_frame_add(&gframeq, gframeq.cur_frame);
        gframeq.cur_frame = NULL; // create a new frame for the next frame later
     }
}


EAPI void
evas_common_frameq_init()
{
   gframeq.frames = NULL;
   pthread_cond_init(&(gframeq.cond_new), NULL);
   pthread_cond_init(&(gframeq.cond_ready), NULL);
   pthread_cond_init(&(gframeq.cond_done), NULL);
   LKI(gframeq.mutex);
   gframeq.initialised = 0;	// worker thread are not created yet
   gframeq.frameq_sz = 1;	// this value ensures the first frame can be enqueued.
}

EAPI void
evas_common_frameq_destroy()
{
#if 0 // let them destroyed indirectly with program exit
   LKL(gframeq.mutex);
   pthread_cond_destroy(&(gframeq.cond_new));
   pthread_cond_destroy(&(gframeq.cond_ready));
   pthread_cond_destroy(&(gframeq.cond_done));
   LKU(gframeq.mutex);
#endif
   LKD(gframeq.mutex);
   
   gframeq.frames = NULL;
   gframeq.initialised = 0;
}

EAPI void
evas_common_frameq_flush()
{
   if (! evas_common_frameq_enabled())
      return;

   LKL(gframeq.mutex);
   while(eina_inlist_count(EINA_INLIST_GET(gframeq.frames)) > 0)
     {
        /* wait a worker thread finish previous frame */
        pthread_cond_wait(&(gframeq.cond_done), &(gframeq.mutex));
   }
   LKU(gframeq.mutex);
}


EAPI void
evas_common_frameq_flush_ready ()
{
   return;
}

EAPI int
evas_common_frameq_get_frameq_sz()
{
   return gframeq.frameq_sz;
}

EAPI int
evas_common_frameq_enabled()
{
   return gframeq.initialised;
}
#endif

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
//   }
// INF(" TH %i DONE", thinfo->thread_num);
        /* send finished signal */
        pthread_barrier_wait(&(thinfo->barrier[1]));
     }
   return NULL;
}

#ifdef EVAS_FRAME_QUEUING
static void
evas_common_frameq_release(void *data)
{
   Evas_FrameQ *frameq;
   Evas_Frameq_Thread_Info *fq_info;
   Thinfo *thinfo;

   thinfo = data;
   fq_info = (Evas_Frameq_Thread_Info *)(thinfo->fq_info);
   frameq = fq_info->frameq;

   /* This thread may or may not own the mutex.
    * But there's no way to determine the ownership of the mutex, so release it anyway 
    */
   LKU(frameq->mutex);
}

static void *
evas_common_frameq_thread(void *data)
{
   Evas_FrameQ *frameq;
   Evas_Frame *frame;
   Evas_Surface *surface;
   RGBA_Pipe *p;
   Thinfo *thinfo;
   Evas_Frameq_Thread_Info *fq_info;
   RGBA_Pipe_Thread_Info p_info;

   thinfo = data;
   fq_info = (Evas_Frameq_Thread_Info *)(thinfo->fq_info);
   frameq = fq_info->frameq;

   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
   /* install  thread cancelation cleanup handler */
   pthread_cleanup_push(evas_common_frameq_release, data); 

   for (;;)
     {
        frame = NULL;
		
        /* 1. pick a frame to draw */
        LKL(frameq->mutex);
        while(frame == NULL)
          {
             EINA_INLIST_FOREACH(EINA_INLIST_GET(frameq->frames), frame)
               {
                  if (!frame->in_process)
                    {
                       frame->in_process = 1;
                       break;
                    }
               }
             if (frame)
               {
                  break;
               }
             pthread_testcancel();
             pthread_cond_wait(&(frameq->cond_new), &(frameq->mutex));
          }
        LKU(frameq->mutex);

        /* 2. draw selected frame */
        EINA_INLIST_FOREACH(EINA_INLIST_GET(frame->surfaces), surface)
          {
             p_info.im = surface->im;
             p_info.x = 0;
             p_info.y = 0;
             p_info.w = surface->im->cache_entry.w;
             p_info.h = surface->im->cache_entry.h;

             EINA_INLIST_FOREACH(EINA_INLIST_GET(p_info.im->pipe), p)
               {
                  int i;

                  for (i = 0; i < p->op_num; i++)
                    {
                       if (p->op[i].op_func)
                         {
                            p->op[i].op_func(p_info.im, &(p->op[i]), &p_info);
                         }
                    }
               }

             /* push surface out */
             if (! surface->dontpush)
               {
                  frame->output_redraws_next_update_push(frame->data,  
                              surface->im, surface->x, surface->y, surface->w, surface->h);
               }
          }

        // record frame ready time, will be used in post worker thread, evas_common_frameq_thread_post()
        gettimeofday(&frame->ready_time, NULL);

        LKL(frameq->mutex);
        frame->ready = 1;
        pthread_cond_signal(&frameq->cond_ready);
        LKU(frameq->mutex);
     }

   // Remove cleanup handler
   pthread_cleanup_pop(0);
   return NULL;
}


#define INTERVAL_QSIZE 17  // Actual size is 'INTERVAL_QSIZE - 1' because of not using index
#define SATISFACTION_THRESHOLD 4 // 4 ms --> 250 FPS
#define RESET_RATIO 4   // RESET_RATIO * [Average Ready Gap | get_max_interval()] --> Reset Threshold
#define DOUBLE_RESET_TIME_INTERVAL_THRESHOLD 16000 // make it double in case of less 16ms
#define RESET_ABSOLUTE_INTERVAL 600000  // 600 msec

struct iq_node
{
   long long rt;
   long long ri;
};

static struct iq_node _IQ[INTERVAL_QSIZE];
static int _IQ_head = 0, _IQ_tail = 0;
static int _IQ_length = 0;
static long long min_ready, max_ready;
static long long average_interval;

static int 
_IQ_next_index(int i)
{
   return (i + 1) % INTERVAL_QSIZE;
}

static int 
_IQ_previous_index(int i)
{
   if (--i < 0) i += INTERVAL_QSIZE;
   return i;
}

static void 
_IQ_init(void)
{
   _IQ_length = _IQ_head = _IQ_tail = 0;
   min_ready = LLONG_MAX, max_ready = LLONG_MIN;
   average_interval = 0;
}

static int 
_IQ_empty(void)
{
   return (_IQ_head == _IQ_tail) ? 1 : 0;
}

static int 
_IQ_full(void)
{
   return (_IQ_head == ((_IQ_tail + 1) % INTERVAL_QSIZE)) ? 1 : 0;
}

static void 
_IQ_insert(long long ready_time, long long last_interval)
{
   if (_IQ_full()) return;

   if (_IQ_empty())
     {
        if (last_interval < 0)
          {
             last_interval = -last_interval;
          }
        _IQ[_IQ_tail].rt = ready_time;
        _IQ[_IQ_tail].ri = last_interval;
        min_ready = ready_time - last_interval;
        max_ready = ready_time;
        _IQ_tail = _IQ_next_index(_IQ_tail);
        _IQ_length++;
     }
   else
     {
        if (max_ready < ready_time)
          {
             _IQ[_IQ_tail].rt = ready_time;
             _IQ[_IQ_tail].ri = ready_time - max_ready;
             _IQ_tail = _IQ_next_index(_IQ_tail);
             _IQ_length++;
             max_ready = ready_time;
          }
        else if (ready_time < min_ready)
          {
             last_interval = _IQ[_IQ_head].ri;
             _IQ[_IQ_head].ri = _IQ[_IQ_head].rt - ready_time;
             _IQ_head = _IQ_previous_index(_IQ_head);
             _IQ[_IQ_head].rt = ready_time;
             _IQ[_IQ_head].ri = last_interval;
             min_ready = ready_time;
             _IQ_length++;
          }
        else
          {
             int i, j, k, l = 0;
             for (i = _IQ_head; i != _IQ_tail; i = j)
               {
                  j = _IQ_next_index(i);
                  if (_IQ[j].rt < ready_time)
                    {
                       continue;
                    }
                  break;
               }
             for (k = _IQ_tail; k != j; k = l)
               {
                  l = _IQ_previous_index(k);
                  _IQ[k] = _IQ[l];
               }
             i = _IQ_next_index(j);
             _IQ[j].ri -= (_IQ[j].rt - ready_time);
             _IQ[j].rt = ready_time;
             _IQ[i].ri = _IQ[i].rt - ready_time;
             _IQ_tail = _IQ_next_index(_IQ_tail);
             _IQ_length++;
          }
     }
   average_interval = (max_ready - min_ready) / _IQ_length;
}

static long long 
_IQ_delete()
{
   struct iq_node oldest;
   if (_IQ_empty()) return 0;
   oldest = _IQ[_IQ_head];
   _IQ_head = ++_IQ_head % INTERVAL_QSIZE;
   if (--_IQ_length == 0)
     {
        _IQ_init();
     }
   else
     {
        min_ready = _IQ[_IQ_head].rt;
        average_interval = (max_ready - min_ready) / _IQ_length;
     }

   return oldest.ri;
}

static long long 
get_max_interval()
{
   int i;
   long long max = LLONG_MIN;

   for ( i= _IQ_head ; i != _IQ_tail ; i = _IQ_next_index(i))
     {
        if (_IQ[i].ri > max)
          {
             max = _IQ[i].ri;
          }
     }

   return max;
}

static long long 
tv_to_long_long(struct timeval *tv)
{
   if (tv == NULL)
     {
        return 0;
     }

   return tv->tv_sec * 1000000LL + tv->tv_usec;
}

static long long
evas_common_frameq_schedule_flush_time( 
                     int frameq_sz, int thread_no, 
                     long long last_ready_time, long long current_ready_time,
                     long long last_flush_time, int ready_frames_num,
                     int dont_schedule)
{
   // to get each time and to do others
   long long current_time = 0LL;
   long long current_ready_interval = 0LL;
   long long theshold_time = SATISFACTION_THRESHOLD * 1000LL;	// ms -> usec
   long long reset_time_interval;
   long long sleep_time = 0LL;
   long long saved_ready_time, saved_ready_interval;
   long long time_slept = 0LL;
   static long long time_lag = 0;
   struct timeval now;
   int frameq_full_threshold =0;
   int need_reset = 0;
   int need_schedule = 0;

   frameq_full_threshold = frameq_sz -thread_no;	// Qsize - threads#
   
   /* 1.5 defer flush time of current frame if need */
   // in case of the first time, just keep ready time only
   if (last_ready_time == 0LL)
     {
        last_ready_time = current_ready_time;
     }
   else
     {
        /* 1.5.1 get current ready time & interval */
        saved_ready_time = current_ready_time;
        saved_ready_interval = current_ready_interval = current_ready_time - last_ready_time;
        // compensate a case which current ready time is older than previous one, 
        // doesn't work on the interval queue
        if (current_ready_interval < 0)
          {
             current_ready_time = last_ready_time;
             current_ready_interval = 0;
          }

        /* 1.5.2 get the reset time interval before keeping a new one */
        if (!_IQ_empty())
          {
             reset_time_interval = RESET_RATIO * average_interval;
             if (average_interval < DOUBLE_RESET_TIME_INTERVAL_THRESHOLD) 
               {
                  reset_time_interval *= 2;
               }
          }

        /* 1.5.3 reset - if too late, discard all saved interval and start from here */
        if (current_ready_interval > RESET_ABSOLUTE_INTERVAL)
          {
             need_reset = 1;
          }
        else if (_IQ_length >= thread_no * 2 && current_ready_interval > reset_time_interval)
          {
             need_reset = 1;
          }
        else if (_IQ_length >= thread_no && _IQ_length < thread_no * 2 
             && current_ready_interval > get_max_interval() * RESET_RATIO)
          {
             need_reset = 1;
          }
       
        if (need_reset)
          {
             _IQ_init();
          }
        else
          {
             /* 1.5.4 enqueue - keep a new interval for next average interval */
             if (_IQ_full())
               {
                  _IQ_delete();
               }
             _IQ_insert(saved_ready_time, saved_ready_interval);

             /* 1.5.5 schedule - if faster than average interval, figure out sleep time to meet it */
             if (!dont_schedule)
               {
                  need_schedule = 0;
                  sleep_time = 0;
                  if (_IQ_length >= thread_no * 2 && average_interval > theshold_time)
                    {
                       need_schedule = 1;
                    }
                  // compensate the case that postworker blocks the workers from getting a new fresh frame
                  // It's actually occurred when during the wait time of postworker, the frame queue is full
                  // Consequently check the number of currently ready frames and apply some time drop to average time according to the number
                  if (ready_frames_num >= frameq_full_threshold)
                    {
                       need_schedule = 0;
                    }
                  if (need_schedule)
                    {
                       gettimeofday(&now, NULL);
                       current_time = tv_to_long_long(&now);
                       time_lag += (current_time - last_flush_time);
                       sleep_time = (average_interval < time_lag) ? 0 : (average_interval - time_lag);
                    }
               }

             /* 1.5.6 sleep - actually sleep and get over-slept time (time_lag) for next frame */
             if (sleep_time > 0)
               {
                  sleep_time = sleep_time * 9 / 10;
                  usleep((unsigned int)sleep_time);
                  gettimeofday(&now, NULL);
                  time_slept = tv_to_long_long(&now) - current_time;
                  time_lag = time_slept - sleep_time;
               }
             else
               {
                  time_lag = 0;
               }
          }
        last_ready_time = current_ready_time;
     }

   return last_ready_time;
}

static void *
evas_common_frameq_thread_post(void *data)
{
   Evas_FrameQ *frameq;
   Evas_Frame *frame;
   Evas_Surface *surface;
   RGBA_Pipe *p;
   Thinfo *thinfo;
   Evas_Frameq_Thread_Info *fq_info;
   RGBA_Pipe_Thread_Info p_info;
   Eina_List   *pending_writes = NULL;
   Eina_List   *prev_pending_writes = NULL;

   long long last_ready_time = 0LL;
   long long current_ready_time;
   Evas_Frame *temp_frame = NULL;
   int ready_frames_num;
   long long last_flush_time = 0LL;
   struct timeval now;
   int dont_schedule = 0;

   thinfo = data;
   fq_info = (Evas_Frameq_Thread_Info *)(thinfo->fq_info);
   frameq = fq_info->frameq;

   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
   /* install  thread cancelation cleanup handler */
   pthread_cleanup_push(evas_common_frameq_release, data); 

   _IQ_init();

   for (;;)
     {
        /* 1. wait the first frame being done */
        LKL(frameq->mutex);
        while(!frameq->frames || !frameq->frames->ready)
          {
             pthread_cond_wait(&(frameq->cond_ready), &(frameq->mutex));
          }
        frame = frameq->frames;

        /* 1.5. prepare to schedule flush time */
        current_ready_time = tv_to_long_long(&frame->ready_time);
        ready_frames_num = 0;
        EINA_INLIST_FOREACH(EINA_INLIST_GET(frameq->frames), temp_frame)
          {
             if (temp_frame->ready == 1)
               {
                  ready_frames_num++;
               }
          }
        dont_schedule = (frame->dont_schedule)?1:0;
        LKU(frameq->mutex);

        /* 2. generate pending_writes */
        EINA_INLIST_FOREACH(EINA_INLIST_GET(frame->surfaces), surface)
         {
            evas_common_pipe_flush(surface->im);
            if (! surface->dontpush)
              {
                 pending_writes = eina_list_append(pending_writes, surface->im);
              }
         }

        /* 2.5. schedule flush time */
        last_ready_time = evas_common_frameq_schedule_flush_time(
                                       frameq->frameq_sz, frameq->thread_num, 
                                       last_ready_time, current_ready_time,
                                       last_flush_time, ready_frames_num, dont_schedule);

        /* 3. flush redraws */
        frame->output_set_priv(frame->data, pending_writes, prev_pending_writes);
        frame->output_flush(frame->data);
        gettimeofday(&now, NULL);
        // keep as the last flush time
        last_flush_time = now.tv_sec * 1000000LL + now.tv_usec;

        prev_pending_writes = pending_writes;
        pending_writes = NULL;

        /* 4. remove this frame from the frame queue */
        LKL(frameq->mutex);
        frameq->frames = 
            (Evas_Frame *)eina_inlist_remove(EINA_INLIST_GET(frameq->frames), 
                  EINA_INLIST_GET(frame));

        LKU(frameq->mutex);
        pthread_cond_broadcast(&frameq->cond_done);
        evas_common_frame_dealloc(frame);
     }

   // Remove cleanup handler
   pthread_cleanup_pop(0);
   return NULL;
}

#endif /* EVAS_FRAME_QUEUING */
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

#ifdef EVAS_FRAME_QUEUING
   return;
#endif

   if (!im->pipe) return;
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
          {
             info->h = im->cache_entry.h - y;
          }
        else
          {
             info->h = h;
          }
        y += info->h;
#endif
        thinfo[i].info = info;
     }
   /* tell worker threads to start */
   pthread_barrier_wait(&(thbarrier[0]));
#endif
}

#ifdef EVAS_FRAME_QUEUING
EAPI void
evas_common_frameq_begin()
{
#ifdef BUILD_PTHREAD
   int i;
   Evas_Frameq_Thread_Info *fp_info;
   pthread_attr_t attr;
   cpu_set_t cpu;

   int set_cpu_affinity=1;
	
	if (!gframeq.initialised)
     {
        int cpunum;

        cpunum = eina_cpu_count();
        gframeq.thread_num = cpunum;
        gframeq.frameq_sz = cpunum * FRAMEQ_SZ_PER_THREAD;

        for (i = 0; i < gframeq.thread_num; i++)
          {

             fp_info = calloc(1, sizeof(Evas_Frameq_Thread_Info));
             fp_info->frameq = &gframeq;

             gframeq.thinfo[i].thread_num = i;
             gframeq.thinfo[i].fq_info = fp_info;

             pthread_attr_init(&attr);
             if (set_cpu_affinity)
               {
                  CPU_ZERO(&cpu);
                  CPU_SET((i+1) % cpunum, &cpu);
                  pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);
               }

             pthread_create(&(gframeq.thinfo[i].thread_id), &attr,
                     evas_common_frameq_thread, &(gframeq.thinfo[i]));
				
             pthread_attr_destroy(&attr);
             pthread_detach(gframeq.thinfo[i].thread_id);
          }

          {
             fp_info = calloc(1, sizeof(Evas_Frameq_Thread_Info));
             fp_info->frameq = &gframeq;

             gframeq.thinfo[i].thread_num = i;
             gframeq.thinfo[i].fq_info = fp_info;

             pthread_attr_init(&attr);
             if (set_cpu_affinity)
               {
                  CPU_ZERO(&cpu);
                  CPU_SET((i+1) % cpunum, &cpu);
                  pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);
               }

             pthread_create(&(gframeq.thinfo[i].thread_id), &attr,
                     evas_common_frameq_thread_post, &(gframeq.thinfo[i]));
             pthread_attr_destroy(&attr);
             pthread_detach(gframeq.thinfo[i].thread_id);
          }
        gframeq.initialised = 1;	// now worker threads are created.
     }
#endif /* BUILD_PTHREAD */
}

EAPI void
evas_common_frameq_finish()
{
   int i;
   
   /* 1. cancel all worker threads */
   for (i = 0; i < gframeq.thread_num; i++)
     {
        pthread_cancel(gframeq.thinfo[i].thread_id);
     }
     // cancel post-worker thread
     pthread_cancel(gframeq.thinfo[i].thread_id);

   /* 2. send signal to worker threads so that they enter to the thread cancelation cleanup handler */
   for (i = 0; i < gframeq.thread_num; i++)
     {
        pthread_cond_signal(&(gframeq.cond_new));
     }
   // send signal to post-worker thread
   pthread_cond_signal(&(gframeq.cond_ready));

   /* all the workers were created and detached before
    *  so don't need to join them here.
    */

}

#endif /* EVAS_FRAME_QUEUING */

EAPI void
evas_common_pipe_flush(RGBA_Image *im)
{

   RGBA_Pipe *p;
   int i;

   if (!im->pipe) return;

#ifndef EVAS_FRAME_QUEUING

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
                    {
                       p->op[i].op_func(im, &(p->op[i]), NULL);
                    }
               }
          }
     }
#endif /* !EVAS_FRAME_QUEUING */
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
               {
                  p->op[i].free_func(&(p->op[i]));
               }
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
     {
        evas_common_rectangle_draw(dst, &(op->context),
               op->op.rect.x, op->op.rect.y,
               op->op.rect.w, op->op.rect.h);
     }
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
   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) return;
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

/**************** GRAD ******************/
static void
evas_common_pipe_op_grad_free(RGBA_Pipe_Op *op)
{
#ifdef EVAS_FRAME_QUEUING
   LKL(op->op.grad.grad->ref_fq_del);
   op->op.grad.grad->ref_fq[1]++;
   LKU(op->op.grad.grad->ref_fq_del);
   pthread_cond_signal(&(op->op.grad.grad->cond_fq_del)); 
#else
   evas_common_gradient_free(op->op.grad.grad);
#endif
   evas_common_pipe_op_free(op);
}

#ifdef EVAS_FRAME_QUEUING
EAPI void
evas_common_pipe_op_grad_flush(RGBA_Gradient *gr)
{
   if (! evas_common_frameq_enabled())
      return;

   LKL(gr->ref_fq_add);
   LKL(gr->ref_fq_del);

   while (gr->ref_fq[0] != gr->ref_fq[1])
      pthread_cond_wait(&(gr->cond_fq_del), &(gr->ref_fq_del));

   LKU(gr->ref_fq_del);
   LKU(gr->ref_fq_add);
}
#endif

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
     {
        evas_common_gradient_draw(dst, &(op->context),
                  op->op.grad.x, op->op.grad.y,
                  op->op.grad.w, op->op.grad.h,
                  op->op.grad.grad);
     }
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
#ifdef EVAS_FRAME_QUEUING
   LKL(gr->ref_fq_add);
   gr->ref_fq[0]++;
   LKU(gr->ref_fq_add);
#else
   gr->references++;
#endif
   op->op.grad.grad = gr;
   op->op_func = evas_common_pipe_grad_draw_do;
   op->free_func = evas_common_pipe_op_grad_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** GRAD2 ******************/
static void
evas_common_pipe_op_grad2_free(RGBA_Pipe_Op *op)
{
#ifdef EVAS_FRAME_QUEUING
   LKL(op->op.grad2.grad->ref_fq_del);
   op->op.grad2.grad->ref_fq[1]++;
   LKU(op->op.grad2.grad->ref_fq_del);
   pthread_cond_signal(&(op->op.grad2.grad->cond_fq_del)); 
#else
   evas_common_gradient2_free(op->op.grad2.grad);
#endif
   evas_common_pipe_op_free(op);
}

#ifdef EVAS_FRAME_QUEUING
EAPI void
evas_common_pipe_op_grad2_flush(RGBA_Gradient2 *gr)
{
   if (! evas_common_frameq_enabled())
      return;

   LKL(gr->ref_fq_add);
   LKL(gr->ref_fq_del);

   while (gr->ref_fq[0] != gr->ref_fq[1])
      pthread_cond_wait(&(gr->cond_fq_del), &(gr->ref_fq_del));

   LKU(gr->ref_fq_del);
   LKU(gr->ref_fq_add);
}
#endif

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
     {
        evas_common_gradient2_draw(dst, &(op->context),
               op->op.grad2.x, op->op.grad2.y,
               op->op.grad2.w, op->op.grad2.h,
               op->op.grad2.grad);
     }
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
#ifdef EVAS_FRAME_QUEUING
   LKL(gr->ref_fq_add);
   gr->ref_fq[0]++;
   LKU(gr->ref_fq_add);
#else
   gr->references++;
#endif
   op->op.grad2.grad = gr;
   op->op_func = evas_common_pipe_grad2_draw_do;
   op->free_func = evas_common_pipe_op_grad2_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** TEXT ******************/
static void
evas_common_pipe_op_text_free(RGBA_Pipe_Op *op)
{
#ifdef EVAS_FRAME_QUEUING
   LKL(op->op.text.font->ref_fq_del);
   op->op.text.font->ref_fq[1]++;
   LKU(op->op.text.font->ref_fq_del);
   pthread_cond_signal(&(op->op.text.font->cond_fq_del));
#else
   evas_common_font_free(op->op.text.font);
#endif
   free(op->op.text.text);
   evas_common_pipe_op_free(op);
}

#ifdef EVAS_FRAME_QUEUING
/* flush all op using @fn */
EAPI void
evas_common_pipe_op_text_flush(RGBA_Font *fn)
{
   if (! evas_common_frameq_enabled())
      return;

   LKL(fn->ref_fq_add);
   LKL(fn->ref_fq_del);

   while (fn->ref_fq[0] != fn->ref_fq[1])
      pthread_cond_wait(&(fn->cond_fq_del), &(fn->ref_fq_del));

   LKU(fn->ref_fq_del);
   LKU(fn->ref_fq_add);
}
#endif

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
     {
        evas_common_font_draw(dst, &(op->context),
                  op->op.text.font, op->op.text.x, op->op.text.y,
                  op->op.text.text);
     }
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
#ifdef EVAS_FRAME_QUEUING
   LKL(fn->ref_fq_add);
   fn->ref_fq[0]++;
   LKU(fn->ref_fq_add);
#else
   fn->references++;
#endif
   op->op.text.font = fn;
   op->op_func = evas_common_pipe_text_draw_do;
   op->free_func = evas_common_pipe_op_text_free;
   evas_common_pipe_draw_context_copy(dc, op);
}

/**************** IMAGE *****************/
static void
evas_common_pipe_op_image_free(RGBA_Pipe_Op *op)
{
#ifdef EVAS_FRAME_QUEUING
   LKL(op->op.image.src->ref_fq_del);
   op->op.image.src->ref_fq[1]++;
   LKU(op->op.image.src->ref_fq_del);
   pthread_cond_signal(&(op->op.image.src->cond_fq_del)); 
#else
   op->op.image.src->ref--;
   if (op->op.image.src->ref == 0)
     {
        evas_cache_image_drop(&op->op.image.src->cache_entry);
     }
#endif
   evas_common_pipe_op_free(op);
}

#ifdef EVAS_FRAME_QUEUING
EAPI void
evas_common_pipe_op_image_flush(RGBA_Image *im)
{
   if (! evas_common_frameq_enabled())
      return;
   
   LKL(im->ref_fq_add);
   LKL(im->ref_fq_del);

   while (im->ref_fq[0] != im->ref_fq[1])
      pthread_cond_wait(&(im->cond_fq_del), &(im->ref_fq_del));

   LKU(im->ref_fq_del);
   LKU(im->ref_fq_add);
}
#endif

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

#ifdef SCALECACHE
        evas_common_rgba_image_scalecache_do(op->op.image.src,
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
        evas_common_rgba_image_scalecache_do(op->op.image.src,
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
#ifdef EVAS_FRAME_QUEUING
   LKL(src->ref_fq_add);
   src->ref_fq[0]++;
   LKU(src->ref_fq_add);
#else
   src->ref++;
#endif
   op->op.image.src = src;
   op->op_func = evas_common_pipe_image_draw_do;
   op->free_func = evas_common_pipe_op_image_free;
   evas_common_pipe_draw_context_copy(dc, op);

#ifdef EVAS_FRAME_QUEUING
   /* laod every src image here.
    * frameq utilize all cpu cores already by worker threads
    * so another threads and barrier waiting can't be of any benefit.
    * therefore, not instantiate loader threads.
    */
   if (src->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
      evas_cache_image_load_data(&src->cache_entry);
   evas_common_image_colorspace_normalize(src);
#else
   evas_common_pipe_image_load(src);
#endif
}

static void
evas_common_pipe_op_map4_free(RGBA_Pipe_Op *op)
{
#ifdef EVAS_FRAME_QUEUING
   LKL(op->op.image.src->ref_fq_del);
   op->op.image.src->ref_fq[1]++;
   LKU(op->op.image.src->ref_fq_del);
#else
   op->op.map4.src->ref--;
   if (op->op.map4.src->ref == 0)
     evas_cache_image_drop(&op->op.map4.src->cache_entry);
#endif
   free(op->op.map4.p);
   evas_common_pipe_op_free(op);
}

static void
evas_common_pipe_map4_draw_do(RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info)
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

	evas_common_map4_rgba(op->op.map4.src, dst,
			      &context, op->op.map4.p,
			      op->op.map4.smooth, op->op.map4.level);
     }
   else
     {
	evas_common_map4_rgba(op->op.map4.src, dst,
			      &(op->context), op->op.map4.p,
			      op->op.map4.smooth, op->op.map4.level);
     }
}

EAPI void
evas_common_pipe_map4_draw(RGBA_Image *src, RGBA_Image *dst,
			   RGBA_Draw_Context *dc, RGBA_Map_Point *p,
			   int smooth, int level)
{
   RGBA_Pipe_Op *op;
   RGBA_Map_Point *pts_copy;
   int i;

   if (!src) return;
   pts_copy = malloc(sizeof (RGBA_Map_Point) * 4);
   if (!pts_copy) return;
   dst->pipe = evas_common_pipe_add(dst->pipe, &op);
   if (!dst->pipe) 
     {
       free(pts_copy);
       return; 
     }

   for (i = 0; i < 4; ++i)
     pts_copy[i] = p[i];

   op->op.map4.smooth = smooth;
   op->op.map4.level = level;
#ifdef EVAS_FRAME_QUEUING
   LKL(src->ref_fq_add);
   src->ref_fq[0]++;
   LKU(src->ref_fq_add);
#else
   src->ref++;
#endif
   op->op.map4.src = src;
   op->op.map4.p = pts_copy;
   op->op_func = evas_common_pipe_map4_draw_do;
   op->free_func = evas_common_pipe_op_map4_free;
   evas_common_pipe_draw_context_copy(dc, op);

#ifdef EVAS_FRAME_QUEUING
   /* laod every src image here.
    * frameq utilize all cpu cores already by worker threads
    * so another threads and barrier waiting can't be of any benefit.
    * therefore, not instantiate loader threads.
    */
   if (src->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
      evas_cache_image_load_data(&src->cache_entry);
   evas_common_image_colorspace_normalize(src);
#else
   evas_common_pipe_image_load(src);
#endif
}

static void
evas_common_pipe_map4_render(RGBA_Image *root)
{
  RGBA_Pipe *p;
  int i;

  /* Map imply that we need to process them recursively first. */
  for (p = root->pipe; p; p = (RGBA_Pipe *)(EINA_INLIST_GET(p))->next)
    {
      for (i = 0; i < p->op_num; i++) 
	{
	  if (p->op[i].op_func == evas_common_pipe_map4_draw_do)
	    {
	      if (p->op[i].op.map4.src->pipe)
		evas_common_pipe_map4_render(p->op[i].op.map4.src);
	    }
	  else if (p->op[i].op_func == evas_common_pipe_image_draw_do)
	    {
	      if (p->op[i].op.image.src->pipe)
		evas_common_pipe_map4_render(p->op[i].op.image.src);
	    }
	}
    }

  evas_common_pipe_begin(root);
  evas_common_pipe_flush(root);
}

#ifdef BUILD_PTHREAD
static Eina_List *task = NULL;
static Thinfo task_thinfo[TH_MAX];
static pthread_barrier_t task_thbarrier[2];
static LK(task_mutext) = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifdef BUILD_PTHREAD
static void*
evas_common_pipe_load(void *data)
{
  Thinfo *thinfo;

  thinfo = data;
  for (;;)
    {
      /* wait for start signal */
      pthread_barrier_wait(&(thinfo->barrier[0]));

      while (task)
	{
	  RGBA_Image *im = NULL;

	  LKL(task_mutext);
	  im = eina_list_data_get(task);
	  task = eina_list_remove_list(task, task);
	  LKU(task_mutext);

	  if (im)
	    {
	      if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
		evas_cache_image_load_data(&im->cache_entry);
	      evas_common_image_colorspace_normalize(im);

	      im->flags &= ~RGBA_IMAGE_TODO_LOAD;
	    }
	}

      /* send finished signal */    
      pthread_barrier_wait(&(thinfo->barrier[1]));
    }

  return NULL;
}
#endif

static void
evas_common_pipe_image_load_do(void)
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
	if (thread_num == 1) return EINA_FALSE;

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
  task = eina_list_append(task, im);
  im->flags |= RGBA_IMAGE_TODO_LOAD;
}

EAPI void
evas_common_pipe_map4_begin(RGBA_Image *root)
{
  if (!evas_common_pipe_init())
    {
      RGBA_Image *im;

      EINA_LIST_FREE(task, im)
	{
	  if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
	    evas_cache_image_load_data(&im->cache_entry);
	  evas_common_image_colorspace_normalize(im);
	  
	  im->flags &= ~RGBA_IMAGE_TODO_LOAD;
	}
    }

  evas_common_pipe_image_load_do();

  evas_common_pipe_map4_render(root);
}

#endif
