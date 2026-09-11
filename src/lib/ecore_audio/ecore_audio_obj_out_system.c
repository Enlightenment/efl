#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "ecore_audio_private.h"

#define MY_CLASS ECORE_AUDIO_OUT_SYSTEM_CLASS

typedef enum _Ecore_Audio_System_Request_Type
{
  REQUEST_VOLUME,
  REQUEST_PAUSED,
  REQUEST_ATTACH,
  REQUEST_DETACH,
  REQUEST_VIO
} Ecore_Audio_System_Request_Type;

typedef struct _Ecore_Audio_System_Request
{
  Ecore_Audio_System_Request_Type type;
  double volume;
  Eo *input;
  Ecore_Audio_Vio *vio;
  void *data;
  efl_key_data_free_func free_func;
  Eina_Bool paused : 1;
} Ecore_Audio_System_Request;

typedef struct _Ecore_Audio_Out_System_Data
{
  Eo *backend;
  Ecore_Job *ready_job;
  Eina_List *requests;
  Eina_List *pending;
  Eina_Bool waiting : 1;
  Eina_Bool failed : 1;
  Eina_Bool replaying : 1;
} Ecore_Audio_Out_System_Data;

/* Cache the getter, rather than an Eo class pointer which can become stale
 * across ecore_audio_shutdown()/ecore_audio_init(). All access is on the
 * Ecore main loop, just like the backend objects themselves.
 */
#ifdef _WIN32
static const Efl_Class *(*_backend_class_get)(void) = ecore_audio_out_wasapi_class_get;
#else
static const Efl_Class *(*_backend_class_get)(void);
static Eina_Bool _backend_checked = EINA_FALSE;
static Eina_Bool _backend_env_checked = EINA_FALSE;
static Eina_List *_waiting = NULL;
static Ecore_Job *_probe_job = NULL;
static Ecore_Timer *_probe_timer = NULL;
#endif

static void
_request_free(Ecore_Audio_System_Request *request)
{
   if (request->input) efl_unref(request->input);
   if (request->free_func) request->free_func(request->data);
   free(request);
}

static Ecore_Audio_System_Request *
_request_add(Ecore_Audio_Out_System_Data *pd, Ecore_Audio_System_Request_Type type)
{
   Ecore_Audio_System_Request *request = calloc(1, sizeof(*request));
   Eina_List *requests;

   if (!request) return NULL;
   requests = eina_list_append(pd->requests, request);
   if (eina_list_last_data_get(requests) != request)
     {
        free(request);
        return NULL;
     }
   pd->requests = requests;
   request->type = type;
   return request;
}

static void
_requests_clear(Ecore_Audio_Out_System_Data *pd)
{
   Ecore_Audio_System_Request *request;
   Eina_List *requests = pd->requests;

   pd->requests = NULL;
   pd->pending = eina_list_free(pd->pending);
   EINA_LIST_FREE(requests, request) _request_free(request);
}

static void
_context_fail_cb(void *data, const Efl_Event *event)
{
   Ecore_Audio_Out_System_Data *pd = efl_data_scope_get(data, MY_CLASS);

   efl_ref(data);
   pd->waiting = EINA_FALSE;
   pd->failed = EINA_TRUE;
   _requests_clear(pd);
   efl_event_callback_call(data,
                           ECORE_AUDIO_OUT_SYSTEM_EVENT_CONTEXT_FAIL,
                           event ? event->info : NULL);
   efl_unref(data);
}

