#include "shared_buffer.h"

struct _Shared_Buffer
{
   Ecore_Buffer *buffer;
   Shared_Buffer_State state;
   struct bq_buffer *proxy;
   int w, h;
   int format;
   unsigned int flags;
   const char *backend;
};

Shared_Buffer *
_shared_buffer_new(struct bq_buffer *proxy, int w, int h, int format, unsigned int flags)
{
   Shared_Buffer *sb;

   sb = calloc(sizeof(Shared_Buffer), 1);
   if (!sb)
     return NULL;

   sb->proxy = proxy;
   sb->w = w;
   sb->h = h;
   sb->format = format;
   sb->flags = flags;

   return sb;
}

void
_shared_buffer_free(Shared_Buffer *sb)
{
   if (!sb)
     return;

   free(sb);
}

Eina_Bool
_shared_buffer_info_get(Shared_Buffer *sb, int *w, int *h, int *format, unsigned int *flags)
{
   if (!sb)
     return EINA_FALSE;

   if (w) *w = sb->w;
   if (h) *h = sb->h;
   if (format) *format = sb->format;
   if (flags) *flags = sb->flags;

   return EINA_TRUE;
}

Eina_Bool
_shared_buffer_buffer_set(Shared_Buffer *sb, Ecore_Buffer *buffer)
{
   if (!sb)
     return EINA_FALSE;

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
   if (!sb)
     return NULL;

   return sb->buffer;
}

Eina_Bool
_shared_buffer_proxy_set(Shared_Buffer *sb, struct bq_buffer *proxy)
{
   if (!sb)
     return EINA_FALSE;

   if (sb->proxy)
     {
        ERR("Already exist proxy");
        return EINA_FALSE;
     }

   sb->proxy = proxy;

   return EINA_TRUE;
}

struct bq_buffer *
_shared_buffer_proxy_get(Shared_Buffer *sb)
{
   if (!sb)
     return NULL;

   return sb->proxy;
}

void
_shared_buffer_state_set(Shared_Buffer *sb, Shared_Buffer_State state)
{
   if (!sb)
     return;

   sb->state = state;
}

Shared_Buffer_State
_shared_buffer_state_get(Shared_Buffer *sb)
{
   if (!sb)
     return SHARED_BUFFER_STATE_UNKNOWN;

   return sb->state;
}

const char *
_shared_buffer_state_string_get(Shared_Buffer *sb)
{
   if (!sb)
     return "INVALID_OBJECT";

   switch (sb->state)
     {
      case SHARED_BUFFER_STATE_ENQUEUE:
         return "SHARED_BUFFER_STATE_ENQUEUE";
      case SHARED_BUFFER_STATE_SUBMIT:
         return "SHARED_BUFFER_STATE_SUBMIT";
      case SHARED_BUFFER_STATE_DEQUEUE:
         return "SHARED_BUFFER_STATE_SUBMIT";
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
