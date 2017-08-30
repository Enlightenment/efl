#define EFL_IO_COPIER_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"

#define MY_CLASS EFL_IO_COPIER_CLASS
#define DEF_READ_CHUNK_SIZE 4096

typedef struct _Efl_Io_Copier_Data
{
   Efl_Io_Reader *source;
   Efl_Io_Writer *destination;
   Eina_Future *inactivity_timer;
   Eina_Future *job;
   Eina_Binbuf *buf;
   Eina_Slice line_delimiter;
   size_t buffer_limit;
   size_t read_chunk_size;
   struct {
      uint64_t read, written, total;
   } progress;
   double timeout_inactivity;
   Eina_Bool closed;
   Eina_Bool done;
   Eina_Bool force_dispatch;
   Eina_Bool close_on_exec;
   Eina_Bool close_on_destructor;
} Efl_Io_Copier_Data;

static void _efl_io_copier_write(Eo *o, Efl_Io_Copier_Data *pd);
static void _efl_io_copier_read(Eo *o, Efl_Io_Copier_Data *pd);

#define _COPIER_DBG(o, pd) \
  do \
    { \
       if (eina_log_domain_level_check(_ecore_log_dom, EINA_LOG_LEVEL_DBG)) \
         { \
            DBG("copier={%p %s, refs=%d, closed=%d, done=%d, buf=%zd}", \
                o, \
                efl_class_name_get(efl_class_get(o)), \
                efl_ref_get(o), \
                efl_io_closer_closed_get(o), \
                pd->done, \
                pd->buf ? eina_binbuf_length_get(pd->buf): 0); \
            if (!pd->source) \
              DBG("source=NULL"); \
            else \
              DBG("source={%p %s, refs=%d, can_read=%d, eos=%d, closed=%d}", \
                  pd->source, \
                  efl_class_name_get(efl_class_get(pd->source)), \
                  efl_ref_get(pd->source), \
                  efl_io_reader_can_read_get(pd->source), \
                  efl_io_reader_eos_get(pd->source), \
                  efl_isa(pd->source, EFL_IO_CLOSER_MIXIN) ? \
                  efl_io_closer_closed_get(pd->source) : 0); \
            if (!pd->destination) \
              DBG("destination=NULL"); \
            else \
              DBG("destination={%p %s, refs=%d, can_write=%d, closed=%d}", \
                  pd->destination, \
                  efl_class_name_get(efl_class_get(pd->destination)), \
                  efl_ref_get(pd->destination), \
                  efl_io_writer_can_write_get(pd->destination), \
                  efl_isa(pd->destination, EFL_IO_CLOSER_MIXIN) ? \
                  efl_io_closer_closed_get(pd->destination) : 0); \
         } \
    } \
  while (0)

static Eina_Value
_efl_io_copier_timeout_inactivity_cb(Eo *o, const Eina_Value v)
{
   Eina_Error err = ETIMEDOUT;
   efl_event_callback_call(o, EFL_IO_COPIER_EVENT_ERROR, &err);
   return v;
}

static void
_efl_io_copier_timeout_inactivity_reschedule(Eo *o, Efl_Io_Copier_Data *pd)
{
   if (pd->inactivity_timer) eina_future_cancel(pd->inactivity_timer);
   if (pd->timeout_inactivity <= 0.0) return;

   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_timeout(efl_loop_get(o), pd->timeout_inactivity),
                                  .success = _efl_io_copier_timeout_inactivity_cb,
                                  .storage = &pd->inactivity_timer);
}