static void
_context_ready_cb(void *data, const Efl_Event *event)
{
   Ecore_Audio_Out_System_Data *pd = efl_data_scope_get(data, MY_CLASS);
   Ecore_Audio_System_Request *request;
   Eina_Bool success = EINA_TRUE;

   if (pd->failed || pd->replaying) return;
   efl_ref(data);
   pd->replaying = EINA_TRUE;
   /* Keep waiting set during replay so reentrant requests also join the FIFO. */
   while (pd->requests)
     {
        request = eina_list_data_get(pd->requests);
        pd->requests = eina_list_remove_list(pd->requests, pd->requests);
        switch (request->type)
          {
           case REQUEST_VOLUME:
             ecore_audio_obj_volume_set(pd->backend, request->volume);
             break;
           case REQUEST_PAUSED:
             ecore_audio_obj_paused_set(pd->backend, request->paused);
             break;
           case REQUEST_ATTACH:
             if (!ecore_audio_obj_out_input_attach(pd->backend, request->input))
               success = EINA_FALSE;
             break;
           case REQUEST_DETACH:
             if (!ecore_audio_obj_out_input_detach(pd->backend, request->input))
               success = EINA_FALSE;
             break;
           case REQUEST_VIO:
             ecore_audio_obj_vio_set(pd->backend, request->vio, request->data,
                                     request->free_func);
             request->free_func = NULL;
             break;
          }
        _request_free(request);
        if (pd->failed)
          {
             pd->replaying = EINA_FALSE;
             efl_unref(data);
             return;
          }
     }
   pd->pending = eina_list_free(pd->pending);
   pd->waiting = EINA_FALSE;
   pd->replaying = EINA_FALSE;
   efl_event_callback_call(data,
                           success ?
                           ECORE_AUDIO_OUT_SYSTEM_EVENT_CONTEXT_READY :
                           ECORE_AUDIO_OUT_SYSTEM_EVENT_CONTEXT_FAIL,
                           event ? event->info : NULL);
   efl_unref(data);
}

static void
_backend_callbacks(Eo *obj, Ecore_Audio_Out_System_Data *pd, Eina_Bool add)
{
   const Efl_Event_Description *ready = NULL, *fail = NULL;

#ifdef _WIN32
   ready = ECORE_AUDIO_OUT_WASAPI_EVENT_CONTEXT_READY;
   fail = ECORE_AUDIO_OUT_WASAPI_EVENT_CONTEXT_FAIL;
#else
#ifdef HAVE_PIPEWIRE
   if (_backend_class_get == ecore_audio_out_pipewire_class_get)
     {
        ready = ECORE_AUDIO_OUT_PIPEWIRE_EVENT_CONTEXT_READY;
        fail = ECORE_AUDIO_OUT_PIPEWIRE_EVENT_CONTEXT_FAIL;
     }
#endif
#ifdef HAVE_PULSE
   if (_backend_class_get == ecore_audio_out_pulse_class_get)
     {
        ready = ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_READY;
        fail = ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_FAIL;
     }
#endif
#endif
   if (!ready) return;
   if (add)
     {
        efl_event_callback_add(pd->backend, ready, _context_ready_cb, obj);
        efl_event_callback_add(pd->backend, fail, _context_fail_cb, obj);
     }
   else
     {
        efl_event_callback_del(pd->backend, ready, _context_ready_cb, obj);
        efl_event_callback_del(pd->backend, fail, _context_fail_cb, obj);
     }
}

#ifdef _WIN32
static void
_stop_cb(void *data, const Efl_Event *event)
{
   efl_event_callback_call(data, ECORE_AUDIO_OUT_SYSTEM_EVENT_STOP, event->info);
}

static void
_ready_job(void *data)
{
   Ecore_Audio_Out_System_Data *pd = efl_data_scope_get(data, MY_CLASS);

   pd->ready_job = NULL;
   _context_ready_cb(data, NULL);
}
#endif

static Eina_Bool
_backend_new(Eo *obj, Ecore_Audio_Out_System_Data *pd)
{
   if (!_backend_class_get) return EINA_FALSE;
   pd->backend = efl_add_ref(_backend_class_get(), NULL);
   if (!pd->backend) return EINA_FALSE;
   _backend_callbacks(obj, pd, EINA_TRUE);
#ifdef _WIN32
   efl_event_callback_add(pd->backend,
                          ECORE_AUDIO_OUT_WASAPI_EVENT_STOP,
                          _stop_cb, obj);
   /* WASAPI initializes synchronously and does not emit context,ready. */
   pd->ready_job = ecore_job_add(_ready_job, obj);
   if (!pd->ready_job) return EINA_FALSE;
#else
   pd->waiting = EINA_TRUE;
#endif
   return EINA_TRUE;
}

#ifndef _WIN32
#if defined(HAVE_PULSE) || defined(HAVE_PIPEWIRE)
static void _probe_finish(void *data);

static Eina_Bool
_probe_retry(void *data EINA_UNUSED)
{
   _probe_timer = NULL;
   _probe_finish(NULL);
   return ECORE_CALLBACK_CANCEL;
}
#endif

