#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pipewire/pipewire.h>

#include "ecore_audio_private.h"

#define MY_CLASS ECORE_AUDIO_OUT_PIPEWIRE_CLASS

typedef struct _Ecore_Audio_Out_Pipewire_Data
{
  Eo *obj;
  struct pw_loop *loop;
  struct pw_context *context;
  struct pw_core *core;
  struct spa_hook listener;
  Ecore_Fd_Handler *handler;
  Ecore_Job *state_job;
  Ecore_Timer *timeout;
  Eina_List *streams;
  int sync;
  Eina_Bool init : 1;
  Eina_Bool failed : 1;
} Ecore_Audio_Out_Pipewire_Data;

typedef struct _Ecore_Audio_Pipewire_Stream
{
  Ecore_Audio_Out_Pipewire_Data *output;
  Eo *input;
  struct pw_stream *stream;
  struct spa_hook listener;
  unsigned int stride;
  Eina_Bool processing : 1;
  Eina_Bool detached : 1;
  Eina_Bool ended : 1;
} Ecore_Audio_Pipewire_Stream;

typedef struct _Ecore_Audio_Pipewire_Probe
{
  int sync;
  Eina_Bool done : 1;
  Eina_Bool available : 1;
} Ecore_Audio_Pipewire_Probe;

/* Keep the library loaded across ecore_audio_shutdown(): objects and deferred
 * cleanup jobs may still call PipeWire.
 */
Ecore_Audio_Lib_Pipewire *ecore_audio_pipewire_lib = NULL;

static Eina_Bool _probe_available = EINA_FALSE;