static Eina_Value
_efl_io_copier_job(Eo *o, const Eina_Value v)
{
   Efl_Io_Copier_Data *pd = efl_data_scope_get(o, MY_CLASS);
   uint64_t old_read = pd->progress.read;
   uint64_t old_written = pd->progress.written;
   uint64_t old_total = pd->progress.total;

   _COPIER_DBG(o, pd);

   efl_ref(o);

   if (pd->source && efl_io_reader_can_read_get(pd->source))
     _efl_io_copier_read(o, pd);

   if (pd->destination && efl_io_writer_can_write_get(pd->destination))
     _efl_io_copier_write(o, pd);

   if ((old_read != pd->progress.read) ||
       (old_written != pd->progress.written) ||
       (old_total != pd->progress.total))
     {
        efl_event_callback_call(o, EFL_IO_COPIER_EVENT_PROGRESS, NULL);
        if (pd->closed) return v; /* cb may call close */
        _efl_io_copier_timeout_inactivity_reschedule(o, pd);
     }

   if (!pd->source || efl_io_reader_eos_get(pd->source))
     {
        if ((!pd->done) &&
            ((!pd->destination) || (eina_binbuf_length_get(pd->buf) == 0)))
          efl_io_copier_done_set(o, EINA_TRUE);
     }

   efl_unref(o);
   return v;
}

static void
_efl_io_copier_job_schedule(Eo *o, Efl_Io_Copier_Data *pd)
{
   if (pd->job) return;

   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_job(efl_loop_get(o)),
                                  .success = _efl_io_copier_job,
                                  .storage = &pd->job);
}

/* NOTE: the returned slice may be smaller than requested since the
 * internal binbuf may be modified from inside event calls.
 *
 * parameter slice_of_binbuf must have mem pointing to pd->binbuf
 */
static Eina_Slice
_efl_io_copier_dispatch_data_events(Eo *o, Efl_Io_Copier_Data *pd, Eina_Slice slice_of_binbuf)
{
   Eina_Slice tmp;
   size_t offset;

   tmp = eina_binbuf_slice_get(pd->buf);
   if ((slice_of_binbuf.bytes < tmp.bytes) ||
       (eina_slice_end_get(slice_of_binbuf) > eina_slice_end_get(tmp)))
     {
        CRI("slice_of_binbuf=" EINA_SLICE_FMT " must be inside binbuf=" EINA_SLICE_FMT,
            EINA_SLICE_PRINT(slice_of_binbuf), EINA_SLICE_PRINT(tmp));
        return (Eina_Slice){.mem = NULL, .len = 0};
     }

   offset = slice_of_binbuf.bytes - tmp.bytes;

   efl_event_callback_call(o, EFL_IO_COPIER_EVENT_DATA, &slice_of_binbuf);
   if (pd->closed) return (Eina_Slice){.mem = NULL, .len = 0}; /* cb may call close */

   /* user may have modified pd->buf, like calling
    * efl_io_copier_buffer_limit_set()
    */
   tmp = eina_binbuf_slice_get(pd->buf);
   if (offset <= tmp.len)
     {
        tmp.len -= offset;
        tmp.bytes += offset;
     }
   if (tmp.len > slice_of_binbuf.len)
     tmp.len = slice_of_binbuf.len;
   slice_of_binbuf = tmp;

   if (pd->line_delimiter.len > 0)
     {
        efl_event_callback_call(o, EFL_IO_COPIER_EVENT_LINE, &slice_of_binbuf);
        if (pd->closed) return (Eina_Slice){.mem = NULL, .len = 0}; /* cb may call close */

        /* user may have modified pd->buf, like calling
         * efl_io_copier_buffer_limit_set()
         */
        tmp = eina_binbuf_slice_get(pd->buf);
        if (offset <= tmp.len)
          {
             tmp.len -= offset;
             tmp.bytes += offset;
          }
        if (tmp.len > slice_of_binbuf.len)
          tmp.len = slice_of_binbuf.len;
        slice_of_binbuf = tmp;
     }

   return slice_of_binbuf;
}