static void
_probe_finish(void *data EINA_UNUSED)
{
   Eo *obj;
   Ecore_Audio_Out_System_Data *pd;

   _probe_job = NULL;
#ifdef HAVE_PULSE
   if (_ecore_audio_out_pulse_probe())
     _backend_class_get = ecore_audio_out_pulse_class_get;
#endif
#ifdef HAVE_PIPEWIRE
   if (!_backend_class_get && _ecore_audio_out_pipewire_probe())
     _backend_class_get = ecore_audio_out_pipewire_class_get;
#endif
   /* A server may still be starting. Keep the outputs and their queued
    * requests waiting, and cache only a successful automatic selection.
    */
   _backend_checked = !!_backend_class_get;
#if defined(HAVE_PULSE) || defined(HAVE_PIPEWIRE)
   if ((!_backend_checked) && (_waiting))
     {
        _probe_timer = ecore_timer_add(1.0, _probe_retry, NULL);
        if (_probe_timer) return;
     }
#endif

   /* Callbacks may destroy other waiters, so remove each one before calling. */
   while (_waiting)
     {
        obj = eina_list_data_get(_waiting);
        _waiting = eina_list_remove_list(_waiting, _waiting);
        efl_ref(obj);
        pd = efl_data_scope_get(obj, MY_CLASS);
        if (!_backend_new(obj, pd)) _context_fail_cb(obj, NULL);
        efl_unref(obj);
     }
}

static void
_backend_check(void)
{
   const char *backend;

   if (_backend_checked || _probe_job || _probe_timer) return;
   backend = _backend_env_checked ? NULL : getenv("ECORE_AUDIO_BACKEND");
   _backend_env_checked = EINA_TRUE;
   if ((backend) && (backend[0]) && (!!strcmp(backend, "auto")))
     {
        _backend_checked = EINA_TRUE;
#ifdef HAVE_PIPEWIRE
        if (!strcmp(backend, "pipewire"))
          _backend_class_get = ecore_audio_out_pipewire_class_get;
#endif
#ifdef HAVE_PULSE
        if (!strcmp(backend, "pulseaudio") || !strcmp(backend, "pulse"))
          _backend_class_get = ecore_audio_out_pulse_class_get;
#endif
        if (!_backend_class_get)
          ERR("Unknown or unavailable ECORE_AUDIO_BACKEND: %s", backend);
        return;
     }
   _probe_job = ecore_job_add(_probe_finish, NULL);
   if (!_probe_job) _backend_checked = EINA_TRUE;
}
#endif

EOLIAN static Eo *
_ecore_audio_out_system_efl_object_constructor(Eo *eo_obj, Ecore_Audio_Out_System_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   if (!eo_obj) return NULL;

#ifndef _WIN32
   _backend_check();
   if (!_backend_checked)
     {
        _waiting = eina_list_append(_waiting, eo_obj);
        if (eina_list_last_data_get(_waiting) != eo_obj) return NULL;
        pd->waiting = EINA_TRUE;
        return eo_obj;
     }
#endif
   if (!_backend_new(eo_obj, pd))
     {
        ERR("No system audio output backend is available");
        return NULL;
     }
   return eo_obj;
}

