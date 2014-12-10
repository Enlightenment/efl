#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_PULSE
#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Audio.h"
#include "ecore_audio_private.h"

#include <pulse/pulseaudio.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

int _ecore_audio_pa_log_dom = -1;
static Ecore_Audio_Module *pulse_module = NULL;

/* Ecore mainloop integration start */
struct pa_io_event
{
   struct _Ecore_Audio_Pa_Private *mainloop;
   Ecore_Fd_Handler               *handler;

   void                           *userdata;

   pa_io_event_flags_t             flags;
   pa_io_event_cb_t                callback;
   pa_io_event_destroy_cb_t        destroy_callback;
};

static Ecore_Fd_Handler_Flags
map_flags_to_ecore(pa_io_event_flags_t flags)
{
   return (Ecore_Fd_Handler_Flags)((flags & PA_IO_EVENT_INPUT ? ECORE_FD_READ : 0) | (flags & PA_IO_EVENT_OUTPUT ? ECORE_FD_WRITE : 0) | (flags & PA_IO_EVENT_ERROR ? ECORE_FD_ERROR : 0) | (flags & PA_IO_EVENT_HANGUP ? ECORE_FD_READ : 0));
}

static Eina_Bool
_ecore_io_wrapper(void *data, Ecore_Fd_Handler *handler)
{
   char buf[64];
   pa_io_event_flags_t flags = 0;
   pa_io_event *event = (pa_io_event *)data;

   if (ecore_main_fd_handler_active_get(handler, ECORE_FD_READ))
     {
        flags |= PA_IO_EVENT_INPUT;

        /* Check for HUP and report */
        if (recv(ecore_main_fd_handler_fd_get(handler), buf, 64, MSG_PEEK))
          {
             if (errno == ESHUTDOWN || errno == ECONNRESET || errno == ECONNABORTED || errno == ENETRESET)
               {
                  DBG("HUP condition detected");
                  flags |= PA_IO_EVENT_HANGUP;
               }
          }
     }

   if (ecore_main_fd_handler_active_get(handler, ECORE_FD_WRITE))
     flags |= PA_IO_EVENT_OUTPUT;
   if (ecore_main_fd_handler_active_get(handler, ECORE_FD_ERROR))
     flags |= PA_IO_EVENT_ERROR;

   event->callback(&event->mainloop->api, event, ecore_main_fd_handler_fd_get(handler), flags, event->userdata);

   return ECORE_CALLBACK_RENEW;
}

static pa_io_event *
_ecore_pa_io_new(pa_mainloop_api *api, int fd, pa_io_event_flags_t flags, pa_io_event_cb_t cb, void *userdata)
{
   pa_io_event *event;
   struct _Ecore_Audio_Pa_Private *mloop;

   mloop = api->userdata;

   event = calloc(1, sizeof(pa_io_event));
   event->mainloop = mloop;
   event->userdata = userdata;
   event->callback = cb;
   event->flags = flags;
   event->handler = ecore_main_fd_handler_add(fd, map_flags_to_ecore(flags), _ecore_io_wrapper, event, NULL, NULL);

   return event;
}

static void
_ecore_pa_io_enable(pa_io_event *event, pa_io_event_flags_t flags)
{
   event->flags = flags;
   ecore_main_fd_handler_active_set(event->handler, map_flags_to_ecore(flags));
}

static void
_ecore_pa_io_free(pa_io_event *event)
{
   ecore_main_fd_handler_del(event->handler);
   free(event);
}

static void
_ecore_pa_io_set_destroy(pa_io_event *event, pa_io_event_destroy_cb_t cb)
{
   event->destroy_callback = cb;
}

/* Timed events */
struct pa_time_event
{
   struct _Ecore_Audio_Pa_Private *mainloop;
   Ecore_Timer                    *timer;
   struct timeval                  tv;

   void                           *userdata;

   pa_time_event_cb_t              callback;
   pa_time_event_destroy_cb_t      destroy_callback;
};