static void
_efl_io_copier_read(Eo *o, Efl_Io_Copier_Data *pd)
{
   Eina_Rw_Slice rw_slice;
   Eina_Error err;
   size_t used, expand_size;

   EINA_SAFETY_ON_TRUE_RETURN(pd->closed);

   expand_size = pd->read_chunk_size;
   used = eina_binbuf_length_get(pd->buf);
   if (pd->buffer_limit > 0)
     {
        if (pd->buffer_limit <= used)
          {
             return;
          }
        else if (pd->buffer_limit > used)
          {
             size_t available = pd->buffer_limit - used;
             if (expand_size > available)
               expand_size = available;
          }
     }

   rw_slice = eina_binbuf_expand(pd->buf, expand_size);
   if (rw_slice.len == 0)
     {
        err = ENOMEM;
        efl_event_callback_call(o, EFL_IO_COPIER_EVENT_ERROR, &err);
        return;
     }

   err = efl_io_reader_read(pd->source, &rw_slice);
   if (err)
     {
        if (err != EAGAIN)
          efl_event_callback_call(o, EFL_IO_COPIER_EVENT_ERROR, &err);
        return;
     }

   if (pd->closed) return; /* read(source) triggers cb, may call close */

   if (!eina_binbuf_use(pd->buf, rw_slice.len))
     {
        err = ENOMEM;
        efl_event_callback_call(o, EFL_IO_COPIER_EVENT_ERROR, &err);
        return;
     }

   pd->progress.read += rw_slice.len;
   efl_io_copier_done_set(o, EINA_FALSE);

   if ((!pd->destination) && (eina_binbuf_length_get(pd->buf) > used))
     {
        /* Note: if there is a destination, dispatch data and line
         * from write since it will remove from binbuf and make it
         * simple to not repeat data that was already sent.
         *
         * however, if there is no destination, then emit the event
         * here.
         */
        _efl_io_copier_dispatch_data_events(o, pd, eina_rw_slice_slice_get(rw_slice));
     }

   _efl_io_copier_job_schedule(o, pd);
}

static void
_efl_io_copier_write(Eo *o, Efl_Io_Copier_Data *pd)
{
   Eina_Slice ro_slice;
   Eina_Error err;

   EINA_SAFETY_ON_TRUE_RETURN(pd->closed);
   EINA_SAFETY_ON_NULL_RETURN(pd->buf);

   ro_slice = eina_binbuf_slice_get(pd->buf);
   if (ro_slice.len == 0)
     {
        return;
     }

   if ((pd->line_delimiter.len > 0) && (!pd->force_dispatch) &&
       (pd->source && !efl_io_reader_eos_get(pd->source)))
     {
        const uint8_t *p = eina_slice_find(ro_slice, pd->line_delimiter);
        if (p)
          ro_slice.len = p - ro_slice.bytes + pd->line_delimiter.len;
        else if ((pd->buffer_limit == 0) || (ro_slice.len < pd->buffer_limit))
          {
             return;
          }
     }

   err = efl_io_writer_write(pd->destination, &ro_slice, NULL);
   if (err)
     {
        if (err != EAGAIN)
          efl_event_callback_call(o, EFL_IO_COPIER_EVENT_ERROR, &err);
        return;
     }
   if (ro_slice.len == 0)
     return;

   pd->progress.written += ro_slice.len;

   if (pd->closed) return; /* write(destination) triggers cb, may call close */

   efl_io_copier_done_set(o, EINA_FALSE);

   /* Note: dispatch data and line from write since it will remove
    * from binbuf and make it simple to not repeat data that was
    * already sent.
    */
   ro_slice = _efl_io_copier_dispatch_data_events(o, pd, ro_slice);

   if (!eina_binbuf_remove(pd->buf, 0, ro_slice.len))
     {
        err = ENOMEM;
        efl_event_callback_call(o, EFL_IO_COPIER_EVENT_ERROR, &err);
        return;
     }

   _efl_io_copier_job_schedule(o, pd);
}