Eina_Bool
ecore_audio_pipewire_lib_load(void)
{
   if (ecore_audio_pipewire_lib)
     {
        if (!ecore_audio_pipewire_lib->mod) return EINA_FALSE;
        return EINA_TRUE;
     }

   ecore_audio_pipewire_lib = calloc(1, sizeof(Ecore_Audio_Lib_Pipewire));
   if (!ecore_audio_pipewire_lib) return EINA_FALSE;
# define LOAD(m, x)                                               \
   if (!ecore_audio_pipewire_lib->m) {                             \
      if ((ecore_audio_pipewire_lib->m = eina_module_new(x))) {    \
         if (!eina_module_load(ecore_audio_pipewire_lib->m)) {     \
            eina_module_free(ecore_audio_pipewire_lib->m);         \
            ecore_audio_pipewire_lib->m = NULL;                    \
         }                                                        \
      }                                                           \
   }
# if defined(__APPLE__) && defined(__MACH__)
   LOAD(mod, "libpipewire-0.3.0.dylib");
# endif
   LOAD(mod, "libpipewire-0.3.so.0");
   if (!ecore_audio_pipewire_lib->mod)
     {
        WRN("Cannot find libpipewire at runtime!");
        return EINA_FALSE;
     }

# define SYM(x) \
   if (!(ecore_audio_pipewire_lib->x = eina_module_symbol_get(ecore_audio_pipewire_lib->mod, #x))) { \
      ERR("Cannot find symbol '%s' in '%s'", #x, eina_module_file_get(ecore_audio_pipewire_lib->mod)); \
      goto err; \
   }
   SYM(pw_init);
   SYM(pw_deinit);
   SYM(pw_loop_new);
   SYM(pw_loop_destroy);
   SYM(pw_loop_get_fd);
   SYM(pw_loop_enter);
   SYM(pw_loop_leave);
   SYM(pw_loop_iterate);
   SYM(pw_context_new);
   SYM(pw_context_destroy);
   SYM(pw_context_connect);
   SYM(pw_core_disconnect);
   SYM(pw_core_add_listener);
   SYM(pw_core_sync);
   SYM(pw_properties_new);
   SYM(pw_stream_new);
   SYM(pw_stream_destroy);
   SYM(pw_stream_add_listener);
   SYM(pw_stream_connect);
   SYM(pw_stream_update_params);
   SYM(pw_stream_dequeue_buffer);
   SYM(pw_stream_queue_buffer);
   SYM(pw_stream_set_active);
   SYM(pw_stream_flush);
# undef SYM

   /* SPA helpers may live in the separate library shipped by PipeWire. */
# define SPA_SYM(x) \
   if (!(ecore_audio_pipewire_lib->x = eina_module_symbol_get(ecore_audio_pipewire_lib->mod, #x))) { \
      LOAD(spa_mod, ECORE_AUDIO_SPA_LIBRARY); \
      if (!ecore_audio_pipewire_lib->spa_mod) { \
         WRN("Cannot find '%s' at runtime!", ECORE_AUDIO_SPA_LIBRARY); \
         goto err; \
      } \
      if (!(ecore_audio_pipewire_lib->x = eina_module_symbol_get(ecore_audio_pipewire_lib->spa_mod, #x))) { \
         ERR("Cannot find symbol '%s' in '%s'", #x, eina_module_file_get(ecore_audio_pipewire_lib->spa_mod)); \
         goto err; \
      } \
   }
   SPA_SYM(spa_hook_remove);
   SPA_SYM(spa_format_audio_raw_build);
# undef SPA_SYM
# undef LOAD
   return EINA_TRUE;
err:
   if (ecore_audio_pipewire_lib->spa_mod)
     eina_module_free(ecore_audio_pipewire_lib->spa_mod);
   eina_module_free(ecore_audio_pipewire_lib->mod);
   memset(ecore_audio_pipewire_lib, 0, sizeof(*ecore_audio_pipewire_lib));
   return EINA_FALSE;
}

void
ecore_audio_pipewire_lib_unload(void)
{
   if (ecore_audio_pipewire_lib)
     {
        if (ecore_audio_pipewire_lib->spa_mod)
          eina_module_free(ecore_audio_pipewire_lib->spa_mod);
        if (ecore_audio_pipewire_lib->mod)
          eina_module_free(ecore_audio_pipewire_lib->mod);
        free(ecore_audio_pipewire_lib);
        ecore_audio_pipewire_lib = NULL;
     }
}

static void
_probe_done(void *data, uint32_t id, int seq)
{
   Ecore_Audio_Pipewire_Probe *probe = data;

   if ((id != PW_ID_CORE) || (seq != probe->sync) || probe->done) return;
   probe->available = EINA_TRUE;
   probe->done = EINA_TRUE;
}

static void
_probe_error(void *data, uint32_t id EINA_UNUSED, int seq EINA_UNUSED,
             int res EINA_UNUSED, const char *message EINA_UNUSED)
{
   Ecore_Audio_Pipewire_Probe *probe = data;

   probe->available = EINA_FALSE;
   probe->done = EINA_TRUE;
}

static const struct pw_core_events _probe_events = {
   PW_VERSION_CORE_EVENTS,
   .done = _probe_done,
   .error = _probe_error,
};

Eina_Bool
_ecore_audio_out_pipewire_probe(void)
{
   struct pw_loop *loop;
   struct pw_context *context;
   struct pw_core *core;
   struct spa_hook listener;
   Ecore_Audio_Pipewire_Probe probe = { 0 };
   double deadline, remaining;
   int ret;

   if (_probe_available) return EINA_TRUE;
   if (!EPW_LOAD()) return EINA_FALSE;
   EPW_CALL(pw_init)(NULL, NULL);
   loop = EPW_CALL(pw_loop_new)(NULL);
   if (!loop) goto end;
   EPW_CALL(pw_loop_enter)(loop);
   context = EPW_CALL(pw_context_new)(loop, NULL, 0);
   if (!context) goto destroy_loop;
   core = EPW_CALL(pw_context_connect)(context, NULL, 0);
   if (!core) goto destroy_context;
   EPW_CALL(pw_core_add_listener)(core, &listener, &_probe_events, &probe);
   probe.sync = EPW_CALL(pw_core_sync)(core, PW_ID_CORE, 0);
   if (probe.sync < 0) goto disconnect;
   deadline = ecore_time_get() + 5.0;
   while (!probe.done)
     {
        remaining = deadline - ecore_time_get();
        if (remaining <= 0.0) break;
        ret = EPW_CALL(pw_loop_iterate)(loop, (int)(remaining * 1000));
        if ((ret < 0) && (ret != -EINTR)) break;
     }
   _probe_available = probe.available;
disconnect:
   EPW_CALL(spa_hook_remove)(&listener);
   EPW_CALL(pw_core_disconnect)(core);
destroy_context:
   EPW_CALL(pw_context_destroy)(context);
destroy_loop:
   EPW_CALL(pw_loop_leave)(loop);
   EPW_CALL(pw_loop_destroy)(loop);
end:
   EPW_CALL(pw_deinit)();
   return _probe_available;
}

static void
_state_job(void *data)
{
   Ecore_Audio_Out_Pipewire_Data *pd = data;

   pd->state_job = NULL;
   if (pd->timeout) ecore_timer_del(pd->timeout);
   pd->timeout = NULL;
   efl_event_callback_call(pd->obj,
                          pd->failed ? ECORE_AUDIO_OUT_PIPEWIRE_EVENT_CONTEXT_FAIL :
                          ECORE_AUDIO_OUT_PIPEWIRE_EVENT_CONTEXT_READY, NULL);
}

static void
_fail(Ecore_Audio_Out_Pipewire_Data *pd)
{
   if (pd->failed) return;
   pd->failed = EINA_TRUE;
   if (!pd->state_job) pd->state_job = ecore_job_add(_state_job, pd);
}

static Eina_Bool
_timeout(void *data)
{
   Ecore_Audio_Out_Pipewire_Data *pd = data;

   pd->timeout = NULL;
   _fail(pd);
   return ECORE_CALLBACK_CANCEL;
}

static void
_core_done(void *data, uint32_t id, int seq)
{
   Ecore_Audio_Out_Pipewire_Data *pd = data;

   if ((id != PW_ID_CORE) || (seq != pd->sync) || pd->failed) return;
   if (!pd->state_job) pd->state_job = ecore_job_add(_state_job, pd);
}

static void
_core_error(void *data, uint32_t id EINA_UNUSED, int seq EINA_UNUSED,
            int res, const char *message)
{
   Ecore_Audio_Out_Pipewire_Data *pd = data;

   WRN("PipeWire error %d: %s", res, message);
   _fail(pd);
}

static const struct pw_core_events _core_events =
{
   PW_VERSION_CORE_EVENTS,
   .done = _core_done,
   .error = _core_error,
};

static Eina_Bool
_loop_cb(void *data, Ecore_Fd_Handler *handler EINA_UNUSED)
{
   Ecore_Audio_Out_Pipewire_Data *pd = data;
   Eo *obj = efl_ref(pd->obj);

   EPW_CALL(pw_loop_enter)(pd->loop);
   if (EPW_CALL(pw_loop_iterate)(pd->loop, 0) < 0) _fail(pd);
   EPW_CALL(pw_loop_leave)(pd->loop);
   efl_unref(obj);
   return ECORE_CALLBACK_RENEW;
}

static void
_stream_free_job_cb(void *data)
{ // we delayed the real freee until later to avoid re-entrant usage
   Ecore_Audio_Pipewire_Stream *stream = data;
   EPW_CALL(spa_hook_remove)(&stream->listener);
   EPW_CALL(pw_stream_destroy)(stream->stream);
   free(stream);
}

static void
_stream_free(Ecore_Audio_Pipewire_Stream *stream)
{ // delay to void using what has been freed
   ecore_job_add(_stream_free_job_cb, stream);
}

static void
_process(void *data)
{
   Ecore_Audio_Pipewire_Stream *stream = data;
   struct pw_buffer *buffer;
   struct spa_data *plane;
   Eo *input;
   unsigned int frames, size, i;
   ssize_t count;
   double volume;
   float *samples;

   buffer = EPW_CALL(pw_stream_dequeue_buffer)(stream->stream);
   if (!buffer) return;
   if (!buffer->buffer->n_datas) goto queue;
   plane = &buffer->buffer->datas[0];
   if (!plane->data || !plane->chunk) goto queue;
   frames = plane->maxsize / stream->stride;
   if (buffer->requested && (buffer->requested < frames))
     frames = buffer->requested;
   /* VIO read callbacks accept an int length. */
   if (frames > INT_MAX / stream->stride) frames = INT_MAX / stream->stride;
   size = frames * stream->stride;
   plane->chunk->offset = 0;
   plane->chunk->stride = stream->stride;
   plane->chunk->size = size;
   buffer->size = frames;
   if (!size) goto queue;

   volume = ecore_audio_obj_volume_get(stream->output->obj);
   stream->processing = EINA_TRUE;
   input = efl_ref(stream->input);
   count = stream->ended ? 0 : ecore_audio_obj_in_read(input, plane->data, size);
   efl_unref(input);
   stream->processing = EINA_FALSE;
   /* An input callback may detach itself, including during its destructor. */
   if (stream->detached)
     {
        _stream_free(stream);
        return;
     }
   if (count < 0)
     {
        _fail(stream->output);
        count = 0;
     }
   if ((size_t)count > size) count = size;
   count -= count % stream->stride;
   memset(((unsigned char *)plane->data) + count, 0, size - count);
   samples = plane->data;
   for (i = 0; i < (unsigned int)count / sizeof(float); i++)
     samples[i] *= volume;
   if (!count && !stream->ended)
     {
        stream->ended = EINA_TRUE;
        EPW_CALL(pw_stream_queue_buffer)(stream->stream, buffer);
        EPW_CALL(pw_stream_flush)(stream->stream, true);
        return;
     }
queue:
   EPW_CALL(pw_stream_queue_buffer)(stream->stream, buffer);
}

static void
_drained(void *data)
{
   Ecore_Audio_Pipewire_Stream *stream = data;

   if (stream->ended) EPW_CALL(pw_stream_set_active)(stream->stream, false);
}

static void
_stream_state(void *data, enum pw_stream_state old EINA_UNUSED,
              enum pw_stream_state state, const char *error)
{
   Ecore_Audio_Pipewire_Stream *stream = data;

   if (state == PW_STREAM_STATE_ERROR)
     {
        WRN("PipeWire stream failed: %s", error);
        _fail(stream->output);
     }
}

static const struct pw_stream_events _stream_events =
{
   PW_VERSION_STREAM_EVENTS,
   .state_changed = _stream_state,
   .process = _process,
   .drained = _drained,
};

static const struct spa_pod *
_format(Eo *input, struct spa_pod_builder *builder)
{
   struct spa_audio_info_raw info = { 0 };
   int channels = ecore_audio_obj_in_channels_get(input);
   double rate = ecore_audio_obj_in_samplerate_get(input) *
                 ecore_audio_obj_in_speed_get(input);

   if ((channels < 1) || ((unsigned int)channels > SPA_AUDIO_MAX_CHANNELS) ||
       !isfinite(rate) || (rate < 1) || (rate > UINT32_MAX)) return NULL;
   info.format = SPA_AUDIO_FORMAT_F32;
   info.rate = rate;
   info.channels = channels;
   if (channels == 1) info.position[0] = SPA_AUDIO_CHANNEL_MONO;
   else if (channels == 2)
     {
        info.position[0] = SPA_AUDIO_CHANNEL_FL;
        info.position[1] = SPA_AUDIO_CHANNEL_FR;
     }
   else info.flags = SPA_AUDIO_FLAG_UNPOSITIONED;
   return EPW_CALL(spa_format_audio_raw_build)(builder, SPA_PARAM_EnumFormat, &info);
}

static void
_samplerate_changed(void *data, const Efl_Event *event)
{
   Ecore_Audio_Pipewire_Stream *stream = data;
   uint8_t buffer[1024];
   struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
   const struct spa_pod *param = _format(event->object, &builder);

   if (!param || (EPW_CALL(pw_stream_update_params)(stream->stream, &param, 1) < 0))
     _fail(stream->output);
}

EOLIAN static Eina_Bool
_ecore_audio_out_pipewire_ecore_audio_out_input_attach(Eo *eo_obj, Ecore_Audio_Out_Pipewire_Data *pd, Eo *input)
{
   Ecore_Audio_Pipewire_Stream *stream;
   struct pw_properties *props;
   uint8_t buffer[1024];
   struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
   const struct spa_pod *param;
   enum pw_stream_flags flags = PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS;
   const char *name;

   if (!efl_isa(input, ECORE_AUDIO_IN_CLASS) || pd->failed) return EINA_FALSE;
   if (ecore_audio_obj_in_output_get(input) == eo_obj) return EINA_TRUE;
   param = _format(input, &builder);
   if (!param) return EINA_FALSE;
   stream = calloc(1, sizeof(*stream));
   if (!stream) return EINA_FALSE;
   stream->output = pd;
   stream->input = input;
   stream->stride = ecore_audio_obj_in_channels_get(input) * sizeof(float);
   name = efl_name_get(input);
   props = EPW_CALL(pw_properties_new)(PW_KEY_MEDIA_TYPE, "Audio",
                             PW_KEY_MEDIA_CATEGORY, "Playback",
                             PW_KEY_MEDIA_ROLE, "Music", NULL);
   if (!props) goto fail;
   stream->stream = EPW_CALL(pw_stream_new)(pd->core, name ? name : "ecore_audio", props);
   if (!stream->stream) goto fail;
   EPW_CALL(pw_stream_add_listener)(stream->stream, &stream->listener, &_stream_events, stream);
   if (ecore_audio_obj_paused_get(eo_obj)) flags |= PW_STREAM_FLAG_INACTIVE;
   if (EPW_CALL(pw_stream_connect)(stream->stream, PW_DIRECTION_OUTPUT, PW_ID_ANY,
                         flags, &param, 1) < 0) goto fail_stream;
   if (!ecore_audio_obj_out_input_attach(efl_super(eo_obj, MY_CLASS), input))
     goto fail_stream;
   pd->streams = eina_list_append(pd->streams, stream);
   efl_event_callback_add(input, ECORE_AUDIO_IN_EVENT_IN_SAMPLERATE_CHANGED,
                          _samplerate_changed, stream);
   return EINA_TRUE;

fail_stream:
   _stream_free(stream);
   return EINA_FALSE;
fail:
   free(stream);
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_ecore_audio_out_pipewire_ecore_audio_out_input_detach(Eo *eo_obj, Ecore_Audio_Out_Pipewire_Data *pd, Eo *input)
{
   Eina_List *l;
   Ecore_Audio_Pipewire_Stream *stream;

   EINA_LIST_FOREACH(pd->streams, l, stream)
     if (stream->input == input)
       {
          efl_event_callback_del(input, ECORE_AUDIO_IN_EVENT_IN_SAMPLERATE_CHANGED,
                                 _samplerate_changed, stream);
          pd->streams = eina_list_remove_list(pd->streams, l);
          stream->detached = EINA_TRUE;
          if (!stream->processing) _stream_free(stream);
          return ecore_audio_obj_out_input_detach(efl_super(eo_obj, MY_CLASS), input);
       }
   return EINA_FALSE;
}

EOLIAN static void
_ecore_audio_out_pipewire_ecore_audio_volume_set(Eo *eo_obj, Ecore_Audio_Out_Pipewire_Data *pd EINA_UNUSED, double volume)
{
   if (!isfinite(volume) || (volume < 0)) volume = 0;
   ecore_audio_obj_volume_set(efl_super(eo_obj, MY_CLASS), volume);
}

EOLIAN static void
_ecore_audio_out_pipewire_ecore_audio_paused_set(Eo *eo_obj, Ecore_Audio_Out_Pipewire_Data *pd, Eina_Bool paused)
{
   Eina_List *l;
   Ecore_Audio_Pipewire_Stream *stream;

   ecore_audio_obj_paused_set(efl_super(eo_obj, MY_CLASS), paused);
   EINA_LIST_FOREACH(pd->streams, l, stream)
     {
        if (!paused) stream->ended = EINA_FALSE;
        EPW_CALL(pw_stream_set_active)(stream->stream, !paused);
     }
}

EOLIAN static Eo *
_ecore_audio_out_pipewire_efl_object_constructor(Eo *eo_obj, Ecore_Audio_Out_Pipewire_Data *pd)
{
   Ecore_Audio_Output *output;

   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   if (!eo_obj) return NULL;
   pd->obj = eo_obj;
   output = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);
   output->need_writer = EINA_FALSE;
   if (!EPW_LOAD()) return NULL;
   EPW_CALL(pw_init)(NULL, NULL);
   pd->init = EINA_TRUE;
   pd->loop = EPW_CALL(pw_loop_new)(NULL);
   if (!pd->loop) return NULL;
   pd->context = EPW_CALL(pw_context_new)(pd->loop, NULL, 0);
   if (!pd->context) return NULL;
   pd->core = EPW_CALL(pw_context_connect)(pd->context, NULL, 0);
   if (!pd->core) return NULL;
   EPW_CALL(pw_core_add_listener)(pd->core, &pd->listener, &_core_events, pd);
   pd->sync = EPW_CALL(pw_core_sync)(pd->core, PW_ID_CORE, 0);
   if (pd->sync < 0) return NULL;
   pd->handler = ecore_main_fd_handler_add(EPW_CALL(pw_loop_get_fd)(pd->loop),
                                           ECORE_FD_READ, _loop_cb, pd,
                                           NULL, NULL);
   if (!pd->handler) return NULL;
   pd->timeout = ecore_timer_add(0.5, _timeout, pd);
   if (!pd->timeout) return NULL;
   return eo_obj;
}

typedef struct
{
   struct pw_loop *loop;
   struct pw_context *context;
   struct pw_core *core;
  Eina_Bool init : 1;
} Ecore_Audio_Pipewire_Delayed_Destroy_Data;

static void
_ecore_audio_out_pipewire_destroy_job_cb(void *data)
{
   Ecore_Audio_Pipewire_Delayed_Destroy_Data *dat = data;

   if (dat->core) EPW_CALL(pw_core_disconnect)(dat->core);
   if (dat->context) EPW_CALL(pw_context_destroy)(dat->context);
   if (dat->loop) EPW_CALL(pw_loop_destroy)(dat->loop);
   if (dat->init) EPW_CALL(pw_deinit)();
   free(dat);
}

EOLIAN static void
_ecore_audio_out_pipewire_efl_object_destructor(Eo *eo_obj, Ecore_Audio_Out_Pipewire_Data *pd)
{
   Ecore_Audio_Pipewire_Stream *stream;
   Ecore_Audio_Pipewire_Delayed_Destroy_Data *dat;

   if (pd->handler) ecore_main_fd_handler_del(pd->handler);
   if (pd->state_job) ecore_job_del(pd->state_job);
   if (pd->timeout) ecore_timer_del(pd->timeout);
   while (pd->streams)
     {
        stream = eina_list_data_get(pd->streams);
        ecore_audio_obj_out_input_detach(eo_obj, stream->input);
     }
   // delay destruction of all the core pw stuff until queued stuff is done
   dat = calloc(1, sizeof(Ecore_Audio_Pipewire_Delayed_Destroy_Data));
   if (pd->core)
     {
        EPW_CALL(spa_hook_remove)(&pd->listener);
        if (dat) dat->core = pd->core;
     }
   if (dat) dat->context = pd->context;
   if (dat) dat->loop = pd->loop;
   if (dat) dat->init = pd->init;
   if (dat) ecore_job_add(_ecore_audio_out_pipewire_destroy_job_cb, dat);
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "ecore_audio_out_pipewire.eo.c"