Eina_Bool
_ecore_time_wrapper(void *data)
{
   pa_time_event *event = (pa_time_event *)data;

   event->callback(&event->mainloop->api, event, &event->tv, event->userdata);

   return ECORE_CALLBACK_CANCEL;
}

pa_time_event *
_ecore_pa_time_new(pa_mainloop_api *api, const struct timeval *tv, pa_time_event_cb_t cb, void *userdata)
{
   pa_time_event *event;
   struct _Ecore_Audio_Pa_Private *mloop;
   struct timeval now;
   double interval;

   mloop = api->userdata;

   event = calloc(1, sizeof(pa_time_event));
   event->mainloop = mloop;
   event->userdata = userdata;
   event->callback = cb;
   event->tv = *tv;

   if (gettimeofday(&now, NULL) == -1)
     {
        ERR("Failed to get the current time!");
        free(event);
        return NULL;
     }

   interval = (tv->tv_sec - now.tv_sec) + (tv->tv_usec - now.tv_usec) / 1000;
   event->timer = ecore_timer_add(interval, _ecore_time_wrapper, event);

   return event;
}

void
_ecore_pa_time_restart(pa_time_event *event, const struct timeval *tv)
{
   struct timeval now;
   double interval;

   /* If tv is NULL disable timer */
   if (!tv)
     {
        ecore_timer_del(event->timer);
        event->timer = NULL;
        return;
     }

   event->tv = *tv;

   if (gettimeofday(&now, NULL) == -1)
     {
        ERR("Failed to get the current time!");
        return;
     }

   interval = (tv->tv_sec - now.tv_sec) + (tv->tv_usec - now.tv_usec) / 1000;
   if (event->timer)
     {
        event->timer = ecore_timer_add(interval, _ecore_time_wrapper, event);
     }
   else
     {
        ecore_timer_interval_set(event->timer, interval);
        ecore_timer_reset(event->timer);
     }
}

void
_ecore_pa_time_free(pa_time_event *event)
{
   if (event->timer)
     ecore_timer_del(event->timer);

   event->timer = NULL;

   free(event);
}

void
_ecore_pa_time_set_destroy(pa_time_event *event, pa_time_event_destroy_cb_t cb)
{
   event->destroy_callback = cb;
}

/* Deferred events */
struct pa_defer_event
{
   struct _Ecore_Audio_Pa_Private *mainloop;
   Ecore_Idler                    *idler;

   void                           *userdata;

   pa_defer_event_cb_t             callback;
   pa_defer_event_destroy_cb_t     destroy_callback;
};

Eina_Bool
_ecore_defer_wrapper(void *data)
{
   pa_defer_event *event = (pa_defer_event *)data;

   event->idler = NULL;
   event->callback(&event->mainloop->api, event, event->userdata);

   return ECORE_CALLBACK_CANCEL;
}

pa_defer_event *
_ecore_pa_defer_new(pa_mainloop_api *api, pa_defer_event_cb_t cb, void *userdata)
{
   pa_defer_event *event;
   struct _Ecore_Audio_Pa_Private *mloop;

   mloop = api->userdata;

   event = calloc(1, sizeof(pa_defer_event));
   event->mainloop = mloop;
   event->userdata = userdata;
   event->callback = cb;

   event->idler = ecore_idler_add(_ecore_defer_wrapper, event);

   return event;
}

void
_ecore_pa_defer_enable(pa_defer_event *event, int b)
{
   if (!b && event->idler)
     {
        ecore_idler_del(event->idler);
        event->idler = NULL;
     }
   else if (b && !event->idler)
     {
        event->idler = ecore_idler_add(_ecore_defer_wrapper, event);
     }
}

void
_ecore_pa_defer_free(pa_defer_event *event)
{
   if (event->idler)
     ecore_idler_del(event->idler);

   event->idler = NULL;

   free(event);
}

void
_ecore_pa_defer_set_destroy(pa_defer_event *event, pa_defer_event_destroy_cb_t cb)
{
   event->destroy_callback = cb;
}

static void
_ecore_pa_quit(pa_mainloop_api *api EINA_UNUSED, int retval EINA_UNUSED)
{
   /* FIXME: Need to clean up timers, etc.? */
   WRN("Not quitting mainloop, although PA requested it");
}