static void
_efl_io_copier_source_can_read_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Io_Copier_Data *pd = efl_data_scope_get(o, MY_CLASS);
   if (pd->closed) return;

   _COPIER_DBG(o, pd);

   if (efl_io_reader_can_read_get(pd->source))
     _efl_io_copier_job_schedule(o, pd);
}

static void
_efl_io_copier_source_eos(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Io_Copier_Data *pd = efl_data_scope_get(o, MY_CLASS);
   if (pd->closed) return;

   _COPIER_DBG(o, pd);

   _efl_io_copier_job_schedule(o, pd);
}

static void
_efl_io_copier_source_size_apply(Eo *o, Efl_Io_Copier_Data *pd)
{
   if (pd->closed) return;
   pd->progress.total = efl_io_sizer_size_get(pd->source);

   _COPIER_DBG(o, pd);

   if (pd->destination && efl_isa(pd->destination, EFL_IO_SIZER_MIXIN))
     efl_io_sizer_resize(pd->destination, pd->progress.total);

   efl_event_callback_call(o, EFL_IO_COPIER_EVENT_PROGRESS, NULL);
}

static void
_efl_io_copier_source_resized(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Io_Copier_Data *pd = efl_data_scope_get(o, MY_CLASS);
   _efl_io_copier_source_size_apply(o, pd);
}

static void
_efl_io_copier_source_closed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Io_Copier_Data *pd = efl_data_scope_get(o, MY_CLASS);
   if (pd->closed) return;

   _COPIER_DBG(o, pd);

   _efl_io_copier_job_schedule(o, pd);
}

EFL_CALLBACKS_ARRAY_DEFINE(source_cbs,
                          { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _efl_io_copier_source_can_read_changed },
                          { EFL_IO_READER_EVENT_EOS, _efl_io_copier_source_eos });

EOLIAN static Efl_Io_Reader *
_efl_io_copier_source_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->source;
}

EOLIAN static void
_efl_io_copier_source_set(Eo *o, Efl_Io_Copier_Data *pd, Efl_Io_Reader *source)
{
   if (pd->source == source) return;

   if (pd->source)
     {
        if (efl_isa(pd->source, EFL_IO_SIZER_MIXIN))
          {
             efl_event_callback_del(pd->source, EFL_IO_SIZER_EVENT_SIZE_CHANGED,
                                    _efl_io_copier_source_resized, o);
             pd->progress.total = 0;
          }
        if (efl_isa(pd->source, EFL_IO_CLOSER_MIXIN))
          {
             efl_event_callback_del(pd->source, EFL_IO_CLOSER_EVENT_CLOSED,
                                    _efl_io_copier_source_closed, o);
          }
        efl_event_callback_array_del(pd->source, source_cbs(), o);
        efl_unref(pd->source);
        pd->source = NULL;
     }

   if (source)
     {
        EINA_SAFETY_ON_TRUE_RETURN(pd->closed);
        pd->source = efl_ref(source);
        efl_event_callback_array_add(pd->source, source_cbs(), o);

        if (efl_isa(pd->source, EFL_IO_SIZER_MIXIN))
          {
             efl_event_callback_add(pd->source, EFL_IO_SIZER_EVENT_SIZE_CHANGED,
                                    _efl_io_copier_source_resized, o);
             _efl_io_copier_source_size_apply(o, pd);
          }

        if (efl_isa(pd->source, EFL_IO_CLOSER_MIXIN))
          {
             efl_io_closer_close_on_exec_set(pd->source, efl_io_closer_close_on_exec_get(o));
             efl_io_closer_close_on_destructor_set(pd->source, efl_io_closer_close_on_destructor_get(o));
             efl_event_callback_add(pd->source, EFL_IO_CLOSER_EVENT_CLOSED,
                                     _efl_io_copier_source_closed, o);
          }
     }
}

static void
_efl_io_copier_destination_can_write_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Io_Copier_Data *pd = efl_data_scope_get(o, MY_CLASS);
   if (pd->closed) return;

   _COPIER_DBG(o, pd);

   if (efl_io_writer_can_write_get(pd->destination))
     _efl_io_copier_job_schedule(o, pd);
}

