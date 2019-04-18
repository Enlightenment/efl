#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1

#include "config.h"
#include "Efl.h"

#define MY_CLASS EFL_IO_QUEUE_CLASS

/*
 * This queue is simple and based on a single buffer that is
 * reallocated as needed up to some limit, keeping some pre-allocated
 * amount of bytes.
 *
 * Writes appends to the buffer. Reads consume and remove data from
 * buffer head.
 *
 * To avoid too much memmove(), reads won't immediately remove data,
 * instead will only increment position_read and allow some
 * slack. When the slack limit is reached or the buffer needs more
 * memory for write, then the memmove() happens.
 *
 * A more complex and possibly efficient version of this would be to
 * keep a list of internal buffers of fixed size. Writing would result
 * into segment and write into these chunks, creating new if
 * needed. Reading would consume from multiple chunks and if they're
 * all used, would be freed.
 */

typedef struct _Efl_Io_Queue_Data
{
   uint8_t *bytes;
   size_t allocated;
   size_t preallocated;
   size_t limit;
   size_t position_read; /* to avoid memmove(), allows some slack */
   size_t position_write;
   Eina_Bool pending_eos;
   Eina_Bool eos;
   Eina_Bool closed;
   Eina_Bool can_read;
   Eina_Bool can_write;
} Efl_Io_Queue_Data;

static Eina_Bool
_efl_io_queue_realloc(Eo *o, Efl_Io_Queue_Data *pd, size_t size)
{
   void *tmp;
   size_t limit = efl_io_queue_limit_get(o);

   if ((limit > 0) && (size > limit))
     size = limit;

   if (pd->allocated == size) return EINA_FALSE;

   if (size == 0)
     {
        free(pd->bytes);
        tmp = NULL;
     }
   else
     {
        tmp = realloc(pd->bytes, size);
        EINA_SAFETY_ON_NULL_RETURN_VAL(tmp, EINA_FALSE);
     }

   pd->bytes = tmp;
   pd->allocated = size;
   return EINA_TRUE;
}

static size_t
_efl_io_queue_slack_get(const Efl_Io_Queue_Data *pd)
{
   const size_t used = pd->position_write - pd->position_read;

   if (used >= 4096) return 4096;
   else if (used >= 1024) return 1024;
   else if (used >= 128) return 128;
   else return 32;
}

static Eina_Bool
_efl_io_queue_realloc_rounded(Eo *o, Efl_Io_Queue_Data *pd, size_t size)
{
   if ((size > 0) && (size < 128))
     size = ((size / 32) + 1) * 32;
   else if (size < 1024)
     size = ((size / 128) + 1) * 128;
   else if (size < 8192)
     size = ((size / 1024) + 1) * 1024;
   else
     size = ((size / 4096) + 1) * 4096;

   return _efl_io_queue_realloc(o, pd, size);
}

/* reset position_read to zero, allowing all memory for write */
static void
_efl_io_queue_adjust(Efl_Io_Queue_Data *pd)
{
   size_t used = pd->position_write - pd->position_read;
   memmove(pd->bytes, pd->bytes + pd->position_read, used);
   pd->position_write = used;
   pd->position_read = 0;
}

static void
_efl_io_queue_adjust_and_realloc_if_needed(Eo *o, Efl_Io_Queue_Data *pd)
{
   const size_t slack = _efl_io_queue_slack_get(pd);
   size_t spare;

   if (pd->limit > 0)
     {
        if (pd->position_write + slack >= pd->limit)
          _efl_io_queue_adjust(pd);
     }
   else if (pd->position_read > slack)
     _efl_io_queue_adjust(pd);

   spare = pd->allocated - pd->position_write;
   if (spare > slack)
     {
        size_t new_size = pd->position_write + slack;

        /*
         * this may result in going over slack again, no
         * problems with that.
         */
        if (new_size < pd->preallocated)
          new_size = pd->preallocated;

        /* use rounded so we avoid too many reallocs */
        _efl_io_queue_realloc_rounded(o, pd, new_size);
     }
}

static void
_efl_io_queue_update_cans(Eo *o, Efl_Io_Queue_Data *pd)
{
   size_t used = pd->position_write - pd->position_read;
   size_t limit;

   efl_io_reader_can_read_set(o, used > 0);
   if (pd->closed) return; /* may be closed from "can_read,changed" */

   limit = efl_io_queue_limit_get(o);
   if (pd->pending_eos)
     efl_io_writer_can_write_set(o, EINA_FALSE);
   else
     efl_io_writer_can_write_set(o, (limit == 0) || (used < limit));
}

EOLIAN static void
_efl_io_queue_preallocate(Eo *o, Efl_Io_Queue_Data *pd, size_t size)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->allocated < size)
     _efl_io_queue_realloc_rounded(o, pd, size);
   pd->preallocated = size;
}