/* Function table for PA mainloop integration */
static const pa_mainloop_api functable = {
   .userdata = NULL,

   .io_new = _ecore_pa_io_new,
   .io_enable = _ecore_pa_io_enable,
   .io_free = _ecore_pa_io_free,
   .io_set_destroy = _ecore_pa_io_set_destroy,

   .time_new = _ecore_pa_time_new,
   .time_restart = _ecore_pa_time_restart,
   .time_free = _ecore_pa_time_free,
   .time_set_destroy = _ecore_pa_time_set_destroy,

   .defer_new = _ecore_pa_defer_new,
   .defer_enable = _ecore_pa_defer_enable,
   .defer_free = _ecore_pa_defer_free,
   .defer_set_destroy = _ecore_pa_defer_set_destroy,

   .quit = _ecore_pa_quit,
};

/* *****************************************************
 * Ecore mainloop integration end
 */

static Ecore_Audio_Object *
_pulse_input_new(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   return (Ecore_Audio_Object *)in;
}

static Ecore_Audio_Object *
_pulse_output_new(Ecore_Audio_Object *output)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   struct _Ecore_Audio_Pulse *pulse;

   pulse = calloc(1, sizeof(struct _Ecore_Audio_Pulse));
   if (!pulse)
     {
        ERR("Could not allocate memory for private structure.");
        free(out);
        return NULL;
     }

   out->module_data = pulse;

   return (Ecore_Audio_Object *)out;
}

static void
_pulse_output_del(Ecore_Audio_Object *output)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   free(out->module_data);
}

static void
_pulse_output_volume_set(Ecore_Audio_Object *output, double vol)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   struct _Ecore_Audio_Pa_Private *priv = (struct _Ecore_Audio_Pa_Private *)out->module->priv;
   Eina_List *input;
   Ecore_Audio_Input *in;
   pa_stream *stream;
   uint32_t idx;
   pa_cvolume volume;
   pa_operation *op;

   if (vol < 0)
     vol = 0;

   pa_cvolume_set(&volume, 2, vol * PA_VOLUME_NORM);

   EINA_LIST_FOREACH(out->inputs, input, in)
     {
        stream = in->obj_data;
        idx = pa_stream_get_index(stream);
        op = pa_context_set_sink_input_volume(priv->context, idx, &volume, NULL, NULL);
        pa_operation_unref(op);
     }
}

static void
_pulse_output_write_cb(pa_stream *stream, size_t len, void *data)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)data;

   void *buf;
   size_t bread;

   buf = malloc(len);

   bread = ecore_audio_input_read((Ecore_Audio_Object *)in, buf, len);
   pa_stream_write(stream, buf, bread, free, 0, PA_SEEK_RELATIVE);
   if (bread < len && !in->ended)
     {
       in->ended = EINA_TRUE;
       pa_operation_unref(pa_stream_drain(stream, NULL, NULL));
     }
}

static Eina_Bool
_pulse_output_add_input(Ecore_Audio_Object *output, Ecore_Audio_Object *input)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   Ecore_Audio_Module *outmod = out->module;
   struct _Ecore_Audio_Pa_Private *priv = (struct _Ecore_Audio_Pa_Private *)outmod->priv;
   pa_stream *stream;

   pa_sample_spec ss = {
      .format = PA_SAMPLE_FLOAT32LE,
      .rate = in->samplerate * in->speed,
      .channels = in->channels,
   };

   stream = pa_stream_new(priv->context, in->name, &ss, NULL);
   if (!stream)
     {
        ERR("Could not create stream");
        return EINA_FALSE;
     }

   in->obj_data = stream;

   pa_stream_set_write_callback(stream, _pulse_output_write_cb, in);
   pa_stream_connect_playback(stream, NULL, NULL, PA_STREAM_VARIABLE_RATE, NULL, NULL);

   return EINA_TRUE;
}

static void
_pulse_drain_cb(pa_stream *stream, int success EINA_UNUSED, void *data EINA_UNUSED)
{
   // XXX: Check success?
   pa_stream_disconnect(stream);
   pa_stream_unref(stream);
}