static void
_efl_io_copier_destination_closed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Io_Copier_Data *pd = efl_data_scope_get(o, MY_CLASS);
   if (pd->closed) return;

   _COPIER_DBG(o, pd);

   if (eina_binbuf_length_get(pd->buf) == 0)
     {
        if (!pd->done)
          efl_io_copier_done_set(o, EINA_TRUE);
     }
   else
     {
        Eina_Error err = EBADF;
        if (pd->inactivity_timer) eina_future_cancel(pd->inactivity_timer);
        WRN("copier %p destination %p closed with %zd bytes pending...",
            o, pd->destination, eina_binbuf_length_get(pd->buf));
        efl_event_callback_call(o, EFL_IO_COPIER_EVENT_ERROR, &err);
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(destination_cbs,
                          { EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, _efl_io_copier_destination_can_write_changed });

EOLIAN static Efl_Io_Writer *
_efl_io_copier_destination_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->destination;
}

EOLIAN static void
_efl_io_copier_destination_set(Eo *o, Efl_Io_Copier_Data *pd, Efl_Io_Writer *destination)
{
   if (pd->destination == destination) return;

   if (pd->destination)
     {
        efl_event_callback_array_del(pd->destination, destination_cbs(), o);
        if (efl_isa(pd->destination, EFL_IO_CLOSER_MIXIN))
          {
             efl_event_callback_del(pd->destination, EFL_IO_CLOSER_EVENT_CLOSED,
                                    _efl_io_copier_destination_closed, o);
          }
        efl_unref(pd->destination);
        pd->destination = NULL;
     }

   if (destination)
     {
        EINA_SAFETY_ON_TRUE_RETURN(pd->closed);
        pd->destination = efl_ref(destination);
        efl_event_callback_array_add(pd->destination, destination_cbs(), o);

        if (efl_isa(pd->destination, EFL_IO_CLOSER_MIXIN))
          {
             efl_io_closer_close_on_exec_set(pd->destination, efl_io_closer_close_on_exec_get(o));
             efl_io_closer_close_on_destructor_set(pd->destination, efl_io_closer_close_on_destructor_get(o));
             efl_event_callback_add(pd->destination, EFL_IO_CLOSER_EVENT_CLOSED,
                                     _efl_io_copier_destination_closed, o);
          }
        if (efl_isa(pd->destination, EFL_IO_SIZER_MIXIN) &&
            pd->source && efl_isa(pd->source, EFL_IO_SIZER_MIXIN))
          {
             efl_io_sizer_resize(pd->destination, pd->progress.total);
          }
     }
}

EOLIAN static void
_efl_io_copier_buffer_limit_set(Eo *o, Efl_Io_Copier_Data *pd, size_t size)
{
   size_t used;

   EINA_SAFETY_ON_TRUE_RETURN(pd->closed);

   if (pd->buffer_limit == size) return;
   pd->buffer_limit = size;
   if (size == 0) return;

   used = eina_binbuf_length_get(pd->buf);
   if (used > size) eina_binbuf_remove(pd->buf, size, used);
   if (pd->read_chunk_size > size) efl_io_copier_read_chunk_size_set(o, size);
}

EOLIAN static size_t
_efl_io_copier_buffer_limit_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->buffer_limit;
}

EOLIAN static void
_efl_io_copier_line_delimiter_set(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd, Eina_Slice slice)
{
   if (pd->line_delimiter.mem == slice.mem)
     {
        pd->line_delimiter.len = slice.len;
        return;
     }

   free((void *)pd->line_delimiter.mem);
   if (slice.len == 0)
     {
        pd->line_delimiter.mem = NULL;
        pd->line_delimiter.len = 0;
     }
   else
     {
        Eina_Rw_Slice rw_slice = eina_slice_dup(slice);
        pd->line_delimiter = eina_rw_slice_slice_get(rw_slice);
     }
}

