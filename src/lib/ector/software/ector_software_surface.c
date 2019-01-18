#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

#define MY_CLASS ECTOR_SOFTWARE_SURFACE_CLASS

typedef struct _Ector_Software_Task Ector_Software_Task;

struct _Ector_Software_Task
{
   Eina_Thread_Queue_Msg member;

   Ector_Thread_Worker_Cb cb;
   Eina_Free_Cb done;
   void *data;
};

static int _count_init = 0;
static unsigned int current = 0;
static unsigned int cpu_core = 0;
static Ector_Software_Thread *ths = NULL;
static Eina_Thread_Queue *render_queue = NULL;
static Ector_Software_Thread render_thread;

static void *
_prepare_process(void *data, Eina_Thread t)
{
   Ector_Software_Thread *th = data;

   eina_thread_name_set(t, "Ector Preparing Thread");
   do
     {
        Ector_Software_Task *task, todo;
        void *ref;

        task = eina_thread_queue_wait(th->queue, &ref);

        if (!task) break ;
        todo.cb = task->cb;
        todo.data = task->data;
        todo.done = task->done;

        eina_thread_queue_wait_done(th->queue, ref);

        if (!todo.cb) break ;

        todo.cb(todo.data, th);

        task = eina_thread_queue_send(render_queue, sizeof (Ector_Software_Task), &ref);
        task->cb = todo.cb;
        task->data = todo.data;
        task->done = todo.done;
        eina_thread_queue_send_done(render_queue, ref);
     }
   while (1);

   return th;
}


static void
_ector_software_init(void)
{
   int cpu, i;

   ++_count_init;
   if (_count_init != 1) return;

   cpu = eina_cpu_count() - 1;
   if (cpu < 1)
     {
        render_thread.queue = NULL;
        ector_software_thread_init(&render_thread);
        return ;
     }
   cpu = cpu > 8 ? 8 : cpu;
   cpu_core = cpu;

   render_queue = eina_thread_queue_new();

   ths = malloc(sizeof(Ector_Software_Thread) * cpu);
   for (i = 0; i < cpu; i++)
     {
        Ector_Software_Thread *t;

        t = &ths[i];
        t->queue = eina_thread_queue_new();
        ector_software_thread_init(t);
        if (!eina_thread_create(&t->thread, EINA_THREAD_NORMAL, -1,
                                _prepare_process, t))
          {
             eina_thread_queue_free(t->queue);
             t->queue = NULL;
          }
     }
}

static void
_ector_software_shutdown(void)
{
   Ector_Software_Thread *t;
   unsigned int i;

   --_count_init;
   if (_count_init != 0) return;

   if (!ths)
     {
        ector_software_thread_shutdown(&render_thread);
        return ;
     }

   for (i = 0; i < cpu_core; i++)
     {
        Ector_Software_Task *task;
        void *ref;

        t = &ths[i];

        task = eina_thread_queue_send(t->queue, sizeof (Ector_Software_Task), &ref);
        task->cb = NULL;
        task->data = NULL;
        eina_thread_queue_send_done(t->queue, ref);

        eina_thread_join(t->thread);
        eina_thread_queue_free(t->queue);
        ector_software_thread_shutdown(t);
     }

   eina_thread_queue_free(render_queue);
   render_queue = NULL;

   free(ths);
   ths = NULL;
}

void
ector_software_schedule(Ector_Thread_Worker_Cb cb, Eina_Free_Cb done, void *data)
{
   Ector_Software_Thread *t;
   Ector_Software_Task *task;
   void *ref;

   // Not enough CPU, doing it inline in the rendering thread
   if (!ths) return ;

   t = &ths[current];
   current = (current + 1) % cpu_core;

   task = eina_thread_queue_send(t->queue, sizeof (Ector_Software_Task), &ref);
   task->cb = cb;
   task->done = done;
   task->data = data;
   eina_thread_queue_send_done(t->queue, ref);
}

// Do not call this function if the done function has already called
void
ector_software_wait(Ector_Thread_Worker_Cb cb, Eina_Free_Cb done, void *data)
{
   Ector_Software_Task *task, covering;

   // First handle case with just inlined prepare code call inside the rendering thread
   if (!ths)
     {
        render_thread.thread = eina_thread_self();
        cb(data, &render_thread);
        done(data);

        return ;
     }

   // We don't know which task is going to be done first, so
   // we iterate until we find ourself back and trigger all
   // the done call along the way.
   do
     {
        void *ref;

        task = eina_thread_queue_wait(render_queue, &ref);
        if (!task) break;
        covering.cb = task->cb;
        covering.done = task->done;
        covering.data = task->data;
        eina_thread_queue_wait_done(render_queue, ref);

        covering.done(covering.data);
     }
   while (covering.cb != cb ||
          covering.done != done ||
          covering.data != data);
}

static Ector_Renderer *
_ector_software_surface_ector_surface_renderer_factory_new(Eo *obj,
                                                           Ector_Software_Surface_Data *pd EINA_UNUSED,
                                                           const Efl_Class *type)
{
   if (type == ECTOR_RENDERER_SHAPE_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_SOFTWARE_SHAPE_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_LINEAR_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_RADIAL_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));

   ERR("Couldn't find class for type: %s", efl_class_name_get(type));
   return NULL;
}

static Eo *
_ector_software_surface_efl_object_constructor(Eo *obj, Ector_Software_Surface_Data *pd)
{
   _ector_software_init();

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->rasterizer = (Software_Rasterizer *) calloc(1, sizeof(Software_Rasterizer));
   ector_software_rasterizer_init(pd->rasterizer);
   pd->rasterizer->fill_data.raster_buffer = efl_data_ref(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   return obj;
}

static void
_ector_software_surface_efl_object_destructor(Eo *obj, Ector_Software_Surface_Data *pd)
{
   efl_data_unref(obj, pd->rasterizer->fill_data.raster_buffer);
   free(pd->rasterizer);
   pd->rasterizer = NULL;
   efl_destructor(efl_super(obj, ECTOR_SOFTWARE_SURFACE_CLASS));

   _ector_software_shutdown();
}

static void
_ector_software_surface_ector_surface_reference_point_set(Eo *obj EINA_UNUSED,
                                                          Ector_Software_Surface_Data *pd,
                                                          int x, int y)
{
   pd->x = x;
   pd->y = y;
}

#include "ector_software_surface.eo.c"
#include "ector_renderer_software.eo.c"