static Eina_Bool
_pulse_output_del_input(Ecore_Audio_Object *output EINA_UNUSED, Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;

   pa_stream *stream = (pa_stream *)in->obj_data;
   in->obj_data = NULL;

   pa_stream_set_write_callback(stream, NULL, NULL);
   pa_operation_unref(pa_stream_drain(stream, _pulse_drain_cb, in));

   return EINA_TRUE;
}

static void
_pulse_output_update_input_format(Ecore_Audio_Object *output EINA_UNUSED, Ecore_Audio_Object *input)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  pa_stream *stream = (pa_stream *)in->obj_data;

  pa_operation_unref(pa_stream_update_sample_rate(stream, in->samplerate * in->speed, NULL, NULL));
}

static int
_pulse_input_read(Ecore_Audio_Object *input EINA_UNUSED, void *data EINA_UNUSED, int len EINA_UNUSED)
{
  // XXX: Implement
   //Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;

   return 0;
}

static void
_ecore_pa_state_cb(pa_context *context, void *data)
{
   struct _Ecore_Audio_Pa_Private *priv = (struct _Ecore_Audio_Pa_Private *)data;
   pa_context_state_t state;

   state = pa_context_get_state(context);

   if (state == PA_CONTEXT_READY)
     {
        DBG("PA context connected.");
     }
   else
     {
        DBG("Connection state %i", state);
     }
   priv->state = state;
}

static struct input_api inops = {
   .input_new = _pulse_input_new,
   .input_read = _pulse_input_read,
};

static struct output_api outops = {
   .output_new = _pulse_output_new,
   .output_del = _pulse_output_del,
   .output_volume_set = _pulse_output_volume_set,
   .output_add_input = _pulse_output_add_input,
   .output_del_input = _pulse_output_del_input,
   .output_update_input_format = _pulse_output_update_input_format,
};

/* externally accessible functions */

/**
 * @brief Initialize the Ecore_Audio PA module
 *
 * @return the initialized module on success, NULL on error
 */
Ecore_Audio_Module *
ecore_audio_pulse_init(void)
{
   struct _Ecore_Audio_Pa_Private *priv;

   pulse_module = calloc(1, sizeof(Ecore_Audio_Module));
   if (!pulse_module)
     {
        ERR("Could not allocate memory for module.");
        return NULL;
     }

   priv = calloc(1, sizeof(struct _Ecore_Audio_Pa_Private));
   if (!priv)
     {
        ERR("Could not allocate memory for private module region.");
        free(pulse_module);
        return NULL;
     }

   priv->api = functable;
   priv->api.userdata = priv;
   /* FIXME: Get name from application */
   priv->context = pa_context_new(&priv->api, "ecore_audio");
   if (!priv->context)
     {
        ERR("Could not create PulseAudio context.");
        free(priv);
        free(pulse_module);
        return NULL;
     }

   pa_context_set_state_callback(priv->context, _ecore_pa_state_cb, priv);
   pa_context_connect(priv->context, NULL, PA_CONTEXT_NOFLAGS, NULL);

   ECORE_MAGIC_SET(pulse_module, ECORE_MAGIC_AUDIO_MODULE);
   pulse_module->type = ECORE_AUDIO_TYPE_PULSE;
   pulse_module->name = "pulse";
   pulse_module->priv = priv;
   pulse_module->inputs = NULL;
   pulse_module->outputs = NULL;
   pulse_module->in_ops = &inops;
   pulse_module->out_ops = &outops;

   return pulse_module;
}

/**
 * @brief Shut down the Ecore_Audio PA module
 */
void
ecore_audio_pulse_shutdown(void)
{
   struct _Ecore_Audio_Pa_Private *priv = (struct _Ecore_Audio_Pa_Private *)pulse_module->priv;

   /* XXX: Make sure all pending events are freed */
   priv->api.userdata = NULL;
   free(priv);
   free(pulse_module);
   pulse_module = NULL;
}

/**
 * @}
 */

#endif /* HAVE_PULSE */