EOLIAN static Eina_Slice
_efl_io_copier_line_delimiter_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->line_delimiter;
}


EOLIAN static void
_efl_io_copier_read_chunk_size_set(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd, size_t size)
{
   EINA_SAFETY_ON_TRUE_RETURN(pd->closed);

   if (size == 0) size = DEF_READ_CHUNK_SIZE;
   pd->read_chunk_size = size;
}

EOLIAN static size_t
_efl_io_copier_read_chunk_size_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->read_chunk_size > 0 ? pd->read_chunk_size : DEF_READ_CHUNK_SIZE;
}

EOLIAN static Eina_Error
_efl_io_copier_efl_io_closer_close(Eo *o, Efl_Io_Copier_Data *pd)
{
   Eina_Error err = 0, r;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->closed, EINVAL);

   _COPIER_DBG(o, pd);

   while (pd->buf)
     {
        size_t pending = eina_binbuf_length_get(pd->buf);
        if (pending == 0) break;
        else if (pd->destination && efl_io_writer_can_write_get(pd->destination))
          {
             DBG("copier %p destination %p closed with %zd bytes pending, do final write...",
                 o, pd->destination, pending);
             pd->force_dispatch = EINA_TRUE;
             _efl_io_copier_write(o, pd);
             pd->force_dispatch = EINA_FALSE;
          }
        else if (!pd->destination)
          {
             Eina_Slice binbuf_slice = eina_binbuf_slice_get(pd->buf);
             DBG("copier %p destination %p closed with %zd bytes pending, dispatch events...",
                 o, pd->destination, pending);
             _efl_io_copier_dispatch_data_events(o, pd, binbuf_slice);
             break;
          }
        else
          {
             DBG("copier %p destination %p closed with %zd bytes pending...",
                 o, pd->destination, pending);
             break;
          }
     }

   if (pd->job)
     eina_future_cancel(pd->job);

   if (pd->inactivity_timer)
     eina_future_cancel(pd->inactivity_timer);

   if (!pd->done)
     efl_io_copier_done_set(o, EINA_TRUE);

   if (pd->source)
     {
        if (efl_isa(pd->source, EFL_IO_SIZER_MIXIN))
          {
             efl_event_callback_del(pd->source, EFL_IO_SIZER_EVENT_SIZE_CHANGED,
                                    _efl_io_copier_source_resized, o);
             pd->progress.total = 0;
          }
        efl_event_callback_array_del(pd->source, source_cbs(), o);
        if (efl_isa(pd->source, EFL_IO_CLOSER_MIXIN) &&
            !efl_io_closer_closed_get(pd->source))
          {
             efl_event_callback_del(pd->source, EFL_IO_CLOSER_EVENT_CLOSED,
                                    _efl_io_copier_source_closed, o);
             err = efl_io_closer_close(pd->source);
          }
     }

   if (pd->destination)
     {
        efl_event_callback_array_del(pd->destination, destination_cbs(), o);
        if (efl_isa(pd->destination, EFL_IO_CLOSER_MIXIN) &&
            !efl_io_closer_closed_get(pd->destination))
          {
             efl_event_callback_del(pd->destination, EFL_IO_CLOSER_EVENT_CLOSED,
                                    _efl_io_copier_destination_closed, o);
             r = efl_io_closer_close(pd->destination);
             if (!err) err = r;
          }
     }

   pd->closed = EINA_TRUE;
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);

   if (pd->buf)
     {
        eina_binbuf_free(pd->buf);
        pd->buf = NULL;
     }

   return err;
}

EOLIAN static Eina_Bool
_efl_io_copier_efl_io_closer_closed_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->closed;
}