EOLIAN static void
_ecore_audio_out_system_efl_object_destructor(Eo *eo_obj, Ecore_Audio_Out_System_Data *pd)
{
#ifndef _WIN32
   _waiting = eina_list_remove(_waiting, eo_obj);
   if ((!_waiting) && (_probe_job))
     {
        ecore_job_del(_probe_job);
        _probe_job = NULL;
     }
   if ((!_waiting) && (_probe_timer))
     {
        ecore_timer_del(_probe_timer);
        _probe_timer = NULL;
     }
#endif
   if (pd->ready_job) ecore_job_del(pd->ready_job);
   pd->failed = EINA_TRUE;
   pd->waiting = EINA_FALSE;
   _requests_clear(pd);
   if (pd->backend)
     {
        _backend_callbacks(eo_obj, pd, EINA_FALSE);
#ifdef _WIN32
        efl_event_callback_del(pd->backend,
                               ECORE_AUDIO_OUT_WASAPI_EVENT_STOP,
                               _stop_cb, eo_obj);
#endif
        efl_unref(pd->backend);
        pd->backend = NULL;
     }
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

EOLIAN static void
_ecore_audio_out_system_ecore_audio_volume_set(Eo *eo_obj, Ecore_Audio_Out_System_Data *pd, double volume)
{
   Ecore_Audio_System_Request *request;

   if (pd->waiting)
     {
        request = _request_add(pd, REQUEST_VOLUME);
        if (!request) return;
        request->volume = volume;
        ecore_audio_obj_volume_set(efl_super(eo_obj, MY_CLASS), volume);
     }
   else if (pd->backend)
     ecore_audio_obj_volume_set(pd->backend, volume);
}

EOLIAN static double
_ecore_audio_out_system_ecore_audio_volume_get(const Eo *eo_obj, Ecore_Audio_Out_System_Data *pd)
{
   if ((pd->waiting) || (!pd->backend))
     return ecore_audio_obj_volume_get(efl_super(eo_obj, MY_CLASS));
   return ecore_audio_obj_volume_get(pd->backend);
}

EOLIAN static void
_ecore_audio_out_system_ecore_audio_paused_set(Eo *eo_obj, Ecore_Audio_Out_System_Data *pd, Eina_Bool paused)
{
   Ecore_Audio_System_Request *request;

   if (pd->waiting)
     {
        request = _request_add(pd, REQUEST_PAUSED);
        if (!request) return;
        request->paused = paused;
        ecore_audio_obj_paused_set(efl_super(eo_obj, MY_CLASS), paused);
     }
   else if (pd->backend)
     ecore_audio_obj_paused_set(pd->backend, paused);
}

EOLIAN static Eina_Bool
_ecore_audio_out_system_ecore_audio_paused_get(const Eo *eo_obj, Ecore_Audio_Out_System_Data *pd)
{
   if ((pd->waiting) || (!pd->backend))
     return ecore_audio_obj_paused_get(efl_super(eo_obj, MY_CLASS));
   return ecore_audio_obj_paused_get(pd->backend);
}

EOLIAN static void
_ecore_audio_out_system_ecore_audio_vio_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd, Ecore_Audio_Vio *vio, void *data, efl_key_data_free_func free_func)
{
   Ecore_Audio_System_Request *request;

   if (pd->waiting)
     {
        request = _request_add(pd, REQUEST_VIO);
        if (request)
          {
             request->vio = vio;
             request->data = data;
             request->free_func = free_func;
             return;
          }
     }
   else if (pd->backend && !pd->failed)
     {
        ecore_audio_obj_vio_set(pd->backend, vio, data, free_func);
        return;
     }
   if (free_func) free_func(data);
}

EOLIAN static Eina_Bool
_ecore_audio_out_system_ecore_audio_out_input_attach(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd, Eo *input)
{
   Ecore_Audio_System_Request *request;

   if (!efl_isa(input, ECORE_AUDIO_IN_CLASS) || pd->failed) return EINA_FALSE;
   if (pd->waiting)
     {
        if (eina_list_data_find(pd->pending, input)) return EINA_FALSE;
        request = _request_add(pd, REQUEST_ATTACH);
        if (!request) return EINA_FALSE;
        pd->pending = eina_list_append(pd->pending, input);
        if (eina_list_last_data_get(pd->pending) != input)
          {
             pd->requests = eina_list_remove(pd->requests, request);
             _request_free(request);
             return EINA_FALSE;
          }
        request->input = efl_ref(input);
        return EINA_TRUE;
     }
   return ecore_audio_obj_out_input_attach(pd->backend, input);
}

EOLIAN static Eina_Bool
_ecore_audio_out_system_ecore_audio_out_input_detach(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd, Eo *input)
{
   Ecore_Audio_System_Request *request;

   if (!efl_isa(input, ECORE_AUDIO_IN_CLASS)) return EINA_FALSE;
   if (pd->waiting)
     {
        if (!eina_list_data_find(pd->pending, input)) return EINA_FALSE;
        request = _request_add(pd, REQUEST_DETACH);
        if (!request) return EINA_FALSE;
        request->input = efl_ref(input);
        pd->pending = eina_list_remove(pd->pending, input);
        return EINA_TRUE;
     }
   if (!pd->backend) return EINA_FALSE;
   return ecore_audio_obj_out_input_detach(pd->backend, input);
}

EOLIAN static Eina_List *
_ecore_audio_out_system_ecore_audio_out_inputs_get(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd)
{
   if (pd->waiting) return pd->pending;
   if (!pd->backend) return NULL;
   return ecore_audio_obj_out_inputs_get(pd->backend);
}

#include "ecore_audio_out_system.eo.c"
