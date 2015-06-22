#include "shared_buffer.h"

struct _Shared_Buffer
{
   Ecore_Buffer *buffer;
   struct bq_buffer *resource;
   const char *engine;
   int w, h;
   int format;
   unsigned int flags;
   Shared_Buffer_State state;
};

Shared_Buffer *
_shared_buffer_new(const char *engine, struct bq_buffer *resource, int w, int h, int format, unsigned int flags)
{
   Shared_Buffer *sb;

   sb = calloc(1, sizeof(Shared_Buffer));
   if (!sb)
     return NULL;

   sb->engine = eina_stringshare_add(engine);
   sb->resource = resource;
   sb->w = w;
   sb->h = h;
   sb->format = format;
   sb->flags = flags;
   sb->state = SHARED_BUFFER_STATE_NEW;

   return sb;
}

void
_shared_buffer_free(Shared_Buffer *sb)
{
   EINA_SAFETY_ON_NULL_RETURN(sb);

   if (sb->engine) eina_stringshare_del(sb->engine);
   free(sb);
}

Eina_Bool
_shared_buffer_info_get(Shared_Buffer *sb, const char **engine, int *w, int *h, int *format, unsigned int *flags)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sb, EINA_FALSE);

   if (engine) *engine = sb->engine;
   if (w) *w = sb->w;
   if (h) *h = sb->h;
   if (format) *format = sb->format;
   if (flags) *flags = sb->flags;

   return EINA_TRUE;
}

Eina_Bool
_shared_buffer_buffer_set(Shared_Buffer *sb, Ecore_Buffer *buffer)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sb, EINA_FALSE);

   if (sb->buffer)
     {
        ERR("Already exist buffer");
        return EINA_FALSE;
     }

   sb->buffer = buffer;

   return EINA_TRUE;
}

Ecore_Buffer *
_shared_buffer_buffer_get(Shared_Buffer *sb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sb, NULL);

   return sb->buffer;
}

Eina_Bool
_shared_buffer_resource_set(Shared_Buffer *sb, struct bq_buffer *resource)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sb, EINA_FALSE);

   if (sb->resource)
     {
        ERR("Already exist resource");
        return EINA_FALSE;
     }

   sb->resource = resource;

   return EINA_TRUE;
}

struct bq_buffer *
_shared_buffer_resource_get(Shared_Buffer *sb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sb, NULL);

   return sb->resource;
}

void
_shared_buffer_state_set(Shared_Buffer *sb, Shared_Buffer_State state)
{
   EINA_SAFETY_ON_NULL_RETURN(sb);

   sb->state = state;
}

Shared_Buffer_State
_shared_buffer_state_get(Shared_Buffer *sb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sb, SHARED_BUFFER_STATE_UNKNOWN);

   return sb->state;
}

const char *
_shared_buffer_state_string_get(Shared_Buffer *sb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sb, "INVAILD OBJECT");

   switch (sb->state)
     {
      case SHARED_BUFFER_STATE_ENQUEUE:
         return "SHARED_BUFFER_STATE_ENQUEUE";
      case SHARED_BUFFER_STATE_SUBMIT:
         return "SHARED_BUFFER_STATE_SUBMIT";
      case SHARED_BUFFER_STATE_DEQUEUE:
         return "SHARED_BUFFER_STATE_DEQUEUE";
      case SHARED_BUFFER_STATE_ATTACH:
         return "SHARED_BUFFER_STATE_ATTACH";
      case SHARED_BUFFER_STATE_IMPORT:
         return "SHARED_BUFFER_STATE_IMPORT";
      case SHARED_BUFFER_STATE_DETACH:
         return "SHARED_BUFFER_STATE_DETACH";
      case SHARED_BUFFER_STATE_ACQUIRE:
         return "SHARED_BUFFER_STATE_ACQUIRE";
      case SHARED_BUFFER_STATE_RELEASE:
         return "SHARED_BUFFER_STATE_RELEASE";
      default:
      case SHARED_BUFFER_STATE_UNKNOWN:
         return "SHARED_BUFFER_STATE_UNKNOWN";
     }
}