EOLIAN static void
_efl_io_copier_progress_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd, uint64_t *read, uint64_t *written, uint64_t *total)
{
   if (read) *read = pd->progress.read;
   if (written) *written = pd->progress.written;
   if (total) *total = pd->progress.total;
}

EOLIAN static Eina_Binbuf *
_efl_io_copier_binbuf_steal(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   Eina_Binbuf *ret = pd->buf;
   pd->buf = eina_binbuf_new();
   return ret;
}

EOLIAN static void
_efl_io_copier_timeout_inactivity_set(Eo *o, Efl_Io_Copier_Data *pd, double seconds)
{
   pd->timeout_inactivity = seconds;
   _efl_io_copier_timeout_inactivity_reschedule(o, pd);
}

EOLIAN static double
_efl_io_copier_timeout_inactivity_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->timeout_inactivity;
}

EOLIAN static Eina_Bool
_efl_io_copier_flush(Eo *o, Efl_Io_Copier_Data *pd, Eina_Bool may_block, Eina_Bool ignore_line_delimiter)
{
   uint64_t old_read = pd->progress.read;
   uint64_t old_written = pd->progress.written;
   uint64_t old_total = pd->progress.total;

   _COPIER_DBG(o, pd);

   if (pd->source && !efl_io_reader_eos_get(pd->source))
     {
        if (may_block || efl_io_reader_can_read_get(pd->source))
          _efl_io_copier_read(o, pd);
     }

   if (pd->destination)
     {
        if (may_block || efl_io_writer_can_write_get(pd->destination))
          {
             pd->force_dispatch = ignore_line_delimiter;
             _efl_io_copier_write(o, pd);
             pd->force_dispatch = EINA_FALSE;
          }
     }
   else if (ignore_line_delimiter && pd->buf)
     {
        size_t pending = eina_binbuf_length_get(pd->buf);
        if (pending)
          {
             Eina_Slice binbuf_slice = eina_binbuf_slice_get(pd->buf);
             _efl_io_copier_dispatch_data_events(o, pd, binbuf_slice);
          }
     }

   if ((old_read != pd->progress.read) ||
       (old_written != pd->progress.written) ||
       (old_total != pd->progress.total))
     {
        efl_event_callback_call(o, EFL_IO_COPIER_EVENT_PROGRESS, NULL);
        if (pd->closed) return EINA_TRUE; /* cb may call close */
        _efl_io_copier_timeout_inactivity_reschedule(o, pd);
     }

   if (!pd->source || efl_io_reader_eos_get(pd->source))
     {
        if ((!pd->done) &&
            ((!pd->destination) || (eina_binbuf_length_get(pd->buf) == 0)))
          efl_io_copier_done_set(o, EINA_TRUE);
     }

   return pd->done;
}

EOLIAN static size_t
_efl_io_copier_pending_size_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->buf ? eina_binbuf_length_get(pd->buf) : 0;
}

EOLIAN static Eina_Bool
_efl_io_copier_done_get(Eo *o, Efl_Io_Copier_Data *pd)
{
   DBG("%p done=%d pending=%zd source={%p %s, eos=%d, closed=%d}, destination={%p %s, closed=%d}",
       o, pd->done,
       pd->buf ? eina_binbuf_length_get(pd->buf) : 0,
       pd->source,
       pd->source ? efl_class_name_get(pd->source) : "",
       pd->source ? efl_io_reader_eos_get(pd->source) : 1,
       pd->source ? (efl_isa(pd->source, EFL_IO_CLOSER_MIXIN) ? efl_io_closer_closed_get(pd->source) : 0) : 1,
       pd->destination,
       pd->destination ? efl_class_name_get(pd->destination) : "",
       pd->destination ? (efl_isa(pd->destination, EFL_IO_CLOSER_MIXIN) ? efl_io_closer_closed_get(pd->destination) : 0) : 1);

   return pd->done;
}