EOLIAN static void
_efl_io_queue_limit_set(Eo *o, Efl_Io_Queue_Data *pd, size_t limit)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));

   if (pd->limit == limit) return;
   pd->limit = limit;
   if (pd->limit == 0) goto end;

   _efl_io_queue_adjust(pd);

   if (pd->allocated > limit)
     _efl_io_queue_realloc(o, pd, limit);

   if (pd->position_write > limit)
     {
        pd->position_write = limit;
        if (pd->position_read > limit) pd->position_read = limit;
     }

   _efl_io_queue_adjust_and_realloc_if_needed(o, pd);
   efl_event_callback_call(o, EFL_IO_QUEUE_EVENT_SLICE_CHANGED, NULL);
   if (pd->closed) return;

 end:
   _efl_io_queue_update_cans(o, pd);
}

EOLIAN static size_t
_efl_io_queue_limit_get(const Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd)
{
   return pd->limit;
}

EOLIAN static size_t
_efl_io_queue_usage_get(const Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd)
{
   return pd->position_write - pd->position_read;
}

EOLIAN static Eina_Slice
_efl_io_queue_slice_get(const Eo *o, Efl_Io_Queue_Data *pd)
{
   Eina_Slice slice = { };

   if (!efl_io_closer_closed_get(o))
     {
        slice.mem = pd->bytes + pd->position_read;
        slice.len = efl_io_queue_usage_get(o);
     }

   return slice;
}

EOLIAN static void
_efl_io_queue_clear(Eo *o, Efl_Io_Queue_Data *pd)
{
   pd->position_read = 0;
   pd->position_write = 0;
   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_event_callback_call(o, EFL_IO_QUEUE_EVENT_SLICE_CHANGED, NULL);
   if (pd->closed) return;
   if (pd->pending_eos)
     efl_io_reader_eos_set(o, EINA_TRUE);
}

EOLIAN static void
_efl_io_queue_eos_mark(Eo *o, Efl_Io_Queue_Data *pd)
{
   if (pd->eos) return;

   if (efl_io_queue_usage_get(o) > 0)
     pd->pending_eos = EINA_TRUE;
   else
     efl_io_reader_eos_set(o, EINA_TRUE);
}

EOLIAN static Efl_Object *
_efl_io_queue_efl_object_finalize(Eo *o, Efl_Io_Queue_Data *pd EINA_UNUSED)
{
   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   _efl_io_queue_update_cans(o, pd);

   return o;
}

EOLIAN static void
_efl_io_queue_efl_object_destructor(Eo *o, Efl_Io_Queue_Data *pd)
{
   if (!efl_io_closer_closed_get(o))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   efl_destructor(efl_super(o, MY_CLASS));

   if (pd->bytes)
     {
        free(pd->bytes);
        pd->bytes = NULL;
        pd->allocated = 0;
        pd->position_read = 0;
        pd->position_write = 0;
     }
}

EOLIAN static Eina_Error
_efl_io_queue_efl_io_reader_read(Eo *o, Efl_Io_Queue_Data *pd, Eina_Rw_Slice *rw_slice)
{
   Eina_Slice ro_slice;
   size_t available;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);
   EINA_SAFETY_ON_TRUE_GOTO(efl_io_closer_closed_get(o), error);

   available = pd->position_write - pd->position_read;
   if (rw_slice->len > available)
     {
        rw_slice->len = available;
        if (rw_slice->len == 0)
          return EAGAIN;
     }

   ro_slice.len = rw_slice->len;
   ro_slice.mem = pd->bytes + pd->position_read;

   *rw_slice = eina_rw_slice_copy(*rw_slice, ro_slice);
   pd->position_read += ro_slice.len;

   efl_io_reader_can_read_set(o, pd->position_read < pd->position_write);
   efl_event_callback_call(o, EFL_IO_QUEUE_EVENT_SLICE_CHANGED, NULL);
   if (pd->closed) return 0;

   if ((pd->pending_eos) && (efl_io_queue_usage_get(o) == 0))
     efl_io_reader_eos_set(o, EINA_TRUE);

   return 0;

 error:
   rw_slice->len = 0;
   rw_slice->mem = NULL;
   efl_io_reader_can_read_set(o, EINA_FALSE);
   return EINVAL;
}

EOLIAN static void
_efl_io_queue_discard(Eo *o, Efl_Io_Queue_Data *pd, size_t amount)
{
   size_t available;

   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));

   available = pd->position_write - pd->position_read;
   if (amount > available)
     {
        amount = available;
        if (amount == 0)
          return;
     }

   pd->position_read += amount;

   efl_io_reader_can_read_set(o, pd->position_read < pd->position_write);
   efl_event_callback_call(o, EFL_IO_QUEUE_EVENT_SLICE_CHANGED, NULL);
   if (pd->closed) return;

   if ((pd->pending_eos) && (efl_io_queue_usage_get(o) == 0))
     efl_io_reader_eos_set(o, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_io_queue_efl_io_reader_can_read_get(const Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd)
{
   return pd->can_read;
}

EOLIAN static void
_efl_io_queue_efl_io_reader_can_read_set(Eo *o, Efl_Io_Queue_Data *pd, Eina_Bool can_read)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, &can_read);
}

EOLIAN static Eina_Bool
_efl_io_queue_efl_io_reader_eos_get(const Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd EINA_UNUSED)
{
   return pd->eos;
}

EOLIAN static void
_efl_io_queue_efl_io_reader_eos_set(Eo *o, Efl_Io_Queue_Data *pd EINA_UNUSED, Eina_Bool is_eos)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->eos == is_eos) return;
   pd->eos = is_eos;
   if (is_eos)
     {
        pd->pending_eos = EINA_FALSE;
        efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);
     }
}

EOLIAN static Eina_Error
_efl_io_queue_efl_io_writer_write(Eo *o, Efl_Io_Queue_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
   size_t available_write, available_total, todo, limit;
   int err = EINVAL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(slice, EINVAL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(slice->mem, EINVAL);
   EINA_SAFETY_ON_TRUE_GOTO(efl_io_closer_closed_get(o), error);

   err = EBADF;
   EINA_SAFETY_ON_TRUE_GOTO(pd->pending_eos, error);

   available_write = pd->allocated - pd->position_write;
   available_total = available_write + pd->position_read;
   limit = efl_io_queue_limit_get(o);

   err = ENOSPC;
   if (available_write >= slice->len)
     {
        todo = slice->len;
     }
   else if (available_total >= slice->len)
     {
        _efl_io_queue_adjust(pd);
        todo = slice->len;
     }
   else if ((limit > 0) && (pd->allocated == limit)) goto error;
   else
     {
        _efl_io_queue_adjust(pd);
        _efl_io_queue_realloc_rounded(o, pd, pd->position_write + slice->len);
        if (pd->allocated >= pd->position_write + slice->len)
          todo = slice->len;
        else
          todo = pd->allocated - pd->position_write;

        if (todo == 0) goto error;
     }

   memcpy(pd->bytes + pd->position_write, slice->mem, todo);
   if (remaining)
     {
        remaining->len = slice->len - todo;
        if (remaining->len)
          remaining->mem = slice->bytes + todo;
        else
          remaining->mem = NULL;
     }
   slice->len = todo;

   pd->position_write += todo;

   _efl_io_queue_adjust_and_realloc_if_needed(o, pd);
   efl_event_callback_call(o, EFL_IO_QUEUE_EVENT_SLICE_CHANGED, NULL);
   if (pd->closed) return 0;
   _efl_io_queue_update_cans(o, pd);

   return 0;

 error:
   if (remaining) *remaining = *slice;
   slice->len = 0;
   slice->mem = NULL;
   efl_io_writer_can_write_set(o, EINA_FALSE);
   return err;
}

EOLIAN static Eina_Bool
_efl_io_queue_efl_io_writer_can_write_get(const Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd)
{
   return pd->can_write;
}

EOLIAN static void
_efl_io_queue_efl_io_writer_can_write_set(Eo *o, Efl_Io_Queue_Data *pd, Eina_Bool can_write)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->can_write == can_write) return;
   pd->can_write = can_write;
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, &can_write);
}

EOLIAN static Eina_Error
_efl_io_queue_efl_io_closer_close(Eo *o, Efl_Io_Queue_Data *pd)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EINVAL);
   efl_io_queue_eos_mark(o);
   efl_io_queue_clear(o);
   pd->closed = EINA_TRUE;
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
   return 0;
}

EOLIAN static Eina_Bool
_efl_io_queue_efl_io_closer_closed_get(const Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd)
{
   return pd->closed;
}

EOLIAN static Eina_Bool
_efl_io_queue_efl_io_closer_close_on_exec_set(Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd EINA_UNUSED, Eina_Bool close_on_exec)
{
   if (!close_on_exec) return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_io_queue_efl_io_closer_close_on_exec_get(const Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static void
_efl_io_queue_efl_io_closer_close_on_invalidate_set(Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd EINA_UNUSED, Eina_Bool close_on_invalidate EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_efl_io_queue_efl_io_closer_close_on_invalidate_get(const Eo *o EINA_UNUSED, Efl_Io_Queue_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

#include "interfaces/efl_io_queue.eo.c"