EOLIAN static void
_efl_io_copier_done_set(Eo *o, Efl_Io_Copier_Data *pd, Eina_Bool value)
{
   if (pd->done == value) return;
   pd->done = value;
   if (!value) return;
   if (pd->inactivity_timer) eina_future_cancel(pd->inactivity_timer);
   efl_event_callback_call(o, EFL_IO_COPIER_EVENT_DONE, NULL);
}


EOLIAN static Eo *
_efl_io_copier_efl_object_constructor(Eo *o, Efl_Io_Copier_Data *pd)
{
   pd->buf = eina_binbuf_new();
   pd->close_on_exec = EINA_TRUE;
   pd->close_on_destructor = EINA_TRUE;
   pd->timeout_inactivity = 0.0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->buf, NULL);

   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static Eo *
_efl_io_copier_efl_object_finalize(Eo *o, Efl_Io_Copier_Data *pd)
{
   if (pd->read_chunk_size == 0)
     efl_io_copier_read_chunk_size_set(o, DEF_READ_CHUNK_SIZE);

   if (!efl_loop_get(o))
     {
        ERR("Set a loop provider as parent of this copier!");
        return NULL;
     }

   if ((pd->source && efl_io_reader_can_read_get(pd->source)) ||
       (pd->destination && efl_io_writer_can_write_get(pd->destination)))
     _efl_io_copier_job_schedule(o, pd);

   _COPIER_DBG(o, pd);

   return efl_finalize(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_io_copier_efl_object_destructor(Eo *o, Efl_Io_Copier_Data *pd)
{
   _COPIER_DBG(o, pd);

   if (pd->job)
     eina_future_cancel(pd->job);

   if (pd->inactivity_timer)
     eina_future_cancel(pd->inactivity_timer);

   if (efl_io_closer_close_on_destructor_get(o) &&
       (!efl_io_closer_closed_get(o)))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   efl_io_copier_source_set(o, NULL);
   efl_io_copier_destination_set(o, NULL);

   efl_destructor(efl_super(o, MY_CLASS));

   if (pd->buf)
     {
        eina_binbuf_free(pd->buf);
        pd->buf = NULL;
     }

   if (pd->line_delimiter.mem)
     {
        free((void *)pd->line_delimiter.mem);
        pd->line_delimiter.mem = NULL;
        pd->line_delimiter.len = 0;
     }
}

EOLIAN static Eina_Bool
_efl_io_copier_efl_io_closer_close_on_exec_set(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd, Eina_Bool close_on_exec)
{
   if (pd->close_on_exec == close_on_exec) return EINA_TRUE;
   pd->close_on_exec = close_on_exec;

   if (pd->source && efl_isa(pd->source, EFL_IO_CLOSER_MIXIN))
     efl_io_closer_close_on_exec_set(pd->source, close_on_exec);

   if (pd->destination && efl_isa(pd->destination, EFL_IO_CLOSER_MIXIN))
     efl_io_closer_close_on_exec_set(pd->destination, close_on_exec);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_io_copier_efl_io_closer_close_on_exec_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->close_on_exec;
}

EOLIAN static void
_efl_io_copier_efl_io_closer_close_on_destructor_set(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd, Eina_Bool close_on_destructor)
{
   if (pd->close_on_destructor == close_on_destructor) return;
   pd->close_on_destructor = close_on_destructor;

   if (pd->source && efl_isa(pd->source, EFL_IO_CLOSER_MIXIN))
     efl_io_closer_close_on_destructor_set(pd->source, close_on_destructor);

   if (pd->destination && efl_isa(pd->destination, EFL_IO_CLOSER_MIXIN))
     efl_io_closer_close_on_destructor_set(pd->destination, close_on_destructor);
}

EOLIAN static Eina_Bool
_efl_io_copier_efl_io_closer_close_on_destructor_get(Eo *o EINA_UNUSED, Efl_Io_Copier_Data *pd)
{
   return pd->close_on_destructor;
}

#include "efl_io_copier.eo.c"
